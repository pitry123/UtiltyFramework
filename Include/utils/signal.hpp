#pragma once
#include <utils/func_wrapper.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/scope_guard.hpp>

#include <mutex>
#include <vector>
#include <map>

#define MAX_STACK_SUBSCRIBERS_COUNT 32

namespace utils
{
	using signal_token = int;
	static constexpr signal_token signal_token_undefined = -1;

	template<typename HostingClass, typename... Args>
	class signal
	{
		friend HostingClass;

	public:
		signal()
		{
		}

		signal(signal&& other)
		{
			std::lock_guard<std::mutex> locker(other.m_mutex);
			m_subscribers = std::move(other.m_subscribers);

			other.m_subscribers.clear();
		}

		signal& operator=(signal&& other)
		{
			std::lock_guard<std::mutex> locker(other.m_mutex);
			m_subscribers = std::move(other.m_subscribers);

			other.m_subscribers.clear();
			return *this;
		}

		virtual ~signal()
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			m_subscribers.clear();
		}		
		
		virtual size_t count()
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			return m_subscribers.size();
		}

		virtual signal_token subscribe(const std::function<void(Args...)>& func, size_t& callback_count)
		{
			std::lock_guard<std::mutex> locker(m_mutex);

			utils::scope_guard count_update([&]()
			{
				callback_count = m_subscribers.size();
			});

			signal_token token = signal_token_undefined;
			for (signal_token i = 0; i < (std::numeric_limits<signal_token>::max)(); i++)
			{
				if (m_subscribers.find(i) == m_subscribers.end())
				{
					token = i;
					break;
				}
			}

			if (token != signal_token_undefined)
				m_subscribers.emplace(token, utils::make_ref_count_ptr<func_wrapper<Args...>>(func));

			return token;
		}

		virtual signal_token subscribe(const std::function<void(Args...)>& func)
		{
			size_t dummy;
			return subscribe(func, dummy);
		}

		virtual bool unsubscribe(signal_token token, size_t& callback_count)
		{
			std::lock_guard<std::mutex> locker(m_mutex);

			utils::scope_guard count_update([&]()
			{
				callback_count = m_subscribers.size();
			});

			typename std::map<signal_token, utils::ref_count_ptr<func_wrapper<Args...>>>::iterator it = m_subscribers.find(token);
			if (it == m_subscribers.end())
				return false;
			
			m_subscribers.erase(token);
			return true;
		}

		virtual bool unsubscribe(signal_token token)
		{
			size_t dummy;
			return unsubscribe(token, dummy);
		}

		signal_token operator+=(const std::function<void(Args...)>& func)
		{
			return subscribe(func);
		}

		bool operator-=(signal_token token)
		{
			return unsubscribe(token);
		}
	
    protected:
		virtual bool raise(Args... args)
		{
			bool retVal = false;

			std::unique_lock<std::mutex> locker(m_mutex);

			size_t subscribers_count = m_subscribers.size();
			if (subscribers_count > 0)
			{
				if (subscribers_count <= MAX_STACK_SUBSCRIBERS_COUNT)
				{
					utils::ref_count_ptr<func_wrapper<Args...>> cloned_functions[MAX_STACK_SUBSCRIBERS_COUNT];

					typename std::map<signal_token, utils::ref_count_ptr<func_wrapper<Args...>>>::iterator it = m_subscribers.begin();

					size_t index = 0;
					while (it != m_subscribers.end())
					{
						cloned_functions[index++] = it->second;
						++it;
					}					

					locker.unlock();

					for (size_t i = 0; i < subscribers_count; i++)
					{
						cloned_functions[i]->invoke(std::forward<Args>(args)...);
					}
				}
				else
				{
					std::vector<utils::ref_count_ptr<func_wrapper<Args...>>> cloned_functions(subscribers_count);

					typename std::map<signal_token, utils::ref_count_ptr<func_wrapper<Args...>>>::iterator it = m_subscribers.begin();

					size_t index = 0;
					while (it != m_subscribers.end())
					{
						cloned_functions[index++] = it->second;
						++it;
					}

					locker.unlock();

					for (size_t i = 0; i < subscribers_count; i++)
					{
						cloned_functions[i]->invoke(std::forward<Args>(args)...);
					}
				}				
			}						

			return retVal;
		}

		bool operator()(Args... args)
		{
			return raise(std::forward<Args>(args)...);
		}

	private:
		signal(const signal& other) = delete;           // non construction-copyable
		signal& operator=(const signal&) = delete;		// non copyable				

		std::mutex m_mutex;
		std::map<signal_token, utils::ref_count_ptr<func_wrapper<Args...>>> m_subscribers;
	};
}
