// file_stream.cpp : Defines the exported functions for the DLL application.
//

#include "file_stream.h"
#include <utils/ref_count_ptr.hpp>

streams::file_stream::file_stream(const char* file_path, 
                                  streams::file_stream_interface::access_mode mode)
                                : m_file_path(file_path),
                                  m_mode(mode)
{
	if (open(file_path, mode) != core::stream_status::status_no_error)
	{
		throw std::runtime_error("failed to open file");
	}
}

streams::file_stream::~file_stream()
{
	m_file.close();
}

core::stream_status streams::file_stream::close()
{
	if (m_file.is_open())
		m_file.close();

	return m_file.is_open() ? core::stream_status::status_close_failed : core::stream_status::status_no_error;
}

core::stream_status streams::file_stream::read_bytes(void* data, size_t number_of_bytes_to_read, size_t& number_of_bytes_read)
{
	number_of_bytes_read = 0;
    m_file.read((char*)data, static_cast<std::streamsize>(number_of_bytes_to_read));

	// The cast below is safe (also for 32bit) since the number_of_bytes_to_read is size_t
    number_of_bytes_read = static_cast<size_t>(m_file.gcount()); // Real number of bytes read

    core::stream_status ret_val = (!m_file.fail() || m_file.eof()) ? core::stream_status::status_no_error : core::stream_status::status_read_failed;
    
    // Reset any error before returning
    if (m_file.eof() || m_file.fail())
        m_file.clear();

	return ret_val;
}

core::stream_status streams::file_stream::write_bytes(const void* data, size_t number_of_bytes_to_write, size_t& number_of_bytes_written)
{
	number_of_bytes_written = 0;
    m_file.write((char*)data, static_cast<std::streamsize>(number_of_bytes_to_write));
	if (m_file)
		number_of_bytes_written = number_of_bytes_to_write;

    core::stream_status ret_val = m_file ? core::stream_status::status_no_error : core::stream_status::status_write_failed;

    // Reset any error before returning
    if (m_file.eof() || m_file.fail())
        m_file.clear();

    return ret_val;
}

core::stream_status streams::file_stream::set_position(int64_t offset, core::stream_interface::relative_position relative_to)
{
	switch (relative_to)
	{
	case core::stream_interface::relative_position::current: m_file.seekp(offset, std::ios::cur); break;
	case core::stream_interface::relative_position::begin: m_file.seekp(offset, std::ios::beg); break;
	case core::stream_interface::relative_position::end: m_file.seekp(offset, std::ios::end); break;
	}

	return m_file ? core::stream_status::status_no_error : core::stream_status::status_read_failed;
}

core::stream_status streams::file_stream::set_position(int64_t offset, core::stream_interface::relative_position relative_to, uint64_t& position)
{
	core::stream_status retval = set_position(offset, relative_to);
	if (retval != core::stream_status::status_no_error)
		return retval;

    position = static_cast<uint64_t>(m_file.tellp());
	return m_file ? core::stream_status::status_no_error : core::stream_status::status_read_failed;
}

core::stream_status streams::file_stream::get_position(uint64_t& position)
{
	position = static_cast<uint64_t>(m_file.tellp());
	return m_file ? core::stream_status::status_no_error : core::stream_status::status_read_failed;
}

core::stream_status streams::file_stream::reset(bool wipe_data)
{
    m_file.close();
    if (wipe_data)
    {
        if (remove(m_file_path.c_str()) != 0)
        {
            throw std::runtime_error("failed to delete file");
        }
    }
    if (open(m_file_path, m_mode) != core::stream_status::status_no_error)
    {
        throw std::runtime_error("failed to open file");
    }

	return m_file ? core::stream_status::status_no_error : core::stream_status::status_read_failed;
}

core::stream_status streams::file_stream::flush()
{
    m_file.flush();

    return m_file ? core::stream_status::status_no_error : core::stream_status::status_read_failed;
}

core::stream_status streams::file_stream::open(const std::string& file_path, streams::file_stream_interface::access_mode mode)
{
    using acc_mode = streams::file_stream_interface::access_mode;
	switch (mode)
	{
	    case acc_mode::read:
            // Will not create file if not exit
		    m_file.open(file_path, std::ios::in | std::ios::binary);
		    break;

	    case acc_mode::write:
            // Will create file if not exist
		    m_file.open(file_path, std::ios::out | std::ios::binary);
		    break;

        case acc_mode::read_write:
            // Make sure file is created if not exist
            if (std::ifstream(file_path).good() == false)
                std::ofstream(file_path).close();

            m_file.open(file_path, std::ios::in | std::ios::out | std::ios::binary);
            break;

        case acc_mode::append:
            // Will create file if not exist
            m_file.open(file_path, std::ios::app | std::ios::binary);
            break;

	    default:  // just to avoid warning
		    break;
	}

	bool is_open = m_file.is_open();
	return is_open == true ? core::stream_status::status_no_error : core::stream_status::status_open_failed;
}

bool streams::file_stream_interface::create(const char* file_path, streams::file_stream_interface::access_mode mode, core::stream_interface** stream)
{
	if (stream == nullptr)
		return false;

	utils::ref_count_ptr<core::stream_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<file_stream>(file_path, mode);
	}
	catch (.../*std::exception& e*/)
	{
		return false;
	}

	instance->add_ref();
	*stream = instance;
	return true;
}
