#include "log_file_impl.h"

#include <utils/buffer_allocator.hpp>

#include <string>
#include <cstring>

files::log_file::log_file(const char* file_path, 
                          bool is_cyclic, 
                          size_t max_buff_size, 
                          size_t max_file_size, 
                          const char* header_data, 
                          uint8_t lines_to_skip)
                  : Super(file_path),
                    m_is_cyclic(is_cyclic),
                    m_max_buff_size(max_buff_size),
                    m_max_file_size(max_file_size),
                    m_header_data(header_data),
                    m_lines_to_skip(lines_to_skip),
                    m_curr_buff(0),
                    m_curr_buff_size(0),
                    m_buff_to_write(-1),
                    m_buff_size_to_write(0),
                    m_is_empty(true)
{
    for (int i=0; i < NUM_OF_BUFFERS; i++)
        m_buffers[i] = utils::make_ref_count_ptr<utils::ref_count_buffer>(max_buff_size);
}

files::log_file::~log_file()
{
    // Flush before destruction
    flush();
}

bool 
files::log_file::flush()
{
    // Get hold of file synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_file_guard(m_file_mutex);

    // Get hold of data synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_data_guard(m_data_mutex);
    
    // Make sure file is created if not exist
    if (std::ifstream(m_file_path).good() == false)
        std::ofstream(m_file_path).close();

    // Open file for reading & writing
    m_file.open(m_file_path, std::fstream::in | std::fstream::out | std::fstream::binary);
    if (m_file.is_open() == false)
        return false;

    // If no buffer is full to write, write the current buffer, but not before switching
    // the buffers.
    if (m_buff_to_write == -1)
        switch_buffers();

    // Reset position of stream to end of current data so I will start writing from
    // there. If file is empty then m_is_empty will be updated.
    if (reposition_to_write() == 0)
        return false;

    // If there is a file head-data to write and file is empty, write it.
    if (m_header_data.size() != 0 && m_is_empty)
    {
        m_file.write(m_header_data.c_str(), static_cast<std::streamsize>(m_header_data.size()));
        if (!m_file)
        {
            // Writing failed
            m_file.close();
            return false;
        }
        m_curr_file_size += m_header_data.size();
        m_is_empty = false;
    }

    // Check total file size. If the current file size plus the buffer
    // length will cross the file size limit and file is not cyclic close
    // the file and return. If file is cyclic restart file before writing
    // the buffer.
    if ((m_curr_file_size + m_buff_size_to_write) >= m_max_file_size)
    {
        if (!m_is_cyclic)
            return false;
            
        m_curr_file_size = restart_file(true);
    }

    // Write the buffer into the opened file
    m_file.write((char*)m_buffers[m_buff_to_write]->data(), static_cast<std::streamsize>(m_buff_size_to_write));
    if (!m_file)
    {
        // Writing failed
        m_file.close();
        return false;
    }
    m_curr_file_size += m_buff_size_to_write;

    set_cyclic_file_curr_pos(m_curr_file_size);
    m_buff_to_write = -1;
    m_buff_size_to_write = 0;

    // Closing file (will also cause FS flush)
    m_file.close(); 
    return true;
}

size_t 
files::log_file::write(const char* buffer, size_t size)
{
    // Validate input
    if (buffer == nullptr || size == 0 || size >= m_max_buff_size)
        return 0;

    // Get hold of data synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_guard(m_data_mutex);

    // Check if we are going to overflow the buffer. Check if the 
    // current buffer size with the new addition overflows
    if ((m_curr_buff_size + size) > m_max_buff_size)
    {
        if (m_buff_to_write != -1)
        {
            // Both buffers are full. One waits to be flushed to FS and 
            // the other one (the current one) is full. Can't write
            // until one of them is emptied. Data will be lost.
            return 0;
        }

        // Switch active buffers and send the full buffer for writing only if
        // write action is allowed (no need to bother files update task if 
        // write is not allowed).
        switch_buffers();

        // TODO: Notify files update task to flush me.
        // ...
    }

    // Copy the given stream into the active buffer
    std::memcpy(m_buffers[m_curr_buff]->data() + m_curr_buff_size, buffer, size);
    m_curr_buff_size += size;
    return size;
}

size_t 
files::log_file::write_line(const char* buffer, size_t size)
{
    // Validate input
    if (buffer == nullptr || size == 0 || size >= m_max_buff_size)
        return 0;

    // Add newline to given data
    std::string mod_buff(buffer);
    mod_buff += '\n';

    // Write Line
    return write(mod_buff.c_str(), mod_buff.size());
}

