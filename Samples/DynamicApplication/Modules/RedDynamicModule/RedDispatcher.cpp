#include "RedDispatcher.h"
#include <utils/ref_count_ptr.hpp>
#include <utils/random.hpp>

#include <Factories.hpp>

#include <string>
#include <sstream>
#include <iostream>

Logging::Logger Dispatchers::RedDispatcher::LOGGER = Core::Framework::CreateLogger("Red Dispatcher", Logging::Severity::TRACE);

void Dispatchers::RedDispatcher::HandleMyData(const char* data)
{
	std::stringstream stream;
	stream << "Got data from Row 1: " << data;
	LOG_INFO(LOGGER) << stream.str().c_str();

	std::cout << stream.str().c_str() << "\n";
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
}

void Dispatchers::RedDispatcher::Start()
{
	LOG_FUNC(LOGGER);
	// Subscribe to Row 1 in the data-table
	m_rowTokens += Subscribe(m_table.GetRowByName("ROW_4"), [this](const Database::RowData& data)
	{// Here is the event handler handling data update from Row 4
		LOG_SCOPE(LOGGER, "DB data event");
		Core::Console::ColorPrint(Core::Console::Colors::RED, "Update row 4\n");
		m_table.GetRowByName("ROW_2").Write<int>(utils::random_int(100));

	});

	m_rowTokens += Subscribe(m_table.GetRowByName("ROW_1"), [this](const Database::RowData& data)
	{// Here is the event handler handling data update from Row 1
		LOG_SCOPE(LOGGER, "DB data event");
		std::string jsonDtr;
		data.TryGetJson(jsonDtr, Parsers::JsonDetailsLevel::JSON_ENUM_FULL, false);
		HandleMyData(jsonDtr.c_str());
	});

	m_timerToken = Context().RegisterTimer(200, [this]()
	{
		LOG_SCOPE(LOGGER, "Timer event");

		LOG_INFO(LOGGER) << "Publishing data to Row 2";
		m_table.GetRowByName("ROW_3").Write();
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
	if (m_rowTokens != nullptr)
	{
		LOG_DEBUG(LOGGER) << "Unregistering database's row";
		m_rowTokens = nullptr;
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
