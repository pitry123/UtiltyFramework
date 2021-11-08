#pragma once
#include <random>

namespace utils
{
	static inline int random_int(int max_value)
	{
		return (std::rand() % max_value);
	}

	static inline float random_float(float max_value)
	{
		return (static_cast<float>(std::rand()) / (static_cast<float>(static_cast<float>(RAND_MAX) / max_value)));
	}
}
