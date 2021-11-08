#pragma once
#include <core/buffer_interface.h>
#include <core/guid.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/signal.hpp>
#include <utils/scope_guard.hpp>
#include <utils/strings.hpp>

#define BOOST_DATE_TIME_NO_LIB

#ifdef _WIN32
#include <Windows.h>
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/sync/windows/mutex.hpp>
#include <boost/interprocess/sync/windows/condition.hpp>
#define IPC_MUTEX boost::interprocess::ipcdetail::windows_mutex
#define IPC_CONDITION boost::interprocess::ipcdetail::windows_condition
#else
#include <boost/interprocess/shared_memory_object.hpp>
#define IPC_MUTEX boost::interprocess::interprocess_mutex
#define IPC_CONDITION boost::interprocess::interprocess_condition
#endif

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <string>
#include <atomic>
#include <mutex>
#include <string>
#include <functional>
#include <thread>

static constexpr uint32_t UNDEFINED_INDEX = (std::numeric_limits<uint32_t>::max)();
static constexpr int64_t TIMEOUT_BEFORE_REMAPPING = 2000; // 2 Seconds...
static constexpr uint32_t MAX_RERTIES_TO_ACQUIRE_SHARED_LOCK = 10;

// {AA315FB1-B710-47D4-B082-F831EB463261}
static constexpr core::guid SESSION_SEGMENT_IDENTIFIER = { 0xaa315fb1, 0xb710, 0x47d4,{ 0xb0, 0x82, 0xf8, 0x31, 0xeb, 0x46, 0x32, 0x61 } };

// {C0F94C1B-1187-45D1-8001-8CA4B525CF9C}
static constexpr core::guid STREAM_SEGMENT_IDENTIFIER = { 0xc0f94c1b, 0x1187, 0x45d1,{ 0x80, 0x1, 0x8c, 0xa4, 0xb5, 0x25, 0xcf, 0x9c } };

enum shared_memory_access_mode
{
	WRITER,
	READER
};

class shared_memory_buffer : public utils::ref_count_base<core::buffer_interface>
{
private:
	struct shm_remove
	{
	private:
		std::string m_shared_memory_name;

	public:
		shm_remove() :
			m_shared_memory_name("")
		{			
		}

#ifndef _WIN32
		shm_remove(const char* shared_memory_name) :
			m_shared_memory_name(shared_memory_name)
		{
			if (m_shared_memory_name.empty() == false)
				boost::interprocess::shared_memory_object::remove(m_shared_memory_name.c_str());
		}

		~shm_remove()
		{
			if (m_shared_memory_name.empty() == false)
				boost::interprocess::shared_memory_object::remove(m_shared_memory_name.c_str());
		}
#endif		
	};

	std::string m_name;
	size_t m_size;
	shm_remove m_remover;
	shared_memory_access_mode m_mode;
	bool m_is_mapped;
#ifdef _WIN32
	boost::interprocess::windows_shared_memory m_shared_memory;
#else
	boost::interprocess::shared_memory_object m_shared_memory;
#endif	
	boost::interprocess::mapped_region m_shared_region;	

protected:
	bool map()
	{
		m_is_mapped = false;

		if (m_mode == shared_memory_access_mode::WRITER)
		{
			try
			{
#ifdef _WIN32					
				m_shared_memory = boost::interprocess::windows_shared_memory(
					boost::interprocess::open_or_create,
					m_name.c_str(),
					boost::interprocess::mode_t::read_write,
					m_size);
#else
				m_remover = shm_remove(m_name.c_str());
				m_shared_memory = boost::interprocess::shared_memory_object(
					boost::interprocess::create_only,
					m_name.c_str(),
					boost::interprocess::mode_t::read_write);
				m_shared_memory.truncate(m_size);
#endif					
			}
			catch (...)
			{
				return false;
			}			
		}
		else if (m_mode == shared_memory_access_mode::READER)
		{
			try
			{
#ifdef _WIN32
				m_shared_memory = boost::interprocess::windows_shared_memory(
					boost::interprocess::open_only,
					m_name.c_str(),
					boost::interprocess::mode_t::read_write);
#else
				m_remover = shm_remove();
				m_shared_memory = boost::interprocess::shared_memory_object(
					boost::interprocess::open_only,
					m_name.c_str(),
					boost::interprocess::mode_t::read_write);
#endif							
			}
			catch (...)
			{
				return false;
			}
		}
		else
		{
			throw std::runtime_error("Unexpected access mode... memory corruption ???");
		}

		m_shared_region = boost::interprocess::mapped_region(m_shared_memory, boost::interprocess::read_write);
		
		if (m_size == 0)
			m_size = this->size();

		m_is_mapped = true;
		return true;
	}

public:
	// C'tor for writers
	shared_memory_buffer(const char* name, size_t size) :
		m_name(name),
		m_size(size),
		m_mode(shared_memory_access_mode::WRITER),
		m_is_mapped(false)
	{
		if (map() == false)
			throw std::runtime_error("Failed to map shared memory");
	}

