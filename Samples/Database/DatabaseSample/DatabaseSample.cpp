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

void ThreadSpinning()
{
	for (int i = 0; i < 32; i++)
	{
		std::thread thread([]()
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		});

		thread.join();
	}

	std::vector<std::future<void>> handles;
	for (int i = 0; i < 32; i++)
	{
		handles.emplace_back(std::async(std::launch::async, []() mutable
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}));
	}

	for (auto& handle : handles)
	{
		handle.wait();
	}
}

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
	TABLE_1,
	TABLE_2
};

enum Table1Rows
{
	TABLE1_ROW_1,
	TABLE1_ROW_2
};

enum Table2Rows
{
	TABLE2_ROW_1,
	TABLE2_ROW_2
};

struct MyData
{
public:
	int val1;
	int val2;
	float val3;
	double val4;
};

void StressSample()
{
	// Init the database
	DataSet dataset = MemoryDatabase::Create("My Database");
	dataset.AddTable("Table 1");
	dataset["Table 1"].AddRow<MyData>("Row 1");
	dataset["Table 1"].AddRow<int>("Row 2");

	std::vector<std::future<void>> handles;
	for (int i = 0; i < 4; i++)
	{
		handles.emplace_back(std::async(std::launch::async, [dataset]() mutable
		{
			auto func = [](const RowData& data) -> void
			{
				if (data.Key().Equals("Row 1") == true)
				{
					auto row1_val = data.Read<MyData>();
					//printf("%s changed. Value is: {%d, %d, %f, %lf}\n", data.name(), row1_val.val1, row1_val.val2, row1_val.val3, row1_val.val4);
					(void)row1_val;
				}
				else if (data.Key().Equals("Row 2") == true)
				{
					auto row2_val = data.Read<int>();
					//printf("%s changed. Value is: {%d}\n", data.name(), row2_val);
					(void)row2_val;
				}
				else
				{
					throw std::runtime_error("Unknown row!");
				}
			};

			Subscriber dispatcher;
			auto token1 = dispatcher.Subscribe(dataset["Table 1"]["Row 1"], func);
			utils::scope_guard unsubscribe1([&]()
			{
				dispatcher.Unsubscribe(dataset["Table 1"]["Row 1"], token1);
			});

			auto token2 = dispatcher.Subscribe(dataset["Table 1"]["Row 2"], func);
			utils::scope_guard unsubscribe2([&]()
			{
				dispatcher.Unsubscribe(dataset["Table 1"]["Row 2"], token2);
			});

			for (int i = 0; i < 10000; i++)
			{
				dataset["Table 1"]["Row 1"].Write<MyData>({ i + 1, i + 2, static_cast<float>(i + 3), static_cast<double>(i + 4) });
				dataset["Table 1"]["Row 2"].Write<int>(i + 1);

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}));
	}

	for (auto& handle : handles)
	{
		handle.wait();
	}
}

class GreenDispatcher : public Dispatcher
{
	DataSet m_dataset;
	SubscriptionToken m_token;
	Utils::Timer m_timer;
	Database::DataBinder m_binder;

private:
	void HandleIntData(int data, Tables table)
	{
		Core::Console::ColorPrint(
			Core::Console::Colors::GREEN,
			"Got data from Table %d, Row 2 (int): %d\n",
			table+1,
			data);
	}

public:
	GreenDispatcher(DataSet dataset) :
		Dispatcher(),
		m_dataset(dataset)
	{
		m_token = Subscribe(dataset[Tables::TABLE_1][Table1Rows::TABLE1_ROW_2], [this](const RowData& data)
		{
			int myData = data.Read<int>();
			HandleIntData(myData, Tables::TABLE_1);
		});

		m_token = Subscribe(dataset[Tables::TABLE_2][Table2Rows::TABLE2_ROW_2], [this](const RowData& data)
		{
			int myData = data.Read<int>();
			HandleIntData(myData, Tables::TABLE_2);
		});

		m_timer.Elapsed() += [this]()
		{
			m_dataset[Tables::TABLE_1][Table1Rows::TABLE1_ROW_1].Write<MyData>({ RandomInt(100), RandomInt(100), RandomFloat(100.0f), RandomFloat(100.0f) });
		};

		m_binder.Bind(dataset[Tables::TABLE_1], dataset[Tables::TABLE_2], true, false);
	}

	~GreenDispatcher()
	{
		Unsubscribe(m_dataset, Tables::TABLE_1, Table1Rows::TABLE1_ROW_2, m_token);
	}

	virtual void Start() override
	{
		m_timer.Start(50);
	}
};

class RedDispatcher : public Dispatcher
{
	DataSet m_dataset;
	SubscriptionToken m_token;
	Utils::Timer m_timer;
	MyData m_data;

private:
	void HandleMyData(const MyData& data, Tables table)
	{
		Core::Console::ColorPrint(
			Core::Console::Colors::RED,
			"Got data from Table %d Row 1 (MyData): {%d, %d, %f, %lf}\n",
			table + 1,
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
		m_token = Subscribe(dataset, Tables::TABLE_1, Table1Rows::TABLE1_ROW_1, [this](const RowData& data)
		{
			data.Read<MyData>(m_data);
			HandleMyData(m_data, TABLE_1);
		});

		m_token = Subscribe(dataset, Tables::TABLE_2, Table2Rows::TABLE2_ROW_1, [this](const RowData& data)
		{
			data.Read<MyData>(m_data);
			HandleMyData(m_data, TABLE_2);
		});

		m_timer.Elapsed() += [this]()
		{
			m_dataset[Tables::TABLE_1][Table1Rows::TABLE1_ROW_2].Write<int>(RandomInt(100));
		};
	}

	~RedDispatcher()
	{
		Unsubscribe(m_dataset, Tables::TABLE_1, Table1Rows::TABLE1_ROW_1, m_token);
	}

	virtual void Start() override
	{
		m_timer.Start(200);
	}
};

void SimpleSample(bool enableGreen, bool enableRed)
{
	DataSet dataset = MemoryDatabase::Create("My Database");

	dataset.AddTable(Tables::TABLE_1);
	dataset[Tables::TABLE_1].AddRow<MyData>(Table1Rows::TABLE1_ROW_1);
	dataset[Tables::TABLE_1].AddRow<int>(Table1Rows::TABLE1_ROW_2);
	
	dataset.AddTable(Tables::TABLE_2);
	dataset[Tables::TABLE_2].AddRow<MyData>(Table2Rows::TABLE2_ROW_1);
	dataset[Tables::TABLE_2].AddRow<int>(Table2Rows::TABLE2_ROW_2);

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

	///////////////////////////////////////

	// Init the dispatchers
	if (enableGreen == true && enableRed == true)
	{
		GreenDispatcher distaptcherA(dataset);
		RedDispatcher distaptcherB(dataset);

		// Start the dispatchers
		distaptcherA.Start();
		distaptcherB.Start();

		// Wait for any key to exit
		getchar();
	}
	else if (enableGreen == true)
	{
		GreenDispatcher distaptcherA(dataset);

		// Start the dispatchers
		distaptcherA.Start();

		// Wait for any key to exit
		getchar();
	}
	else if (enableRed == true)
	{
		RedDispatcher distaptcherB(dataset);

		// Start the dispatchers
		distaptcherB.Start();

		// Wait for any key to exit
		getchar();
	}
}

int main(int argc, const char* argv[])
{
	//StressSample();
	if (argc == 1)
		SimpleSample(true, true);
	else
	{
		if (std::strcmp(argv[1], "green") == 0)
			SimpleSample(true, false);
		else if (std::strcmp(argv[1], "red") == 0)
			SimpleSample(false, true);
		else
			SimpleSample(true, true);

	}
	return 0;
}
