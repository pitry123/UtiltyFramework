#include <utils/parser.hpp>
#include <utils/buffer_allocator.hpp>

#include "../nlohmann/fifo_json.hpp"
#include "binary_parser_impl.h"

using namespace core::types;
using namespace utils::types;
using namespace core::parsers;
using namespace parsers;
constexpr char const* ERROR_EXPRESION = "<<<ERROR:: in field>>>";
constexpr char const* OUTOFBOUND_EXPRESION = "<<<ERROR:: value out of bounds>>>";
constexpr char const* ENUM_VALUEOUTOFBOUNDS_EXPRESION = "<<<ERROR::enum value out of bounds>>>";
constexpr char const* TOO_BIG_EXPRESION = "<<<ERROR::Data too big>>>";
constexpr char const* INTERNAL_OBJ_FAILED_EXPRESION = "<<<ERROR:: in internal object field>>>";

bool parsers::binary_parser::create(const core::parsers::binary_metadata_interface* metadata, bool reset,
	core::parsers::binary_parser_interface** parser)
{ 
	if (metadata == nullptr)
		return false;
	if (parser == nullptr)
		return false;
	

	utils::ref_count_ptr<core::parsers::binary_parser_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<binary_parser_impl>(metadata, reset);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*parser = instance;
	(*parser)->add_ref();
	return true;

	
}

parsers::binary_parser_impl::binary_parser_impl(const core::parsers::binary_metadata_interface* metadata, bool reset) :
	m_metadata(metadata),
	m_buffer(utils::make_ref_count_ptr<utils::safe_buffer>(metadata->size())),
	m_offset(0)
{
	parse_nested();
	if(reset)
		set_default_values();

}

parsers::binary_parser_impl::binary_parser_impl(core::parsers::binary_metadata_interface* metadata, core::safe_buffer_interface* buffer, size_t offset) :
	m_metadata(metadata),
	m_buffer(buffer),
	m_offset(offset)
{
	parse_nested();
}

bool parsers::binary_parser_impl::build_nested_parser(core::parsers::binary_node_interface* node)
{
	if (node->type() != type_enum::COMPLEX && node->type() != type_enum::ARRAY)
		return false;

	return m_parsers.use<bool>([&](parser_map &internal_parsers)
	{
		utils::ref_count_ptr<binary_parser_impl> parser;
		auto it = internal_parsers.find(node->name());
		if (it == internal_parsers.end())
		{
			utils::ref_count_ptr<core::parsers::binary_metadata_interface> metadata;
			if (node->nested(&metadata))
			{
				parser = utils::make_ref_count_ptr<binary_parser_impl>(metadata, m_buffer, m_offset + node->offset());
				internal_parsers.emplace(node->name(), parser);
			}
			else
			{
				return false;
			}
		}
		else
		{
			//replace the existing parser
			it->second = parser;
		}

		return true;
	});
}

