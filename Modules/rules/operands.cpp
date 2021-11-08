#include "operands.h"

operand_constant::operand_constant(double dValue) :
	m_dValue(dValue)
{

}

operand_constant::~operand_constant()
{

}

double operand_constant::evaluate()
{
	return m_dValue;
}

operand_enum::operand_enum(int nValue)
{
	m_dValue = (double)nValue;
}

operand_enum::~operand_enum()
{

}

double operand_enum::evaluate()
{
	return m_dValue;
}

operand_function::operand_function(std::function<double()> func) :
	m_func(func)
{
}

operand_function::~operand_function()
{

}

double operand_function::evaluate()
{
	return m_func();
}
