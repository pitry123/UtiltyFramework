/// @file	sources/gstreamer_raw_data_source.h.
/// @brief	Declares the GStreamer raw data source class
#pragma once
#include <video/sources/gstreamer_video_source.h>

namespace video
{
	namespace sources
	{
		/// @class	gstreamer_raw_data_source
		/// @brief	A GStreamer based video source for playing raw video files.
		/// 		Raw video files contains sequence of uncompressed image buffers (e.g. YUY2 images with known width and height) 		
		/// @date	15/05/2018
		class DLL_EXPORT gstreamer_raw_data_source : public video::sources::gstreamer_video_source
		{
		public:
			/// @fn	virtual gstreamer_raw_data_source::~gstreamer_raw_data_source() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~gstreamer_raw_data_source() = default;

			/// @fn	static bool gstreamer_raw_data_source::create(const char* file_path, uint32_t width, uint32_t height, core::imaging::pixel_format foramt, core::video::framerate framerate, core::video::video_source_interface** source);
			/// @brief	Static factory: Creates a new raw data source instance
			/// @date	15/05/2018
			/// @param 		   	file_path	Full pathname of the file.
			/// @param 		   	width	 	The input video width in pixels.
			/// @param 		   	height   	The input video height in pixels.
			/// @param 		   	foramt   	The pixel-foramt.
			/// @param 		   	framerate	The desired video framerate to play.
			/// @param [out]	source   	An address of a pointer to core::video::video_source_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* file_path, uint32_t width, uint32_t height, core::imaging::pixel_format format, const core::video::framerate& framerate, core::video::video_source_interface** source);
			static bool create(const char* file_path, uint32_t width, uint32_t height, core::imaging::pixel_format format, const core::video::framerate& framerate, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source);
		};
	}
}