#pragma once
#include <core/buffer_interface.h>
#include <core/guid.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/signal.hpp>
#include <utils/scope_guard.hpp>
#include <utils/buffer_allocator.hpp>
#include <utils/strings.hpp>

#define BOOST_DATE_TIME_NO_LIB

#include <string>
#include <atomic>
#include <mutex>
#include <string>
#include <functional>
#include <thread>

#ifdef _WIN32
#include <Windows.h>

#ifdef __GNUC__
#   pragma GCC diagnostic ignored "-Wconversion-null"
#   pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

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
//#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace shared_memory
{
	static constexpr uint32_t UNDEFINED_INDEX = (std::numeric_limits<uint32_t>::max)();
	static constexpr int64_t TIMEOUT_BEFORE_REMAPPING = 2000; // 2 Seconds...
	static constexpr uint32_t MAX_RERTIES_TO_ACQUIRE_SHARED_LOCK = 20;
	static constexpr uint32_t MAX_BLOCKING_TIME_MS = 100;
	static constexpr uint32_t ITERATIONS = (TIMEOUT_BEFORE_REMAPPING / MAX_BLOCKING_TIME_MS);

	// {C0F94C1B-1187-45D1-8001-8CA4B525CF9C}
	static constexpr core::guid BUFFER_POOL_SEGMENT_IDENTIFIER = { 0xc0f94c1b, 0x1187, 0x45d1,{ 0x80, 0x1, 0x8c, 0xa4, 0xb5, 0x25, 0xcf, 0x9c } };

	// {AA315FB1-B710-47D4-B082-F831EB463261}
	static constexpr core::guid SESSION_SEGMENT_IDENTIFIER = { 0xaa315fb1, 0xb710, 0x47d4,{ 0xb0, 0x82, 0xf8, 0x31, 0xeb, 0x46, 0x32, 0x61 } };

	static constexpr uint32_t MAX_SHARABLE_POOLS_PER_SESSION = 5;

	class guid_generator
	{
	private:
		boost::uuids::random_generator m_generator;

	public:
		core::guid generate()
		{
			boost::uuids::uuid uuid = m_generator();
			core::guid retval;

#ifndef BOOST_NO_CXX11_CONSTEXPR
			static_assert(boost::uuids::uuid::static_size() == sizeof(core::guid), "Cannot convert boost::uuids::uuid to core::guid. Different size...");
#endif

			std::memcpy(&retval, &uuid, sizeof(core::guid));
			return retval;
		}
	};

	enum access_mode
	{
		NONE,
		READER,
		WRITER
	};

	enum error_codes
	{
		SHM_NO_ERROR,
		SHM_INVALID_ARGUMENT,
		SHM_TIMEOUT,
		SHM_NOT_MAPPED,
		SHM_POOL_DOES_NOT_EXIST,
		SHM_LOCK_UNIQUE_FAILED,
		SHM_LOCK_SHARED_FAILED,
		SHM_PERMISSION_DENIED,
		SHM_DATA_CORRUPTION
	};

	class shm_buffer_interface :
		public core::buffer_interface
	{
	public:
		virtual ~shm_buffer_interface() = default;

		virtual const char* name() const = 0;
		virtual bool is_mapped() const = 0;
		virtual shared_memory::access_mode mode() const = 0;
	};

	class shm_sharable_buffer_interface :
		public core::buffer_interface
	{
	public:
		virtual ~shm_sharable_buffer_interface() = default;

		virtual bool unlock() = 0;
		virtual bool lock_unique() = 0;
		virtual bool unlock_unique() = 0;
		virtual bool lock_shared() = 0;
		virtual bool unlock_shared() = 0;
		virtual error_codes publish() = 0;
	};

	class shm_bufferpool_interface
	{

	};

	class shm_buffer :
		public utils::ref_count_base<shared_memory::shm_buffer_interface>
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
		access_mode m_mode;
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

			if (m_mode == access_mode::WRITER)
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
                    m_shared_memory.truncate(static_cast<boost::interprocess::offset_t>(m_size));
#endif					
				}
				catch (...)
				{
					return false;
				}
			}
			else if (m_mode == access_mode::READER)
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
		shm_buffer(const char* name, size_t size) :
			m_name(name),
			m_size(size),
			m_mode(access_mode::WRITER),
			m_is_mapped(false)
		{
			if (map() == false)
				throw std::runtime_error("Failed to map shared memory");
		}

		// C'tor for readers
		shm_buffer(const char* name) :
			m_name(name),
			m_size(0),
			m_mode(access_mode::READER),
			m_is_mapped(false)
		{
			map();
		}

		virtual ~shm_buffer() = default;

		virtual const char* name() const override
		{
			return m_name.c_str();
		}

		virtual access_mode mode() const override
		{
			return m_mode;
		}

		virtual bool is_mapped() const override
		{
			return m_is_mapped;
		}

		virtual size_t size() const override
		{
			return m_shared_region.get_size();
		}

		virtual uint8_t* data() override
		{
			return static_cast<uint8_t*>(m_shared_region.get_address());
		}
	};

	class shm_sharable_bufferpool :
		public shm_buffer
	{
	private:
		class shm_bufferpool_wait_handler
		{
		private:
			IPC_MUTEX m_mutex;
			IPC_CONDITION m_wait_handle;

			uint32_t m_sequence_id;
			uint32_t m_index;

		public:
			shm_bufferpool_wait_handler(uint32_t index) :
				m_sequence_id(0),
				m_index(index)
			{
			}

			uint32_t get()
			{
				boost::interprocess::scoped_lock<IPC_MUTEX> locker(m_mutex);
				return m_index;
			}

			error_codes set(uint32_t index)
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
							return error_codes::SHM_TIMEOUT;
					}
					catch (...)
					{
						return error_codes::SHM_DATA_CORRUPTION;
					}

					m_sequence_id = ((m_sequence_id + 1) % (std::numeric_limits<uint32_t>::max)());
					m_index = index;
				}

				error_codes retval = error_codes::SHM_NO_ERROR;
				for (uint32_t i = 0; i < ITERATIONS; i++)
				{
					try
					{
						m_wait_handle.notify_all();

						retval = error_codes::SHM_NO_ERROR;
						break;
					}
					catch (...)
					{
						retval = error_codes::SHM_DATA_CORRUPTION;
						std::this_thread::sleep_for(std::chrono::milliseconds(MAX_BLOCKING_TIME_MS));
					}
				}

				return retval;
			}

			error_codes wait(int64_t timeout, uint32_t& buffer_index)
			{
				try
				{
					boost::interprocess::scoped_lock<IPC_MUTEX> locker(m_mutex);
					uint32_t sequence_id = m_sequence_id;

					if (timeout > 0)
					{
						boost::posix_time::ptime deadline = (boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(timeout));
						bool got_buffer_index = m_wait_handle.timed_wait(locker, deadline, [&]()
						{
							return (m_sequence_id != sequence_id);
						});

						if (got_buffer_index == false)
							return error_codes::SHM_TIMEOUT;
					}
					else
					{
						m_wait_handle.wait(locker, [&]()
						{
							return (m_sequence_id != sequence_id);
						});
					}

					buffer_index = m_index;
				}
				catch (...)
				{
					return error_codes::SHM_DATA_CORRUPTION;
				}

				return error_codes::SHM_NO_ERROR;
			}
		};

		class shm_shared_buffer_mutex
		{
		private:
			//boost::interprocess::interprocess_upgradable_mutex m_mutex;
			boost::interprocess::interprocess_sharable_mutex m_mutex;

		public:
			shm_shared_buffer_mutex()
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

			/*void unlock_write_and_lock_read()
			{
			m_mutex.unlock_and_lock_sharable();
			}*/

			bool lock_read()
			{
				bool retval = false;

				for (uint32_t i = 0; i < MAX_RERTIES_TO_ACQUIRE_SHARED_LOCK; ++i)
				{
					retval = m_mutex.try_lock_sharable();
					if (retval == true)
						break;
				}

				if (retval == false)
					std::cout << "All retries failed to aquire shared lock";

				return retval;
			}

			/*bool unlock_read_and_lock_write()
			{
			bool retval = false;

			for (uint32_t i = 0; i < MAX_RERTIES_TO_ACQUIRE_SHARED_LOCK; ++i)
			{
			retval = m_mutex.try_unlock_sharable_and_lock();
			if (retval == true)
			break;
			}

			return retval;
			}*/

			void unlock_read()
			{
				m_mutex.unlock_sharable();
			}
		};

		class shm_sharable_buffer : public utils::relative_buffer_base<utils::ref_count_base<shared_memory::shm_sharable_buffer_interface>>
		{
		private:
			utils::ref_count_ptr<core::buffer_interface> m_relative_buffer;
			uint32_t m_index;
			shared_memory::access_mode m_access_mode;
			std::mutex m_mutex;

			bool unlock_unsafe()
			{
				if (m_access_mode == shared_memory::access_mode::NONE)
					return true;

				utils::ref_count_ptr<shm_sharable_bufferpool> pool;
				if (query_source_buffer((core::buffer_interface**)(&pool)) == false)
					return false;

				shm_shared_buffer_mutex* shared_mutex = pool->get_buffer_mutex(m_index);
				if (shared_mutex == nullptr)
					return false;

				if (m_access_mode == shared_memory::access_mode::WRITER)
					shared_mutex->unlock_write();
				else if (m_access_mode == shared_memory::access_mode::READER)
					shared_mutex->unlock_read();

				m_access_mode = shared_memory::access_mode::NONE;
				return true;
			}

			bool lock_unique_unsafe()
			{
				utils::ref_count_ptr<shm_sharable_bufferpool> pool;
				if (query_source_buffer((core::buffer_interface**)(&pool)) == false)
					return false;

				shm_shared_buffer_mutex* shared_mutex = pool->get_buffer_mutex(m_index);
				if (shared_mutex == nullptr)
					return false;

                if (m_access_mode == shared_memory::access_mode::READER)
					return false;

				if (shared_mutex->lock_write() == false)
					return false;

				m_access_mode = shared_memory::access_mode::WRITER;
				return true;
			}

			bool unlock_unique_unsafe()
			{
				utils::ref_count_ptr<shm_sharable_bufferpool> pool;
				if (query_source_buffer((core::buffer_interface**)(&pool)) == false)
					return false;

				shm_shared_buffer_mutex* shared_mutex = pool->get_buffer_mutex(m_index);
				if (shared_mutex == nullptr)
					return false;

				if (m_access_mode != shared_memory::access_mode::WRITER)
					return false;

				shared_mutex->unlock_write();
				m_access_mode = shared_memory::access_mode::NONE;
				return true;
			}

			bool lock_shared_unsafe()
			{
				utils::ref_count_ptr<shm_sharable_bufferpool> pool;
				if (query_source_buffer((core::buffer_interface**)(&pool)) == false)
					return false;

				shm_shared_buffer_mutex* shared_mutex = pool->get_buffer_mutex(m_index);
				if (shared_mutex == nullptr)
					return false;

				if (m_access_mode == shared_memory::access_mode::READER)
				{
					return true;
				}

				if (m_access_mode == shared_memory::access_mode::WRITER)
				{
					/*shared_mutex->unlock_write_and_lock_read();
					m_access_mode = shared_memory::access_mode::READER;
					return true;*/

					return false;
				}

				if (shared_mutex->lock_read() == false)
					return false;

				m_access_mode = shared_memory::access_mode::READER;
				return true;
			}

			bool unlock_shared_unsafe()
			{
				utils::ref_count_ptr<shm_sharable_bufferpool> pool;
				if (query_source_buffer((core::buffer_interface**)(&pool)) == false)
					return false;

				shm_shared_buffer_mutex* shared_mutex = pool->get_buffer_mutex(m_index);
				if (shared_mutex == nullptr)
					return false;

				if (m_access_mode != shared_memory::access_mode::READER)
					return false;

				shared_mutex->unlock_read();
				m_access_mode = shared_memory::access_mode::NONE;
				return true;
			}

			error_codes publish_unsafe()
			{
				if (m_access_mode != shared_memory::access_mode::WRITER)
					return error_codes::SHM_PERMISSION_DENIED;

				utils::ref_count_ptr<shm_sharable_bufferpool> pool;
				if (query_source_buffer((core::buffer_interface**)(&pool)) == false)
					return error_codes::SHM_POOL_DOES_NOT_EXIST;

				shm_bufferpool_wait_handler* wait_handler = pool->get_wait_handler();
				if (wait_handler == nullptr)
					return error_codes::SHM_DATA_CORRUPTION;

				if (unlock_unsafe() == false)
					return error_codes::SHM_LOCK_SHARED_FAILED;

				return wait_handler->set(m_index);
			}

		public:
			shm_sharable_buffer(shm_sharable_bufferpool* bufferpool, uint32_t index, shared_memory::access_mode access_mode) :
				utils::relative_buffer_base<utils::ref_count_base<shared_memory::shm_sharable_buffer_interface>>(bufferpool, bufferpool->get_buffer_offset(index), bufferpool->buffer_size()),
				m_index(index),
				m_access_mode(access_mode)
			{
			}

			virtual ~shm_sharable_buffer()
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				unlock_unsafe();
			}

			shared_memory::access_mode access_mode()
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				return m_access_mode;
			}

			virtual bool unlock() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				return unlock_unsafe();
			}

			virtual bool lock_unique() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				return lock_unique_unsafe();
			}

			virtual bool unlock_unique() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				return unlock_unique_unsafe();
			}

			virtual bool lock_shared() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				return lock_shared_unsafe();
			}

			virtual bool unlock_shared() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				return unlock_shared_unsafe();
			}

			virtual error_codes publish() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				return publish_unsafe();
			}
		};

		uint32_t m_pool_size;
		uint32_t m_buffer_size;
		uint32_t m_next_unique_query;

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

		bool destroy()
		{
			if (mode() != access_mode::WRITER)
				return false;

			for (uint32_t i = 0; i < pool_size(); i++)
			{
				shm_shared_buffer_mutex* handler = get_buffer_mutex(i);
				if (handler == nullptr)
				{
					// TODO: Log unexpected...
					continue;
				}

				// Destruct
				handler->~shm_shared_buffer_mutex();
			}

			shm_bufferpool_wait_handler* wait_handler = get_wait_handler();
			if (wait_handler == nullptr)
			{
				// TODO: Log unexpected...
			}

			wait_handler->~shm_bufferpool_wait_handler();
			return true;
		}

		bool construct()
		{
			if (mode() != access_mode::WRITER)
				return false;

			uint8_t* base_addr = this->data();
			std::memcpy(base_addr, &BUFFER_POOL_SEGMENT_IDENTIFIER, sizeof(core::guid));
			base_addr += sizeof(core::guid);

			std::memcpy(base_addr, &m_pool_size, sizeof(uint32_t));
			base_addr += sizeof(uint32_t);

			std::memcpy(base_addr, &m_buffer_size, sizeof(uint32_t));
			base_addr += sizeof(uint32_t);

			new (base_addr) shm_bufferpool_wait_handler(UNDEFINED_INDEX);
			base_addr += sizeof(shm_bufferpool_wait_handler);

			for (uint32_t i = 0; i < m_pool_size; i++)
			{
				new (base_addr) shm_shared_buffer_mutex();
				base_addr += sizeof(shm_shared_buffer_mutex);
			}

			get_wait_handler()->set(UNDEFINED_INDEX);
			return true;
		}

		bool remap()
		{
			if (map() == false)
				return false;

			if (mode() == access_mode::WRITER)
			{
				construct();
			}
			else // if (mode() == shared_memory::access_mode::READER)
			{
				m_pool_size = get_pool_size();
				m_buffer_size = get_buffer_size();
			}

            return true;
		}

		shm_bufferpool_wait_handler* get_wait_handler()
		{
			return static_cast<shm_bufferpool_wait_handler*>(static_cast<void*>(this->data() + sizeof(core::guid) + sizeof(uint32_t) + sizeof(uint32_t)));
		}

		shm_shared_buffer_mutex* get_buffer_mutex(uint32_t index)
		{
			if (index >= m_pool_size)
				return nullptr;

			return static_cast<shm_shared_buffer_mutex*>(static_cast<void*>(this->data() + sizeof(core::guid) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(shm_bufferpool_wait_handler) + (index * sizeof(shm_shared_buffer_mutex))));
		}

		bool query_sharable_buffer(uint32_t index, shared_memory::access_mode access_mode, shm_sharable_buffer_interface** buffer)
		{
			if (buffer == nullptr)
				return false;

			if (index >= m_pool_size)
				return false;

			utils::ref_count_ptr<shm_sharable_buffer_interface> instance;
			try
			{
				instance = utils::make_ref_count_ptr<shm_sharable_buffer>(this, index, access_mode);
			}
			catch (...)
			{
				return false;
			}

			if (instance == nullptr)
				return false;

			*buffer = instance;
			(*buffer)->add_ref();
			return true;
		}

	public:
		// C'tor for writers
		shm_sharable_bufferpool(const char* name, uint32_t pool_size, uint32_t buffer_size) :
			shm_buffer(name,
				sizeof(core::guid) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(shm_bufferpool_wait_handler) + (pool_size * sizeof(shm_shared_buffer_mutex)) + (pool_size * buffer_size)),
			m_pool_size(pool_size),
			m_buffer_size(buffer_size),
			m_next_unique_query(0)
		{
			if (is_mapped() == false)
				throw std::runtime_error("Allocation failed");

			construct();
		}

		// C'tor for readers
		shm_sharable_bufferpool(const char* name) :
			shm_buffer(name),
			m_pool_size(0),
			m_buffer_size(0)
		{
			if (is_mapped() == false)
				return;

			m_pool_size = get_pool_size();
			m_buffer_size = get_buffer_size();
		}

		virtual ~shm_sharable_bufferpool() = default;

		uint32_t pool_size()
		{
			return m_pool_size;
		}

		uint32_t buffer_size()
		{
			return m_buffer_size;
		}

		size_t get_buffer_offset(uint32_t index)
		{
			return sizeof(core::guid) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(shm_bufferpool_wait_handler) + (pool_size() * sizeof(shm_shared_buffer_mutex)) + (index * buffer_size());
		}

		bool query_buffer_unlocked(uint32_t index, shm_sharable_buffer_interface** buffer)
		{
			if (buffer == nullptr)
				return false;

			if (is_mapped() == false)
				return false;

			shm_shared_buffer_mutex* shared_mutex = get_buffer_mutex(index);
			if (shared_mutex == nullptr ||
				query_sharable_buffer(index, shared_memory::access_mode::NONE, buffer) == false)
				return false;

			return true;
		}

		bool query_buffer_shared(uint32_t index, shm_sharable_buffer_interface** buffer)
		{
			if (buffer == nullptr)
				return false;

			if (is_mapped() == false)
				return false;

			shm_shared_buffer_mutex* shared_mutex = get_buffer_mutex(index);
			if (shared_mutex == nullptr ||
				shared_mutex->lock_read() == false ||
				query_sharable_buffer(index, shared_memory::access_mode::READER, buffer) == false)
				return false;

			return true;
		}

		bool query_buffer_unique(shm_sharable_buffer_interface** buffer)
		{
			if (buffer == nullptr)
				return false;

			if (is_mapped() == false)
				return false;

			for (uint32_t i = 0; i < m_pool_size; i++)
			{
				uint32_t index = ((m_next_unique_query + i) % m_pool_size);
				shm_shared_buffer_mutex* shared_mutex = get_buffer_mutex(index);
				if (shared_mutex == nullptr ||
					shared_mutex->lock_write() == false ||
					query_sharable_buffer(index, shared_memory::access_mode::WRITER, buffer) == false)
					continue;

				m_next_unique_query = ((index + 1) % m_pool_size);
				return true;
			}

			return false;
		}

		error_codes wait_for_buffer_shared(int64_t timeout, shm_sharable_buffer_interface** buffer)
		{
			if (buffer == nullptr)
				return error_codes::SHM_INVALID_ARGUMENT;

			if (is_mapped() == false)
				return error_codes::SHM_NOT_MAPPED;

			shm_bufferpool_wait_handler* wait_handle = get_wait_handler();
			if (wait_handle == nullptr)
				return error_codes::SHM_DATA_CORRUPTION;

			uint32_t index;
			error_codes error = wait_handle->wait(timeout, index);
			if (error != error_codes::SHM_NO_ERROR)
				return error;

			if (query_buffer_shared(index, buffer) == false)
				return error_codes::SHM_LOCK_SHARED_FAILED;

			return error_codes::SHM_NO_ERROR;
		}
	};

	struct shm_pool_params
	{
		uint32_t index;
		uint32_t pool_size;
		uint32_t buffer_size;
	};

	class shm_session :
		public utils::ref_count_base<core::ref_count_interface>
	{
	private:
		class shm_session_buffer :
			public shm_buffer
		{
		private:
			static constexpr size_t SIZE = sizeof(core::guid) + sizeof(uint32_t) + (sizeof(core::guid) * MAX_SHARABLE_POOLS_PER_SESSION);

			shared_memory::guid_generator m_generator;

			bool write_segment_identifier()
			{
				if (mode() != shared_memory::access_mode::WRITER)
					return false;

				uint8_t* base_addr = this->data();
				std::memcpy(base_addr, &SESSION_SEGMENT_IDENTIFIER, sizeof(core::guid));

				return true;
			}

			bool write_max_pool_count()
			{
				if (mode() != shared_memory::access_mode::WRITER)
					return false;

				uint8_t* base_addr = this->data() + sizeof(core::guid);
				std::memcpy(base_addr, &MAX_SHARABLE_POOLS_PER_SESSION, sizeof(uint32_t));

				return true;
			}

			bool write_pool_id(uint32_t index, const core::guid& id)
			{
				if (index >= MAX_SHARABLE_POOLS_PER_SESSION)
					return false;

				if (is_mapped() == false)
					return false;

				size_t offset = sizeof(core::guid) + sizeof(uint32_t) + (sizeof(core::guid) * index);
				if (size() < offset)
					return false;

				uint8_t* base_addr = this->data() + offset;
				std::memcpy(base_addr, &id, sizeof(core::guid));
				return true;
			}

			core::guid read_segment_identifier()
			{
				core::guid retval;
				std::memcpy(&retval, data(), sizeof(core::guid));
				return retval;
			}

		public:
			struct writer_t {};
			static constexpr writer_t as_writer() { return writer_t(); }

			struct reader_t {};
			static constexpr reader_t as_reader() { return reader_t(); }

			// C'tor for writers
			shm_session_buffer(const writer_t&, const char* name) :
				shm_buffer(name, SIZE)
			{
				if (this->size() < SIZE)
					throw std::runtime_error("Session buffer allocation failed");

				// Initialize the shared memory structure if not already initialized
				if (read_segment_identifier() != SESSION_SEGMENT_IDENTIFIER)
				{
					write_segment_identifier();
					write_max_pool_count();

					core::guid id = core::guid::undefined();
					for (uint32_t i = 0; i < MAX_SHARABLE_POOLS_PER_SESSION; i++)
						write_pool_id(i, id);
				}
			}

			shm_session_buffer(const reader_t&, const char* name) :
				shm_buffer(name)
			{
			}

			bool query_pool_id(uint32_t index, core::guid& id)
			{
				if (index >= MAX_SHARABLE_POOLS_PER_SESSION)
					return false;

				if (is_mapped() == false)
					return false;

				size_t offset = sizeof(core::guid) + sizeof(uint32_t) + (sizeof(core::guid) * index);
				if (size() < offset)
					return false;

				uint8_t* base_addr = this->data() + offset;
				std::memcpy(&id, base_addr, sizeof(core::guid));
				if (id == core::guid::undefined())
					return false;

				return true;
			}

			bool set_pool_id(uint32_t index, core::guid& id)
			{
				id = m_generator.generate();
				return write_pool_id(index, id);
			}

			bool clear_pool_id(uint32_t index)
			{
				core::guid id = core::guid::undefined();
				return write_pool_id(index, id);
			}

			uint32_t pool_count()
			{
				if (is_mapped() == false)
					return 0;

				uint32_t retval = 0;

				core::guid id;
				for (uint32_t i = 0; i < MAX_SHARABLE_POOLS_PER_SESSION; i++)
				{
					if (query_pool_id(i, id) == false)
						continue;

					++retval;
				}

				return retval;
			}
		};

		std::string m_name;
		std::mutex m_mutex;

		shared_memory::access_mode m_mode;
		utils::ref_count_ptr<shm_session_buffer> m_session_buffer;
		std::vector<shm_pool_params> m_pool_parmas;
		utils::ref_count_ptr<shm_sharable_bufferpool> m_pools[MAX_SHARABLE_POOLS_PER_SESSION];

		void unmap()
		{
			if (m_mode != shared_memory::access_mode::WRITER)
				return;

			std::lock_guard<std::mutex> locker(m_mutex);

			for (auto& curr_pool_params : m_pool_parmas)
			{
				m_pools[curr_pool_params.index].release();
				m_session_buffer->clear_pool_id(curr_pool_params.index);
			}
		}

		bool query_pool(uint32_t index, shm_sharable_bufferpool** pool)
		{
			if (index >= MAX_SHARABLE_POOLS_PER_SESSION)
				return false;

			if (pool == nullptr)
				return false;

			std::lock_guard<std::mutex> locker(m_mutex);

			if (m_session_buffer == nullptr || m_session_buffer->is_mapped() == false)
				return false;

			if (m_pools[index] == nullptr)
				return false;

			*pool = m_pools[index];
			(*pool)->add_ref();
			return true;
		}

	public:
		shm_session(const char* name, shm_pool_params* pool_params, uint32_t pool_params_count) :
			m_name(name),
			m_mode(shared_memory::access_mode::WRITER)
		{
			if (name == nullptr)
				throw std::invalid_argument("name");

			if (pool_params == nullptr)
				throw std::invalid_argument("pool_params");

			if (pool_params_count == 0)
				throw std::invalid_argument("pool_params_count");

			for (size_t i = 0; i < pool_params_count; i++)
			{
				if (pool_params->index >= MAX_SHARABLE_POOLS_PER_SESSION ||
					pool_params[i].pool_size == 0 ||
					pool_params[i].buffer_size == 0)
					continue;

				m_pool_parmas.emplace_back(pool_params[i]);
			}

			remap();
		}

		shm_session(const char* name) :
			m_name(name),
			m_mode(shared_memory::access_mode::READER)
		{
			if (name == nullptr)
				throw std::invalid_argument("name");

			remap();
		}

		~shm_session()
		{
			unmap();
		}

		bool remap(uint32_t pool_index)
		{
			if (pool_index >= MAX_SHARABLE_POOLS_PER_SESSION)
				return false;

			std::lock_guard<std::mutex> locker(m_mutex);

			if (m_session_buffer == nullptr)
				return false;

			bool retval = false;
			for (auto& curr_pool_params : m_pool_parmas)
			{
				if (curr_pool_params.index != pool_index)
					continue;

				m_pools[curr_pool_params.index].release();

				core::guid id;
				if (m_session_buffer->set_pool_id(curr_pool_params.index, id) == false)
					break;

				std::stringstream stream;
				stream << id;

				utils::ref_count_ptr<shm_sharable_bufferpool> pool =
					utils::make_ref_count_ptr<shm_sharable_bufferpool>(stream.str().c_str(), curr_pool_params.pool_size, curr_pool_params.buffer_size);

				m_pools[curr_pool_params.index] = pool;
				retval = true;
			}

			return retval;

		}

		bool remap()
		{
			std::lock_guard<std::mutex> locker(m_mutex);

			m_session_buffer.release();
			utils::ref_count_ptr<shm_session_buffer> session_buffer;

			if (m_mode == shared_memory::access_mode::WRITER)
			{
				session_buffer = utils::make_ref_count_ptr<shm_session_buffer>(shm_session_buffer::as_writer(), m_name.c_str());
				if (session_buffer->is_mapped() == false)
					return false;

				for (auto& curr_pool_params : m_pool_parmas)
				{
					m_pools[curr_pool_params.index].release();

					core::guid id;
					if (session_buffer->set_pool_id(curr_pool_params.index, id) == false)
						continue;

					std::stringstream stream;
					stream << id;

					utils::ref_count_ptr<shm_sharable_bufferpool> pool =
						utils::make_ref_count_ptr<shm_sharable_bufferpool>(stream.str().c_str(), curr_pool_params.pool_size, curr_pool_params.buffer_size);

					m_pools[curr_pool_params.index] = pool;
				}
			}
			else if (m_mode == shared_memory::access_mode::READER)
			{
				session_buffer = utils::make_ref_count_ptr<shm_session_buffer>(shm_session_buffer::as_reader(), m_name.c_str());
				if (session_buffer->is_mapped() == false)
					return false;

				for (uint32_t i = 0; i < MAX_SHARABLE_POOLS_PER_SESSION; i++)
				{
					m_pools[i].release();

					core::guid id;
					if (session_buffer->query_pool_id(i, id) == false)
						continue;

					std::stringstream stream;
					stream << id;

					utils::ref_count_ptr<shm_sharable_bufferpool> pool =
						utils::make_ref_count_ptr<shm_sharable_bufferpool>(stream.str().c_str());

					m_pools[i] = pool;
				}
			}

			m_session_buffer = session_buffer;
			return true;
		}

		error_codes query_buffer_unlocked(uint32_t pool_index, uint32_t buffer_index, shm_sharable_buffer_interface** buffer)
		{
			if (buffer == nullptr)
				return error_codes::SHM_INVALID_ARGUMENT;

			if (m_mode != shared_memory::access_mode::WRITER)
				return error_codes::SHM_PERMISSION_DENIED;

			utils::ref_count_ptr<shm_sharable_bufferpool> pool;
			if (query_pool(pool_index, &pool) == false)
				return error_codes::SHM_POOL_DOES_NOT_EXIST;

			if (pool->query_buffer_unlocked(buffer_index, buffer) == false)
				return error_codes::SHM_LOCK_SHARED_FAILED;

			return error_codes::SHM_NO_ERROR;
		}

		error_codes query_buffer_unique(uint32_t pool_index, shm_sharable_buffer_interface** buffer)
		{
			if (buffer == nullptr)
				return error_codes::SHM_INVALID_ARGUMENT;

			if (m_mode != shared_memory::access_mode::WRITER)
				return error_codes::SHM_PERMISSION_DENIED;

			utils::ref_count_ptr<shm_sharable_bufferpool> pool;
			if (query_pool(pool_index, &pool) == false)
				return error_codes::SHM_POOL_DOES_NOT_EXIST;

			if (pool->query_buffer_unique(buffer) == false)
				return error_codes::SHM_LOCK_UNIQUE_FAILED;

			return error_codes::SHM_NO_ERROR;
		}

		error_codes query_buffer_shared(uint32_t pool_index, uint32_t buffer_index, core::buffer_interface** buffer)
		{
			if (buffer == nullptr)
				return error_codes::SHM_INVALID_ARGUMENT;

			utils::ref_count_ptr<shm_sharable_bufferpool> pool;
			if (query_pool(pool_index, &pool) == false)
				return error_codes::SHM_POOL_DOES_NOT_EXIST;

			utils::ref_count_ptr<shm_sharable_buffer_interface> instance;
			if (pool->query_buffer_shared(buffer_index, &instance) == false)
				return error_codes::SHM_LOCK_SHARED_FAILED;

			*buffer = instance;
			(*buffer)->add_ref();
			return error_codes::SHM_NO_ERROR;
		}

		error_codes wait_for_buffer_share(uint32_t pool_index, int64_t timeout, core::buffer_interface** buffer)
		{
			if (buffer == nullptr)
				return error_codes::SHM_INVALID_ARGUMENT;

			utils::ref_count_ptr<shm_sharable_bufferpool> pool;
			if (query_pool(pool_index, &pool) == false)
				return error_codes::SHM_POOL_DOES_NOT_EXIST;

			utils::ref_count_ptr<shm_sharable_buffer_interface> instance;
			error_codes error = pool->wait_for_buffer_shared(timeout, &instance);
			if (error != error_codes::SHM_NO_ERROR)
				return error;

			*buffer = instance;
			(*buffer)->add_ref();
			return error_codes::SHM_NO_ERROR;
		}

		error_codes publish(shm_sharable_buffer_interface* buffer)
		{
			if (buffer == nullptr)
				return error_codes::SHM_INVALID_ARGUMENT;

			error_codes retval = buffer->publish();
			if (retval != error_codes::SHM_NO_ERROR)
				remap();

			return retval;
		}
	};

	class shm_session_player :
		public utils::ref_count_base<core::ref_count_interface>
	{
	private:
		utils::ref_count_ptr<shm_session> m_session;
		uint32_t m_pool_index;

		std::mutex m_mutex;
		std::atomic<bool> m_running;
		std::thread m_listening_thread;

	public:
		utils::signal<shm_session_player, uint32_t, core::buffer_interface*> OnBuffer;

		shm_session_player(shared_memory::shm_session* session, uint32_t pool_index) :
			m_session(session),
			m_pool_index(pool_index),
			m_running(false)

		{
			if (session == nullptr)
				throw std::invalid_argument("session");

			if (m_pool_index >= MAX_SHARABLE_POOLS_PER_SESSION)
				throw std::out_of_range("pool_index");
		}

		void start()
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			if (m_running == true)
				return;

			m_running = true;
			m_listening_thread = std::thread([this]()
			{
				while (m_running == true)
				{
					utils::ref_count_ptr<core::buffer_interface> buffer;

					error_codes error = error_codes::SHM_TIMEOUT;
					for (int64_t i = 0; i < ITERATIONS; i++)
					{
						error = m_session->wait_for_buffer_share(m_pool_index, MAX_BLOCKING_TIME_MS, &buffer);
						if (m_running == false ||
							error == error_codes::SHM_NO_ERROR ||
							error != error_codes::SHM_TIMEOUT)
							break;
					}

					if (m_running == false)
						break;

					if (error != error_codes::SHM_NO_ERROR)
					{
						for (int64_t i = 0; i < ITERATIONS; i++)
						{
#if defined(_WIN32) && defined(_MSC_VER)
							Sleep(static_cast<DWORD>(MAX_BLOCKING_TIME_MS));
#else
							std::this_thread::sleep_for(std::chrono::milliseconds(MAX_BLOCKING_TIME_MS));
#endif // _WIN32 && _MSC_VER
							if (m_running == false)
								break;
						}

						if (m_running == false)
							break;

						m_session->remap();
						continue;
					}

					OnBuffer(m_pool_index, buffer);
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
}