bool parsers::binary_parser_impl::parse_nested()
{
	for (size_t i = 0; i < m_metadata->node_count(); i++)
	{
		utils::ref_count_ptr<core::parsers::binary_node_interface> node;
		if (m_metadata->query_node_by_index(i, &node))
		{
			if (node->type() == type_enum::COMPLEX ||
				node->type() == type_enum::ARRAY)
			{
				if (false == build_nested_parser(node))
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}
void parsers::binary_parser_impl::write_simple_default(const core::parsers::binary_node_interface* node)
{
	utils::parsers::simple_options options(node->options());
	if (options.has_default())
	{
		write_by_node(options.raw_defval(), node->size(), node);
	}
}
void parsers::binary_parser_impl::write_string_default(const core::parsers::binary_node_interface* node)
{
	const char* def_string = node->string_default();
	if (def_string != nullptr &&
		def_string[0] != 0)
	{
		write_string(node->name(), def_string);
	}
}

void parsers::binary_parser_impl::set_default_value(const char* name)
{
	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
	if (m_metadata->query_node(name, &node))
	{
		if (utils::types::is_simple_type(node->type()) ||
			node->type() == type_enum::ENUM)
			write_simple_default(node);
		else if (node->type() == STRING)
		{
			write_string_default(node);
		}
		else if (node->type() == COMPLEX)
		{
			read_complex(node->name(), &parser);
			parser->set_default_values();
		}

	}
}
void parsers::binary_parser_impl::set_default_values()
{
	for (size_t i = 0; i < m_metadata->node_count(); i++)
	{
		utils::ref_count_ptr<core::parsers::binary_node_interface> node;
		utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
		if (m_metadata->query_node_by_index(i, &node))
		{
			if (utils::types::is_simple_type(node->type()) ||
				node->type() == type_enum::ENUM)
				write_simple_default(node);
			else if (node->type() == STRING)
			{
				write_string_default(node);
			}
			else if (node->type() == COMPLEX)
			{
				read_complex(node->name(), &parser);
				parser->set_default_values();
			}
			else if (node->type() == type_enum::ARRAY)
			{
				size_t element_size, num_of_elements;
				core::types::type_enum type;
				if (read_array_attributes(i, element_size, num_of_elements, type))
				{
					if (node->options().has_def && 
						(utils::types::is_simple_type(type) || node->type() == type_enum::ENUM))
					{
						for (size_t j = 0; j < num_of_elements; j++)
						{
							utils::parsers::simple_options options(node->options());
							if (options.has_default())
							{
								write_at(i, options.raw_defval(), element_size, j);
							}
						}
					}
					else if (type == type_enum::COMPLEX)
					{
						
						utils::ref_count_ptr<binary_parser_impl> complex_md;
						read_complex_at(i, &complex_md, 0);
						complex_md->set_default_values();
						utils::ref_count_ptr<utils::ref_count_buffer> buffer =  utils::make_ref_count_ptr<utils::ref_count_buffer>(element_size);
						size_t offset = complex_md->m_offset;
						complex_md->m_buffer->safe_read(buffer->data(), element_size, offset);
						for (size_t j = 1; j < num_of_elements; j++)
						{
							m_buffer->safe_write(buffer->data(), element_size, offset + j * element_size);
						}

					}
						
					
				}
			}

		}
	}
}

void parsers::binary_parser_impl::nullify()
{
	m_buffer->nullify();
}

size_t parsers::binary_parser_impl::offset()
{
	return m_offset;
}



bool parsers::binary_parser_impl::write_by_node(const void* data, size_t number_of_bytes_to_write, const core::parsers::binary_node_interface* node)
{
	if (data == nullptr)
		return false;

	if (node == nullptr)
		return false;

	if (number_of_bytes_to_write > node->size())
		return false;
	if (node->type() == type_enum::BITMAP)
		return write_bits_by_node(data, number_of_bytes_to_write, node);

	size_t offset = m_offset + node->offset();

	return m_buffer->safe_write(data, number_of_bytes_to_write, offset);
}

bool parsers::binary_parser_impl::write_bits_by_node(const void* data, size_t number_of_bytes_to_write, const core::parsers::binary_node_interface* node)
{
	if (node == nullptr)
		return false;

	size_t offset = m_offset + node->offset();
	uint64_t bit_node_data = 0;

	if (false == m_buffer->safe_read(&bit_node_data, node->size(), offset))
		return false;

	if (false == node->write(data, number_of_bytes_to_write, &bit_node_data, node->size()))
		return false;

	return m_buffer->safe_write(&bit_node_data, node->size(), offset);
}

bool parsers::binary_parser_impl::read_complex(const char* name, binary_parser_impl** parser) const
{
	binary_parser_interface **instance;
	instance = (binary_parser_interface **)parser;
	return read_complex(name, instance);
}

bool parsers::binary_parser_impl::read_complex(size_t index, binary_parser_impl** parser) const
{
	binary_parser_interface **instance;
	instance = (binary_parser_interface **)parser;
	return read_complex(index, instance);
}

bool parsers::binary_parser_impl::read_complex_at(size_t index, binary_parser_impl** parser, size_t array_index) const
{
	binary_parser_interface **instance;
	instance = (binary_parser_interface **)parser;
	return read_complex_at(index, instance, array_index);
}

bool parsers::binary_parser_impl::bitmap_to_json(core::parsers::binary_node_interface* node, unordered_json& json) const
{
	utils::ref_count_ptr<core::parsers::binary_metadata_interface> metadata;
	uint64_t data = 0;
	type_enum type;
	if (read_by_node(&data, node->size(),node,type))
	{
		json[node->name()] = data;
		return true;
	}
	
	return false;
}

bool parsers::binary_parser_impl::enum_val_to_json(int64_t val, const core::parsers::binary_node_interface* node, unordered_json& json, core::parsers::json_details_level details_level) const
{
	bool no_errors = true;
	//minimal json therefore only val or lables is required
	utils::ref_count_ptr<core::parsers::enum_data_interface> enum_data;
	core::parsers::enum_data_item enum_item;
	if (false == node->query_enum(&enum_data))
	{
		if (details_level == core::parsers::json_details_level::JSON_ENUM_FULL)
		{
			json["name"] = ERROR_EXPRESION;
			no_errors = false;
		}
		else
		{
			json = ERROR_EXPRESION;
			no_errors = false;
		}
		return no_errors;
	}

	if (details_level == core::parsers::json_details_level::JSON_ENUM_FULL)
	{
		if (false == enum_data->item_by_val(val, enum_item))
		{
			json["name"] = ENUM_VALUEOUTOFBOUNDS_EXPRESION;
			json["val"] = val;
			no_errors = false;
		}
		else
		{
			json["name"] = enum_item.name;
			json["val"] = enum_item.value;
		}
	}
	else
	{
		if (false == enum_data->item_by_val(val, enum_item))
		{
			json = ENUM_VALUEOUTOFBOUNDS_EXPRESION;
			no_errors = false;
		}
		else
		{
			if (details_level == core::parsers::json_details_level::JSON_ENUM_LABLES)
			{
				json = enum_item.name;
			}
			else if (details_level == core::parsers::json_details_level::JSON_ENUM_VALUES)
				json = enum_item.value;
			else
			{
				json = ENUM_VALUEOUTOFBOUNDS_EXPRESION;
				no_errors = false;
			}
		}

	}
	return no_errors;
}

bool parsers::binary_parser_impl::enum_node_to_json(const core::parsers::binary_node_interface* node, unordered_json& json, core::parsers::json_details_level details_level) const
{
	
	int64_t val = 0;

	if (sizeof(val) < node->size())
		return false;
	
	core::types::type_enum type;
			
	if (false == read_enum_by_node(val, node->size(), node, type))
	{
		if (details_level == core::parsers::json_details_level::JSON_ENUM_FULL)
		{
			json["name"] = ERROR_EXPRESION;
			return false;
		}
		else
		{
			json = ERROR_EXPRESION;
			return false;
		}

	}

	return enum_val_to_json(val, node, json,details_level);
}


bool parsers::binary_parser_impl::array_to_json(const core::parsers::binary_node_interface* node, unordered_json& json, size_t index, core::parsers::json_details_level details_level) const
{
	size_t element_size, num_of_elements;
	type_enum type;
	utils::ref_count_ptr<binary_parser_impl> parser;
	read_array_attributes(index, element_size, num_of_elements, type);
	bool no_errors = true;
	if (type == type_enum::COMPLEX)
	{
		for (size_t j = 0; j < num_of_elements; j++)
		{
			parser = nullptr;
			read_complex_at(index, &parser, j);
			utils::ref_count_ptr<binary_parser_impl> concrete_parser = static_cast<binary_parser_impl*>((binary_parser_interface*)parser);
			unordered_json complex_json;
			no_errors &= concrete_parser->json(complex_json, details_level);
			json[node->name()][j] = complex_json;
		}
	}
	else
	{
		utils::ref_count_ptr<core::parsers::binary_metadata_interface> internal_metadata;
		utils::ref_count_ptr<core::parsers::binary_node_interface> child_node;
		node->nested(&internal_metadata);
		if (false == internal_metadata->query_node_by_index(size_t(0), &child_node))
			throw std::runtime_error("query_node_by_index for array index 0 - does not exist");
		
		utils::ref_count_ptr<core::parsers::enum_data_interface> enum_data;
		if (type == type_enum::ENUM)
		{
			if (false == child_node->query_enum(&enum_data))
			{
				json[node->name()] = ERROR_EXPRESION;
				return false;
			}
			
		}
		utils::parsers::simple_options options(child_node->options());
		for (size_t j = 0; j < num_of_elements; j++)
		{
			uint64_t data = 0;
			read_at(index, &data, element_size, type, j);
			if (type != type_enum::ENUM ||
				enum_data == nullptr)
			{
				if (false == options.is_in_bounds(reinterpret_cast<uint8_t*>(&data), utils::types::sizeof_type(type), type))
				{
					std::stringstream str;
					str << OUTOFBOUND_EXPRESION << "(" << data << ")";
					json[node->name()][j] = str.str().c_str();
					no_errors = false;
				}
				else if (type == type_enum::DOUBLE)
					json[node->name()][j] = *reinterpret_cast<double*>(&data);
				else if (type == type_enum::FLOAT)
					json[node->name()][j] = *reinterpret_cast<float*>(&data);
				else
					json[node->name()][j] = data;
			}
			else
			{
				no_errors &= enum_val_to_json(data, node, json[node->name()][j],details_level);
			}
		}
	}
	return no_errors;
}

bool parsers::binary_parser_impl::json(unordered_json& json,core::parsers::json_details_level details_level) const
{
	bool no_errors = true;
	for (size_t i = 0; i < m_metadata->node_count(); i++)
	{
		utils::ref_count_ptr<binary_node_interface> node;
		if (m_metadata->query_node_by_index(i, &node))
		{
			utils::parsers::simple_options options(node->options());
			if (utils::types::is_simple_type(node->type()))
			{
				if (false == validate(node))
				{
					json[node->name()] = OUTOFBOUND_EXPRESION;
					no_errors = false;
					continue;
				}
			}

			switch (node->type())
			{
			case type_enum::INT8:
			case type_enum::CHAR:
				json[node->name()] = read_simple<int8_t>(i);
				break;
			case type_enum::UINT8:
			case type_enum::BYTE:
				json[node->name()] = read_simple<uint8_t>(i);
				break;
			case type_enum::INT16:
			case type_enum::SHORT:
				json[node->name()] = read_simple<int16_t>(i);
				break;
			case type_enum::UINT16:
			case type_enum::USHORT:
				json[node->name()] = read_simple<uint16_t>(i);
				break;
			case type_enum::BOOL:
				json[node->name()] = read_simple<bool>(i);
				break;
			case type_enum::INT32:
				json[node->name()] = read_simple<int32_t>(i);
				break;
			case type_enum::UINT32:
				json[node->name()] = read_simple<uint32_t>(i);
				break;
			case type_enum::INT64:
				json[node->name()] = read_simple<int64_t>(i);
				break;
			case type_enum::UINT64:
				json[node->name()] = read_simple<uint64_t>(i);
				break;
			case type_enum::BITMAP:
				no_errors &= bitmap_to_json(node, json);
				break;
			case type_enum::ENUM:
			{
				no_errors &= enum_node_to_json(node, json[node->name()],details_level);
			}
			break;
			case type_enum::FLOAT:
				json[node->name()] = read_simple<float>(i);
				break;
			case type_enum::DOUBLE:
				json[node->name()] = read_simple<double>(i);
				break;
			case type_enum::STRING:
			{
				char str[BUFF_MAX_SIZE];
				if (true == read_string_by_index(i, str, sizeof(str)))
					json[node->name()] = str;
				else
				{
					json[node->name()] = ERROR_EXPRESION;
					no_errors = false;
				}
				break;
			}
			case type_enum::BUFFER:
			{
				uint8_t static_data[BUFF_MAX_SIZE] = { 0 };
				char hex_str[BUFF_MAX_SIZE * 2 + 1] = { 0 };
				
				uint8_t *data = nullptr;
				char *hex_str_ptr = nullptr;

				size_t size, size_hex_str = 0;
				//Assumption that buffer max size is limited to BUFF_MAX_SIZE if not read only the first BUFF_MAX_SIZE
				if (node->size() > sizeof(static_data))
				{
					std::stringstream str;
					str << TOO_BIG_EXPRESION << "(" << node->size() << ")";
					json[node->name()] = str.str().c_str();
					no_errors = false;
				}
				else
				{
					size = node->size();
					data = static_data;
					hex_str_ptr = hex_str;
					size_hex_str = sizeof(hex_str);

					type_enum node_type;
					bool success = false;

					success = read_by_node(static_cast<void*>(data), size, node, node_type);

					if(false == success)
					{
						json[node->name()] = ERROR_EXPRESION;
						no_errors = false;
					}
					else
					{
						if (false == buf2hex_string(data, size, hex_str_ptr, size_hex_str))
						{
							json[node->name()] = ERROR_EXPRESION;
							no_errors = false;
						}
						else
						{
							json[node->name()] = hex_str;
						}
					}
				}
				break;
			}
			case type_enum::ARRAY:
			{
				no_errors &= array_to_json(node, json, i,details_level);
			}
			break;
			case COMPLEX: //complex type
			{
				utils::ref_count_ptr<binary_parser_impl> parser;
				if (read_complex(i, &parser))
				{
					unordered_json complex_json;
					no_errors &= parser->json(complex_json, details_level);
					if (complex_json.empty())
					{
						json[node->name()] = INTERNAL_OBJ_FAILED_EXPRESION;
						no_errors = false;
					}
					else
						json[node->name()] = complex_json;
				}
			}
			break;
			default: //just put it as a string 
				break;

			}
		}
	}
	return no_errors;
}

bool parsers::binary_parser_impl::bitmap_from_json(const unordered_json & json, core::parsers::binary_node_interface * node, size_t& index)
{
	if (node->type() != type_enum::BITMAP)
		return false;

	if ((false == json[node->name()].is_number() &&
		false == json[node->name()].is_boolean()))
			return false;

	uint64_t data = json[node->name()].get<uint64_t>();

	if(false == write_by_node(&data ,sizeof(data),node))
			return false;
	
	return true;
}
bool parsers::binary_parser_impl::enum_val_from_json(const unordered_json& enum_json,const core::parsers::binary_node_interface* node,int64_t& val)
{
	core::parsers::enum_data_item item;
	
	if (node == nullptr)
		return false;

	utils::ref_count_ptr<core::parsers::enum_data_interface> enum_data;
	if (false == node->query_enum(&enum_data))
		return false;

	if (enum_json.is_number_integer())
	{
		val = enum_json.get<int64_t>();
		if (false == enum_data->item_by_val(val, item))
			return false;
		return true;
	}
	else if (enum_json.is_object())
	{ //enum was stored as an object of name and value
		if (enum_json.find("val") != enum_json.end())
		{
			val = enum_json["val"].get<int64_t>();
			if (false == enum_data->item_by_val(val, item))
				return false;
		}
		else if (enum_json.find("name") != enum_json.end())
		{
			if (false == enum_data->item_by_name(enum_json["name"].get<std::string>().c_str(), item))
				return false;
			val = item.value;
		}
	}
	else if (enum_json.is_string())
	{
		if (false == enum_data->item_by_name(enum_json.get<std::string>().c_str(), item))
			return false;
		val = item.value;
	}
	else
		return false;

	return true;
}

bool parsers::binary_parser_impl::array_from_json(const unordered_json& json, size_t i, utils::ref_count_ptr<binary_node_interface>& node)
{
	//no support for array of arrays
	if (json.is_array())
		return false;
	
	if (json.find(node->name()) == json.end()) //field does not exist - ignore the field
		return true;

	if (false == json[node->name()].is_array())
		return false;

	size_t json_array_size = json[node->name()].size();
	size_t element_size, num_of_elements;
	type_enum type;
	utils::ref_count_ptr<binary_parser_impl> parser;
	read_array_attributes(i, element_size, num_of_elements, type);

	if (json_array_size > num_of_elements)
		return false;

	if (type == type_enum::COMPLEX)
	{
		for (size_t j = 0; j < json_array_size; j++)
		{
			if (false == json[node->name()][j].is_object())
				return false;
			parser = nullptr;
			if (read_complex_at(i, &parser, j))
			{
				if (false == parser->from_json(json[node->name()][j]))
					return false;
			}
			else
				return false;
		}
	}
	else
	{
		utils::parsers::simple_options options(node->options());
		for (size_t j = 0; j < json_array_size; j++)
		{
			if (type == type_enum::ENUM)
			{
				utils::ref_count_ptr<core::parsers::binary_metadata_interface> metadata;
				if (false == node->nested(&metadata))
					return false;
				utils::ref_count_ptr<core::parsers::binary_node_interface> nested_node;
				if (false == metadata->query_node_by_index(0, &nested_node))
					return false;
				
				int64_t val = 0;
				if (false == enum_val_from_json(json[node->name()][j], nested_node, val))
					return false;

				write_at(i, &val, element_size, j);
			}
			else if (type == type_enum::DOUBLE)
			{
				double data = 0.;
				data = json[node->name()][j].get<double>();
				if (false == options.is_in_bounds(data))
					return false;

				write_at(i, &data, element_size, j);
			}
			else if (type == type_enum::FLOAT)
			{
				float data = 0.;
				data = json[node->name()][j].get<float>();
				if (false == options.is_in_bounds(data))
					return false;
				write_at(i, &data, element_size, j);
			}
			else
			{
				uint64_t data = 0;
				data = json[node->name()][j].get<uint64_t>();
				if (false == options.is_in_bounds(reinterpret_cast<uint8_t*>(&data), utils::types::sizeof_type(node->type()), node->type()))
					return false;

				write_at(i, &data, element_size, j);
			}


		}


	}
	return true;
}
/// Initializes this object from the given from JSON
/// @date	03/10/2018
/// @param	json	The JSON.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::from_json(const unordered_json& json)
{
	for (size_t i = 0; i < m_metadata->node_count(); i++)
	{
		utils::ref_count_ptr<binary_node_interface> node;
		if (m_metadata->query_node_by_index(i, &node))
		{
			switch (node->type())
			{
			case type_enum::INT8:
			case type_enum::CHAR:
				if (false == val_from_json<int8_t>(json, node->name(), i))
					return false;
				break;
			case type_enum::UINT8:
			case type_enum::BYTE:
				if (false == val_from_json<uint8_t>(json, node->name(), i))
					return false;
				break;
			case type_enum::INT16:
			case type_enum::SHORT:
				if (false == val_from_json<int16_t>(json, node->name(), i))
					return false;
				break;
			case type_enum::UINT16:
			case type_enum::USHORT:
				if (false == val_from_json<uint16_t>(json, node->name(), i))
					return false;
				break;
			case type_enum::BOOL:
				if (false == val_from_json<bool>(json, node->name(), i))
					return false;
				break;
			case type_enum::INT32:
				if (false == val_from_json<int32_t>(json, node->name(), i))
					return false;
				break;
			case type_enum::UINT32:
				if (false == val_from_json<uint32_t>(json, node->name(), i))
					return false;
				break;
			case type_enum::ENUM:
			{
				int64_t val = 0;
				unordered_json enum_json;
				if (json.is_array())
				{
					enum_json  = json[i];
					if(enum_json == nullptr)
						continue;
				}
				else
				{
					enum_json = json[node->name()];
				}
				
				if (false == enum_val_from_json(enum_json, node, val))
					return false;

				write(i, &val, node->size(), core::types::type_enum::ENUM);
				break;
			}
			case type_enum::INT64:
				if (false == val_from_json<int64_t>(json, node->name(), i))
					return false;
				break;
			case type_enum::UINT64:
				if (false == val_from_json<uint64_t>(json, node->name(), i))
					return false;
				break;
			case type_enum::FLOAT:
				if (false == val_from_json<float>(json, node->name(), i))
					return false;
				break;
			case type_enum::DOUBLE:
				if (false == val_from_json<double>(json, node->name(), i))
					return false;
				break;
			case type_enum::BITMAP:
				bitmap_from_json(json, node,i);
				break;
			case type_enum::STRING:
				if (true == json.is_array())
				{
					if (node->type() != type_enum::STRING)
						return false;
					write_string(i, json[i].get<std::string>().c_str());
				}
				else 
				{
					if(json.find(node->name()) == json.end())
						continue;

					if ((false == json[node->name()].is_string())||
						(false == write_string(i, json[node->name()].get<std::string>().c_str())))
							return false;
				}

				break;
			case type_enum::BUFFER:
			{
				//No support for array of arrays
				if (json.is_array())
					return false;
				if(json.find(node->name()) == json.end())
					continue;

				uint8_t data[BUFF_MAX_SIZE] = { 0 };
				//Assumption that buffer max size is limited to BUFF_MAX_SIZE
				if (json[node->name()].is_string())
				{
					size_t data_size = 0;
					if (hex_string2buf(json[node->name()].get<std::string>().c_str(), data, node->size(), data_size))
					{
						if (false == write(i, data, data_size, type_enum::BUFFER))
							return false;
					}
				}
				else
					return false;
				break;
			}
			case type_enum::ARRAY:
			{
				if (false == array_from_json(json, i, node))
					return false;
				else
				{
					continue;
				}
			}
			break;
			case type_enum::COMPLEX: //complex type
			{
				//if here and JSON is array it is a problem
				if (json.is_array())
					return false;

				if (json.find(node->name()) == json.end())
					continue;

				utils::ref_count_ptr<binary_parser_impl> parser;
				if (false == json[node->name()].is_object())
					return false;

				if (read_complex(i, &parser))
				{
					if (false == parser->from_json(json[node->name()]))
						return false;
					else
					{
						continue;
					}
				}
				else
					return false;
			}
			break;
			default: //unsupported type
				return false;

			}

			//Check if the value is in range
			if (false == validate(node))
					return false;
			
		}
	}
	return true;
}

/// Queries a the parser metadata
/// @date	20/11/2018
/// @param [in,out]	parser_metadata	If non-null, the parser metadata.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::query_metadata(const binary_metadata_interface** parser_metadata) const 
{
	
	if (parser_metadata == nullptr)
		return false;

	*parser_metadata = m_metadata;

	(*parser_metadata)->add_ref();

	return true;

}

/// Parses provided buffer according the metadata
/// @date	03/10/2018
/// @param	data	The data.
/// @param	size	The data size.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::parse(const void* data, size_t size) 
{
	if (m_metadata->size() != size)
		return false; //if size changed it does not relevant to this parser

	if (size != m_buffer->size())
		return false;

	m_buffer->safe_write(data, size, m_offset);
	return true;
}

/// Parses provided buffer according the metadata
/// @date	03/10/2018
/// @param	data	The data.
/// @param	size	The data size.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::parse(core::safe_buffer_interface *buffer)
{


	if (m_metadata->size() != buffer->size())
		return false; //if size changed it does not relevant to this parser

	
	m_buffer = buffer;

	return true;
}

/// Parse from string - parse an hex string into a buffer and call parse
/// @date	03/10/2018
/// @param	data	The data.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::parse_from_string(const char* data) 
{
	try
	{
		std::string hex_string(data);
		std::vector<uint8_t> bytes(m_buffer->size());
		size_t str_length = hex_string.length();
		if (str_length % 2 != 0)
			return false; //has to be even

		if (str_length / 2 > m_buffer->size())
			return false; //string is bigger than expected buffer size
		size_t offset = 0;
		for (size_t i = 0; i < bytes.size(); i++)
		{
			std::string byteString = hex_string.substr(offset, 2);
			char byte = (char)std::stol(byteString.c_str(), nullptr, 16);
            bytes[i] = static_cast<uint8_t>(byte);
			offset += 2;
		}

		m_buffer->safe_write(bytes.data(), bytes.size(), m_offset);

	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool parsers::binary_parser_impl::query_buffer(core::safe_buffer_interface** buffer) const
{
	if (buffer == nullptr)
		return false;

	*buffer = m_buffer;
	if (*buffer == nullptr)
		return false;

	(*buffer)->add_ref();
	return true;
}

size_t parsers::binary_parser_impl::buffer_size() const
{
	return m_metadata->size();
}

const char* parsers::binary_parser_impl::to_json(core::parsers::json_details_level details_level, bool compact)
{
	bool no_errors = true;
	const char* json_str = check_and_get_json(details_level, compact, no_errors);
	if (no_errors)
		return json_str;
	else
		return nullptr;
}

const char* parsers::binary_parser_impl::check_and_get_json(core::parsers::json_details_level details_level, bool compact, bool& no_errors)
{
	int indent = -1;
	no_errors = true;
	if (false == compact)
		indent = 4;
	unordered_json obj;
	no_errors = json(obj,details_level);

	m_json_str = obj.dump(indent);

	return m_json_str.c_str();
}

bool parsers::binary_parser_impl::from_json(const char *json)
{
	unordered_json temp_json;
	temp_json = unordered_json::parse(json);
	return from_json(temp_json);
}
/// Reads a simple data (primitive) from the buffer
/// @date	03/10/2018
/// @param 		   	index				   	Zero-based index of the.
/// @param [out]	data				   If non-null, the data read from the buffer
/// @param 		   	number_of_bytes_to_read	Number of bytes to reads (size of the object).
/// @param [out]	type				   	The type.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_simple(size_t index, void* data, size_t number_of_bytes_to_read, type_enum& type) const 
{
	
	if (index >= m_metadata->node_count())
		return false;

	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node_by_index(index, &node))
		return false;

	return read_by_node(data, number_of_bytes_to_read, node, type);
}

/// Reads a simple data (primitive) from the buffer
/// @date	03/10/2018
/// @param 		   	name				   	The name of the data to read.
/// @param [out]	data				   	If non-null, the data read from the buffer.
/// @param 		   	number_of_bytes_to_read	Number of bytes to reads (size of the object).
/// @param [out]	type				   	The type of the data.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_simple(const char* name, void* data, size_t number_of_bytes_to_read, type_enum& type) const 
{
	if (name == nullptr)
		return false;

	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node(name, &node))
		return false;

	return read_by_node(data, number_of_bytes_to_read, node, type);
}

bool parsers::binary_parser_impl::read_enum_by_node(int64_t& val, size_t size, const core::parsers::binary_node_interface* node, core::types::type_enum& type) const
{
	val = 0;
	if (size == 1)
	{
		int8_t internal_val;
		if (read_by_node(&internal_val, size, node, type))
			val = internal_val;
		else
			return false;
	}
	else if (size == 2)
	{
		int16_t internal_val;
		if (read_by_node(&internal_val, size, node, type))
			val = internal_val;
		else
			return false;
	}
	else if (size == 4)
	{
		int32_t internal_val;
		if (read_by_node(&internal_val, size, node, type))
			val = internal_val;
		else
			return false;
	}
	else if (size == 8)
	{
		int64_t internal_val;
		if (read_by_node(&internal_val, size, node, type))
			val = internal_val;
		else
			return false;
	}
	else 
		return false;
	
	return true;
}
bool parsers::binary_parser_impl::read_enum(const char* name, core::parsers::enum_data_item& enum_item) const
{
	if (name == nullptr)
		return false;
	
	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node(name, &node))
		return false;
	
	if (node->type() != core::types::type_enum::ENUM)
		return false;

	int64_t val = 0;
	if (sizeof(val) < node->size())
		return false;
	core::types::type_enum type;
	if (false == read_enum_by_node(val, sizeof(val), node, type))
		return false;

	utils::ref_count_ptr<core::parsers::enum_data_interface> enum_data;
	if (node->query_enum(&enum_data))
	{
		return enum_data->item_by_val(val, enum_item);
	}
	else //no enum metadata - just return the number with no string
	{
		enum_item.name[0] = 0;
		enum_item.value = val;
	}
	return true;

}

bool parsers::binary_parser_impl::read_enum_by_index(size_t index, core::parsers::enum_data_item & enum_item) const
{
	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node_by_index(index, &node))
		return false;

	if (node->type() != core::types::type_enum::ENUM)
		return false;

	int64_t val = 0;
	if (sizeof(val) < node->size())
		return false;
	core::types::type_enum type;
	if (false == read_enum_by_node(val, node->size(), node, type))
		return false;

	utils::ref_count_ptr<core::parsers::enum_data_interface> enum_data;
	if (node->query_enum(&enum_data))
	{
		return enum_data->item_by_val(val, enum_item);
	}
	else //no enum metadata - just return the number with no string
	{
		enum_item.name[0] = 0;
		enum_item.value = val;
	}
	return true;
}

bool parsers::binary_parser_impl::read_bits_by_node(void* data, size_t number_of_bytes_to_read, const core::parsers::binary_node_interface *node) const
{
	if (node == nullptr)
		return false;

	size_t offset = m_offset + node->offset();
	uint64_t bit_node_data = 0;

	if (false == m_buffer->safe_read(&bit_node_data, node->size(), offset))
		return false;

	if (false == node->read(data, number_of_bytes_to_read, &bit_node_data, node->size()))
		return false;
	return true;
}

/// Reads data from the buffer
/// @date	03/10/2018
/// @param [out]	data				   	If non-null, the data read from the buffer.
/// @param 		   	number_of_bytes_to_read	Number of bytes to reads (size of the object).
/// @param [in]	node				   	If non-null, the node that describes the data to read.
/// @param [out]	type				   	The type.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_by_node(void* data, size_t number_of_bytes_to_read, const core::parsers::binary_node_interface *node, type_enum& type) const 
{
	if (data == nullptr)
		return false;

	if (node == nullptr)
		return false;

	if (number_of_bytes_to_read > node->size())
		return false;

	type = node->type();
	
	if (type == type_enum::BITMAP)
		return read_bits_by_node(data, number_of_bytes_to_read, node);

	size_t offset = m_offset + node->offset();
	return read_from_buffer(data, number_of_bytes_to_read, offset,node->big_endian());
}

bool parsers::binary_parser_impl::read_from_buffer(void* data, size_t number_of_bytes_to_read,size_t offset,bool big_endian) const
{
	if (m_buffer->safe_read(data, number_of_bytes_to_read, offset))
	{
		if (big_endian != utils::types::is_big_endian())
		{
			//if data stream is mismatched to the platform endian convert
			size_t convert_data_size;
			uint8_t* data_change = (uint8_t*)data;
			if (number_of_bytes_to_read % 2 != 0)
			{
				convert_data_size = number_of_bytes_to_read - 1;
			}
			else
			{
				convert_data_size = number_of_bytes_to_read;
			}

			for (size_t i = 0; i < convert_data_size; i += 2)
			{
				endian_swap(*((uint16_t*)&data_change[i]));
			}
		}

		return true;
	}
	return false;
}
/// Reads a string from the parser
/// @date	10/10/2018
/// @exception	std::runtime_error	Raised when a runtime error condition
/// 	occurs.
/// @param	index	Zero-based index of the field.
/// @return	The string.
bool parsers::binary_parser_impl::read_string_by_index(size_t index, char* str, size_t length) const 
{

	utils::ref_count_ptr<const binary_metadata_interface> metadata;
	if (false == query_metadata(&metadata))
		return false;

	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == metadata->query_node_by_index(index, &node))
		return false;

	if (node->size() > length)
		return false;

	type_enum type;
	if (false == read_by_node(str, node->size(), node, type))
		return false;
	//assuming the string is null terminated
	return true;
}

