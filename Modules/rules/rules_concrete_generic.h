#pragma  once
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/rules.hpp>
#include <utils/buffer_allocator.hpp>

#include <core/rules.h>
#include <core/parser.h>
#include <vector>
#include <map>
class rules_concrete_generic : public utils::rules::rules_data_and_types_base<core::rules::rules_data_and_types_interface>
{
private:
	utils::ref_count_ptr<core::database::dataset_interface> m_dataset;
	utils::ref_count_ptr<core::database::table_interface> m_input_table;
	utils::ref_count_ptr<core::database::table_interface> m_enable_table;
	utils::ref_count_ptr<core::database::table_interface> m_existence_table;
	utils::ref_count_ptr<core::database::table_interface> m_output_table;
	utils::ref_count_ptr<core::database::table_interface> m_management_table;
	utils::ref_count_ptr<core::parsers::binary_metadata_store_interface> m_store;
	std::map<size_t, utils::ref_count_ptr<core::database::row_interface>> m_rule_existence_map;
	std::map<size_t, utils::ref_count_ptr<core::database::row_interface>> m_rule_enabled_map;

	bool read_simple_val_from_comlex_val(utils::ref_count_ptr <core::parsers::binary_parser_interface>& parser,
		std::vector<std::string>& path_vector, core::types::type_enum& type, uint8_t data[]);
	bool query_row_by_string(const char* row_string, core::database::row_interface** row, std::vector<std::string>& path_vector);

	std::string get_row_name_by_prefix(const char* func_string);
	bool data_path_vector(const std::string& input, std::string& table, std::string& row, std::vector<std::string>& path_vector);
	std::function<double()> get_function(const char* func_string);
	bool add_enum_map(const char* enum_name);
	void normalize_rule_name(const char* row_name, const char* prefix, std::string& normalized_row_name);
public:

	rules_concrete_generic(
		core::database::dataset_interface* dataset,
		core::database::table_interface* input_table,
		core::database::table_interface* enable_table,
		core::database::table_interface* existence_table,
		core::database::table_interface* output_table,
		core::database::table_interface* management_table,
		core::parsers::binary_metadata_store_interface* store);

	rules_concrete_generic(
		core::database::dataset_interface* dataset,
		core::parsers::binary_metadata_store_interface* store);
	
	bool query_row_by_string(const char* row_string, core::database::row_interface** row) override;
	bool query_rule_existence_row(size_t rule_id, core::database::row_interface** row) override;
	bool add_rule_existence_row(size_t rule_id, const char* row_name) override;
	bool query_rule_enable_row(size_t rule_id, core::database::row_interface** row) override;
	bool add_rule_enable_row(size_t rule_id, const char* row_name) override;
	bool query_reload_rules_row(core::database::row_interface** row) override;
	bool get_enumeration(const char* enum_name_val, const char* enum_type_name, int64_t& val) override;
	bool query_function(const char* func_string, core::rules::rule_func_callback_interface** callback) override;
};