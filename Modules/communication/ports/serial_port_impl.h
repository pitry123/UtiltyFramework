#pragma once
#include <boost/asio.hpp>
#include <communication/ports/serial_port.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <mutex>

namespace communication
{
	namespace ports
	{
		class serial_port_impl : public utils::ref_count_base <communication::ports::serial_port>
		{
		public:
			serial_port_impl(
				const char* strPortName, 
				uint32_t nBaudRate, 
				communication::ports::serial_port::parity eParity,
				size_t nDataBits, 
				communication::ports::serial_port::stop_bits eStopBits);
			// Inherited via ref_count_base
			virtual core::communication::communication_status status() const override;
			virtual bool connect() override;
			virtual bool disconnect() override;
			virtual size_t send(const void* buffer, size_t size) const override;
			virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override;
		private:
			std::string m_strPortName;
			uint32_t m_nBaudRate;
			communication::ports::serial_port::parity m_eParity;
			size_t m_nDataBits;
			communication::ports::serial_port::stop_bits m_eStopBits;
			core::communication::communication_status m_eStatus;
			boost::asio::io_service m_io_service;
			mutable boost::asio::serial_port m_port;

			core::communication::communication_error ParseError(boost::system::error_code ec);
		};
	}	
}

