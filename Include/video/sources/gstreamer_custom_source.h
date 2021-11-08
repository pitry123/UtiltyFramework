/// @file	sources/gstreamer_custom_source.h.
/// @brief	Declares the GStreamer custom source class
#pragma once
#include <video/sources/gstreamer_video_source.h>

namespace video
{
	namespace sources
	{
		class DLL_EXPORT gstreamer_custom_source : public video::sources::gstreamer_video_source
		{
		public:
			virtual ~gstreamer_custom_source() = default;
			
			static bool create(const char* pipeline, const char* appsink_name, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source);
			static bool create(const char* pipeline, const char* appsink_name, core::video::video_source_interface** source);			
		};
	}
}
