#pragma once
#include <streams/file_stream_interface.h>
#include <utils/ref_count_base.hpp>

#include <fstream>

namespace streams
{
	class file_stream : public utils::ref_count_base<streams::file_stream_interface>
	{
	private:
		std::fstream                                m_file;
        std::string                                 m_file_path;
        streams::file_stream_interface::access_mode m_mode;
		virtual core::stream_status open(const std::string& file_path, streams::file_stream_interface::access_mode mode);

	public:
		file_stream(const char* file_path, streams::file_stream::access_mode mode);
		virtual ~file_stream();

		virtual core::stream_status close();
		virtual core::stream_status read_bytes(void* data, size_t number_of_bytes_to_read, size_t& number_of_bytes_read) override;
		virtual core::stream_status write_bytes(const void* data, size_t number_of_bytes_to_write, size_t& number_of_bytes_written) override;
		virtual core::stream_status set_position(int64_t offset, core::stream_interface::relative_position relative_to) override;
		virtual core::stream_status set_position(int64_t offset, core::stream_interface::relative_position relative_to, uint64_t& position) override;
		virtual core::stream_status get_position(uint64_t& position) override;
		virtual core::stream_status reset(bool wipe_data) override;
        virtual core::stream_status flush() override;
	};
}

