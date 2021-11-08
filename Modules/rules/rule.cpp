#include "rule.h"
#include <utils/rules.hpp>
#include "rules_dispatcher_impl.h"
#include <utils/buffer_allocator.hpp>

rule::rule(rules_dispatcher_impl* rules_dispatcher,
		   size_t id,
		   std::string name,
		   std::string form,
		   std::string comment,
		   std::shared_ptr<std::vector<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>> tagged_expression_vector,
		   std::shared_ptr<std::vector<std::string>> triggers_vector,
		   std::shared_ptr<std::vector<rules_defs::true_false_task_struct>>	true_tasks_vector,
		   std::shared_ptr<std::vector<rules_defs::true_false_task_struct>>	false_tasks_vector) 
	: m_rules_dispatcher(rules_dispatcher),
	  m_rule_id(id),
	  m_name(name),
	  m_form(form),
	  m_comment(comment),
	  m_tagged_expression_vector(tagged_expression_vector),
	  m_triggers_vector(triggers_vector),
	  m_expression(m_rules_dispatcher, m_tagged_expression_vector)
{
	parse_true_false_tasks(true_tasks_vector, m_true_tasks_functions_vector);
	parse_true_false_tasks(false_tasks_vector, m_false_tasks_functions_vector);
}

rule::~rule()
{

}

void rule::print_rule() const
{
	std::cout << std::endl;
	std::cout << "Rule: " << m_name << " ; id:" << m_rule_id << std::endl;
	std::cout << "Rules Triggers: " << std::endl;

	for (auto token : *m_triggers_vector)
	{
		std::cout << token << std::endl;
	}
	std::cout << std::endl;

	for (auto token : *m_tagged_expression_vector)
	{
		std::cout << token->second;
	}
	std::cout << std::endl;
	m_expression.print_tree();

	std::cout << std::endl;
	std::cout << "Rule Result: " << m_expression.evaluate() << std::endl;
}

size_t rule::id() const
{
	return m_rule_id;
}

