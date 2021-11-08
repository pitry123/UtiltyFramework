/// @file	protocols/fixed_length_protocol.h.
/// @brief	Declares the fixed length protocol class
#pragma once
#include <core/communication.h>

namespace communication
{
	namespace protocols
	{
		/// @class	fixed_length_protocol
		/// @brief	Fixed length protocols split a data stream to messages of a fixed size
		/// @date	15/05/2018
		class DLL_EXPORT fixed_length_protocol : public core::communication::client_channel_interface
		{
		public:
			/// @fn	virtual fixed_length_protocol::~fixed_length_protocol() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~fixed_length_protocol() = default;

			/// @fn	static bool fixed_length_protocol::create(core::communication::client_channel_interface* port, size_t nReceiveTimeout, size_t nLength, core::communication::client_channel_interface** protocol);
			/// @brief	Static factory: Creates a new fixed length protocol instance
			/// @date	15/05/2018
			/// @param [in,out]	port		   	A pointer to the underlying channel to be used by the protocol (e.g. communication::ports::tcp_client_port).
			/// @param 		   	nReceiveTimeout	The receive timeout.
			/// @param 		   	nLength		   	The length.
			/// @param [out]	protocol		An address of a pointer to core::communication::client_channel_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(core::communication::client_channel_interface* port, size_t nReceiveTimeout, size_t nLength, core::communication::client_channel_interface** protocol);
		};
	}
}