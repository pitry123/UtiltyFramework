/// @file	files/log_file_interface.h.
/// @brief	Declares the log file class
#pragma once

#include <core/files.h>
#include <stdint.h>
#include <cstddef>

namespace files
{
    /// @class  log_file_interface
    /// 		
    /// @brief Interface for log file implementation.
    ///        Defines API for Log files
    ///        
    /// @date   01/01/2018
    class DLL_EXPORT log_file_interface : public core::files::file_access_interface
    {
    public:
        /// @brief  Default destructor
        virtual ~log_file_interface() = default;

        /// @brief  Static factory: Creates new log file instance
        /// 		
        /// @param          file_path       Full pathname of the file.
        /// @param          is_cyclic       True if this file is cyclic.
        /// @param          max_buff_size   Maximum size of internal buffer.
        /// @param          max_file_size   Maximum size of file.
        /// @param          header_data     File header information
        /// @param          lines_to_skip   Number of lines to skip from start
        /// @param [out]	file            An address of a pointer to log_file_interface
        /// 				
        /// @return True if it succeeds, false if it fails.
        static bool create(const char* file_path, 
                           bool is_cyclic, 
                           size_t max_buff_size, 
                           size_t max_file_size, 
                           const char* header_data,
                           uint8_t lines_to_skip,
                           log_file_interface** file);

        // Log file API:
        // -------------

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
        virtual size_t write(const char* buffer, size_t size) = 0;
        
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
        virtual size_t write_line(const char* buffer, size_t size) = 0;
        
        /// @brief  Reads from the file
        ///         This method reads the whole log file - from beginning of 
        ///         data to end (and not from SOF to EOF) - into the given 
        ///         buffer. Read action is synchronous (i.e. performed from 
        ///         the context of the caller thread).
        ///         
        /// @param [in]		buffer      Buffer to contain the file contents.
        /// @param          size        Maximum bytes to read from file.
        /// @param          skip_header True if file header is not needed.
        /// 							
        /// @return Returns the number of bytes read from the file.
        virtual size_t read_file(char* buffer, size_t size, bool skip_header) const = 0;
    };
}