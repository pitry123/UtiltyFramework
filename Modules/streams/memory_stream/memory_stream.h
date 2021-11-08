#pragma once
#include <streams/memory_stream_interface.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <mutex>

namespace streams
{
	class memory_stream : public utils::ref_count_base<streams::memory_stream_interface>
	{
	private:
		utils::ref_count_ptr<core::buffer_interface> m_buffer;
		uint64_t m_position;
		std::mutex m_mutex;

	public:
		memory_stream(size_t size);
		memory_stream(core::buffer_interface* buffer);

		virtual core::stream_status read_bytes(void* data, size_t number_of_bytes_to_read, size_t& number_of_bytes_read) override;
		virtual core::stream_status write_bytes(const void* data, size_t number_of_bytes_to_write, size_t& number_of_bytes_written) override;
		virtual core::stream_status set_position(int64_t offset, core::stream_interface::relative_position relative_to) override;
		virtual core::stream_status set_position(int64_t offset, core::stream_interface::relative_position relative_to, uint64_t& position) override;
		virtual core::stream_status get_position(uint64_t& position) override;
		virtual core::stream_status reset(bool wipe_data) override;
        virtual core::stream_status flush() override;
	};
}

