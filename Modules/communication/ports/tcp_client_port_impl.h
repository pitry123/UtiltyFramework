#pragma once
#include <boost/asio.hpp>

#include <communication/ports/tcp_client_port.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <mutex>
#include <string>

namespace communication
{
	namespace ports
	{
		class tcp_client_port_impl : public utils::ref_count_base <communication::ports::tcp_client_port>
		{
		public:

			//--------------------------------------------------------
			//Class constructor
			//--------------------------------------------------------
			tcp_client_port_impl(const char* strRemoteHostname, uint16_t usRemotePort, const char* strLocalHostname, uint16_t usLocalPort, bool bNoDelaySend);

			
			// Inherited via ref_count_base
			virtual core::communication::communication_status status() const override;
			virtual bool connect() override;
			virtual bool disconnect() override;
			virtual size_t send(const void* buffer, size_t size) const override;
			virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override;
			virtual bool query_local_endpoint(core::communication::ip_endpoint& end_point)  const override ;
			virtual bool query_remote_endpoint(core::communication::ip_endpoint& end_point)const override ;			
			
		private:
			//--------------------------------------------------------
			//member variables
			//--------------------------------------------------------
			std::string m_strRemoteHostname;
			uint16_t m_usRemotePort;
			std::string m_strLocalHostname;
			uint16_t m_usLocalPort;
			core::communication::communication_status m_eStatus;
			boost::asio::io_service m_io_service;
			mutable boost::asio::ip::tcp::socket m_socket;
			boost::asio::ip::tcp::resolver m_resolver;
			bool m_noDelay;

			core::communication::communication_error ParseError(boost::system::error_code ec);
		};
	}	
}

