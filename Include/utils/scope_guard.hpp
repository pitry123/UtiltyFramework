#pragma once
#include <functional>

namespace utils
{
	class scope_guard
	{
	public:
		explicit scope_guard(const std::function<void(void)>& func)
			: m_func(func)
		{
		}

		explicit scope_guard(std::function<void(void)>&& func)
			: m_func(std::move(func))
		{
		}

		explicit scope_guard(scope_guard&& other) : 
			m_func(std::move(other.m_func))
		{			
		}

		~scope_guard()
		{
			if (m_func != nullptr)
			{
				m_func();
			}
		}		

	private:
		//Non copyable
		scope_guard(const scope_guard &) = delete;
		scope_guard &operator=(const scope_guard &) = delete;		

		std::function<void(void)> m_func;
	};
}