	// C'tor for readers
	shared_memory_buffer(const char* name) :
		m_name(name),
		m_size(0),
		m_mode(shared_memory_access_mode::READER),
		m_is_mapped(false)
	{
		map();
	}

	virtual ~shared_memory_buffer() = default;

	const char* name() const
	{
		return m_name.c_str();
	}

	shared_memory_access_mode mode()
	{
		return m_mode;
	}

	bool is_mapped()
	{
		return m_is_mapped;
	}

	virtual size_t size() override
	{
		return m_shared_region.get_size();
	}

	virtual uint8_t* data() override
	{
		return static_cast<uint8_t*>(m_shared_region.get_address());
	}
};

class data_index
{
private:
	IPC_MUTEX m_mutex;
	IPC_CONDITION m_wait_handle;

	uint32_t m_sequence_id;
	uint32_t m_index;

public:
	data_index(uint32_t index) :
		m_sequence_id(0),
		m_index(index)
	{
	}

	uint32_t get()
	{
		boost::interprocess::scoped_lock<IPC_MUTEX> locker(m_mutex);
		return m_index;
	}

	bool set(const uint32_t& index)
	{
		// Set m_sequence_id & m_index inside the scope and unlock at the enf of it...
		{			
			bool locked = false;
			utils::scope_guard unlocker([&]()
			{
				if (locked == true)
					m_mutex.unlock();
			});

			boost::posix_time::ptime deadline = (boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(TIMEOUT_BEFORE_REMAPPING));
			try
			{
				locked = m_mutex.timed_lock(deadline);
				if (locked == false)
					return false;
			}
			catch (...)
			{
				return false;
			}

			m_sequence_id = ((m_sequence_id + 1) % (std::numeric_limits<uint32_t>::max)());
			m_index = index;
		}

		try
		{
            m_wait_handle.notify_all();
            return true;
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	uint32_t wait(const std::function<bool()>& wait_condition, int64_t timeout)
	{		
		uint32_t retval = UNDEFINED_INDEX;

		try
		{
			boost::interprocess::scoped_lock<IPC_MUTEX> locker(m_mutex);
			uint32_t sequence_id = m_sequence_id;

			auto now = boost::posix_time::microsec_clock::universal_time();
			while (wait_condition() == true)
			{
				boost::posix_time::ptime deadline = (boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(10));
				bool got_buffer_index = m_wait_handle.timed_wait(locker, deadline, [&]()
				{
					return (m_sequence_id != sequence_id);
				});

				if (wait_condition() == false)
				{
					retval = UNDEFINED_INDEX;
					break;
				}

				if (got_buffer_index)
				{
					retval = m_index;
					break;
				}

				if ((boost::posix_time::microsec_clock::universal_time() - now).total_milliseconds() > timeout)
				{
					retval = UNDEFINED_INDEX;
					break;
				}
			}
		}
		catch (...)
		{
			retval = UNDEFINED_INDEX;
		}

		return retval;
	}
};

class data_ownership_handler
{
private:
	boost::interprocess::interprocess_sharable_mutex m_mutex;

public:
	data_ownership_handler()
	{
	}

	bool lock_write()
	{
		return m_mutex.try_lock();
	}

	void unlock_write()
	{
		m_mutex.unlock();
	}

	bool lock_read()
	{
		bool retval = false;

		for (uint32_t i = 0; i < MAX_RERTIES_TO_ACQUIRE_SHARED_LOCK; ++i)
		{
			retval = m_mutex.try_lock_sharable();
			if (retval == true)			
				break;			
		}

		return retval;
	}

	void unlock_read()
	{
		m_mutex.unlock_sharable();
	}
};

class costum_deleter_relative_buffer : public utils::ref_count_relative_buffer
{
private:
	std::function<void()> m_releaser;

public:
	costum_deleter_relative_buffer(const std::function<void()>& releaser, core::buffer_interface* buffer, size_t offset, size_t size) :		
        ref_count_relative_buffer(buffer, offset, size),
        m_releaser(releaser)
	{
	}

	virtual ~costum_deleter_relative_buffer()
	{
		if (m_releaser != nullptr)
			m_releaser();
	}
};

class shared_memory_stream_buffer :
	public shared_memory_buffer
{
private:
	uint32_t m_pool_size;
	uint32_t m_buffer_size;

	core::guid get_segment_identifier()
	{
		core::guid retval;
		std::memcpy(&retval, data(), sizeof(core::guid));
		return retval;
	}	

	uint32_t get_pool_size()
	{
		uint32_t retval;
		std::memcpy(&retval, data() + sizeof(core::guid), sizeof(uint32_t));
		return retval;
	}

	uint32_t get_buffer_size()
	{
		uint32_t retval;
		std::memcpy(&retval, data() + sizeof(core::guid) + sizeof(uint32_t), sizeof(uint32_t));
		return retval;
	}

public:
	// C'tor for writers
	shared_memory_stream_buffer(const char* name, uint32_t pool_size, uint32_t buffer_size) :
		shared_memory_buffer(name, 
		sizeof(core::guid) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(data_index) + (pool_size * sizeof(data_ownership_handler)) + (pool_size * buffer_size)),
		m_pool_size(pool_size),
		m_buffer_size(buffer_size)
	{
		if (is_mapped() == false)
			throw std::runtime_error("Allocation failed");

		construct();		
	}

	// C'tor for readers
	shared_memory_stream_buffer(const char* name) :
		shared_memory_buffer(name),
		m_pool_size(0),
		m_buffer_size(0)
	{
		if (is_mapped() == false)
			return;

		m_pool_size = get_pool_size();
		m_buffer_size = get_buffer_size();
	}

	virtual ~shared_memory_stream_buffer()
	{
		//destroy();
	}

	bool destroy()
	{
		if (mode() != shared_memory_access_mode::WRITER)
			return false;		

		for (uint32_t i = 0; i < pool_size(); i++)
		{
			data_ownership_handler* handler = get_handler(i);
			if (handler == nullptr)
			{
				// TODO: Log unexpected...
				continue;
			}

			// Destruct
			handler->~data_ownership_handler();
		}

		data_index* index_handler = get_data_index();
		if (index_handler == nullptr)
		{
			// TODO: Log unexpected...
		}

		index_handler->~data_index();
		return true;
	}

	bool construct()
	{
		if (mode() != shared_memory_access_mode::WRITER)
			return false;		

		uint8_t* base_addr = this->data();
		std::memcpy(base_addr, &STREAM_SEGMENT_IDENTIFIER, sizeof(core::guid));
		base_addr += sizeof(core::guid);

		std::memcpy(base_addr, &m_pool_size, sizeof(uint32_t));
		base_addr += sizeof(uint32_t);

		std::memcpy(base_addr, &m_buffer_size, sizeof(uint32_t));
		base_addr += sizeof(uint32_t);

		new (base_addr) data_index(UNDEFINED_INDEX);
		base_addr += sizeof(data_index);

		for (uint32_t i = 0; i < m_pool_size; i++)
		{
			new (base_addr) data_ownership_handler();
			base_addr += sizeof(data_ownership_handler);
		}

		get_data_index()->set(UNDEFINED_INDEX);
		return true;
	}

	bool reconstruct()
	{	
		return (/*destroy() && */construct());		
	}

	bool remap()
	{
		if (map() == false)
			return false;

		if (mode() == shared_memory_access_mode::WRITER)
		{
			construct();
		}
		else // if (mode() == shared_memory_access_mode::READER)
		{
			m_pool_size = get_pool_size();
			m_buffer_size = get_buffer_size();
		}
	}

	uint32_t pool_size()
	{
		return m_pool_size;
	}

	uint32_t buffer_size()
	{
		return m_buffer_size;
	}

	data_index* get_data_index()
	{
		return static_cast<data_index*>(static_cast<void*>(this->data() + sizeof(core::guid) + sizeof(uint32_t) + sizeof(uint32_t)));
	}

	data_ownership_handler* get_handler(uint32_t index)
	{
		if (index >= m_pool_size)
			return nullptr;

		return static_cast<data_ownership_handler*>(static_cast<void*>(this->data() + sizeof(core::guid) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(data_index) + (index * sizeof(data_ownership_handler))));
	}	

	size_t get_buffer_offset(uint32_t index)
	{
		return sizeof(core::guid) + sizeof(data_index) + sizeof(uint32_t) + sizeof(uint32_t) + (pool_size() * sizeof(data_ownership_handler)) + (index * buffer_size());
	}
};

class shared_memory_error_callback : public core::ref_count_interface
{
public:
	virtual ~shared_memory_error_callback() = default;
	virtual void on_error() = 0;
};

class shared_memory_stream_controller : public utils::ref_count_base<core::ref_count_interface>
{
private:
	mutable std::mutex m_mutex;
	shared_memory_access_mode m_mode;
	
	uint32_t m_pool_size;
	uint32_t m_buffer_size;
	utils::ref_count_ptr<shared_memory_stream_buffer> m_stream_buffer;

	uint32_t m_next_write_buffer_index;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_writer_out_of_resources_timestamp;	

	bool map(const char* stream_name)
	{
		if (stream_name == nullptr)
			return false;

		std::lock_guard<std::mutex> locker(m_mutex);
		m_stream_buffer.release();

		if (m_mode == shared_memory_access_mode::WRITER)
		{
			m_stream_buffer = utils::make_ref_count_ptr<shared_memory_stream_buffer>(stream_name, m_pool_size, m_buffer_size);
			if (is_valid() == false)
				return false;
		}
		else if (m_mode == shared_memory_access_mode::READER)
		{
			m_stream_buffer = utils::make_ref_count_ptr<shared_memory_stream_buffer>(stream_name);
			if (is_valid() == false)
				return false;

			m_pool_size = m_stream_buffer->pool_size();
			m_buffer_size = m_stream_buffer->buffer_size();
			if (m_pool_size == 0 || m_buffer_size == 0)
				return false;
		}
		else
		{
			throw std::runtime_error("Unexpected access mode... memory corruption ???");
		}

		return true;
	}

	bool is_valid() const
	{
		if (m_stream_buffer == nullptr ||
			m_stream_buffer->is_mapped() == false ||
			m_stream_buffer->pool_size() == 0 ||
			m_stream_buffer->buffer_size() == 0)
			return false;

		return true;
	}

	bool query_stream_buffer(shared_memory_stream_buffer** buffer) const
	{
		if (buffer == nullptr)
			return false;

		std::lock_guard<std::mutex> locker(m_mutex);
		if (is_valid() == false)
			return false;

		*buffer = m_stream_buffer;
		(*buffer)->add_ref();
		return true;
	}

	bool query_buffer(shared_memory_stream_buffer* stream, uint32_t index, shared_memory_access_mode access_mode, core::buffer_interface** buffer, shared_memory_error_callback* error_callback = nullptr)
	{
		if (stream == nullptr)
			return false;

		auto offset = m_stream_buffer->get_buffer_offset(index);
		auto size = m_stream_buffer->buffer_size();

		utils::ref_count_ptr<shared_memory_stream_buffer> video_stream_buffer = stream;
		utils::ref_count_ptr<shared_memory_error_callback> owned_error_callback = error_callback;
		*buffer = utils::make_ref_count_ptr<costum_deleter_relative_buffer>(
			[this, index, access_mode, video_stream_buffer, owned_error_callback]()
		{
			if (access_mode == shared_memory_access_mode::READER)
			{
				data_ownership_handler* handler = video_stream_buffer->get_handler(index);
				if (handler == nullptr)
					throw std::runtime_error("Failed to [SHARED] unlock resource");

				handler->unlock_read();
			}
			else if (access_mode == shared_memory_access_mode::WRITER)
			{
				data_ownership_handler* handler = video_stream_buffer->get_handler(index);
				if (handler == nullptr)
					throw std::runtime_error("Failed to [UNIQUE] unlock resource");

				handler->unlock_write();

				data_index* data_index = video_stream_buffer->get_data_index();
				if (data_index == nullptr)
					throw std::runtime_error("Failed to get data index");

				if (data_index->set(index) == false)
				{
					if (owned_error_callback != nullptr)
						owned_error_callback->on_error();
				}
			}
			else
			{
				throw std::runtime_error("Unexpected access mode... memory corruption ???");
			}
		}, video_stream_buffer, offset, size).detach();

		return true;
	}

	bool query_read_buffer(uint32_t index, core::buffer_interface** buffer, shared_memory_error_callback* error_callback = nullptr)
	{
		if (buffer == nullptr)
			return false;

		utils::ref_count_ptr<shared_memory_stream_buffer> stream_buffer;
		if (query_stream_buffer(&stream_buffer) == false)
			return false;

		if (index >= stream_buffer->pool_size())
			return false;

		data_ownership_handler* handler = stream_buffer->get_handler(index);
		if (handler == nullptr)
			throw std::runtime_error("Unexpceted 'data_ownership_handler = nullptr'");

		if (handler->lock_read() == false)
			return false;

		bool success;
		utils::scope_guard failure_unlocker([&handler, &success]()
		{
			if (success == false)
				handler->unlock_read();
		});

		success = query_buffer(stream_buffer, index, shared_memory_access_mode::READER, buffer, error_callback);
		return success;
	}

public:		
	shared_memory_stream_controller(const char* name, uint32_t pool_size, uint32_t buffer_size) :
		m_mode(shared_memory_access_mode::WRITER),
		m_pool_size(pool_size),
		m_buffer_size(buffer_size),
		m_next_write_buffer_index(0),
		m_writer_out_of_resources_timestamp((std::chrono::time_point<std::chrono::high_resolution_clock>::max)())
	{
		if (pool_size == 0)
			throw std::invalid_argument("pool_size");

		if (buffer_size == 0)
			throw std::invalid_argument("buffer_size");

		map(name);
	}

	shared_memory_stream_controller(const char* name) :
		m_mode(shared_memory_access_mode::READER),
		m_pool_size(0),
		m_buffer_size(0),
		m_next_write_buffer_index(0),
		m_writer_out_of_resources_timestamp((std::chrono::time_point<std::chrono::high_resolution_clock>::max)())
	{
		map(name);
	}

	shared_memory_stream_controller(const shared_memory_stream_controller& other) :
		m_mode(other.m_mode),
		m_pool_size(other.m_pool_size),
		m_buffer_size(other.m_buffer_size),
		m_stream_buffer(other.m_stream_buffer),
		m_next_write_buffer_index(other.m_next_write_buffer_index)
	{
	}

	const char* name() const
	{
		utils::ref_count_ptr<shared_memory_stream_buffer> stream_buffer;
		if (query_stream_buffer(&stream_buffer) == false)
			return nullptr;

		return stream_buffer->name();
	}

	bool query_write_buffer(core::buffer_interface** buffer, shared_memory_error_callback* error_callback = nullptr)
	{
		bool retval = false;
		if (buffer == nullptr)
			return retval;

		utils::scope_guard writer_resources_manager([&]()
		{
			if (retval == true)
			{
				m_writer_out_of_resources_timestamp = (std::chrono::time_point<std::chrono::high_resolution_clock>::max)();
			}
			else
			{
				if (m_writer_out_of_resources_timestamp == (std::chrono::time_point<std::chrono::high_resolution_clock>::max)())
				{
					m_writer_out_of_resources_timestamp = std::chrono::high_resolution_clock::now();
				}
				else
				{
					if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_writer_out_of_resources_timestamp).count() > TIMEOUT_BEFORE_REMAPPING)
					{
						m_writer_out_of_resources_timestamp = (std::chrono::time_point<std::chrono::high_resolution_clock>::max)();

						if (error_callback != nullptr)
							error_callback->on_error();
					}
				}
			}
		});

		if (m_mode != shared_memory_access_mode::WRITER)
			return retval;

		utils::ref_count_ptr<shared_memory_stream_buffer> stream_buffer;
		if (query_stream_buffer(&stream_buffer) == false)
			return retval;

		uint32_t pool_size = m_stream_buffer->pool_size();
		for (uint32_t i = 0; i < pool_size; ++i)
		{
			uint32_t current_index = ((m_next_write_buffer_index + i) % pool_size);
			data_ownership_handler* handler = stream_buffer->get_handler(current_index);
			if (handler == nullptr)
				throw std::runtime_error("Unexpceted 'data_ownership_handler == nullptr'");

			if (handler->lock_write() == false)
				continue;

			bool success;
			utils::scope_guard failure_unlocker([&handler, &success]()
			{
				if (success == false)
					handler->unlock_write();
			});

			success = query_buffer(stream_buffer, current_index, shared_memory_access_mode::WRITER, buffer, error_callback);
			m_next_write_buffer_index = ((current_index + 1) % pool_size);

			if (success == true)
			{
				retval = true;
				break;
			}			
		}

		return retval;
	}

