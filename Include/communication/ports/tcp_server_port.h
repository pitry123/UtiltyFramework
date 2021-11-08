/// @file	ports/tcp_server_port.h.
/// @brief	Declares the TCP server port class
#pragma once
#include <core/communication.h>

namespace communication
{
	namespace ports
	{
		/// @class	tcp_server_port
		/// @brief	A TCP server port.
		/// @date	15/05/2018
		class DLL_EXPORT tcp_server_port : public core::communication::server_channel_interface
		{
		public:
			/// @fn	virtual tcp_server_port::~tcp_server_port() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~tcp_server_port() = default;

			/// @fn	static bool tcp_server_port::create(const char* strLocalHostname, uint16_t usLocalPort, core::communication::server_channel_interface** channel);
			/// @brief	Static factory: Creates a new TCP server instance
			/// @date	15/05/2018
			/// @param 		   	strLocalHostname	The local host name (DNS resolved or IP address).
			/// @param 		   	usLocalPort			The local IP port.
			/// @param [out]	channel				An address of a pointer to core::communication::server_channel_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* strLocalHostname, uint16_t usLocalPort, core::communication::server_channel_interface** channel);

			/// @fn	static bool tcp_server_port::create(const char* strLocalHostname, uint16_t usLocalPort, core::communication::server_channel_interface** channel);
			/// @brief	Static factory: Creates a new TCP server instance
			/// @date	31/10/2020
			/// @param 		   	strLocalHostname	The local host name (DNS resolved or IP address).
			/// @param 		   	usLocalPort			The local IP port.
			/// @param			bNoDelaySend		True to cancel Nagel Algorithm that is batching messages to one packet
			/// @param [out]	channel				An address of a pointer to core::communication::server_channel_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* strLocalHostname, uint16_t usLocalPort, bool bNoDelay, core::communication::server_channel_interface** channel);
		};
	}
}