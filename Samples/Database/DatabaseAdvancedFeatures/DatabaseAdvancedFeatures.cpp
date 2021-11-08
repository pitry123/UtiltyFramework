// DatabaseSample.cpp : Defines the entry point for the console application.
//
#include <Core.hpp>
#include <Factories.hpp>
#include <Utils.hpp>
#include <Strings.hpp>

#include <cstring>
#include <future>
#include <random>
#include <iostream>
#include <sstream>

using namespace Database;


int RandomInt(int max_value)
{
	return (std::rand() % max_value);
}

float RandomFloat(float max_value)
{
	return (static_cast<float>(std::rand()) / (static_cast<float>(static_cast<float>(RAND_MAX) / max_value)));
}

enum Tables
{
	TABLE_1
};

enum Table1Rows
{
	ROW_1,
	ROW_2
};

struct MyData
{
public:
	int val1;
	int val2;
	float val3;
	double val4;
};


class GreenDispatcher : public Dispatcher
{
	DataSet m_dataset;
	
	Utils::Timer m_timer;
	AutoTableToken m_token;
private:
	void HandleIntData(int data)
	{
		Core::Console::ColorPrint(
			Core::Console::Colors::GREEN,
			"Got data from Row 2 In GreenDispatcher(int): %d\n",
			data);
	}

	void HandleMyData(const MyData& data)
	{
		Core::Console::ColorPrint(
			Core::Console::Colors::GREEN,
			"Got data from Row 1 in GreenDispatcher(MyData): {%d, %d, %f, %lf}\n",
			data.val1,
			data.val2,
			data.val3,
			data.val4);
	}
public:
	GreenDispatcher(DataSet dataset) :
		Dispatcher(),
		m_dataset(dataset)
	{
		m_token = SubscribeTable(dataset[Tables::TABLE_1], [this](const RowData& data)
		{
				if (data.DBRow().Key().Equals<int>(ROW_1))
				{
					int myData = data.Read<int>();
					HandleIntData(myData);
				}

				if (data.DBRow().Key().Equals<int>(ROW_2))
				{
					MyData myData = data.Read<MyData>();
					HandleMyData(myData);
				}

		});

		m_timer.Elapsed() += [this]()
		{
			Database::Row row;
			if(m_dataset[Tables::TABLE_1].TryGet(Table1Rows::ROW_2,row))
				row.Write<MyData>({ RandomInt(100), RandomInt(100), RandomFloat(100.0f), RandomFloat(100.0f) });
			if (m_dataset[Tables::TABLE_1].TryGet(Table1Rows::ROW_1, row))
				row.Write<int>(RandomInt(100));
		};
	}

	~GreenDispatcher()
	{
		
	}

	virtual void Start() override
	{
		m_timer.Start(50);
	}

	virtual void Stop() override
	{
	
	}
};

class RedDispatcher : public Dispatcher
{
	DataSet m_dataset;
	
	Utils::Timer m_timer;
	MyData m_data;
	int m_counter;
private:
	void HandleIntData(int data)
	{
		Core::Console::ColorPrint(
			Core::Console::Colors::RED,
			"Got data from Row 2 in RedDispatcher (int): %d\n",
			data);
	}

	void HandleMyData(const MyData& data)
	{
		Core::Console::ColorPrint(
			Core::Console::Colors::RED,
			"Got data from Row 1 (MyData): {%d, %d, %f, %lf}\n",
			data.val1,
			data.val2,
			data.val3,
			data.val4);
	}

public:
	RedDispatcher(DataSet dataset) :
		Dispatcher(),
		m_dataset(dataset),
		m_data({})
	{
		m_counter = 0;
			
		m_timer.Elapsed() += [this]()
		{
			m_counter++;
			if (m_counter == 100)
			{
				m_dataset[Tables::TABLE_1].AddRow<MyData>(Table1Rows::ROW_2);
			}
			if (m_counter == 150)
			{
				m_dataset[Tables::TABLE_1].AddRow<int>(Table1Rows::ROW_1);
			}
			if (m_counter == 300)
				m_dataset[Tables::TABLE_1].RemoveRow(Table1Rows::ROW_2);

			if (m_counter == 400)
				m_dataset[Tables::TABLE_1].RemoveRow(Table1Rows::ROW_1);

			if (m_counter == 450)
			{
				m_counter = 0;
			}
			if (m_counter % 50 == 0)
			{
				std::stringstream tableStream;
				tableStream << "Table (" << m_dataset[Tables::TABLE_1].Key() << "): Parent=" << m_dataset.Key() << " RowCount=" << m_dataset[Tables::TABLE_1].Size() << std::endl;
				Core::Console::ColorPrint(Core::Console::Colors::CYAN, "%s", tableStream.str().c_str());
			}

		};
	}

	~RedDispatcher()
	{
		
	}

	virtual void Start() override
	{
		m_timer.Start(50);
	}

	virtual void Stop() override
	{
		
	}
};

void SimpleSample(bool enableGreen, bool enableRed)
{
	DataSet dataset = MemoryDatabase::Create("My Database");
	//DataSet dataset = DDSDatabase::Create("My Database");
	dataset.AddTable(Tables::TABLE_1);
	

	/////// Print Database Description... ////////

	std::stringstream datasetStream;
	datasetStream << "Database Description (" << dataset.Key() << "): TableCount=" << dataset.Size() << std::endl;
	Core::Console::ColorPrint(
		Core::Console::Colors::WHITE,
		"%s\n", datasetStream.str().c_str());

	for (Table& table : dataset)
	{
		std::stringstream tableStream;
		tableStream << "Table (" << table.Key() << "): Parent=" << table.Parent().Key() << " RowCount=" << table.Size() << std::endl;
		Core::Console::ColorPrint(Core::Console::Colors::CYAN, "%s", tableStream.str().c_str());

		for (Row& row : table)
		{
			std::stringstream rowStream;
			rowStream << "	Row (" << row.Key() << "): Parent=" << row.Parent().Key() << " DataSize=" << row.DataSize() << std::endl;
			Core::Console::ColorPrint(Core::Console::Colors::GREEN, "%s", rowStream.str().c_str());
		}
	}

	Core::Console::ColorPrint(
		Core::Console::Colors::WHITE, "\nPress Enter to start the test...\n");

	getchar();
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "Starting...");
	///////////////////////////////////////

	// Init the dispatchers
	
	{
		RedDispatcher distaptcherB(dataset);
		{
			GreenDispatcher distaptcherA(dataset);


			// Start the dispatchers
			distaptcherA.Start();
			distaptcherB.Start();

			// Wait for any key to exit
			getchar();

			distaptcherA.Stop();
			distaptcherB.Stop();
		}
		getchar();
		

	}
	
	
}

int main(int argc, const char* argv[])
{
	SimpleSample(true,true);
	
	return 0;
}