	bool query_read_buffer(const std::function<bool()>& wait_condition, int64_t timeout, core::buffer_interface** buffer, shared_memory_error_callback* error_callback = nullptr)
	{
		utils::ref_count_ptr<shared_memory_stream_buffer> stream_buffer;
		if (query_stream_buffer(&stream_buffer) == false)
			return false;

		data_index* data_index_ptr = stream_buffer->get_data_index();
		uint32_t index = data_index_ptr->wait(wait_condition, timeout);
		if (index == UNDEFINED_INDEX)
			return false;

		return query_read_buffer(index, buffer, error_callback);
	}
};

class shared_memory_stream_wrapper : public shared_memory_error_callback
{
private:
	mutable std::mutex m_mutex;
	utils::ref_count_ptr<shared_memory_stream_controller> m_stream;	

public:
	utils::signal<shared_memory_stream_wrapper> OnError;

	shared_memory_stream_wrapper()
	{
	}

	virtual ~shared_memory_stream_wrapper() = default;
	
	virtual void on_error() override
	{
		OnError();
	}

	void set_stream(shared_memory_stream_controller* stream)
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		m_stream = stream;
	}

	bool query_stream(shared_memory_stream_controller** stream) const
	{
		if (stream == nullptr)
			return false;

		std::lock_guard<std::mutex> locker(m_mutex);
		if (m_stream == nullptr)
			return false;

		*stream = m_stream;
		(*stream)->add_ref();
		return true;
	}

	const char* name() const
	{
		utils::ref_count_ptr<shared_memory_stream_controller> stream;
		if (query_stream(&stream) == false)
			return nullptr;

		return stream->name();
	}
};

