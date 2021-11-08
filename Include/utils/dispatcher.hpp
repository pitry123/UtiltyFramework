#pragma once
#include <functional>
#include <condition_variable>
#include <stdexcept>
#include <mutex>
#include <chrono>
#include <queue>
#include <thread>
#include <atomic>
#include <map>
#include <algorithm>
#include <string>
#include <core/context.h>
#include <utils/ref_count_base.hpp>
#include <utils/disposable_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/disposable_ptr.hpp>
#include <utils/scope_guard.hpp>

#define ACTIONS_ALLOCATOR_RESERVE_SIZE 1024
#define TIMERS_ALLOCATOR_RESERVE_SIZE 32

#ifdef _MSC_VER
#if _MSC_VER <= 1800
#define MSVC_12
#endif
#endif

#define UNKNOWN_DISPATCHER_NAME "Unknown_Disp"

namespace utils
{
	// Note for MSVC users:
	// Prior to MSVC 14.0 (Visual Studio 2015),
	// Microsoft is using system_clock as high_resolution_clock
	// which might affect timer's performance
	using Clock = std::chrono::high_resolution_clock;

	using timer_token = core::context_interface::timer_token;
	static constexpr timer_token timer_token_undefined = core::context_interface::timer_token_undefined;

	class invokable_func : public utils::ref_count_base<core::invokable_interface>
	{
	private:
		std::function<void()> m_func;

	public:
		invokable_func(const std::function<void()>& func) :
			m_func(func)
		{
		}

		virtual void invoke() override
		{
			m_func();
		}
	};

	class context_exception : public std::exception
	{
	private:
		const core::context_interface& m_context;

	public:
		context_exception(const core::context_interface& context) : m_context(context)
		{
		}

		const core::context_interface& context()
		{
			return m_context;
		}
	};

	class exception_handler_interface
	{
	public:
		virtual void on_exception() = 0;
		virtual void on_exception(context_exception& e) = 0;
		virtual void on_exception(std::exception& e) = 0;
	};

	class async_action_sync_exception : public context_exception
	{
	public:
		async_action_sync_exception(const core::context_interface& context) : context_exception(context)
		{
		}

#ifdef MSVC_12
		virtual const char* what() const override
#else
		virtual const char* what() const noexcept override
#endif
		{
			return "AsyncAction cannot be synchronized from the context it's scheduled to run or currently running";
		}
	};

	class context_disposed_exception : public context_exception
	{
	public:
		context_disposed_exception(const core::context_interface& context) : context_exception(context)
		{
		}

#ifdef MSVC_12
		virtual const char* what() const override
#else
		virtual const char* what() const noexcept override
#endif
		{
			return "Context disposed";
		}
	};

	class base_async_action : public utils::ref_count_base<core::action_interface>
	{
	private:
		core::async_state m_state;
		mutable std::mutex m_mutex;
		std::condition_variable m_wait_handle;
		const core::context_interface& m_context;

		base_async_action(const base_async_action& other);			// non construction-copyable
		base_async_action(base_async_action&& other);					// non construction-movable
		base_async_action& operator=(const base_async_action&) = delete;		// non copyable
		base_async_action& operator=(base_async_action&& other) = delete;		// non movable

		void set_state(core::async_state state)
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			m_state = state;
		}

		void signal(core::async_state state)
		{
			set_state(state);
			m_wait_handle.notify_all();
		}

		virtual void invoke() override
		{
			bool completed_successfully = false;

			utils::scope_guard scopeGuard([this, &completed_successfully]() -> void
			{
				signal((completed_successfully == true) ? core::async_state::completed : core::async_state::exception);
			});

			set_state(core::async_state::running);

			perform();
			completed_successfully = true;
		}

		virtual void cancel() override
		{
			signal(core::async_state::cancelled);
		}

	protected:
		base_async_action(const core::context_interface& context) :
			m_state(core::async_state::pending),
			m_context(context)
		{
		}

		virtual void perform() = 0;

	public:
		virtual ~base_async_action() = default;

		virtual core::async_state state() const override
		{
			std::unique_lock<std::mutex> locker(m_mutex);
			core::async_state retVal = m_state;
			locker.unlock();

			return retVal;
		}

