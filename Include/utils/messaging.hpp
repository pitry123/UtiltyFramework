#pragma once
#include <core/messaging.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/dispatcher.hpp>

#include <vector>
#include <mutex>

namespace utils
{
	namespace messaging
	{
		class smart_message_callback : public utils::ref_count_base<core::messaging::message_callback>
		{
		private:
			std::function<void(core::messaging::message_interface*)> m_func;

		public:
			smart_message_callback(const std::function<void(core::messaging::message_interface*)>& func) :
				m_func(func)
			{
			}

			virtual bool supported_message(const core::guid& id) override
			{
				return true;
			}

			virtual void on_message(core::messaging::message_interface* message) override
			{
				if (m_func != nullptr)
					m_func(message);
			}
		};

		template <typename T>
		class structed_message : public utils::ref_count_base<core::messaging::message_interface>
		{
		private:
			T m_val;

		public:
			structed_message()
			{
			}

			structed_message(const T& val) :
				m_val(val)
			{
			}

			structed_message(const structed_message& other) : m_val(other.m_val)
			{
			}

            virtual uint64_t data_size() const override
			{
				return sizeof(T);
			}

            virtual bool serialize(core::stream_interface& stream) const override
			{
				if (stream.write_object<T>(m_val) != core::stream_status::status_no_error)
					return false;

				return true;
			}

			virtual bool deserialize(core::stream_interface& stream) override
			{
				if (stream.read_object<T>(m_val) != core::stream_status::status_no_error)
					return false;

				return true;
			}

			const T& Val() const
			{
				return m_val;
			}
		};

		template <typename T>
		class message_dispatcher_base : public T
		{
		private:
			class message_invoker :
				public utils::ref_count_base<core::ref_count_interface>
			{
			private:
				utils::ref_count_ptr<core::messaging::message_callback> m_callback;
				utils::dispatcher m_incvocation_thread;

			public:
				message_invoker(core::messaging::message_callback* callback) :
					m_callback(callback)
				{
				}

				utils::ref_count_ptr<core::messaging::message_callback>& callback()
				{
					return m_callback;
				}

				inline bool supported_message(const core::guid& id)
				{
					return m_callback->supported_message(id);
				}

				bool invoke_async(core::messaging::message_interface* message)
				{
					if (supported_message(message->id()) == true)
					{
						utils::ref_count_ptr<core::messaging::message_interface> local_message = message;

						m_incvocation_thread.begin_invoke([this, local_message]()
						{
							try
							{
								m_callback->on_message(local_message);
							}
							catch (...)
							{
								// TODO: Log  - exception on user callback
							}
						});
					}

					return true;
				}

				void sync()
				{
					m_incvocation_thread.sync();
				}
			};

			std::vector<utils::ref_count_ptr<message_invoker>> m_callbacks;
			std::mutex m_mutex;

		protected:
			virtual void on_message_posting(core::messaging::message_interface* message, bool& cancel)
			{
				//do nothing
			}

			virtual void post_message(core::messaging::message_interface* message)
			{
				bool cancel = false;
				on_message_posting(message, cancel);
				if (cancel == false)
				{
					std::lock_guard<std::mutex> locker(m_mutex);
					for (auto cb : m_callbacks)
					{
						if (cb->invoke_async(message) == false)
						{
							// TODO: virtually report to derived class (don't forget to supply the cb)
						}
					}
				}
			}

			virtual bool has_clinets(const core::guid& id)
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				for (auto callback : m_callbacks)
				{
					if (callback->supported_message(id) == true)
						return true;
				}

				return false;
			}
		public:
			virtual bool add_callback(core::messaging::message_callback* callback) override
			{
				if (callback == nullptr)
					return false;

				std::lock_guard<std::mutex> locker(m_mutex);

				auto it = std::find_if(m_callbacks.begin(),
					m_callbacks.end(),
					[callback](utils::ref_count_ptr<message_invoker>& c)
				{
					return c->callback() == callback;
				});

				if (it == m_callbacks.end())
				{
					m_callbacks.push_back(utils::make_ref_count_ptr<message_invoker>(callback));
					return true;
				}

				return false;
			}

			virtual bool remove_callback(core::messaging::message_callback* callback) override
			{
				if (callback == nullptr)
					return false;

				utils::ref_count_ptr<message_invoker> removed_cb;
				std::unique_lock<std::mutex> locker(m_mutex);

				auto it = std::find_if(
					m_callbacks.begin(),
					m_callbacks.end(),
					[callback](utils::ref_count_ptr<message_invoker>& c)
				{
					return c->callback() == callback;
				});

				if (it != m_callbacks.end())
				{
					removed_cb = *it;
					m_callbacks.erase(it);
				}

				locker.unlock();

				return (removed_cb != nullptr); //Implicitly synchronizing the pending callbacks
			}
		};
	}
}