class shared_memory_stream_reader : public utils::ref_count_base<shared_memory_stream_wrapper>
{
private:
	std::mutex m_mutex;
	std::atomic<bool> m_running;
	std::thread m_listening_thread;

	bool query_read_buffer(const std::function<bool()>& wait_condition, int64_t timeout, core::buffer_interface** buffer)
	{
		utils::ref_count_ptr<shared_memory_stream_controller> stream;
		if (query_stream(&stream) == false ||
			stream->query_read_buffer(wait_condition, timeout, buffer, this) == false)
			return false;

		return true;
	}

public:
	utils::signal<shared_memory_stream_reader, core::buffer_interface*> OnBuffer;	

	shared_memory_stream_reader() :
		m_running(false)
	{
	}

	~shared_memory_stream_reader()
	{
		stop();
	}

	void start()
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		if (m_running == true)
			return;

		m_running = true;
		m_listening_thread = std::thread([this]()
		{
			auto wait_condition = [this]()
			{
				return (m_running == true);
			};

			while (m_running == true)
			{
				utils::ref_count_ptr<core::buffer_interface> buffer;
				if (query_read_buffer(wait_condition, TIMEOUT_BEFORE_REMAPPING, &buffer) == false)
				{
					auto now = std::chrono::high_resolution_clock::now();
					while (m_running == true)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() > TIMEOUT_BEFORE_REMAPPING)
						{
							on_error();
							if (query_read_buffer(wait_condition, TIMEOUT_BEFORE_REMAPPING, &buffer) == true)
								break;

							now = std::chrono::high_resolution_clock::now();
						}
					}

					if (buffer == nullptr)
						continue;
				}

				OnBuffer(buffer);
			}
		});
	}

	void stop()
	{
		std::lock_guard<std::mutex> locker(m_mutex);

		if (m_running == true)
		{
			m_running = false;
			if (m_listening_thread.joinable())
				m_listening_thread.join();
		}
	}
};

