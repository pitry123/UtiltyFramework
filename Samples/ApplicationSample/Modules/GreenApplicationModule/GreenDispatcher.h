#pragma once
#include <dispatchers/green_dispatcher.h>
#include <Core.hpp>
#include <Database.hpp>
#include <ApplicationSample/ApplicationSampleDB.h>
namespace Dispatchers
{
	class GreenDispatcher : public Database::DispatcherBase<dispatchers::green_dispatcher>
	{
	private:
		static Logging::Logger LOGGER;

		Database::Table m_table;
		Database::AutoToken m_rowToken;
		Utils::AutoTimerToken m_timerToken;

		void HandleInt(int data);

	public:
		GreenDispatcher(core::database::table_interface* table);
		virtual ~GreenDispatcher();

	protected:
		virtual void Init() override;
		virtual void Start() override;
		virtual void Stop() override;
	};	
}