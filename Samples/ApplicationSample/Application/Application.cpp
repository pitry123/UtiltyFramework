#include <Factories.hpp>
#include <Monitor.hpp>
#include <Dispatchers.hpp>
#include <INILoader.hpp>
#include <XMLLoader.hpp>
#include <Strings.hpp>
#include <ApplicationSample/ApplicationSampleDB.h>
#include <ApplicationSample/ProjectCommon.h>
#include <ApplicationSample/ParamStore.h>
#include <utils/common_converter.hpp>
#include <sstream>
#include <limits>

using namespace Logging;

class MonitorLogger : public Database::Dispatcher
{
private:
	Database::LoggerUnits m_loggers;
	Utils::AutoTimerToken m_token;

public:
	MonitorLogger(
		const Database::Table& configurationTable,
		const Database::Table& reportingTable) :
		m_loggers(
			configurationTable,
			reportingTable,
			DBLoggerCommon::LogsDBEnum::LOGGING_DATA_DB_SIZE,
			Core::Framework::CreateLogger("Monitor Logger", Severity::TRACE))
	{
	}

	virtual void Start() override
	{
		Stop();

		m_token = RegisterTimer(2000, [&]()
		{
			static int COUNTER = 0;
			if (COUNTER == (std::numeric_limits<int>::max)())
				COUNTER = 0;

			int counter = COUNTER++;
			int index = ((counter % 2) != 0) ?
				DBLoggerCommon::LogsDBEnum::LOG_UNIT_1 :
				DBLoggerCommon::LogsDBEnum::LOG_UNIT_2;

			LOG_TRACE	(m_loggers[index]) << "This is a periodic DB logger update: " << counter;
			LOG_DEBUG	(m_loggers[index]) << "This is a periodic DB logger update: " << counter;
			LOG_INFO	(m_loggers[index]) << "This is a periodic DB logger update: " << counter;
			LOG_WARNING	(m_loggers[index]) << "This is a periodic DB logger update: " << counter;
			LOG_ERROR	(m_loggers[index]) << "This is a periodic DB logger update: " << counter;
			LOG_FATAL	(m_loggers[index]) << "This is a periodic DB logger update: " << counter;
		});
	}

	virtual void Stop() override
	{
		m_token = nullptr;
	}
};

/// An application builder.
///  This is a sample app and therefore cover set of options using ezFramework libraries
/// if executed as is, it will start and run with hard coded data
/// if one wants to used the command line parameters run the application with the following options in the command line
/// -f path to configuration file for factory configuration files
/// -u optional - path to configuration file for user configuration data (that override the factory file)
/// -s path to the DataSet XML generated by DebugEnvironment this will create the database table and row
/// automatically using the offline builder functions
/// @date	23/11/2018
class ApplicationBuilder : public Application::Builder
{
public:
	ApplicationBuilder() :
		Builder()
	{
	}

private:
	Files::FilesHandler m_handler;
	Application::MainApp m_mainApp;
	Database::DataSet m_dataset;	
	Common::CommonTypes::MonitorParams m_debugEnv;
	static Logger APP_LOGGER;

protected:

	virtual void BuildEnvironment() override
	{
		m_dataset = Database::MemoryDatabase::Create("MyDataBase");	
		uint16_t week = 0;
		uint32_t milli = 0;
		common_converter::unit_convertion::T_DateTimeType t_date_time_type = {};
		common_converter::unit_convertion::GPS_time_to_DDS_time(week, milli, t_date_time_type);
		//Try to initialize the database from the dataset xml file
		std::string dbSchamPath = m_mainApp.DataBasePath() + m_mainApp.AppName() + "DataSet.xml";
		if (false == Database::Schema::Load(m_dataset, dbSchamPath))
		{
			Core::Console::ColorPrint(Core::Console::Colors::RED,"Failed to Build Environment from Data Set Build Normally\n");						

			// Initialize the application database hard coded
			m_dataset.AddTable(ProjectCommon::MonitorDBIndex::LogsDBEnum);
			m_dataset.AddTable(ProjectCommon::MonitorDBIndex::LoggerConfigDBEnum);

			m_dataset.AddTable(ProjectCommon::MonitorDBIndex::GreenRedTable);
			LOG_INFO(APP_LOGGER) << "Build XML Loader DB";

			m_dataset.AddTable(ProjectCommon::MonitorDBIndex::XML_StoreDBEnum);
			LOG_INFO(APP_LOGGER) << "Build INI Loader DB";
		}
		else
		{
			Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Successfully Build Environment from XML Data Set \n");
		}

		AddRunnable<MonitorLogger>(
			m_dataset[ProjectCommon::MonitorDBIndex::LoggerConfigDBEnum],
			m_dataset[ProjectCommon::MonitorDBIndex::LogsDBEnum]);
		try
		{
			//this will Work only if factory/user path is a valid path with conf.xml file
			ReadParamsStore();
		}
		catch (...)
		{
			//use default
			Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "configuration files not found, using default\n");

			throw std::runtime_error("ReadParamsStore");
		}

