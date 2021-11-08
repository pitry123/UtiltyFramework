#pragma once
#include <core/ref_count_interface.h>
#include <utils/ref_count_base.hpp>
#include "rules_defs.h"
#include "node.h"
#include "expression.h"
#include <rules/rules.h>
#include <core/parser.h>

#include <utils/ref_count_ptr.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <memory>


class rules_dispatcher_impl;

class rule : public utils::ref_count_base<core::ref_count_interface>
{
public:
	rule(
		rules_dispatcher_impl* rules_dispatcher,
		size_t id,
		std::string name,
		std::string form,
		std::string comment,
		std::shared_ptr<std::vector<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>> tagged_expression_vector,
		std::shared_ptr<std::vector<std::string>> triggers_vector,
 		std::shared_ptr<std::vector<rules_defs::true_false_task_struct>> true_tasks_vector,
 		std::shared_ptr<std::vector<rules_defs::true_false_task_struct>> false_tasks_vector
	);
	~rule();

	void		print_rule() const;
	size_t		id()		const;
	std::string name()	const;

	std::shared_ptr<std::vector<std::string>> get_triggers()	{ return m_triggers_vector; };
	void handle_enable_state(rules::RulesDefs::RulesEnabled enable_state);
	bool evaluate();

private:
	utils::ref_count_ptr<rules_dispatcher_impl> m_rules_dispatcher;

	size_t		m_rule_id;
	std::string m_name;
	std::string m_form;
	std::string m_comment;
	std::shared_ptr<std::vector<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>>		m_tagged_expression_vector;
	std::shared_ptr<std::vector<std::string>>														m_triggers_vector;
	std::vector<std::shared_ptr<rules_defs::task_struct>>											m_true_tasks_functions_vector;	// after parsing m_trueTasksVector
	std::vector<std::shared_ptr<rules_defs::task_struct>>											m_false_tasks_functions_vector;	// after parsing m_trueTasksVector

	expression  m_expression;

	std::function<void()> create_complex_task(core::database::row_interface* row, std::vector<std::string>& path_vector,
		const std::string& data, bool force_report);

	void parse_true_false_tasks(std::shared_ptr<std::vector<rules_defs::true_false_task_struct>>	&input_tasks_vector,
								std::vector<std::shared_ptr<rules_defs::task_struct>>				&output_tasks_vector);

	void perform_task(std::vector<std::shared_ptr<rules_defs::task_struct>> &tasks_vector, bool res);
	void cancel_task_timers(std::vector<std::shared_ptr<rules_defs::task_struct>>& tasks_vector);
};

