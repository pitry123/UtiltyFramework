#pragma once
#include <core/video.h>
#include <core/serializable_interface.h>

#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/dispatcher.hpp>
#include <utils/buffer_allocator.hpp>

#include <utils/thread_safe_object.hpp>

#include <functional>
#include <vector>
#include <mutex>
#include <exception>
#include <cstring>
#include <cstddef>

#if defined(__clang__)
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Winconsistent-missing-override" // False positive detection of missing override warning
#endif

namespace utils
{
	namespace video
	{
		class smart_error_callback :
			public utils::ref_count_base<core::video::video_error_callback>
		{
		private:
			std::function<void(int)> m_error_func;
		public:
			smart_error_callback(const std::function<void(int)>& func) :
				m_error_func(func)
			{
			}

			virtual void on_error(int error_code) override
			{
				if (m_error_func != nullptr)
				{
					m_error_func(error_code);
				}
			}
		};

		class smart_frame_callback :
			public utils::ref_count_base<core::video::frame_callback>
		{
		private:
			std::function<void(core::video::frame_interface*)> m_frame_func;
		public:
			smart_frame_callback(const std::function<void(core::video::frame_interface*)>& func) :
				m_frame_func(func)
			{
			}

			virtual void on_frame(core::video::frame_interface* frame) override
			{
				if (m_frame_func != nullptr)
				{
					m_frame_func(frame);
				}
			}
		};

		template <typename T>
		class video_controller_base : public T
		{
		private:
			using error_callbacks = std::vector<utils::ref_count_ptr<core::video::video_error_callback>>;
			utils::thread_safe_object<error_callbacks> m_error_callbacks;

			utils::ref_count_ptr<utils::dispatcher> m_error_reporting_thread;

		protected:
			virtual void on_error_raising(int error_code, bool& cancel)
			{
				//do nothing
			}

			virtual void raise_error(int error_code)
			{
				bool cancel = false;
				on_error_raising(error_code, cancel);
				if (cancel == true)
					return;

				m_error_callbacks.use([this, error_code](error_callbacks& callbacks)
				{
					if (m_error_reporting_thread == nullptr)
						m_error_reporting_thread = utils::make_ref_count_ptr<utils::dispatcher>();

					for (auto cb : callbacks)
					{
						utils::ref_count_ptr<core::video::video_error_callback> local_callback = cb;
						m_error_reporting_thread->begin_invoke([local_callback, error_code]()
						{
							local_callback->on_error(error_code);
						});
					}
				});
			}

		public:
			virtual ~video_controller_base() = default;

			virtual bool add_error_callback(core::video::video_error_callback* callback) override
			{
				if (callback == nullptr)
					return false;

				return m_error_callbacks.use<bool>([callback](error_callbacks& callbacks)
				{
					auto it = std::find_if(callbacks.begin(),
						callbacks.end(),
						[callback](utils::ref_count_ptr<core::video::video_error_callback>& c)
					{
						return c == callback;
					});

					if (it != callbacks.end())
						return false;

					callbacks.push_back(callback);
					return true;
				});
			}

			virtual bool remove_error_callback(core::video::video_error_callback* callback) override
			{
				if (callback == nullptr)
					return false;

				return m_error_callbacks.use<bool>([callback](error_callbacks& callbacks)
				{
					auto it = std::find_if(
						callbacks.begin(),
						callbacks.end(),
						[callback](utils::ref_count_ptr<core::video::video_error_callback>& c)
					{
						return c == callback;
					});

					if (it == callbacks.end())
						return false;

					callbacks.erase(it);
					return true;
				});
			}
		};

		template <typename T>
		class video_source_base : public video_controller_base<T>
		{
		private:
			class callback_invoker :
				public utils::ref_count_base<core::ref_count_interface>
			{
			private:
				utils::ref_count_ptr<core::video::frame_callback> m_callback;
				utils::ref_count_ptr<utils::dispatcher> m_incvocation_thread;
				std::atomic<size_t> m_pending_frames_count;				

			public:
				callback_invoker(core::video::frame_callback* callback, size_t max_pending_frames) :
					m_callback(callback),
					m_pending_frames_count(max_pending_frames)

				{
					if (callback == nullptr)
						throw std::invalid_argument("callback");

					if (max_pending_frames == 0)
						throw std::invalid_argument("max_pending_frames");
				}

				void invoke(core::video::frame_interface* frame)
				{
					try
					{
						m_callback->on_frame(frame);
					}
					catch (...)
					{
						// TODO: Log  - exception on user callback
					}
				}

				bool invoke_async(core::video::frame_interface* frame)
				{
					if (m_incvocation_thread == nullptr)
						m_incvocation_thread = utils::make_ref_count_ptr<utils::dispatcher>("Video Source Frames Invoker");

					size_t remaining_calls = --m_pending_frames_count;
					if (remaining_calls == 0)
					{
						++m_pending_frames_count;

						// Max pending invocations reached... dropping frame.
						return false;
					}

					utils::ref_count_ptr<core::video::frame_interface> local_frame = frame;
					m_incvocation_thread->begin_invoke([this, local_frame]()
					{
						utils::scope_guard pending_releaser([this]()
						{
							++m_pending_frames_count;
						});

						this->invoke(local_frame);
					});

					return true;
				}

				const core::video::frame_callback* callback() const
				{
					return m_callback;
				}

				void sync()
				{
					if (m_incvocation_thread == nullptr)
						return;

					m_incvocation_thread->sync();
				}
			};					
						
