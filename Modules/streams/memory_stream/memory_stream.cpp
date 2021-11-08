// memory_stream.cpp : Defines the exported functions for the DLL application.
//

#include "memory_stream.h"
#include <utils/scope_guard.hpp>
#include <utils/buffer_allocator.hpp>

#include <cstring>

streams::memory_stream::memory_stream(size_t size) :
	m_buffer(utils::make_ref_count_ptr<utils::ref_count_buffer>(size)),
	m_position(0)
{
    reset(true);
}

streams::memory_stream::memory_stream(core::buffer_interface* buffer) :
	m_buffer(buffer),
	m_position(0)
{
	if (m_buffer == nullptr)
		throw std::invalid_argument("buffer");
}

core::stream_status streams::memory_stream::read_bytes(void* data, size_t number_of_bytes_to_read, size_t& number_of_bytes_read)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_position + number_of_bytes_to_read > m_buffer->size())
		return core::stream_status::status_read_failed;

	uint8_t* source = (m_buffer->data() + m_position);
	std::memcpy(data, source, number_of_bytes_to_read);

	m_position += number_of_bytes_to_read;
	number_of_bytes_read = number_of_bytes_to_read;

	return core::stream_status::status_no_error;
}

core::stream_status streams::memory_stream::write_bytes(const void* data, size_t number_of_bytes_to_write, size_t& number_of_bytes_written)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_position + number_of_bytes_to_write > m_buffer->size())
		return core::stream_status::status_write_failed;

	uint8_t* dest = (m_buffer->data() + m_position);
	std::memcpy(dest, data, number_of_bytes_to_write);

	m_position += number_of_bytes_to_write;
	number_of_bytes_written = number_of_bytes_to_write;

	return core::stream_status::status_no_error;
}

core::stream_status streams::memory_stream::set_position(int64_t offset, core::stream_interface::relative_position relative_to)
{
	uint64_t position;
	return set_position(offset, relative_to, position);
}

core::stream_status streams::memory_stream::set_position(int64_t offset, core::stream_interface::relative_position relative_to, uint64_t& position)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	utils::scope_guard position_set([&]()
	{
		position = m_position;
	});

	int64_t index;
	if (relative_to == core::stream_interface::relative_position::begin)
		index = 0;
	else if (relative_to == core::stream_interface::relative_position::end)
        index = static_cast<int64_t>(m_buffer->size());
    else if (relative_to == core::stream_interface::relative_position::current)
        index = static_cast<int64_t>(m_position);
	else
		return core::stream_status::status_read_failed;

	index += offset;
	if (index < 0 || static_cast<uint64_t>(index) >= m_buffer->size())
		return core::stream_status::status_read_failed;

	m_position = static_cast<uint64_t>(index);
	return core::stream_status::status_no_error;
}

core::stream_status streams::memory_stream::get_position(uint64_t& position)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	position = m_position;
	return core::stream_status::status_no_error;
}

core::stream_status streams::memory_stream::reset(bool wipe_data)
{
	std::lock_guard<std::mutex> locker(m_mutex);

    if (wipe_data)
        std::memset(m_buffer->data(), '\0', m_buffer->size());

	m_position = 0;
	return core::stream_status::status_no_error;
}

core::stream_status streams::memory_stream::flush()
{
    // Memory data is always flushed. Return no-error
    return core::stream_status::status_no_error;
}

bool streams::memory_stream_interface::create(uint32_t size, core::stream_interface** stream)
{
	if (stream == nullptr)
		return false;

	utils::ref_count_ptr<core::stream_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<streams::memory_stream>(size);
	}
	catch (.../*const std::exception& e*/)
	{
		return false;
	}

	instance->add_ref();
	(*stream) = instance;
	return true;
}

bool streams::memory_stream_interface::create(core::buffer_interface* buffer, core::stream_interface** stream)
{
	if (stream == nullptr)
		return false;

	utils::ref_count_ptr<core::stream_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<streams::memory_stream>(buffer);
	}
	catch (.../*const std::exception& e*/)
	{
		return false;
	}

	instance->add_ref();
	(*stream) = instance;
	return true;
}
