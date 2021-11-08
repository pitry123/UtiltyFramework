#pragma once
#include <string>
#include <cstring>
#include <cstdint>
#include <typeinfo>
#include <utils/strings.hpp>
#include <core/types.h>
#include <regex>


namespace utils
{
	namespace types
	{
		inline bool is_numeric(const std::string& str_input)
		{
			const std::regex expression("^-?[0-9]+([.][0-9]+)?$");

			std::smatch base_match;
			if (std::regex_match(str_input, base_match, expression))
				return true;

			return false;
		}

		inline bool is_integer(const std::string& str_input)
		{
			const std::regex expression("^-?\\d+?$");

			std::smatch base_match;
			if (std::regex_match(str_input, base_match, expression))
				return true;

			return false;
		}

		template <typename T>
		inline core::types::type_enum get_type()
		{
			if (std::is_enum<T>::value)
				return core::types::ENUM;
			if (typeid(T) == typeid(char))
				return core::types::CHAR;
			else if (typeid(T) == typeid(unsigned char))
				return core::types::BYTE;
			else if (typeid(T) == typeid(short))
				return core::types::INT16;
			else if (typeid(T) == typeid(unsigned short))
				return core::types::UINT16;
			else if (typeid(T) == typeid(float))
				return core::types::FLOAT;
			else if (typeid(T) == typeid(double))
				return core::types::DOUBLE;
			else if (typeid(T) == typeid(int))
				return core::types::INT32;
			else if (typeid(T) == typeid(bool))
				return core::types::BOOL;
			else if (typeid(T) == typeid(uint8_t))
				return core::types::BYTE;
			else if (typeid(T) == typeid(uint16_t))
				return core::types::UINT16;
			else if (typeid(T) == typeid(uint32_t))
				return core::types::UINT32;
			else if (typeid(T) == typeid(uint64_t))
				return core::types::UINT64;
			else if (typeid(T) == typeid(int8_t))
				return core::types::CHAR;
			else if (typeid(T) == typeid(int16_t))
				return core::types::INT16;
			else if (typeid(T) == typeid(int32_t))
				return core::types::INT32;
			else if (typeid(T) == typeid(int64_t))
				return core::types::INT64;
			else if (typeid(T) == typeid(bool))
				return core::types::BOOL;
			else if (std::is_class<T>::value)
				return core::types::COMPLEX;
			else
				return core::types::UNKNOWN;

		}

		inline bool is_simple_type(core::types::type_enum type)
		{
			switch (type)
			{
			case core::types::type_enum::UINT8:
			case core::types::type_enum::BYTE:
			case core::types::type_enum::BOOL:
			case core::types::type_enum::INT8:
			case core::types::type_enum::CHAR:
			case core::types::type_enum::USHORT:
			case core::types::type_enum::UINT16:
			case core::types::type_enum::SHORT:
			case core::types::type_enum::INT16:
			case core::types::type_enum::UINT32:
			case core::types::type_enum::INT32:
			case core::types::type_enum::UINT64:
			case core::types::type_enum::INT64:
			case core::types::type_enum::FLOAT:
			case core::types::type_enum::DOUBLE:
			{
				return true;
			}
			break;
			default:
				return false;
			}


		}

		/// Gets type name based on T - This function only relevant to name mangling of MSVC compiler
		/// @tparam	T	Generic type parameter.
		/// @returns	The type name.
		template <typename T>
		inline std::string get_type_name()
		{
			std::string name = typeid(T).name();
			//remove the namespace (currently not supported by DE) and the word struct
			std::vector<std::string> strArr = split_string(name,"::");
			if ( strArr.size()>2)
			{
				name = strArr[strArr.size() - 2] + "::" + strArr[strArr.size() - 1];
			}
			else
			{
				strArr = split_string(name, " ");
				if (strArr.size() > 1)
					name = strArr[1];
			}

			return name;
		}

