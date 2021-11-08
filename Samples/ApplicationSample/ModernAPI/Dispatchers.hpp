#pragma once
#include <dispatchers/green_dispatcher.h>
#include <dispatchers/red_dispatcher.h>

#include <Database.hpp>
#include <Application.hpp>

namespace Dispatchers
{
	class GreenDispatcher :
		public Common::NonConstructible
	{
	public:
		static Application::Runnable Create(const Database::Table& table)
		{
			utils::ref_count_ptr<core::application::runnable_interface> instance;
			if (dispatchers::green_dispatcher::create(static_cast<core::database::table_interface*>(table), &instance) == false)
				throw std::runtime_error("Failed to create green dispatcher");

			return Application::Runnable(instance);
		}
	};

	class RedDispatcher :
		public Common::NonConstructible
	{
	public:
		static Application::Runnable Create(const Database::Table& table)
		{
			utils::ref_count_ptr<core::application::runnable_interface> instance;
			if (dispatchers::red_dispatcher::create(static_cast<core::database::table_interface*>(table), &instance) == false)
				throw std::runtime_error("Failed to create green dispatcher");

			return Application::Runnable(instance);
		}
	};
}