#pragma once
#include <stdexcept>
#include <cassert>

namespace utils
{
	template<class T>
	class ref_count_ptr
	{
	public:
		ref_count_ptr() : m_ptr(nullptr)
		{
		}

		ref_count_ptr(const ref_count_ptr<T>& other)
		{
			m_ptr = (T*)other;

			if (m_ptr)
			{
				m_ptr->add_ref();
			}
		}

		ref_count_ptr(ref_count_ptr<T>&& other)
		{
			m_ptr = (T*)other;

			if (m_ptr)
			{
				m_ptr->add_ref();
			}

			other.release();
		}

		template <typename CASTABLE>
		ref_count_ptr(ref_count_ptr<CASTABLE>& other)
		{
			m_ptr = static_cast<T*>(other);

			if (m_ptr)
			{
				m_ptr->add_ref();
			}
		}

		template <typename CASTABLE>
		ref_count_ptr(ref_count_ptr<CASTABLE>&& other)
		{
			m_ptr = static_cast<T*>(other);

			if (m_ptr)
			{
				m_ptr->add_ref();
			}

			other.release();
		}

		ref_count_ptr(T* other) : m_ptr(nullptr)
		{
			if (other != nullptr)
			{
				m_ptr = other;
				m_ptr->add_ref();
			}
		}

		~ref_count_ptr()
		{
			if (m_ptr)
			{
				m_ptr->release();
				m_ptr = nullptr;
			}
		}

		operator T*() const
		{
			return m_ptr;
		}

		T& operator*() const
		{
			assert(m_ptr != nullptr);
			return *m_ptr;
		}

		T** operator&()
		{
			assert(m_ptr == nullptr);
			return &m_ptr;
		}

		T* operator->() const
		{
			assert(m_ptr != nullptr);
			return m_ptr;
		}

		T* operator=(T* other)
		{
			if (m_ptr != nullptr && other != nullptr &&
                ((void*)m_ptr) == ((void*)(other)))
			{
				return m_ptr;
			}

			release();

			if (other != nullptr)
			{
				other->add_ref();
				m_ptr = other;
			}

			return m_ptr;
		}

		T* operator=(const ref_count_ptr<T>& other)
		{
			T* other_ptr = (T*)other;
			if (m_ptr == other_ptr)
			{
				return m_ptr;
			}

			release();
			m_ptr = other_ptr;
			if (m_ptr != nullptr)
			{
				m_ptr->add_ref();
			}

			return m_ptr;
		}

		template <typename CASTABLE>
		T* operator=(const ref_count_ptr<CASTABLE>& other)
		{
			T* other_ptr = static_cast<T*>(other);
			if (m_ptr == other_ptr)
			{
				return m_ptr;
			}

			release();
			m_ptr = other_ptr;
			if (m_ptr != nullptr)
			{
				m_ptr->add_ref();
			}

			return m_ptr;
		}

		T* operator=(ref_count_ptr<T>&& other)
		{
			release();

			m_ptr = (T*)other;
			if (m_ptr != nullptr)
			{
				m_ptr->add_ref();
			}

			other.release();
			return m_ptr;
		}

		template <typename CASTABLE>
		T* operator=(ref_count_ptr<CASTABLE>&& other)
		{
			release();

			m_ptr = static_cast<T*>(other);
			if (m_ptr != nullptr)
			{
				m_ptr->add_ref();
			}

			other.release();
			return m_ptr;
		}		

		void attach(T* other)
		{
			if (m_ptr != nullptr)
			{
				int ref_count = m_ptr->release();

				if (ref_count == 0)
					assert(m_ptr != other);
			}

			m_ptr = other;
		}

		T* detach()
		{
			T* ptr = m_ptr;
			m_ptr = nullptr;

			return ptr;
		}

		void release()
		{
			if (m_ptr != nullptr)
			{
				m_ptr->release();
				m_ptr = nullptr;
			}
		}

		static ref_count_ptr<T> make_attached(T* other)
		{
			ref_count_ptr<T> retval;
			retval.attach(other);

			return retval;
		}

	private:
		T* m_ptr;
	};

	template <typename T>
	struct ref_count_ptr_hash
	{
		size_t operator()(const utils::ref_count_ptr<T>& ptr) const
		{
			return (std::hash<T*>()(ptr));
		}
	};

	template<typename T, typename... Args>
	inline utils::ref_count_ptr<T> make_ref_count_ptr(Args&&... args)
	{
		return utils::ref_count_ptr<T>::make_attached(new T(std::forward<Args>(args)...));
	}

	template <typename CASTABLE, typename T>
	inline CASTABLE* ref_count_ptr_cast(const utils::ref_count_ptr<T>& ptr)
	{
		return static_cast<CASTABLE*>(static_cast<T*>(ptr));
	}

	template <typename CASTABLE, typename T>
	inline CASTABLE* ref_count_ptr_cast(T* ptr)
	{
		return static_cast<CASTABLE*>(ptr);
	}
}
