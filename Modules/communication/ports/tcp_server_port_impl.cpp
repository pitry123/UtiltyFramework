#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4834) // discarding return value of function with 'nodiscard' attribute (VS2019 + boost asio)
#endif

#include "tcp_server_port_impl.h"

bool communication::ports::tcp_server_port::create(const char* strLocalHostname, uint16_t nLocalPort, core::communication::server_channel_interface** channel)
{
	return create(strLocalHostname, nLocalPort, false, channel);
}

bool communication::ports::tcp_server_port::create(const char* strLocalHostname, uint16_t nLocalPort, bool bNoDelaySend, core::communication::server_channel_interface** channel)
{
	if (channel == nullptr)
		return false;

	utils::ref_count_ptr<core::communication::server_channel_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<tcp_server_port_impl>(strLocalHostname, nLocalPort, bNoDelaySend);
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

communication::ports::tcp_server_port_impl::tcp_server_port_impl(const char* strLocalHostname, uint16_t usLocalPort, bool bNoDelaySend) :
	m_strLocalHostname(strLocalHostname),
	m_usLocalPort(usLocalPort),
	m_eStatus(core::communication::communication_status::DISCONNECTED),
	m_acceptor(m_io_service),
	m_noDelay(bNoDelaySend)
{	
}

communication::ports::tcp_server_port_impl::~tcp_server_port_impl()
{
	m_accepting = false;
	disconnect();	

	std::lock_guard<std::mutex> locker(m_accept_mutex);	
}

core::communication::communication_status communication::ports::tcp_server_port_impl::status() const
{
	return m_eStatus;
}

bool communication::ports::tcp_server_port_impl::connect()
{
    disconnect();

	try
	{
		boost::asio::ip::tcp::endpoint endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(m_strLocalHostname), m_usLocalPort);
		m_acceptor.open(endpoint.protocol());
		m_acceptor.set_option(boost::asio::socket_base::reuse_address(true));
		m_acceptor.bind(endpoint);
		m_acceptor.listen();
	}
	catch (...)
	{
		return false;
	}

	m_accepting = true;
	m_eStatus = core::communication::communication_status::CONNECTED;
	return true;
}

bool communication::ports::tcp_server_port_impl::disconnect()
{
	m_eStatus = core::communication::communication_status::DISCONNECTED;

	try
	{
		m_acceptor.close();
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool communication::ports::tcp_server_port_impl::accept(core::communication::client_channel_interface** client)
{
	if (client == nullptr)
		return false;

	if (m_accepting.load() == false)
		return false;

	utils::ref_count_ptr<core::communication::client_channel_interface> instance;

	try
	{
		std::lock_guard<std::mutex> locker(m_accept_mutex);

		boost::asio::ip::tcp::socket client_socket(m_io_service);
		boost::system::error_code ec;

		m_acceptor.async_accept(client_socket, [&](boost::system::error_code innerec)
		{
			ec = innerec;
		});

        m_io_service.run();
        m_io_service.reset();
		
        if (ec.value() != 0)
			return false;

		client_socket.set_option(boost::asio::ip::tcp::no_delay(m_noDelay), ec);
		if (ec.value() != 0)
		{
			return false;
		}

		instance = utils::make_ref_count_ptr<tcp_servers_client>(std::move(client_socket));
	}
	catch (...)
	{
		m_eStatus = core::communication::communication_status::DISCONNECTED;
		return false;
	}

	if (instance == nullptr)
		return false;

	*client = instance;
	(*client)->add_ref();
	return true;
}

core::communication::communication_error communication::ports::tcp_server_port_impl::parse_error(boost::system::error_code ec)
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

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#	pragma warning(pop)
#endif
