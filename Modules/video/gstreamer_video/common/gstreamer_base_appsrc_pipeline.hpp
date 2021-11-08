#pragma once
#include "gstreamer_base_video_source.hpp"

#include <utils/ref_count_ptr.hpp>

#include <queue>
#include <condition_variable>

#include <gst/app/gstappsrc.h>
#include <libgstatime/gstatimemeta.h>

namespace video
{
	namespace sources
	{
		template <class T>
		class gstreamer_base_appsrc_pipeline : 
			public video::sources::gstreamer_base_video_source<T>
		{
		private:
			static void appsrc_need_data(GstElement *appsrc, guint unused_size, gpointer user_data)
			{
				gstreamer_base_appsrc_pipeline<T>* impl = static_cast<video::sources::gstreamer_base_appsrc_pipeline<T>*>(user_data);
				impl->push_frames();
			}

			static void stop_feed(GstElement* pipeline, gpointer user_data)
			{
				static_cast<gstreamer_base_appsrc_pipeline<T>*>(user_data)->m_stop_feed = true;
			}

			std::mutex m_configure_mutex;
			std::mutex m_frames_mutex;			
			size_t m_max_appsrc_pending_frames;			
			std::queue<utils::ref_count_ptr<core::video::frame_interface>> m_pending_frames;
			std::atomic<bool> m_initiated;
			bool m_stop_feed;
			bool m_first_frame;
			uint64_t m_pts_offset;
			uint64_t m_dts_offset;
			std::condition_variable m_wait_handle;

			// Usually we won't use such declarations as it's not RAII.
			// However, on this specific case and for simplicity reasons,
			// since this member is initialized and destructed solely in the working tread,
			// and since the working thread is an RAII managed object,
			// we can set the member as plain pointer
			GstElement* m_appsrc;

			bool push_frame(core::video::frame_interface* frame)
			{
				if (frame == nullptr)
					return false;

				core::imaging::image_params image_params;
				if (frame->query_image_params(image_params) == false)
					return false;

				core::video::display_params display_params;
				if (frame->query_display_params(display_params) == false)
					return false;

				core::video::video_params video_params;
				if (frame->query_video_params(video_params) == false)
					return false;

				utils::ref_count_ptr<core::buffer_interface> buffer;
				if (frame->query_buffer(&buffer) == false)
					return false;				

				GstBuffer* gstbuffer = nullptr;

				frame->add_ref();
				gsize size = image_params.size;
				gstbuffer = gst_buffer_new_wrapped_full((GstMemoryFlags)0, (gpointer)(buffer->data()), size, 0, size, frame,
					[](gpointer data)
				{
					core::video::frame_interface* pframe = static_cast<core::video::frame_interface*>(data);
					pframe->release();
				});

				if (m_pts_offset == 0)
					m_pts_offset = display_params.pts;

				if (m_dts_offset == 0)
					m_dts_offset = display_params.dts;

				if (display_params.pts < m_pts_offset)
					display_params.pts = m_pts_offset;

				if (display_params.dts < m_dts_offset)
					display_params.dts = m_dts_offset;

				GST_BUFFER_PTS(gstbuffer) = display_params.pts - m_pts_offset;
				GST_BUFFER_DTS(gstbuffer) = display_params.dts - m_dts_offset;
				GST_BUFFER_DURATION(gstbuffer) = display_params.duration;
				
                /*if (video_params.data_type == core::video::video_data_type::H264)
                    GST_BUFFER_FLAGS(gstbuffer) = static_cast<guint>(display_params.frame_id);*/

				if (display_params.timestamp > 0)
				{
					gst_buffer_add_atime_meta(gstbuffer, static_cast<GstClockTime>(display_params.timestamp));
				}

				GstFlowReturn ret;
				g_signal_emit_by_name(m_appsrc, "push-buffer", gstbuffer, &ret);
				gst_buffer_unref(gstbuffer);
				if (ret != GST_FLOW_OK)
				{
					// TODO: Log error...
					return false;
				}

				return true;
			}

			int push_frames()
			{
				m_stop_feed = false;
				int retval = 0;

				std::unique_lock<std::mutex> locker(m_frames_mutex);

				bool initiated = true;
				m_wait_handle.wait(locker, [&]()
				{
					initiated = m_initiated.load();
					return ((initiated == false) || (m_pending_frames.size() > 0));
				});

				if (initiated == false)
					return 0;			

				utils::ref_count_ptr<core::video::frame_interface> frame = m_pending_frames.front();
				m_pending_frames.pop();

				locker.unlock();

				while (frame != nullptr)
				{
					if (push_frame(frame) == true)
					{
						++retval;
					}

					frame.release();					

					locker.lock();
					utils::scope_guard locker_releaser([&]
					{
						locker.unlock();
					});

					if (m_stop_feed == true)
					{
						if (this->async() == true)
						{
							// Drop all pending frames !!!!
							// TODO: We might want to log this...
							std::queue<utils::ref_count_ptr<core::video::frame_interface>> empty;
							std::swap(m_pending_frames, empty);
						}
						
						continue;
					}

					if (m_pending_frames.size() > 0)
					{
						frame = m_pending_frames.front();
						m_pending_frames.pop();						
					}					
				}				

				return retval;
			}

		protected:
			virtual std::string appsrc_name() = 0;

			virtual bool set_caps(GstElement* appsrc, core::video::frame_interface* frame) = 0;
			
			virtual void on_pipeline_constructed(GstElement* pipeline) override
			{
				video::sources::gstreamer_base_video_source<T>::on_pipeline_constructed(pipeline);

				std::lock_guard<std::mutex> configure_lock(m_configure_mutex);

				GstElement* appsrc = gst_bin_get_by_name(GST_BIN(pipeline), appsrc_name().c_str());
				if (appsrc == nullptr)
					throw std::runtime_error("Failed to find the appsrc element");

				if (m_appsrc == appsrc)
					return;

				std::queue<utils::ref_count_ptr<core::video::frame_interface>> empty;
				std::swap(m_pending_frames, empty);

				m_first_frame = true;
				m_initiated = false;

				if (m_appsrc != nullptr)
					gst_object_unref(m_appsrc);

				m_appsrc = appsrc;

				g_object_set(G_OBJECT(m_appsrc),
					"stream-type", 0, // GST_APP_STREAM_TYPE_STREAM
					"format", GST_FORMAT_TIME,
					"is-live", this->async() == true ? TRUE : FALSE,
					NULL);

				g_signal_connect(m_appsrc, "need-data", G_CALLBACK(video::sources::gstreamer_base_appsrc_pipeline<T>::appsrc_need_data), this);
				g_signal_connect(m_appsrc, "enough-data", G_CALLBACK(video::sources::gstreamer_base_appsrc_pipeline<T>::stop_feed), this);
				//gst_app_src_set_latency(GST_APP_SRC(m_appsrc), 0, 0);

				m_first_frame = true;
				m_initiated = true;
			}

			virtual void on_pipeline_completed() override
			{
				video::sources::gstreamer_base_video_source<T>::on_pipeline_completed();

				m_initiated = false;
				m_wait_handle.notify_one();
			}

			virtual void on_pipeline_destroyed() override
			{
				video::sources::gstreamer_base_video_source<T>::on_pipeline_destroyed();

				if (m_appsrc != nullptr)
					gst_object_unref(m_appsrc);

				m_appsrc = nullptr;
			}

			virtual bool set_frame(core::video::frame_interface* frame)
			{
				if (frame == nullptr)
					return false;

				if (m_initiated == false)
					return false;

				if (m_first_frame == true)
				{
					if (set_caps(m_appsrc, frame) == false)
						return false;

					m_pts_offset = 0;
                    m_dts_offset = 0;

                    m_first_frame = false;
				}

				bool retval = true;
				std::unique_lock<std::mutex> locker(m_frames_mutex);
				if (m_pending_frames.size() > m_max_appsrc_pending_frames)
				{
					retval = false;
				}

				if (retval == true)
					m_pending_frames.push(frame);

				locker.unlock();

				m_wait_handle.notify_one();
				return retval;
			}

		public:
			gstreamer_base_appsrc_pipeline(
				size_t max_appsrc_pending_frames = video::sources::gstreamer_base_video_source<T>::DEFAULT_MAX_PENDING_FRAMES,
				bool async = true, 
				size_t max_pending_frames = video::sources::gstreamer_base_video_source<T>::DEFAULT_MAX_PENDING_FRAMES) :
				video::sources::gstreamer_base_video_source<T>(nullptr, async, max_pending_frames),
				m_max_appsrc_pending_frames(max_appsrc_pending_frames),
				m_initiated(false),
				m_stop_feed(false),
				m_first_frame(true),
				m_pts_offset(0),
				m_dts_offset(0),
				m_appsrc(nullptr)
			{
			}

			virtual ~gstreamer_base_appsrc_pipeline() override
			{
				this->stop();
			}
		};
	}
}