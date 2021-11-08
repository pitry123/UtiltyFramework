#pragma once
#include <core/disposable_interface.h>
#include <utils/ref_count_base.hpp>
#include <utils/callback_handler.hpp>
#include <utils/signal.hpp>

namespace utils
{
	template<typename T>
	class disposable_base : public utils::ref_count_base<T>
	{
	private:
		mutable callback_handler<core::disposable_callback_interface> m_disposable_callback_handler;

	protected:
		virtual ~disposable_base()
		{
			m_disposable_callback_handler.raise_callbacks([](core::disposable_callback_interface* callback)
			{
				callback->on_disposed();
			});
		}		

	public:
		virtual bool register_disposable_callback(core::disposable_callback_interface* callback) const override
		{
			return m_disposable_callback_handler.add_callback(callback);
		}

		virtual bool unregister_disposable_callback(core::disposable_callback_interface* callback) const override
		{
			return m_disposable_callback_handler.remove_callback(callback);
		}
	};

	class smart_disposable_callback :
		public utils::ref_count_base<core::disposable_callback_interface>
	{
	public:
		utils::signal<smart_disposable_callback> disposed;

		virtual void on_disposed() override
		{
			disposed();
		}
	};
}