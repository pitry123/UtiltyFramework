#pragma once
#include <core/application.h>
#include <core/database.h>

namespace dispatchers
{
	class DLL_EXPORT green_dispatcher : public core::application::runnable_interface
	{
	public:
		virtual ~green_dispatcher() = default;
		static bool create(core::database::table_interface* table, core::application::runnable_interface** runnable);
	};
}