class shared_memory_stream_writer : public utils::ref_count_base<shared_memory_stream_wrapper>
{
public:
	bool query_write_buffer(core::buffer_interface** buffer)
	{
		utils::ref_count_ptr<shared_memory_stream_controller> stream;
		utils::ref_count_ptr<core::buffer_interface> local_buffer;
		if (query_stream(&stream) == false ||
			stream->query_write_buffer(buffer, this) == false)
			return false;

		return true;
	}
};

class shared_memory_stream_params
{
private:
	uint32_t m_pool_size;
	uint32_t m_buffer_size;

public:
	shared_memory_stream_params() :
		m_pool_size(0),
		m_buffer_size(0)
	{
	}

	shared_memory_stream_params(uint32_t pool_size, uint32_t buffer_size) :
		m_pool_size(pool_size),
		m_buffer_size(buffer_size)
	{
	}

	bool undefined()
	{
		return (m_pool_size == 0 || m_buffer_size == 0);
	}

	uint32_t pool_size()
	{
		return m_pool_size;
	}

	uint32_t buffer_size()
	{
		return m_buffer_size;
	}
};

class shared_memory_session_buffer :
	public shared_memory_buffer
{
private:
	class guid_generator
	{
	private:
		boost::uuids::random_generator m_generator;

	public:
		core::guid generate()
		{
			boost::uuids::uuid uuid = m_generator();
			core::guid retval;

			static_assert(boost::uuids::uuid::static_size() == sizeof(core::guid), "Cannot convert boost::uuids::uuid to core::guid. Different size...");
			std::memcpy(&retval, &uuid, sizeof(core::guid));
			return retval;
		}
	};

	guid_generator m_generator;

	bool write_segment_identifier()
	{
		if (mode() != shared_memory_access_mode::WRITER)
			return false;

		uint8_t* base_addr = this->data();
		std::memcpy(base_addr, &SESSION_SEGMENT_IDENTIFIER, sizeof(core::guid));

		return true;
	}

	bool write_stream_count(uint32_t stream_count)
	{
		if (mode() != shared_memory_access_mode::WRITER)
			return false;

		uint8_t* base_addr = this->data() + sizeof(core::guid);
		std::memcpy(base_addr, &stream_count, sizeof(uint32_t));

		return true;
	}

	bool write_stream_identifiers(uint32_t stream_count)
	{
		if (mode() != shared_memory_access_mode::WRITER)
			return false;

		uint8_t* base_addr = this->data() + sizeof(core::guid) + sizeof(uint32_t);

		for (uint32_t i = 0; i < stream_count; i++)
		{
			core::guid id = m_generator.generate();
			std::memcpy(base_addr + (i * sizeof(core::guid)), &id, sizeof(core::guid));
		}

		return true;
	}

	core::guid read_segment_identifier()
	{
		core::guid retval;
		std::memcpy(&retval, data(), sizeof(core::guid));
		return retval;
	}

public:
	// C'tor for writers
	shared_memory_session_buffer(const char* name, uint32_t stream_count) :
        shared_memory_buffer(name, (sizeof(core::guid) * (stream_count + 1)))
	{
		if (this->size() < (sizeof(core::guid) * (stream_count + 1)))
			throw std::runtime_error("Invlaid buffer size");

		// Initialize the shared memory structure if not already initialized
		if (read_segment_identifier() != SESSION_SEGMENT_IDENTIFIER)
			write_segment_identifier();

		write_stream_count(stream_count);
		write_stream_identifiers(stream_count);
	}

	// C'tor for readers
	shared_memory_session_buffer(const char* name) :
		shared_memory_buffer(name)
	{
	}

	uint32_t stream_count()
	{
		if (is_mapped() == false)
			return 0;

		size_t offset = sizeof(core::guid);
		if (size() < offset)
			return 0;

		uint32_t retval;
		uint8_t* base_addr = this->data() + offset;
		std::memcpy(&retval, base_addr, sizeof(uint32_t));
		return retval;
	}

	core::guid get_stream_identifier(uint32_t index)
	{
		if (is_mapped() == false)
			return core::guid::undefined();

		size_t offset = sizeof(core::guid) + sizeof(uint32_t) + (sizeof(core::guid) * index);
		if (size() < offset)
			return core::guid::undefined();

		core::guid retval;
		uint8_t* base_addr = this->data() + offset;
		std::memcpy(&retval, base_addr, sizeof(core::guid));

		return retval;
	}
};

