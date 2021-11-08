#pragma once
#include <video/sources/gstreamer_video_source.h>

namespace video
{
	namespace sources
	{
		class DLL_EXPORT gstreamer_h264_player : public video::sources::gstreamer_video_source
		{
		public:
			virtual ~gstreamer_h264_player() = default;
			static bool create(core::video::video_source_interface* h264_source, bool live, core::imaging::pixel_format output_format, bool async, core::video::video_source_interface** source);
			static bool create(core::video::video_source_interface* h264_source, bool live, core::imaging::pixel_format output_format, core::video::video_source_interface** source);
			static bool create(core::video::video_source_interface* h264_source, bool live, core::video::video_source_interface** source);			
		};
	}
}