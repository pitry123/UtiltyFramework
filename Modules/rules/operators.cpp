#include "operators.h"
operator_equal::operator_equal()
{

}

operator_equal::~operator_equal()
{

}

double operator_equal::evaluate()
{
	double dLeftRes = m_left->evaluate();
	double dRightRes = m_right->evaluate();

	return dLeftRes == dRightRes;
}

operator_not_equal::operator_not_equal() 
{

}

operator_not_equal::~operator_not_equal()
{

}

double operator_not_equal::evaluate()
{
	return m_left->evaluate() != m_right->evaluate();
}

operator_or::operator_or() 
{

}

operator_or::~operator_or()
{

}

double operator_or::evaluate()
{
    return (m_left->evaluate() != 0 || m_right->evaluate() != 0);
}

operator_less_than_equal::operator_less_than_equal() 
{

}

operator_less_than_equal::~operator_less_than_equal()
{

}

double operator_less_than_equal::evaluate()
{
	return m_left->evaluate() <= m_right->evaluate();
}

operator_less_than::operator_less_than() 
{

}

operator_less_than::~operator_less_than()
{

}

double operator_less_than::evaluate()
{
	return m_left->evaluate() < m_right->evaluate();
}

operator_greater_than_equal::operator_greater_than_equal() 
{

}

operator_greater_than_equal::~operator_greater_than_equal()
{

}

double operator_greater_than_equal::evaluate()
{
	return m_left->evaluate() >= m_right->evaluate();
}

operator_greater_than::operator_greater_than() 
{

}

operator_greater_than::~operator_greater_than()
{

}

double operator_greater_than::evaluate()
{
	return m_left->evaluate() > m_right->evaluate();
}

operator_and::operator_and() 
{

}

operator_and::~operator_and()
{

}

double operator_and::evaluate()
{
    return (m_left->evaluate() != 0.0 && m_right->evaluate() != 0.0);
}

right_bracket::right_bracket() 
{

}

right_bracket::~right_bracket()
{

}

double right_bracket::evaluate()
{
	std::cout << "Evaluation on Right Bracket is impossible\n";
	throw std::logic_error("Evaluation on Right Bracket is impossible");
}

left_bracket::left_bracket() 
{

}

left_bracket::~left_bracket()
{

}

double left_bracket::evaluate()
{
	std::cout << "Evaluation on Left Bracket is impossible\n";
	throw std::logic_error("Evaluation on Left Bracket is impossible");
}

operator_not::operator_not() 
{

}

operator_not::~operator_not()
{

}

double operator_not::evaluate()
{
	if (m_right != nullptr)
	{
		std::cout << "Illegal expression: NOT has a right child\n";
		throw std::logic_error("Illegal expression : NOT has a right child");
	}
    return !(m_left->evaluate() != 0.0);
}
