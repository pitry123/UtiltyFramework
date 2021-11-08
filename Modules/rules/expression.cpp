#include "expression.h"
#include "operands.h"
#include "operators.h"
#include "rules_dispatcher_impl.h"

expression::expression(
	rules_dispatcher_impl* rules_dispatcher,
	std::shared_ptr<std::vector<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>> vectorTokens) :
	m_rules_dispatcher(rules_dispatcher),
	m_vector_tokens(vectorTokens)
{
	build_parse_tree();
}

expression::~expression()
{
}

std::shared_ptr<node>
expression::make_operand(std::shared_ptr<std::pair<node::node_type_enum, std::string>> token) const
{
	std::shared_ptr<node> res_oper;

	switch (token->first)
	{
	case node::OPERAND_CONSTANT:
	{
		double operand_const;

		try
		{
			operand_const = std::stod(token->second);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << "\n" << __func__ << ": maybe \"" << token->second << "\" is not a Constant?" << std::endl;
			throw;
		}

		res_oper = std::make_shared<operand_constant>(operand_const);

		break;
	}
	case node::OPERAND_ENUM:
	{
		int64_t eOperand = 0;
		if (m_rules_dispatcher->try_row_int_value(token->second,nullptr, &eOperand) == false)
		{
			// If int not found assume it is a function.
			std::function<double()> operand_func = m_rules_dispatcher->get_function(token->second);
			res_oper = std::make_shared<operand_function>(operand_func);
		}
		else
			res_oper = std::make_shared<operand_enum>(static_cast<int>(eOperand));
		
		break;
	}
	case node::OPERAND_FUNCTION:
	{
		std::function<double()> operand_func = m_rules_dispatcher->get_function(token->second);
		res_oper = std::make_shared<operand_function>(operand_func);
		break;
	}
	default:
		break;
	}

	return res_oper;
}

std::shared_ptr<node>
expression::make_operator(node::node_type_enum oper) const
{
	std::shared_ptr<node> res_oper;

	switch (oper)
	{
	case node::OPERATOR_AND:
		res_oper = std::make_shared<operator_and>();
		break;
	case node::OPERATOR_OR:
		res_oper = std::make_shared<operator_or>();
		break;
	case node::OPERATOR_NOT:
		res_oper = std::make_shared<operator_not>();
		break;
	case node::OPERATOR_EQUAL:
		res_oper = std::make_shared<operator_equal>();
		break;
	case node::OPERATOR_GT:
		res_oper = std::make_shared<operator_greater_than>();
		break;
	case node::OPERATOR_GTE:
		res_oper = std::make_shared<operator_greater_than_equal>();
		break;
	case node::OPERATOR_LT:
		res_oper = std::make_shared<operator_less_than>();
		break;
	case node::OPERATOR_LTE:
		res_oper = std::make_shared<operator_less_than_equal>();
		break;
	case node::OPERATOR_NOT_EQUAL:
		res_oper = std::make_shared<operator_not_equal>();
		break;
	default:
		std::cout << __func__ << ": node_type_enum is not operator\n";
		throw std::logic_error("node_type_enum is not operator");
		break;
	}

	return res_oper;

}

void
expression::build_parse_tree()
{
	shuting_yards();

	if (!m_token_stack.empty())
	{
		std::cout << __func__ << "The stack should be empty\n";
		throw std::logic_error("The stack should be empty");
	}

	while (!m_token_queue.empty())
	{
		switch (m_token_queue.front()->first)
		{
		case node::OPERAND_CONSTANT:
		case node::OPERAND_ENUM:
		case node::OPERAND_FUNCTION:
		{

			m_node_stack.push(make_operand(m_token_queue.front()));
			m_token_queue.pop();
			break;
		}
		case node::OPERATOR_AND:
		case node::OPERATOR_OR:
		case node::OPERATOR_EQUAL:
		case node::OPERATOR_GT:
		case node::OPERATOR_GTE:
		case node::OPERATOR_LT:
		case node::OPERATOR_LTE:
		case node::OPERATOR_NOT_EQUAL:
		{
			// All of our operators takes 2 arguments
			if (m_node_stack.size() < 2)
			{
				std::cout << __func__ << "The user has not input sufficient values in the expression\n";
				throw std::logic_error("The user has not input sufficient values in the expression");
			}

			std::shared_ptr<node> param1 = m_node_stack.top();
			m_node_stack.pop();
			std::shared_ptr<node> param2 = m_node_stack.top();
			m_node_stack.pop();

			std::shared_ptr<node> oper = make_operator(m_token_queue.front()->first);
			m_token_queue.pop();

			oper->m_right = param1;
			oper->m_left = param2;

			m_node_stack.push(oper);
			break;
		}
		case node::OPERATOR_NOT:
		{
			// All of our operators takes 2 arguments
			if (m_node_stack.size() < 1)
			{
				std::cout << __func__ << "The user has not input sufficient values in the expression\n";
				throw std::logic_error("The user has not input sufficient values in the expression");
			}

			std::shared_ptr<node> param1 = m_node_stack.top();
			m_node_stack.pop();

			std::shared_ptr<node> oper = make_operator(m_token_queue.front()->first);
			m_token_queue.pop();

			oper->m_left = param1;

			m_node_stack.push(oper);
			break;
		}
		case node::OPERATOR_LEFT_BRAKET:
		case node::OPERATOR_RIGHT_BRAKET:
		{
			std::cout << __func__ << ": mismatched parentheses\n";
			throw std::logic_error("mismatched parentheses");
			break;
		}
		default:
			break;
		}
	}

	if (m_node_stack.size() != 1)
	{
		std::cout << __func__ << "The user has not input sufficient values in the expression\n";
		throw std::logic_error("The user has not input sufficient values in the expression");
	}

	m_root = m_node_stack.top();
	m_node_stack.pop();

}

