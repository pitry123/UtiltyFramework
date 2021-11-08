#pragma once
#include <cassert>
#include <memory>


namespace utils
{
	template <typename T>
	struct capture_variable_move_wrapper
	{
		capture_variable_move_wrapper() = delete;

		capture_variable_move_wrapper(T && x) : x{ std::move(x) } 
		{
		}

		capture_variable_move_wrapper(capture_variable_move_wrapper& other)
			: x{ std::move(other.x) }, 
			isCopied{ true }
		{
			assert(other.isCopied == false);
		}

		capture_variable_move_wrapper(capture_variable_move_wrapper&& other)
			: x{ std::move(other.x) },
			isCopied{ std::move(other.isCopied) }
		{
		}

		capture_variable_move_wrapper& operator=(capture_variable_move_wrapper other) = delete;

		T&& move()
		{
			return std::move(x);
		}

	private:
		T x;
		bool isCopied = false;
	};

	template<typename T> capture_variable_move_wrapper<T> make_move_wrapper(T&& x)
	{
		return capture_variable_move_wrapper<T>{ std::move(x) };
	}
}