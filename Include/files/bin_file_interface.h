/// @file	files/bin_file_interface.h.
/// @brief	Declares the binary file class
#pragma once

#include <core/files.h>
#include <stdint.h>
#include <cstddef>

namespace files
{
    /// @class  bin_file_interface
    /// 		
    /// @brief  Interface for binary data file implementation.
    ///         Defines API for Binary data files
    ///         
    /// @date   01/01/2018
    class DLL_EXPORT bin_file_interface : public core::files::file_access_interface
    {
    public:
        /// @brief  Default destructor
        virtual ~bin_file_interface() = default;

        /// @brief  Static factory: Creates new binary file instance
        /// @param          file_path       Full pathname of the file.
        /// @param          max_file_size   Maximum size of the file.
        /// @param [out]	file            An address of a pointer to bin_file_interface
        /// @return True if it succeeds, false if it fails.
        static bool create(const char* file_path, size_t max_file_size, bin_file_interface** file);

        // Binary data file API:
        // ---------------------

        /// @brief  Write data to file.
        ///         This method re-writes the internal buffer with the given 
        ///         buffer content. The method performs the file-update action 
        ///         through the files handler asynchronously unless file is 
        ///         flushed manually after calling this method.
        /// @param  buffer  Buffer to write to the file.
        /// @param  size    Buffer size.                 
        /// @return The number of bytes written to file. This number will be less
        ///         than 'size' only in case of failure or invalid input
        virtual size_t write_file(const void* buffer, size_t size) = 0;
        

        /// @brief  Reads data from file
        ///         This method reads the contents of the whole file and fills 
        ///         the given buffer with this contents. Read action is synchronous 
        ///         (i.e. performed from the context of the caller thread).
        /// @param [in]		buffer  Buffer to fill with the contents of the file.
        /// @param          size    Number of bytes to read.
        /// @return The number of bytes read from file. This number will be less 
        ///         than 'size' only if read error or EOF occurred. If file is 
        ///         longer than 'size', only 'size' is read from file.
        virtual size_t read_file(void* buffer, size_t size) const = 0;
    };
}