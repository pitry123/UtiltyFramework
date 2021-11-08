/// @file	sources/gstreamer_test_source.h.
/// @brief	Declares the GSteamer test source class
#pragma once
#include <video/sources/gstreamer_video_source.h>

namespace video
{
	namespace sources
	{
		/// @class	gstreamer_test_source
		/// @brief	A GStreamer based video source for video simulation/generation.
		/// 		This source can generate video of virtually any dimension, framerate and pixel-format.
		/// @date	15/05/2018
		class DLL_EXPORT gstreamer_test_source : public video::sources::gstreamer_video_source
		{
		public:
			/// @fn	virtual gstreamer_test_source::~gstreamer_test_source() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~gstreamer_test_source() = default;

			/// @fn	static bool gstreamer_test_source::create(uint32_t width, uint32_t height, core::imaging::pixel_format foramt, core::video::framerate framerate, core::video::video_source_interface** source);
			/// @brief	Static factory: Creates a new test video source instance
			/// @date	15/05/2018
			/// @param 		   	width	 	The desired video width in pixels.
			/// @param 		   	height   	The desired video height in pixels.
			/// @param 		   	foramt   	The desired pixel-foramt.
			/// @param 		   	framerate	The desired framerate.
			/// @param [out]	source   	An address of a pointer to core::video::video_source_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(uint32_t width, uint32_t height, core::imaging::pixel_format foramt, const core::video::framerate& framerate, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source);
			static bool create(uint32_t width, uint32_t height, core::imaging::pixel_format foramt, const core::video::framerate& framerate, core::video::video_source_interface** source);
		};
	}
}