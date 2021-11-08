#pragma once
#include <utils/ref_count_ptr.hpp>
#include <utils/disposable_ptr.hpp>
#include <utils/signal.hpp>
#include <utils/dispatcher.hpp>
#include <utils/timer.hpp>

#include <Common.hpp>

namespace Utils
{
	using SignalToken = utils::signal_token;
	static constexpr SignalToken SignalTokenUndefined = utils::signal_token_undefined;

	using AsyncState = core::async_state;
	using TimerToken = utils::timer_token;
	static constexpr TimerToken TimerTokenUndefined = utils::timer_token_undefined;

	using TimerRegistrationParams = utils::timer_registration_params;

	template <class T>
	class WeakObject
	{
	private:
		utils::disposable_ptr<typename T::UnderlyingObjectType> m_weak_ptr;

	public:
		WeakObject()
		{
		}

		WeakObject(const T& obj) :
            m_weak_ptr(static_cast<typename T::UnderlyingObjectType*>(obj))
		{
		}

		T Lock()
		{
            utils::ref_count_ptr<typename T::UnderlyingObjectType> strong_ptr;
			if (m_weak_ptr.lock(&strong_ptr) == false)
				return T();

			return T(strong_ptr);
		}
	};

	template<typename Func, typename Host, typename... Args>
	class SignalAdapter
	{
	private:
		utils::signal<Host, Args...>& m_signal;

	public:
		SignalAdapter(utils::signal<Host, Args...>& signal) :
			m_signal(signal)
		{
		}

		SignalToken operator+=(const Func& func)
		{
			return m_signal += [func](Args... args)
			{
				func(std::forward<Args>(args)...);
			};
		}

		bool operator-=(SignalToken token)
		{
			return m_signal -= token;
		}
	};

	template <typename T>
	class BaseAsyncAction : public Common::CoreObjectWrapper<T>
	{
	public:
		BaseAsyncAction()
		{
		}

		BaseAsyncAction(T* action) :
			Common::CoreObjectWrapper<T>(action)
		{
		}

		AsyncState State()
		{
            this->ThrowOnEmpty("AsyncAction");
            return this->m_core_object->state();
		}

		bool CompletedSynchronously()
		{
            this->ThrowOnEmpty("AsyncAction");
            return this->m_core_object->completed_synchronously();
		}

		bool Wait(long timeout = 0L)
		{
            this->ThrowOnEmpty("AsyncAction");
            return this->m_core_object->wait(timeout);
		}
	};

	class AsyncAction : public BaseAsyncAction<core::action_interface>
	{
	public:
		AsyncAction()
		{
		}

		AsyncAction(core::action_interface* action) :
			BaseAsyncAction<core::action_interface>(action)
		{
		}		
	};

	template <typename T>
	class AsyncResultAction : public BaseAsyncAction<utils::base_async_result_action<T>>
	{
	public:
		AsyncResultAction()
		{
		}

		AsyncResultAction(utils::base_async_result_action<T>* action) :
			BaseAsyncAction<utils::base_async_result_action<T>>(action)
		{
		}

		const T& Result()
		{
            this->ThrowOnEmpty("AsyncResultAction");
            return this->m_core_object->result();
		}
	};

	class Context : public Common::CoreObjectWrapper<utils::dispatcher>
	{
	public:
		Context(const char* name = nullptr) :
			Common::CoreObjectWrapper<utils::dispatcher>(utils::make_ref_count_ptr<utils::dispatcher>(name))
		{
		}

		Context(std::nullptr_t)
		{
			// Empty Context
		}

		Context(utils::dispatcher* dispatcher) :
			Common::CoreObjectWrapper<utils::dispatcher>(dispatcher)
		{
		}

		Context(const Context& other) : 
			Common::CoreObjectWrapper<utils::dispatcher>(other)
		{
		}					

		void BeginInvoke(const AsyncAction& action, bool forceAsync = false)
		{
			ThrowOnEmpty("Context");

			if (action.Empty() == true)			
				throw std::invalid_argument("action");

			utils::ref_count_ptr<core::action_interface> core_action;
			action.UnderlyingObject(&core_action);

			m_core_object->begin_invoke(core_action, forceAsync);
		}

		void BeginInvoke(const std::function<void()>& func, bool forceAsync = false)
		{
			ThrowOnEmpty("Context");
			m_core_object->begin_invoke(func, nullptr, forceAsync);
		}

		void BeginInvoke(const std::function<void()>& func, AsyncAction& action, bool forceAsync = false)
		{
			ThrowOnEmpty("Context");
			utils::ref_count_ptr<core::action_interface> core_action;
			m_core_object->begin_invoke(func, &core_action, forceAsync);
			action = AsyncAction(core_action);
		}
		
		template <typename T>
		void BeginInvoke(const std::function<T()>& func, const std::function<void(const T&)>& resultCallback, bool forceAsync = false)
		{
			ThrowOnEmpty("Context");
			m_core_object->begin_invoke<T>(func, resultCallback, nullptr, forceAsync);
		}

		template <typename T>
		void BeginInvoke(const std::function<T()>& func, AsyncResultAction<T>& action, bool forceAsync = false)
		{
			ThrowOnEmpty("Context");

			utils::ref_count_ptr<utils::base_async_result_action<T>> core_action;
			m_core_object->begin_invoke<T>(func, nullptr, &core_action, forceAsync);
			action = AsyncResultAction<T>(core_action);
		}

		void EndInvoke(const AsyncAction& action)
		{
			ThrowOnEmpty("Context");

			if (action.Empty() == true)
				throw std::invalid_argument("action");

			utils::ref_count_ptr<core::action_interface> core_action;
			action.UnderlyingObject(&core_action);

			m_core_object->end_invoke(core_action);
		}

