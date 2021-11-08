#pragma once
#include <core/parser.h>
#include <core/types.h>
#include <parsers/binary_parser.h>
#include <utils/types.hpp>
#include <utils/ref_count_base.hpp>

#include <utility>
#include <string>
#include <stdexcept>

namespace utils
{
	namespace parsers
	{

		class simple_options
		{
		private:
			core::parsers::simple_options_data m_options;
			template <typename T>
			T to_val(const uint8_t* data)
			{
				T the_val = *(reinterpret_cast<const T*>(data));

				return the_val;
			}


			void from_val(uint8_t data[core::parsers::VAL_SIZE], uint8_t val[core::parsers::VAL_SIZE], size_t size)
			{
#ifdef WIN32
				memcpy_s(data, core::parsers::VAL_SIZE, val, size);
#else
				std::memcpy(data, val, size);
#endif
			}

			template <typename T>
            void from_val(uint8_t data[core::parsers::VAL_SIZE], T val)
			{
#ifdef WIN32
				memcpy_s(data, core::parsers::VAL_SIZE, &val, sizeof(T));
#else
				std::memcpy(data, &val, sizeof(T));
#endif
			}

            void from_string(const char* val, core::types::type_enum type, uint8_t(&data)[core::parsers::VAL_SIZE])
			{
				if (false == utils::types::is_simple_type(type) &&
					type != core::types::type_enum::ENUM)
					throw std::invalid_argument("type");

				switch (type)
				{
				case core::types::type_enum::INT8:
				case core::types::type_enum::CHAR:
					from_val<int8_t>(data,(int8_t)std::stoi(val));
					break;
				case core::types::type_enum::UINT8:
				case core::types::type_enum::BYTE:
					from_val<uint8_t>(data, (uint8_t)std::stoul(val));
					break;
				case core::types::type_enum::INT16:
				case core::types::type_enum::SHORT:
					from_val<int16_t>(data, (int16_t)std::stoi(val));
					break;
				case core::types::type_enum::UINT16:
				case core::types::type_enum::USHORT:
					from_val<uint16_t>(data, (uint16_t)std::stoul(val));
					break;
				case core::types::type_enum::BOOL:
					from_val<bool>(data, (bool)std::stoul(val));
					break;
				case core::types::type_enum::INT32:
					from_val<int32_t>(data, (int32_t)std::stoll(val));
					break;
				case core::types::type_enum::UINT32:
					from_val<uint32_t>(data, (uint32_t)std::stoul(val));
					break;
				case core::types::type_enum::INT64:
					from_val<int64_t>(data, (int64_t)std::stoll(val));
					break;
				case core::types::type_enum::ENUM:
					from_val<int64_t>(data, (int64_t)std::stoll(val));
				case core::types::type_enum::UINT64:
					from_val<uint64_t>(data, (uint64_t)std::stoull(val));
					break;
				case core::types::type_enum::FLOAT:
					from_val<float>(data, (float)std::stof(val));
					break;
				case core::types::type_enum::DOUBLE:
					from_val<double>(data, (double)std::stod(val));
					break;
				default:
					break;
				}
			}

		public:
			template <typename T>
			static simple_options create(const T& min, const T& max, const T& def)
			{
				simple_options options;
				
				options.minval<T>(min);
				options.maxval<T>(max);
				options.defval<T>(def);

				return options;
			}

			simple_options(const core::parsers::simple_options_data& options)
			{
				m_options = options;
			}

			simple_options()
			{
				reset();
			}

			

			simple_options(simple_options& other) :
				m_options(other.m_options)
			{
			}

			simple_options(simple_options&& other) :
				m_options(std::move(other.m_options))
			{
			}

			void reset() noexcept
			{
				minval<uint64_t>(0);
				maxval<uint64_t>(0);
				defval<uint64_t>(0);
				m_options.has_def = m_options.has_max = m_options.has_min = false;
			}

			bool empty()
			{
				if (!has_default() &&
					!has_max() &&
					!has_min())
					return true;

				return false;
			}
			template <typename T>
			std::string to_string(const uint8_t* data)
			{
				T val = to_val<T>(data);
				return std::to_string(val);
			}

			std::string to_string(const uint8_t* val, core::types::type_enum type)
			{
				if (false == utils::types::is_simple_type(type) &&
					type != core::types::type_enum::ENUM)
					throw std::invalid_argument("type");

				switch (type)
				{
				case core::types::type_enum::INT8:
				case core::types::type_enum::CHAR:
					return to_string<int8_t>(val);
					break;
				case core::types::type_enum::UINT8:
				case core::types::type_enum::BYTE:
					return to_string<uint8_t>(val);
					break;
				case core::types::type_enum::INT16:
				case core::types::type_enum::SHORT:
					return to_string<int16_t>(val);
					break;
				case core::types::type_enum::UINT16:
				case core::types::type_enum::USHORT:
					return to_string<uint16_t>(val);
					break;
				case core::types::type_enum::BOOL:
					return to_string<bool>(val);
					break;
				case core::types::type_enum::INT32:
					return to_string<int32_t>(val);
					break;
				case core::types::type_enum::UINT32:
					return to_string<uint32_t>(val);
						break;
				case core::types::type_enum::INT64:
					return to_string<int64_t>(val);
						break;
				case core::types::type_enum::UINT64:
					return to_string<uint64_t>(val);
						break;
				case core::types::type_enum::FLOAT:
					return to_string<float>(val);
						break;
				case core::types::type_enum::DOUBLE:
					return to_string<double>(val);
						break;
				case core::types::type_enum::ENUM:
					return to_string<int>(val);
					break;
				default:
					return std::string();
				}
				
				

			}