/// Reads a string from the parser
/// @date	10/10/2018
/// @exception	std::runtime_error	Raised when a runtime error condition
/// 	occurs.
/// @param	name	The name of the field.
/// @return	The string.
bool parsers::binary_parser_impl::read_string(const char* name, char* str, size_t length) const 
{
	utils::ref_count_ptr<const binary_metadata_interface> metadata;
	if (false == query_metadata(&metadata))
		return false;

	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == metadata->query_node(name, &node))
		return false;

	if (node->size() > length)
		return false;

	type_enum type;
	if (false == read_by_node(str, node->size(), node, type))
		return false;

	return true;
}
/// Reads array attributes, read an array attributes
/// @date	03/10/2018
/// @param 		   	name		   	The name.
/// @param [out]	element_size   	Size of a single element in the array.
/// @param [out]	num_of_elements	Number of elements.
/// @param [out]	type		   	The type of a single element.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_array_attributes(const char* name, size_t& element_size, size_t& num_of_elements, type_enum& type) const
{
	utils::ref_count_ptr<binary_node_interface> node;
	if (m_metadata->query_node(name, &node))
	{
		if (node->type() != type_enum::ARRAY)
			return false;

		//Array is also a complex data so we can get its parser
		utils::ref_count_ptr<binary_metadata_interface> metadata;
		node->nested(&metadata);

		num_of_elements = node->count();

		//get the array details from the first node of the array
		utils::ref_count_ptr<binary_node_interface> child_node;
		if (false == metadata->query_node_by_index(size_t(0), &child_node))
			return false;

		element_size = child_node->size();
		type = child_node->type();
	}

	return true;
}

