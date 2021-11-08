/// @file	ports/tcp_client_port.h.
/// @brief	Declares the TCP client port class
#pragma once
#include <core/communication.h>

namespace communication
{
	namespace ports
	{
		/// @class	tcp_client_port
		/// @brief	A TCP client port.
		/// @date	15/05/2018
		class DLL_EXPORT tcp_client_port : public core::communication::ip_client_channel_interface
		{
		public:
			/// @fn	virtual tcp_client_port::~tcp_client_port() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~tcp_client_port() = default;

			/// @fn	static bool tcp_client_port::create(const char* strRemoteHostname, uint16_t usRemotePort, const char* strLocalHostname, uint16_t usLocalPort, core::communication::client_channel_interface** channel);
			/// @brief	Static factory: Creates a new TCP client instance
			/// @date	15/05/2018
			/// @param 		   	strRemoteHostname	The remote host name (DNS resolved or IP address).
			/// @param 		   	usRemotePort	 	The remote IP port.
			/// @param 		   	strLocalHostname 	The local host name  (DNS resolved or IP address).
			/// @param 		   	usLocalPort		 	The local IP port.
			/// @param [out]	channel			 	An address of a pointer to core::communication::client_channel_interface.
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* strRemoteHostname, uint16_t usRemotePort, const char* strLocalHostname, uint16_t usLocalPort, core::communication::client_channel_interface** channel);

			/// @fn	static bool tcp_client_port::create(const char* strRemoteHostname, uint16_t usRemotePort, const char* strLocalHostname, uint16_t usLocalPort, core::communication::client_channel_interface** channel);
			/// @brief	Static factory: Creates a new TCP client instance
			/// @date	31/10/2020
			/// @param 		   	strRemoteHostname	The remote host name (DNS resolved or IP address).
			/// @param 		   	usRemotePort	 	The remote IP port.
			/// @param 		   	strLocalHostname 	The local host name  (DNS resolved or IP address).
			/// @param 		   	usLocalPort		 	The local IP port.
			/// @param			bNoDelaySend		True to cancel Nagel Algorithm that is batching messages to one packet
			/// @param [out]	channel			 	An address of a pointer to core::communication::client_channel_interface.
			/// @return	True if it succeeds, false if it fails.
			static bool create(const char* strRemoteHostname, uint16_t usRemotePort, const char* strLocalHostname, uint16_t usLocalPort, bool bNoDelay, core::communication::client_channel_interface** channel);
		};
	}
}