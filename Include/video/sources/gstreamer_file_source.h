/// @file	sources/gstreamer_file_source.h.
/// @brief	Declares the GStreamer file source class
#pragma once
#include <video/sources/gstreamer_video_source.h>

namespace video
{
	namespace sources
	{
		/// @class	gstreamer_file_source
		/// @brief	A GStreamer based video source for playing video files.
		/// 		Supports all common formats and containers (e.g. AVI, MP4, MKV...)
		/// @date	15/05/2018
		class DLL_EXPORT gstreamer_file_source : public video::sources::gstreamer_video_source
		{
		public:
			/// @fn	virtual gstreamer_file_source::~gstreamer_file_source() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~gstreamer_file_source() = default;

			/// @fn	static bool gstreamer_file_source::create(const char* file_path, core::video::video_source_interface** source);
			/// @brief	Creates a new file source instance
			/// @date	15/05/2018
			/// @param 		   	file_path	Full pathname of the file.
			/// @param [out]	source   	An address of a pointer to core::video::video_source_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* file_path, core::video::video_source_interface** source);
			static bool create(const char* file_path, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source);
		};
	}
}