class shared_memory_session_writer : public utils::ref_count_base<core::ref_count_interface>
{
private:
	std::string m_name;
	std::mutex m_mutex;
	
	utils::ref_count_ptr<shared_memory_session_buffer> m_session_buffer;	
	std::vector<shared_memory_stream_params> m_description;
	std::vector<utils::ref_count_ptr<shared_memory_stream_writer>> m_writers;

	bool remap()
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		m_session_buffer.release();

		uint32_t stream_count = static_cast<uint32_t>(m_description.size());		
		utils::ref_count_ptr<shared_memory_session_buffer> session_buffer = utils::make_ref_count_ptr<shared_memory_session_buffer>(m_name.c_str(), stream_count);
		if (session_buffer->is_mapped() == false)
			return false;				

		for (uint32_t i = 0; i < stream_count; i++)
		{
			m_writers[i]->set_stream(nullptr);			
			core::guid id = session_buffer->get_stream_identifier(i);
			if (id == core::guid::undefined())
				continue;
				
			std::stringstream stream;
			stream << id;

			utils::ref_count_ptr<shared_memory_stream_controller> stream_controller =
				utils::make_ref_count_ptr<shared_memory_stream_controller>(stream.str().c_str(), m_description[i].pool_size(), m_description[i].buffer_size());

			m_writers[i]->set_stream(stream_controller);		
		}

