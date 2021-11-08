#pragma once
#include <core/application.h>
#include <core/database.h>

namespace dispatchers
{
	class DLL_EXPORT red_dispatcher : public core::application::runnable_interface
	{
	public:
		virtual ~red_dispatcher() = default;
		static bool create(core::database::table_interface* table, core::application::runnable_interface** runnable);
	};
}