			using frame_callbacks = std::vector<utils::ref_count_ptr<callback_invoker>>;
			utils::thread_safe_object<frame_callbacks> m_frame_callbacks;
			size_t m_max_pending_frames;

		protected:
			static constexpr size_t DEFAULT_MAX_PENDING_FRAMES = 10;

			uint32_t callback_count()
			{
				return m_frame_callbacks.template use<uint32_t>([](frame_callbacks& callbacks)
				{
					return static_cast<uint32_t>(callbacks.size());
				});
			}

			virtual void on_frame_raising(core::video::frame_interface* frame, bool& cancel)
			{
				//do nothing
			}

			virtual void raise_frame(core::video::frame_interface* frame, bool async = true)
			{
				bool cancel = false;
				on_frame_raising(frame, cancel);
				if (cancel == false)
				{
					m_frame_callbacks.use([&](frame_callbacks& callbacks)
					{
						if (async == true)
						{
							for (auto cb : callbacks)
							{
								if (cb->invoke_async(frame) == false)
								{
									// TODO: virtually report to derived class (don't forget to supply the cb)
								}
							}
						}
						else
						{
							for (auto cb : callbacks)
							{
								cb->invoke(frame);
							}
						}
					});
				}
			}

			virtual void sync_frame_callbacks()
			{
				m_frame_callbacks.use([](frame_callbacks& callbacks)
				{
					for (auto cb : callbacks)
					{
						cb->sync();
					}
				});
			}

		public:
			video_source_base(size_t max_pending_frames = DEFAULT_MAX_PENDING_FRAMES) :
				m_max_pending_frames(max_pending_frames)
			{
				if (max_pending_frames == 0)
					throw std::invalid_argument("max_pending_frames");
			}

			virtual ~video_source_base() = default;

			virtual void sync()
			{
				sync_frame_callbacks();
			}

			virtual bool add_frame_callback(core::video::frame_callback* callback) override
			{
				if (callback == nullptr)
					return false;

				return m_frame_callbacks.template use<bool>([&](frame_callbacks& callbacks)
				{
					auto it = std::find_if(callbacks.begin(),
						callbacks.end(),
						[callback](utils::ref_count_ptr<callback_invoker>& c)
					{
						return c->callback() == callback;
					});

					if (it != callbacks.end())
						return false;

					callbacks.push_back(utils::make_ref_count_ptr<callback_invoker>(callback, m_max_pending_frames));
					return true;
				});
			}

			virtual bool remove_frame_callback(core::video::frame_callback* callback) override
			{
				if (callback == nullptr)
					return false;

				utils::ref_count_ptr<callback_invoker> removed_cb;

				m_frame_callbacks.use([callback, &removed_cb](frame_callbacks& callbacks)
				{
					auto it = std::find_if(
						callbacks.begin(),
						callbacks.end(),
						[callback](utils::ref_count_ptr<callback_invoker>& c)
					{
						return c->callback() == callback;
					});

					if (it == callbacks.end())
						return;

					removed_cb = *it;
					callbacks.erase(it);
				});

				return (removed_cb != nullptr); //Implicitly synchronizing the pending callbacks
			}
		};

		template <typename T>
		class frame_base : public utils::ref_count_base<T>
		{
		private:
			core::imaging::image_params m_image_params;
			core::video::display_params m_display_params;
			core::video::video_params m_video_params;
			utils::ref_count_ptr<core::buffer_interface> m_buffer;

		public:
			frame_base(
				const core::imaging::image_params& image_params,
				const core::video::display_params& display_params,
				const core::video::video_params& video_params,
				core::buffer_interface* buffer) :
				m_image_params(image_params),
				m_display_params(display_params),
				m_video_params(video_params),
				m_buffer(buffer)
			{
				if (m_buffer == nullptr)
					throw std::invalid_argument("buffer");
			}

