#pragma once
#include <core/compression.h>

namespace compression
{
	class DLL_EXPORT lz4_compression_interface : public core::compression_interface
	{
	public:
		virtual ~lz4_compression_interface() = default;
		static bool create(uint32_t compression_level, core::compression_interface** compression);
	};
}
