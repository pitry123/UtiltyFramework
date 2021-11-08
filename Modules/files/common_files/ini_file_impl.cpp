#include "ini_file_impl.h"

#include <stdexcept>
#include <cstring>
#include <string>
#include <algorithm>

files::ini_file::ini_file(const char* file_path)
                  : Super(file_path),
                    m_need_update(false)
{
    if (read_file_data() == false)
        throw std::runtime_error("Failed reading INI file");
}

files::ini_file::~ini_file()
{
	// Flush before destruction
	flush();
}

bool 
files::ini_file::flush()
{
	if (m_need_update == false)
		return true;    // Nothing to do. Exit gracefully

	// Get hold of file synchronization mutex (will be unlocked when goes out of scope)
	std::lock_guard<std::mutex> sync_file_guard(m_file_mutex);

	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	std::lock_guard<std::mutex> sync_data_guard(m_data_mutex);

	// Open file for reading
	m_file.open(m_file_path, std::fstream::out | std::fstream::binary);
	if (m_file.is_open() == false)
		return false;

	for (ini_file::ini_entry entry : m_file_entry_list)
	{
		std::string str = entry.entry_text + "\n";
        m_file.write(str.c_str(), static_cast<std::streamsize>(str.size()));
	}
	m_file.close();
	m_need_update = false;
	return true;
}

bool 
files::ini_file::write_bool(const char* section, const char* key, bool value)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	std::string str_val = value ? "1" : "0";
	return write_string(section, key, str_val.c_str());
}

bool 
files::ini_file::write_int(const char* section, const char* key, int32_t value)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	std::string str_val = std::to_string(value);
	return write_string(section, key, str_val.c_str());
}

bool 
files::ini_file::write_float(const char* section, const char* key, float value)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	std::string str_val = std::to_string(value);
	return write_string(section, key, str_val.c_str());
}

bool 
files::ini_file::write_long(const char* section, const char* key, int64_t value)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	std::string str_val = std::to_string(value);
	return write_string(section, key, str_val.c_str());
}

bool 
files::ini_file::write_double(const char* section, const char* key, double value)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	std::string str_val = std::to_string(value);
	return write_string(section, key, str_val.c_str());
}

bool 
files::ini_file::write_string(const char* section, const char* key, const char* value)
{
	// Input check
	if (section == nullptr || key == nullptr || value == nullptr)
		return false;

	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	std::lock_guard<std::mutex> sync_data_guard(m_data_mutex);

	// Prepare strings to enter
	std::string key_value = key;
	key_value += "=";
	key_value += value;

	std::string sec = "[";
	sec += section;
	sec += "]";

	std::list<ini_entry>::iterator itr_sec = {};
	std::list<ini_entry>::iterator itr_key = {};
	ini_file::ini_entry entry = {};

	// First check if section exist. If not I'll need to add both section & key-value
	if (find_section(section, itr_sec) == false)
	{
		// First add mock up comment (an empty line)
		entry.entry_type = ini_file::TYPE_COMMENT;
		entry.entry_text = "";
		m_file_entry_list.push_back(entry);

		// section
		entry.entry_type = ini_file::TYPE_SECTION;
		entry.entry_text = sec;
		m_file_entry_list.push_back(entry);
		// key value
		entry.entry_type = ini_file::TYPE_KEY_VALUE;
		entry.entry_text = key_value;
		m_file_entry_list.push_back(entry);
	}
	else
	{
		// Find Section-Key first
		if (find_entry(section, key, itr_key) == false)
		{
			// Key not found. Add key-value to section. Use itr_sec to add
			// the key_value to the right place
			entry.entry_type = ini_file::TYPE_KEY_VALUE;
			entry.entry_text = key_value;
			m_file_entry_list.insert(++itr_sec, entry);
		}
		else
		{
			// Key found. Replace value
			(*itr_key).entry_text = key_value;
		}
	}

	m_need_update = true;
	return true;
}

bool 
files::ini_file::delete_key(const char* section, const char* key)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	std::lock_guard<std::mutex> sync_data_guard(m_data_mutex);

	// Find Section-Key first
	std::list<ini_entry>::iterator itr = {};
	if (find_entry(section, key, itr) == true)
	{
		// Found. Remove from list
		m_file_entry_list.erase(itr);
		m_need_update = true;
		return true;
	}
	return false;	// Key not found
}

