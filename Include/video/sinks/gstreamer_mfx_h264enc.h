/// @file	sinks/gstreamer_mfx_h264enc.h.
/// @brief	Declares the GStreamer Intel Media SDK h264 encoder class
#pragma once
#include <core/video.h>

namespace video
{
	namespace sinks
	{
		/// @class	gstreamer_mfx_h264enc
		/// @brief	A 'GStreamer' based H.264 video encoder.
		/// 		This sink assumes that the GStreamer Intel media SDK plug-in is installed (https://github.com/ishmael1985/gstreamer-media-SDK).
		/// 		It also assumes that we're running on a machine with Intel graphics available and driver is installed
		/// @date	15/05/2018
		class DLL_EXPORT gstreamer_mfx_h264enc : public core::video::video_sink_interface
		{
		public:
			/// @fn	virtual gstreamer_mfx_h264enc::~gstreamer_mfx_h264enc() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~gstreamer_mfx_h264enc() = default;
			
			/// @fn	static bool gstreamer_mfx_h264enc::create(const char* file_path, uint16_t bitrate, core::video::video_sink_interface** sink);
			/// @brief	Creates a new gstreamer_mfx_h264enc instance
			/// @date	15/05/2018
			/// @param 		   	file_path	Full pathname of the output file.
			/// @param 		   	bitrate		The desigred bitrate in kbps.
			/// @param [out]	sink	 	An address of a pointer to core::video::video_sink_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* file_path, uint16_t bitrate, core::video::video_sink_interface** sink);

			/// @fn	static bool gstreamer_mfx_h264enc::create(const char* file_path, core::video::video_sink_interface** sink);
			/// @brief	Creates a new gstreamer_mfx_h264enc instance
			/// @date	15/05/2018
			/// @param 		   	file_path	Full pathname of the output file.
			/// @param [out]	sink	 	An address of a pointer to core::video::video_sink_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* file_path, core::video::video_sink_interface** sink);
		};
	}
}