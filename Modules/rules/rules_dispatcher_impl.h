#pragma once
#include <utils/ref_count_ptr.hpp>
#include "rules_defs.h"
#include <utils/database.hpp>
#include <Logging.hpp>
#include <unordered_map>
#include <functional>
#include <core/parser.h>
#include "rules_parser.h"

#include <utils/rules.hpp>
class rules_dispatcher_impl : public utils::database::database_dispatcher_base<utils::ref_count_base<rules::rules_dispatcher>>
{

public:
	// Build rules with the old native method.
	rules_dispatcher_impl(const char* file_path,
						  core::rules::rules_data_and_types_interface* rules_data_and_types);

	// Build rules with data loader.
	rules_dispatcher_impl(const char* file_path,
						  core::database::table_interface* inputTable,
						  core::database::table_interface* enableTable,
						  core::database::table_interface* existenceTable,
						  core::database::table_interface* outputTable,
						  core::database::table_interface* managementTable,
						  core::parsers::binary_metadata_store_interface* store);

	// Build rules with data loader.
	rules_dispatcher_impl(const char* file_path,
		core::database::dataset_interface* dataset,
		core::parsers::binary_metadata_store_interface* store);

	~rules_dispatcher_impl();

	void print_rules();
	bool query_row_by_string(const std::string &strRow, core::database::row_interface** row);
	std::function<double()>	get_function(const std::string& strFunc);
	bool try_row_int_value(std::string &strRow, core::database::row_interface* row, int64_t* val);
	bool try_row_float_value(std::string &strRow, core::database::row_interface* row, float* val);
	bool try_get_enum_value(const std::string &enum_name, const std::string& enum_val, int64_t& return_val);

	virtual void init() override;
	virtual void start() override;
	virtual void stop() override;

	int add_rule(files::xml_element_interface* rule_element) override;

	bool unsubscribe_rule_triggers(size_t rule_Id);

	bool subscribe_rule_triggers(size_t rule_Id);
	bool subscribe_rule_enabled(size_t rule_id);
	bool subscribe_rule_enabled(utils::ref_count_ptr<rule> curr_rule);
	bool publish_rule_existence(size_t rule_id, bool is_existence);
	void register_for_timer(std::shared_ptr<rules_defs::task_struct> task);
	bool unregister_for_timer(std::shared_ptr<rules_defs::task_struct> task);

private:
	std::string											m_rules_Xml_file_path;
	std::shared_ptr<std::vector<utils::ref_count_ptr<rule>>> m_rules_vector;
	std::unique_ptr<rules_parser>						m_rule_parser;

	utils::ref_count_ptr<core::rules::rules_data_and_types_interface> m_rules_data_and_types;	

	std::unordered_map<size_t, std::unordered_map<size_t, utils::timer_registration_params>>			m_subscription_task_timer_tokens;
	std::unordered_map<size_t, std::unordered_map<std::string, utils::database::subscription_token>>	m_subscription_triggers_tokens;
	std::unordered_map<size_t, utils::database::subscription_token>										m_subscription_enable_tokens;
	
	bool query_rule_by_id(size_t rule_Id, utils::ref_count_ptr<rule>& curr_rule);
	bool subscribe_all_rules_triggers();
	bool subscribe_all_rules_enable();
	bool unsubscribe_rule_tasks_timers(size_t rule_id);
	bool unsubscribe_rule_enable(size_t rule_id);
	void reload_new_rules();
	void create_rules_mechanism();
	
};