/// Reads array attributes, read an array attributes
/// @date	03/10/2018
/// @param 		   	index		   	Zero-based index of the.
/// @param [out]	element_size   	Size of a single element in the array.
/// @param [out]	num_of_elements	Number of elements.
/// @param [out]	type		   	The type of a single element.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_array_attributes(size_t index, size_t &element_size, size_t &num_of_elements, type_enum &type) const
{
	utils::ref_count_ptr<binary_node_interface> node;
	if (m_metadata->query_node_by_index(index, &node))
	{
		if (node->type() != type_enum::ARRAY)
			return false;

		//Array is also a complex data so we can get its parser
		utils::ref_count_ptr<binary_parser_impl> parser;
		utils::ref_count_ptr<const binary_metadata_interface> metadata;
		read_complex(index, &parser);
		if (parser != nullptr)
		{
			if (false == parser->query_metadata(&metadata))
				return false;

			num_of_elements = node->count();

			//get the array details from the first node of the array
			utils::ref_count_ptr<binary_node_interface> child_node;
			if (false == metadata->query_node_by_index(size_t(0), &child_node))
				return false;

			element_size = child_node->size();
			type = child_node->type();
		}
	}

	return true;
}

/// Reads at, reads an element from an array - good only for simple arrays
/// @date	03/10/2018
/// @param 		   	index				   	Zero-based index of the of the node that start the array.
/// @param [out]	data				   	If non-null, the data.
/// @param 		   	number_of_bytes_to_read	Number of bytes to reads.
/// @param [out]	type				   	The type.
/// @param 		   	array_index			   	Zero-based index of the element to read in the array.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_at(size_t index, void* data, size_t number_of_bytes_to_read, type_enum &type, size_t array_index) const
{
	utils::ref_count_ptr<binary_node_interface> node;
	if (m_metadata->query_node_by_index(index, &node))
	{
		return read_at(node->name(), data, number_of_bytes_to_read, type, array_index);
	}

	return false;
}

