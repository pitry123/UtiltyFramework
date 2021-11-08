/// @file	protocols/delimiter_protocol.h.
/// @brief	Declares the delimiter protocol class
#pragma once
#include <core/communication.h>

namespace communication
{
	namespace protocols
	{
		/// @struct	delimiter_couple
		/// @brief	Defines the delimiter characters as null terminated strings.
		/// @date	15/05/2018
		struct delimiter_couple
		{
			/// @brief	The start delimiter
			const char* strStartDelimiter;
			/// @brief	The end delimiter
			const char* strEndDelimiter;
		};

		/// @class	delimiter_protocol
		/// @brief	Delimiter protocols split a data stream to messages by searching for start and end delimiter strings
		/// @date	15/05/2018
		class DLL_EXPORT delimiter_protocol : public core::communication::client_channel_interface
		{
		public:
			/// @fn	virtual delimiter_protocol::~delimiter_protocol() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~delimiter_protocol() = default;

			/// @fn	static bool delimiter_protocol::create(core::communication::client_channel_interface* port, const delimiter_couple* array_delimiter_couples, size_t couples_count, size_t nMaxMsgSize, core::communication::client_channel_interface** protocol);
			/// @brief	Static factory: Creates a new delimiter protocol instance
			/// @date	15/05/2018
			/// @param [in]		port				   	A pointer to the underlying channel to be used by the protocol (e.g. communication::ports::serial_port).
			/// @param 		   	array_delimiter_couples	An array of delimiter couples which allows the protocol to support multiple delimiters to be inspected.
			/// @param 		   	couples_count		   	The number of delimiter couples in array_delimiter_couples.
			/// @param 		   	nMaxMsgSize			   	The maximum message size of the initiated protocol.
			/// @param [out]	protocol			   	An address of a pointer to core::communication::client_channel_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(core::communication::client_channel_interface* port, const delimiter_couple* array_delimiter_couples, size_t couples_count, size_t nMaxMsgSize, core::communication::client_channel_interface** protocol);

			/// @fn	static bool delimiter_protocol::create(core::communication::client_channel_interface* port, const delimiter_couple& couple, size_t nMaxMsgSize, core::communication::client_channel_interface** protocol);
			/// @brief	Static factory: Creates a new delimiter protocol instance
			/// @date	15/05/2018
			/// @param [in]		port	   	A pointer to the underlying channel to be used by the protocol (e.g. communication::ports::serial_port).
			/// @param 		   	couple	   	The delimiter couple (This factory initiates the protocol with exactly 1 delimiter couple).
			/// @param 		   	nMaxMsgSize	The maximum message size of the initiated protocol.
			/// @param [out]	protocol	An address of a pointer to core::communication::client_channel_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(core::communication::client_channel_interface* port, const delimiter_couple& couple, size_t nMaxMsgSize, core::communication::client_channel_interface** protocol);
		};
	}
}