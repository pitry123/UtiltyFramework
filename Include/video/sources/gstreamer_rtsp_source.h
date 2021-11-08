/// @file	sources/gstreamer_rtsp_source.h.
/// @brief	Declares the GStreamer RTSP source class
#pragma once
#include <video/sources/gstreamer_video_source.h>

namespace video
{
	namespace sources
	{
		/// @class	gstreamer_rtsp_source
		/// @brief	A GStreamer based video source for playing RTSP video streams.
		/// 		The stream's Video format is assumed to be H.264
		/// @date	15/05/2018
		class DLL_EXPORT gstreamer_rtsp_source : public video::sources::gstreamer_video_source
		{
		public:
			/// @fn	virtual gstreamer_rtsp_source::~gstreamer_rtsp_source() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~gstreamer_rtsp_source() = default;

			/// @fn	static bool gstreamer_rtsp_source::create(const char* url, core::video::video_source_interface** source);
			/// @brief	Static factory: Creates a new RTSP video source instance.
			/// 		This factory assumes the RTSP server is not using multicast for the RTP stream.
			/// 		This factory is probably not suitable for live sources as it schedule the frames according to the PTS (presentation time) value.
			/// @date	15/05/2018
			/// @param 		   	url   	The stream URL.
			/// @param [out]	source	An address of a pointer to core::video::video_source_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* url, core::video::video_source_interface** source);

			/// @fn	static bool gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, core::video::video_source_interface** source);
			/// @brief	Static factory: Creates a new RTSP source instance.
			/// @date	15/05/2018
			/// @param 		   	url		 		The stream URL.
			/// @param 		   	live	 		True for live stream (upload frames immediately), False for playback (Schedule frames according to presentation time).
			/// @param 		   	multicast		True for multicast RTP streams.			
			/// @param [out]	source   		An address of a pointer to core::video::video_source_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* url, bool live, bool multicast, core::video::video_source_interface** source);

			/// @fn	static bool gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, core::video::video_source_interface** source);
			/// @brief	Static factory: Creates a new RTSP source instance.
			/// @date	15/05/2018
			/// @param 		   	url		 		The stream URL.
			/// @param 		   	live	 		True for live stream (upload frames immediately), False for playback (Schedule frames according to presentation time).
			/// @param 		   	multicast		True for multicast RTP streams.
			/// @param 		   	rtp_timestamps	True for try and getting timestamps from RTP packets (applicable for streams supporting the relevant RTP extension)
			/// @param [out]	source   		An address of a pointer to core::video::video_source_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* url, bool live, bool multicast, bool rtp_timestamps, core::video::video_source_interface** source);

			/// @fn	static bool gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, const char* network_device_name, core::video::video_source_interface** source);
			/// @brief	Static factory: Creates a new RTSP source instance.
			/// @date	15/05/2018
			/// @param 		   	url		 			The stream URL.
			/// @param 		   	live	 			True for live stream (upload frames immediately), False for playback (Schedule frames according to presentation time).
			/// @param 		   	multicast			True for multicast RTP streams.
			/// @param 		   	rtp_timestamps		True for try and getting timestamps from RTP packets (applicable for streams supporting the relevant RTP extension)
			/// @param 		   	network_device_name	The name of the network device to bind for multicast RTP stream.
			/// @param [in,out]	source			   	An address of a pointer to core::video::video_source_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* url, bool live, bool multicast, bool rtp_timestamps, const char* network_device_name, core::video::video_source_interface** source);

			/// @fn	static bool gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, const char* network_device_name, core::imaging::pixel_format output_format, core::video::video_source_interface** source);
			/// @brief	Static factory: Creates a new RTSP source instance.
			/// @date	15/05/2018
			/// @param 		   	url		 			The stream URL.
			/// @param 		   	live	 			True for live stream (upload frames immediately), False for playback (Schedule frames according to presentation time).
			/// @param 		   	multicast			True for multicast RTP streams.
			/// @param 		   	rtp_timestamps		True for try and getting timestamps from RTP packets (applicable for streams supporting the relevant RTP extension)
			/// @param 		   	network_device_name	The name of the network device to bind for multicast RTP stream.
			/// @param 		   	output_format	   	The desired output format.
			/// 									Allows color space conversion to a desired pixel-format.
			/// 									When set to core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT, no conversion is performed.
			/// @param [in,out]	source			   	An address of a pointer to core::video::video_source_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* url, bool live, bool multicast, bool rtp_timestamps, const char* network_device_name, core::imaging::pixel_format output_format, core::video::video_source_interface** source);

			static bool create(const char* url, bool live, bool multicast, bool rtp_timestamps, const char* network_device_name, core::imaging::pixel_format output_format, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source);

			static bool create(const char* url, bool live, bool multicast, bool rtp_timestamps, const char* network_device_name, core::imaging::pixel_format output_format, core::imaging::image_algorithm_interface* algo, uint32_t stream_index, core::video::video_source_interface** source);

			static bool create(const char* url, bool live, bool multicast, bool rtp_timestamps, const char* network_device_name, core::imaging::pixel_format output_format, core::imaging::image_algorithm_interface* algo, uint32_t stream_index, core::video::video_data_type data_type, core::video::video_source_interface** source);
		};
	}
}
