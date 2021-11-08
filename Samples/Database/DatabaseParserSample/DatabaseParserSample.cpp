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
	TABLE_1
};

enum Table1Rows
{
	ROW_1,    //t:MyData
	ROW_2     //t:int min:6 max:10 def:4
};
#pragma pack(1)
struct MyData
{
public:
	int val1;  //min:2 max:5 def:3
	int val2;  //min:5 max:10
	float val3; //min:0.5 
	double val4; 
};
#pragma pack()
void CheckData(const Parsers::BinaryParser& parser)
{
	//Find the wrong field
	for (size_t i = 0; i < parser.Count(); i++)
	{
		if (parser.Validate(i) == false)
		{
			Core::Console::ColorPrint(Core::Console::Colors::CYAN, "Data Out of Range for field %s\n", parser.FieldName(i));
		}
	}
}
class GreenDispatcher : public Dispatcher
{
	DataSet m_dataset;
	SubscriptionToken m_token;
	Utils::Timer m_timer;

private:
	void HandleIntData(int data,const Parsers::BinaryParser& parser)
	{
		CheckData(parser);
		Core::Console::ColorPrint(
			Core::Console::Colors::GREEN,
			"Got data from Row 2 (int): %d\n",
			data);
	}

public:
	GreenDispatcher(DataSet dataset) :
		Dispatcher(),
		m_dataset(dataset)
	{
		m_token = Subscribe(dataset[Tables::TABLE_1][Table1Rows::ROW_2], [=](const RowData& data)
		{
			int myData = data.Read<int>();
			Parsers::BinaryParser parser = dataset[Tables::TABLE_1][Table1Rows::ROW_2].ParserMetadata().CreateParser();

			parser.Parse(&myData, sizeof(int));
			HandleIntData(myData,parser);
		});

		m_timer.Elapsed() += [this]()
		{
			MyData data;
			
		
			data.val4 = 304.221;
			data.val1 = RandomInt(5);
			data.val2 = RandomInt(10);
			data.val3 = RandomFloat(3);
			if (m_dataset[Tables::TABLE_1][Table1Rows::ROW_1].CheckAndWrite<MyData>(data) == false)
			{
				Core::Console::ColorPrint(Core::Console::Colors::CYAN, "Failed to write MyData\n");
				//Make a write and let the reader to indicate a out of range (if out of range)
				m_dataset[Tables::TABLE_1][Table1Rows::ROW_1].Write<MyData>(data);
			}

		};
	}

	~GreenDispatcher()
	{
		Unsubscribe(m_dataset, Tables::TABLE_1, Table1Rows::ROW_2, m_token);
	}

	virtual void Start() override
	{
		m_timer.Start(5000);
	}
};

class RedDispatcher : public Dispatcher
{
	DataSet m_dataset;
	SubscriptionToken m_token;
	Utils::Timer m_timer;
	MyData m_data;

private:
	

	void HandleMyData(const MyData& data, const Parsers::BinaryParser& parser)
	{
		if (parser.Validate() == false)
		{
			CheckData(parser);
		}
		else
			Core::Console::ColorPrint(Core::Console::Colors::CYAN, "Data in Range\n");

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
		m_token = Subscribe(dataset, Tables::TABLE_1, Table1Rows::ROW_1, [this](const RowData& data)
		{
			data.Read<MyData>(m_data);
			Parsers::BinaryParser parser = m_dataset[Tables::TABLE_1][Table1Rows::ROW_1].ParserMetadata().CreateParser();

			parser.Parse(&m_data, sizeof(MyData));
			HandleMyData(m_data, parser);
		});

		m_timer.Elapsed() += [this]()
		{
			for (int i = 5; i <= 11; i++)
			{
				if (m_dataset[TABLE_1][ROW_2].CheckAndWrite<int>(i) == false)
				{
					Core::Console::ColorPrint(Core::Console::Colors::RED, "Data %d, written out range\n", i);
					//Write it and allow the read to check
					m_dataset[TABLE_1][ROW_2].Write<int>(i);
				}
				
				
			}
		};
	}

	~RedDispatcher()
	{
		Unsubscribe(m_dataset, Tables::TABLE_1, Table1Rows::ROW_1, m_token);
	}

	virtual void Start() override
	{
		m_timer.Start(5000);
	}
};

void ParserSimpleSample(bool enableGreen, bool enableRed)
{
	DataSet dataset = MemoryDatabase::Create("My Database");
	//DataSet dataset = DDSDatabase::Create("My Database");
	dataset.AddTable(Tables::TABLE_1);
	Parsers::BinaryMetaDataBuilder parserMetadata = Parsers::BinaryMetaDataBuilder::Create();
	parserMetadata.Namely("MyData");
	Parsers::SimpleOptions options =  Parsers::SimpleOptions::create<int>(2, 5, 3);
	parserMetadata.Simple<int>("val1", options);

	options = Parsers::SimpleOptions();
	options.minval<int>(5);
	options.minval<int>(10);
	parserMetadata.Simple<int>("val2");
	options.reset();
	options.minval<float>(0.5);
	parserMetadata.Simple<float>("val3");
	parserMetadata.Simple<double>("val4");
	Database::RowInfo infor1 = { TypeEnum::COMPLEX,"ROW_1","" };
	dataset[Tables::TABLE_1].AddRow<MyData>(Table1Rows::ROW_1,infor1,parserMetadata);
	Parsers::BinaryMetaDataBuilder intMetadata = Parsers::BinaryMetaDataBuilder::Create();
	options.reset();
	options = Parsers::SimpleOptions::create<int>(6,10,7);
	intMetadata.Simple<int>("ROW_2",options);
	Database::RowInfo infor2 = { TypeEnum::INT32,"ROW_2","" };
	dataset[Tables::TABLE_1].AddRow<int>(Table1Rows::ROW_2,infor2,intMetadata);

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
		Core::Console::Colors::WHITE, "\nstarting the test...\n");

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
	ParserSimpleSample(true, true);
	
}
