#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4834) // discarding return value of function with 'nodiscard' attribute (VS2019 + boost asio)
#endif

#include "udp_client_port_impl.h"
#include "helpers.hpp"

#include <utils/buffer_allocator.hpp>

// The actual limit for the UDP data length,
// which is imposed by the underlying IPv4 protocol,
// is 65,507 bytes (65,535 - 8 byte UDP header - 20 byte IP header)
static constexpr size_t UDP_CACHE_SIZE = 65507;

bool communication::ports::udp_client_port::create(
	const char* strRemoteHostname, uint16_t nRemotePort, 
	const char* strLocalHostname, uint16_t nLocalPort,
	bool multicast,
	int receive_buffer_size,
	int send_buffer_size,
	core::communication::client_channel_interface** client)
{
	if (client == nullptr)
		return false;

	utils::ref_count_ptr<core::communication::client_channel_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<udp_client_port_impl>(strRemoteHostname, nRemotePort, strLocalHostname, nLocalPort, multicast, receive_buffer_size, send_buffer_size);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*client = instance;
	(*client)->add_ref();
	return true;
}

bool communication::ports::udp_client_port::create(
	const char* strRemoteHostname, uint16_t nRemotePort,
	const char* strLocalHostname, uint16_t nLocalPort,
	bool multicast,
	core::communication::client_channel_interface** client)
{
	return communication::ports::udp_client_port::create(strRemoteHostname, nRemotePort, strLocalHostname, nLocalPort, multicast, 0, 0, client);
}

bool communication::ports::udp_client_port::create(
	const char* strRemoteHostname, uint16_t nRemotePort,
	const char* strLocalHostname, uint16_t nLocalPort,
	core::communication::client_channel_interface** client)
{
	return communication::ports::udp_client_port::create(strRemoteHostname, nRemotePort, strLocalHostname, nLocalPort, false, client);
}

communication::ports::udp_client_port_impl::udp_client_port_impl(
	const char* strRemoteHostname, 
	uint16_t usRemotePort, 
	const char* strLocalHostname, 
	uint16_t usLocalPort,
	bool multicast,
	int receive_buffer_size,
	int send_buffer_size) :
	m_local_endpoint(boost::asio::ip::address::from_string(strLocalHostname), usLocalPort),
	m_remote_endpoint(boost::asio::ip::address::from_string(strRemoteHostname), usRemotePort),
	m_multicast(multicast),
    m_cache(utils::make_ref_count_ptr<utils::ref_count_buffer>(UDP_CACHE_SIZE)),
	m_cache_index(0),
	m_last_read_size(0),
	m_eStatus(core::communication::communication_status::DISCONNECTED),
	m_socket(m_io_service),
	m_resolver(m_io_service),
	m_receive_buffer_size(receive_buffer_size),
	m_send_buffer_size(send_buffer_size)
{	
}

core::communication::communication_status communication::ports::udp_client_port_impl::status() const
{
	return m_eStatus;
}

bool communication::ports::udp_client_port_impl::connect()
{	
	disconnect();
	m_cache_index = m_last_read_size = 0;

	bool ans = true;
	try 
	{
		boost::system::error_code ec;		
		m_socket.open(boost::asio::ip::udp::v4(), ec);
		if (ec.value() != 0)
		{
			ans = false;
		}	

		if (m_receive_buffer_size > 0)
		{
			m_socket.set_option(boost::asio::socket_base::receive_buffer_size(m_receive_buffer_size), ec);
			if (ec.value() != 0)
			{
				ans = false;
			}
		}

		if (m_send_buffer_size > 0)
		{
			m_socket.set_option(boost::asio::socket_base::send_buffer_size(m_send_buffer_size), ec);
			if (ec.value() != 0)
			{
				ans = false;
			}
		}

		m_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
		if (ec.value() != 0)
		{
			ans = false;
		}
		
		m_socket.bind(m_local_endpoint, ec);
		if (ec.value() != 0)
		{
			ans = false;
		}

		if (m_multicast == true)
		{
			m_socket.set_option(boost::asio::ip::multicast::join_group(m_remote_endpoint.address().to_v4(), m_local_endpoint.address().to_v4()), ec);		
			if (ec.value() != 0)
			{
				ans = false;
			}
		}				

		if (ans ==false)
		{
			m_eStatus = core::communication::communication_status::DISCONNECTED;
		}
		else
		{
			m_eStatus = core::communication::communication_status::CONNECTED;
		}

		return ans;
	}
	catch (...) 
	{
		m_eStatus = core::communication::communication_status::DISCONNECTED;
		return ans;
	}
}

