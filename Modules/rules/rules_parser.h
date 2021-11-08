#pragma once
#include "rule.h"
#include "rules_defs.h"
#include <string>
#include <memory>
#include <vector>

// for the xml parsing
#include <Files.hpp>


class rules_dispatcher_impl;

class rules_parser
{
private:
	utils::ref_count_ptr<rules_dispatcher_impl> m_rules_dispatcher;
	std::string	m_path_to_rules_xml;
	std::shared_ptr<std::vector<utils::ref_count_ptr<rule>>> m_rules_vector;
	utils::ref_count_ptr<files::xml_file_interface> m_rules_xml_file;
	bool m_is_loaded;
public:
	rules_parser(
		rules_dispatcher_impl* rules_dispatcher_impl,
		std::string	rules_xml_file_path,
		std::shared_ptr<std::vector<utils::ref_count_ptr<rule>>> rules_vector);

	bool load();
	bool reload();
	bool query_rules_tables(const core::database::dataset_interface* dataset,
	core::database::table_interface** input_table,
	core::database::table_interface** enable_table,
	core::database::table_interface** existence_table,
	core::database::table_interface** output_table,
	core::database::table_interface** management_table);

	void parse_rules();
	int add_rule(files::xml_element_interface* elem);
private:
	void get_rule_triggers(
		utils::ref_count_ptr<files::xml_element_interface> triggers_ptree,
		std::shared_ptr<std::vector<std::string>> triggers_vector);

	void get_rule_true_tasks(utils::ref_count_ptr<files::xml_element_interface>  trueTask_ptree, std::shared_ptr<std::vector<rules_defs::true_false_task_struct>> true_tasks_vector);
	void get_rule_false_tasks(utils::ref_count_ptr<files::xml_element_interface> false_task_ptree, std::shared_ptr<std::vector<rules_defs::true_false_task_struct>> false_task_vector);

	void get_rule_expression(utils::ref_count_ptr<files::xml_element_interface> rules_Ptree, std::shared_ptr<std::vector<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>> vec_expr);

	std::shared_ptr<std::pair<node::node_type_enum, std::string>>	get_operator_pair_from_string(const std::string& _operator);
 	std::shared_ptr<std::pair<node::node_type_enum, std::string>>	get_function_pair_from_string(const std::string& function_name);
 	std::shared_ptr<std::pair<node::node_type_enum, std::string>>	get_enumeration_pair_from_string(const std::string& enumeration_name);
};


