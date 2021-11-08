#pragma once
#include <cstdint>
#include <cstring>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <vector>
#include <regex>
#include <algorithm>

#include <core/communication.h>
#include <core/framework.h>
#include <core/guid.h>
#include <core/video.h>
#include <core/database.h>
#include <utils/ref_count_ptr.hpp>
#include <core/buffer_interface.h>

#ifdef VXWORKS
#undef ERROR
#endif

template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const core::communication::ip_address& address)
{
	if (address.type == core::communication::ip_address_type::IP_VER4)
	{
		return (os << static_cast<int>(address.val[0]) << "." << static_cast<int>(address.val[1]) << "." << static_cast<int>(address.val[2]) << "." << static_cast<int>(address.val[3]));
	}
	else if (address.type == core::communication::ip_address_type::IP_VER6)
	{
		static constexpr size_t MAX_IPV6_STR_LENGTH = 256;

		char dest[MAX_IPV6_STR_LENGTH];
		size_t n = 0, b = 0, z = 0;
		while (n < MAX_IPV6_STR_LENGTH && b < 16)
		{
			if (address.val[b] == 0 && address.val[b + 1] == 0 && z == 0)
			{
				do b += 2; while (b < 16 && address.val[b] == 0 && address.val[b + 1] == 0);

#ifdef _WIN32
                n += sprintf_s(dest + n, MAX_IPV6_STR_LENGTH - n, ":%s", b < 16 ? "" : ":");
#else
                n += static_cast<size_t>(std::sprintf(dest + n, ":%s", b < 16 ? "" : ":"));
#endif
                ++z;
			}
			else
			{
#ifdef _WIN32
				n += sprintf_s(dest + n, MAX_IPV6_STR_LENGTH - n, "%s%x", b ? ":" : "",
					(static_cast<uint32_t>(address.val[b]) << 8) | address.val[b + 1]);
#else
                n += static_cast<size_t>(std::sprintf(dest + n, "%s%x", b ? ":" : "",
                    (static_cast<uint32_t>(address.val[b]) << 8) | address.val[b + 1]));
#endif				
				b += 2;
			}
		}

		if (address.scope_id > 0)
		{
#ifdef _WIN32
            n += sprintf_s(dest + n, MAX_IPV6_STR_LENGTH - n, "%%%lu", static_cast<unsigned long>(address.scope_id));
#else
            n += static_cast<size_t>(std::sprintf(dest + n, "%%%lu", static_cast<unsigned long>(address.scope_id)));
#endif			
		}

		os << dest;	
		return os;
	}
	
	return (os << "UNDEFINED");
}

template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const core::communication::ip_endpoint& end_point)
{
	return (os << end_point.address << ":" << end_point.port);
}

template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const core::framework::version_struct& ver)
{
	return (os << ver.major << "." << ver.minor << "." << ver.patch);
}

template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const core::logging::severity& severity)
{
	switch (severity)
	{
	case core::logging::severity::TRACE:
		os << "TRACE";
		break;
	case core::logging::severity::DEBUG:
		os << "DEBUG";
		break;
	case core::logging::severity::INFO:
		os << "INFO";
		break;
	case core::logging::severity::WARNING:
		os << "WARNINIG";
		break;
	case core::logging::severity::ERROR:
		os << "ERROR";
		break;
	case core::logging::severity::FATAL:
		os << "FATAL";
		break;
	default:
		os << "UNDEFINED";
		break;
	}

	return os;
}

template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const core::guid& id)
{
    std::ostringstream oss;
    oss << "{0x" << std::hex << std::setw(8) << std::setfill('0') << id.data1
        << ", 0x" << std::hex << std::setw(4) << std::setfill('0') << id.data2
        << ", 0x" << std::hex << std::setw(4) << std::setfill('0') << id.data3
        << ", {";

    for (int i = 0; i < 8; i++)
    {
        if (i > 0)
        {
            oss << ", ";
        }

        oss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(id.data4[i]);
    }
    oss << "}}";
    os << oss.str();
    return os;
}