			frame_base(const frame_base& other) :
				m_image_params(other.m_image_params),
				m_display_params(other.m_display_params),
				m_video_params(other.m_video_params),
				m_buffer(other.m_buffer)
			{
			}

			virtual ~frame_base() = default;

			virtual bool query_image_params(core::imaging::image_params& image_params) const override
			{
				image_params = m_image_params;
				return true;
			}

			virtual bool query_buffer(core::buffer_interface** buffer) const override
			{
				if (buffer == nullptr)
					return false;

				if (m_buffer == nullptr)
					return false;

				*buffer = m_buffer;
				(*buffer)->add_ref();
				return true;
			}

			virtual bool query_display_params(core::video::display_params& display_params) const override
			{
				display_params = m_display_params;
				return true;
			}

			virtual bool query_video_params(core::video::video_params& video_params) const override
			{
				video_params = m_video_params;
				return true;
			}
		};

		class frame_binary_serializer :
			public core::serializable_interface
		{
		private:
			class frame : public frame_base<core::video::frame_interface>
			{
			public:
				frame(
					const core::imaging::image_params& image_params,
					const core::video::display_params& display_params,
					const core::video::video_params& video_params,
					core::buffer_interface* buffer) :
					frame_base(image_params, display_params, video_params, buffer)
				{
				}

				frame(const frame& other) :
					frame_base(other)
				{
				}

			};

			utils::ref_count_ptr<core::video::frame_interface> m_frame;
			utils::ref_count_ptr<core::buffer_allocator> m_buffer_allocator;
			uint32_t m_data_alignment;

		public:
			frame_binary_serializer(core::video::frame_interface* frame, uint32_t data_alignment = 0) :
				m_frame(frame),
				m_data_alignment(data_alignment)
			{
			}

			frame_binary_serializer(core::buffer_allocator* buffer_allocator, uint32_t data_alignment = 0) :
				m_buffer_allocator(buffer_allocator),
				m_data_alignment(data_alignment)
			{
			}

			virtual ~frame_binary_serializer() = default;

			bool query_frame(core::video::frame_interface** frame) const
			{
				if (frame == nullptr)
					return false;

				*frame = m_frame;
				(*frame)->add_ref();
				return true;
			}

			virtual uint64_t data_size() const override
			{
				if (m_frame == nullptr)
					return 0;

				utils::ref_count_ptr<core::buffer_interface> buffer;
				if (m_frame->query_buffer(&buffer) == false)
					return 0;

				core::imaging::image_params image_params;
				if (m_frame->query_image_params(image_params) == false)
					return 0;

				core::video::display_params display_params;
				if (m_frame->query_display_params(display_params) == false)
					return 0;

				core::video::video_params video_params;
				if (m_frame->query_video_params(video_params) == false)
					return 0;

				return (sizeof(core::imaging::image_params) + sizeof(core::video::display_params) + sizeof(core::video::video_params) + sizeof(uint32_t) + m_data_alignment + image_params.size);
			}

			virtual bool serialize(core::stream_interface& stream) const override
			{
				if (m_frame == nullptr)
					return false;

				utils::ref_count_ptr<core::buffer_interface> buffer;
				if (m_frame->query_buffer(&buffer) == false)
					return false;

				core::imaging::image_params image_params;
				if (m_frame->query_image_params(image_params) == false || image_params.size == 0)
					return false;

				core::video::display_params display_params;
				if (m_frame->query_display_params(display_params) == false)
					return false;

				core::video::video_params video_params;
				if (m_frame->query_video_params(video_params) == false)
					return false;

				if (stream.write_object<core::imaging::image_params>(image_params) != core::stream_status::status_no_error)
					return false;

				if (stream.write_object<core::video::display_params>(display_params) != core::stream_status::status_no_error)
					return false;

				if (stream.write_object<core::video::video_params>(video_params) != core::stream_status::status_no_error)
					return false;

				if (stream.write_object<uint32_t>(0) != core::stream_status::status_no_error)
					return false;

				size_t bytes_written = 0;
				if (stream.write_bytes(buffer->data(), image_params.size, bytes_written) != core::stream_status::status_no_error ||
					bytes_written != image_params.size)
					return false;

				return true;
			}

			virtual bool serialize(core::buffer_interface* buffer)
			{
				if (buffer == nullptr)
					return false;

				if (m_frame == nullptr)
					return false;

				utils::ref_count_ptr<core::buffer_interface> data_buffer;
				if (m_frame->query_buffer(&data_buffer) == false)
					return false;

				size_t buffer_size = buffer->size();
				if (buffer_size < sizeof(core::imaging::image_params))
					return false;

				uint8_t* pos = buffer->data();
				core::imaging::image_params* image_params = reinterpret_cast<core::imaging::image_params*>(pos);
				if (m_frame->query_image_params(*image_params) == false || image_params->size == 0)
					return false;

				pos += sizeof(core::imaging::image_params);

				uint32_t data_size = image_params->size;
				if (buffer_size <
					(sizeof(core::imaging::image_params) +
						sizeof(core::video::display_params) +
						sizeof(core::video::video_params) +
						sizeof(uint32_t) +
						m_data_alignment +
						data_size))
					return false;

				core::video::display_params* display_params = reinterpret_cast<core::video::display_params*>(pos);
				if (m_frame->query_display_params(*display_params) == false)
					return false;

				pos += sizeof(core::video::display_params);

				core::video::video_params* video_params = reinterpret_cast<core::video::video_params*>(pos);
				if (m_frame->query_video_params(*video_params) == false)
					return false;

				pos += sizeof(core::video::video_params);

				void* unaligned_ptr = (pos + sizeof(uint32_t));
				void* aligned_ptr = unaligned_ptr;

				if (m_data_alignment > 0)
				{
                    size_t bounds = static_cast<size_t>((buffer->data() + buffer->size()) - static_cast<uint8_t*>(unaligned_ptr));
					if (std::align(m_data_alignment, data_size, aligned_ptr, bounds) == nullptr || aligned_ptr == nullptr)
						return false;
				}

				uint32_t data_offset = static_cast<uint32_t>((static_cast<uint8_t*>(aligned_ptr) - static_cast<uint8_t*>(unaligned_ptr)));
				*(reinterpret_cast<uint32_t*>(pos)) = data_offset;
				pos += (sizeof(uint32_t) + data_offset);

				std::memcpy(pos, data_buffer->data(), data_size);
				return true;
			}

			virtual bool deserialize(core::stream_interface& stream) override
			{
				core::imaging::image_params image_params = {};
				core::video::display_params display_params = {};
				core::video::video_params video_params = {};

				if (stream.read_object<core::imaging::image_params>(image_params) != core::stream_status::status_no_error ||
					image_params.size == 0)
					return false;

				if (stream.read_object<core::video::display_params>(display_params) != core::stream_status::status_no_error)
					return false;

				if (stream.read_object<core::video::video_params>(video_params) != core::stream_status::status_no_error)
					return false;

				uint32_t offset = 0;
				if (stream.read_object<uint32_t>(offset) != core::stream_status::status_no_error)
					return false;

				if (offset > 0)
					stream.set_position(offset, core::stream_interface::relative_position::current);

				utils::ref_count_ptr<core::buffer_interface> buffer;
				if (m_frame != nullptr)
				{
					if (m_frame->query_buffer(&buffer) == false ||
						buffer->ref_count() > 2 ||
						buffer->size() < static_cast<size_t>(image_params.size))
						buffer = nullptr;

					m_frame.release();
				}

				if (buffer == nullptr)
				{
					if (m_buffer_allocator != nullptr)
					{
						if (m_buffer_allocator->allocate(image_params.size, &buffer) == false)
							return false;
					}
					else
					{
						buffer = utils::make_ref_count_ptr<utils::ref_count_buffer>(image_params.size);
					}
				}

				size_t bytes_read = 0;
				if (stream.read_bytes(buffer->data(), image_params.size, bytes_read) != core::stream_status::status_no_error ||
					bytes_read != image_params.size)
					return false;

				m_frame = utils::make_ref_count_ptr<frame>(image_params, display_params, video_params, buffer);
				return true;
			}
		};

