#pragma once
#include "common/gstreamer_base_video_source.hpp"
#include <video/sources/gstreamer_rtsp_source.h>

namespace video
{
	namespace sources
	{		
		class gstreamer_rtsp_source_impl :
			public video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_rtsp_source>>
		{
		private:	
			// Usually we won't use such declarations as it's not RAII.
			// However, on this specific case and for simplicity reasons,
			// since this member is initialized and destructed solely in the working tread,
			// and since the working thread is an RAII managed object,
			// we can set the member as plain pointer
			GstElement* m_rtspsrc_element;
			gulong m_rtspsrc_stream_select_signal_handler;
			uint32_t m_stream_index;
			core::video::video_data_type m_data_type;

			std::string m_uri;
			bool m_live;
			bool m_multicast;			
            bool m_rtp_timestamps;
			std::string m_nic_name;			            
            core::imaging::pixel_format m_output_format;			
            uint32_t m_latency;

			static gboolean	on_select_stream(GstElement * rtspsrc, guint idx, GstCaps * caps, gpointer user_data);
			gboolean is_streams_allowed(guint index);

		protected:
			virtual std::string pipeline_description() override;
			virtual void on_pipeline_constructed(GstElement* pipeline) override;
			virtual void on_pipeline_completed() override;

		public:
            gstreamer_rtsp_source_impl(const char* uri, bool live, bool multicast, bool rtp_timestamps, const char* network_device_name, core::imaging::pixel_format output_format, core::imaging::image_algorithm_interface* algo, uint32_t stream_index, core::video::video_data_type data_type);
			virtual ~gstreamer_rtsp_source_impl();
		};
	}
}

