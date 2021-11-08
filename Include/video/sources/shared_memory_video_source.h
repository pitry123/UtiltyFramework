/// @file	sources/shared_memory_video_source.h.
/// @brief	Declares the shared memory video source class
#pragma once
#include <core/video.h>

namespace video
{
	namespace sources
	{
		/// @class	shared_memory_video_source
		/// @brief	The shared memory video source captures and uploads frames
		/// 		sent by video::publishers::shared_memory_video_publisher
		/// @date	15/05/2018
		class DLL_EXPORT shared_memory_video_source :
			public core::video::video_source_interface
		{
		public:
			/// @fn	virtual shared_memory_video_source::~shared_memory_video_source() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~shared_memory_video_source() = default;

			/// @fn	static bool shared_memory_video_source::create(const char* video_name, core::video::video_source_interface** source);
			/// @brief	Static factory: Creates a new shared memory video source instance.
			/// @date	15/05/2018
			/// @param 		   	video_name	The shared memory video name.
			/// @param [in,out]	source	  	An address of a pointer to core::video::video_source_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* video_name, core::video::video_source_interface** source);
		};
	}
}