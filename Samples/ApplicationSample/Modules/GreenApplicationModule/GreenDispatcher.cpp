#include "GreenDispatcher.h"
#include <utils/ref_count_ptr.hpp>
#include <utils/random.hpp>

#include <Core.hpp>
#include <Factories.hpp>

#include <string>
#include <sstream>
#include <iostream>

Logging::Logger Dispatchers::GreenDispatcher::LOGGER = Core::Framework::CreateLogger("Green Dispatcher", Logging::Severity::TRACE);

void Dispatchers::GreenDispatcher::HandleInt(int data)
{
	std::stringstream stream;
	stream << "Got data from Row 2 (int): " << data;
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

	//only create the row if it was not already define, this is done in order to support both offline loader or code loader. 
	// in a real application, one should choose the proper method to do so (offline or code)
	Database::Row row;
	if (m_table.TryGet(ApplicationSampleDB::GreenRedTable::ROW_1, row) == false)
	{
		//Here the dispatcher can add its relevant rows
		m_table.AddRow<ApplicationSampleDB::MyData>(ApplicationSampleDB::GreenRedTable::ROW_1);
	}
}

void Dispatchers::GreenDispatcher::Start()
{
	LOG_FUNC(LOGGER);

	Stop();
	// Subscribe to Row 2 in the data-table
	int val = 0;
	m_table[ApplicationSampleDB::GreenRedTable::ROW_2].Write<int>(19);
	m_rowToken = SubGet(m_table[ApplicationSampleDB::GreenRedTable::ROW_2], [this](const Database::RowData& data)
	{// Here is the event handler handling data update from Row 2
		LOG_SCOPE(LOGGER, "DB data event");
		int intData = data.Read<int>();
		HandleInt(intData);
	}, val);

	Core::Console::ColorPrint(Core::Console::Colors::GREEN,"Subget of GreenRedTable::ROW_2 = %d",val );

	// Start publishing to Row 1 periodically 
	m_timerToken = RegisterTimer(50, [this]()
	{
		LOG_SCOPE(LOGGER, "Timer event");

		LOG_INFO(LOGGER) << "Publishing data to Row 1";
		ApplicationSampleDB::MyInternalData internalData;
		internalData.val1 = utils::random_int(100);
		int size = static_cast<int>(sizeof(internalData.MyArray) / sizeof(ApplicationSampleDB::MyArrayStruct));
		for (auto i = 0; i < size; i++)
		{
			internalData.MyArray[i].options = static_cast<ApplicationSampleDB::MyEnum>(utils::random_int(2));
			internalData.MyArray[i].bit = static_cast<uint8_t>(utils::random_int(13) & 0xf);
			internalData.MyArray[i].bit1 = static_cast<uint8_t>(utils::random_int(10) & 0xf);
		}

		internalData.oneMoreStruct.options = static_cast<ApplicationSampleDB::MyEnum>(utils::random_int(2));
		internalData.oneMoreStruct.bit = static_cast<uint8_t>(utils::random_int(13) & 0xf);
		internalData.oneMoreStruct.bit1 = static_cast<uint8_t>(utils::random_int(10) & 0xf);
		m_table[ApplicationSampleDB::GreenRedTable::ROW_1].Write<ApplicationSampleDB::MyData>({ utils::random_int(100), utils::random_int(100), utils::random_float(100), utils::random_float(100),{ApplicationSampleDB::MyEnum::OPTION1,ApplicationSampleDB::MyEnum::OPTION1,ApplicationSampleDB::MyEnum::OPTION2},internalData });
		m_table[ApplicationSampleDB::GreenRedTable::ROW_3].Write<int>(utils::random_int(1));

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
	if (m_rowToken != nullptr)
	{
		LOG_DEBUG(LOGGER) << "Unregistering database's row";
		m_rowToken = nullptr;
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
