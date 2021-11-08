#pragma once

#include <files/log_file_interface.h>
#include <utils/files.hpp>
#include <utils/ref_count_ptr.hpp>
#include <core/buffer_interface.h>

namespace files
{
    /// @class  log_file
    ///
    /// @brief  Implementation for log file.
    ///
    /// @date   04/01/2018
    class log_file : public utils::files::file_access_base<files::log_file_interface>
    {
    private:
        enum constants_enum
        {
            FILE_HEADER_LENGTH = 21,    // To match length in characters
            NUM_OF_BUFFERS = 2
        };

        /// @brief The internal buffers of the log file (double buffering)
        utils::ref_count_ptr<core::buffer_interface>    m_buffers[NUM_OF_BUFFERS];
        /// @brief Indicates if file is cyclic or not
        bool                                            m_is_cyclic;
        /// @brief The maximum size of each buffer in the buffers array (all of the same size)
        size_t                                          m_max_buff_size;
        /// @brief The maximum size of the file
        size_t                                          m_max_file_size;
        /// @brief A data to write after the file header (which contains current file size)
        std::string                                     m_header_data;
        /// @brief Number of lines to skip when restarting a cyclic file.
        size_t                                          m_lines_to_skip;
        /// @brief The index of the current active buffer
        int8_t                                          m_curr_buff;
        /// @brief The current size of the current active buffer
        size_t                                          m_curr_buff_size;
        /// @brief The index of the buffer that needs to be written to the file
        int8_t                                          m_buff_to_write;
        /// @brief The size of the full buffer that needs to be written to the file
        size_t                                          m_buff_size_to_write;
        /// @brief  Current size of the file (cyclic point)
        size_t                                          m_curr_file_size;
        /// @brief Indicates if file is empty or not.
        bool                                            m_is_empty;

        /// @brief Mutex for internal data protection
        mutable std::mutex                              m_data_mutex;

        /// @brief  Base class type definition
        using Super = utils::files::file_access_base<log_file_interface>;

        /// @brief  This method switches the current internal active buffer 
        ///         (double buffering mechanism).
        void switch_buffers();

        /// @brief  Gets current writing position of cyclic file.
        ///         This information is written in the first line of the file
        ///
        /// @return Current position for writing
        size_t get_cyclic_file_curr_pos() const;

        /// @brief  Sets current writing position of cyclic file.
        ///         This information is written in the first line of the file
        ///
        /// @return Current position for writing
        size_t set_cyclic_file_curr_pos(size_t pos);

        /// @brief  Reposition file pointer to writing position
        ///
        /// @return Current position for writing
        size_t reposition_to_write();

        /// @brief  This method resets the file pointer to the beginning of 
        ///         the file. This is relevant only for cyclic files. When 
        ///         reaching the maximum size of the file, in cyclic files, 
        ///         the file pointer is reset to the beginning of the file.
        size_t restart_file(bool skip_header) const;

    public:

        /// @brief  Constructor
        ///
        /// @param  file_path       Full pathname of the file.
        /// @param  is_cyclic       True if this object is cyclic.
        /// @param  max_buff_size   Maximum size of internal buffers.
        /// @param  max_file_size   Maximum size of the file.
        /// @param  header_data     File header information.
        /// @param  lines_to_skip   Number of lines to skip from start.
        log_file(const char* file_path,
                 bool is_cyclic,
                 size_t max_buff_size,
                 size_t max_file_size,
                 const char* header_data,
                 uint8_t lines_to_skip);

        /// @brief  Destructor
        virtual ~log_file();

        // Inherited from file_access_interface:
        
        /// @brief  File flush implementation for file writing logic and format
        ///
        /// @return True if it succeeds, false if it fails.
        virtual bool flush() override;

        // Inherited from log_file_interface:

        /// @brief  Writes data to the file.
        ///         This method writes, from the given buffer, 'size' bytes to 
        ///         the internal file buffer.
        ///         Content of the internal buffer is written to FS only within 
        ///         the scope of the files handler thread
        ///
        /// @param  buffer  Data to write.
        /// @param  size    Data size.
        ///
        /// @return The number of bytes successfully written, which will be less 
        ///         than the given length only if a write error is encountered
        virtual size_t write(const char* buffer, size_t size) override;

        /// @brief  Writes a line to the file
        ///         This method writes, from the given buffer, a line to the 
        ///         internal file buffer. If the buffer is shorter than the 
        ///         given line size, it fills the rest of the line with spaces.
        ///         If the given buffer is longer than the given line size, 
        ///         it will shorten the buffer to the given length.
        ///         Method adds newline character to the end of the line. Other 
        ///         newline characters inside the buffer will stay there.
        ///
        /// @param  buffer  Data to write.
        /// @param  size    Data size.
        ///
        /// @return The number of bytes successfully written which will be less 
        ///         than the given length only if a write error is encountered.
        virtual size_t write_line(const char* buffer, size_t size) override;

        /// @brief  Reads from the file
        ///         This method reads the whole log file - from beginning of 
        ///         data to end (and not from SOF to EOF) - into the given 
        ///         buffer. Read action is synchronous (i.e. performed from 
        ///         the context of the caller thread).
        ///
        /// @param [in,out] buffer      Buffer to contain the file contents.
        /// @param          size        Maximum bytes to read from file.
        /// @param          skip_header True if file header is not needed.
        ///
        /// @return Returns the number of bytes read from the file.
        virtual size_t read_file(char* buffer, size_t size, bool skip_header) const override;
    };
}