#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4834) // discarding return value of function with 'nodiscard' attribute (VS2019 + boost asio)
#endif

#include "serial_port_impl.h"

communication::ports::serial_port_impl::serial_port_impl(
	const char* strPortName, uint32_t nBaudRate,
	communication::ports::serial_port::parity eParity, 
	size_t nDataBits, 
	communication::ports::serial_port::stop_bits eStopBits) :
	m_strPortName(strPortName),
	m_nBaudRate(nBaudRate),
	m_eParity(eParity),
	m_nDataBits(nDataBits),
	m_eStopBits(eStopBits),
	m_eStatus(core::communication::communication_status::DISCONNECTED),
	m_port(m_io_service)	
{
}

bool communication::ports::serial_port::create(
	const char* portName, 
	uint32_t baudRate, 
	communication::ports::serial_port::parity parity, 
	size_t dataBits, 
	communication::ports::serial_port::stop_bits stop_bits,
	core::communication::client_channel_interface** channel)
{
	if (channel == nullptr)
		return false;

	utils::ref_count_ptr<core::communication::client_channel_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<serial_port_impl>(portName, baudRate, parity, dataBits, stop_bits);
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

core::communication::communication_status communication::ports::serial_port_impl::status() const
{
	return m_eStatus;
}

bool communication::ports::serial_port_impl::connect()
{
	// Conversion mapping between infrastructure parity enum and boost parity enum
	boost::asio::serial_port::parity::type parity[] =		{ boost::asio::serial_port::parity::none,				// communication::ports::serial_port::parity::None
															  boost::asio::serial_port::parity::odd,				// communication::ports::serial_port::parity::Odd
															  boost::asio::serial_port::parity::even,				// communication::ports::serial_port::parity::Even
															  boost::asio::serial_port::parity::none,				// communication::ports::serial_port::parity::Mark	- No such option in boost. Use default
															  boost::asio::serial_port::parity::none };				// communication::ports::serial_port::parity::Space - No such option in boost. Use default 
	// Conversion mapping between infrastructure stop_bits enum and boost stop_bits enum
	boost::asio::serial_port::stop_bits::type stop_bits[] = { boost::asio::serial_port::stop_bits::one,				// communication::ports::serial_port::stop_bits::StopBitsNone - No such option in boost. Use default
															  boost::asio::serial_port::stop_bits::one,				// communication::ports::serial_port::stop_bits::One
															  boost::asio::serial_port::stop_bits::two,				// communication::ports::serial_port::stop_bits::Two
															  boost::asio::serial_port::stop_bits::onepointfive };	// communication::ports::serial_port::stop_bits::OnePointFive
	try 
	{
		m_port.open(m_strPortName);
		m_port.set_option(boost::asio::serial_port_base::baud_rate(m_nBaudRate));
		m_port.set_option(boost::asio::serial_port_base::parity(parity[m_eParity]));
		m_port.set_option(boost::asio::serial_port_base::stop_bits(stop_bits[m_eStopBits]));
		m_port.set_option(boost::asio::serial_port_base::character_size(static_cast<unsigned int>(m_nDataBits)));

		m_eStatus = core::communication::communication_status::CONNECTED;
		return true;
	}
	catch (...) 
	{
		m_eStatus = core::communication::communication_status::DISCONNECTED;
		return false;
	}
}

bool communication::ports::serial_port_impl::disconnect()
{
	bool ans = true;
	try 
	{
		boost::system::error_code ec;
		m_port.close(ec);
		if (ec.value() != 0)
		{
			ans = false;
		}
		else
		{
			ans = true;
		}
		return ans;
	}
	catch (...)
	{
		return false;
	}
	
}

size_t communication::ports::serial_port_impl::send(const void* buffer, size_t size) const
{
	bool ans = true;
	try 
	{
		boost::system::error_code ec;
		boost::asio::write(m_port, boost::asio::buffer(buffer, size), ec);
		if (ec.value() != 0) 
		{
			ans = false;
		}
		return ans;
	}
	catch (...)
	{
		return false;
	}
}

size_t communication::ports::serial_port_impl::recieve(void* buffer, size_t size, core::communication::communication_error* commError)
{
	boost::system::error_code ec;
	try 
	{
		size_t nTotal = 0;
		size_t nDataleft = size;
		size_t nCurrentRead = 0;

		uint8_t* bytes_arr = static_cast<uint8_t*>(buffer);
		while (nTotal < size)
		{
			nCurrentRead = boost::asio::read(m_port, boost::asio::buffer(&bytes_arr[nTotal], nDataleft), ec);
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

core::communication::communication_error communication::ports::serial_port_impl::ParseError(boost::system::error_code ec)
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