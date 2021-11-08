#pragma once
#include "common/gstreamer_base_rtsp_server.hpp"
#include <video/publishers/gstreamer_rtsp_server.h>
#include <utils/ref_count_base.hpp>

#include <utils/ref_count_ptr.hpp>
#include <utils/video.hpp>

#include <vector>
#include <string>

// gstreamer includes
#ifdef __GNUC__
// Avoid tons of warnings with root code
#pragma GCC system_header
#endif

#include <gst/app/gstappsrc.h>

namespace video
{
	namespace publishers
	{		
		using base_class = video::publishers::gstreamer_base_rtsp_server<utils::ref_count_base<video::publishers::gstreamer_rtsp_server>>;

		class gstreamer_rtsp_server_impl : public base_class
		{
		private:			
			static void appsrc_need_data(GstElement *appsrc, guint unused_size, gpointer user_data);
			static void stop_feed(GstElement* pipeline, gpointer user_data);
			
			utils::ref_count_ptr<core::video::video_source_interface> m_source;
			utils::ref_count_ptr<utils::video::smart_frame_callback> m_frame_callback;						

			std::mutex m_configure_mutex;
			std::mutex m_frames_mutex;			
			std::vector <utils::ref_count_ptr<core::video::frame_interface>> m_pending_frames;
			std::vector<utils::ref_count_ptr<core::video::frame_interface>> m_working_frames;			
			std::atomic<bool> m_initiated;
			bool m_stop_feed;
			bool m_first_frame;
			uint64_t m_pts_offset;
			std::condition_variable m_wait_handle;

			// Usually we won't use such declarations as it's not RAII.
			// However, on this specific case and for simplicity reasons,
			// since this member is initialized and destructed solely in the working tread,
			// and since the working thread is an RAII managed object,
			// we can set the member as plain pointer
			GstElement* m_appsrc;

			bool push_frame(core::video::frame_interface* frame);
			int push_frames();

		protected:
			virtual std::string pipeline_description();

			virtual void on_started() override;
			virtual void on_stop() override;

			virtual void on_pipeline_constructed(GstElement* pipeline) override;
			virtual void on_pipeline_completed() override;
			virtual void on_pipeline_destroyed() override;

			virtual bool set_frame(core::video::frame_interface* frame);

		public:
			gstreamer_rtsp_server_impl(
				const char* stream_name,
				uint16_t port,
				const char* minmum_multicast_address,
				const char* maximum_multicast_address,
				uint16_t minmum_multicast_port,
				uint16_t maximum_multicast_port,
				core::video::video_source_factory_interface* source_factory);

			virtual ~gstreamer_rtsp_server_impl();
		};
	}
}