		virtual bool wait(long timeout = 0) override
		{
			if (timeout < 0)
				return false;

			std::unique_lock<std::mutex> locker(m_mutex);

			if ((m_state == core::async_state::pending || m_state == core::async_state::running) && m_context.invoke_required() == false)
			{
				throw async_action_sync_exception(m_context);
			}

			auto pred = [this]() -> bool
			{
				return ((m_state == core::async_state::completed) || (m_state == core::async_state::exception) || (m_state == core::async_state::cancelled));
			};

			if (timeout == 0)
			{
				m_wait_handle.wait(locker, pred);
				return (m_state != core::async_state::cancelled);
			}
			else
			{
				return ((m_wait_handle.wait_for(locker, std::chrono::milliseconds(timeout), pred) == true) && (m_state != core::async_state::cancelled));
			}
		}
	};
	
	template <typename T>
	class base_async_result_action : public base_async_action
	{
	protected:
		base_async_result_action(const core::context_interface& context) :
			base_async_action(context)
		{
		}

	public:
		virtual ~base_async_result_action() = default;
		virtual const T& result() = 0;
	};

	class async_action : public base_async_action
	{
	private:
		std::function<void()> m_func;

	protected:
		virtual void perform() override
		{
			scope_guard func_releaser([this]() -> void
			{
				m_func = nullptr;
			});

			m_func();
		}

	public:
		async_action(const core::context_interface& context, const std::function<void()>& func) :
			base_async_action(context), m_func(func)
		{
		}

		virtual ~async_action() = default;
	};

	template <typename T>
	class async_result_action : public base_async_result_action<T>
	{
	private:
		T m_result;
		std::function<T()> m_func;
		std::function<void(const T&)> m_result_callback;

	protected:
		virtual void perform() override
		{
			scope_guard func_releaser([this]() -> void
			{
				m_func = nullptr;
				m_result_callback = nullptr;
			});

			m_result = m_func();

			if (m_result_callback != nullptr)
			{
				m_result_callback(m_result);
			}
		}

	public:
		async_result_action(
			const core::context_interface& context,
			const std::function<T()>& func,
			const std::function<void(const T&)>& result_callback) :
			base_async_result_action<T>(context),
			m_func(func), m_result_callback(result_callback)
		{
		}

		virtual const T& result() override
		{
			base_async_result_action<T>::wait();
			return m_result;
		}
	};	

	class timer_registration_params;
	
	class simple_context : public utils::disposable_base<core::context_interface>
	{
	private:
		class timer : public utils::ref_count_base<core::ref_count_interface>
		{
#define RESERVED_TIMER_CLIENTS_SIZE 2
		private:
			class client_wrapper :
				public utils::ref_count_base<core::ref_count_interface>
			{
#define UNDEFINED_CLINET_ID -1
			private:
				timer_token m_id;
				utils::ref_count_ptr<core::invokable_interface> m_timer_client;
				mutable unsigned int m_invocation_count;
				mutable bool m_expired;

				timer_token get_next_client_id()
				{
					static timer_token NEXT_ID = 0;
					static std::mutex ID_MUTEX;

					std::lock_guard<std::mutex> locker(ID_MUTEX);
					timer_token retval = NEXT_ID++;

					if (NEXT_ID == (std::numeric_limits<int>::max)())
						NEXT_ID = 0;

					return retval;
				}

			public:
				client_wrapper() : m_id(UNDEFINED_CLINET_ID), m_invocation_count(0)
				{
				}

				client_wrapper(core::invokable_interface* timer_client, unsigned int invocation_count) :
					m_id(get_next_client_id()),
					m_timer_client(timer_client),
					m_invocation_count(invocation_count),
					m_expired(false)
				{
				}

				timer_token id() const
				{
					return m_id;
				}

				bool expired() const
				{
					return m_expired;
				}

				void invoke() const
				{
					if (m_invocation_count > 0)
					{
						--m_invocation_count;
						if (m_invocation_count == 0)
						{
							m_expired = true;
						}
					}

					m_timer_client->invoke();
				}
			};

			double m_interval;
			std::mutex m_mutex;
			std::vector<utils::ref_count_ptr<client_wrapper>> m_clients;
			std::vector<utils::ref_count_ptr<client_wrapper>> m_executing_clients;

			std::vector<utils::ref_count_ptr<client_wrapper>>::iterator find_client(timer_token id)
			{
				for (auto it = m_clients.begin(); it != m_clients.end(); it++)
				{
					if ((*it)->id() == id)
					{
						return it;
					}
				}

				return m_clients.end();
			}

		public:
			timer(double interval) :
				m_interval(interval)
			{
				m_clients.reserve(RESERVED_TIMER_CLIENTS_SIZE);
				m_executing_clients.reserve(RESERVED_TIMER_CLIENTS_SIZE);
			}

			double interval()
			{
				return m_interval;
			}

			int client_count()
			{
				std::unique_lock<std::mutex> locker(m_mutex);
				int retval = static_cast<int>(m_clients.size());

				return retval;
			}

			timer_token add_client(core::invokable_interface* timer_client, unsigned int invocation_count)
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				utils::ref_count_ptr<client_wrapper> client = utils::make_ref_count_ptr<client_wrapper>(timer_client, invocation_count);
				m_clients.emplace_back(client);

				return client->id();
			}

			bool remove_client(timer_token id)
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				auto it = find_client(id);
				if (it == m_clients.end())
					return false;

				m_clients.erase(it);
				return true;
			}

			bool is_expired(Clock::time_point& now, Clock::time_point& reference_time, unsigned long long& expire_interval)
			{
				bool retval = false;

#ifndef MSVC_12
				double time_interval = (double)((std::chrono::duration_cast<std::chrono::nanoseconds>)(now - reference_time)).count();
				if (time_interval >= (m_interval * 1000000.0))
				{
					expire_interval = 0;
					retval = true;
				}
				else
				{
					expire_interval = static_cast<unsigned long long>((m_interval * 1000000.0) - time_interval);
				}
#else
				double time_interval = (double)((std::chrono::duration_cast<std::chrono::microseconds>)(now - reference_time)).count();
				if (time_interval >= (m_interval * 1000.0))
				{
					expire_interval = 0;
					retval = true;
				}
				else
				{
					expire_interval = static_cast<unsigned long long>((m_interval * 1000.0) - time_interval);
				}
#endif

				return retval;
			}

			void invoke()
			{
				std::unique_lock<std::mutex> locker(m_mutex);
				m_executing_clients.insert(m_executing_clients.end(), m_clients.begin(), m_clients.end());
				locker.unlock();

				utils::scope_guard executing_clients_clear([this]() -> void
				{
					m_executing_clients.clear();
				});

				for (auto& client : m_executing_clients)
				{
					client->invoke();

					if (client->expired())
						remove_client(client->id());
				}
			}
		};
		
		std::thread m_invocation_thread;
		unsigned long m_id;
		std::string m_name;
		std::atomic<bool> m_running;

		bool m_suspendable;
		std::atomic<bool> m_suspended;
		volatile bool* m_stack_stopper;
		mutable bool m_idle;
		mutable bool m_adding_timer;
		exception_handler_interface* m_exception_handler;

		mutable std::mutex m_mutex;
		mutable std::condition_variable m_wait_handle;
		mutable std::vector<utils::ref_count_ptr<core::action_interface>> m_actions;
		mutable std::vector<std::pair<Clock::time_point, utils::ref_count_ptr<timer>>> m_timers;

		simple_context(const simple_context& other);                    // non construction-copyable
		simple_context& operator=(const simple_context&) = delete;		// non copyable

		void swap(simple_context& other)
		{
			other.dispose(true);

			std::unique_lock<std::mutex> locker(m_mutex);

			m_id = other.m_id;
			m_name = other.m_name;
			m_suspended = other.suspended();

			m_exception_handler = other.m_exception_handler;

			m_actions = std::move(other.m_actions);
			m_timers = std::move(other.m_timers);

			locker.unlock();
			m_wait_handle.notify_one();
		}

		unsigned long get_next_task_id()
		{
			static std::atomic<unsigned long> ID;
			return ID++;
		}

		void invoke_action(core::invokable_interface* invokable) const
		{
			try
			{
				invokable->invoke();
			}
			catch (context_exception& e)
			{
				if (m_exception_handler != nullptr)
				{
					m_exception_handler->on_exception(e);
				}
				else
				{
					throw e;
				}
			}
			catch (std::exception& e)
			{
				if (m_exception_handler != nullptr)
				{
					m_exception_handler->on_exception(e);
				}
				else
				{
					throw e;
				}
			}
			catch (...)
			{
				if (m_exception_handler != nullptr)
				{
					m_exception_handler->on_exception();
				}
				else
				{
					throw;
				}
			}
		}

		void worker()
		{
			// Set thread name (OS dependencies are handled inside function)
			if (m_name.empty() == false)
				set_current_thread_name(m_name.c_str());

			bool stack_stopper = false;
			m_stack_stopper = &stack_stopper;
			std::vector<utils::ref_count_ptr<core::action_interface>> pending_actions;
			pending_actions.reserve(ACTIONS_ALLOCATOR_RESERVE_SIZE);

			std::vector<utils::ref_count_ptr<timer>> pending_timers;
			pending_timers.reserve(TIMERS_ALLOCATOR_RESERVE_SIZE);

			Clock::time_point deadline;
			unsigned long long wait_interval = 0;
			bool timer_iteration;

			std::function<bool()> pred;

			// We check if suspension is supported in order to avoid checking the value of m_suspended which interlocks and might affect performance
			if (suspendable() == false)
			{
				pred = [this]() -> bool
				{
					return ((m_actions.size() > 0) || (m_adding_timer == true) || (m_running == false));
				};
			}
			else
			{
				pred = [this]() -> bool
				{
					return ((((m_actions.size() > 0) || (m_adding_timer == true)) && m_suspended == false) || (m_running == false));
				};
			}

			while (stack_stopper == false)
			{
				pending_timers.clear();
				std::unique_lock<std::mutex> locker(m_mutex);

				m_idle = true;
				if ((suspendable() == true && m_suspended == true) || get_next_timers(pending_timers, deadline, wait_interval) == false)
				{
					timer_iteration = false;
					m_wait_handle.wait(locker, pred);
				}
				else
				{
					if (wait_interval > 0)
					{
						timer_iteration = (m_wait_handle.wait_until(locker, deadline, pred) == false);
					}
					else
					{
						timer_iteration = true;
					}
				}

				if (suspendable() && m_suspended == true)
				{
					timer_iteration = false;
				}
				else
				{
					m_adding_timer = false;
					std::swap(m_actions, pending_actions);
				}

				if (timer_iteration == true)
					m_idle = (check_timers(deadline, pending_timers) == false);

				if (m_idle == true)
					m_idle = (pending_actions.size() == 0);

				bool stop_thread = (m_running == false);

				locker.unlock();

				if (timer_iteration == true)
				{
					for (auto timer : pending_timers)
					{
						if (stack_stopper == true)
							break;

						timer->invoke();
					}
				}

				for (auto& action : pending_actions)
				{
					if (stack_stopper == true)
					{
						action->cancel();
					}
					else
					{
						invoke_action(action);
					}
				}

				pending_actions.clear();

				if (stop_thread == true)
					break;
			}
		}

		bool get_next_timers(std::vector<utils::ref_count_ptr<timer>>& timers, Clock::time_point& deadline, unsigned long long& wait_interval)
		{
			// Clean empty timers
			m_timers.erase(std::remove_if(m_timers.begin(), m_timers.end(), [](const std::pair<Clock::time_point, utils::ref_count_ptr<timer>>& timePair) -> bool
			{
				return (timePair.second->client_count() == 0);
			}), m_timers.end());

			wait_interval = (std::numeric_limits<unsigned long long>::max)();

			if (m_timers.size() > 0)
			{
				Clock::time_point now = Clock::now();

				for (auto it = m_timers.begin(); it != m_timers.end(); it++)
				{
					utils::ref_count_ptr<simple_context::timer> timer = it->second;

					if (timer->client_count() > 0)
					{
						unsigned long long expire_interval;
						timer->is_expired(now, it->first, expire_interval);

						bool add_timer = false;
						if (expire_interval < wait_interval)
						{
							timers.clear();
							wait_interval = expire_interval;

#ifndef MSVC_12
							deadline = now + std::chrono::nanoseconds(wait_interval);
#else
							deadline = now + std::chrono::microseconds(wait_interval);
#endif
							add_timer = true;
						}
						else if (expire_interval == wait_interval)
						{
							add_timer = true;
						}

						if (add_timer == true)
						{
							timers.emplace_back(timer);
						}
					}
				}
			}

			return (timers.size() > 0);
		}

		bool check_timers(Clock::time_point& now, std::vector<utils::ref_count_ptr<timer>>& timers)
		{
			std::vector<std::vector<utils::ref_count_ptr<timer>>::iterator> removed_timers;

			for (std::vector<utils::ref_count_ptr<timer>>::iterator itX = timers.begin(); itX != timers.end(); itX++)
			{
				bool found = false;
				for (auto itY = m_timers.begin(); itY != m_timers.end(); itY++)
				{
					if ((*itX) == itY->second)
					{
						std::pair<Clock::time_point, utils::ref_count_ptr<timer>> swap_pair(now, itY->second);
						(*itY).swap(swap_pair);
						found = true;

						break;
					}
				}

				if (found == false)
					removed_timers.emplace_back(itX);
			}

			for (auto removedTimer : removed_timers)
			{
				timers.erase(removedTimer);
			}

			return (timers.size() > 0);
		}

		void add_action(core::action_interface* action) const
		{
			std::unique_lock<std::mutex> locker(m_mutex);

			if (m_running == false)
				throw context_disposed_exception(*(this));

			m_actions.emplace_back(action);
			m_idle = false;
			locker.unlock();

			m_wait_handle.notify_one();
		}

		timer_token add_timer(double interval, core::invokable_interface* invokable, unsigned int invocation_count) const
		{
			std::unique_lock<std::mutex> locker(m_mutex);

			auto it = invocation_count == 0 ? std::find_if(m_timers.begin(), m_timers.end(), [&interval](const std::pair<Clock::time_point, utils::ref_count_ptr<timer>>& timePair) -> bool
			{
				return (timePair.second->interval() == interval);
			}) : m_timers.end();

			if (it != m_timers.end() && it->second->client_count() == 0)
			{
				m_timers.erase(it);
				it = m_timers.end();
			}

			utils::ref_count_ptr<simple_context::timer> timer;
			if (it != m_timers.end())
			{
				timer = it->second;
			}
			else
			{
				timer = utils::make_ref_count_ptr<simple_context::timer>(interval);
				m_timers.emplace_back(std::pair<Clock::time_point, utils::ref_count_ptr<simple_context::timer>>(Clock::now(), timer));
			}

			int retval = timer->add_client(invokable, invocation_count);
			m_adding_timer = true;
			locker.unlock();

			m_wait_handle.notify_one();
			return retval;
		}

		bool remove_timer(timer_token id) const
		{
			bool retval = false;

			std::unique_lock<std::mutex> locker(m_mutex);

			m_timers.erase(std::remove_if(m_timers.begin(), m_timers.end(), [&id, &retval](const std::pair<Clock::time_point, utils::ref_count_ptr<timer>>& time_pair) -> bool
			{
				if (time_pair.second->remove_client(id) == true)
				{
					retval = true;
				}
				else
				{
					return false;
				}

				return (time_pair.second->client_count() == 0);
			}), m_timers.end());

			locker.unlock();

			if (suspended() == false)
				sync();

			return retval;
		}

		void dispose(bool swap)
		{
			bool expected = true;
			bool desired = false;
			if (m_running.compare_exchange_strong(expected, desired) == true)
			{
				if (invoke_required() == true)
				{
					if (m_invocation_thread.joinable() == true)
					{
#if defined(_WIN32) && defined(_MSC_VER)
						// On Windows, a thread might terminate due to DLL unload when the process is being closed.
						// We determine if the thread is alive in order to avoid deadlock when notifying the wait_handle.
						// This issue seems to affect Windows 7 and below only.
						// It seems to be resolved on Windows 10.
						DWORD result = WaitForSingleObject(m_invocation_thread.native_handle(), 0);
						if (result != WAIT_OBJECT_0)
							m_wait_handle.notify_one();
#else
						m_wait_handle.notify_one();
#endif
						m_invocation_thread.join();
					}
				}
				else
				{
					// That's a bit tricky...
					// Obviously we can't join a thread from its context...
					// We use a stack variable belonging to the context to notify the invocation thread NOT to access any of the class members and stop gracefully.
					*m_stack_stopper = true;
					m_invocation_thread.detach();
				}

				if (swap == false)
				{
					// We need to clean (cancel) pending actions to avoid deadlocks of waiting executions
					// We know for sure that no more action will be added since (m_running == false)                            
					// Note that we're not cleaning the actions if we're swapping since they can be handled by the swapped dispatcher
					std::lock_guard<std::mutex> locker(m_mutex);
					for (auto action : m_actions)
					{
						action->cancel();
					}

					m_actions.clear();
				}
			}
		}		

	public:
		simple_context(const char* name, bool start_suspended, exception_handler_interface* exception_handler = nullptr) :
			m_id(get_next_task_id()), m_name(name == nullptr ? UNKNOWN_DISPATCHER_NAME : name), m_running(true), m_suspendable(true), m_suspended(start_suspended), m_adding_timer(false), m_exception_handler(exception_handler)
		{
			m_actions.reserve(ACTIONS_ALLOCATOR_RESERVE_SIZE);
			m_timers.reserve(TIMERS_ALLOCATOR_RESERVE_SIZE);
			m_invocation_thread = std::thread([this] { worker(); });
		}

		simple_context(const char* name, exception_handler_interface* exception_handler = nullptr) :
			simple_context(name, false, exception_handler)
		{
			m_suspendable = false;
		}

		simple_context(exception_handler_interface* exception_handler = nullptr) :
			simple_context(UNKNOWN_DISPATCHER_NAME, exception_handler)
		{
		}

		simple_context(simple_context&& other) : m_running(true)
		{
			swap(other);
			m_invocation_thread = std::thread([this] { worker(); });
		}

		virtual ~simple_context()
		{
			dispose(false);
		}

		simple_context& operator=(simple_context&& other)
		{
			swap(other);
			return *this;
		}

		virtual unsigned long id() const override
		{
			return m_id;
		}

		virtual const char* name() const override
		{
			return m_name.c_str();
		}

		virtual bool disposed() const override
		{
			return (m_running == false);
		}

		virtual bool invoke_required() const override
		{
			return (std::this_thread::get_id() != m_invocation_thread.get_id());
		}

		virtual bool idle() const override
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			bool retval = m_idle;

			return retval;
		}

		virtual bool suspendable() const override
		{
			return m_suspendable;
		}	

		virtual bool suspended() const override
		{
			return (suspendable() == true && m_suspended.load() == true);
		}

		virtual bool suspend() override
		{
			if (suspendable() == false)
				return false;

			m_suspended.exchange(true);
			return true;
		}

		virtual bool resume() override
		{
			if (suspendable() == false)
				return false;

			if (m_suspended.exchange(false) == true)
				m_wait_handle.notify_one(); // Notifying the worker thread only if context was already suspended

			return true;
		}

		virtual void begin_invoke(core::action_interface* action, bool force_async = false) const override
		{
			if (action == nullptr)
				throw std::invalid_argument("action");

			if (force_async == true || invoke_required() == true)
			{
				add_action(action);
			}
			else
			{
				invoke_action(action);
			}
		}
		
		virtual void end_invoke(core::action_interface* action, core::async_state* action_state = nullptr) const override
		{
			if (action == nullptr)
				throw std::invalid_argument("action");

			action->wait(0);

			if (action_state != nullptr)
				*action_state = action->state();
		}

		virtual void invoke(core::action_interface* action, core::async_state* action_state = nullptr) const override
		{
			begin_invoke(action);
			end_invoke(action, action_state);
		}

		virtual void sync() const override
		{
			utils::ref_count_ptr<core::action_interface> action = utils::make_ref_count_ptr<utils::async_action>(*this, []() -> void {});
			invoke(action);
		}

		virtual core::context_interface::timer_token register_timer(double interval, core::invokable_interface* invokable, unsigned int invocation_count) const override
		{
			return add_timer(interval, invokable, invocation_count);
		}

		virtual bool unregister_timer(core::context_interface::timer_token id) const override
		{
			return remove_timer(id);
		}		
	};

	class dispatcher;
	class auto_timer_token;

	class timer_registration_params
	{
		friend class dispatcher;
		friend class auto_timer_token;

	private:
		utils::disposable_ptr<const core::context_interface> m_context;
		utils::timer_token m_token;

		void copy(const timer_registration_params& other)
		{
			m_context = other.m_context;
			m_token = other.m_token;
		}

		void move(timer_registration_params& other)
		{
			m_context = std::move(other.m_context);
			m_token = other.m_token;

			other.m_token = utils::timer_token_undefined;
		}

		bool query(const core::context_interface** context, utils::timer_token& token) const
		{
			if (context == nullptr)
				return false;

			utils::ref_count_ptr<const core::context_interface> strong_context;
			if (m_context.lock(&strong_context) == false)
				return false;

			if (m_token == utils::timer_token_undefined)
				return false;

			*context = strong_context;
			(*context)->add_ref();
			token = m_token;
			return true;
		}

		void reset()
		{
			m_token = utils::timer_token_undefined;
			m_context.reset();
		}

	public:
		timer_registration_params() :
			m_token(utils::timer_token_undefined)
		{
		}

		timer_registration_params(const core::context_interface* context, utils::timer_token token) :
			m_context(context), m_token(token)
		{
		}

		timer_registration_params(const timer_registration_params& other)
		{
			copy(other);
		}

		timer_registration_params(timer_registration_params&& other)
		{
			move(other);
		}

		timer_registration_params& operator=(const timer_registration_params& other)
		{
			copy(other);
			return *this;
		}

		timer_registration_params& operator=(timer_registration_params&& other)
		{
			move(other);
			return *this;
		}

		bool operator==(std::nullptr_t) const
		{
			utils::ref_count_ptr<const core::context_interface> strong_context;
			return (
				m_context.lock(&strong_context) == false ||
				m_token == utils::timer_token_undefined);
		}

		bool operator!=(std::nullptr_t) const
		{
			return !(*this == nullptr);
		}

		operator utils::timer_token() const
		{
			return m_token;
		}
	};

	class auto_timer_token : public utils::ref_count_base<core::ref_count_interface>
	{
	private:
		timer_registration_params m_registration_params;

		auto_timer_token(const auto_timer_token& other) = delete;				// non construction-copyable
		auto_timer_token& operator=(const auto_timer_token& other) = delete;	// non copyable						

	public:
		auto_timer_token() :
			m_registration_params()
		{
		}

		auto_timer_token(const utils::timer_registration_params& registration_params) :
			m_registration_params(registration_params)
		{
		}

		auto_timer_token(utils::timer_registration_params&& registration_params) :
			m_registration_params(std::move(registration_params))
		{
		}

		auto_timer_token(auto_timer_token&& other) :
			m_registration_params(std::move(other.m_registration_params))
		{
		}

		auto_timer_token& operator=(auto_timer_token&& other)
		{
			m_registration_params = std::move(other.m_registration_params);
			return *this;
		}

		void unregister()
		{
			utils::ref_count_ptr<const core::context_interface> context;
			utils::timer_token token;
			if (m_registration_params.query(&context, token) == false)
				return;

			utils::scope_guard reset_params([this]()
			{
				m_registration_params.reset();
			});

			context->unregister_timer(token);
		}

		virtual ~auto_timer_token()
		{
			unregister();
		}
	};

	/// This class define the worker thread and message pump.
	/// allowing calls between threads while avoiding locks between calls. 
	/// It implement the Invocation methods allowing synchronous (by blocking) and asynchronous calls
	/// Pay attention - this is not the Database dispatcher but a more basic dispatcher that allow any action/timer to be perform within
	/// @date	22/05/2018
	class dispatcher : public utils::disposable_base<core::context_interface>
	{
	private:
		utils::ref_count_ptr<core::context_interface> m_context;

		virtual void async_loop(utils::func_wrapper_base<bool>* loop_condition, utils::func_wrapper_base<void>* func) const
		{
			// This function is guarantied to be called inside our context so there is no need to check if invoke_required

			if (loop_condition == nullptr)
				throw std::invalid_argument("loop_condition");

			if (func == nullptr)
				throw std::invalid_argument("func");

			if (loop_condition->invoke() == true)
			{
				func->invoke();

				if (loop_condition->invoke() == true && disposed() == false)
				{
					utils::ref_count_ptr<utils::func_wrapper_base<bool>> local_loop_condition = loop_condition;
					utils::ref_count_ptr <utils::func_wrapper_base<void>> local_func = func;

					begin_invoke([this, local_loop_condition, local_func]() -> void
					{
						async_loop(static_cast<utils::func_wrapper_base<bool>*>(local_loop_condition),static_cast<utils::func_wrapper_base<void>*>(local_func));
					}, nullptr, true);
				}
			}
		}

	protected:
		dispatcher(const char* name, bool start_suspended, exception_handler_interface* exception_handler = nullptr) :
			dispatcher(utils::make_ref_count_ptr<utils::simple_context>(name, start_suspended, exception_handler))
		{			
		}

	public:
		dispatcher(core::context_interface* context) :
			m_context(context)
		{
			if (context == nullptr)
				throw std::invalid_argument("context");
		}

		/// Constructor -  this should be the main constructor used 
		///
		/// @date	23/05/2018
		///
		/// @param 		   	name			 	the dispatcher name (friendly name)
		/// @param [in]		exception_handler	(Optional) If non-null, the
		/// 	exception handler.
		dispatcher(const char* name, exception_handler_interface* exception_handler = nullptr) :
			dispatcher(utils::make_ref_count_ptr<utils::simple_context>(name, exception_handler))
		{
		}

		/// Constructor - this constructor which allow creating a no-name dispatcher
		///
		/// @date	23/05/2018
		///
		/// @param [in,out]	exception_handler	(Optional) If non-null, the
		/// 	exception handler.
		dispatcher(exception_handler_interface* exception_handler = nullptr) :
			dispatcher(utils::make_ref_count_ptr<utils::simple_context>(exception_handler))
		{
		}

		dispatcher(const dispatcher& other) :
			m_context(other.m_context)
		{
		}

		dispatcher(dispatcher&& other) :
			m_context(std::move(other.m_context))
		{
		}

		virtual ~dispatcher()
		{
			m_context = nullptr;
		}

		dispatcher& operator=(const dispatcher& other)
		{
			m_context = other.m_context;
			return *this;
		}

		dispatcher& operator=(dispatcher&& other)
		{
			m_context = std::move(other.m_context);
			return *this;
		}

		virtual unsigned long id() const override
		{
			return m_context->id();
		}

		virtual const char* name() const override
		{
			return m_context->name();
		}

		virtual bool disposed() const override
		{
			return m_context->disposed();
		}

		/// check whether invoke is required. 
		///
		/// @date	24/05/2018
		///
		/// @return	True if invoke is required (means call to invoke_required is from a different thread) , false if it from the dispatcher's thread
		virtual bool invoke_required() const override
		{
			return m_context->invoke_required();
		}

		virtual bool idle() const override
		{
			return m_context->idle();
		}

		virtual bool suspendable() const override
		{
			return m_context->suspendable();
		}

        virtual bool suspended() const override
		{
			return m_context->suspended();
		}

		virtual bool suspend() override
		{
			return m_context->suspend();
		}

		virtual bool resume() override
		{
			return m_context->resume();
		}

		/// Executes the given action on a different thread, asynchronously
		///
		/// @date	22/05/2018
		///
		/// @exception	std::invalid_argument	Thrown when action is null
		///
		/// @param [in]	action	   	the action to preform.
		/// @param 	   	force_async	(Optional) True to force asynchronous even if
		/// 	the call was perform on the same thread.
		virtual void begin_invoke(core::action_interface* action, bool force_async = false) const override
		{
			m_context->begin_invoke(action, force_async);			
		}

		/// Executes the given action on a different thread,
		/// asynchronously and allow getting a return value of the performed function by using the base_async_action 
		///
		/// @date	22/05/2018
		///
		/// @param 		   	func		   	The function to preform   
		/// @param [out]	pp_async_action	(Optional) If non-null, the asynchronous 
		/// 	action to return to the caller in order to allow it ti get the return value of the func
		/// @param 		   	force_async	(Optional) True to force asynchronous even if the call was perform on the same thread.
		virtual void begin_invoke(const std::function<void()>& func, core::action_interface** action = nullptr, bool force_async = false) const
		{
			utils::ref_count_ptr<core::action_interface> action_instance = utils::make_ref_count_ptr<utils::async_action>(*(this), func);
			begin_invoke(action_instance, force_async);

			if (action != nullptr)
			{
				(*action) = action_instance;
				(*action)->add_ref();
			}
		}

		/// Executes the given operation on a different thread, asynchronously
		/// and allow getting a return value of the performed function by calling the
		/// resault_callback
		///
		/// @date	22/05/2018
		///
		/// @tparam	T	Generic type parameter - represent the return value of the invoked function
		/// @param 		   	func				  	The function to preform.
		/// @param 		   	result_callback		  	(Optional) The result callback
		/// 	function initiated upon action finished and deliver the return
		/// 	value of the action.
		/// @param [in,out]	pp_async_result_action	(Optional) If non-null, the
		/// 	asynchronous result action.
		/// @param 		   	force_async			  	(Optional) True to force
		/// 	asynchronous even if the call was perform on the same thread.
		template <typename T> void begin_invoke(const std::function<T()>& func, std::function<void(const T&)> result_callback = nullptr, base_async_result_action<T>** action = nullptr, bool force_async = false) const
		{
			utils::ref_count_ptr<base_async_result_action<T>> base_action = utils::make_ref_count_ptr<utils::async_result_action<T>>(*(this), func, result_callback);
			begin_invoke(base_action, force_async);

			if (action != nullptr)
			{
				(*action) = base_action;
				(*action)->add_ref();
			}
		}

		/// Ends an invoke wait until the action finish and return with action status
		///
		/// @date	22/05/2018
		///
		/// @exception	std::invalid_argument	Thrown when action is null
		/// 	error condition occurs.
		///
		/// @param [in]	action		  	   the action.
		/// @param [out]	p_action_state	(Optional) If non-null, state of the
		/// 	action.
		virtual void end_invoke(core::action_interface* action, core::async_state* action_state = nullptr) const override
		{
			m_context->end_invoke(action, action_state);
		}

		/// Ends an invocation call. Wait until the begin_invoke finish and return with action status and set the return value of the function 
		///
		/// @date	22/05/2018
		///
		/// @exception	std::invalid_argument	Thrown when action is null
		/// 	error condition occurs.
		///
		/// @tparam	T	the return value type to be returned by the original function triggered by begin_invoke()
		/// @param [in,out]	action		  	If non-null, the action 
		/// @param [out]	p_action_state	(Optional) If non-null, state of the action.
		///
		/// @return	A T - the result from the called function
		template <typename T> T end_invoke(base_async_result_action<T>* action, core::async_state* action_state = nullptr) const
		{
			if (action == nullptr)
				throw std::invalid_argument("action");

			action->wait();
			
			if (action_state != nullptr)
				*action_state = action->state();

			return action->result();
		}

		/// Blocking: Executes the given operation on a different thread, and waits for
		/// the action to finish
		///
		/// @date	22/05/2018
		///
		/// @param [in,out]	action		  	If non-null, the action to perform
		/// @param [out]	p_action_state	(Optional) If non-null, state of the action
		virtual void invoke(core::action_interface* action, core::async_state* action_state = nullptr) const override
		{
			m_context->invoke(action, action_state);
		}

		/// Blocking: Executes the given operation on a different thread, and waits for
		/// the action to finish
		///
		/// @date	22/05/2018
		///
		/// @param 		   	func		  	The function to be invoked
		/// @param [in,out]	p_action_state	(Optional) If non-null, state of the
		/// 	action.
		virtual void invoke(const std::function<void()>& func, core::async_state* action_state = nullptr) const
		{
			utils::ref_count_ptr <core::action_interface > action;
			begin_invoke(func, &action);

			end_invoke(action, action_state);
		}

		/// Executes the given operation on a different thread, and waits for
		/// the result
		///
		/// @date	22/05/2018
		///
		/// @tparam	T	the result value of the invoked action
		/// @param [in]	action		  	the action.
		/// @param [out]	p_action_state	(Optional) If non-null, state of the
		/// 	action.
		///
		/// @return	A T - the result value of the invoked action
		template <typename T> T invoke(base_async_result_action<T>* action, core::async_state* action_state = nullptr) const
		{
			begin_invoke(action);
			return end_invoke(action, action_state);
		}

		/// Executes the given operation on a different thread, and waits for
		/// the result
		///
		/// @date	22/05/2018
		///
		/// @tparam	T	the function signature to be performed
		/// @param 		   	func		  	The function to be invoked
		/// @param [out]	p_action_state	(Optional) If non-null, state of the
		/// 	action.
		///
		/// @return	A T - the result value of the invoked action
		template <typename T> T invoke(const std::function<T()>& func, core::async_state* action_state = nullptr) const
		{
			utils::ref_count_ptr<base_async_result_action<T>> action;
			begin_invoke<T>(func, nullptr, &action);
			return end_invoke<T>(action, action_state);
		}

		/// Sync is used to create a blocking function that return only upon all pending action of this dispatcher were invoked. 
		/// this is helpful in order to wait until a set of actions were performed or to finalize the thead gracefully.
		/// @date	22/05/2018
		virtual void sync() const override
		{
			if (suspended())
				return;

			m_context->sync();
		}

		virtual core::context_interface::timer_token register_timer(double interval, core::invokable_interface* invokable, unsigned int invocation_count = 0) const override
		{
			return m_context->register_timer(interval, invokable, invocation_count);
		}

		/// Registers a timer on the thread 
		///
		/// @date	24/05/2018
		///
		/// @exception	std::invalid_argument	Thrown when an invalid argument
		/// 	error condition occurs.
		///
		/// @param	interval			The interval in milliseconds
		/// @param	func				The function to perform on the thread context 
		/// @param	invocation_count	(Optional) Number of invocations the timer is supposed to run, if 0 run infinitely.
		///
		/// @return	An int - the timer id (used as a token for unregister)
		virtual timer_registration_params register_timer(double interval, const std::function<void()>& func, unsigned int invocation_count = 0) const
		{
			if (interval <= 0)
				throw std::invalid_argument("interval");

			utils::ref_count_ptr<core::invokable_interface> invokable = utils::make_ref_count_ptr<utils::invokable_func>(func);
			return timer_registration_params(this, register_timer(interval, static_cast<core::invokable_interface*>(invokable), invocation_count));
		}

		/// Unregisters the timer described by ID
		///
		/// @date	24/05/2018
		///
		/// @param	id	The identifier.
		///
		/// @return	True if it succeeds, false if it fails.
		virtual bool unregister_timer(core::context_interface::timer_token id) const override
		{
			return m_context->unregister_timer(id);
		}

		virtual bool unregister_timer(const timer_registration_params& registration_params) const
		{
			utils::ref_count_ptr<const core::context_interface> context;
			utils::timer_token token;
			if (registration_params.query(&context, token) == false)
				return false;

			if (context != this)
				return false;

			return unregister_timer(token);
		}

		/// Asynchronous loop - allows a "while" call in the thread while avoid
		/// abuse of the thread message pump.
		/// 					using this function, allows the dispatcher to handle additional
		/// 					actions invoked during the loop is active
		///
		/// @date	24/05/2018
		///
		/// @param	loop_condition	function that checks whether the loop should continue.
		/// @param	func		  	The function to be performed inside the loop.
		virtual void async_loop(const std::function<bool()>& loop_condition, const std::function<void()>& func) const
		{
			utils::ref_count_ptr<utils::func_wrapper_base<bool>> loop_condition_wrapper =
				utils::make_ref_count_ptr<utils::func_wrapper_base<bool>>(loop_condition);

			utils::ref_count_ptr<utils::func_wrapper_base<void>> func_wrapper =
				utils::make_ref_count_ptr<utils::func_wrapper_base<void>>(func);

			this->begin_invoke([this, loop_condition_wrapper, func_wrapper]()
			{
				async_loop(static_cast<utils::func_wrapper_base<bool>*>(loop_condition_wrapper), static_cast<utils::func_wrapper_base<void>*>(func_wrapper));
			}, nullptr, true);
		}
	};

	class suspendable_dispatcher : public dispatcher
	{
	public:
		suspendable_dispatcher(const char* name, bool start_suspended = false, utils::exception_handler_interface* exception_handler = nullptr) :
			dispatcher(name, start_suspended, exception_handler)
		{
		}
	};
}