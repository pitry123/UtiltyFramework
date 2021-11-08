#include "rules_parser.h"
#include "node.h"
#include "rules_dispatcher_impl.h"
#include <utils/application.hpp>
// for the xml parsing
#include <locale>         // std::locale, std::isdigit

rules_parser::rules_parser(
	rules_dispatcher_impl* rules_dispatcher_impl,
	std::string rules_xml_file_path,
	std::shared_ptr<std::vector<utils::ref_count_ptr<rule>>> rulesVector) :
	m_rules_dispatcher(rules_dispatcher_impl),
    m_path_to_rules_xml(rules_xml_file_path),
    m_rules_vector(rulesVector),
	m_is_loaded(false)
{
	
}

bool rules_parser::load()
{
	if (m_is_loaded)
		return true;
	//if not path - just create the rule dispatcher
	if (m_path_to_rules_xml.empty() || 
		m_path_to_rules_xml == "")
	{
		utils::color_print(false, false, core::console::colors::WHITE, "m_rule_parser->load() empty xml file - no rule were loaded\n");
		return false;
	}
	// Load rules xml file
	if (files::xml_file_interface::create(m_path_to_rules_xml.c_str(), &m_rules_xml_file) == false)
	{
		utils::color_print(false, false, core::console::colors::RED, "m_rule_parser->load() failed to load rule file\n");
		return false;
	}
	m_is_loaded = true;
	return true;
}

bool rules_parser::reload()
{
	m_rules_xml_file = nullptr;
	m_is_loaded = false;
	return load();
}
bool rules_parser::query_rules_tables(const core::database::dataset_interface * dataset, core::database::table_interface ** input_table, core::database::table_interface ** enable_table, core::database::table_interface ** existence_table, core::database::table_interface ** output_table, core::database::table_interface ** management_table)
{
	if (false == m_is_loaded)
		return false;
	if (dataset == nullptr ||
		input_table == nullptr ||
		enable_table == nullptr ||
		existence_table == nullptr ||
		output_table == nullptr ||
		management_table == nullptr)
		return false;

	// Get the root element <Rules></Rules>
	std::string root = "/Rules";
	utils::ref_count_ptr<files::xml_element_interface> root_node;
	if (m_rules_xml_file->query_element(root.c_str(), &root_node) == false)
		return false;

	utils::ref_count_ptr<files::xml_element_interface> elem;

	if (false == root_node->query_child("InputDataBase", &elem))
		return false;
	else
	{
		if (false == dataset->query_table_by_name(elem->value(), input_table))
			return false;
	}
	elem = nullptr;
	if (false == root_node->query_child("OutputDataBase", &elem))
		return false;
	else
	{
		if (false == dataset->query_table_by_name(elem->value(), output_table))
			return false;
	}

	elem = nullptr;
	if (false == root_node->query_child("RulesExistenceDataBase", &elem))
		return false;
	else
	{
		if (false == dataset->query_table_by_name(elem->value(), existence_table))
			return false;
	}

	elem = nullptr;
	if (false == root_node->query_child("RulesEnabledDataBase", &elem))
		return false;
	else
	{
		if (false == dataset->query_table_by_name(elem->value(), enable_table))
			return false;
	}

	dataset->query_table_by_name("RulesManagementDBEnum", management_table);
	
	return true;
}
std::shared_ptr<std::pair<node::node_type_enum, std::string>>
rules_parser::get_operator_pair_from_string(const std::string& _operator)
{
	std::shared_ptr<std::pair<node::node_type_enum, std::string>> res;
	if (_operator == "&&")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_AND, "&&");
	}
	else if (_operator == "||")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_OR, "||");
	}
	else if (_operator == "!")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_NOT, "!");
	}
	else if (_operator == "==")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_EQUAL, "==");
	}
	else if (_operator == "!=")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_NOT_EQUAL, "!=");
	}
	else if (_operator == "<")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_LT, "<");
	}
	else if (_operator == "<=")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_LT, "<=");
	}
	else if (_operator == ">")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_GT, ">");
	}
	else if (_operator == ">=")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_GT, ">=");
	}	
	else if (_operator == "(")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_LEFT_BRAKET, "(");
	}
	else if (_operator == ")")
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERATOR_RIGHT_BRAKET, ")");
	}
	else
	{
		std::cout << __func__ << ": unknown operator." << std::endl;
		throw std::logic_error("unknown operator.");
	}

	return res;
}

std::shared_ptr<std::pair<node::node_type_enum, std::string>>
rules_parser::get_function_pair_from_string(const std::string& function_name)
{
	std::shared_ptr<std::pair<node::node_type_enum, std::string>> res =
		std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERAND_FUNCTION, function_name);

	return res;
}