		inline core::types::type_enum get_type(const char* type_name)
		{
			const std::string type_str(type_name);

			if (type_str == "byte" ||
				type_str == "uint8" ||
				type_str == "uint8_t" ||
				type_str == "unsigned char" ||
				type_str == "byteEnum" || 
				type_str == "sbyte")
				return core::types::type_enum::BYTE;

			if (type_str == "char" ||
				type_str == "int8_t" ||
				type_str == "char8" )
				return core::types::type_enum::CHAR;

			if (type_str == "bool" ||
				type_str == "bool8")
				return core::types::type_enum::BOOL;

			if (type_str == "short" ||
				type_str == "int16_t")
				return core::types::type_enum::INT16;

			if (type_str == "unsigned int" ||
				type_str == "dword" ||
				type_str == "unsignedinteger" ||
				type_str == "uint32" ||
				type_str == "uint32_t" ||
				type_str == "unsigned long" ||
				type_str == "ulong32" ||
				type_str == "socketip")
				return core::types::type_enum::UINT32;

			if (type_str == "long" ||
				type_str == "long32" ||
				type_str == "int" ||
				type_str == "int32" ||
				type_str == "int32_t")
				return core::types::type_enum::INT32;

			if (type_str == "float" ||
				type_str == "float32")
				return core::types::type_enum::FLOAT;

			if (type_str == "double" ||
				type_str == "long double" ||
				type_str == "double64")
				return core::types::type_enum::DOUBLE;

			if (type_str == "long long" ||
				type_str == "int64" ||
				type_str == "int64_t")
				return core::types::type_enum::INT64;

			if (type_str == "unsigned long long" ||
				type_str == "unsigned __int64" ||
				type_str == "uint64" ||
				type_str == "uint64_t")
				return core::types::type_enum::UINT64;

			if (type_str == "unsignedshortinteger" ||
				type_str == "word" ||
				type_str == "unsigned short" ||
				type_str == "uint16" ||
				type_str == "uint16_t")
				return core::types::type_enum::UINT16;
			if (type_str == "none")
				return core::types::EMPTY_TYPE;
			if ((type_str.find("charbuff[")!= std::string::npos) ||
				(type_str.find("char[") != std::string::npos)|| 
				type_str == "buffer")
			{
				return core::types::type_enum::BUFFER;
			}

			if(type_str == "complex")
				return core::types::type_enum::COMPLEX;

			if (type_str == "string")
				return core::types::type_enum::STRING;

			if (type_str == "enum")
				return core::types::type_enum::ENUM;

			if (type_str == "array")
				return core::types::type_enum::ARRAY;

			std::vector<std::string> str = split_string(type_str, ":");
			if (str.size() == 2)
			{
				core::types::type_enum type = utils::types::get_type(str[0].c_str());
				if ((is_simple_type(type) ||
					type == core::types::type_enum::ENUM) &&
					isdigit(str[1][0]))
					return core::types::type_enum::BITMAP;
			}

			//if it is the name of the strcut 
			return core::types::type_enum::COMPLEX;
		}



		/// Gets a type name
		/// @date	28/07/2019
		/// @param	type   	The type.
		/// @param	[out]ret_str	The ret name as string.
		/// @return	True if it succeeds, false if it fails.
		inline bool get_type_name(core::types::type_enum type, char ret_str[256])
		{
			constexpr size_t array_size = 256;
			(void)(array_size+1);
			switch (type)
			{
			case core::types::type_enum::UINT8:
			case core::types::type_enum::BYTE:
			case core::types::type_enum::INT8:
			case core::types::type_enum::CHAR:
				STRCPY(ret_str, array_size, "byte");
				break;
			case core::types::type_enum::USHORT:
			case core::types::type_enum::UINT16:
				STRCPY(ret_str, array_size, "word");
				break;
			case core::types::type_enum::BOOL:
				STRCPY(ret_str, array_size, "bool");
				break;
			case core::types::type_enum::INT16:
			case core::types::type_enum::SHORT:
				STRCPY(ret_str, array_size, "short");
				break;
			case core::types::type_enum::UINT32:
				STRCPY(ret_str, array_size, "dword");
				break;
			case core::types::type_enum::INT32:
				STRCPY(ret_str, array_size, "int");
				break;
			case core::types::type_enum::UINT64:
				STRCPY(ret_str, array_size, "uint64_t");
				break;
			case core::types::type_enum::INT64:
				STRCPY(ret_str, array_size,"int64_t");
				break;
			case core::types::type_enum::FLOAT:
				STRCPY(ret_str, array_size, "float");
				break;
			case core::types::type_enum::DOUBLE:
				STRCPY(ret_str, array_size, "double");
				break;
			case core::types::type_enum::ENUM:
				STRCPY(ret_str, array_size, "enum");
				break;
			case core::types::type_enum::STRING:
				STRCPY(ret_str, array_size, "string");
				break;
			case core::types::type_enum::ARRAY:
				STRCPY(ret_str, array_size, "array");
				break;
			case core::types::EMPTY_TYPE:
				STRCPY(ret_str, array_size, "none");
				break;
			case core::types::type_enum::BUFFER:
				STRCPY(ret_str, array_size, "buffer");
				break;
			case core::types::type_enum::COMPLEX:
				STRCPY(ret_str, array_size, "complex");
				break;
			case core::types::type_enum::BITMAP:
				STRCPY(ret_str, array_size, "bitmap");
				break;
			default: //did not find anything
				STRCPY(ret_str, array_size, "unknwon");
				return false;
				break;
			}

			return true;
		}
		
