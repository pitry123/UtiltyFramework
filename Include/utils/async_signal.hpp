#pragma once
#include <utils/signal.hpp>
#include <utils/dispatcher.hpp>

namespace utils
{
	template<typename HostingClass, typename... Args>
	class async_signal : public utils::signal<HostingClass, Args...>
	{
	private:
		utils::dispatcher m_context;

		async_signal(const async_signal& other) = delete;           // non construction-copyable
		async_signal& operator=(const async_signal&) = delete;	  // non copyable

	protected:
		virtual bool raise(Args... args) override
		{
			m_context.begin_invoke([=]()
			{
				utils::signal<HostingClass, Args...>::raise(std::forward<Args>(args)...);
			});

			return true;
		}

	public:
		async_signal()
		{
		}

		async_signal(async_signal&& other)
		{
			utils::signal<HostingClass, Args...>::operator=(std::move(other));
			m_context = std::move(other.m_context);
		}

		async_signal& operator=(async_signal&& other)
		{
			utils::signal<HostingClass, Args...>::operator=(std::move(other));
			m_context = std::move(other.m_context);

			return *this;
		}

		virtual signal_token subscribe(const std::function<void(Args...)>& func) override
		{
			return m_context.invoke<signal_token>([=]()
			{
				return utils::signal<HostingClass, Args...>::subscribe(func);
			});
		}

		virtual bool unsubscribe(signal_token token) override
		{
			return m_context.invoke<bool>([=]()
			{
				return utils::signal<HostingClass, Args...>::unsubscribe(token);
			});
		}
	};
}
