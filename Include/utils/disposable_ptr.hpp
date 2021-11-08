#pragma once
#include <utils/ref_count_ptr.hpp>
#include <utils/disposable_base.hpp>
#include <mutex>

namespace utils
{
	template <class T>
	class disposable_ptr
	{
	private:
		mutable std::mutex m_mutex;
		utils::ref_count_ptr<utils::smart_disposable_callback> m_diposable_callback;
		mutable T* m_ptr;	

		void register_callback()
		{
			if (m_ptr != nullptr)
				m_ptr->register_disposable_callback(m_diposable_callback);
		}

		void clear() const
		{			
			if (m_ptr == nullptr)
				return;

			m_ptr->unregister_disposable_callback(m_diposable_callback);
			m_ptr = nullptr;
		}		

	public:
		disposable_ptr(T* ptr) :
			m_diposable_callback(utils::make_ref_count_ptr<utils::smart_disposable_callback>()),
			m_ptr(ptr)
		{			
			m_diposable_callback->disposed += [this]()
			{
				reset();
			};

			register_callback();
		}

		disposable_ptr(const utils::ref_count_ptr<T>& ptr) :
			disposable_ptr(static_cast<T*>(ptr))
		{
		}

		disposable_ptr() :
			disposable_ptr(nullptr)
		{
		}

		disposable_ptr(const disposable_ptr<T>& other) :
			m_diposable_callback(utils::make_ref_count_ptr<utils::smart_disposable_callback>()),
			m_ptr(nullptr)
		{
			m_diposable_callback->disposed += [this]()
			{
				reset();
			};

			utils::ref_count_ptr<T> strong_ptr;
			if (other.lock(&strong_ptr) == true)
				m_ptr = strong_ptr;

			register_callback();
		}

		disposable_ptr(disposable_ptr<T>&& other) :
			m_diposable_callback(utils::make_ref_count_ptr<utils::smart_disposable_callback>()),
			m_ptr(nullptr)
		{
			m_diposable_callback->disposed += [this]()
			{
				reset();
			};

			utils::ref_count_ptr<T> strong_ptr;
			if (other.lock(&strong_ptr) == true)
				m_ptr = strong_ptr;

			other.reset();
			register_callback();
		}

		disposable_ptr<T>& operator=(const disposable_ptr<T>& other)
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			clear();

			utils::ref_count_ptr<T> strong_ptr;
			if (other.lock(&strong_ptr) == true)
				m_ptr = strong_ptr;

			register_callback();
			return *this;
		}

		disposable_ptr<T>& operator=(disposable_ptr<T>&& other)
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			clear();

			utils::ref_count_ptr<T> strong_ptr;
			if (other.lock(&strong_ptr) == true)
				m_ptr = strong_ptr;

			other.reset();
			register_callback();
			return *this;
		}

		disposable_ptr<T>& operator=(T* ptr)
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			clear();

			m_ptr = ptr;
			register_callback();
			return *this;
		}

		~disposable_ptr()
		{
			reset();
		}

		bool lock(T** ptr) const
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			if (m_ptr == nullptr)
				return false;

			auto count = m_ptr->add_ref();

			// That's a bit tricky...
			// As a result of a race-condition and/or some logic performed on m_ptr's destruction,
			// it might be that m_ptr d'tor was called but it didn't notify us yet (it might be even blocked on our mutex).
			// If the above happend, the ref_count of m_ptr BEFORE we called add_ref was surely 0.
			// That means the current value (count) is exactly 1. Hence, (count < 2)
			// That also means that if we won't check this condition, we would probably return a dangling pointer as m_ptr is currently being destructed.
			if (count < 2)
			{
				// We have to clear in order to handle cases where another thread is trying to lock
				// and might be scheduled before m_ptr's disposed callback.
				// That might result in (count > 1), hence, returning a dangling pointer.
				clear();
				return false;
			}

			*ptr = m_ptr;
			return true;
		}

		void reset()
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			clear();
		}
	};
}