bool 
files::ini_file::read_bool(const char* section, const char* key, bool& value, bool def_val)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	std::string val_str = get_key_value(section, key);
	if (val_str.empty())
	{
		value = def_val;
		return false;
	}

	value = std::atoi(val_str.c_str()) ? 1 : 0;
	return true;
}

bool 
files::ini_file::read_int(const char* section, const char* key, int32_t& value, int32_t def_val)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	std::string val_str = get_key_value(section, key);
	if (val_str.empty())
	{
		value = def_val;
		return false;
	}

	value = std::atoi(val_str.c_str());
	return true;
}

bool 
files::ini_file::read_float(const char* section, const char* key, float& value, float def_val)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	std::string val_str = get_key_value(section, key);
	if (val_str.empty())
	{
		value = def_val;
		return false;
	}

	value = (float)std::atof(val_str.c_str());
	return true;
}

bool 
files::ini_file::read_long(const char* section, const char* key, int64_t& value, int64_t def_val)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	std::string val_str = get_key_value(section, key);
	if (val_str.empty())
	{
		value = def_val;
		return false;
	}

	value = std::atoll(val_str.c_str());
	return true;
}

bool 
files::ini_file::read_double(const char* section, const char* key, double& value, double def_val)
{
	// Input check
	if (section == nullptr || key == nullptr)
		return false;

	std::string val_str = get_key_value(section, key);
	if (val_str.empty())
	{
		value = def_val;
		return false;
	}

	value = std::atof(val_str.c_str());
	return true;
}

bool 
files::ini_file::read_string(const char* section, const char* key, char* value, size_t& size, const char* def_val)
{
	// Input check
	if (section == nullptr || key == nullptr || value == nullptr || def_val == nullptr)
		return false;

	std::string val_str = get_key_value(section, key);
	if (val_str.empty())
	{
#ifdef _WIN32
    strncpy_s(value, size, def_val, std::strlen(def_val));
#else
    size_t length = std::strlen(def_val);
    std::memset(value, 0, length + 1);
    // Suppressing FORTIFY's -Werror=stringop-truncation which warns against potential memory issues
    // when using strncpy (def_val may not include null termination)
    // That might just be a false-positive warning.
    // When asigning to std::string FORTIFY assumes std::string::c_str() always return a null terminated buffer
    std::string def_val_str(def_val);
    std::strncpy(value, def_val_str.c_str(), length);
#endif
		return false;
	}

#ifdef _WIN32
    strncpy_s(value, size, val_str.c_str(), val_str.size());
#else
    size_t length = val_str.size();
    std::memset(value, 0, length + 1);
    std::strncpy(value, val_str.c_str(), length);
#endif
	return true;
}



bool 
files::ini_file::read_string(size_t index, char* section, char* key, char* value, size_t& max_buffer_size)
{
	// Input check
	if (section == nullptr || key == nullptr || value == nullptr)
		return false;
	
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	std::lock_guard<std::mutex> sync_data_guard(m_data_mutex);

	//iterate on m_file_entry_list till getting the entry with "index"
	size_t i = 0;
	bool found(false);	
	for (auto itr : m_file_entry_list)
	{		
		if (itr.entry_type == TYPE_SECTION)
		{			
			itr.entry_text.erase(std::remove(itr.entry_text.begin(), itr.entry_text.end(), '['), itr.entry_text.end());
			itr.entry_text.erase(std::remove(itr.entry_text.begin(), itr.entry_text.end(), ']'), itr.entry_text.end());
		
#ifdef _WIN32
			strncpy_s(section, max_buffer_size, itr.entry_text.c_str(), itr.entry_text.size());
#else
			size_t length = itr.entry_text.size();
            std::memset(section, 0, length + 1);
            std::strncpy(section, itr.entry_text.c_str(), length);
#endif
		}

		if (itr.entry_type == TYPE_KEY_VALUE)
		{
			if (i == index)
			{
				std::string::size_type pos = itr.entry_text.find("=");
				std::string k = itr.entry_text.substr(0, pos);
				std::string val = itr.entry_text.substr(pos + 1);

#ifdef _WIN32
				strncpy_s(key, max_buffer_size, k.c_str(), k.size());
				strncpy_s(value, max_buffer_size, val.c_str(), val.size());
#else
                size_t length = k.size();
				std::memset(key, 0, length + 1);
                std::strncpy(key, k.c_str(), length);

                length = val.size();
				std::memset(value, 0, length + 1);
                std::strncpy(value, val.c_str(), length);
#endif			

				found = true;
				break;
			}

			i++;
		}
	}

    if (found == false || section[0] == '\0' || key[0] == '\0' || value[0] == '\0')
		return false;

	return true;
}




