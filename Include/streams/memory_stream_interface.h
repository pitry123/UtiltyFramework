/// @file	streams/memory_stream_interface.h.
/// @brief	Declares the memory stream class
#pragma once
#include <core/stream_interface.h>
#include <core/buffer_interface.h>

namespace streams
{
	/// @class	memory_stream_interface
	/// @brief	Memory streams access the process memory for read and write by using the generic core::stream_interface API
	/// @date	15/05/2018
	class DLL_EXPORT memory_stream_interface : public core::stream_interface
	{
	public:
		/// @fn	virtual memory_stream_interface::~memory_stream_interface() = default;
		/// @brief	Destructor
		/// @date	15/05/2018
		virtual ~memory_stream_interface() = default;

		/// @fn	static bool memory_stream_interface::create(uint32_t size, core::stream_interface** stream);
		/// @brief	Static factory: Creates a new memory stream instance
		/// @date	15/05/2018
		/// @param 		   	size  	The memory size to allocate.
		/// @param [out]	stream	An address of a pointer to core::stream_interface
		/// @return	True if it succeeds, false if it fails.
		static bool create(uint32_t size, core::stream_interface** stream);

		/// @fn	static bool memory_stream_interface::create(core::buffer_interface* buffer, core::stream_interface** stream);
		/// @brief	Creates a new bool
		/// @date	15/05/2018
		/// @param [in]		buffer	A pre-allocated buffer to be used by the stream.
		/// @param [out]	stream	An address of a pointer to core::stream_interface
		/// @return	True if it succeeds, false if it fails.
		static bool create(core::buffer_interface* buffer, core::stream_interface** stream);
	};
}