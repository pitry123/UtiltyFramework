#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4834) // discarding return value of function with 'nodiscard' attribute (VS2019 + boost asio)
#endif

#include "tcp_client_port_impl.h"
#include "helpers.hpp"

bool communication::ports::tcp_client_port::create(const char* strRemoteHostname, uint16_t nRemotePort, const char* strLocalHostname, uint16_t nLocalPort, core::communication::client_channel_interface** channel)
{
	return create(strRemoteHostname, nRemotePort, strLocalHostname, nLocalPort, false, channel);
}

bool communication::ports::tcp_client_port::create(const char* strRemoteHostname, uint16_t nRemotePort, const char* strLocalHostname, uint16_t nLocalPort, bool bNoDelaySend, core::communication::client_channel_interface** channel)
{
	if (channel == nullptr)
		return false;

	utils::ref_count_ptr<core::communication::client_channel_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<tcp_client_port_impl>(strRemoteHostname, nRemotePort, strLocalHostname, nLocalPort, bNoDelaySend);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*channel = instance;
	(*channel)->add_ref();
	return true;
}

communication::ports::tcp_client_port_impl::tcp_client_port_impl(const char* strRemoteHostname, uint16_t usRemotePort, const char* strLocalHostname, uint16_t usLocalPort, bool bNoDelaySend) :
	m_strRemoteHostname(strRemoteHostname),
	m_usRemotePort(usRemotePort),
	m_strLocalHostname(strLocalHostname),
	m_usLocalPort(usLocalPort),
	m_eStatus(core::communication::communication_status::DISCONNECTED),
	m_socket(m_io_service),
	m_resolver(m_io_service),
	m_noDelay(bNoDelaySend)
{
}



core::communication::communication_status communication::ports::tcp_client_port_impl::status() const
{
	return m_eStatus;
}

bool communication::ports::tcp_client_port_impl::connect()
{
	disconnect();

	try {
		boost::system::error_code ec;
		boost::asio::ip::tcp::endpoint localEndpoint(boost::asio::ip::address::from_string(m_strLocalHostname), m_usLocalPort);
		boost::asio::ip::tcp::endpoint remoteEndpoint(boost::asio::ip::address::from_string(m_strRemoteHostname), m_usRemotePort);
		m_socket.open(boost::asio::ip::tcp::v4(), ec);
		if (ec.value() != 0) 
		{
			return false;
		}
		m_socket.set_option(boost::asio::ip::tcp::no_delay(m_noDelay), ec);
		if (ec.value() != 0)
		{
			return false;
		}
		m_socket.bind(localEndpoint, ec);
		if (ec.value() != 0) 
		{
			return false;
		}
		m_socket.connect(remoteEndpoint, ec);
		if (ec.value() != 0) 
		{
			return false;
		}
		m_eStatus = core::communication::communication_status::CONNECTED;
		return true;
	}
	catch (...) {
		return false;
	}
}

bool communication::ports::tcp_client_port_impl::disconnect()
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

size_t communication::ports::tcp_client_port_impl::send(const void* buffer, size_t size) const
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

size_t communication::ports::tcp_client_port_impl::recieve(void* buffer, size_t size, core::communication::communication_error* commError)
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
				*commError = ParseError(ec);
				return 0;
			}

			nTotal += nCurrentRead;
			nDataleft -= nCurrentRead;
		}
		*commError = ParseError(ec);
		return nTotal;
	}
	catch (std::exception& e)
	{
		(void)e;

		m_eStatus = core::communication::communication_status::DISCONNECTED;
		*commError = ParseError(ec);
		return 0;
	}
}

core::communication::communication_error communication::ports::tcp_client_port_impl::ParseError(boost::system::error_code ec)
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

bool communication::ports::tcp_client_port_impl::query_local_endpoint(core::communication::ip_endpoint& end_point)const
{
	return communication::helpers::convert_ip_endpoint(m_socket.local_endpoint(), end_point);
}

bool communication::ports::tcp_client_port_impl::query_remote_endpoint(core::communication::ip_endpoint& end_point) const
{
	return communication::helpers::convert_ip_endpoint(m_socket.remote_endpoint(), end_point);
}

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#	pragma warning(pop)
#endif