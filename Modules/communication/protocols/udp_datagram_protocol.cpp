#include "udp_datagram_protocol.h"

core::communication::communication_status communication::protocols::udp_datagram_protocol_impl::status() const
{
	return m_udp_port->status();
}

bool communication::protocols::udp_datagram_protocol_impl::connect()
{
	return m_udp_port->connect();
}

bool communication::protocols::udp_datagram_protocol_impl::disconnect()
{
	return m_udp_port->disconnect();
}

size_t communication::protocols::udp_datagram_protocol_impl::send(const void * buffer, size_t size) const
{
	return m_udp_port->send(buffer, size);
}

size_t communication::protocols::udp_datagram_protocol_impl::recieve(void* buffer, size_t size, core::communication::communication_error * commError)
{
	(void)size;
	return m_udp_port->recieve(buffer, 0, commError);
}

bool communication::protocols::udp_datagram_protocol_impl::query_local_endpoint(core::communication::ip_endpoint & end_point) const
{
	return m_udp_port->query_local_endpoint(end_point);
}

bool communication::protocols::udp_datagram_protocol_impl::query_remote_endpoint(core::communication::ip_endpoint & end_point) const
{
	return m_udp_port->query_remote_endpoint(end_point);
}

bool communication::protocols::udp_datagram_protocol::create(
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
		instance = utils::make_ref_count_ptr<udp_datagram_protocol_impl>(strRemoteHostname, nRemotePort, strLocalHostname, nLocalPort, multicast, receive_buffer_size, send_buffer_size);
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

bool communication::protocols::udp_datagram_protocol::create(
	const char* strRemoteHostname, uint16_t nRemotePort,
	const char* strLocalHostname, uint16_t nLocalPort,
	bool multicast,
	core::communication::client_channel_interface** client)
{
	return communication::protocols::udp_datagram_protocol::create(strRemoteHostname, nRemotePort, strLocalHostname, nLocalPort, multicast, 0, 0, client);
}

bool communication::protocols::udp_datagram_protocol::create(
	const char* strRemoteHostname, uint16_t nRemotePort,
	const char* strLocalHostname, uint16_t nLocalPort,
	core::communication::client_channel_interface** client)
{
	return communication::protocols::udp_datagram_protocol::create(strRemoteHostname, nRemotePort, strLocalHostname, nLocalPort, false, client);
}
