#pragma once
#include <core/ref_count_interface.h>
#include <utils/ref_count_base.hpp>
#include <stdexcept>
#include <functional>

namespace utils
{
	template<typename Res, typename... Args>
	class func_wrapper_base :
		public utils::ref_count_base<core::ref_count_interface>
	{
	private:
		std::function<Res(Args...)> m_func;

	public:
		func_wrapper_base(const std::function<Res(Args...)>& func) :
			m_func(func)
		{
			if (m_func == nullptr)
				throw std::invalid_argument("func");
		}

		Res invoke(Args... args)
		{
			return m_func(args...);
		}

		Res operator()(Args... args)
		{
			return invoke(std::forward<Args>(args)...);
		}
	};

	template<typename... Args>
	class func_wrapper :
		public utils::func_wrapper_base<void, Args...>
	{
	public:
		func_wrapper(const std::function<void(Args...)>& func) :
			utils::func_wrapper_base<void, Args...>(func)
		{
		}
	};
}