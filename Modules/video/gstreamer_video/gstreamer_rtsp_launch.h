#pragma once
#include "common/gstreamer_base_rtsp_server.hpp"
#include <video/publishers/gstreamer_rtsp_launch.h>
#include <utils/ref_count_base.hpp>

namespace video
{
	namespace publishers
	{		
		using base_class = video::publishers::gstreamer_base_rtsp_server<utils::ref_count_base<video::publishers::gstreamer_rtsp_launch>>;

		class gstreamer_rtsp_launch_impl : public base_class
		{
		private:
			std::string m_launch_pipeline;

		protected:
			std::string pipeline_description() override;

		public:
			gstreamer_rtsp_launch_impl(
				const char* stream_name, 
				uint16_t port, const char* launch_pipeline,
				const char* minmum_multicast_address,
				const char* maximum_multicast_address,
				uint16_t minmum_multicast_port,
				uint16_t maximum_multicast_port);
		};
	}
}

