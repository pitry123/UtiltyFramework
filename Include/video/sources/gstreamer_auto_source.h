/// @file	sources/gstreamer_auto_source.h.
/// @brief	Declares the GStreamer automatic source class
#pragma once
#include <video/sources/gstreamer_video_source.h>

namespace video
{
	namespace sources
	{
		/// @class	gstreamer_auto_source
		/// @brief	Uses GStreamer auto-source plug-in to capture the first available video input (If available).
		/// 		Typically, it would probably capture your webcam (if any)
		/// @date	15/05/2018
		class DLL_EXPORT gstreamer_auto_source : public video::sources::gstreamer_video_source
		{
		public:
			/// @fn	virtual gstreamer_auto_source::~gstreamer_auto_source() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~gstreamer_auto_source() = default;

			/// @fn	static bool gstreamer_auto_source::create(core::video::video_source_interface** source);
			/// @brief	Static factory: Creates a new auto video source instance.
			/// 		Note that frame-sync is 'ON' by default when using this factory. You might want to use the other factory and disable it when streaming live sources.
			/// @date	15/05/2018
			/// @param [out]	source	An address of a pointer to core::video::video_source_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(core::video::video_source_interface** source);

			/// @fn	static bool gstreamer_auto_source::create(bool sync, core::video::video_source_interface** source);
			/// @brief	Static factory: Creates a new auto video source instance.
			/// @date	15/05/2018
			/// @param 		   	sync  	True to schedule the frames according to the PTS (presentation time) value. False to upload immediately.
			/// 						In general, it's usually recommended to set this value to 'false' when streaming live sources.
			/// @param [out]	source	An address of a pointer to core::video::video_source_interface.
			/// @return	True if it succeeds, false if it fails.
			static bool create(bool sync, core::video::video_source_interface** source);
			static bool create(bool sync, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source);
			static bool create(bool sync, uint32_t width, uint32_t height, core::video::video_source_interface** source);
			static bool create(bool sync, uint32_t width, uint32_t height, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source);
			static bool create(bool sync, uint32_t width, uint32_t height, core::imaging::pixel_format format, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source);
		};
	}
}