#pragma once
#include <communication/protocols/variable_length_protocol.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <mutex>

namespace communication
{
	namespace protocols
	{
		class variable_length_protocol_impl : public utils::ref_count_base <communication::protocols::variable_length_protocol>
		{
		public:
			enum SizeEnum
			{
				ByteSize8Bit=1,
				ShortSize16Bit=2,
				IntSize32Bit=4
			};
			//--------------------------------------------------------
			//Class constructor
			//--------------------------------------------------------
			variable_length_protocol_impl(
				core::communication::client_channel_interface* port, 
				size_t nSizeOfDataSize, 
				size_t nPlaceOfDataSize,
				size_t nConstatSizeAddition,
				bool bAbsolute_data_size,
				size_t nMaxDataSize,
				core::types::endian eEndian);


			// Inherited via ref_count_base
			virtual core::communication::communication_status status() const override;
			virtual bool connect() override;
			virtual bool disconnect() override;
			virtual size_t send(const void* buffer, size_t size) const override;
			virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override;

		private:			
			utils::ref_count_ptr<core::communication::client_channel_interface> m_port;
			size_t m_nSizeOfDataSize;
			size_t m_nPlaceOfDataSize;
			size_t m_nConstatSizeAddition;
			bool m_bAbsolute_data_size;
			size_t m_nMaxDataSize;
			bool m_bEndianMismatch;			
		};
	}	
}

