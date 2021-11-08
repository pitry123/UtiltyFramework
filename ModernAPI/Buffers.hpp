#pragma once
#include <core/buffer_interface.h>

#include <Common.hpp>

namespace Buffers
{
	class Buffer :
		public Common::CoreObjectWrapper<core::buffer_interface>
	{
	public:
		Buffer()
		{
			// Empty Buffer
		}

		Buffer(core::buffer_interface* buffer) :
			CoreObjectWrapperBase(buffer)
		{
		}

		size_t Size()
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Buffer");

			return m_core_object->size();
		}

		uint8_t* Data()
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Buffer");

			return m_core_object->data();
		}
	};

	class SafeBuffer :
		public Common::CoreObjectWrapper<core::safe_buffer_interface>
	{
	public:
		SafeBuffer()
		{
			// Empty Buffer
		}

		SafeBuffer(core::safe_buffer_interface* buffer) :
			CoreObjectWrapperBase(buffer)
		{
		}

		size_t Size()
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Buffer");

			return m_core_object->size();
		}

		bool SafeRead(void *data, size_t numOfByteToRead, size_t offset)
		{
			ThrowOnEmpty("SafeWrite");

			return m_core_object->safe_read(data, numOfByteToRead, offset);
		}

		bool SafeWrite(const void *data, size_t numOfByteToWrite, size_t offset)
		{
			ThrowOnEmpty("SafeWrite");

			return m_core_object->safe_write(data, numOfByteToWrite, offset);
		}
		
	};
}