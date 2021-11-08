#include "variable_length_protocol_impl.h"
#include <utils/types.hpp>

communication::protocols::variable_length_protocol_impl::variable_length_protocol_impl(
	core::communication::client_channel_interface* port, 
	size_t nSizeOfDataSize, 
	size_t nPlaceOfDataSize,
	size_t nConstatSizeAddition,
	bool bAbsolute_data_size,
	size_t nMaxDataSize,
	core::types::endian eEndian):
	m_port(port),
	m_nSizeOfDataSize(nSizeOfDataSize),
	m_nPlaceOfDataSize(nPlaceOfDataSize),
	m_nConstatSizeAddition(nConstatSizeAddition),
	m_bAbsolute_data_size(bAbsolute_data_size),
	m_nMaxDataSize(nMaxDataSize),
	m_bEndianMismatch(eEndian != utils::types::platform_endian())
{
}

bool communication::protocols::variable_length_protocol::create(
	core::communication::client_channel_interface* port,
	size_t nSizeOfDataSize, 
	size_t nPlaceOfDataSize, 
	size_t nConstatSizeAddition,
	bool bAbsolute_data_size,
	size_t nMaxDataSize,
	core::types::endian eEndian,
	core::communication::client_channel_interface** protocol)
{
	if (port == nullptr)
		return false;

	utils::ref_count_ptr<core::communication::client_channel_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<variable_length_protocol_impl>(port, nSizeOfDataSize, nPlaceOfDataSize , nConstatSizeAddition, bAbsolute_data_size, nMaxDataSize, eEndian);
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

bool communication::protocols::variable_length_protocol::create(
	core::communication::client_channel_interface* port,
	size_t nSizeOfDataSize,
	size_t nPlaceOfDataSize,
	size_t nConstatSizeAddition,
	bool bAbsolute_data_size,
	size_t nMaxDataSize,
	core::communication::client_channel_interface** protocol)
{
	return communication::protocols::variable_length_protocol::create(port, nSizeOfDataSize, nPlaceOfDataSize, nConstatSizeAddition, bAbsolute_data_size, nMaxDataSize, utils::types::platform_endian(), protocol);
}

bool communication::protocols::variable_length_protocol::create(
	core::communication::client_channel_interface* port,
	size_t nSizeOfDataSize,
	size_t nPlaceOfDataSize,
	bool bAbsolute_data_size,
	size_t nMaxDataSize,
	core::communication::client_channel_interface** protocol)
{
	return communication::protocols::variable_length_protocol::create(port, nSizeOfDataSize, nPlaceOfDataSize, 0, bAbsolute_data_size, nMaxDataSize, utils::types::platform_endian(), protocol);
}

core::communication::communication_status communication::protocols::variable_length_protocol_impl::status() const
{
	return m_port->status();
}

bool communication::protocols::variable_length_protocol_impl::connect()
{
	return m_port->connect();
}

bool communication::protocols::variable_length_protocol_impl::disconnect()
{
	return m_port->disconnect();
}

size_t communication::protocols::variable_length_protocol_impl::send(const void* buffer, size_t size) const
{
	return m_port->send(buffer, size);
}

size_t communication::protocols::variable_length_protocol_impl::recieve(void* buffer, size_t size, core::communication::communication_error* commError)
{
	try {

		size_t nNumRecived = m_port->recieve(buffer, m_nPlaceOfDataSize + m_nSizeOfDataSize, commError);
		if (0 == nNumRecived) {
			return 0;
		}
		size_t nDataSize = 0;
		uint8_t* bytes_arr = static_cast<uint8_t*>(buffer);
		switch (m_nSizeOfDataSize)
		{
		case variable_length_protocol_impl::ByteSize8Bit:
			nDataSize = (uint8_t)bytes_arr[m_nPlaceOfDataSize];
			break;
		case variable_length_protocol_impl::ShortSize16Bit:
		{
			uint16_t nConvertedDataSize = *(reinterpret_cast<uint16_t*>(&bytes_arr[m_nPlaceOfDataSize]));			
			if (m_bEndianMismatch == true)
			{
				utils::types::endian_swap(nConvertedDataSize);
			}

			nDataSize = static_cast<size_t>(nConvertedDataSize);
		}
			break;
		case variable_length_protocol_impl::IntSize32Bit:
		{
			uint32_t nConvertedDataSize = *(reinterpret_cast<uint32_t*>(&bytes_arr[m_nPlaceOfDataSize]));
			if (m_bEndianMismatch == true)
			{
				utils::types::endian_swap(nConvertedDataSize);
			}

			nDataSize = static_cast<size_t>(nConvertedDataSize);
		}
			break;
		default:
			break;
		}

		if (nDataSize == 0)
			return nNumRecived;

		nDataSize += m_nConstatSizeAddition;

		if (m_bAbsolute_data_size == true)
			nDataSize -= (m_nPlaceOfDataSize + m_nSizeOfDataSize);
		
		if (nDataSize + (m_nPlaceOfDataSize + m_nSizeOfDataSize) > m_nMaxDataSize)
			return 0;		

		// udp case we got all data in bufferheader
		if (nNumRecived == nDataSize + m_nPlaceOfDataSize + m_nSizeOfDataSize)
		{
			return nNumRecived;
		}
		else 
		{
			// tcp case we got just header we need to read the data from stream
			nNumRecived = m_port->recieve(bytes_arr + m_nPlaceOfDataSize + m_nSizeOfDataSize, nDataSize, commError);
			if (0 == nNumRecived)
				return 0;

			return nNumRecived + m_nPlaceOfDataSize + m_nSizeOfDataSize;
		}
	}
	catch (...) 
	{
		return 0;
	}
}
