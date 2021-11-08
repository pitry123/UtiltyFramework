#pragma once
#include "common/gstreamer_base_video_controller.hpp"
#include <video/sinks/gstreamer_mfx_h264enc.h>

#include <utils/ref_count_base.hpp>
#include <utils/dispatcher.hpp>
#include <utils/video.hpp>

#include <gst/app/gstappsrc.h>

#include <vector>
#include <string>

namespace video
{
	namespace sinks
	{		
		class gstreamer_h264_encoder_impl :
			public utils::ref_count_base<video::common::gstreamer_base_video_controller<utils::video::video_controller_base<core::video::video_sink_interface>>>
		{
		private:
			static void appsrc_need_data(GstElement *appsrc, guint unused_size, gpointer user_data);

			std::string m_file_path;
            uint16_t m_bitrate;

			std::mutex m_mutex;
			std::vector <utils::ref_count_ptr<core::video::frame_interface>> m_pending_frames;
			std::vector<utils::ref_count_ptr<core::video::frame_interface>> m_working_frames;		
			std::atomic<bool> m_running;
			bool m_first_frame;
			std::condition_variable m_wait_handle;			

			// Usually we won't use such declarations as it's not RAII.
			// However, on this specific case and for simplicity reasons,
			// since this member is initialized and destructed solely in the working tread,
			// and since the working thread is an RAII managed object,
			// we can set the member as plain pointer
			GstElement* m_appsrc;
			int m_index;			

			bool PushFrame(core::video::frame_interface* frame);
			int PushFrames();

		protected:	
			virtual std::string pipeline_description() override;

			virtual void on_pipeline_constructed(GstElement* pipeline) override;
			virtual void on_pipeline_completed() override;
			virtual void on_pipeline_destroyed() override;

		public:
            gstreamer_h264_encoder_impl(const char* file_path, uint16_t bitrate);
			virtual ~gstreamer_h264_encoder_impl();

			virtual bool set_frame(core::video::frame_interface* frame) override;
		};
	}
}

