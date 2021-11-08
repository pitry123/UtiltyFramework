#pragma once
#include <core/communication.h>

namespace communication
{
	namespace protocols
	{
		class DLL_EXPORT udp_datagram_protocol : public core::communication::ip_client_channel_interface
		{
		public:
			virtual ~udp_datagram_protocol() = default;

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