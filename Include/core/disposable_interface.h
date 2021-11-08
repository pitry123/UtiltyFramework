#pragma once
#include <core/ref_count_interface.h>

namespace core
{
	class DLL_EXPORT disposable_callback_interface : public core::ref_count_interface
	{
	public:
		virtual ~disposable_callback_interface() = default;
		virtual void on_disposed() = 0;
	};

	class DLL_EXPORT disposable_interface : public core::ref_count_interface
	{
	public:
		virtual ~disposable_interface() = default;

		virtual bool register_disposable_callback(core::disposable_callback_interface* callback) const = 0;
		virtual bool unregister_disposable_callback(core::disposable_callback_interface* callback) const = 0;
	};
}