/// Reads at, reads an element from an array - good only for simple
/// arrays
/// @date	03/10/2018
/// @param 			name				   	The name.
/// @param [out]	data				   	If non-null, the data.
/// @param 			number_of_bytes_to_read	Number of bytes to reads.
/// @param [out]	type				   	The type.
/// @param 			array_index			   	Zero-based index of the element
/// 	to read in the array.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_at(const char* name, void* data, size_t number_of_bytes_to_read, type_enum &type, size_t array_index) const
{
	return m_parsers.use<bool>([&](const parser_map &internal_parsers)
	{
		auto it = internal_parsers.find(name);
		if (it == internal_parsers.end())
			return false;

		utils::ref_count_ptr<binary_parser_impl> internal_parser = static_cast<binary_parser_impl*>((binary_parser_interface*)it->second);
		utils::ref_count_ptr<const core::parsers::binary_metadata_interface> internal_metadata;
		utils::ref_count_ptr<core::parsers::binary_node_interface> child_node;
		if (false == internal_parser->query_metadata(&internal_metadata))
			return false;
		internal_metadata->query_node_by_index(size_t(0), &child_node);
		if (child_node->type() != type_enum::COMPLEX)
		{
			size_t offset = internal_parser->m_offset  + (array_index*child_node->size());
			return internal_parser->read_from_buffer(data, number_of_bytes_to_read, offset, child_node->big_endian());
		}
	
		return false;
	});

}

