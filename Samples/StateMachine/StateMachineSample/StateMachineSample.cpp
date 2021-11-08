// DatabaseSample.cpp : Defines the entry point for the console application.
//
#include "StateMachineLogic.h"
#include "Databases.h"
#include "Tables.h"

#include <Factories.hpp>
#include <Utils.hpp>
#include <Monitor.hpp>
#include <Strings.hpp>

#include <cstring>
#include <future>
#include <random>
#include <sstream>

using namespace Core;
using namespace Utils;
using namespace Database;
using namespace Communication;
using namespace Communication::Ports;
using namespace Communication::Protocols;

class GreenDispatcher : public Dispatcher
{
	DataSet m_dataset;
	Timer m_timer;
	StateMachineLogic m_statemachine;
	int m_counter;

private:
	void HandleIntData(int data, int index)
	{
		Console::ColorPrint(
			Console::Colors::GREEN,
			"Got data from Row %d (int): %d\n",
			index, data);
	}

public:
	GreenDispatcher(DataSet dataset) :
		Dispatcher(),
		m_dataset(dataset),
		m_statemachine(this, m_dataset, m_dataset[ ProjectCommon::Table1DBEnum][MonitorSample::STATE_MACHINE_STS]),
		m_counter(0)

	{
		for (int entryIndex = 0; entryIndex < MonitorSample::NUM_OF_TABLE1_ROWS; entryIndex++)
		{
			Subscribe(dataset, ProjectCommon::Table1DBEnum, entryIndex, [this, entryIndex](const RowData& data)
			{
				if (entryIndex != MonitorSample::STATE_MACHINE_STS)
				{
					int myData = data.Read<int>();
					HandleIntData(myData, entryIndex + 1);
				}
				
			});
		}

		m_statemachine.InitializeResetRun(0, StateMachineLogic::STATE_FIRST);
		
		m_timer.Elapsed() += [this]()
		{
			m_dataset[ProjectCommon::Table1DBEnum][MonitorSample::VALUE].Write(m_counter);

			if (++m_counter > 60)
				m_counter = 0;
		};
	}

	~GreenDispatcher()
	{
	}

	virtual void Start() override
	{
		m_timer.Start(500);
	}
};

void SimpleSample()
{
	DataSet dataset = MemoryDatabase::Create("StateMachine");
	dataset.AddTable(ProjectCommon::Table1DBEnum);
	dataset.AddTable(ProjectCommon::Table2DBEnum);

	dataset[ProjectCommon::Table1DBEnum].AddRow<int>(MonitorSample::VALUE);
	dataset[ProjectCommon::Table1DBEnum][MonitorSample::VALUE].Write<int>(0);

	dataset[ProjectCommon::Table1DBEnum].AddRow<int>(MonitorSample::STATUS);
	dataset[ProjectCommon::Table1DBEnum][MonitorSample::STATUS].Write<int>(MonitorSample::RUNNING);
	
	dataset[ProjectCommon::Table1DBEnum].AddRow<StateMachine::StateMachineEntryStruct>(MonitorSample::STATE_MACHINE_STS);
	
	dataset[ProjectCommon::Table2DBEnum].AddRow<int>(MonitorSample::STATUS);
	dataset[ProjectCommon::Table2DBEnum][MonitorSample::STATUS].Write<int>(MonitorSample::RUNNING);


	Monitor monitor("127.0.0.1", 4321, "127.0.0.1", 1234, dataset);
	monitor.Connect();

	GreenDispatcher distaptcherA(dataset);
	distaptcherA.Start();

	getchar();
}

int main(int argc, const char* argv[])
{
	SimpleSample();
	return 0;
}