		std::stringstream os;
		os << "Build Monitor remote address " << m_debugEnv.remoteAddress << ":" << m_debugEnv.remotePort << " local address " << m_debugEnv.localAddress << ":" << m_debugEnv.localPort;
		LOG_INFO(APP_LOGGER) << os.str().c_str();
		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "%s\n", os.str().c_str());

		//create the monitor listener
		AddRunnable<Database::Monitor>(m_debugEnv.remoteAddress, m_debugEnv.remotePort, m_debugEnv.localAddress, m_debugEnv.localPort, m_dataset);		
	}
	virtual void BuildDispatchers() override
	{

		AddRunnable(Dispatchers::GreenDispatcher::Create(m_dataset[ProjectCommon::MonitorDBIndex::GreenRedTable]));
		AddRunnable(Dispatchers::RedDispatcher::Create(m_dataset[ProjectCommon::MonitorDBIndex::GreenRedTable]));
	}

	bool SetLoggerSevirity(const char* logName, const char* logSevirity)
	{
		Logger log;
		Severity sevirity;
		log = m_mainApp.Log(logName);
		if (false == log.Empty())
		{
			if (parse(logSevirity, sevirity))
			{
				log.Filter(sevirity);
				return true;
			}
		}

		return false;
	}

	bool SetAllLoggersSevirity(const char* logSevirity)
	{
		Logger log;
		Severity sevirity;
		if (false == parse(logSevirity, sevirity))
			return false;

		size_t size = m_mainApp.LoggrsCount();
		for (size_t i = 0; i < size; i++)
		{
			log = m_mainApp.Log(i);
			log.Filter(sevirity);
		}
		
		return false;
	}

	void ReadParamsStore()
	{
		std::string  xmlFactorySetting = "";
		std::string  xmlDeveloperSetting = "";
		std::string  xmlUserSetting = "";
		std::string  INIFilePath = "";
		m_handler = Files::FilesHandler::Create();
		
		//Only in sample since all samples are created to the same directory in Windows the configuration files need to be seprated. 
		//on stand alone application call `xmlFactorySetting = m_mainApp.FactorySettings();` directly
		xmlFactorySetting = m_mainApp.FactorySettings();
		xmlDeveloperSetting = m_mainApp.DeveloperSettings();
		xmlUserSetting = m_mainApp.UserSettings();

		LOG_INFO(APP_LOGGER) << "Create XmlStoreDB";
		AddRunnable<ConfigurationLoader::XmlStoreDB>(ProjectCommon::MonitorDBIndex::XML_StoreDBEnum,
			m_dataset,
			xmlFactorySetting,
			xmlDeveloperSetting,
			xmlUserSetting,
			m_handler,
			m_mainApp.AppName(),
			"");
		
		ParamStore::LogData logData;
		
		m_dataset[ProjectCommon::MonitorDBIndex::XML_StoreDBEnum][ParamStore::XML_StoreDBEnum::DEBUG_ENV].Read<Common::CommonTypes::MonitorParams>(m_debugEnv);
		m_dataset[ProjectCommon::MonitorDBIndex::XML_StoreDBEnum][ParamStore::XML_StoreDBEnum::GENERAL_LOG].Read<ParamStore::LogData>(logData);
		SetAllLoggersSevirity(logData.logSevirty);
		m_dataset[ProjectCommon::MonitorDBIndex::XML_StoreDBEnum][ParamStore::XML_StoreDBEnum::LOG1].Read<ParamStore::LogData>(logData);
		SetLoggerSevirity(logData.logName, logData.logSevirty);
		m_dataset[ProjectCommon::MonitorDBIndex::XML_StoreDBEnum][ParamStore::XML_StoreDBEnum::LOG2].Read<ParamStore::LogData>(logData);
		SetLoggerSevirity(logData.logName, logData.logSevirty);
		

	}
};

Logger ApplicationBuilder::APP_LOGGER = Core::Framework::CreateLogger("ApplicationBuilder", Severity::TRACE);

int main(int argc, const char* argv[])
{	
	try
	{
		Application::MainApp mainApp(argc, argv);
		
		return mainApp.BuildAndRun<ApplicationBuilder>();
	}
	catch (std::exception &e)
	{
        std::cout << "Error: " << e.what() << std::endl;
	}	
}