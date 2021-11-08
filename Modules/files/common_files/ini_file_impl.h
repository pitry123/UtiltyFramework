#pragma once

#include <files/ini_file_interface.h>
#include <utils/files.hpp>
#include <utils/ref_count_ptr.hpp>

#include <list>

namespace files
{
    /// @class  ini_file
    ///
    /// @brief  Implementation for INI file.
    ///
    /// @date   10/01/2018
    class ini_file : public utils::files::file_access_base<files::ini_file_interface>
    {
    private:

        /// @enum   ini_entry_type
        ///
        /// @brief  Values that represent INI file entry types
        enum ini_entry_type
        {
            TYPE_NONE,
            TYPE_SECTION,
            TYPE_KEY_VALUE,
            TYPE_COMMENT
        };

        /// @struct ini_entry
        ///
        /// @brief  An INI file entries list node object.
        ///
        /// @date   10/01/2018
        struct ini_entry
        {
            ini_entry_type  entry_type;
            std::string     entry_text;

            bool operator ==(const ini_entry &entry) const
			{
				if (entry_type == entry.entry_type && entry_text == entry.entry_text)
					return true;
				return false;
			}
        };

        /// @brief  List of INI file entries
        std::list<ini_entry> m_file_entry_list;
        
        /// @brief  Mutex for internal data protection
        mutable std::mutex m_data_mutex;

        /// @brief  True if file needs to be flushed
        bool m_need_update;
        
        /// @brief  Base class type definition
        using Super = utils::files::file_access_base<ini_file_interface>;

        /// @brief  Reads file and maps its data into internal entries list
        ///
        /// @return True if it succeeds, false if it fails.
        bool read_file_data();

		/// @brief  Reads the value associated to given section & key.
		///
		/// @param	section The values' section.
		/// @param	key	    The values' key.
		///
		/// @return A string representation of the value. If section & key is
		/// 		not found then the string will be empty
		std::string get_key_value(const char* section, const char* key);

		/// @brief Searches for the section in the list
		///
		/// @param 		   	section Section to find.
		/// @param [in,out]	itr	    If found will contain the iterator pointing
		/// 						to the entry in the list. Otherwise, itr
		/// 						will point to end of list.
		///
		/// @return True if it succeeds, false if it fails.
		bool find_section(const char* section, std::list<ini_entry>::iterator& itr);

		/// @brief Searches for the entry in the list that matches section & key.
		///
		/// @param 		   	section Section to find
		/// @param 		   	key	    Key to find
		/// @param [in,out]	itr	    If found will contain the iterator pointing
		/// 						to the entry in the list. Otherwise, itr
		/// 						will point to end of list.
		///
		/// @return True if found, false otherwise.
		bool find_entry(const char* section, const char* key, std::list<ini_entry>::iterator& itr);

    public:
        /// @brief  Constructor
        ///
        /// @param  file_path   Full pathname of the file.
        ini_file(const char* file_path);

        /// @brief  Destructor
        virtual ~ini_file();

		// Inherited from file_access_interface:

		/// @brief  File flush implementation for file writing logic and format
		///
		/// @return True if it succeeds, false if it fails.
		virtual bool flush() override;

		// Inherited from ini_file_interface:

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

		virtual bool write_bool(const char* section, const char* key, bool value) override;
		virtual bool write_int(const char* section, const char* key, int32_t value) override;
		virtual bool write_float(const char* section, const char* key, float value) override;
		virtual bool write_long(const char* section, const char* key, int64_t value) override;
		virtual bool write_double(const char* section, const char* key, double value) override;
		virtual bool write_string(const char* section, const char* key, const char* value) override;

		/// @brief  This method deletes the specified key from the specified 
		///         section inside INI file.
		///
		/// @param  section Section in the INI file ([section])
		/// @param  key     Key in the section (key=value)
		///
		/// @return True upon success, false if section/key not found
		virtual bool delete_key(const char* section, const char* key) override;

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

		virtual bool read_bool(const char* section, const char* key, bool& value, bool def_val) override;
		virtual bool read_int(const char* section, const char* key, int32_t& value, int32_t def_val) override;
		virtual bool read_float(const char* section, const char* key, float& value, float def_val) override;
		virtual bool read_long(const char* section, const char* key, int64_t& value, int64_t def_val) override;
		virtual bool read_double(const char* section, const char* key, double& value, double def_val) override;
		virtual bool read_string(const char* section, const char* key, char* value, size_t& size, const char* def_val) override;
		virtual bool read_string(size_t index, char* section, char* key, char* value, size_t& max_buffer_size) override;
    };
}