		m_session_buffer = session_buffer;
		return true;		
	}

public:
	shared_memory_session_writer(const char* name, shared_memory_stream_params* streams, uint32_t stream_count) :
		m_name(name)
	{
		if (name == nullptr)
			throw std::invalid_argument("name");

		if (streams == nullptr)
			throw std::invalid_argument("streams");

		if (stream_count == 0)
			throw std::invalid_argument("stream_count");

		m_description.resize(stream_count);		
		m_writers.resize(stream_count);		

		for (size_t i = 0; i < stream_count; i++)
		{
			m_description[i] = streams[i];
		}

		for (uint32_t i = 0; i < stream_count; i++)
		{
			if (m_writers[i] == nullptr)
			{
				utils::ref_count_ptr<shared_memory_stream_writer> writer =
					utils::make_ref_count_ptr<shared_memory_stream_writer>();

				writer->OnError += [this]()
				{
					remap();
				};

				m_writers[i] = writer;
			}
		}

		remap();
	}

	bool query_write_buffer(uint32_t index, core::buffer_interface** buffer)
	{
		if (buffer == nullptr)
			return false;

		std::unique_lock<std::mutex> locker(m_mutex);
		if (index >= m_writers.size())
			return false;

		utils::ref_count_ptr<shared_memory_stream_writer> writer = m_writers[index];
		locker.unlock();

		if (writer == nullptr)
			return false;

		return writer->query_write_buffer(buffer);
	}
};

