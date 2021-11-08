#pragma once
#include <video/sources/shared_memory_video_source.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/video.hpp>

#include "shared_memory_streaming.hpp"
#include <thread>
#include <atomic>

namespace video
{
	namespace sources
	{
		static constexpr uint32_t VIDEO_CHANNEL_ID = 0;

		class shared_memory_video_subscriber : 
			public utils::ref_count_base<utils::video::video_source_base<video::sources::shared_memory_video_source>>
		{
		private:
			class shared_memory_zero_copy_frame : public utils::ref_count_base<core::video::frame_interface>
			{
			private:
				utils::ref_count_ptr<core::buffer_interface> m_buffer;

				core::imaging::image_params& direct_image_params() const
				{
					return *(static_cast<core::imaging::image_params*>(static_cast<void*>(m_buffer->data())));
				}

			public:
				shared_memory_zero_copy_frame(core::buffer_interface* buffer) :
					m_buffer(buffer)
				{
				}

				virtual bool query_image_params(core::imaging::image_params& image_params) const override
				{
					if (m_buffer == nullptr)
						return false;

					std::memcpy(&image_params, m_buffer->data(), sizeof(core::imaging::image_params));
					return true;
				}

				virtual bool query_display_params(core::video::display_params& display_params) const override
				{
					if (m_buffer == nullptr)
						return false;

					std::memcpy(&display_params, m_buffer->data() + sizeof(core::imaging::image_params), sizeof(core::video::display_params));
					return true;
				}

                virtual bool query_video_params(core::video::video_params& video_params) const override
                {
                    if (m_buffer == nullptr)
                        return false;

                    std::memcpy(&video_params, m_buffer->data() + sizeof(core::imaging::image_params) + sizeof(core::video::display_params), sizeof(core::video::video_params));
                    return true;
                }

				virtual bool query_buffer(core::buffer_interface** image_buffer) const override
				{
					if (image_buffer == nullptr)
						return false;

					if (m_buffer == nullptr)
						return false;					

					if (direct_image_params().size == 0)
						return false;

					size_t alignment_offset = *(reinterpret_cast<uint32_t*>(
						m_buffer->data() + 
						sizeof(core::imaging::image_params) + 
						sizeof(core::video::display_params) + 
						sizeof(core::video::video_params)));

					size_t data_offset = sizeof(core::imaging::image_params) +
						sizeof(core::video::display_params) +
						sizeof(core::video::video_params) +
						sizeof(uint32_t) + alignment_offset;
					
					utils::ref_count_ptr<core::buffer_interface> relative_buffer = utils::make_ref_count_ptr<utils::ref_count_relative_buffer>(
						m_buffer,
						data_offset,
						direct_image_params().size);

					*image_buffer = relative_buffer;
					(*image_buffer)->add_ref();
					return true;
				}
			};

			std::mutex m_mutex;
			std::atomic<core::video::video_state> m_state;
			utils::ref_count_ptr<shared_memory::shm_session_player> m_player;

		public:
			shared_memory_video_subscriber(const char* video_name) :
				m_state(core::video::video_state::STOPPED)
			{
				utils::ref_count_ptr<shared_memory::shm_session> session =
					utils::make_ref_count_ptr<shared_memory::shm_session>(video_name);

				m_player = utils::make_ref_count_ptr<shared_memory::shm_session_player>(session, VIDEO_CHANNEL_ID);

				m_player->OnBuffer += [this](uint32_t stream_index, core::buffer_interface* buffer)
				{
					if (stream_index != VIDEO_CHANNEL_ID)
						return;

					if (state() != core::video::video_state::PLAYING)
						return;

					utils::ref_count_ptr<core::video::frame_interface> frame = utils::make_ref_count_ptr<shared_memory_zero_copy_frame>(buffer);
					raise_frame(frame);
				};
			}

			virtual ~shared_memory_video_subscriber()
			{
				stop();
			}

			virtual core::video::video_state state() override
			{
				return m_state;
			}

			virtual void start() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);

				if (m_state != core::video::video_state::PLAYING)
				{
					if (m_state == core::video::video_state::STOPPED)
					{
						m_player->start();
					}

					m_state = core::video::video_state::PLAYING;
				}
			}

			virtual void stop() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);

				if (m_state != core::video::video_state::STOPPED)
				{
					m_state = core::video::video_state::STOPPED;
					m_player->stop();
				}
			}

			virtual void pause() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				if (m_state != core::video::video_state::PAUSED)
				{
					m_state = core::video::video_state::PAUSED;
				}
			}			
		};
	}
}