template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const core::imaging::pixel_format& format)
{
	switch (format)
	{
	case core::imaging::pixel_format::RGB:
		os << "RGB";
		break;
	case core::imaging::pixel_format::RGBA:
		os << "RGBA";
		break;
	case core::imaging::pixel_format::BGR:
		os << "BGR";
		break;
	case core::imaging::pixel_format::BGRA:
		os << "BGRA";
		break;
	case core::imaging::pixel_format::I420:
		os << "I420";
		break;
	case core::imaging::pixel_format::NV12:
		os << "NV12";
		break;
	case core::imaging::pixel_format::YUY2:
		os << "YUY2";
		break;
    case core::imaging::pixel_format::UYVY:
        os << "UYVY";
        break;
	case core::imaging::pixel_format::GRAY8:
		os << "GRAY8";
		break;
	case core::imaging::pixel_format::GRAY16_LE:
		os << "GRAY16_LE";
		break;
	default:
		os << "UNDEFINED";
		break;
	}

	return os;
}

template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const core::video::interlace_mode& mode)
{
	switch (mode)
	{
	case core::video::interlace_mode::PROGRESSIVE:
		os << "progressive";
		break;
	case core::video::interlace_mode::INTERLEAVED:
		os << "interleaved";
		break;
	case core::video::interlace_mode::MIXED:
		os << "mixed";
		break;
	case core::video::interlace_mode::FIELDS:
		os << "fields";
		break;
	default:
		os << "UNDEFINED";
		break;
	}

	return os;
}

template<typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const core::database::key& key)
{
	if (key.length > 0)
	{
		for (size_t i = 0; i < key.length; i++)
			os << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(key.data[i]);
	}

	return os;
}

template<typename CharT, typename T>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const std::vector<T>& vec)
{
    if (vec.empty() == false)
    {
        std::copy(vec.begin(), vec.end() - 1,
                  std::ostream_iterator<T>(os, ","));
        os << vec.back();
    }
    return os;
}

inline std::vector<std::string> split_string(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos)
			pos = str.length();

		std::string token = str.substr(prev, pos - prev);
		if (token.empty() == false)
			tokens.push_back(token);

		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());

	return tokens;
}

inline std::vector<std::string> split_string_with_quote(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	std::string token;
	size_t prev = 0, pos = 0, factor = 0;;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos)
			pos = str.length();
		
		if (str[prev] == '\"')
		{
			pos = str.find_first_of("\"", prev + 1);
			if (pos == std::string::npos)
				break; //Something is wrong in the string here break the loop and return only with what make sense
			prev = prev+1;
			factor = 1; //ignore the quotes
		}
		token = str.substr(prev, pos - prev);
		tokens.push_back(token);

		prev = pos + delim.length() + factor;
		factor = 0;
	} while (pos < str.length() && prev < str.length());

	return tokens;
}

