#pragma once
#include <core/buffer_interface.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_object_pool.hpp>
#include <utils/strings.hpp>
#include <map>
#include <mutex>

namespace utils
{
	/// managed buffer class, allows ref cout buffer.
	/// @date	03/10/2018
	class ref_count_buffer : public utils::ref_count_base<core::buffer_interface>
	{
	private:
		std::vector<uint8_t> m_buffer;

	protected:
		virtual const uint8_t* data() const
		{
			return m_buffer.data();
		}

	public:
		ref_count_buffer(size_t size) :
			m_buffer(size)
		{
		}

		ref_count_buffer(const std::vector<uint8_t>& buffer) :
			m_buffer(buffer)
		{
		}

		ref_count_buffer(const uint8_t* data, size_t size) :
			m_buffer(size)
		{
#ifdef _WIN32
			memcpy_s(m_buffer.data(), m_buffer.size(), data, size);
#else
			std::memcpy(m_buffer.data(), data, size);
#endif
		}

		ref_count_buffer(std::vector<uint8_t>&& buffer) :
			m_buffer(std::move(buffer))
		{
		}

		virtual size_t size() const override
		{
			return m_buffer.size();
		}

		virtual uint8_t* data() override
		{
			return m_buffer.data();
		}

		virtual void swap(std::vector<uint8_t>& buffer)
		{
			m_buffer.swap(buffer);
		}

		void resize(size_t size)
		{
			m_buffer.resize(size);
		}
	};

	/// Buffer for safe usage allow protected read and write from a managed buffer.
	/// @date	03/10/2018
	class safe_buffer : public utils::ref_count_base<core::safe_buffer_interface>
	{
	private:
		std::vector<uint8_t> m_buffer;
		mutable std::mutex m_mutex;

	public:
		safe_buffer(size_t size) :
			m_buffer(size)
		{
		  std::memset(m_buffer.data(),0, m_buffer.size());
		}

		safe_buffer(const std::vector<uint8_t>& buffer) :
			m_buffer(buffer)
		{
		}

		safe_buffer(const uint8_t* data, size_t size) :
			m_buffer(size)
		{
#ifdef _WIN32
			memcpy_s(m_buffer.data(), m_buffer.size(), data, size);
#else
			std::memcpy(m_buffer.data(), data, size);
#endif
		}

		virtual size_t size() const override
		{
			return m_buffer.size();
		}

		void swap(std::vector<uint8_t>& buffer)
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			m_buffer.swap(buffer);
		}

		bool safe_read(void *data_ptr, size_t num_of_byte_to_read, size_t offset) const override
		{
			if (data_ptr == nullptr)
				return false;

			if ((num_of_byte_to_read + offset) > this->size())
				return false;

			std::lock_guard<std::mutex> locker(m_mutex);
#ifdef _WIN32
			memcpy_s(data_ptr, num_of_byte_to_read, &m_buffer[offset], num_of_byte_to_read);
#else
			std::memcpy(data_ptr, &m_buffer[offset], num_of_byte_to_read);
#endif
			return true;

		}

		bool safe_write(const void *data_ptr, size_t num_of_byte_to_write, size_t offset) override
		{
			if (data_ptr == nullptr)
				return false;
			if ((num_of_byte_to_write + offset) > size())
				return false;

			std::lock_guard<std::mutex> locker(m_mutex);
#ifdef _WIN32
			memcpy_s(&m_buffer[offset], num_of_byte_to_write, data_ptr, num_of_byte_to_write);
#else
			std::memcpy(&m_buffer[offset], data_ptr, num_of_byte_to_write);
#endif
			return true;

		}

		void safe_write_hex(const char* str) override
		{
			if (str == nullptr)
				throw std::invalid_argument("str");

			size_t str_length = std::strlen(str);

			if (str_length % 2 != 0)
				throw std::runtime_error("not an even buffer size");

			if (str_length / 2 > size())
				throw std::runtime_error("size is bigger than buffer size");

			uint8_t single_byte;
			int offset = 0;

			std::lock_guard<std::mutex> locker(m_mutex);
			for (size_t i = 0; i < size(); i++)
			{
				single_byte = static_cast<uint8_t>(hex_char2int(str[offset]) * 16 + hex_char2int(str[offset + 1]));
				m_buffer[i] = single_byte;
				offset += 2;
			}
		}

