#include "rules_concrete_generic.h"

#include <sstream>

#include <core/database.h>
#include <core/parser.h>
#include <utils/types.hpp>
#include <utils/strings.hpp>

rules_concrete_generic::rules_concrete_generic(
	core::database::dataset_interface * dataset,
	core::database::table_interface* input_table,
	core::database::table_interface* enable_table,
	core::database::table_interface* existence_table,
	core::database::table_interface* output_table,
	core::database::table_interface* management_table,
	core::parsers::binary_metadata_store_interface* store) :
	m_dataset(dataset),
	m_input_table(input_table),
	m_enable_table(enable_table),
	m_existence_table(existence_table),
	m_output_table(output_table),
	m_management_table(management_table),
	m_store(store)
{}

bool
rules_concrete_generic::query_row_by_string(const char* row_string, core::database::row_interface** row, std::vector<std::string>& path_vector)
{
	if (row == nullptr)
		return false;

	if (m_input_table->query_row_by_name(row_string, row))
		return true;

	if (m_output_table->query_row_by_name(row_string, row))
		return true;

	if (m_existence_table->query_row_by_name(row_string, row))
		return true;

	if (m_enable_table->query_row_by_name(row_string, row))
		return true;

	std::string table_string, row_string_from_path;

	if (data_path_vector(row_string, table_string, row_string_from_path, path_vector))
	{
		utils::ref_count_ptr<core::database::table_interface> table;
		if (false == m_dataset->query_table_by_name(table_string.c_str(), &table))
			return false;
		if (table->query_row_by_name(row_string_from_path.c_str(), row))
			return true;
	}

	return false;
}

bool
rules_concrete_generic::query_row_by_string(const char* row_string, core::database::row_interface** row)
{
	std::vector<std::string> path_vector;
	return query_row_by_string(row_string, row, path_vector);
}


bool rules_concrete_generic::data_path_vector(const std::string& input, std::string& table, std::string& row, std::vector<std::string>& path_vector)
{
	path_vector = split_string(input, "/");
	if (path_vector.size() <= 1)
	{
		return false;
	}
	
	if (path_vector.size() > 1)
	{
		table = path_vector[0];
		path_vector = split_string(path_vector[1].c_str(), ".");
		row = path_vector[0];
		path_vector.erase(path_vector.begin());
	}
	
	return true;
}

bool
rules_concrete_generic::query_rule_existence_row(size_t rule_id, core::database::row_interface** row)
{
	if (row == nullptr)
		return false;

	auto it = m_rule_existence_map.find(rule_id);

	if (it != m_rule_existence_map.end())
	{
		*row = it->second;
	}
	else
		return false;

	if (*row == nullptr)
		return false;

	(*row)->add_ref();

	return true;
}

bool
rules_concrete_generic::query_rule_enable_row(size_t rule_id, core::database::row_interface** row)
{
	if (row == nullptr)
		return false;

	auto it = m_rule_enabled_map.find(rule_id);

	if (it != m_rule_enabled_map.end())
	{
		*row = it->second;
	}
	else
		return false;

	if (*row == nullptr)
		return false;

	(*row)->add_ref();

	return true;
}

bool
rules_concrete_generic::query_reload_rules_row(core::database::row_interface** row)
{
	if (row == nullptr)
		return false;

	if (m_management_table == nullptr)
		return false;

	return m_management_table->query_row_by_name("RELOAD_RULES", row);
}


void rules_concrete_generic::normalize_rule_name(const char* row_name, const char* prefix,std::string& normalized_row_name)
{
	normalized_row_name = row_name;

	// Make the row name as written by rules editor conventions.
	normalized_row_name.erase(std::remove(normalized_row_name.begin(), normalized_row_name.end(), '-'),normalized_row_name.end());
	std::replace(normalized_row_name.begin(), normalized_row_name.end(), ' ', '_');
	std::replace(normalized_row_name.begin(), normalized_row_name.end(), ',', '_');
	normalized_row_name.insert(0, prefix); 
}

bool
rules_concrete_generic::add_rule_enable_row(size_t rule_id, const char* row_name)
{
	std::string row_name_str;
	normalize_rule_name(row_name, "en",row_name_str);
	utils::ref_count_ptr<core::database::row_interface> row;

	if (false == query_row_by_string(row_name_str.c_str(), &row))
		return false;

	m_rule_enabled_map.emplace(rule_id, row);
	return true;
}

bool
rules_concrete_generic::add_rule_existence_row(size_t rule_id, const char* row_name)
{
	std::string row_name_str;
	normalize_rule_name(row_name,"r", row_name_str);

	utils::ref_count_ptr<core::database::row_interface> row;

	if (false == m_existence_table->query_row_by_name(row_name_str.c_str(), &row))
		return false;

	m_rule_existence_map.emplace(rule_id, row);
	return true;
}

std::function<double()>
rules_concrete_generic::get_function(const char* func_string)
{
	std::function<double()> func = nullptr;
	utils::ref_count_ptr<core::rules::rule_func_callback_interface> callback;
	if (query_function(func_string, &callback))
	{
		func = std::function<double()>([callback]() {
			double retval;
			if (false == callback->execute(retval))
				throw std::runtime_error("execute failed");

			return retval;
		});
	}
	else
		throw std::runtime_error("function does not exist");
	return func;
}



