#include "RedDispatcher.h"
#include <utils/ref_count_ptr.hpp>
#include <utils/random.hpp>

#include <Factories.hpp>

#include <string>
#include <sstream>
#include <iostream>

Logging::Logger Dispatchers::RedDispatcher::LOGGER = Core::Framework::CreateLogger("Red Dispatcher", Logging::Severity::TRACE);

void Dispatchers::RedDispatcher::HandleMyData(const ApplicationSampleDB::MyData& data)
{
	std::stringstream stream;
	stream << "Got data from Row 1 (MyData): {" << data.val1 << ", " << data.val2 << ", " << data.val3 << ", " << data.val4 << "}";
	LOG_INFO(LOGGER) << stream.str().c_str();

	Core::Console::ColorPrint(Core::Console::Colors::RED, "%s\n", stream.str().c_str());
}

Dispatchers::RedDispatcher::RedDispatcher(const Database::Table& table) :
	Database::DispatcherBase<dispatchers::red_dispatcher>("Red Dispatcher", true),
	m_table(table)
{
	LOG_FUNC(LOGGER);
}

Dispatchers::RedDispatcher::~RedDispatcher()
{
	LOG_FUNC(LOGGER);
	Stop();
}

void Dispatchers::RedDispatcher::Init()
{
	LOG_FUNC(LOGGER);
	//only create the row if it was not already define, this is done in order to support both offline loader or code loader. 
	// in a real application, one should choose the proper method to do so (offline or code)
	Database::Row row;
	if (m_table.TryGet(ApplicationSampleDB::GreenRedTable::ROW_2,row) == false)
	{
		//Here the dispatcher can add its relevant rows
		m_table.AddRow<int>(ApplicationSampleDB::GreenRedTable::ROW_2);
	}
}

void Dispatchers::RedDispatcher::Start()
{
	LOG_FUNC(LOGGER);

	Stop();	

	// Subscribe to Row 1 in the data-table
	m_rowToken = Subscribe(m_table[ApplicationSampleDB::GreenRedTable::ROW_1], [this](const Database::RowData& data)
	{// Here is the event handler handling data update from Row 2
		LOG_SCOPE(LOGGER, "DB data event");
		ApplicationSampleDB::MyData myData = data.Read<ApplicationSampleDB::MyData>();
		HandleMyData(myData);
	});

	m_timerToken = Context().RegisterTimer(200, [this]()
	{
		LOG_SCOPE(LOGGER, "Timer event");

		LOG_INFO(LOGGER) << "Publishing data to Row 2";
		m_table[ApplicationSampleDB::GreenRedTable::ROW_2].Write<int>(utils::random_int(100));
	});			
}

void Dispatchers::RedDispatcher::Stop()
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

bool dispatchers::red_dispatcher::create(core::database::table_interface* table, core::application::runnable_interface** runnable)
{
	if (table == nullptr)
		return false;

	if (runnable == nullptr)
		return false;

	utils::ref_count_ptr< core::application::runnable_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<Dispatchers::RedDispatcher>(table);
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
