/// @file	sources/gstreamer_video_source.h.
/// @brief	Declares the GStreamer video source class
#pragma once
#include <core/video.h>

namespace video
{
	namespace sources
	{
		/// @enum	gstreamer_error_codes
		/// @brief	Values that represent the GStreamer pipeline's
		/// 		specific error codes.
		/// 		To be used only with GStreamer based video sources.
		///-------------------------------------------------------------------
		enum gstreamer_error_codes
		{
			/// @brief	The GStreamer pipeline was interrupted with 'End of Stream'
			end_of_stream = 0xFFFF,
			/// @brief	The GStreamer pipeline was interrupted with an error
			stream_error = end_of_stream + 1,
			/// @brief	The GStreamer pipeline was interrupted with a warning
			stream_warning = stream_error + 1
		};

		/// @class	gstreamer_video_source
		/// @brief	A base class for all GStreamer based video sources
		/// @date	15/05/2018
		class DLL_EXPORT gstreamer_video_source : public core::video::video_source_interface
		{
		public:
			/// @fn	virtual gstreamer_video_source::~gstreamer_video_source() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~gstreamer_video_source() = default;
		};
	}
}