		template <typename T>
		class smart_video_controller_base : public utils::ref_count_base<T>
		{
		private:
			utils::ref_count_ptr<core::video::video_error_callback> m_error_callback;
			utils::ref_count_ptr<T> m_controller;

			void unregister_callbacks()
			{
				if (controller() == nullptr)
					return;

				if (m_error_callback != nullptr)
					controller()->remove_error_callback(m_error_callback);

				m_error_callback = nullptr;
			}

			void register_callbacks()
			{
				if (controller() == nullptr)
					return;

				unregister_callbacks();

				m_error_callback = utils::make_ref_count_ptr<utils::video::smart_error_callback>([this](int errorCode)
				{
					on_error(errorCode);
				});

				if (controller()->add_error_callback(m_error_callback) == false)
					throw std::runtime_error("Failed to register error callback to source");
			}

		public:
			utils::signal<smart_video_controller_base<T>, int> on_error;

			smart_video_controller_base(T* controller) :
				m_controller(controller)
			{
				if (controller == nullptr)
					throw std::invalid_argument("controller");

				register_callbacks();
			}

			virtual ~smart_video_controller_base()
			{
				unregister_callbacks();
			}

			virtual core::video::video_state state() override
			{
				return controller()->state();
			}

			void start() override
			{
				controller()->start();
			}

