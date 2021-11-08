#pragma once
#include <core/disposable_interface.h>

namespace core
{
	enum async_state
	{
		pending,
		running,
		completed,
		exception,
		cancelled
	};

	class DLL_EXPORT invokable_interface : public core::ref_count_interface
	{
	public:
		virtual ~invokable_interface() = default;
		virtual void invoke() = 0;
	};

	class DLL_EXPORT action_interface : public invokable_interface
	{
	public:
		virtual ~action_interface() = default;
		virtual async_state state() const = 0;

		virtual bool wait(long timeout) = 0;
		virtual void cancel() = 0;
	};

	class DLL_EXPORT context_interface : public core::disposable_interface
	{
	public:
		using timer_token = int;
		static constexpr timer_token timer_token_undefined = -1;

		virtual ~context_interface() = default;
		virtual unsigned long id() const = 0;
		virtual const char* name() const = 0;
		virtual bool disposed() const = 0;
		virtual bool invoke_required() const = 0;
		virtual bool idle() const = 0;

		virtual bool suspendable() const = 0;
		virtual bool suspended() const = 0;
		virtual bool suspend() = 0;
		virtual bool resume() = 0;

		virtual void begin_invoke(core::action_interface* action, bool force_async = false) const = 0;
		virtual void end_invoke(core::action_interface* action, core::async_state* action_state) const = 0;
		virtual void invoke(core::action_interface* action, core::async_state* action_state) const = 0;

		virtual core::context_interface::timer_token register_timer(double interval, core::invokable_interface* invokable, unsigned int invocation_count) const = 0;
		virtual bool unregister_timer(core::context_interface::timer_token id) const = 0;

		virtual void sync() const = 0;
	};
}