class shared_memory_session_reader : public utils::ref_count_base<core::ref_count_interface>
{
private:
	std::string m_name;
	std::mutex m_mutex;
	uint32_t m_stream_count;

	utils::ref_count_ptr<shared_memory_session_buffer> m_session_buffer;
	std::vector<utils::ref_count_ptr<shared_memory_stream_reader>> m_readers;

	bool create_reader(uint32_t index, shared_memory_stream_reader** reader)
	{
		if (reader == nullptr)
			return false;

		utils::ref_count_ptr<shared_memory_stream_reader> instance;
		try
		{
			instance = utils::make_ref_count_ptr<shared_memory_stream_reader>();

			instance->OnError += [this]()
			{
				remap();
			};

			instance->OnBuffer += [this, index](core::buffer_interface* buffer)
			{
				OnBuffer(index, buffer);
			};
		}
		catch (...)
		{
			return false;
		}

		if (instance == nullptr)
			return false;

		*reader = instance;
		(*reader)->add_ref();
		return true;
	}

	bool remap()
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		m_session_buffer.release();
		
		utils::ref_count_ptr<shared_memory_session_buffer> session_buffer;
		session_buffer = utils::make_ref_count_ptr<shared_memory_session_buffer>(m_name.c_str());
		if (session_buffer == nullptr || session_buffer->is_mapped() == false)
			return false;

		m_session_buffer = session_buffer;
		

		uint32_t stream_count = m_session_buffer->stream_count();

		if (m_stream_count == 0)
			m_readers.resize(stream_count);
		else
			stream_count = (std::min)(m_stream_count, stream_count);

		for (uint32_t i = 0; i < stream_count; i++)
		{
			if (m_readers[i] == nullptr)
			{
				utils::ref_count_ptr<shared_memory_stream_reader> reader;
				if (create_reader(i, &reader) == false)
					throw std::runtime_error("Failed to create stream reader");				

				m_readers[i] = reader;
			}
		}		

		for (uint32_t i = 0; i < stream_count; i++)
		{
			core::guid id = m_session_buffer->get_stream_identifier(i);
			if (id == core::guid::undefined())
			{
				m_readers[i]->set_stream(nullptr);
				continue;
			}


			std::stringstream stream;
			stream << id;

			std::string id_str = stream.str();
			if (m_readers[i]->name() == nullptr ||
				std::strcmp(m_readers[i]->name(), id_str.c_str()) != 0)
			{
				m_readers[i]->set_stream(nullptr);
				utils::ref_count_ptr<shared_memory_stream_controller> stream_controller =
					utils::make_ref_count_ptr<shared_memory_stream_controller>(id_str.c_str());

				m_readers[i]->set_stream(stream_controller);
			}			
		}

		return true;
	}

	bool query_reader(uint32_t index, shared_memory_stream_reader** reader)
	{
		if (reader == nullptr)
			return false;

		std::lock_guard<std::mutex> locker(m_mutex);
		if (index >= m_readers.size())
			return false;

		utils::ref_count_ptr<shared_memory_stream_reader> local_reader = m_readers[index];
		if (reader == nullptr)
			return false;

		*reader = local_reader;
		(*reader)->add_ref();
		return true;
	}

public:
	utils::signal < shared_memory_session_reader, uint32_t, core::buffer_interface*> OnBuffer;

	shared_memory_session_reader(const char* name, uint32_t stream_count = 0) :
		m_name(name),
		m_stream_count(stream_count)
	{
		if (name == nullptr)
			throw std::invalid_argument("name");

		if (stream_count > 0)
		{
			m_readers.resize(stream_count);

			for (uint32_t i = 0; i < stream_count; i++)
			{
				utils::ref_count_ptr<shared_memory_stream_reader> reader;
				if (create_reader(i, &reader) == false)
					throw std::runtime_error("Failed to create stream reader");

				m_readers[i] = reader;
			}
		}

		remap();
	}

	bool start(uint32_t index)
	{
		utils::ref_count_ptr<shared_memory_stream_reader> reader;
		if (query_reader(index, &reader) == false)
			return false;

		reader->start();
		return true;
	}

	bool stop(uint32_t index)
	{
		utils::ref_count_ptr<shared_memory_stream_reader> reader;
		if (query_reader(index, &reader) == false)
			return false;

		reader->stop();
		return true;
	}
};