size_t 
files::log_file::read_file(char* buffer, size_t size, bool skip_header) const
{
    if (buffer == nullptr || size == 0)
        return 0;

    // Get hold of file synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_guard(m_file_mutex);

    // Open file for reading & writing
    m_file.open(m_file_path, std::fstream::in | std::fstream::out | std::fstream::binary);
    if (m_file.is_open() == false)
        return false;

    // Set position of file for read as requested
    restart_file(skip_header);

    // If 'size' >  m_max_file_size, set it to m_max_file_size and read until EOF.
    if (size == m_max_file_size)
        size = m_max_file_size;

    m_file.read(buffer, static_cast<std::streamsize>(size));
	// cast below is safe (also for 32bit) since 'size' is 'size_t'
    size_t bytes_read = static_cast<size_t>(m_file.gcount());
    buffer[bytes_read] = '\0';

    m_file.close();
    return bytes_read;
}

void 
files::log_file::switch_buffers()
{
    // Set Buffer to flush
    m_buff_to_write = m_curr_buff;
    m_buff_size_to_write = m_curr_buff_size;

    // Switch buffer index (increment index modulo NUM_OF_BUFFERS)
	m_curr_buff = static_cast<int8_t>((m_curr_buff + 1) % NUM_OF_BUFFERS);
    m_curr_buff_size = 0;

    // Initialize new current buffer to contain nothing
    std::memset(m_buffers[m_curr_buff]->data(), '\0', m_max_buff_size);
}

size_t 
files::log_file::get_cyclic_file_curr_pos() const
{
    char buffer[FILE_HEADER_LENGTH + 1] = {};

    // Set file pointer to the beginning of the file
    m_file.seekp(0, std::ios::beg);
    
    // Get first line (where file current position is)
    m_file.getline(buffer, FILE_HEADER_LENGTH);

    // To make writing later possible
    if (m_file.eof() || m_file.fail())
        m_file.clear();

    // Return the result file length (read from file)
    return static_cast<std::size_t>(std::atoi(buffer));
}

size_t
files::log_file::set_cyclic_file_curr_pos(size_t pos)
{
    m_file.seekp(0, std::ios::beg);
    char buffer[FILE_HEADER_LENGTH + 1] = {};

#ifdef _WIN32
	uint64_t pos_to_sprint = pos;
    size_t len = sprintf_s(buffer, "%20llu\n", pos_to_sprint);
#else
	const char* format = ((sizeof(void*) == sizeof(uint32_t)) ? "%20llu\n" : "%20lu\n");
    size_t len = static_cast<std::size_t>(sprintf(buffer, format, pos));
#endif

    m_file.write(buffer, static_cast<std::streamsize>(len));
    return m_file ? len : 0;
}

size_t 
files::log_file::reposition_to_write()
{
    size_t length = get_cyclic_file_curr_pos();
    if (length == 0)
    {
        // Empty file
        m_is_empty = true;
        m_curr_file_size = FILE_HEADER_LENGTH;
        return set_cyclic_file_curr_pos(m_curr_file_size);
    }
    else
    {
        // File is not empty
        m_is_empty = false;
        m_file.seekp(static_cast<std::ios::off_type>(length), std::ios::beg);
        m_curr_file_size = length;
        return m_curr_file_size;
    }
}

size_t 
files::log_file::restart_file(bool skip_header) const
{
    std::string str;
    size_t line_count(0), bytes_count(0);

    // Set file pointer to the beginning of the data in the file (according to what
    // user requested.
    // 
    //            +--------------------------------------------------------------------+
    //            |                                                                    |
    //            V                                                                    |
    // Skip the first line of the file (containing the file size) and the defined      |
    // number of lines (if requested to skip header)                                   |
    size_t lines = skip_header ? m_lines_to_skip + 1 : 1; // One for file header-------+
    m_file.seekp(0, std::ios::beg);
    while (line_count < lines)
    {
        std::getline(m_file, str);
        if (m_file.eof() || m_file.fail() || bytes_count > m_max_file_size)
        {
            // Couldn't find the requested number of lines
            m_file.clear();
            return 0;
        }
        bytes_count += str.size() + 1;  // +1 for newline which is excluded by getline
        line_count++;
    }
    m_file.seekp(static_cast<std::ios::off_type>(bytes_count), std::ios::beg);
    return bytes_count;
}

bool 
files::log_file_interface::create(const char* file_path, 
                                  bool is_cyclic, 
                                  size_t max_buff_size,
                                  size_t max_file_size, 
                                  const char* header_data, 
                                  uint8_t lines_to_skip, 
                                  log_file_interface** file)
{
    if (file_path == nullptr || max_file_size == 0 || max_buff_size == 0 || file == nullptr)
        return false;

    utils::ref_count_ptr<log_file_interface> instance;
    try
    {
        instance = utils::make_ref_count_ptr<log_file>(file_path, is_cyclic, max_buff_size, max_file_size, header_data, lines_to_skip);
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