std::shared_ptr<std::pair<node::node_type_enum, std::string>>
rules_parser::get_enumeration_pair_from_string(const std::string& enumeration_name)
{
	std::shared_ptr<std::pair<node::node_type_enum, std::string>> res;
	std::locale loc;
	if (std::isdigit(enumeration_name[0], loc))
	{
		res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERAND_CONSTANT, enumeration_name);
	}
	else 
	{
		// Check whether the enum num is a function.
		size_t index =  enumeration_name.find("Get");
		if(index == std::string::npos || index != 0)
			res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERAND_ENUM, enumeration_name);
		else
			res = std::make_shared<std::pair<node::node_type_enum, std::string>>(node::OPERAND_FUNCTION, enumeration_name);
	}

	return res;
}

void
rules_parser::get_rule_triggers(utils::ref_count_ptr<files::xml_element_interface> triggers_Ptree, std::shared_ptr<std::vector<std::string>> triggers_vector)
{
	utils::ref_count_ptr<files::xml_element_interface> trigger_entry;
	size_t i = 0;
	//for (auto &trigger_entry : triggers_Ptree.Children())
	while(triggers_Ptree->query_child(i,&trigger_entry))
	{
		triggers_vector->push_back(std::string(trigger_entry->value()));
		trigger_entry = nullptr;
		i++;

	}
}

void
rules_parser::get_rule_true_tasks(utils::ref_count_ptr<files::xml_element_interface> true_task_PTree, std::shared_ptr<std::vector<rules_defs::true_false_task_struct>> true_tasks_vector)
{
	utils::ref_count_ptr<files::xml_element_interface> true_task;
	size_t i = 0;
	while(true_task_PTree->query_child(i,&true_task))
	{
		rules_defs::true_false_task_struct true_task_struct;
		utils::ref_count_ptr<files::xml_element_interface> true_task_entry;
		size_t j = 0;
		while (true_task->query_child(j, &true_task_entry))
		{
			if (std::string(true_task_entry->name()).compare("True_Task_Field") == 0)
			{
				true_task_struct.task_field = std::string(true_task_entry->value());
			}
			else if (std::string(true_task_entry->name()).compare("True_Task_State") == 0)
			{
				true_task_struct.task_state = std::string(true_task_entry->value());
			}
			else if (std::string(true_task_entry->name()).compare("True_Task_Notify") == 0)
			{
				true_task_struct.task_notify = std::string(true_task_entry->value());
			}
			else if (std::string(true_task_entry->name()).compare("True_Task_TimeOut") == 0)
			{
				true_task_struct.task_timeout = std::string(true_task_entry->value());
			}
			true_task_entry = nullptr;
			j++;
		}
		true_tasks_vector->push_back(true_task_struct);
		true_task = nullptr;
		i++;
	}
}

void
rules_parser::get_rule_false_tasks(utils::ref_count_ptr<files::xml_element_interface> false_task_PTree, std::shared_ptr<std::vector<rules_defs::true_false_task_struct>> false_tasks_vector)
{
	utils::ref_count_ptr<files::xml_element_interface> false_task;
	size_t i = 0;
	while(false_task_PTree->query_child(i,&false_task))
	{

		rules_defs::true_false_task_struct false_task_struct;
		utils::ref_count_ptr<files::xml_element_interface> false_task_entry;
		size_t j = 0;
		while (false_task->query_child(j, &false_task_entry))
		{
			if (std::string(false_task_entry->name()).compare("False_Task_Field") == 0)
			{
				false_task_struct.task_field = std::string(false_task_entry->value());
			}
			if (std::string(false_task_entry->name()).compare("False_Task_State") == 0)
			{
				false_task_struct.task_state = std::string(false_task_entry->value());
			}
			if (std::string(false_task_entry->name()).compare("False_Task_Notify") == 0)
			{
				false_task_struct.task_notify = std::string(false_task_entry->value());
			}
			if (std::string(false_task_entry->name()).compare("False_Task_TimeOut") == 0)
			{
				false_task_struct.task_timeout = std::string(false_task_entry->value());
			}
			false_task_entry = nullptr;
			j++;
		}
		false_tasks_vector->push_back(false_task_struct);
		false_task = nullptr;
		i++;
	}
}