		void EndInvoke(const AsyncAction& action, AsyncState& state)
		{
			ThrowOnEmpty("Context");

			if (action.Empty() == true)
				throw std::invalid_argument("action");

			utils::ref_count_ptr<core::action_interface> core_action;
			action.UnderlyingObject(&core_action);

			m_core_object->end_invoke(core_action, &state);
		}

		template <typename T>
		T EndInvoke(const AsyncResultAction<T>& action)
		{
			ThrowOnEmpty("Context");

			if (action.Empty() == true)
				throw std::invalid_argument("action");

			utils::ref_count_ptr<utils::base_async_result_action<T>> core_action;
			action.UnderlyingObject(&core_action);

			return m_core_object->end_invoke<T>(core_action);
		}

		template <typename T>
		T EndInvoke(const AsyncResultAction<T>& action, AsyncState& state)
		{
			ThrowOnEmpty("Context");

			if (action.Empty() == true)
				throw std::invalid_argument("action");

			utils::ref_count_ptr<utils::base_async_result_action<T>> core_action;
			action.UnderlyingObject(&core_action);

			return m_core_object->end_invoke<T>(core_action, &state);
		}

		void Invoke(const AsyncAction& action)
		{
			ThrowOnEmpty("Context");

			if (action.Empty() == true)
				throw std::invalid_argument("action");

			utils::ref_count_ptr<core::action_interface> core_action;
			action.UnderlyingObject(&core_action);

			m_core_object->invoke(static_cast<core::action_interface*>(core_action));
		}

		void Invoke(const AsyncAction& action, AsyncState& state)
		{
			ThrowOnEmpty("Context");

			if (action.Empty() == true)
				throw std::invalid_argument("action");

			utils::ref_count_ptr<core::action_interface> core_action;
			action.UnderlyingObject(&core_action);

			m_core_object->invoke(static_cast<core::action_interface*>(core_action), &state);
		}

		void Invoke(const std::function<void()>& func)
		{
			ThrowOnEmpty("Context");
			m_core_object->invoke(func);
		}

		void Invoke(const std::function<void()>& func, AsyncState& state)
		{
			ThrowOnEmpty("Context");
			m_core_object->invoke(func, &state);
		}

		template <typename T>
		void Invoke(const AsyncResultAction<T>& action)
		{
			if (action.Empty() == true)
				throw std::invalid_argument("action");

			utils::ref_count_ptr<utils::base_async_result_action<T>> core_action;
			action.UnderlyingObject(&core_action);

			m_core_object->invoke<T>(core_action);
		}

		template <typename T>
		void Invoke(const AsyncResultAction<T>& action, AsyncState& state)
		{
			ThrowOnEmpty("Context");

			if (action.Empty() == true)
				throw std::invalid_argument("action");

			utils::ref_count_ptr<utils::base_async_result_action<T>> core_action;
			action.UnderlyingObject(&core_action);

			m_core_object->invoke<T>(core_action, &state);
		}

		template <typename T>
		T Invoke(const std::function<T()>& func)
		{
			ThrowOnEmpty("Context");
			return m_core_object->invoke<T>(func);
		}

		template <typename T>
		T Invoke(const std::function<T()>& func, AsyncState& state)
		{
			ThrowOnEmpty("Context");
			return m_core_object->invoke<T>(func, &state);
		}
		
		TimerRegistrationParams RegisterTimer(double interval, const std::function<void()>& func, unsigned int invocationCount = 0)
		{
			ThrowOnEmpty("Context");
			return m_core_object->register_timer(interval, func, invocationCount);
		}

		bool UnregisterTimer(TimerToken token)
		{
			ThrowOnEmpty("Context");
			return m_core_object->unregister_timer(token);
		}

		bool UnregisterTimer(const TimerRegistrationParams& registrationParams)
		{
			ThrowOnEmpty("Context");
			return m_core_object->unregister_timer(registrationParams);
		}

		void AsyncLoop(const std::function<bool()>& pred, const std::function<void()>& func)
		{
			ThrowOnEmpty("Context");
			m_core_object->async_loop(pred, func);
		}

		void Sync()
		{
			ThrowOnEmpty("Context");
			m_core_object->sync();
		}
	};
	
	class AutoTimerToken :
		public Common::CoreObjectWrapper<utils::auto_timer_token>
	{
	public:
		AutoTimerToken()
		{
			// Empty Token
		}		

		AutoTimerToken(utils::auto_timer_token* token) :
			Common::CoreObjectWrapper<utils::auto_timer_token>(token)
		{
		}

		AutoTimerToken(const Utils::TimerRegistrationParams& registrationParams) :
			AutoTimerToken(utils::make_ref_count_ptr<utils::auto_timer_token>(registrationParams))
		{
		}

		AutoTimerToken(Utils::TimerRegistrationParams&& registrationParams) :
			AutoTimerToken(utils::make_ref_count_ptr<utils::auto_timer_token>(std::forward<Utils::TimerRegistrationParams>(registrationParams)))
		{
		}

		void Unregister()
		{
			if (Empty() == true)
				return;

			m_core_object->unregister();
		}
	};

	class Timer : public Common::CoreObjectWrapper<utils::timer>
	{
	public:
		using Signal = utils::signal<utils::timer>;

		Timer() :
			Common::CoreObjectWrapper<utils::timer>(utils::make_ref_count_ptr<utils::timer>())
		{
		}

		Signal& Elapsed()
		{
			ThrowOnEmpty("Timer");
			return m_core_object->elapsed;
		}

		void Start(double interval, bool autoReset = false)
		{
			ThrowOnEmpty("Timer");
			m_core_object->start(interval, autoReset);
		}

		void Stop()
		{
			ThrowOnEmpty("Timer");
			m_core_object->stop();
		}
	};
}