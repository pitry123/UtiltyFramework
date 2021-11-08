#pragma once
#include <utils/types.hpp>

namespace Types
{
	using Endian = core::types::endian;

	static inline Types::Endian PlatformEndian()
	{
		return utils::types::platform_endian();
	}

	template <typename T>
	static inline void EndianSwap(T& val)
	{
		utils::types::endian_swap(val);
	}
}