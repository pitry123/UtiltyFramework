/// @file	core/buffer_interface.h.
/// @brief	Declares buffer interface classes
#pragma once
#include <core/ref_count_interface.h>
#include <cstdint>
#include <cstddef>

namespace core
{
	/// @class	buffer_interface
	/// @brief	An interface defining a reference counted buffer.
	/// @date	14/05/2018
	class DLL_EXPORT buffer_interface : 
		public core::ref_count_interface
	{
	public:
		/// @fn	virtual buffer_interface::~buffer_interface() = default;
		/// @brief	Destructor
		/// @date	14/05/2018
		virtual ~buffer_interface() = default;

		/// @fn	virtual size_t buffer_interface::size() = 0;
		/// @brief	Gets the buffer's size
		/// @date	14/05/2018
		/// @return	A size_t.
		virtual size_t size() const = 0;

		/// @fn	virtual uint8_t* buffer_interface::data() = 0;
		/// @brief	Gets the data pointer
		/// @date	14/05/2018
		/// @return	uint8_t*.
		virtual uint8_t* data() = 0;
	};

	/// @class	buffer_allocator
	/// @brief	An interface defining a buffer allocator.
	/// @date	14/05/2018
	class DLL_EXPORT buffer_allocator : 
		public core::ref_count_interface
	{
	public:
		/// @fn	virtual buffer_allocator::~buffer_allocator() = default;
		/// @brief	Destructor
		/// @date	14/05/2018
		virtual ~buffer_allocator() = default;

		/// @fn	virtual bool buffer_allocator::allocate(size_t buffer_size, core::buffer_interface** buffer) = 0;
		/// @brief	Allocates a buffer
		/// @date	14/05/2018
		/// @param 		   	buffer_size	Size of the buffer to allocate.
		/// @param [out]	buffer	   	the allocated buffer.
		/// @return	True if it succeeds, false if it fails.
		virtual bool allocate(size_t buffer_size, core::buffer_interface** buffer) = 0;
	};

	class DLL_EXPORT safe_buffer_interface : public core::ref_count_interface
	{
	public:

		/// Destructor
		/// @date	27/12/2018
		virtual ~safe_buffer_interface() = default;

		/// Gets the size
		/// @date	27/12/2018
		/// @return	A size_t.
		virtual size_t size() const = 0;

		/// Safe read of the buffer (copy the the data_ptr)
		/// @date	27/12/2018
		/// @param [in,out]	data_ptr		   	If non-null, the return data .
		/// @param 		   	num_of_byte_to_read	Number of byte to reads (has to be smalled or equal to buffer size - offset).
		/// @param 		   	offset			   	The offset to read from the beginning of the buffer.
		/// @return	True if it succeeds, false if it fails.
		virtual bool safe_read(void *data_ptr, size_t num_of_byte_to_read, size_t offset) const = 0;

		/// Safe write of the buffer from data_ptr
		/// @date	27/12/2018
		/// @param	data_ptr				The data pointer.
		/// @param	num_of_byte_to_write	Number of byte to writes into the buffer starting from the offser.
		/// @param	offset					The offset.
		/// @return	True if it succeeds, false if it fails.
		virtual bool safe_write(const void *data_ptr, size_t num_of_byte_to_write, size_t offset) = 0;

		/// Safe write hexadecimal write the buffer from a string
		/// @date	27/12/2018
		/// @param	str	The string.
		virtual void safe_write_hex(const char* str) = 0;

		/// Nullifies the buffer
		/// @date	06/08/2020
		virtual void nullify() = 0;
	};
}
