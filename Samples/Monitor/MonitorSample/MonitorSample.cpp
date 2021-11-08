// DatabaseSample.cpp : Defines the entry point for the console application.
//
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

int RandomInt(int max_value)
{
	return (std::rand() % max_value);
}

float RandomFloat(float max_value)
{
	return (static_cast<float>(std::rand()) / (static_cast<float>(static_cast<float>(RAND_MAX) / max_value)));
}

class GreenDispatcher : public Dispatcher
{
	DataSet m_dataset;
	SubscriptionToken m_token;
	Timer m_timer;

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
		m_dataset(dataset)

	{

		for (int entryIndex = 0; entryIndex < MonitorSample::NUM_OF_TABLE1_ROWS; entryIndex++)
		{
			m_token = Subscribe(dataset, ProjectCommon::Table1DBEnum, entryIndex, [this, entryIndex](const RowData& data)
			{
				int myData = data.Read<int>();
				HandleIntData(myData, entryIndex+1);
			});

		}

		m_timer.Elapsed() += [this]()
		{
			m_dataset[ProjectCommon::Table1DBEnum][RandomInt(MonitorSample::NUM_OF_TABLE1_ROWS)].Write(RandomInt(1000));
		};
	}

	~GreenDispatcher()
	{
		// TODO: Unsubscribe all rows...
	}

	virtual void Start() override
	{
		m_timer.Start(5000);
	}
};

void SimpleSample()
{
	DataSet dataset = MemoryDatabase::Create("Monitor");
	dataset.AddTable(ProjectCommon::Table1DBEnum);
	dataset.AddTable(ProjectCommon::Table2DBEnum);

	for (int i= 0; i < MonitorSample::NUM_OF_TABLE1_ROWS; i++)
	{
		dataset[ProjectCommon::Table1DBEnum].AddRow<int>(i);
		dataset[ProjectCommon::Table1DBEnum][i].Write<int>(i);
	}

	for (int i = 0; i < MonitorSample::NUM_OF_TABLE2_ROWS; i++)
	{
		dataset[ProjectCommon::Table2DBEnum].AddRow<float>(i);
		dataset[ProjectCommon::Table2DBEnum][i].Write<float>(static_cast<float>(i));
	}

	
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