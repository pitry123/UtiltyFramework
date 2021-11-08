#pragma once
#include <video/publishers/shared_memory_video_publisher.h>
#include <streams/memory_stream_interface.h>
#include <core/serializable_interface.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/scope_guard.hpp>
#include <utils/video.hpp>

#include <shared_memory_streaming.hpp>
#include <string>

namespace video
{
	namespace publishers
	{
		static constexpr uint32_t VIDEO_CHANNEL_ID = 0;
		static constexpr uint32_t VIDEO_CHANNEL_POOL_SIZE = 20;
		static constexpr uint32_t VIDEO_CHANNEL_BUFFER_SIZE = 10485760; // 10 MBs

		class shared_memory_video_publisher_impl : 
			public utils::ref_count_base<utils::video::video_controller_base<video::publishers::shared_memory_video_publisher>>
		{
		private:	
			utils::ref_count_ptr<shared_memory::shm_session> m_session;
			utils::ref_count_ptr<core::video::video_source_interface> m_source;
			utils::ref_count_ptr<core::video::frame_callback> m_frame_callback;
			utils::ref_count_ptr<core::video::video_error_callback> m_error_callback;

		protected:
			virtual void on_frame(core::video::frame_interface* frame)
			{
				utils::ref_count_ptr<shared_memory::shm_sharable_buffer_interface> buffer;
				if (m_session->query_buffer_unique(VIDEO_CHANNEL_ID, &buffer) != shared_memory::error_codes::SHM_NO_ERROR)
					return;			

				utils::video::frame_binary_serializer serializer(frame, 0);
				if (serializer.serialize(buffer) == false)
					return;				

				m_session->publish(buffer);
			}		

		public:
			shared_memory_video_publisher_impl(const char* video_name, 
				core::video::video_source_interface* source,
				uint32_t buffer_size,
				uint32_t buffer_pool_size) :
				m_source(source)
			{
				if (source == nullptr)
					throw std::invalid_argument("source");				

				m_frame_callback = utils::make_ref_count_ptr<utils::video::smart_frame_callback>([this](core::video::frame_interface* frame)
				{
					on_frame(frame);
				});

				if (m_source->add_frame_callback(m_frame_callback) == false)
					throw std::runtime_error("Failed to set frame callback to source");

				m_error_callback = utils::make_ref_count_ptr<utils::video::smart_error_callback>([this](int error_code)
				{
					raise_error(error_code);
				});

				if (m_source->add_error_callback(m_error_callback) == false)
					throw std::runtime_error("Failed to set error callback to source");


				shared_memory::shm_pool_params stream_params[] = 
				{ 
					shared_memory::shm_pool_params{ VIDEO_CHANNEL_ID, buffer_pool_size, buffer_size},
				};

                m_session = utils::make_ref_count_ptr<shared_memory::shm_session>(video_name, stream_params, static_cast<uint32_t>(1));
			}

			~shared_memory_video_publisher_impl()
			{
				m_source->stop();
				m_source->remove_error_callback(m_error_callback);
				m_source->remove_frame_callback(m_frame_callback);
			}

			virtual core::video::video_state state() override
			{
				return m_source->state();
			}

			virtual void start() override
			{
				m_source->start();
			}

			virtual void stop() override
			{
				m_source->stop();
			}

			virtual void pause() override
			{
				m_source->pause();
			}
		};
	}
}