		void nullify() override
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			memset(m_buffer.data(), 0, m_buffer.size());
		}
	};

	template <typename T>
	class relative_buffer_base : public T
	{
	private:
		utils::ref_count_ptr<core::buffer_interface> m_source_buffer;
		size_t m_offset;
		size_t m_size;

	protected:
		bool query_source_buffer(core::buffer_interface** source_buffer)
		{
			if (source_buffer == nullptr)
				return false;

			*source_buffer = m_source_buffer;
			(*source_buffer)->add_ref();
			return true;
		}

	public:
		relative_buffer_base(core::buffer_interface* source_buffer, size_t offset, size_t size) :
			m_source_buffer(source_buffer),
			m_offset(offset),
			m_size(size)
		{
			if (source_buffer == nullptr)
				throw std::invalid_argument("source_buffer");

			size_t source_size = source_buffer->size();
			if ((offset + size) > source_size)
				throw std::out_of_range("size + offset is out of range");
		}

		virtual ~relative_buffer_base() = default;

		virtual size_t size() const override
		{
			return m_size;
		}

		virtual uint8_t* data() override
		{
			return (m_source_buffer->data() + m_offset);
		}
	};

	/// relative buffer which allow handling a portion of buffer interface based on initial buffer and offset.
	/// @date	03/10/2018
	class ref_count_relative_buffer : public relative_buffer_base<utils::ref_count_base<core::buffer_interface>>
	{
	public:
		ref_count_relative_buffer(core::buffer_interface* source_buffer, size_t offset, size_t size) :
			relative_buffer_base<utils::ref_count_base<core::buffer_interface>>(source_buffer, offset, size)
		{
		}

		virtual ~ref_count_relative_buffer() = default;
	};

	class buffer_pool : public utils::ref_count_object_pool<ref_count_buffer>
	{
	public:
		buffer_pool(size_t pool_size, bool lazy, size_t buffer_size) :
			utils::ref_count_object_pool<ref_count_buffer>(pool_size, growing_mode::none, lazy, buffer_size)
		{
		}
	};

	/// A buffer allocator which allow preallocation of buffers to a buffer pool .
	/// @date	03/10/2018
	class buffer_allocator : public utils::ref_count_base<core::buffer_allocator>
	{
	private:
		std::map<size_t, utils::ref_count_ptr<buffer_pool>> m_pools;
		size_t m_max_allocated_memory_per_buffer_size;
		bool m_lazy;
		std::mutex m_mutex;

	public:
		buffer_allocator(size_t max_allocated_memory_per_buffer_size, bool lazy = true) :
			m_max_allocated_memory_per_buffer_size(max_allocated_memory_per_buffer_size),
			m_lazy(lazy)
		{
		}

		virtual bool allocate(size_t buffer_size, core::buffer_interface** buffer) override
		{
			if (buffer == nullptr)
				return false;

			std::lock_guard<std::mutex> locker(m_mutex);

			utils::ref_count_ptr<utils::ref_count_buffer> local_buffer;
			auto it = m_pools.find(buffer_size);
			if (it != m_pools.end())
			{
				if (it->second->get_item(&local_buffer) == false)
					return false;
			}
			else
			{
				size_t buffer_count = (m_max_allocated_memory_per_buffer_size / buffer_size);
				if (buffer_size == 0)
					return false;

				auto emplace_it = m_pools.emplace(
					buffer_size,
					utils::make_ref_count_ptr<buffer_pool>(buffer_count, m_lazy, buffer_size));
				if (emplace_it.second == false)
					return false;

				if (emplace_it.first->second->get_item(&local_buffer) == false)
					return false;
			}

			local_buffer->add_ref();
			*buffer = local_buffer;

			return true;
		}
	};


}
