#pragma once
#include <core/video.h>

namespace video
{
	namespace publishers
	{
		class DLL_EXPORT gstreamer_rtsp_launch : public core::video::video_publisher_interface
		{
		public:
			virtual ~gstreamer_rtsp_launch() = default;

			static bool create(
				const char* stream_name,
				uint16_t port,
				const char* launch_pipeline,
				const char* minmum_multicast_address,
				const char* maximum_multicast_address,
				uint16_t minmum_multicast_port,
				uint16_t maximum_multicast_port,
				core::video::video_publisher_interface** publisher);

			static bool create(
				const char* stream_name, 
				uint16_t port, 
				const char* launch_pipeline, 
				core::video::video_publisher_interface** publisher);
		};
	}
}