/// @file	files/ini_file_interface.h.
/// @brief	Declares the INI file class
#pragma once

#include <core/files.h>
#include <stdint.h>
#include <cstddef>

namespace files
{
    /// @class  ini_file_interface
    /// 		
    /// @brief  Interface for INI file implementation.
    ///         Defines API for INI files
    ///         
    /// @date   01/01/2018
    class DLL_EXPORT ini_file_interface : public core::files::file_access_interface
    {
    public:
        /// @brief  Default destructor.
        virtual ~ini_file_interface() = default;

        /// @brief  Static factory: Creates new instance of INI file
        /// 		
        /// @param          file_path   Full pathname of the file.
        /// @param [out]	file        An address of a pointer to ini_file_interface
        /// 				
        /// @return True if it succeeds, false if it fails.
        static bool create(const char* file_path, ini_file_interface** file);

        // INI file API:
        // -------------

        /// @brief  Value Writers:
        ///         These methods writes bool, int, float or string values to 
        ///         'section/key' in the INI file. If section or key is not 
        ///         found method adds the missing data
        ///         
        /// @param  section Section in the INI file ([section])
        /// @param  key     Key in the section (key=value)
        /// @param  value   To attach to the key
        /// 				
        /// @return True upon success (false otherwise)
        
        virtual bool write_bool(const char* section, const char* key, bool value) = 0;
        virtual bool write_int(const char* section, const char* key, int32_t value) = 0;
        virtual bool write_float(const char* section, const char* key, float value) = 0;
        virtual bool write_long(const char* section, const char* key, int64_t value) = 0;
        virtual bool write_double(const char* section, const char* key, double value) = 0;
        virtual bool write_string(const char* section, const char* key, const char* value) = 0;

        /// @brief  This method deletes the specified key from the specified 
        ///         section inside INI file.
        ///         
        /// @param  section Section in the INI file ([section])
        /// @param  key     Key in the section (key=value)
        /// 				
        /// @return True upon success, false if section/key not found
        virtual bool delete_key(const char* section, const char* key) = 0;

        /// @brief  Value Readers:
        ///         These methods reads bool, int, float or string values from 
        ///         'section/key' in the INI file. If section or key is not 
        ///         found method uses the given default value
        ///         
        /// @param          section Section in the INI file ([section])
        /// @param          key     Key in the section (key=value)
        /// @param [in,out] value   To read from key
        /// @param          def_val To use if section/key is not found
        /// 						
        /// @return True upon success, false if default value is used

        virtual bool read_bool(const char* section, const char* key, bool& value, bool def_val) = 0;
        virtual bool read_int(const char* section, const char* key, int32_t& value, int32_t def_val) = 0;
        virtual bool read_float(const char* section, const char* key, float& value, float def_val) = 0;
        virtual bool read_long(const char* section, const char* key, int64_t& value, int64_t def_val) = 0;
        virtual bool read_double(const char* section, const char* key, double& value, double def_val) = 0;
        virtual bool read_string(const char* section, const char* key, char* value, size_t& size, const char* def_val) = 0;
		virtual bool read_string(size_t index, char* o_section, char* o_key, char* o_value, size_t& max_buffer_size) = 0;
    };
}