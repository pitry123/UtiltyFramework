#pragma once
#include "node.h"

class operator_equal : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string("=="); };

	virtual double evaluate() override;
	operator_equal();
	~operator_equal();
};

class operator_not_equal : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string("!="); };


	operator_not_equal();
	~operator_not_equal();
	virtual double evaluate() override;
};

class operator_greater_than : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string(">"); };

	operator_greater_than();
	~operator_greater_than();
	virtual double evaluate() override;
};

class operator_greater_than_equal : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string(">="); };

	operator_greater_than_equal();
	~operator_greater_than_equal();
	virtual double evaluate() override;
};

class operator_less_than : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string("<"); };

	operator_less_than();
	~operator_less_than();
	virtual double evaluate() override;
};

class operator_less_than_equal : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string("<="); };

	operator_less_than_equal();
	~operator_less_than_equal();
	virtual double evaluate() override;
};

class operator_or : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string("||"); };

	operator_or();
	~operator_or();
	virtual double evaluate() override;
};

class operator_and : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string("&&"); };

	operator_and();
	~operator_and();
	virtual double evaluate() override;
};

class operator_not : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string("!"); };

	operator_not();
	~operator_not();
	virtual double evaluate() override;
};

class left_bracket : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string("("); };

	left_bracket();
	~left_bracket();
	virtual double evaluate() override;
};

class right_bracket : public node
{
public:
	// For debug print
	virtual std::string GetString() { return std::string(")"); };

	right_bracket();
	~right_bracket();
	virtual double evaluate() override;
};
