#include "rules_dispatcher_impl.h"
#include <rules/rules.h>
#include "rules_concrete_generic.h"
#include <utils/application.hpp>

rules_dispatcher_impl::rules_dispatcher_impl(
	const char* file_path,
	core::rules::rules_data_and_types_interface* rules_data_and_types) :
	utils::database::database_dispatcher_base<utils::ref_count_base<rules::rules_dispatcher>>("Rules_Disp", true),
	m_rules_Xml_file_path(file_path),
	m_rules_data_and_types(rules_data_and_types)

{
	m_rules_vector = std::make_shared<std::vector<utils::ref_count_ptr<rule>>>();
	m_rule_parser = std::unique_ptr<rules_parser>(new rules_parser(this, m_rules_Xml_file_path, m_rules_vector));
	if (m_rule_parser->load())
		create_rules_mechanism();
	
}

rules_dispatcher_impl::rules_dispatcher_impl(const char* file_path,
											 core::database::table_interface* input_table,
											 core::database::table_interface* enable_table,
											 core::database::table_interface* existence_table,
											 core::database::table_interface* output_table,
											 core::database::table_interface* management_table,
											 core::parsers::binary_metadata_store_interface* store) :
	utils::database::database_dispatcher_base<utils::ref_count_base<rules::rules_dispatcher>>("Rules_Disp", true),
	m_rules_Xml_file_path(file_path)
{
	utils::ref_count_ptr<core::database::dataset_interface> dataset;

	if (false == input_table->query_parent(&dataset))
		throw std::runtime_error("no dataset parent");
		
	m_rules_vector = std::make_shared<std::vector<utils::ref_count_ptr<rule>>>();
	m_rule_parser = std::unique_ptr<rules_parser>(new rules_parser(this, m_rules_Xml_file_path, m_rules_vector));
	
	m_rules_data_and_types = utils::make_ref_count_ptr<rules_concrete_generic>(
		dataset,
		input_table,
		enable_table,
		existence_table,
		output_table,
		management_table,
		store);

	if(m_rule_parser->load())
		create_rules_mechanism();
	
}

rules_dispatcher_impl::rules_dispatcher_impl(const char * file_path,
	core::database::dataset_interface* dataset,
	core::parsers::binary_metadata_store_interface * store):
	utils::database::database_dispatcher_base<utils::ref_count_base<rules::rules_dispatcher>>("Rules_Disp", true),
	m_rules_Xml_file_path(file_path)
{
	m_rules_vector = std::make_shared<std::vector<utils::ref_count_ptr<rule>>>();
	m_rule_parser = std::unique_ptr<rules_parser>(new rules_parser(this, m_rules_Xml_file_path, m_rules_vector));
	if (false == m_rule_parser->load())
		throw std::runtime_error("failed to load rules.xml");

	utils::ref_count_ptr<core::database::table_interface> input_table;
	utils::ref_count_ptr<core::database::table_interface> enable_table;
	utils::ref_count_ptr<core::database::table_interface> existence_table;
	utils::ref_count_ptr<core::database::table_interface> output_table;
	utils::ref_count_ptr<core::database::table_interface> management_table;

	if (false == m_rule_parser->query_rules_tables(dataset, 
		&input_table, &enable_table, &existence_table, &output_table, &management_table))
	{
		throw std::runtime_error("error getting rules tables");
	}

	m_rules_data_and_types = utils::make_ref_count_ptr<rules_concrete_generic>(
		dataset,
		input_table,
		enable_table,
		existence_table,
		output_table,
		management_table,
		store);

	create_rules_mechanism();

}

rules_dispatcher_impl::~rules_dispatcher_impl()
{
	stop();
}

void rules_dispatcher_impl::create_rules_mechanism()
{
	m_rule_parser->parse_rules();

	for (auto& it : *m_rules_vector)
	{
		if (false == m_rules_data_and_types->add_rule_enable_row(it->id(), it->name().c_str()))
			throw std::runtime_error("failed to add_rule_enable_row");
		if (false == m_rules_data_and_types->add_rule_existence_row(it->id(), it->name().c_str()))
			throw std::runtime_error("failed to add_rule_existence_row");
	}
	
}

