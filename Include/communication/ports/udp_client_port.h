/// @file	ports/udp_client_port.h.
/// @brief	Declares the UDP client port class
#pragma once
#include <core/communication.h>
namespace communication
{
	namespace ports
	{
		/// @class	udp_client_port
		/// @brief	A UDP client port.
		/// @date	15/05/2018
		class DLL_EXPORT udp_client_port : public core::communication::ip_client_channel_interface
		{
		public:
			/// @fn	virtual udp_client_port::~udp_client_port() = default;
			/// @brief	Destructor
			/// @date	15/05/2018
			virtual ~udp_client_port() = default;

			/// @fn	static bool udp_client_port::create(const char* strRemoteHostname, uint16_t usRemotePort, const char* strLocalHostname, uint16_t usLocalPort, core::communication::client_channel_interface** client);
			/// @brief	Static factory: Creates a new UDP communication
			/// @date	15/05/2018
			/// @param 		   	strRemoteHostname	The remote host name (DNS resolved or IP address).
			/// @param 		   	usRemotePort	 	The remote IP port.
			/// @param 		   	strLocalHostname 	The local host name (DNS resolved or IP address).
			/// @param 		   	usLocalPort		 	The local IP port.
			/// @param 		   	receive_buffer_size	The configured receive buffer size
			/// @param			send_buffer_size	The configured send buffer size
			/// @param [out]	client			 	An address to a pointer to core::communication::client_channel_interface
			/// @return	True if it succeeds, false if it fails.
			static bool create(
				const char* strRemoteHostname, 
				uint16_t usRemotePort, 
				const char* strLocalHostname, 
				uint16_t usLocalPort, 
				bool multicast,
				int receive_buffer_size,
				int send_buffer_size,
				core::communication::client_channel_interface** client);

			static bool create(const char* strRemoteHostname, uint16_t usRemotePort, const char* strLocalHostname, uint16_t usLocalPort, core::communication::client_channel_interface** client);

			static bool create(const char* strRemoteHostname, uint16_t usRemotePort, const char* strLocalHostname, uint16_t usLocalPort, bool multicast, core::communication::client_channel_interface** client);
		};
	}
}