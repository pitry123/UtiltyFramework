#pragma once
#include <iostream>
#include <functional>

class rules_defs
{
public:

	//This struct holds plain data from the rule.xml file
	struct true_false_task_struct
	{
		std::string task_field;			// string representation of the Enum
		std::string task_state;			// string representation the Enum Value should be after evaluation
		std::string task_notify;			// string representation when to publish OnChange/Always
		std::string task_timeout;			// string representation of the milliseconds to wait before publish
	};

	//This struct represent a parsed struct holds by some rule
	struct task_struct
	{
		size_t rule_id;
		size_t task_id;
		size_t execution_timeout_interval;
		std::function<void()> func;
	};


};