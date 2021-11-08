#pragma once
#include <communication/protocols/fixed_length_protocol.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <mutex>

namespace communication
{
	namespace protocols
	{
		class fixed_length_protocol_impl : public utils::ref_count_base <communication::protocols::fixed_length_protocol>
		{
		public:
			//--------------------------------------------------------
			//Class constructor
			//--------------------------------------------------------
			fixed_length_protocol_impl(core::communication::client_channel_interface* port, size_t nReceiveTimeout, size_t nLength);


			// Inherited via ref_count_base
			virtual core::communication::communication_status status() const override;
			virtual bool connect() override;
			virtual bool disconnect() override;
			virtual size_t send(const void* buffer, size_t size) const override;
			virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override;

		private:
			utils::ref_count_ptr<core::communication::client_channel_interface> m_port;
			size_t m_nReceiveTimeout;
			size_t m_nLength;
		};
	}	
}

