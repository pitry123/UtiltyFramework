#include "row.h"
#include <cstring>

database::memory_row_impl::memory_row_impl(const core::database::key& key, size_t size, char* buffer, const core::database::row_info& info, core::parsers::binary_metadata_interface* parser, core::database::table_interface* parent) :
	utils::database::row_base<database::memory_row>(key, parent,info,parser),
	m_max_size(buffer == nullptr ? core::database::UNBOUNDED_ROW_SIZE : size),
	m_current_size(0),	
	m_buffer(buffer),
	m_unbounded_data_size(buffer == nullptr),
	m_unbounded_allocation_size(0),
	m_write_priority(0)
{	
	if (m_buffer != nullptr)
		std::memset(m_buffer, 0, size);	
}

database::memory_row_impl::memory_row_impl(const core::database::key & key, const core::database::row_info& info, core::parsers::binary_metadata_interface* parser, core::database::table_interface * parent) :
	database::memory_row_impl::memory_row_impl(key, 0, nullptr,info,parser, parent)
{
}

database::memory_row_impl::~memory_row_impl()
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_unbounded_data_size == true && m_buffer != nullptr)
		delete[] m_buffer;		
}

size_t database::memory_row_impl::data_size() const
{
	return m_max_size;
}

uint8_t database::memory_row_impl::write_priority() const
{
	std::lock_guard<std::mutex> locker(m_mutex);
	return m_write_priority;
}

bool database::memory_row_impl::read_bytes(void* buffer, size_t size) const
{
	if (buffer == nullptr || size > m_max_size || size == 0)
		return false;

	std::lock_guard<std::mutex> locker(m_mutex);

	// We're checking if currently available data size is smaller than the requested.
	// If that's the case, we're zero'ing the user's array before writing the available size
	size_t actual_size = (std::min)(size, m_current_size);
	if (actual_size < size)
		std::memset(buffer, 0, size);

	if (actual_size == 0)
		return true;

	std::memcpy(buffer, m_buffer, actual_size);
	return true;
}

bool database::memory_row_impl::read_bytes(void* buffer) const
{
	return read_bytes(buffer, m_max_size);
}

bool database::memory_row_impl::write_bytes(const void* buffer, size_t size, bool force_report, uint8_t priority)
{
	if ((buffer == nullptr || size > m_max_size || size == 0) && info().type != core::types::EMPTY_TYPE)
		return false;

	bool raise = true;
	utils::scope_guard notifier([&]()
	{
		if (raise == true)
			raise_callbacks(size, buffer);
	});

	// Note that the lock is guarding only the local data writing.
	// Scope will be unlocked BEFORE we're raising the data change callbacks.
	// That's a no issue since the callbacks are reported synchronously with the arguments of this functions,
	// hence, not using the local buffer
	std::lock_guard<std::mutex> locker(m_mutex);

	if (priority < m_write_priority)
	{
		// Writing is not allowed for this call.
		// DO NOT update AND return gracefully
		// TODO: Add log message here...
		raise = false;
		return true;
	}	
	
	//if Empty Row there is no meaning for force_report
	if (info().type != core::types::EMPTY_TYPE &&
		force_report == false && 
		m_current_size == size && 
		std::memcmp(m_buffer, buffer, size) == 0)
		raise = false;

	if (m_unbounded_data_size == true &&
		m_unbounded_allocation_size < size)
	{

		if (m_buffer != nullptr)
			delete[] m_buffer;
		
		m_buffer = new char[size];
		m_unbounded_allocation_size = size;

	}

	if (raise == true && info().type != core::types::EMPTY_TYPE)
	{
		m_current_size = size;
		std::memcpy(m_buffer, buffer, size);
	}

	return true;
}

bool database::memory_row_impl::set_write_priority(uint8_t priority)
{
	std::lock_guard<std::mutex> locker(m_mutex);
	m_write_priority = priority;
	return true;
}

void* database::memory_row_impl::operator new(std::size_t size)
{
	return ::operator new(size);
}

void* database::memory_row_impl::operator new(std::size_t count, void* ptr)
{
	return ::operator new(count, ptr);
}

void database::memory_row_impl::operator delete(void* ptr)
{
	::operator delete(ptr);
}

void database::memory_row_impl::operator delete(void* ptr, void* place)
{
	::operator delete(ptr, place);
}

bool database::memory_row::create(const core::database::key& key, size_t size, const core::database::row_info& info, core::parsers::binary_metadata_interface* parser, core::database::table_interface* parent, core::database::row_interface** row)
{
	if (row == nullptr)
		return false;

	if (size == 0 && info.type !=core::types::EMPTY_TYPE)
		return false;

	utils::ref_count_ptr<core::database::row_interface> instance;
	if (size != core::database::UNBOUNDED_ROW_SIZE)
	{
		try
		{
			char* placenment_buffer = static_cast<char*>(memory_row_impl::operator new(sizeof(database::memory_row_impl) + size));

			try
			{
				core::database::row_interface* p_row = new (placenment_buffer)memory_row_impl(key, size, placenment_buffer + sizeof(database::memory_row_impl), info, parser, parent);
				instance.attach(p_row);
			}
			catch (...)
			{
				memory_row_impl::operator delete(placenment_buffer);
				return false;
			}
		}
		catch (...)
		{
			return false;
		}
	}
	else
	{
		try
		{
			instance = utils::make_ref_count_ptr<memory_row_impl>(key, info, parser, parent);
		}
		catch (...)
		{
			return false;
		}
	}

	*row = instance;
	(*row)->add_ref();
	return true;
}
