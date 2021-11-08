#pragma once

#include <files/bin_file_interface.h>
#include <utils/files.hpp>
#include <utils/ref_count_ptr.hpp>
#include <core/buffer_interface.h>


namespace files
{
    /// @class  bin_file
    ///
    /// @brief  Implementation for binary data file.
    ///
    /// @date   02/01/2018
    class bin_file : public utils::files::file_access_base<files::bin_file_interface>
    {
    private:
        utils::ref_count_ptr<core::buffer_interface> m_buffer;
        size_t m_size;
        size_t m_max_file_size;
        bool m_need_update;

        /// @brief  Base class type definition
        using Super = utils::files::file_access_base<bin_file_interface>;

    public:

        /// @brief  Constructor
        ///
        /// @param  file_path       Full pathname of the file.
        /// @param  max_file_size   Maximum size of the file.
        bin_file(const char* file_path, size_t max_file_size);

        /// @brief  Destructor
        virtual ~bin_file();

        // Inherited from file_access_interface:

        /// @brief  File flush implementation for file writing logic and format
        ///
        /// @return True if it succeeds, false if it fails.
        virtual bool flush() override;

        // Inherited from bin_file_interface:

        /// @brief  Write data to file.
        ///         This method re-writes the internal buffer with the given 
        ///         buffer content. The method performs the file-update action 
        ///         through the files handler asynchronously unless file is 
        ///         flushed manually after calling this method.
        ///
        /// @param  buffer  Buffer to write to the file.
        /// @param  size    Buffer size.
        ///                 
        /// @return The number of bytes written to file. This number will be less
        ///         than 'size' only in case of failure or invalid input
        virtual size_t write_file(const void* buffer, size_t size) override;
        
        /// @brief  Reads data from file
        ///         This method reads the contents of the whole file and fills 
        ///         the given buffer with this contents. Read action is synchronous 
        ///         (i.e. performed from the context of the caller thread).
        ///
        /// @param [in,out] buffer  Buffer to fill with the contents of the file.
        /// @param          size    Number of bytes to read.
        ///
        /// @return The number of bytes read from file. This number will be less 
        ///         than 'size' only if read error or EOF occurred. If file is 
        ///         longer than 'size', only 'size' is read from file.
        virtual size_t read_file(void* buffer, size_t size) const override;
    };
}