/// Reads a complex data from the parser - complex data is usually a struct or a more complex data that requires a parser of its own.
/// @date	03/10/2018
/// @param 		   	index 	Zero-based index of the.
/// @param [out]	parser	If non-null, the parser.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_complex(size_t index, binary_parser_interface** parser) const
{

	if (index >= m_metadata->node_count())
		return false;

	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node_by_index(index, &node))
		return false;

	return read_complex(node->name(), parser);
}

/// Reads a complex data from the parser - complex data is usually a struct or a more complex data that requires a parser of its own.
/// @date	03/10/2018
/// @param 		   	name  	The name.
/// @param [out]	parser	If non-null, the parser.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_complex(const char* name, binary_parser_interface** parser) const
{
	if (name == nullptr)
		return false;

	if (parser == nullptr)
		return false;

	return m_parsers.use<bool>([&](const parser_map &internal_parsers)
	{
		auto it = internal_parsers.find(name);
		if (it != internal_parsers.end())
		{
			*parser = it->second;
			(*parser)->add_ref();
			return true;
		}

		return false;
	});
}

/// Reads at, reads an element from an array - good only for complex objects
/// @date	03/10/2018
/// @param 		   	name  	The name.
/// @param [in,out]	parser	If non-null, the parser.
/// @param 		   	index 	Zero-based index of the.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_complex_at(const char* name, binary_parser_interface** parser, size_t array_index) const 
{
	utils::ref_count_ptr<binary_node_interface> node;
	utils::ref_count_ptr<binary_node_interface> node_internal;
	if (m_metadata->query_node(name, &node))
	{
		if (node->count() <= array_index)
			return false;

		utils::ref_count_ptr<binary_parser_impl> array_complex_parser;
		utils::ref_count_ptr<core::parsers::binary_metadata_interface> metadata;
		node->nested(&metadata);
		metadata->query_node_by_index(size_t(0), &node_internal);
		metadata = nullptr;
		node_internal->nested(&metadata);
		array_complex_parser = utils::make_ref_count_ptr<binary_parser_impl>(metadata, m_buffer, node->offset() + node_internal->size()*array_index);
		
		*parser = array_complex_parser;
		(*parser)->add_ref();
		return true;

	}

	return false;
}

