/// @file	protocols/variable_length_protocol.h.
/// @brief	Declares the variable length protocol class
#pragma once
#include <core/communication.h>
#include <core/types.h>

namespace communication
{
	namespace protocols
	{
		/// @class	variable_length_protocol
		/// @brief	Variable length protocols split a data stream to messages by assuming a message header containing the message length on a specific data offset
		/// @date	15/05/2018
		class DLL_EXPORT variable_length_protocol : public core::communication::client_channel_interface
		{
		public:
			/// @fn	virtual variable_length_protocol::~variable_length_protocol() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~variable_length_protocol() = default;

			/// @fn	static bool variable_length_protocol::create(core::communication::client_channel_interface* port, size_t nSizeOfDataSize, size_t nPlaceOfDataSize, bool bAbsolute_data_size, size_t nMaxDataSize,core::communication::client_channel_interface** protocol);
			/// @brief	Static factory: Creates a new variable length protocol
			/// @date	15/05/2018
			/// @param [in]		port			   		A pointer to the underlying channel to be used by the protocol (e.g. communication::ports::tcp_client_port).
			/// @param 		   	nSizeOfDataSize	   		The size of the the message length data (e.g. 4 bytes for a 32-bit integer value).
			/// @param 		   	nPlaceOfDataSize   		The offset from the beginning of the message to the length data.
			/// @param 		   	nConstatSizeAddition   	Fixed value to add to each message length read
			/// @param 		   	bAbsolute_data_size		True if the length data includes (nPlaceOfDataSize + nSizeOfDataSize), False if not.
			/// @param 		   	nMaxDataSize	   		The maximum message size of the initiated protocol.
			/// @param 		   	eEndian			   		LITTLE when data bytes are ordered in Little Endian manner, BIG for Big Endian
			/// @param [out]	protocol				An address of a pointer to core::communication::client_channel_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(core::communication::client_channel_interface* port, size_t nSizeOfDataSize, size_t nPlaceOfDataSize, size_t nConstatSizeAddition, bool bAbsolute_data_size, size_t nMaxDataSize, core::types::endian eEndian, core::communication::client_channel_interface** protocol);

			/// @fn	static bool variable_length_protocol::create(core::communication::client_channel_interface* port, size_t nSizeOfDataSize, size_t nPlaceOfDataSize, bool bAbsolute_data_size, size_t nMaxDataSize,core::communication::client_channel_interface** protocol);
			/// @brief	Static factory: Creates a new variable length protocol
			/// @date	15/05/2018
			/// @param [in]		port			   		A pointer to the underlying channel to be used by the protocol (e.g. communication::ports::tcp_client_port).
			/// @param 		   	nSizeOfDataSize	   		The size of the the message length data (e.g. 4 bytes for a 32-bit integer value).
			/// @param 		   	nPlaceOfDataSize   		The offset from the beginning of the message to the length data.
			/// @param 		   	nConstatSizeAddition   	Fixed value to add to each message length read
			/// @param 		   	bAbsolute_data_size		True if the length data includes (nPlaceOfDataSize + nSizeOfDataSize), False if not.
			/// @param 		   	nMaxDataSize	   		The maximum message size of the initiated protocol.
			/// @param [out]	protocol				An address of a pointer to core::communication::client_channel_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(core::communication::client_channel_interface* port, size_t nSizeOfDataSize, size_t nPlaceOfDataSize, size_t nConstatSizeAddition, bool bAbsolute_data_size, size_t nMaxDataSize, core::communication::client_channel_interface** protocol);

			/// @fn	static bool variable_length_protocol::create(core::communication::client_channel_interface* port, size_t nSizeOfDataSize, size_t nPlaceOfDataSize, bool bAbsolute_data_size, size_t nMaxDataSize, core::communication::client_channel_interface** protocol);
			/// @brief	Static factory: Creates a new variable length protocol
			/// @date	15/05/2018
			/// @param [in]		port			   	A pointer to the underlying channel to be used by the protocol (e.g. communication::ports::tcp_client_port).
			/// @param 		   	nSizeOfDataSize	   	The size of the the message length data (e.g. 4 bytes for a 32-bit integer value).
			/// @param 		   	nPlaceOfDataSize   	The offset from the beginning of the message to the length data.
			/// @param 		   	bAbsolute_data_size	True if the length data includes (nPlaceOfDataSize + nSizeOfDataSize), False if not.
			/// @param 		   	nMaxDataSize	   	The maximum message size of the initiated protocol.
			/// @param [out]	protocol			An address of a pointer to core::communication::client_channel_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(core::communication::client_channel_interface* port, size_t nSizeOfDataSize, size_t nPlaceOfDataSize, bool bAbsolute_data_size, size_t nMaxDataSize, core::communication::client_channel_interface** protocol);
		};
	}
}