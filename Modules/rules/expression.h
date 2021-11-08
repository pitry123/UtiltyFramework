// ############################################################################################ // 
// ------------------------------- [ Shunting yard algorithm ] -------------------------------- //
// ############################################################################################ // 
// -- The first step is to create RPN - Reverse Polish Notation --
//
//	While there are tokens to be read :
//		Read a token.
//		If the token is a number, then add it to the output queue.
//		If the token is a function token, then push it onto the stack.
//		If the token is a function argument separator(e.g., a comma) :
//			Until the topmost element of the stack is a left parenthesis, pop the element onto the output queue.
//			If no left parentheses are encountered, either the separator was misplaced or parentheses were mismatched.
//		If the token is an operator, o1, then:
//			while	(there is an operator, o2, at the top of the stack, and either
//						o1 is associative or left - associative and its precedence is less than(lower precedence) or equal to that of o2, 
//					OR	o1 is right - associative and its precedence is less than(lower precedence) that of o2)
//			{
//				pop o2 off the stack, onto the output queue
//			}
//			push o1 onto the operator stack.
//		If the token is a left parenthesis, then push it onto the stack.
//		If the token is a right parenthesis :
//		Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue.
//			Pop the left parenthesis from the stack, but not onto the output queue.
//			If the token at the top of the stack is a function token, pop it and onto the output queue.
//			If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
//		When there are no more tokens to read :
//			While there are still operator tokens in the stack :
//				If the operator token on the top of the stack is a parenthesis, then there are mismatched parenthesis.
//				Pop the operator onto the output queue.
//	Exit.
//
// -- The second step is the RPN evaluation algorithm
// 
// While there are input tokens left
//	Read the next token from input.
//
//	If the token is a value
//		Push it onto the stack.
//	Otherwise, the token is a function. (Operators, like + , are simply functions taking two arguments.)
//		It is known that the function takes n arguments.
//		So, pop the top n values from the stack.
//			If there are fewer than n values on the stack
//				(Error)The user has not input sufficient values in the exp<b>< / b>ression.
//		Evaluate the function, with the values as arguments.
//		Push the returned results, if any, back onto the stack.
//	If there is only one value in the stack
//		That value is the result of the calculation.
//	If there are more values in the stack
//		(Error)The user input too many values.

// References : 
// https://www.dreamincode.net/forums/topic/35320-reverse-polish-notation-in-c%23/
// 

#pragma once
#include <memory>
#include "node.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <stack>
#include <vector>
#include <queue>
#include <utils/ref_count_ptr.hpp>

class rules_dispatcher_impl;

class expression
{
public:
	expression(
		rules_dispatcher_impl* rules_dispatcher,
		std::shared_ptr<std::vector<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>> vectorTokens);

	~expression();

	bool evaluate() const;
	std::string to_string() const;
	void print_tree() const;

private:
	utils::ref_count_ptr<rules_dispatcher_impl> m_rules_dispatcher;
	std::shared_ptr<std::vector<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>> m_vector_tokens;
	std::shared_ptr<node>													 m_root;
	std::queue<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>	 m_token_queue;
	std::stack<std::shared_ptr<std::pair<node::node_type_enum, std::string>>>	 m_token_stack;
	std::stack<std::shared_ptr<node>>										 m_node_stack;

	void print_tree_util(std::shared_ptr<node> root, int spaceCount) const;

	void build_parse_tree();
	void shuting_yards();
	std::shared_ptr<node> make_operator(node::node_type_enum oper) const;
	std::shared_ptr<node> make_operand(std::shared_ptr<std::pair<node::node_type_enum, std::string>> token) const;
};