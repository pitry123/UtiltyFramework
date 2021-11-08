#pragma once
#include <communication/protocols/delimiter_protocol.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <string>
#include <mutex>
#include <vector>

namespace communication
{
	namespace protocols
	{
		class delimiters
		{
					
		public:
			std::string m_start_delimiter;
			std::string m_end_delimiter;

			delimiters(const delimiter_couple& couple) :
				m_start_delimiter(couple.strStartDelimiter),
				m_end_delimiter(couple.strEndDelimiter)
			{
			}

			delimiters(size_t start_length, size_t end_length)
			{
				if (start_length == 0)
					throw std::invalid_argument("start_length");

				if (end_length == 0)
					throw std::invalid_argument("end_length");

				m_start_delimiter.resize(start_length, ' ');
				m_end_delimiter.resize(end_length, ' ');
			}
		};

		class delimiter_protocol_impl : public utils::ref_count_base <communication::protocols::delimiter_protocol>
		{
		public:
			//--------------------------------------------------------
			//Class constructor
			//--------------------------------------------------------
			delimiter_protocol_impl(core::communication::client_channel_interface* port, const delimiter_couple* array_delimiter_couples, size_t couples_count, size_t nMaxMsgSize);


			// Inherited via ref_count_base
			virtual core::communication::communication_status status() const override;
			virtual bool connect() override;
			virtual bool disconnect() override;
			virtual size_t send(const void* buffer, size_t size) const override;
			virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override;

		private:
			utils::ref_count_ptr<core::communication::client_channel_interface> m_port;
			std::vector<delimiters> m_delimiters;
			std::vector<delimiters> m_temp_delimiters;
            size_t m_nMaxMsgSize;
		};
	}	
}