std::string rule::name() const
{
	return m_name;
}
std::function<void()> rule::create_complex_task(core::database::row_interface* row, std::vector<std::string>& path_vector, const std::string& data, bool force_report)
{
	size_t offset = 0;
	utils::ref_count_ptr<core::parsers::binary_parser_interface> parser;
	utils::ref_count_ptr < core::parsers::binary_metadata_interface> metadata;
	utils::ref_count_ptr<core::parsers::binary_node_interface> node;
	if (false == row->query_parser_metadata(&metadata))
		throw std::runtime_error("create_complex_task: failed to get metadata from row");
	metadata->create_parser(&parser);
	
	if(false == utils::rules::get_node_and_offset_by_path(parser, path_vector, &node, offset))
		throw std::runtime_error("create_complex_task: failed to get value offset");

	utils::ref_count_ptr<utils::ref_count_buffer> buffer = utils::make_ref_count_ptr<utils::ref_count_buffer>(row->data_size());
	double double_val;
	float  float_val;
	int64_t val = 0;
	if (utils::types::is_simple_type(node->type()))
	{
		if (node->type() == core::types::type_enum::DOUBLE)
		{
			double_val = std::stod(data);
			MEMCPY(&val,sizeof(val), &double_val, sizeof(double));
		}
		else if (node->type() == core::types::type_enum::FLOAT)
		{
			float_val = std::stof(data);
			MEMCPY(&val, sizeof(val), &float_val,sizeof(float));
		}
		else
		{
			val = std::stoll(data);
		}

	}
	else if (node->type() == core::types::ENUM)
	{
		utils::ref_count_ptr<core::parsers::enum_data_interface> enum_data;
		node->query_enum(&enum_data);
		m_rules_dispatcher->try_get_enum_value(enum_data->name(), data, val);
	}
	else //not a simple type
		throw std::runtime_error("create_complex_task: unsupported type");
	

	auto func = [=]() mutable
	{
		row->read_bytes(buffer->data(), buffer->size());
		node->write(&val, node->size(), &(buffer->data())[offset],buffer->size() - offset);
		row->write_bytes(buffer->data(), buffer->size(), force_report, 0);
	};

	return func;

}
void rule::parse_true_false_tasks(std::shared_ptr<std::vector<rules_defs::true_false_task_struct>>	&input_tasks_vector,
								  std::vector<std::shared_ptr<rules_defs::task_struct>>				&output_tasks_vector)
{
	size_t task_id = 0;

	for (auto &task : *input_tasks_vector)
	{
		size_t task_timeout_interval = 0;
		if (task.task_timeout.compare("0") != 0)
		{
            task_timeout_interval = static_cast<size_t>(std::atoi(task.task_timeout.c_str()));	// in milliseconds
		}

		utils::ref_count_ptr<core::database::row_interface> row;
		if (m_rules_dispatcher->query_row_by_string(task.task_field, &row) == false)
		{
			throw std::invalid_argument("task.task_field");
		}

		bool	force_report = task.task_notify.compare("Always") == 0 ? true : false;

		//	use GetRowValue because the row can be int/enumeration/float
		float	float_row_data = 0;
		int64_t		numerical_row_data = 0;

		std::function<void()> taskFunction = nullptr;
		if (row->info().type == core::types::type_enum::COMPLEX)
		{
			std::vector<std::string> path_vector = split_string(task.task_field, ".");
			if (path_vector.size() > 1)
			{
				path_vector.erase(path_vector.begin()); //remove the table row
			}

			taskFunction = create_complex_task(row, path_vector, task.task_state, force_report);
		}
		else
		{
			if (m_rules_dispatcher->try_row_int_value(task.task_state, row, &numerical_row_data))
			{
				// capture variables are 'const' by default
				// without mutable we won't be able to call non-const function of row (Write)
				taskFunction = [=]() mutable
				{
					row->write_bytes((const void*)&numerical_row_data, row->data_size(), force_report, 0);
				};
			}
			else if (m_rules_dispatcher->try_row_float_value(task.task_state, row, &float_row_data))
			{
				// capture variables are 'const' by default
				// without mutable we won't be able to call non-const function of row (Write)
				taskFunction = [=]() mutable
				{
					row->write_bytes((const void*)&float_row_data, sizeof(float_row_data), force_report, 0);
				};
			}
			else
			{
				std::cout << __func__ << ": " << task.task_state << " Is not an Enum or a Constant\n";
				throw std::logic_error(": " + task.task_state + "  Is not an Enum or a Constant");
			}
		}
		output_tasks_vector.emplace_back(std::make_shared<rules_defs::task_struct>(rules_defs::task_struct{ m_rule_id, task_id, task_timeout_interval, taskFunction }));
		task_id++;
	}
}

void rule::perform_task(std::vector<std::shared_ptr<rules_defs::task_struct>> &tasks_vector, bool res)
{
	for (auto task_struct : tasks_vector)
	{
		if (task_struct->execution_timeout_interval != 0)
		{
			m_rules_dispatcher->register_for_timer(task_struct);
		}
		else
		{
			task_struct->func();
		}
	}

	m_rules_dispatcher->publish_rule_existence(m_rule_id, res);
}

void rule::cancel_task_timers(std::vector<std::shared_ptr<rules_defs::task_struct>>& tasks_vector)
{
	for (auto task_struct : tasks_vector)
	{
		if (task_struct->execution_timeout_interval != 0)
			m_rules_dispatcher->unregister_for_timer(task_struct);
	}
}

void rule::handle_enable_state(rules::RulesDefs::RulesEnabled enable_state)
{
	// Reevaluate the rule if it become enabled
	if (enable_state == rules::RulesDefs::RULE_ENABLE)
		evaluate();
}

bool rule::evaluate()
{
	bool res = m_expression.evaluate();

	// Cancel all tasks timers
	cancel_task_timers(m_true_tasks_functions_vector);
	cancel_task_timers(m_false_tasks_functions_vector);

	if (res == true)
	{
		perform_task(m_true_tasks_functions_vector, true);
	}
	else
	{
		perform_task(m_false_tasks_functions_vector, false);
	}
	return res;
}