			void stop() override
			{
				controller()->stop();
			}

			void pause() override
			{
				controller()->pause();
			}

			virtual bool add_error_callback(core::video::video_error_callback* callback) override
			{
				return controller()->add_error_callback(callback);
			}

			virtual bool remove_error_callback(core::video::video_error_callback* callback) override
			{
				return controller()->remove_error_callback(callback);
			}

			T* controller() const
			{
				return m_controller;
			}
		};

		class smart_video_controller : public smart_video_controller_base<core::video::video_controller_interface>
		{
		public:
			smart_video_controller(core::video::video_controller_interface* controller) :
				smart_video_controller_base<core::video::video_controller_interface>(controller)
			{
			}

			virtual ~smart_video_controller() = default;
		};

		class smart_video_publisher : public smart_video_controller_base<core::video::video_publisher_interface>
		{
		public:
			smart_video_publisher(core::video::video_publisher_interface* publisher) :
				smart_video_controller_base<core::video::video_publisher_interface>(publisher)
			{
			}

			virtual ~smart_video_publisher() = default;

			core::video::video_publisher_interface* publisher() const
			{
				return static_cast<core::video::video_publisher_interface*>(controller());
			}
		};

		class smart_video_sink : public smart_video_controller_base<core::video::video_sink_interface>
		{
		public:
			smart_video_sink(core::video::video_sink_interface* sink) :
				smart_video_controller_base<core::video::video_sink_interface>(sink)
			{
			}

			virtual ~smart_video_sink() = default;

			virtual bool set_frame(core::video::frame_interface* frame) override
			{
				return sink()->set_frame(frame);
			}

			core::video::video_sink_interface* sink() const
			{
				return static_cast<core::video::video_sink_interface*>(controller());
			}
		};

		class smart_video_source : public smart_video_controller_base<core::video::video_source_interface>
		{
		private:
			utils::ref_count_ptr<core::video::frame_callback> m_frame_callback;

			void unregister_callbacks()
			{
				if (m_frame_callback != nullptr)
					source()->remove_frame_callback(m_frame_callback);

				m_frame_callback = nullptr;
			}

			void register_callbacks()
			{
				unregister_callbacks();

				m_frame_callback = utils::make_ref_count_ptr<utils::video::smart_frame_callback>([this](core::video::frame_interface* frame)
				{
					on_frame(frame);
				});

				if (source()->add_frame_callback(m_frame_callback) == false)
					throw std::runtime_error("Failed to register frame callback to source");
			}

		public:
			utils::signal<smart_video_source, core::video::frame_interface*> on_frame;

			smart_video_source(core::video::video_source_interface* source) :
				smart_video_controller_base<core::video::video_source_interface>(source)
			{
				register_callbacks();
			}

			virtual ~smart_video_source()
			{
				unregister_callbacks();
			}

			virtual bool add_frame_callback(core::video::frame_callback* callback) override
			{
				return source()->add_frame_callback(callback);
			}

			virtual bool remove_frame_callback(core::video::frame_callback* callback) override
			{
				return source()->remove_frame_callback(callback);
			}

			core::video::video_source_interface* source() const
			{
				return static_cast<core::video::video_source_interface*>(controller());
			}

			static bool create(const core::video::video_source_factory_interface* factory, core::video::video_source_interface** source)
			{
				if (source == nullptr)
					return false;

				if (factory == nullptr)
					return false;

				utils::ref_count_ptr<core::video::video_source_interface> wrapped_source;
				if (factory->create(&wrapped_source) == false)
					return false;

				utils::ref_count_ptr<core::video::video_source_interface> instance;
				try
				{
					instance = utils::make_ref_count_ptr<smart_video_source>(wrapped_source);
				}
				catch (...)
				{
					return false;
				}

				if (instance == nullptr)
					return false;

				(*source) = instance;
				(*source)->add_ref();
				return true;
			}
		};
	}
}

#if defined(__clang__)
#	pragma clang diagnostic pop
#endif
