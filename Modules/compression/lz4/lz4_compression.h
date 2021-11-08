#pragma once
#include <compression/lz4_compression_interface.h>
#include <utils/ref_count_base.hpp>

namespace compression
{
	class lz4_compression :
		public utils::ref_count_base<compression::lz4_compression_interface>
	{
	private:
		uint32_t m_compression_level;
	public:
		lz4_compression(uint32_t compression_level);
		virtual ~lz4_compression() = default;

		virtual bool encode(const void* input, uint32_t input_size, void* output, uint32_t& output_size) override;
		virtual bool decode(const void* input, uint32_t input_size, void* output, uint32_t target_output_size) override;
	};
}