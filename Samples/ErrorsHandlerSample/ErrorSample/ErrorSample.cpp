#include <utils/scope_guard.hpp>
#include <Core.hpp>
#include <Factories.hpp>
#include <Monitor.hpp>
#include <Strings.hpp>
#include <Files.hpp>
#include <utils/ref_count_ptr.hpp>
#include <iostream>
#include <Monitor.hpp>
#include <Rules.hpp>
#include <ErrorSample/RulesDB.h>
#include <XMLLoader.hpp>
#include <ErrorsManagement.hpp>
#include <ErrorSample/ProjectCommon.h>
#include <ErrorSample/ParamStore.h>
#include <Common.h>
class ErrorSampleBuilder : public Application::Builder
{
public:
	ErrorSampleBuilder() :
		Builder()
	{

	}


	using dataset_map = std::map<std::string, Database::DataSet>;

private:
	Database::DataSet m_dataset;
	Application::Runnable m_runnableMonitor;
	Rules::RulesManager m_runnableRule;
	Application::MainApp m_mainApp;
	Files::FilesHandler m_handler;
	Common::CommonTypes::MonitorParams m_debugEnv;
	std::string ExtractPath(const char* fullPath)
	{
		std::string path(fullPath);
		size_t pos1 = path.rfind("/");
		size_t pos2 = path.rfind("\\");
		if (pos1 == std::string::npos)
			pos1 = 0;

		if (pos2 == std::string::npos)
			pos2 = 0;

		size_t pos = pos2 > pos1 ? pos2 : pos1;

		path = path.substr(0, pos);
		return path;
	}
protected:
	virtual void BuildEnvironment() override
	{
		m_dataset = Database::MemoryDatabase::Create("MyDataBase");
		
		//Try to initialize the database from the dataset xml file
		std::string dbSchamPath = m_mainApp.DataBasePath() + m_mainApp.AppName() + "DataSet.xml";
		if (false == Database::Schema::Load(m_dataset, dbSchamPath))
		{
			Core::Console::ColorPrint(Core::Console::Colors::RED, "Failed to Build Environment from DataSet\n");
			throw std::runtime_error("error loading Data Set");
		}

		m_runnableRule = Rules::RulesBuilder::Create("", m_dataset[ProjectCommon::MonitorDBIndex::RulesInputDBEnum],
			m_dataset[ProjectCommon::MonitorDBIndex::RulesEnabledDBEnum],
			m_dataset[ProjectCommon::MonitorDBIndex::RulesExistenceDBEnum],
			m_dataset[ProjectCommon::MonitorDBIndex::RulesOutputDBEnum],
			m_dataset[ProjectCommon::MonitorDBIndex::RulesManagementDBEnum],
			Parsers::BinaryMetadataStore());

		AddRunnable(m_runnableRule);

		std::string errorPath = m_mainApp.DataBasePath() + "ErrorStore.xml";
		AddRunnable<Errors::ErrorsManager>(m_dataset, errorPath.c_str(), m_runnableRule, m_mainApp.Verbose());
		//create the monitor listener
		ReadParamsStore();
		AddRunnable<Database::Monitor>(m_debugEnv.remoteAddress, m_debugEnv.remotePort, m_debugEnv.localAddress, m_debugEnv.localPort, m_dataset);

	}

	virtual void BuildDispatchers() override
	{

	}

	void ReadParamsStore()
	{
		std::string  xmlFactorySetting = "";
		std::string  xmlDeveloperSetting = "";
		std::string  xmlUserSetting = "";
		std::string  INIFilePath = "";
		m_handler = Files::FilesHandler::Create();

		//Check if files are already red by MainApp
		xmlFactorySetting = m_mainApp.FactorySettings();
		xmlDeveloperSetting = m_mainApp.DeveloperSettings();
		xmlUserSetting = m_mainApp.UserSettings();

		//LOG_INFO(APP_LOGGER) << "Create XmlStoreDB";
		AddRunnable<ConfigurationLoader::XmlStoreDB>(ProjectCommon::MonitorDBIndex::XML_StoreDBEnum,
			m_dataset,
			xmlFactorySetting,
			xmlDeveloperSetting,
			xmlUserSetting,
			m_handler,
			m_mainApp.AppName(),
			m_mainApp.AppGroupName());


		m_dataset[ProjectCommon::MonitorDBIndex::XML_StoreDBEnum][ParamStore::XML_StoreDBEnum::DEBUG_ENV].Read<Common::CommonTypes::MonitorParams>(m_debugEnv);
	}
public:
	virtual ~ErrorSampleBuilder()
	{

	}


};
int main(int argc, const char* argv[])
{

	try
	{
		Application::MainApp mainApp(argc, argv);
		return mainApp.BuildAndRun<ErrorSampleBuilder>();
	}
	catch (std::exception &e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	return 0;
}