int
rules_parser::add_rule(files::xml_element_interface* rule_child)
{
	
	// Get Rule's attributes
	utils::ref_count_ptr<files::xml_element_interface> child;
	if (rule_child->query_child("Id", &child) == false)
	{
		throw std::runtime_error("Error parsing rules.xml ; no ID in Rule: ");
		return -1;
	}
	int	nId = child->value_as_int(-1);
	if (nId == -1) {
		throw std::runtime_error("Error parsing rules.xml ; RuleId incorrect in Rule: ");
		return -1;
	}
	child = nullptr;

	std::string rule_name;
	if (rule_child->query_child("Name", &child))
	{

		rule_name = std::string(child->value());
		child = nullptr;
	}
	std::string rule_form;
	if (rule_child->query_child("Form", &child))
	{
		rule_form = std::string(child->value());
		child = nullptr;
	}
	std::string rule_comment;
	if (rule_child->query_child("Comment", &child))
	{
		rule_comment = std::string(child->value());
		child = nullptr;
	}

	// Get Rule's expression
	utils::ref_count_ptr<files::xml_element_interface> expression_child;
	if (rule_child->query_child("Expression", &expression_child) == false)
	{
		throw std::runtime_error("Error parsing rules.xml ; no \"Expression\" in Rule: ");
		return -1;
	}
	std::shared_ptr<std::vector<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>> expr_vector =
		std::make_shared<std::vector<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>>();
	get_rule_expression(expression_child, expr_vector);

	utils::ref_count_ptr<files::xml_element_interface> triggers_child;
	rule_child->query_child("Triggers", &triggers_child);
	std::shared_ptr<std::vector<std::string>> triggers_vector = std::make_shared<std::vector<std::string>>();
	get_rule_triggers(triggers_child, triggers_vector);


	utils::ref_count_ptr<files::xml_element_interface> true_tasks_child;
	rule_child->query_child("True_Tasks",&true_tasks_child);
	std::shared_ptr<std::vector<rules_defs::true_false_task_struct>> true_tasks_vector = std::make_shared<std::vector<rules_defs::true_false_task_struct>>();
	get_rule_true_tasks(true_tasks_child, true_tasks_vector);

	utils::ref_count_ptr<files::xml_element_interface> false_tasks_child;
	rule_child->query_child("False_Tasks",&false_tasks_child);
	std::shared_ptr<std::vector<rules_defs::true_false_task_struct>> false_tasks_vector = std::make_shared<std::vector<rules_defs::true_false_task_struct>>();
	get_rule_false_tasks(false_tasks_child, false_tasks_vector);

    utils::ref_count_ptr<rule> newRule = utils::make_ref_count_ptr<rule>(m_rules_dispatcher, static_cast<size_t>(nId), rule_name, rule_form, rule_comment, expr_vector, triggers_vector, true_tasks_vector, false_tasks_vector);
	m_rules_vector->push_back(newRule);

	return static_cast<int>(m_rules_vector->size()-1); //return the index of the vector
}

void
rules_parser::get_rule_expression(utils::ref_count_ptr<files::xml_element_interface> rules_Ptree, std::shared_ptr<std::vector<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>> vecExpr)
{
	// Iterate over ExpressionRules
	size_t i = 0;
	utils::ref_count_ptr<files::xml_element_interface> rule_entry;
	while(rules_Ptree->query_child(i,&rule_entry))
	{
		if (std::string(rule_entry->name()).compare("Condition") == 0)
		{
			utils::ref_count_ptr<files::xml_element_interface> condition_entry;
			size_t j = 0;
			while (rule_entry->query_child(j, &condition_entry))
			{
				if (std::string(condition_entry->name()).compare("Operator") == 0)
				{
					vecExpr->push_back(get_operator_pair_from_string(std::string(condition_entry->value())));
				}
				else if (std::string(condition_entry->name()).compare("Function") == 0)
				{
					vecExpr->push_back(get_function_pair_from_string(std::string(condition_entry->value())));
				}
				else if (std::string(condition_entry->name()).compare("Enumeration") == 0)
				{
					vecExpr->push_back(get_enumeration_pair_from_string(std::string(condition_entry->value())));
				}
				condition_entry = nullptr;
				j++;
			}
		}
		else if (std::string(rule_entry->name()).compare("Middle_Operator") == 0)
		{
			vecExpr->push_back(get_operator_pair_from_string(std::string(rule_entry->value())));
		}
		rule_entry = nullptr;
		i++;
	}
}

void rules_parser::parse_rules()
{
	if (false == m_is_loaded)
		return;	//if xml not loaded - do not parse 
	
	// Get the root element <Rules></Rules>
	std::string root = "/Rules";
	utils::ref_count_ptr<files::xml_element_interface> root_node;
	if(m_rules_xml_file->query_element(root.c_str(),&root_node) == false)
		throw std::runtime_error("Rules root element is missing \"<Rules>...s</Rules>\"");
	
	// Iterate over Rules
	size_t i = 0;
	utils::ref_count_ptr<files::xml_element_interface> elem;
	while (root_node->query_child(i,&elem))
	{
		if (std::strcmp(elem->name(), "Category") == 0)
		{
			utils::ref_count_ptr<files::xml_element_interface> category_child;
			size_t j = 0;
			while (elem->query_child(j, &category_child))
			{
				if (std::strcmp(category_child->name(), "Rule") == 0)
				{
					add_rule(category_child);
				}
				
				category_child = nullptr;
				j++;
			}
		}
			
		elem = nullptr;
		i++;
	}
	
}