void
expression::shuting_yards()
{
	if (m_vector_tokens->empty())
	{
		std::cout << __func__ << ": Tokens vector is empty!\n";
		throw std::logic_error("Tokens vector is empty!");
	}

	for (auto token : *m_vector_tokens)
	{
		std::shared_ptr<node> curr_node;

		switch (token->first)
		{
		case node::OPERAND_CONSTANT:
		case node::OPERAND_ENUM:
		case node::OPERAND_FUNCTION:
		{
			m_token_queue.push(token);
			break;
		}
		case node::OPERATOR_AND:
		case node::OPERATOR_OR:
		case node::OPERATOR_EQUAL:
		case node::OPERATOR_GT:
		case node::OPERATOR_GTE:
		case node::OPERATOR_LT:
		case node::OPERATOR_LTE:
		case node::OPERATOR_NOT_EQUAL:
		case node::OPERATOR_NOT:
		{
			while (!m_token_stack.empty())
			{
				// if the operator at the top of the operator 
				// stack is not a left bracket
				if (m_token_stack.top()->first != node::OPERATOR_LEFT_BRAKET)
				{
					// - AND -
					// (
					// if there is an operator at the top of the 
					// operator stack with greater precedence
					// - OR -
					// the operator at the top of the operator 
					// stack has equal precedence and is left 
					// associative
					// )
                    if ((get_priority(m_token_stack.top()->first) > get_priority(token->first))||
                        (is_left_assoc(token->first) &&
                        get_priority(m_token_stack.top()->first) == get_priority(token->first)))
					{
						m_token_queue.push(m_token_stack.top());
						m_token_stack.pop();
					}
					else
					{
						break;
					}
				}
				else
				{
					// found
					break;
				}
			}
			m_token_stack.push(token);
			break;
		}
		case node::OPERATOR_LEFT_BRAKET:
		{
			m_token_stack.push(token);
			break;
		}
		case node::OPERATOR_RIGHT_BRAKET:
		{
			while (!m_token_stack.empty() &&
				m_token_stack.top()->first != node::OPERATOR_LEFT_BRAKET)
			{
				m_token_queue.push(m_token_stack.top());
				m_token_stack.pop();
			}

			// if the loop ended and the stack is empty
			// then there is no right_braket in the stack
			if (m_token_stack.empty())
			{
				std::cout << __func__ << ": mismatched parentheses\n";
				throw std::logic_error("mismatched parentheses");
			}
			// here, the top of the stack must be "("
			// we pop out the "(" 
			m_token_stack.pop();
			break;
		}
		default:
			break;
		}
	}

	// if there are still operator tokens in the stack
	// push them to the queue
	while (!m_token_stack.empty())
	{
		if (m_token_stack.top()->first == node::OPERATOR_LEFT_BRAKET ||
			m_token_stack.top()->first == node::OPERATOR_RIGHT_BRAKET)
		{
			std::cout << __func__ << ": mismatched parentheses\n";
			throw std::logic_error("mismatched parentheses");
		}
		m_token_queue.push(m_token_stack.top());
		m_token_stack.pop();
	}
}

std::string expression::to_string() const
{
	std::string res("");
	for (auto token : *m_vector_tokens)
	{
		res += token->second;
	}

	return res;
}

void
expression::print_tree() const
{
	print_tree_util(m_root, 0);
	std::cout << std::endl;
}

void
expression::print_tree_util(std::shared_ptr<node> root, int spaceCount) const
{
	// Base case
	if (root == nullptr)
		return;

	// Increase distance between levels
	spaceCount += 10;

	// Process right child first
	print_tree_util(root->m_right, spaceCount);

	// Print current node after space
	// count
	printf("\n");
	for (int i = 10; i < spaceCount; i++)
		printf(" ");
    printf("%s",root->get_string().c_str());

	// Process left child
	print_tree_util(root->m_left, spaceCount);
}

bool expression::evaluate() const
{
	if (m_root->evaluate() == 0.0)
	{
		return false;
	}
	else
	{
		return true;
	}
}
