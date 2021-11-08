#include "lz4_compression.h"
#include <utils/ref_count_ptr.hpp>

#include <lz4.h>
#include <ctime>
#include <iostream>

#include <limits>

#if defined(__APPLE__) && defined(__clang__)
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#ifdef __GNUC__
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

compression::lz4_compression::lz4_compression(uint32_t compression_level) : 
	m_compression_level(compression_level)
{
}

bool compression::lz4_compression::encode(const void* input, uint32_t input_size, void* output, uint32_t& output_size)
{
	if (input == nullptr || input_size == 0)
	{
		// TODO: log error...
		return false;
	}

	if (input_size > static_cast<uint32_t>((std::numeric_limits<int>::max)()))
	{
		// TODO: log error...
		return false;
	}

	int result = LZ4_compress_fast(
		reinterpret_cast<const char*>(input), 
		reinterpret_cast<char*>(output), 
		static_cast<int>(input_size), 
		static_cast<int>(input_size),
        static_cast<int>(m_compression_level));

	if (result <= 0 ||
		result == static_cast<int>(input_size))
	{
		// TODO: log compression failed
		return false;
	}

	output_size = static_cast<uint32_t>(result);
	return true;
}

bool compression::lz4_compression::decode(const void* input, uint32_t input_size, void* output, uint32_t target_output_size)
{
	if (input_size > static_cast<uint32_t>((std::numeric_limits<int>::max)()) ||
		target_output_size > static_cast<uint32_t>((std::numeric_limits<int>::max)()))
	{
		// TODO: log error...
		return false;
	}
	
	int compressed_size = LZ4_decompress_fast(
		reinterpret_cast<const char*>(input), 
		reinterpret_cast<char*>(output), 
		static_cast<int>(target_output_size));

	if (compressed_size != static_cast<int>(input_size))
	{
		// TODO: log error...
		return false;
	}

	return true;
}

bool compression::lz4_compression_interface::create(uint32_t compression_level, core::compression_interface** compression)
{
    if (compression == nullptr)
        return false;

    utils::ref_count_ptr<core::compression_interface> instance;
    try
    {
        instance = utils::make_ref_count_ptr<lz4_compression>(compression_level);
    }
    catch (...)
    {
        return false;
    }

	if (instance == nullptr)
		return false;

    (*compression) = instance;
	(*compression)->add_ref();
    return true;
}

#if defined(__APPLE__) && defined(__clang__)
#	pragma clang diagnostic pop
#endif
