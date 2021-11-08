#pragma once
#include <boost/asio.hpp>

#include <communication/ports/udp_client_port.h>
#include <core/buffer_interface.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <mutex>

namespace communication
{
	namespace ports
    {
		class udp_client_port_impl : public utils::ref_count_base<communication::ports::udp_client_port>
        {
		public:

			//--------------------------------------------------------
			//Class constructor
			//--------------------------------------------------------
			udp_client_port_impl(
				const char* strRemoteHostname, 
				uint16_t usRemotePort, 
				const char* strLocalHostname, 
				uint16_t usLocalPort, 
				bool multicast, 
				int receive_buffer_size,
				int send_buffer_size);

			// Inherited via ref_count_base
			virtual core::communication::communication_status status() const override;
			virtual bool connect() override;
			virtual bool disconnect() override;
			virtual size_t send(const void* buffer, size_t size) const override;
			virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override;			
			virtual bool query_local_endpoint(core::communication::ip_endpoint& end_point)  const override;
			virtual bool query_remote_endpoint(core::communication::ip_endpoint& end_point) const override;

        private:					
			boost::asio::ip::udp::endpoint m_local_endpoint;
			boost::asio::ip::udp::endpoint m_remote_endpoint;
			bool m_multicast;
			utils::ref_count_ptr<core::buffer_interface> m_cache;
			size_t m_cache_index;
			size_t m_last_read_size;
			core::communication::communication_status m_eStatus;
			boost::asio::io_service m_io_service;
			mutable boost::asio::ip::udp::socket m_socket;
			boost::asio::ip::udp::resolver m_resolver;

			int m_receive_buffer_size;
			int m_send_buffer_size;

			core::communication::communication_error ParseError(boost::system::error_code ec);			
		};
	}	
}