void rules_dispatcher_impl::print_rules()
{
	for (auto & rule : *m_rules_vector)
	{
		rule->print_rule();
		printf("\n");
	}
}

std::function<double()> rules_dispatcher_impl::get_function(const std::string& str_func)
{
	utils::ref_count_ptr<core::rules::rule_func_callback_interface> callback;
	utils::rules::rules_callback_func func;
	if (m_rules_data_and_types->query_function(str_func.c_str(), &callback))
	{
		func = ([callback]() {
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

bool rules_dispatcher_impl::try_get_enum_value(const std::string &enum_name, const std::string& enum_val, int64_t& return_val)
{
	return m_rules_data_and_types->get_enumeration(enum_val.c_str(),enum_name.c_str(),return_val);
}

bool rules_dispatcher_impl::try_row_int_value(std::string &str_row, core::database::row_interface* row,int64_t* val)
{
	if (utils::types::is_integer(str_row))
	{
		*val = std::stoll(str_row);
		return true;
	}
	else
	{
		if (row != nullptr)
			return m_rules_data_and_types->get_enumeration(str_row.c_str(), row->info().type_name, *val);
		else
			return m_rules_data_and_types->get_enumeration(str_row.c_str(), *val);
	}
}

bool rules_dispatcher_impl::try_row_float_value(std::string &str_row, core::database::row_interface* row, float* val)
{
	char *ptr;
	double ret;

	ret = std::strtod(str_row.c_str(), &ptr);
	if (ptr != nullptr)
	{
		//Not a number
		return false;
	}
	*val = static_cast<float>(ret);

	return true;
}

bool rules_dispatcher_impl::query_row_by_string(const std::string &str_row, core::database::row_interface** row)
{
	return m_rules_data_and_types->query_row_by_string(str_row.c_str(), row);
	
}

bool rules_dispatcher_impl::publish_rule_existence(size_t rule_id, bool is_existence)
{
	utils::ref_count_ptr<core::database::row_interface> row;
	if (m_rules_data_and_types->query_rule_existence_row(rule_id, &row) == false)
		return false;

	rules::RulesDefs::RulesExistence existence = (is_existence == true) ?
		rules::RulesDefs::RulesExistence::RULE_TRUE : rules::RulesDefs::RulesExistence::RULE_FALSE;
	
	row->write_bytes(&existence, sizeof(rules::RulesDefs::RulesExistence), true, 0);
	return true;
}

void rules_dispatcher_impl::register_for_timer(std::shared_ptr<rules_defs::task_struct> task)
{
	utils::timer_registration_params timer_token =  register_timer(static_cast<double>(task->execution_timeout_interval), task->func, 1);
	m_subscription_task_timer_tokens[task->rule_id][task->task_id] = timer_token;
}

bool rules_dispatcher_impl::unregister_for_timer(std::shared_ptr<rules_defs::task_struct> task)
{
	// isExist == true
	if (m_subscription_task_timer_tokens.find(task->rule_id) != m_subscription_task_timer_tokens.end())
	{
		return unregister_timer(m_subscription_task_timer_tokens[task->rule_id][task->task_id]);
	}
	return false;
}

bool rules_dispatcher_impl::subscribe_all_rules_triggers()
{
	bool ret_val = true;
	for (auto const rule : *m_rules_vector)
	{
		auto rule_id = rule->id();
		ret_val = ret_val & subscribe_rule_triggers(rule_id);
	}
	return ret_val;
}


bool rules_dispatcher_impl::subscribe_rule_enabled(utils::ref_count_ptr<rule> curr_rule)
{
	utils::ref_count_ptr<core::database::row_interface> row;
	size_t rule_id = curr_rule->id();
	if (m_rules_data_and_types->query_rule_enable_row(rule_id, &row) == false)
		return false;
	
	//Database::Row row = m_rules_data_and_types.GetRuleEnableRow(rule_id);

	utils::database::subscription_params token =
		subscribe(row, [=](const utils::database::row_data& reader)
	{
		bool is_ok = false;
		auto enable_state = reader.read<rules::RulesDefs::RulesEnabled>();

		if (enable_state == rules::RulesDefs::RulesEnabled::RULE_ENABLE)
		{
			is_ok = subscribe_rule_triggers(rule_id);
		}
		else if (enable_state == rules::RulesDefs::RulesEnabled::RULE_DISABLE)
		{
			is_ok = unsubscribe_rule_triggers(rule_id);
			is_ok = is_ok | unsubscribe_rule_tasks_timers(rule_id);
		}
		if (is_ok)
			curr_rule->handle_enable_state(enable_state);

		// Done: guy - unsubscribe tasks timers
	});

	m_subscription_enable_tokens.emplace(rule_id, token);
	return true;
}

bool rules_dispatcher_impl::subscribe_rule_enabled(size_t rule_id)
{
	for (auto const rule : *m_rules_vector)
	{
		if (rule->id() == rule_id)
		{
			subscribe_rule_enabled(rule);
			return true;
		}
	}

	return false;
}

bool rules_dispatcher_impl::subscribe_all_rules_enable()
{
	
	for (auto rule : *m_rules_vector)
	{
		subscribe_rule_enabled(rule);
	}
	return true;
}

bool rules_dispatcher_impl::query_rule_by_id(size_t rule_Id, utils::ref_count_ptr<rule>& curr_rule)
{
	for (auto rule : *m_rules_vector)
	{
		if (rule->id() == rule_Id)
		{
			curr_rule = rule;
			return true;
		}
	}

	return false;
}

bool rules_dispatcher_impl::subscribe_rule_triggers(size_t rule_Id)
{
	// Description: subscribe to all rules triggers only if there is no prior subscription
	if (m_subscription_triggers_tokens[rule_Id].empty() == false)
		return false;

	utils::ref_count_ptr<rule> curr_rule;
	if (false == query_rule_by_id(rule_Id,curr_rule))
		return false;

	auto triggers = curr_rule->get_triggers();
	
	for (auto trigger : *triggers)
	{
		utils::ref_count_ptr<core::database::row_interface> row;
		if (query_row_by_string(trigger, &row) == false)
			return false;

		utils::database::subscription_params token = subscribe(row, [=](const utils::database::row_data& reader)
		{
			/*bool res = */curr_rule->evaluate();
		});
		m_subscription_triggers_tokens[rule_Id].emplace(trigger, token);
	}
	return true;
}

bool rules_dispatcher_impl::unsubscribe_rule_triggers(size_t rule_Id)
{
	if (m_subscription_triggers_tokens[rule_Id].empty())
		return false;

	utils::ref_count_ptr<rule> curr_rule;
	if (false == query_rule_by_id(rule_Id, curr_rule))
		return false;

	auto triggersVector = curr_rule->get_triggers();
	
	for (auto strTrigger : *triggersVector)
	{
		utils::ref_count_ptr<core::database::row_interface> row;

		if (query_row_by_string(strTrigger,&row) == false)
			return false;

		auto token = m_subscription_triggers_tokens[rule_Id][strTrigger];
		unsubscribe(row, token);
		m_subscription_triggers_tokens[rule_Id].erase(strTrigger);
	}
	return true;
}

bool rules_dispatcher_impl::unsubscribe_rule_tasks_timers(size_t rule_Id)
{
	if (m_subscription_task_timer_tokens[rule_Id].empty())
		return false;
	bool ret_val = true;
	for (auto& taskId_subscriptionToken_pair : m_subscription_task_timer_tokens[rule_Id])
	{
		ret_val = ret_val & unregister_timer(taskId_subscriptionToken_pair.second);
	}

	m_subscription_task_timer_tokens[rule_Id].clear();
	return ret_val;
}

bool rules_dispatcher_impl::unsubscribe_rule_enable(size_t rule_Id)
{
	if (m_subscription_enable_tokens.empty())
		return false;
	utils::ref_count_ptr<rule> curr_rule;
	if (false == query_rule_by_id(rule_Id,curr_rule))
		return false;

	utils::ref_count_ptr<core::database::row_interface> row;
	if (m_rules_data_and_types->query_rule_enable_row(rule_Id, &row) == false)
		return false;

	unsubscribe(row, m_subscription_enable_tokens[rule_Id]);
	m_subscription_enable_tokens.erase(rule_Id);
	return true;
}

void rules_dispatcher_impl::reload_new_rules()
{
	for (auto const rule : *m_rules_vector)
	{
		auto nRuleId = rule->id();
		unsubscribe_rule_triggers(nRuleId);
		unsubscribe_rule_tasks_timers(nRuleId);
		unsubscribe_rule_enable(nRuleId);
	}

	m_subscription_task_timer_tokens.clear();
	m_subscription_triggers_tokens.clear();
	m_subscription_enable_tokens.clear();

	m_rules_vector->clear();

	m_rule_parser->reload();
	m_rule_parser->parse_rules();

	subscribe_all_rules_triggers();
	subscribe_all_rules_enable();
}

void rules_dispatcher_impl::init()
{ //do nothing here

}

void rules_dispatcher_impl::start()
{
	subscribe_all_rules_triggers();
	subscribe_all_rules_enable();
	utils::ref_count_ptr<core::database::row_interface> row;
	m_rules_data_and_types->query_reload_rules_row(&row);
	subscribe(row, [this](const utils::database::row_data& data)
	{
		reload_new_rules();
	});
}

void rules_dispatcher_impl::stop()
{
	// Do Nothing...
}

int rules_dispatcher_impl::add_rule(files::xml_element_interface * rule_element)
{
	int index = m_rule_parser->add_rule(rule_element);
	if (index == -1)
		return index;

    auto it = m_rules_vector->at(static_cast<size_t>(index));
	m_rules_data_and_types->add_rule_enable_row(it->id(), it->name().c_str());
	m_rules_data_and_types->add_rule_existence_row(it->id(), it->name().c_str());

	subscribe_rule_triggers(it->id());
	subscribe_rule_enabled(it->id());

	return static_cast<int>(it->id());
}

bool rules::rules_dispatcher::create(
	const char* file_path,
	core::rules::rules_data_and_types_interface* rules_data_and_types,
	rules::rules_dispatcher** runnable)
{
	if (rules_data_and_types == nullptr ||
		runnable == nullptr)
	{
		return false;
	}

	utils::ref_count_ptr<rules::rules_dispatcher> instance;
	try
	{
		instance = utils::make_ref_count_ptr<rules_dispatcher_impl>(
			file_path,
			rules_data_and_types);
	}
	catch (std::exception& e)
	{
		utils::color_print(true, true, core::console::colors::RED, "exception on rules::rules_dispatcher::create- error:%s\n", e.what());
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*runnable = instance;
	return true;
}

bool 
rules::rules_dispatcher::create(const char* file_path,
								core::database::table_interface * input_table,
								core::database::table_interface * enable_table,
								core::database::table_interface * existence_table,
								core::database::table_interface * output_table,
								core::database::table_interface * management_table,
								core::parsers::binary_metadata_store_interface* store,
								rules::rules_dispatcher** runnable)
{

	// Allow empty pointer for management_table just for old rules xml's.
	// (which are not supported by this feature)

	if (input_table == nullptr ||
		enable_table == nullptr ||
		existence_table == nullptr ||
		output_table == nullptr ||
		store == nullptr ||
		runnable == nullptr)
	{
		return false;
	}

	utils::ref_count_ptr<rules::rules_dispatcher> instance;

	try
	{
		instance = utils::make_ref_count_ptr<rules_dispatcher_impl>(file_path,
																	input_table,
																	enable_table,
																	existence_table,
																	output_table,
																	management_table,
																	store);
	}
	catch (std::exception& e)
	{
		utils::color_print(true, true, core::console::colors::RED, "exception on rules::rules_dispatcher::create- error:%s\n", e.what());
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*runnable = instance;
	return true;
}

bool rules::rules_dispatcher::create(const char* file_path,
	core::database::dataset_interface* dataset,
	core::parsers::binary_metadata_store_interface* store,
	rules_dispatcher** runnable)
{

	if (dataset == nullptr ||
		store == nullptr ||
		runnable == nullptr)
	{
		return false;
	}

	utils::ref_count_ptr<rules::rules_dispatcher> instance;

	try
	{
		instance = utils::make_ref_count_ptr<rules_dispatcher_impl>(file_path,
			dataset,
			store);
	}
	catch (std::exception& e)
	{
		utils::color_print(true, true, core::console::colors::RED, "exception on rules::rules_dispatcher::create- error:%s\n", e.what());
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*runnable = instance;
	return true;
}