bool 
files::ini_file::read_file_data()
{
    // Get hold of file synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_file_guard(m_file_mutex);

    // Get hold of data synchronization mutex (will be unlocked when goes out of scope)
    std::lock_guard<std::mutex> sync_data_guard(m_data_mutex);

	// Open file for reading
	m_file.open(m_file_path, std::fstream::in | std::fstream::binary);
	if (m_file.is_open() == false)
		return false;

    // Make sure we start reading file from the beginning
    m_file.seekp(0, std::ios::beg);

    // Make sure list is empty
    m_file_entry_list.clear();

	std::string::size_type pos;
    std::string line;
   
	while (!m_file.eof())
	{
		std::getline(m_file, line);
		// Get rid of CR character
		pos = line.find('\r');
		if (pos != std::string::npos)
			line.erase(pos);

		// Make new entry
		ini_file::ini_entry new_entry;

		// Copy line text into new entry
		new_entry.entry_text = line;

		// Search for ';' (comment beginning) and get string up to that position
		pos = line.find(';');
		line = line.substr(0, pos);

		// Now determine what kind of data this is : Section, Key-Value or Comment
		if (line.find('[') != std::string::npos && line.find(']') != std::string::npos)
		{
			new_entry.entry_type = ini_file::TYPE_SECTION;
		}
		else
		{
			if (line.find('=') != std::string::npos)
			{
				new_entry.entry_type = ini_file::TYPE_KEY_VALUE;
			}
			else
			{
				new_entry.entry_type = ini_file::TYPE_COMMENT;
			}
		}
		m_file_entry_list.push_back(new_entry);
	}
	m_file.close();
    return true;
}

std::string 
files::ini_file::get_key_value(const char* section, const char* key)
{
	std::string ret_val;

	// Input check
	if (section == nullptr || key == nullptr)
		return ret_val;
	
	// Get hold of data synchronization mutex (will be unlocked when goes out of scope)
	std::lock_guard<std::mutex> sync_data_guard(m_data_mutex);

	// Find Section-Key first
	std::list<ini_entry>::iterator itr = {};
	if (find_entry(section, key, itr) == true)
	{
		// Found. Extract the value.
		ini_file::ini_entry entry = *itr;
		size_t pos = entry.entry_text.find('=');
		ret_val = entry.entry_text.c_str() + pos + 1;
	}
	return ret_val;	// If key was found, this string will not be empty
}

bool 
files::ini_file::find_section(const char* section, std::list<ini_entry>::iterator& itr)
{
	// First find the section
	ini_file::ini_entry sec_entry;
	sec_entry.entry_type = ini_file::TYPE_SECTION;
	sec_entry.entry_text = "[";
	sec_entry.entry_text += section;
	sec_entry.entry_text += "]";
	itr = std::find(m_file_entry_list.begin(), m_file_entry_list.end(), sec_entry);
	if (itr == m_file_entry_list.end())
		// Section not found
		return false;
	return true;
}

bool 
files::ini_file::find_entry(const char* section, 
							const char* key, 
							std::list<ini_entry>::iterator& itr)
{
	// First find the section
	if (find_section(section, itr) == false)
		return false;

	// Now search for the key until I encounter the next section or the end of the list
	std::string keyeq = key;
	keyeq += "=";
	++itr;
	for (; itr != m_file_entry_list.end(); ++itr)
	{
		ini_file::ini_entry entry = *itr;
		if (entry.entry_type == ini_file::TYPE_SECTION)
			return false;	// still empty string

		if (entry.entry_type == ini_file::TYPE_KEY_VALUE)
		{
			if (entry.entry_text.find(keyeq) != std::string::npos)
			{
				// Found the key. Iterator already there
				return true;
			}
		}
	}
	return false;	// Key not found
}

bool
files::ini_file_interface::create(const char* file_path,
								  ini_file_interface** file)
{
	if (file_path == nullptr || file == nullptr)
		return false;

	utils::ref_count_ptr<ini_file_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<ini_file>(file_path);
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

