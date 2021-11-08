#include "fixed_length_protocol_impl.h"

communication::protocols::fixed_length_protocol_impl::fixed_length_protocol_impl(core::communication::client_channel_interface* port, size_t nReceiveTimeout, size_t nLength):
	m_port(port),
	m_nReceiveTimeout(nReceiveTimeout),
    m_nLength(nLength)
{
    // Unused
    (void)m_nReceiveTimeout;

}

bool communication::protocols::fixed_length_protocol::create(core::communication::client_channel_interface* port, size_t nReceiveTimeout, size_t nLength, core::communication::client_channel_interface** protocol)
{
	if (port == nullptr)
		return false;

	utils::ref_count_ptr<core::communication::client_channel_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<fixed_length_protocol_impl>(port, nReceiveTimeout, nLength);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*protocol = instance;
	(*protocol)->add_ref();
	return true;
}

core::communication::communication_status communication::protocols::fixed_length_protocol_impl::status() const
{
	return m_port->status();
}

bool communication::protocols::fixed_length_protocol_impl::connect()
{
	return m_port->connect();
}

bool communication::protocols::fixed_length_protocol_impl::disconnect()
{
	return m_port->disconnect();
}

size_t communication::protocols::fixed_length_protocol_impl::send(const void* buffer, size_t size) const
{
	return m_port->send(buffer, m_nLength);
}

size_t communication::protocols::fixed_length_protocol_impl::recieve(void* buffer, size_t size, core::communication::communication_error* commError)
{
	return m_port->recieve(buffer, m_nLength, commError);
}
