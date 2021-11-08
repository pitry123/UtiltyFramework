#pragma once
#include <utils/scope_guard.hpp>
#include <mutex>

namespace utils
{	
	template <typename T>
	class thread_safe_object_lock_token;

	template <typename T>
	class lockable_object_interface
	{
        friend class thread_safe_object_lock_token<T>;

	public:
		virtual ~lockable_object_interface() = default;

	private:
		virtual void lock() = 0;
		virtual void unlock() = 0;
		virtual T& object() = 0;
	};

	template <typename T>
	class thread_safe_object_lock_token
	{
	private:
		bool m_flag;
		lockable_object_interface<T>& m_lockable;
		
		thread_safe_object_lock_token(const thread_safe_object_lock_token&) = delete;				// non copyable by assignment
		thread_safe_object_lock_token& operator=(const thread_safe_object_lock_token&) = delete;	// non construction-copyable
		thread_safe_object_lock_token& operator=(thread_safe_object_lock_token&&) = delete;			// non movable by assignment

	public:
		void unlock()
		{
			if (m_flag == true)
			{
				utils::scope_guard flag_updater([this]()
				{
					m_flag = false;
				});

				m_lockable.unlock();
			}				
		}

		T& safe_object()
		{
			if (m_flag == false)
				throw std::runtime_error("Token is unlocked!");

			return m_lockable.object();
		}

		thread_safe_object_lock_token(lockable_object_interface<T>& lockable) :
			m_flag(true),
			m_lockable(lockable)
		{
			m_lockable.lock();
		}

		thread_safe_object_lock_token(thread_safe_object_lock_token&& other) :
			m_flag(other.m_flag),
            m_lockable(other.m_lockable)
		{
			other.m_flag = false;
		}

		~thread_safe_object_lock_token()
		{
			unlock();
		}			
	};

	template <typename T>
	class thread_safe_object :
		public lockable_object_interface<T>
	{
	private:
		mutable std::mutex m_mutex;
		T m_object;

		virtual void lock() override
		{
			m_mutex.lock();
		}

		virtual void unlock() override
		{
			m_mutex.unlock();
		}

		virtual T& object() override
		{
			return m_object;
		}

	public:
		explicit thread_safe_object()
		{
		}

		explicit thread_safe_object(T&& obj) :
			m_object(std::move(obj))
		{
		}

		explicit thread_safe_object(thread_safe_object&& other) :
			m_object(std::move(other.m_object))
		{			
		}

		thread_safe_object &operator=(thread_safe_object&& other)
		{
			auto token = other.get_lock_token();
			m_object = std::move(other.m_object);
		}
		
		template <typename CALLABLE>
		void use(const CALLABLE& func)
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			func(m_object);
		}

		template <typename RESULT, typename CALLABLE>
		RESULT use(const CALLABLE& func)
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			return func(m_object);
		}

		template <typename CALLABLE>
		void use(const CALLABLE& func) const
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			func(m_object);
		}

		template <typename RESULT, typename CALLABLE>
		RESULT use(const CALLABLE& func) const
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			return func(m_object);
		}

		thread_safe_object_lock_token<T> get_lock_token()
		{
			return std::move(thread_safe_object_lock_token<T>(*this));
		}

	private:
		//Non copyable
		thread_safe_object(const thread_safe_object &) = delete;
		thread_safe_object &operator=(const thread_safe_object &) = delete;
	};
}