bool communication::ports::udp_client_port_impl::disconnect()
{
	bool ans = true;
	try
	{
		boost::system::error_code ec;

		if (m_multicast == true)
		{			
			m_socket.set_option(boost::asio::ip::multicast::leave_group(m_remote_endpoint.address().to_v4(), m_local_endpoint.address().to_v4()), ec);
			if (ec.value() != 0)
			{
				ans = false;
			}
		}

		m_socket.shutdown(boost::asio::ip::udp::socket::shutdown_both, ec);
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

size_t communication::ports::udp_client_port_impl::send(const void* buffer, size_t size) const
{
	try
	{
		boost::system::error_code ec;
		size_t retval = m_socket.send_to(boost::asio::buffer(buffer, size), m_remote_endpoint, 0, ec);
		if (ec.value() != 0)
			return 0;

		return retval;
	}
	catch (...) 
	{
		return 0;
	}
}

size_t communication::ports::udp_client_port_impl::recieve(void* buffer, size_t size, core::communication::communication_error* commError)
{
	boost::system::error_code ec;
	try 
	{
		size_t nTotal = 0;

		if (size == 0)
		{
			m_cache_index = 0;
			while (nTotal == 0)
			{
				boost::asio::ip::udp::endpoint recieve_endpoint;
				nTotal = m_socket.receive_from(boost::asio::buffer(m_cache->data(), m_cache->size()), recieve_endpoint, 0, ec);

				if (nTotal == 0)
				{
					// TODO: Log error...
					m_eStatus = core::communication::communication_status::DISCONNECTED;
					*commError = ParseError(ec);
					return 0;
				}

				if (m_multicast == false && (m_remote_endpoint.address().is_unspecified() == false) && (recieve_endpoint.address() != m_remote_endpoint.address()))
				{
					// Ignoring this message
					nTotal = 0;
					continue;
				}

				std::memcpy(buffer, m_cache->data(), nTotal);
			}
		}
		else
		{
			size_t nDataleft = size;
			uint8_t* target_buffer = static_cast<uint8_t*>(buffer);
			while (nTotal < size)
			{
				bool read = (m_cache_index == m_last_read_size);

				if (read == true)
				{
					boost::asio::ip::udp::endpoint recieve_endpoint;
					size_t current_read_size = m_socket.receive_from(boost::asio::buffer(m_cache->data(), m_cache->size()), recieve_endpoint, 0, ec);

					if (current_read_size == 0)
					{
						// TODO: Log error...
						m_eStatus = core::communication::communication_status::DISCONNECTED;
						*commError = ParseError(ec);
						return 0;
					}

					if (m_multicast == false && (m_remote_endpoint.address().is_unspecified() == false) && (recieve_endpoint.address() != m_remote_endpoint.address()))
					{
						continue;
					}

					// Valid read. Setting cache members
					m_cache_index = 0;
					m_last_read_size = current_read_size;
				}

				size_t read_size = (std::min)(m_last_read_size - m_cache_index, nDataleft);
				std::memcpy(target_buffer + nTotal, m_cache->data() + m_cache_index, read_size);
				m_cache_index += read_size;
				nTotal += read_size;
				nDataleft -= read_size;
			}

			*commError = ParseError(ec);
		}

		return nTotal;
	}
	catch (std::exception& e)
	{
		(void)e;

		*commError = ParseError(ec);
		m_eStatus = core::communication::DISCONNECTED;
		return 0;
	}
}

core::communication::communication_error communication::ports::udp_client_port_impl::ParseError(boost::system::error_code ec)
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

 bool communication::ports::udp_client_port_impl::query_local_endpoint(core::communication::ip_endpoint& end_point) const 
{
	 return communication::helpers::convert_ip_endpoint(m_socket.local_endpoint(), end_point);
}
 bool communication::ports::udp_client_port_impl::query_remote_endpoint(core::communication::ip_endpoint& end_point) const 
{
	 return communication::helpers::convert_ip_endpoint(m_socket.remote_endpoint(), end_point);
}

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#	pragma warning(pop)
#endif