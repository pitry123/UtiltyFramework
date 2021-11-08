#pragma once
#include <communication/protocols/udp_datagram_protocol.h>
#include <communication/ports/udp_client_port.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <mutex>

namespace communication
{
	namespace protocols
	{
		class udp_datagram_protocol_impl : public utils::ref_count_base<communication::protocols::udp_datagram_protocol>
		{
		public:
			udp_datagram_protocol_impl(
				const char* strRemoteHostname, uint16_t nRemotePort,
				const char* strLocalHostname, uint16_t nLocalPort,
				bool multicast,
				int receive_buffer_size,
				int send_buffer_size)
			{
				utils::ref_count_ptr<core::communication::client_channel_interface> port;
				if (communication::ports::udp_client_port::create(
					strRemoteHostname,
					nRemotePort,
					strLocalHostname,
					nLocalPort,
					multicast,
					receive_buffer_size,
					send_buffer_size,
					&port) == false)
					throw std::runtime_error("Failed to create UDP port");

				m_udp_port = static_cast<core::communication::ip_client_channel_interface*>(
					static_cast<core::communication::client_channel_interface*>(port));
			}

			virtual core::communication::communication_status status() const override;
			virtual bool connect() override;
			virtual bool disconnect() override;
			virtual size_t send(const void* buffer, size_t size) const override;
			virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override;

			virtual bool query_local_endpoint(core::communication::ip_endpoint& end_point) const override;
			virtual bool query_remote_endpoint(core::communication::ip_endpoint& end_point) const override;

		private:
			utils::ref_count_ptr<core::communication::ip_client_channel_interface> m_udp_port;
		};
	}
}

