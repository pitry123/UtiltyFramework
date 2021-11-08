#pragma once
#include <core/stream_interface.h>
#include <Common.hpp>

namespace Streams
{
	using Status = core::stream_status;
	using RelativePosition = core::stream_interface::relative_position;

	class Stream :
		public Common::CoreObjectWrapper<core::stream_interface>
	{
	public:
		Stream()
		{
			// Empty stream
		}

		Stream(core::stream_interface* stream) :
			CoreObjectWrapperBase(stream)
		{
		}

        virtual Streams::Status Flush()
        {
            if (Empty() == true)
                throw std::runtime_error("Empty Stream");

            return m_core_object->flush();
        }

		virtual Streams::Status Reset(bool wipeData)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Stream");

			return m_core_object->reset(wipeData);
		}

		virtual Streams::Status GetPosition(uint64_t& position)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Stream");

			return m_core_object->get_position(position);
		}

		virtual Streams::Status SetPosition(int64_t offset, Streams::RelativePosition relativeTo)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Stream");

			return m_core_object->set_position(offset, relativeTo);
		}

		virtual Streams::Status SetPosition(int64_t offset, Streams::RelativePosition relativeTo, uint64_t& position)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Stream");

			return m_core_object->set_position(offset, relativeTo, position);
		}

		virtual Streams::Status ReadBytes(void* data, size_t numberOfBytesToRead, size_t& numberOfBytesRead)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Stream");

			return m_core_object->read_bytes(data, numberOfBytesToRead, numberOfBytesRead);
		}

		virtual Streams::Status WriteBytes(const void* data, size_t numberOfBytesToWrite, size_t& numberOfBytesWritten)
		{
			if (Empty() == true)
				throw std::runtime_error("Empty Stream");

			return m_core_object->write_bytes(data, numberOfBytesToWrite, numberOfBytesWritten);
		}

		template <typename T>
		inline Streams::Status Write(const T& obj)
		{
                        size_t bytesWritten;
                        Streams::Status retval = WriteBytes((void*)&obj, sizeof(T), bytesWritten);
            if (retval != Streams::Status::status_no_error)
				return retval;

			if (bytesWritten != sizeof(T))
                return Streams::Status::status_write_failed;

            return Streams::Status::status_no_error;
		}

		template <typename T>
		inline Streams::Status Read(T& obj)
		{
                        size_t bytesRead;
			Streams::Status retval = ReadBytes((void*)&obj, sizeof(T), bytesRead);
            if (retval != Streams::Status::status_no_error)
				return retval;

			if (bytesRead != sizeof(T))
                return Streams::Status::status_read_failed;

            return Streams::Status::status_no_error;
		}
	};
}
