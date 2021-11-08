/// @file	messages/frame_message.h.
/// @brief	Declares the frame message class
#pragma once
#include <core/video.h>
#include <core/messaging.h>

namespace video
{
	namespace messages
	{
		/// @class	frame_message
		/// @brief	A message containing a video frame which can be posted and dispatched by an implemtation of core::messaging::message_dispatcher
		/// @date	15/05/2018
		class DLL_EXPORT frame_message :
			public core::messaging::message_interface
		{
		public:
			/// @fn	static const core::guid& frame_message::ID()
			/// @brief	Gets the unique identifier of video frames messages
			/// 		{0C8C0F1F-F01F-4418-AF8C-2D405CBA6C98} 		
			/// @date	15/05/2018
			/// @return	A reference to a const core::guid.
			static const core::guid& ID()
			{
				static const core::guid ID = { 0xc8c0f1f, 0xf01f, 0x4418,{ 0xaf, 0x8c, 0x2d, 0x40, 0x5c, 0xba, 0x6c, 0x98 } };
				return ID;
			}

			/// @fn	virtual frame_message::~frame_message() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~frame_message() = default;

			/// @fn	virtual bool frame_message::query_frame(core::video::frame_interface** frame) = 0;
			/// @brief	Queries the video frame from the message
			/// @date	15/05/2018
			/// @param [out]	frame	An address of a pointer to core::video::frame_interface
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_frame(core::video::frame_interface** frame) = 0;

			/// @fn	static bool frame_message::create(core::buffer_allocator* buffer_allocator, core::messaging::message_interface** msg);
			/// @brief	Static factory: Creates a new video frame.
			/// 		A (shared) buffer allocator can be optionally set to allow us to avoid dynamic allocations when querying the internal video frame
			/// @date	15/05/2018
			/// @param [in]		buffer_allocator	If non-null, the buffer allocator.
			/// @param [out]	msg					An address of a pointer to core::messaging::message_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(core::buffer_allocator* buffer_allocator, core::messaging::message_interface** msg);

			/// @fn	static bool frame_message::create(core::video::frame_interface* frame, core::buffer_allocator* buffer_allocator, core::messaging::message_interface** msg);
			/// @brief	Static factory: Creates a new video frame message with an input video frame to be posted.
			/// 		A (shared) buffer allocator can be optionally set to allow us to avoid dynamic allocations when querying the internal video frame
			/// @date	15/05/2018
			/// @param [in]		frame				The input video frame.
			/// @param [in]		buffer_allocator	If non-null, the buffer allocator.
			/// @param [out]	msg					An address of a pointer to core::messaging::message_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(core::video::frame_interface* frame, core::buffer_allocator* buffer_allocator, core::messaging::message_interface** msg);
		};
	}
}