			template <typename T>
			T minval()
			{
				return to_val<T>(m_options.minval);
			}

			template <typename T>
			void minval(T val)
			{
				m_options.has_min = true;
				from_val<T>(m_options.minval, val);
			}

			void raw_minval(uint8_t val[core::parsers::VAL_SIZE],size_t size )
			{
				m_options.has_min = true;
				from_val(m_options.minval, val, size);
			}
			void minval(const char* val, core::types::type_enum type)
			{
				m_options.has_min = true;
				from_string(val, type, m_options.minval);
			}

			const uint8_t* raw_minval()
			{
				return m_options.minval;
			} 

			bool has_min()
			{
				return m_options.has_min;
			}

			template <typename T>
			T maxval()
			{
				return to_val<T>(m_options.maxval);
			}

			const uint8_t* raw_maxval()
			{
				return m_options.maxval;
			}

			template <typename T>
			void maxval(T val)
			{
				m_options.has_max = true;
				from_val<T>(m_options.maxval,val);
			}

			void raw_maxval(uint8_t val[core::parsers::VAL_SIZE], size_t size)
			{
				m_options.has_max = true;
				from_val(m_options.maxval, val, size);
			}

			void maxval(const char* val, core::types::type_enum type)
			{
				m_options.has_max = true;
				from_string(val, type, m_options.maxval);
			}

			bool has_max()
			{
				return m_options.has_max;
			}

			template <typename T>
			T defval()
			{
				return to_val<T>(m_options.defval);
			}

			const uint8_t* raw_defval()
			{
				return m_options.defval;
			}

			template <typename T>
			void defval(T val)
			{
				m_options.has_def = true;
				from_val<T>(m_options.defval,val);
			}

			void raw_defval(uint8_t val[core::parsers::VAL_SIZE], size_t size)
			{
				m_options.has_def = true;
				from_val(m_options.defval, val, size);
			}

			void defval(const char* val, core::types::type_enum type)
			{
				m_options.has_def = true;
				from_string(val, type, m_options.defval);
			} 

			bool has_default()
			{
				return m_options.has_def;
			}

		    operator core::parsers::simple_options_data() const
			{
				return m_options;
			}

			core::parsers::simple_options_data operator=(simple_options& other)
			{
				return m_options = other.m_options;
			}

			template <typename T>
			int compare_val(T val1, T val2)
			{
				if (val1 > val2)
					return 1;
				else if (val1 < val2)
					return -1;
				
				return 0;
			}

			template <typename T>
			bool is_in_bounds(const T data)
			{
				if ((!has_min() || compare_val<T>(data, minval<T>()) >= 0) &&
					(!has_max() || compare_val<T>(data, maxval<T>()) <= 0))
					return true;

				return false;
			}

			bool is_in_bounds(const uint8_t* data, size_t size, core::types::type_enum type)
			{
				switch (type)
				{
				case core::types::type_enum::INT8:
				case core::types::type_enum::CHAR:
				{
					auto val = *(reinterpret_cast<const int8_t*>(data));
					return is_in_bounds(val);
				}
					break;
				case core::types::type_enum::UINT8:
				case core::types::type_enum::BYTE:
				{
					auto val = *(reinterpret_cast<const uint8_t*>(data));
					return is_in_bounds(val);
				}
					break;
				case core::types::type_enum::INT16:
				case core::types::type_enum::SHORT:
				{
					auto val = *(reinterpret_cast<const int16_t*>(data));
					return is_in_bounds(val);
				}
				break;
				case core::types::type_enum::UINT16:
				case core::types::type_enum::USHORT:
				{
					auto val = *(reinterpret_cast<const uint16_t*>(data));
					return is_in_bounds(val);
				}
				break;
				case core::types::type_enum::BOOL:
				    if(*data == 1 || *data == 0)
						return true;
					break;
				case core::types::type_enum::INT32:
				{
					auto val = *(reinterpret_cast<const int32_t*>(data));
					return is_in_bounds(val);
				}
				break;
				case core::types::type_enum::UINT32:
				{
					auto val = *(reinterpret_cast<const uint32_t*>(data));
					return is_in_bounds(val);
				}
				
				break;
				case core::types::type_enum::INT64:
				{
					auto val = *(reinterpret_cast<const int64_t*>(data));
					return is_in_bounds(val);
				}
				break;
				case core::types::type_enum::UINT64:
				{
					auto val = *(reinterpret_cast<const uint64_t*>(data));
					return is_in_bounds(val);
				}
				break;
				case core::types::type_enum::FLOAT:
				{
					auto val = *(reinterpret_cast<const float*>(data));
					return is_in_bounds(val);
				}
				break;
				case core::types::type_enum::DOUBLE:
				{
					auto val = *(reinterpret_cast<const double*>(data));
					return is_in_bounds(val);
				}
				break;
				default: //unsupported type
					return true;
				}
				return false;
			}

			simple_options& operator=(core::parsers::simple_options_data& val)
			{
				m_options = val;
				return *this;
			}

			simple_options& operator=(simple_options&& other)
			{
				m_options = std::move(other.m_options);
				return *this;
			}
		};
		
		class binary_parser_creator : public ref_count_base<core::parsers::binary_parser_creator_interface>
		{
			bool create_parser(const core::parsers::binary_metadata_interface* metadata, core::parsers::binary_parser_interface** parser) const override
			{
				return ::parsers::binary_parser::create(metadata, false, parser);
			}
		};
	}
}