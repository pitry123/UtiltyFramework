#include "bin_file_impl.h"

#include <utils/buffer_allocator.hpp>
#include <cstring>


files::bin_file::bin_file(const char* file_path, 
                          size_t max_file_size)
                  : Super(file_path),
                    m_buffer(utils::make_ref_count_ptr<utils::ref_count_buffer>(max_file_size)),
                    m_size(0),
                    m_max_file_size(max_file_size),
                    m_need_update(false)
{
    // Zero internal buffer
    std::memset(m_buffer->data(), '\0', m_max_file_size);
}

files::bin_file::~bin_file()
{
    // Flush before destruction
    flush();
}

bool 
files::bin_file::flush()
{
    if (m_need_update == false)
        return true;    // Nothing to do. Exit gracefully

    // Get hold of the synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_guard(m_file_mutex);

    // Open file for writing (will also wipe it clean)
    m_file.open(m_file_path, std::fstream::out | std::fstream::binary);
    if (m_file.is_open() == false)
        return false;

    // Write to file
    m_file.write((const char*)m_buffer->data(), static_cast<std::streamsize>(m_size));
    if (!m_file)
    {
        m_file.close();
        return false;
    }

    // Zero internal state
    std::memset(m_buffer->data(), '\0', m_max_file_size);
    m_size = 0;
    m_need_update = false;

    // Closing file (will also cause FS flush)
    m_file.close();
    return true;
}

size_t
files::bin_file::write_file(const void* buffer, size_t size)
{
    if (size == 0 || buffer == nullptr)
        return false;

    if (size > m_max_file_size)
        size = m_max_file_size;     // Write only as long as maximum
                                    // file size.

    // Get hold of the synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_guard(m_file_mutex);

    std::memcpy(m_buffer->data(), buffer, size);
    m_size = size;
    m_need_update = true;
    return true;
}

size_t 
files::bin_file::read_file(void* buffer, size_t size) const
{
    if (size == 0 || buffer == nullptr)
        return 0;

    if (size > m_max_file_size)
        size = m_max_file_size;     // Read only as long as maximum
                                    // file size.

    // Get hold of the synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_guard(m_file_mutex);

    // Open file for reading
    m_file.open(m_file_path, std::fstream::in | std::fstream::binary);
    if (m_file.is_open() == false)
        return false;

    // Set position in file stream to the beginning
    m_file.seekp(0, std::ios::beg);

    // Read from file
    m_file.read((char*)buffer, static_cast<std::streamsize>(size));
	// cast below is safe (also for 32bit) since 'size' is 'size_t'
    size_t num_of_bytes_read = static_cast<size_t>(m_file.gcount());

    // Return number of bytes read
    m_file.close();
    return num_of_bytes_read;
}

// Interface factory method:
bool
files::bin_file_interface::create(const char* file_path, size_t max_file_size, bin_file_interface** file)
{
    if (file_path == nullptr || max_file_size == 0 || file == nullptr)
        return false;

    utils::ref_count_ptr<bin_file_interface> instance;
    try
    {
        instance = utils::make_ref_count_ptr<bin_file>(file_path, max_file_size);
    }
    catch (...)
    {
        return false;
    }

    if (instance == nullptr)
        return false;

    instance->add_ref();
    *file = instance;
    return true;
}
