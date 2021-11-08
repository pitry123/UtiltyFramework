#pragma once
#include "node.h"
#include <functional>

class operand_constant : public node
{
private:
	double m_dValue;

public:
	// For debug print
	virtual std::string get_string() override { return   std::to_string(m_dValue); };


	operand_constant(double dValue);
	~operand_constant();
	virtual double evaluate() override;
};

class operand_enum : public node
{
private:
	double m_dValue;

public:
	// For debug print
	virtual std::string get_string() override { return   std::to_string(m_dValue); };

	operand_enum(int nValue);
	~operand_enum();
	virtual double evaluate() override;
};

class operand_function : public node
{
private:	
	std::function<double()> m_func;

public:
	// For debug print
	virtual std::string get_string() override { return   std::to_string(m_func()); }; 


	operand_function(std::function<double()> func);
	~operand_function();
	
	virtual double evaluate() override;
};