bool rules_concrete_generic::query_function(const char* func_string, core::rules::rule_func_callback_interface** callback)
{
	if (callback == nullptr)
		return false;
	bool retval = false;
	// If not found than add it dynamically.
	auto it = m_funcMap.find(std::string(func_string));
	if (it == m_funcMap.end())
	{
		std::string row_string = get_row_name_by_prefix(func_string);
		utils::ref_count_ptr<core::database::row_interface> row;

		std::vector<std::string> path_vector;
		if (false == query_row_by_string(row_string.c_str(), &row, path_vector))
		{
			return false;
		}
		else if (row->info().type == core::types::ENUM) // Add enum to the map if the row type is enum.
			{
				if (add_enum_map(row->info().type_name) == false)
					return false;
			}
		if (row != nullptr)
		{
			//allow only simple types or enums
			if (utils::types::is_simple_type(row->info().type) ||
				row->info().type == core::types::type_enum::ENUM)
			{
				retval = add_function_map(func_string,
					[row]()
				{
					uint8_t buffer[8] = { 0 };
					double val;
					row->read_bytes(buffer);
					if (row->info().type == core::types::FLOAT)
						val = *(reinterpret_cast<const float*>(buffer));
					else if (row->info().type == core::types::DOUBLE)
						val = *(reinterpret_cast<const double*>(buffer));
					else
						val = static_cast<double>((*(reinterpret_cast<const int32_t*>(buffer))));

					return val;
				});
			}
			else if (row->info().type == core::types::type_enum::COMPLEX)
			{
				utils::ref_count_ptr<utils::ref_count_buffer> buffer = utils::make_ref_count_ptr<utils::ref_count_buffer>(row->data_size());
				utils::ref_count_ptr<core::parsers::binary_node_interface> node;

				utils::ref_count_ptr<core::parsers::binary_metadata_interface> metadata;
				if (false == row->query_parser_metadata(&metadata))
					return false;
				size_t node_offset=0;
				utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
				metadata->create_parser(&parser);
				if (false == utils::rules::get_node_and_offset_by_path(parser,path_vector, &node, node_offset))
					return false;

				if (false == utils::types::is_simple_type(node->type()) &&
					node->type() != core::types::type_enum::ENUM)
					return false;
				else if (node->type() == core::types::type_enum::ENUM)
				{
					utils::ref_count_ptr<core::parsers::enum_data_interface> enum_data;
					node->query_enum(&enum_data);
					// Add enum to the map if the row type is enum.
					if (add_enum_map(enum_data->name()) == false)
						return false;
				}
				
				retval = add_function_map(func_string,
					[row, node, buffer,node_offset]()
				{
					uint8_t data[8] = { 0 };
														
					if (false == row->read_bytes(buffer->data(), buffer->size()))
						throw std::runtime_error("failed to read data from row");

					node->read(data, node->size(), &(buffer->data()[node_offset]), buffer->size() - node_offset);
					
					double val;
					if (node->type() == core::types::FLOAT)
						val = *(reinterpret_cast<const float*>(data));
					else if (node->type() == core::types::DOUBLE)
						val = *(reinterpret_cast<const double*>(data));
					else
						val = static_cast<double>((*(reinterpret_cast<const int32_t*>(data))));

					return val;

				});
			}

			if (false == retval)
				return false;
		}
		
		it = m_funcMap.find(std::string(func_string));

		if (it == m_funcMap.end())
			return false;
	}

	*callback = it->second;
	(*callback)->add_ref();

	return true;
}

bool
rules_concrete_generic::get_enumeration(const char* enum_name_val, const char* enum_type_name, int64_t& val)
{
	if (false == utils::rules::rules_data_and_types_base<core::rules::rules_data_and_types_interface>::
		get_enumeration(enum_name_val, val))
	{
		if (enum_type_name != nullptr && enum_type_name[0] != 0)
		{
			add_enum_map(enum_type_name);
			return utils::rules::rules_data_and_types_base<core::rules::rules_data_and_types_interface>::
				get_enumeration(enum_name_val, val);
		}
		else
			return false;
	}
	return true;
}

bool
rules_concrete_generic::add_enum_map(const char* enum_name)
{
	core::parsers::enum_data_interface* enum_data;
	if (m_store->query_enum(enum_name, &enum_data) == true)
	{
		if (enum_data == nullptr)
			return false;

		for (size_t index = 0; index < enum_data->size(); index++)
		{
			core::parsers::enum_data_item item;
			enum_data->item_by_index(index, item);
			m_enumMap.emplace(std::string(item.name), item.value);
		}

		return true;
	}
	else
	{
		std::cout << __func__ << " query_enum" << ": " << std::string(enum_name);
		return false;
	}
}

std::string
rules_concrete_generic::get_row_name_by_prefix(const char* func_string)
{
	std::string func_str(func_string);
	std::string row_name="";

	if (func_str.find("GetInput") != std::string::npos) {
		std::vector<std::string> split_str = split_string(func_str, "GetInput");
		row_name = "i" + split_str[0];
		return row_name;
	}
	else if (func_str.find("GetOutput") != std::string::npos) {
		std::vector<std::string> split_str = split_string(func_str, "GetOutput");
		row_name = "o" + split_str[0];
		return row_name;
	}
	else if (func_str.find("GetRes") != std::string::npos) {
		std::vector<std::string> split_str = split_string(func_str, "GetRes");
		row_name = "r" + split_str[0];
		return row_name;
	}
	else if (func_str.find("GetEnabled") != std::string::npos) {
		std::vector<std::string> split_str = split_string(func_str, "GetEnabled");
		row_name = "en" + split_str[0];
		return row_name;
	}
	else if (func_str.find("Get") != std::string::npos) {
		// Assume rule names has been changed and rules editor bug was not fixed by then.
		// so refer as result by default.
		std::vector<std::string> split_str = split_string(func_str, "Get");
		if (split_str[0][0] == '(')
		{//Getting a standard ROW
			std::string str;
			row_name = split_str[0].substr(1, split_str[0].rfind(')')-1);
		}
		else
			row_name = "r" + split_str[0];

		return row_name;
	}
	else {
		return row_name;
	}
}
