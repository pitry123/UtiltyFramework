#pragma once
#include <utils/ref_count_ptr.hpp>
#include <utils/thread_safe_object.hpp>
#include <vector>
#include <algorithm>
#include <mutex>

#define MAX_STACK_CALLBACK_COUNT 32

namespace utils
{
	template<typename T_CALLBACK>
	class callback_handler
	{
	private:
		mutable std::mutex m_raise_mutex;
		bool m_sync_raise_on_destruction;

		using callbacks_vector = std::vector<utils::ref_count_ptr<T_CALLBACK>>;
		mutable utils::thread_safe_object<callbacks_vector> m_callbacks;

		callback_handler(const callback_handler& other) = delete;		// non construction-copyable
		callback_handler& operator=(const callback_handler&) = delete;	// non copyable

		callback_handler(callback_handler&& other) = delete;			// non construction-movable
		callback_handler& operator=(callback_handler&&) = delete;		// non movable

	public:
		callback_handler(bool sync_raise_on_destruction = true) :
			m_sync_raise_on_destruction(sync_raise_on_destruction)
		{
		}

		virtual ~callback_handler()
		{
			std::unique_lock<std::mutex> locker = m_sync_raise_on_destruction == true ?
				std::unique_lock<std::mutex>(m_raise_mutex) :
				std::unique_lock<std::mutex>(m_raise_mutex, std::defer_lock);

			clear();
		}

		template <typename CALLABLE>
		void raise_callbacks(const CALLABLE& callable) const
		{
			std::unique_lock<std::mutex> locker = m_sync_raise_on_destruction == true ?
				std::unique_lock<std::mutex>(m_raise_mutex) :
				std::unique_lock<std::mutex>(m_raise_mutex, std::defer_lock);

			// Note that we first try to clone the callbacks on stack in order to avoid dynamic allocation
			// If callbacks count exceeds MAX_STACK_CALLBACK_COUNT we fall back to heap allocation
			// Cloning is done in oder to allow adding/removal of callbacks from within the callback function.

			auto lock_token = m_callbacks.get_lock_token();

			size_t callback_count = lock_token.safe_object().size();
			if (callback_count <= MAX_STACK_CALLBACK_COUNT)
			{
				utils::ref_count_ptr<T_CALLBACK> cloned_callbacks[MAX_STACK_CALLBACK_COUNT];

				for (size_t i = 0; i < callback_count; i++)
				{
					cloned_callbacks[i] = lock_token.safe_object()[i];
				}

				lock_token.unlock();

				for (size_t i = 0; i < callback_count; i++)
				{
					callable(cloned_callbacks[i]);
				}
			}
			else
			{
				callbacks_vector cloned_callbacks(callback_count);

				for (size_t i = 0; i < callback_count; i++)
				{
					cloned_callbacks[i] = lock_token.safe_object()[i];
				}

				lock_token.unlock();

				for (size_t i = 0; i < callback_count; i++)
				{
					callable(cloned_callbacks[i]);
				}
			}
		}

		bool add_callback(T_CALLBACK* callback)
		{
			return m_callbacks.template use<bool>([&](callbacks_vector& callbacks)
			{
				auto it = std::find_if(callbacks.begin(), callbacks.end(),
					[callback](const utils::ref_count_ptr<T_CALLBACK>& current_callback) -> bool
				{
					return (current_callback == callback);
				});

				if (it != callbacks.end())
					return false;

				callbacks.emplace_back(callback);
				return true;
			});
		}

		bool remove_callback(T_CALLBACK* callback)
		{
			return m_callbacks.template use<bool>([&](callbacks_vector& callbacks)
			{
				auto it = std::find_if(callbacks.begin(), callbacks.end(),
					[callback](const utils::ref_count_ptr<T_CALLBACK>& current_callback) -> bool
				{
					return (current_callback == callback);
				});

				if (it == callbacks.end())
					return false;

				callbacks.erase(it);
				return true;
			});
		}

		void clear()
		{
			m_callbacks.use([&](callbacks_vector& callbacks)
			{
				callbacks.clear();
			});
		}
	};
}