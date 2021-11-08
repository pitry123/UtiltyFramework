#pragma once
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <string>
#include <map>

class node
{
public:

	enum node_type_enum
	{
		OPERAND_NONE,
		OPERAND_CONSTANT,
		OPERAND_ENUM,
		OPERAND_FUNCTION,

		OPERATOR_EQUAL,			// ==
		OPERATOR_NOT_EQUAL,		// !=
		OPERATOR_LT,			// <
		OPERATOR_LTE,			// <=
		OPERATOR_GT,			// >
		OPERATOR_GTE,			// >=
		OPERATOR_RIGHT_BRAKET,	// (
		OPERATOR_LEFT_BRAKET,	// )

		OPERATOR_AND,			// &&
		OPERATOR_OR,			// ||
		OPERATOR_NOT,			// !

	};

	virtual double evaluate() = 0;

	node();
	virtual ~node();

    virtual std::string get_string() { return std::string(""); }

	std::shared_ptr<node> m_left;
	std::shared_ptr<node> m_right;

};


// TODO: guy
// Where should i put those maps
static std::map<node::node_type_enum, size_t> m_map_priority =
{
	{ node::OPERAND_CONSTANT,		 0 },
	{ node::OPERAND_ENUM,			 0 },
	{ node::OPERAND_FUNCTION,		 0 },
	{ node::OPERAND_NONE,			 0 },
	{ node::OPERATOR_RIGHT_BRAKET,   0 },
	{ node::OPERATOR_LEFT_BRAKET,	 0 },
	{ node::OPERATOR_NOT,			 5 },
	{ node::OPERATOR_EQUAL,			 4 },
	{ node::OPERATOR_NOT_EQUAL,		 4 },
	{ node::OPERATOR_LT,			 4 },
	{ node::OPERATOR_LTE,			 4 },
	{ node::OPERATOR_GT,			 4 },
	{ node::OPERATOR_GTE,			 4 },
	{ node::OPERATOR_AND,			 3 },
	{ node::OPERATOR_OR,			 2 }
};

static std::map<node::node_type_enum, bool> m_map_left_assoc =
{
	{ node:: OPERAND_CONSTANT,		 false },
	{ node:: OPERAND_ENUM,			 false },
	{ node:: OPERAND_FUNCTION,		 false },
	{ node:: OPERAND_NONE,			 false },
	{ node:: OPERATOR_RIGHT_BRAKET,  false },
	{ node:: OPERATOR_LEFT_BRAKET,	 false },
	{ node:: OPERATOR_NOT,			 false },
	{ node:: OPERATOR_EQUAL,		 true },
	{ node:: OPERATOR_NOT_EQUAL,	 true },
	{ node:: OPERATOR_LT,			 true },
	{ node:: OPERATOR_LTE,			 true },
	{ node:: OPERATOR_GT,			 true },
	{ node:: OPERATOR_GTE,			 true },
	{ node:: OPERATOR_AND,			 true },
	{ node:: OPERATOR_OR,			 true }
};

static size_t get_priority(node::node_type_enum node_type)
{
	return m_map_priority[node_type];
}

static bool is_left_assoc(node::node_type_enum node_type)
{
	return m_map_left_assoc[node_type];
}