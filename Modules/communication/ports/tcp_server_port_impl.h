#pragma once
#include "helpers.hpp"

#include <boost/asio.hpp>
#include <communication/ports/tcp_server_port.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <string>
#include <mutex>

namespace communication
{
	namespace ports
	{
		class tcp_server_port_impl : public utils::ref_count_base <communication::ports::tcp_server_port>
		{
		private:
			class tcp_servers_client : public utils::ref_count_base <core::communication::ip_client_channel_interface>
			{
			private:
				mutable boost::asio::ip::tcp::socket m_socket;
				core::communication::communication_status m_eStatus;

				core::communication::communication_error parse_error(boost::system::error_code ec)
				{
					core::communication::communication_error ePortError;
					switch (ec.value())
					{
					case 0:
						ePortError = core::communication::communication_error::NO_ERRORS;
						break;

					case 1:
						ePortError = core::communication::communication_error::NO_PERMISSION;
						break;

					case 13:
						ePortError = core::communication::communication_error::ACCESS_DENIED;
						break;

					case 113:
						ePortError = core::communication::communication_error::HOST_UNREACHABLE;
						break;

					case 110:
						ePortError = core::communication::communication_error::TIMED_OUT;
						break;

					case 111:
						ePortError = core::communication::communication_error::CONNECTION_REFUSED;
						break;

					default:
						ePortError = core::communication::communication_error::UNKNOWN_ERROR;
						break;
					}

					return ePortError;
				}
			public:
				tcp_servers_client(boost::asio::ip::tcp::socket socket) :
					m_socket(std::move(socket)),
					m_eStatus(core::communication::communication_status::CONNECTED)
				{
				}

				virtual core::communication::communication_status status() const override
				{
					return m_eStatus;
				}

				virtual bool connect() override
				{
					//this is done because in the server the connection is done by a client request
					return m_eStatus == core::communication::communication_status::CONNECTED;
				}

				virtual bool disconnect() override
				{
					bool ans = true;
					try
					{
						boost::system::error_code ec;
						m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
						if (ec.value() != 0)
						{
							ans = false;

						}
						m_socket.close(ec);
						if (ec.value() != 0)
						{
							ans = false;

						}
						m_eStatus = core::communication::communication_status::DISCONNECTED;
						return ans;
					}
					catch (...)
					{
						return false;
					}
				}

				virtual size_t send(const void* buffer, size_t size) const override
				{
					boost::system::error_code ec;
					size_t bytesRead = 0;
					try
					{
						bytesRead = boost::asio::write(m_socket, boost::asio::buffer(buffer, size), ec);
					}
					catch (...)
					{
						// TODO: Log...
					}

					return bytesRead;
				}

				virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override
				{
					boost::system::error_code ec;
					try
					{
						size_t nTotal = 0;
						size_t nDataleft = size;
						size_t nCurrentRead = 0;
						while (nTotal < size)
						{
							uint8_t* bytes_arr = static_cast<uint8_t*>(buffer);
							nCurrentRead = m_socket.receive(boost::asio::buffer(&bytes_arr[nTotal], nDataleft), 0, ec);
							if (nCurrentRead == 0)
							{
								m_eStatus = core::communication::communication_status::DISCONNECTED;
								*commError = parse_error(ec);
								return 0;
							}

							nTotal += nCurrentRead;
							nDataleft -= nCurrentRead;
						}
						*commError = parse_error(ec);
						return nTotal;
					}
					catch (std::exception& e)
					{
						(void)e;

						m_eStatus = core::communication::communication_status::DISCONNECTED;
						*commError = parse_error(ec);
						return 0;
					}
				}

				virtual bool query_local_endpoint(core::communication::ip_endpoint& end_point) const override
				{
					return communication::helpers::convert_ip_endpoint(m_socket.local_endpoint(), end_point);
				}

				virtual bool query_remote_endpoint(core::communication::ip_endpoint& end_point) const override
				{
					return communication::helpers::convert_ip_endpoint(m_socket.remote_endpoint(), end_point);
				}		
			};

		public:
			//--------------------------------------------------------
			//Class constructor
			//--------------------------------------------------------
			tcp_server_port_impl(const char* strLocalHostname, uint16_t usLocalPort, bool bNoDelaySend);
			~tcp_server_port_impl();

			// Inherited via ref_count_base
			virtual core::communication::communication_status status() const override;
			virtual bool connect() override;
			virtual bool disconnect() override;
			virtual bool accept(core::communication::client_channel_interface** client) override;

			//--------------------------------------------------------
			//member variables
			//--------------------------------------------------------
		private:
			std::string m_strLocalHostname;
			uint16_t m_usLocalPort;
			boost::asio::io_service m_io_service;
			core::communication::communication_status m_eStatus;
			boost::asio::ip::tcp::acceptor m_acceptor;
			bool m_noDelay;

			core::communication::communication_error parse_error(boost::system::error_code ec);

			std::atomic<bool> m_accepting;
			std::mutex m_accept_mutex;
		};
	}
}

