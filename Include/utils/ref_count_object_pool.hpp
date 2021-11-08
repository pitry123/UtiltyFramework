#pragma once
#include <core/ref_count_interface.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <new>
#include <vector>
#include <mutex>
#include <functional>

namespace utils
{
	template <class T>
	class ref_count_object_pool : public utils::ref_count_base<core::ref_count_interface>
	{
	public:
		enum class growing_mode
		{
			none,
			doubling
		};

	private:
		mutable std::mutex m_mutex;
		mutable std::vector<utils::ref_count_ptr<T>> m_items;
		growing_mode m_mode;
		bool m_lazy;
		std::function<bool(T**)> m_create_object;
		mutable size_t m_next_search_index;

		//Non copyable
		ref_count_object_pool(const ref_count_object_pool&) = delete;
		ref_count_object_pool& operator=(const ref_count_object_pool&) = delete;

		bool query_item(T** item) const
		{
			for (size_t i = 0; i < m_items.size(); i++)
			{
				size_t index = m_lazy == true ? i : ((m_next_search_index + i) % m_items.size());
				if (m_items[index] == nullptr)
					m_create_object(&(m_items[index]));

				if (m_items[index]->ref_count() == 1)
				{
					m_next_search_index = ((index + 1) % m_items.size());

					(*item) = m_items[index];
					(*item)->add_ref();
					return true;
				}
			}

			return false;
		}

	public:
		template <typename... Args>
		ref_count_object_pool(size_t size, growing_mode mode, bool lazy, Args... args) :
			m_mode(mode),
			m_lazy(lazy),
			m_next_search_index(0)
		{
			m_items.resize(size);
			m_create_object = [args...](T** obj)
			{
				if (obj == nullptr)
					return false;

				utils::ref_count_ptr<T> instance;

				try
				{
					instance = utils::make_ref_count_ptr<T>(args...);
				}
				catch (...)
				{
					instance.release();
					return false;
				}

				if (instance == nullptr)
					return false;

				*obj = instance;
				(*obj)->add_ref();
				return true;
			};

			if (m_lazy == false)
			{
				for (size_t i = 0; i < size; i++)
				{
					if (m_create_object(&(m_items[i])) == false)
						break;
				}
			}
		}

		ref_count_object_pool(size_t size, growing_mode mode, const std::function<bool(T**)>& create_object_func, bool lazy = true) :
			m_mode(mode),
			m_lazy(lazy),
			m_create_object(create_object_func),
			m_next_search_index(0)
		{
			m_items.resize(size);

			if (m_lazy == false)
			{
				for (size_t i = 0; i < size; i++)
				{
					if (m_create_object(&(m_items[i])) == false)
						break;
				}
			}
		}

		ref_count_object_pool(ref_count_object_pool&& other)
		{
			std::lock_guard<std::mutex> locker(other.m_mutex);
			m_items.swap(other.m_items);
			m_create_object = std::move(other.m_create_object);
		}

		ref_count_object_pool& operator=(ref_count_object_pool&& other)
		{
			std::lock_guard<std::mutex> lockre(m_mutex);
			std::lock_guard<std::mutex> other_locker(other.m_mutex);

			m_items.clear();
			m_items.swap(other.m_items);
			m_create_object = std::move(m_create_object);

			return *this;
		}

		virtual bool get_item(T** item) const
		{
			if (item == nullptr)
				return false;

			std::lock_guard<std::mutex> locker(m_mutex);

			if (query_item(item) == true)
				return true;

			if (m_mode == growing_mode::none)
				return false;

			if (m_mode == growing_mode::doubling)
			{
				m_items.resize(m_items.size() * 2);
				if (query_item(item) == false)
					throw std::runtime_error("Failed to dynamically extend pool size. growing_mode is: doubling");

				return true;
			}

			throw std::runtime_error("Unexpected growing_mode enumeration value");
		}
	};
}