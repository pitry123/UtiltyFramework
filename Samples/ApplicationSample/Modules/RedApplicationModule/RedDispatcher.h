#pragma once
#include <dispatchers/red_dispatcher.h>
#include <Core.hpp>
#include <Database.hpp>
#include <ApplicationSample/ApplicationSampleDB.h>

namespace Dispatchers
{
	class RedDispatcher : public Database::DispatcherBase<dispatchers::red_dispatcher>
	{
	private:
		static Logging::Logger LOGGER;
		
		Database::Table m_table;
		Database::AutoToken m_rowToken;
		Utils::AutoTimerToken m_timerToken;		

		void HandleMyData(const ApplicationSampleDB::MyData& data);

	public:
		RedDispatcher(const Database::Table& table);
		virtual ~RedDispatcher();

	protected:
		virtual void Init() override;
		virtual void Start() override;
		virtual void Stop() override;
	};	
}