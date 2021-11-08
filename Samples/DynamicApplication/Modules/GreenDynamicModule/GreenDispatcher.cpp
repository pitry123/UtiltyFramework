#include "GreenDispatcher.h"
#include <utils/ref_count_ptr.hpp>
#include <utils/random.hpp>

#include <Core.hpp>
#include <Factories.hpp>

#include <string>
#include <sstream>
#include <iostream>

Logging::Logger Dispatchers::GreenDispatcher::LOGGER = Core::Framework::CreateLogger("Green Dispatcher", Logging::Severity::TRACE);

void Dispatchers::GreenDispatcher::HandleData(const char* data)
{
	std::stringstream stream;
	stream << "Got data from Row 2 :\n " << data;
	LOG_INFO(LOGGER) << stream.str().c_str();

	Core::Console::ColorPrint(Core::Console::Colors::GREEN, "%s\n", stream.str().c_str());
}

Dispatchers::GreenDispatcher::GreenDispatcher(core::database::table_interface* table) :
	Database::DispatcherBase<dispatchers::green_dispatcher>("Green Dispatcher", true),
	m_table(table)
{
	LOG_FUNC(LOGGER);
}

Dispatchers::GreenDispatcher::~GreenDispatcher()
{
	LOG_FUNC(LOGGER);
	Stop();
}

void Dispatchers::GreenDispatcher::Init()
{
	LOG_FUNC(LOGGER);
}

void Dispatchers::GreenDispatcher::Start()
{
	LOG_FUNC(LOGGER);
	// Subscribe to Row 2 in the data-table
	m_rowTokens += Subscribe(m_table.GetRowByName("ROW_3"), [&](const Database::RowData& data)
	{// Here is the event handler handling data update from Row 3
		LOG_SCOPE(LOGGER, "DB data event");
		ApplicationSampleDB::MyInternalData internalData;
		internalData.val1[0] = utils::random_int(100);
		internalData.val1[1] = utils::random_int(100);

		

		int size = static_cast<int>(sizeof(internalData.MyArray) / sizeof(ApplicationSampleDB::MyArrayStruct));
		for (auto i = 0; i < size; i++)
		{
			internalData.MyArray[i].options = static_cast<ApplicationSampleDB::MyEnum>(utils::random_int(2));
		}

		Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Update row 3\n");
		internalData.oneMoreStruct.options = static_cast<ApplicationSampleDB::MyEnum>(utils::random_int(2));
		ApplicationSampleDB::MyData myData;
		myData.val1 = utils::random_int(100);
		myData.val2 = utils::random_int(100);
		myData.val3 = utils::random_float(100);
		myData.val4 = utils::random_float(100);
		myData.option = static_cast<ApplicationSampleDB::MyEnum>(utils::random_int(2));
		myData.structData = internalData;
		STRCPY(myData.str, sizeof(myData.str), "TEST");
		m_table.GetRowByName("ROW_1").Write<ApplicationSampleDB::MyData>(myData);
	});

	m_rowTokens += Subscribe(m_table.GetRowByName("ROW_2"), [&](const Database::RowData& data)
	{// Here is the event handler handling data update from Row 2
		LOG_SCOPE(LOGGER, "DB data event");
		
		std::string jsonStr;
		data.TryGetJson(jsonStr,Parsers::JsonDetailsLevel::JSON_ENUM_FULL);
		HandleData(jsonStr.c_str());
		
	});
	// Start publishing to Row 1 periodically 
	m_timerToken = RegisterTimer(50, [this]()
	{
		LOG_SCOPE(LOGGER, "Timer event");

		LOG_INFO(LOGGER) << "Publishing data to Row 4";

		//internalData.oneMoreStruct.options = static_cast<ApplicationSampleDB::MyEnum>(utils::random_int(2));
		m_table.GetRowByName("ROW_4").Write();
	});
}

void Dispatchers::GreenDispatcher::Stop()
{
	LOG_FUNC(LOGGER);	

	// Stop the timer
	if (m_timerToken != nullptr)
	{
		LOG_DEBUG(LOGGER) << "Stop publish timer";
		m_timerToken = nullptr;
	}

	// Unsubscribe from data
	if (m_rowTokens != nullptr)
	{
		LOG_DEBUG(LOGGER) << "Unregistering database's row";
		m_rowTokens.Clear();
	}
}

bool dispatchers::green_dispatcher::create(core::database::table_interface* table, core::application::runnable_interface** runnable)
{
	if (table == nullptr)
		return false;

	if (runnable == nullptr)
		return false;

	utils::ref_count_ptr< core::application::runnable_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<Dispatchers::GreenDispatcher>(table);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	instance->add_ref();
	*runnable = instance;
	return true;
}