		inline size_t sizeof_type(core::types::type_enum type)
		{
			switch (type)
			{
			case core::types::type_enum::UINT8:
			case core::types::type_enum::BYTE:
			case core::types::type_enum::BOOL:
			case core::types::type_enum::INT8:
			case core::types::type_enum::CHAR:
			{
				return sizeof(int8_t);
			}
			break;
			case core::types::type_enum::USHORT:
			case core::types::type_enum::UINT16:
			case core::types::type_enum::SHORT:
			case core::types::type_enum::INT16:
			{
				return sizeof(int16_t);
			}
			break;
			case core::types::type_enum::UINT32:
			case core::types::type_enum::INT32:
			case core::types::type_enum::ENUM: //bu default return sizeof int32
			{
				return sizeof(int32_t);
			}
			break;
			case core::types::type_enum::UINT64:
			case core::types::type_enum::INT64:
			{
				return sizeof(int64_t);
			}
			break;

			case core::types::type_enum::FLOAT:
			{
				return sizeof(float);
			}
			break;
			case core::types::type_enum::DOUBLE:
			{
				return sizeof(double);
			}
			break;
			default:
			{
				//Can not determine the size
				return 0;
			}
			}
		}

		inline bool get_bitmap_sizes(const char* str, size_t& num_of_bits, size_t& size_in_bytes)
		{
			core::types::type_enum type;
			std::vector<std::string> type_str = split_string(std::string(str), ":");
			if (type_str.size() == 2)
			{
				type = utils::types::get_type(type_str[0].c_str());
				if ((utils::types::is_simple_type(type) || type == core::types::type_enum::ENUM) && 
					isdigit(type_str[1][0]))
				{
                    num_of_bits = static_cast<size_t>(std::stol(type_str[1].c_str(), nullptr, 10));
					size_in_bytes = utils::types::sizeof_type(type);
					return true;
				}
			}
			return false;
		}

		inline core::types::type_enum get_type_by_size(size_t size, bool unsigned_type = true)
		{
			if (unsigned_type)
			{
				if (size == 1)
					return core::types::type_enum::UINT8;
				else if (size == 2)
					return core::types::type_enum::UINT16;
				else if (size == 4)
					return core::types::type_enum::UINT32;
				else if (size == 8)
					return core::types::type_enum::UINT64;
			}
			else
			{
				if (size == 1)
					return core::types::type_enum::INT8;
				else if (size == 2)
					return core::types::type_enum::INT16;
				else if (size == 4)
					return core::types::type_enum::INT32;
				else if (size == 8)
					return core::types::type_enum::INT64;
			}

			return core::types::type_enum::UNKNOWN;
		}
			
		static inline bool is_big_endian()
		{
			union {
				uint32_t i;
				char c[4];
			} e = { 0x01000000 };
			return static_cast<bool>(e.c[0]);
		}		

		static inline core::types::endian platform_endian()
		{
			if (is_big_endian() == true)
				return core::types::endian::BIG;

			return core::types::endian::LITTLE;
		}

		inline void endian_swap(uint16_t& x)
		{
			x = static_cast<uint16_t>((x >> 8) |
				(x << 8));
		}

		inline void endian_swap(uint32_t& x)
		{
			x = static_cast<uint32_t>((x >> 24) |
				((x << 8) & 0x00FF0000) |
				((x >> 8) & 0x0000FF00) |
				(x << 24));
		}

		inline void endian_swap(uint64_t& x)
		{
			x = static_cast<uint64_t>((x >> 56) |
				((x << 40) & 0x00FF000000000000) |
				((x << 24) & 0x0000FF0000000000) |
				((x << 8) & 0x000000FF00000000) |
				((x >> 8) & 0x00000000FF000000) |
				((x >> 24) & 0x0000000000FF0000) |
				((x >> 40) & 0x000000000000FF00) |
				(x << 56));
		}
	}
}
