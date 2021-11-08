#pragma once
#include <utils/strings.hpp>

inline void AutoExpandEnvironmentVariables(std::string & text)
{
	auto_expand_environment_variables(text);
}

// Leave input alone and return new string.
inline std::string ExpandEnvironmentVariables(const std::string & input)
{
	std::string text = input;
	AutoExpandEnvironmentVariables(text);
	return text;
}