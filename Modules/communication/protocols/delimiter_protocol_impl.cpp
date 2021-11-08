#include "delimiter_protocol_impl.h"
#include <cstring>
#include <string>

communication::protocols::delimiter_protocol_impl::delimiter_protocol_impl(core::communication::client_channel_interface* port, const delimiter_couple* array_delimiter_couples, size_t couples_count, size_t nMaxMsgSize) :
	m_port(port),
    m_nMaxMsgSize(nMaxMsgSize)
{
    // Unused
    (void)m_nMaxMsgSize;

	if (array_delimiter_couples == nullptr)
		throw std::invalid_argument("array_delimiter_couples");

	if (couples_count == 0)
		throw std::invalid_argument("couples_count");

	m_delimiters.reserve(couples_count);
	m_temp_delimiters.reserve(couples_count);
	for (size_t i = 0; i < couples_count; i++)
	{
		m_delimiters.emplace_back(
			array_delimiter_couples[i]);
		
		m_temp_delimiters.emplace_back(
			std::strlen(array_delimiter_couples[i].strStartDelimiter),
			std::strlen(array_delimiter_couples[i].strEndDelimiter));
	}
}

bool communication::protocols::delimiter_protocol::create(core::communication::client_channel_interface* port, const delimiter_couple* array_delimiter_couples, size_t couples_count, size_t nMaxMsgSize, core::communication::client_channel_interface** protocol)
{
	if (port == nullptr)
		return false;

	if (array_delimiter_couples == nullptr)
		return false;

	if (couples_count == 0)
		return false;

	utils::ref_count_ptr<core::communication::client_channel_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<delimiter_protocol_impl>(port, array_delimiter_couples, couples_count, nMaxMsgSize);
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

bool communication::protocols::delimiter_protocol::create(core::communication::client_channel_interface* port, const delimiter_couple& couple, size_t nMaxMsgSize, core::communication::client_channel_interface** protocol)
{
	return create(port, &couple, 1, nMaxMsgSize, protocol);
}

core::communication::communication_status communication::protocols::delimiter_protocol_impl::status() const
{
	return m_port->status();
}

bool communication::protocols::delimiter_protocol_impl::connect()
{
	return m_port->connect();
}

bool communication::protocols::delimiter_protocol_impl::disconnect()
{
	return m_port->disconnect();
}

size_t communication::protocols::delimiter_protocol_impl::send(const void* buffer, size_t size) const
{
	return m_port->send(buffer,size);
}

size_t communication::protocols::delimiter_protocol_impl::recieve(void* buffer, size_t size, core::communication::communication_error* commError)
{

	bool bFoundStartDelimeter = false;
	bool bFoundEndDeliemeter = false;
	size_t coupleIndexFound = 0;
	size_t nBufferIndx = 0;
	uint8_t* bytes_arr = static_cast<uint8_t*>(buffer);

	// init temp delimiters
	for (size_t i = 0; i < m_temp_delimiters.size(); i++)
	{
		for (size_t j = 0; j < m_temp_delimiters[i].m_start_delimiter.length(); j++)
		{
			m_temp_delimiters[i].m_start_delimiter[j] = ' ';
		}
		for (size_t j = 0; j < m_temp_delimiters[i].m_end_delimiter.length(); j++)
		{
			m_temp_delimiters[i].m_end_delimiter[j] = ' ';
		}
	}
		
	// search for start delimiter
	bFoundStartDelimeter = false;
	while (!bFoundStartDelimeter)
	{
		char current_char;
		
		m_port->recieve((uint8_t*)&current_char, 1, commError);

		if (*commError != core::communication::communication_error::NO_ERRORS)
			return 0;


		// add new char
		for (size_t i = 0; i < m_temp_delimiters.size(); i++)
		{
			m_temp_delimiters[i].m_start_delimiter[m_temp_delimiters[i].m_start_delimiter.length()-1] = current_char;
		}
		// check if temp start delimiter contains in delimiters
		for (size_t i = 0; i < m_temp_delimiters.size(); i++)
		{
			if (m_temp_delimiters[i].m_start_delimiter.compare(m_delimiters[i].m_start_delimiter) == 0)
			{
				bFoundStartDelimeter = true;
				coupleIndexFound = i;
				break;
			}
			else
			{
				// shift left temp delimiters
				for (size_t j = 0; j < m_temp_delimiters[i].m_start_delimiter.length() - 1; j++)
				{
					m_temp_delimiters[i].m_start_delimiter[j] = m_temp_delimiters[i].m_start_delimiter[j + 1];
				}
				m_temp_delimiters[i].m_start_delimiter[m_delimiters[i].m_start_delimiter.length() - 1] = ' ';				
			}
		}

		
	}

	// copy start delimiter to buffer
	memcpy(&bytes_arr[nBufferIndx],&m_temp_delimiters[coupleIndexFound].m_start_delimiter[0], m_temp_delimiters[coupleIndexFound].m_start_delimiter.length());
	nBufferIndx += m_temp_delimiters[coupleIndexFound].m_start_delimiter.length();
	
	// search for end delimiter
	while (!bFoundEndDeliemeter)
	{
		char current_char;
		
		m_port->recieve((uint8_t*)&current_char, 1, commError);

		if (*commError != core::communication::communication_error::NO_ERRORS)
			return 0;

		m_temp_delimiters[coupleIndexFound].m_end_delimiter[m_temp_delimiters[coupleIndexFound].m_end_delimiter.length()-1] = current_char;
		// msg is bigger than maximum size
		if (nBufferIndx >= size)
		{
			return 0;
		}
		// add char to buffer
		memcpy(&bytes_arr[nBufferIndx], &current_char, 1);
		nBufferIndx++;
		if (m_temp_delimiters[coupleIndexFound].m_end_delimiter.compare(m_delimiters[coupleIndexFound].m_end_delimiter) == 0)
		{
			bFoundEndDeliemeter = true;	
            continue;
		}
		else
		{
			// shift left end delimeter
			for (size_t j = 0; j < m_temp_delimiters[coupleIndexFound].m_end_delimiter.length() - 1; j++)
			{
				m_temp_delimiters[coupleIndexFound].m_end_delimiter[j] = m_temp_delimiters[coupleIndexFound].m_end_delimiter[j + 1];
			}
			m_temp_delimiters[coupleIndexFound].m_end_delimiter[m_temp_delimiters[coupleIndexFound].m_end_delimiter.length() - 1] = ' ';			
		}
	}
	return nBufferIndx;
}