/// Reads at, reads an element from an array - good only for complex objects
/// @date	03/10/2018
/// @param 		   	index	   	Zero-based index of the.
/// @param [out]	parser	   	If non-null, the parser.
/// @param 		   	array_index	Zero-based index of the element
/// 	to read in the array.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::read_complex_at(size_t index, binary_parser_interface** parser, size_t array_index) const 
{
	utils::ref_count_ptr<binary_node_interface> node;
	utils::ref_count_ptr<binary_node_interface> node_internal;
	if (m_metadata->query_node_by_index(index, &node))
	{
		if (node->count() <= array_index)
			return false;
		
		utils::ref_count_ptr<binary_parser_impl> array_complex_parser;
		utils::ref_count_ptr<core::parsers::binary_metadata_interface> metadata;
		node->nested(&metadata);
		metadata->query_node_by_index(size_t(0), &node_internal);
		metadata = nullptr;
		node_internal->nested(&metadata);
		array_complex_parser = utils::make_ref_count_ptr<binary_parser_impl>(metadata, m_buffer,m_offset +  node->offset() + node_internal->size()*array_index);
		*parser = array_complex_parser;
		(*parser)->add_ref();
		return true;
		
	}

	return false;
}

/// Writes data to the buffer
/// @date	03/10/2018
/// @param 		   	name						The name.
/// @param [in]	data						If non-null, the data.
/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::write(const char* name, const void* data, size_t number_of_bytes_to_write, type_enum type) 
{
	if (name == nullptr)
		return false;

	if (data == nullptr)
		return false;

	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node(name, &node))
		return false;
	if (node->type() != type_enum::BITMAP)
	{
		if (number_of_bytes_to_write != node->size())
			return false;
	}
	return write_by_node(data, number_of_bytes_to_write, node);
}

