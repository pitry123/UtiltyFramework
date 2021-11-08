#pragma once
#include <core/ref_count_interface.h>
#include <cstdint>

namespace core
{
	class DLL_EXPORT compression_interface : public core::ref_count_interface
	{
	public:
		virtual ~compression_interface() = default;

		virtual bool encode(const void* input, uint32_t input_size, void* output, uint32_t& output_size) = 0;
		virtual bool decode(const void* input, uint32_t input_size, void* output, uint32_t target_output_size) = 0;
	};
}
