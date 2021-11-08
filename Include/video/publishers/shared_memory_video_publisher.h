/// @file	publishers/shared_memory_video_publisher.h.
/// @brief	Declares the shared memory video publisher class
#pragma once
#include <core/video.h>

namespace video
{
	namespace publishers
	{
		/// @class	shared_memory_video_publisher
		/// @brief	A shared memory video publisher uses shared memory to share video frames. Allows us to share video between processes on the same machine.
		/// 		Video from shared memory publishers can be captured by video::sources::shared_memory_video_source
		/// @date	15/05/2018
		class DLL_EXPORT shared_memory_video_publisher :
			public core::video::video_publisher_interface
		{
		public:
			/// @fn	virtual shared_memory_video_publisher::~shared_memory_video_publisher() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~shared_memory_video_publisher() = default;

			static bool create(
				const char* video_name,
				core::video::video_source_interface* source,
				uint32_t buffer_size,
				uint32_t buffer_pool_size,
				core::video::video_publisher_interface** publisher);

			static bool create(
				const char* video_name,
				core::video::video_source_interface* source,
				core::video::video_publisher_interface** publisher);

			/// @fn	static bool shared_memory_video_publisher::create(const char* video_name, const core::video::video_source_factory_interface* source_factory, uint32_t buffer_size, uint32_t buffer_pool_size, core::video::video_publisher_interface** publisher);
			/// @brief	Static factory: Creates a new shared memory video publisher instance
			/// @date	15/05/2018
			/// @param 		   	video_name			The name of the video (Also the name of the shared memory segment)
			/// @param 		   	source_factory  	The factory which creates the video source to be published.
			/// @param 		   	buffer_size			The memory size to be allocated for each frame.
			/// 									Note that it's required to set this number to at least 128 bytes bigger than the actual frame data size.
			/// @param 		   	buffer_pool_size	The number of frame buffers to allocate as the publishing pool.
			/// @param [out]	publisher			An address of a pointer to core::video::video_publisher_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(
				const char* video_name,
				const core::video::video_source_factory_interface* source_factory,
				uint32_t buffer_size,
				uint32_t buffer_pool_size,
				core::video::video_publisher_interface** publisher);

			/// @fn	static bool shared_memory_video_publisher::create(const char* video_name, const core::video::video_source_factory_interface* source_factory, core::video::video_publisher_interface** publisher);
			/// @brief	Static factory: Creates a new shared memory video publisher instance.
			/// 		In this factory, the frame buffer size is defaulted to 10 MBs and the pool size is 20.
			/// 		This is sufficient for Full-HD video in every pixel-format.
			/// 		For greater resolutions, please use the other factory.
			/// 		
			/// @date	15/05/2018
			/// @param 		   	video_name	  	The name of the video (Also the name of the shared memory segment)
			/// @param 		   	source_factory	The factory which creates the video source to be published.
			/// @param [in,out]	publisher	  	An address of a pointer to core::video::video_publisher_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(
				const char* video_name,
				const core::video::video_source_factory_interface* source_factory,
				core::video::video_publisher_interface** publisher);
		};
	}
}