/// Writes string to the buffer
/// @date	03/10/2018
/// @param 		   	name						The name.
/// @param [in]	data						If non-null, the data.
/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::write_string(const char* name, const char* data) 
{
	if (name == nullptr)
		return false;

	if (data == nullptr)
		return false;

	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node(name, &node))
		return false;

	if (type_enum::STRING != node->type())
		return false;

	//write the string include the zero at the end
	size_t size = std::strlen(data) + 1;
	if (size > node->size())
		return false;

	return write_by_node(data, size, node);
}

/// Writes string to the buffer
/// @date	03/10/2018
/// @param 		   	name						The name.
/// @param [in]	data						If non-null, the data.
/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::write_string(size_t index, const char* data) 
{
	if (data == nullptr)
		return false;

	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node_by_index(index, &node))
		return false;

	if (type_enum::STRING != node->type())
		return false;

	//write the string include the zero at the end
	size_t size = std::strlen(data) + 1;
	if (size > node->size())
		return false;

	return write_by_node((const uint8_t*)data, size, node);
}

/// Writes
/// @date	03/10/2018
/// @param	index						Zero-based index of the.
/// @param	data						The data.
/// @param	number_of_bytes_to_write	Number of bytes to writes.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::write(size_t index, const void* data, size_t number_of_bytes_to_write, type_enum type) 
{
	if (index >= m_metadata->node_count())
		return false;

	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node_by_index(index, &node))
		return false;

	if (type != node->type())
		return false;

	return write_by_node(data, number_of_bytes_to_write, node);
}

/// Writes at a specific index to and array
/// @date	03/10/2018
/// @param 		   	name						The name.
/// @param [in]	data						If non-null, the data.
/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
/// @param 		   	index						Zero-based index of the.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::write_at(const char* name, const void* data, size_t number_of_bytes_to_write, size_t index) 
{
	return m_parsers.use<bool>([&](parser_map& internal_parsers)
	{
		auto it = internal_parsers.find(name);
		if (it == internal_parsers.end())
			return false;

		utils::ref_count_ptr<binary_parser_impl> internal_parser = static_cast<binary_parser_impl*>((binary_parser_interface*)it->second);
		utils::ref_count_ptr<const core::parsers::binary_metadata_interface> internal_metadata;
		
		if (false == internal_parser->query_metadata(&internal_metadata))
			return false;
		utils::ref_count_ptr<core::parsers::binary_node_interface> child_node;
		internal_metadata->query_node_by_index(size_t(0), &child_node);
		
		if (child_node->type() != type_enum::COMPLEX)
		{
			size_t offset = internal_parser->m_offset + (index*child_node->size());
			m_buffer->safe_write(data, number_of_bytes_to_write, offset);
			return true;
		}

		return false;
	});
}

/// Writes at a specific index to and array
/// @date	03/10/2018
/// @param          index						The index.
/// @param          [in]	data                If non-null, the data.
/// @param 		   	number_of_bytes_to_write	Number of bytes to writes.
/// @param 		   	index						Zero-based index of the.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::write_at(size_t index, const void* data, size_t number_of_bytes_to_write, size_t array_index) 
{
	utils::ref_count_ptr<binary_node_interface> node;
	if (false == m_metadata->query_node_by_index(index, &node))
		return false;

	return write_at(node->name(), data, number_of_bytes_to_write, array_index);
}


/// Checks this object if all data is valid
/// @date	19/08/2019
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::validate() const 
{
	for (size_t i = 0; i < m_metadata->node_count(); i++)
	{
		utils::ref_count_ptr<core::parsers::binary_node_interface> node;
		if (false == m_metadata->query_node_by_index(i, &node))
			return false;
		if (false == validate(node))
			return false;
	}

	return true;
}

/// Validates the data of the field of the given name
/// @date	19/08/2019
/// @param	name	The name.
/// @return	True if it succeeds, false if it fails.
bool parsers::binary_parser_impl::validate(const char* name) const 
{
	if (name == nullptr)
		return false;

	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node(name, &node))
		return false;

	return validate(node);
	
}

bool parsers::binary_parser_impl::validate(size_t index) const
{
	
	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == m_metadata->query_node_by_index(index, &node))
		return false;

	return validate(node);

}

int64_t parsers::binary_parser_impl::reinterpret_enum(size_t size, int64_t val) const
{
	
	if(size == sizeof(int8_t))
		val = *reinterpret_cast<int8_t*>(&val);
	else if(size == sizeof(int16_t))
		val = *reinterpret_cast<int16_t*>(&val);
	else if (size == sizeof(int32_t))
		val = *reinterpret_cast<int32_t*>(&val);
	
	return val;
}

bool parsers::binary_parser_impl::validate(const core::parsers::binary_node_interface* node) const
{
	uint8_t data[core::parsers::VAL_SIZE]; //max size of primitive
	core::types::type_enum type;
	if (utils::types::is_simple_type(node->type()))
	{
		read_by_node(data, node->size(), node, type);
		utils::parsers::simple_options options(node->options());
		return options.is_in_bounds(data, node->size(), type);
	}
	else if (node->type() == type_enum::ENUM)
	{
		int64_t enum_val = 0;
		read_by_node(&enum_val, node->size(), node, type);

		enum_val = reinterpret_enum(node->size(), enum_val);
		utils::ref_count_ptr<enum_data_interface> enum_data;
		if (false == node->query_enum(&enum_data))
			return false;

		enum_data_item item;
		if (false == enum_data->item_by_val(enum_val, item))
			return false;
	}
	else if (node->type() == COMPLEX)
	{
		utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
		read_complex(node->name(), &parser);
		return parser->validate();
	}
	else if (node->type() == ARRAY)
	{
		size_t num_of_elements, element_size;
		core::types::type_enum type;
		if (read_array_attributes(node->name(), element_size, num_of_elements, type))
		{

			if (type == core::types::type_enum::COMPLEX)
			{
				
				for (size_t i = 0; i < num_of_elements; i++)
				{
					utils::ref_count_ptr<core::parsers::binary_parser_interface> array_parser;
					if (read_complex_at(node->name(), &array_parser, i))
					{
						if (false == array_parser->validate())
							return false;
					}

				}
			}
			else if (utils::types::is_simple_type(type))
			{
				for (size_t i = 0; i < num_of_elements; i++)
				{
					if (read_at(node->name(), data,element_size,type,i))
					{
						utils::parsers::simple_options options(node->options());
						if (false == options.is_in_bounds(data, node->size(), type))
							return false;
					}

				}
			}
		}
	}

	return true;
}