inline bool parse(const char* str, uint32_t& val)
{
	try
	{
		int v = std::stoi(str);
		if (v < 0)
			throw std::invalid_argument("v");

		val = static_cast<uint32_t>(v);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

inline bool parse(const char* str, float& val)
{
	try
	{
		val = std::stof(str);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

inline bool parse(const char* str, core::imaging::pixel_format& format)
{
	bool retval = false;
	std::string formatU(str);
	auto conversion_func = [](int val) -> char { return static_cast<char>(::toupper(val)); };
	std::transform(formatU.begin(), formatU.end(), formatU.begin(), conversion_func);

	for (int i = 0; i <= core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT; i++)
	{
		core::imaging::pixel_format current_format = static_cast<core::imaging::pixel_format>(i);

		std::stringstream input;
		input << current_format;

		if (std::strcmp(formatU.c_str(), input.str().c_str()) == 0)
		{
			format = current_format;
			retval = true;
			break;
		}
	}

	return retval;
}

inline bool parse(const char* str, core::video::framerate& framerate)
{
	std::regex pattern("((\\d)+/(\\d)+)");
	std::string framerate_str(str);
	std::smatch pattern_match;

	std::regex numerator_pattern("((\\d)+)/");
	std::smatch numerator_pattern_match;

	std::regex denominator_pattern("/((\\d)+)");
	std::smatch denominator_pattern_match;

	if (std::regex_match(framerate_str, pattern_match, pattern) == false ||
		std::regex_search(framerate_str, numerator_pattern_match, numerator_pattern) == false ||
		std::regex_search(framerate_str, denominator_pattern_match, denominator_pattern) == false)
		return false;

	std::string numerator_pattern_str = numerator_pattern_match[0].str();
	std::string denominator_pattern_str = denominator_pattern_match[0].str();

	std::regex number_pattern("(\\d)+");
	std::smatch numerator_match;
	std::smatch denominator_match;

	if (std::regex_search(numerator_pattern_str, numerator_match, number_pattern) == false ||
		std::regex_search(denominator_pattern_str, denominator_match, number_pattern) == false ||
		parse(numerator_match[0].str().c_str(), framerate.numerator) == false ||
		parse(denominator_match[0].str().c_str(), framerate.denominator) == false)
		return false;

	return true;
}

inline bool parse(const char* str, core::logging::severity& severity)
{
	if (str == nullptr)
		return false;

	std::string severity_str(str);
	auto conversion_func = [](int val) -> char { return static_cast<char>(::toupper(val));	};
	std::transform(severity_str.begin(), severity_str.end(), severity_str.begin(), conversion_func);

	if (std::strcmp(severity_str.c_str(), "TRACE") == 0)
	{
		severity = core::logging::severity::TRACE;
	}
	else if (std::strcmp(severity_str.c_str(), "DEBUG") == 0)
	{
		severity = core::logging::severity::DEBUG;
	}
	else if (std::strcmp(severity_str.c_str(), "INFO") == 0)
	{
		severity = core::logging::severity::INFO;
	}
	else if (std::strcmp(severity_str.c_str(), "WARNING") == 0)
	{
		severity = core::logging::severity::WARNING;
	}
	else if (std::strcmp(severity_str.c_str(), "ERROR") == 0)
	{
		severity = core::logging::severity::ERROR;
	}
	else if (std::strcmp(severity_str.c_str(), "FATAL") == 0)
	{
		severity = core::logging::severity::FATAL;
	}
	else
	{
		return false;
	}

	return true;
}

inline bool parse(core::logging::severity servirity, char ret_str[256])
{
	constexpr size_t array_size = 256;
	(void)(array_size+1); //To avoid warning that occur because of an unused macro on GCC compiler
	
	switch (servirity)
	{
	case core::logging::severity::TRACE:
		STRCPY(ret_str, array_size, "TRACE");
		break;
	case core::logging::severity::DEBUG:
		STRCPY(ret_str, array_size, "DEBUG");
		break;
	case core::logging::severity::INFO:
		STRCPY(ret_str, array_size, "INFO");
		break;
	case core::logging::severity::WARNING:
		STRCPY(ret_str, array_size, "WARNING");
		break;
	case core::logging::severity::ERROR:
		STRCPY(ret_str, array_size, "ERROR");
		break;
	case core::logging::severity::FATAL:
		STRCPY(ret_str, array_size, "FATAL");
		break;
	default:
		return false;
	}

	return true;
}

inline bool parse(const char* str, core::console::colors &color)
{
	std::string color_str(str);
	auto conversion_func = [](int val) -> char { return static_cast<char>(::toupper(val));	};
	std::transform(color_str.begin(), color_str.end(), color_str.begin(), conversion_func);

	if (std::strcmp(color_str.c_str(), "RED") == 0)
	{
		color = core::console::colors::RED;
	}
	else if (std::strcmp(color_str.c_str(), "GREEN") == 0)
	{
		color = core::console::colors::GREEN;
	}
	else if (std::strcmp(color_str.c_str(), "YELLOW") == 0)
	{
		color = core::console::colors::YELLOW;
	}
	else if (std::strcmp(color_str.c_str(), "BLUE") == 0)
	{
		color = core::console::colors::BLUE;
	}
	else if (std::strcmp(color_str.c_str(), "MAGENTA") == 0)
	{
		color = core::console::colors::MAGENTA;
	}
	else if (std::strcmp(color_str.c_str(), "CYAN") == 0)
	{
		color = core::console::colors::CYAN;
	}
	else if (std::strcmp(color_str.c_str(), "WHITE") == 0)
	{
		color = core::console::colors::WHITE;
	}
	else
	{
		return false;
	}

	return true;
}

/// convert Hexadecimal character to int
/// @date	20/12/2018
/// @exception	std::invalid_argument	Thrown when an invalid argument
/// 	error condition occurs.
/// @param	input	The input.
/// @return	An uint8_t.
inline uint8_t hex_char2int(char input)
{
	if (input >= '0' && input <= '9')
		return static_cast<uint8_t>(input - '0');
	if (input >= 'A' && input <= 'F')
		return static_cast<uint8_t>(input - 'A' + 10);
	if (input >= 'a' && input <= 'f')
		return static_cast<uint8_t>(input - 'a' + 10);

	throw std::invalid_argument("Invalid input string");
}

/// convert buffer to Hexadecimal string
/// @date	20/12/2018
/// @param 		   	buf		 	The buffer.
/// @param 		   	buff_size	Size of the buffer.
/// @param [in,out]	str		 	If non-null, the string.
/// @param 		   	str_size 	The size.
/// @return	True if it succeeds, false if it fails.
inline bool buf2hex_string(const uint8_t* buf, size_t buff_size, char* str, size_t str_size)
{
	//need to consider the \0
	if ((buff_size * 2 + 1)> str_size)
		return false;

	for (size_t j = 0; j < buff_size; j++)
	{
#ifdef _WIN32
		sprintf_s(str + (j * 2), 2*(buff_size - j)+1, "%02hhX", buf[j]);
#else
		std::sprintf(str + (j * 2), "%02hhX", buf[j]);
#endif
	}

	return true;
}

/// Hexadecimal string  to buffer
/// @date	20/12/2018
/// @param 		   	str			The string.
/// @param 		   	str_len 	The length.
/// @param [in,out]	buf			If non-null, the buffer.
/// @param 		   	buf_size	Size of the buffer.
/// @return	True if it succeeds, false if it fails.
inline bool hex_string2buf(const char* str, uint8_t* buf, size_t buf_max_size, size_t &buf_actual_size)
{
	if (str == nullptr)
		return false;

	size_t str_length = std::strlen(str);

	//no an even buffer
	if (str_length % 2 != 0)
		return false;

	//size is bigger than buffer size
	if (str_length / 2 > buf_max_size)
		return false;

	uint8_t single_byte;
	size_t offset = 0;
	buf_actual_size = 0;
	while (offset < str_length)
	{
		single_byte = static_cast<uint8_t>(hex_char2int(str[offset]) * 16 + hex_char2int(str[offset + 1]));
		buf[buf_actual_size] = single_byte;
		offset += 2;
		buf_actual_size++;
	}

	return true;
}

inline void auto_expand_environment_variables(std::string& text)
{

#ifdef _WIN32
	static std::regex env("\\%([^}]+)\\%");
#else
	static std::regex env("\\$\\{([^}]+)\\}");
#endif 
	std::smatch match;
	while (std::regex_search(text, match, env))
	{
#if defined(_WIN32) && defined(_MSC_VER)
		char *s = nullptr;
		size_t sz = 0;
		_dupenv_s(&s, &sz, match[1].str().c_str());
#else
		char* s = getenv(match[1].str().c_str());
#endif 				
		const std::string var(s == NULL ? "" : s);

		//in the future replace the bellow with pure cpp11 "replace" function 
		std::size_t size = match[0].str().size();
		std::size_t pos = text.find(match[0]);
		std::string myString = text.substr(0, pos) + var + text.substr(pos + size);
		text = myString;
	}

#ifndef _WIN32  // need to replace ~ with $HOME
	std::size_t pos = text.find('~');

	if (pos != std::string::npos)
	{
		char* home = getenv("HOME");
		std::string myString = text.substr(0, pos) + home + "/" + text.substr(pos + 1);
		text = myString;
	}
#endif
}
