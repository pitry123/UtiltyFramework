

#include <Application.hpp>
#include <Factories.hpp>
#include <Monitor.hpp>
#include <INILoader.hpp>
#include <XMLLoader.hpp>

#include <Utils.hpp>


#include <Common.h>
#include <utils/random.hpp>
#include <RemoteAgent.hpp>
#include <RemoteAgentSample/ProjectCommon.h>
#include <RemoteAgentSample/ParamStore.h>
#include <RemoteAgentSample/ApplicationSampleDB.h>
#include <sstream>

class ApplicationBuilder : public Application::Builder
{
public:
	ApplicationBuilder() :
		Builder(),
		m_mainApp()
	{
	}

private:
	Files::FilesHandler m_handler;
	Database::DataSet m_dataset;
	Application::MainApp m_mainApp;
	Common::CommonTypes::MonitorParams m_RemoteAgentDebugEnv;
	Common::CommonTypes::MonitorParams m_RemoteAgentEnv;

	static Logging::Logger LOGGER;
	Database::table_info_map m_tableInfoMap;
	///-------------------------------------------------------------------
	/// @brief	set the initialize data for the TOU tables 
	///			the tables and rows created with the function Database::Schema::Load
	///			this function update the initialize data after building the database
	/// @date	09/01/2019
	///
	/// @param	None
	///
	/// @return	None
	///-------------------------------------------------------------------
	void InitializeTables()
	{
		ApplicationSampleDB::MyInternalData internalData;
		internalData.val1 = 55;
		int size = static_cast<int>(sizeof(internalData.MyArray) / sizeof(ApplicationSampleDB::MyArrayStruct));
		for (auto i = 0; i < size; i++)
		{
			internalData.MyArray[i].options = static_cast<ApplicationSampleDB::MyEnum>(utils::random_int(2));
		}

		internalData.oneMoreStruct.options = static_cast<ApplicationSampleDB::MyEnum>(utils::random_int(2));

		ApplicationSampleDB::MyData data = { utils::random_int(100), utils::random_int(10), utils::random_float(20), utils::random_float(20), internalData };
		m_dataset[ProjectCommon::GreenRedTable2][ApplicationSampleDB::GreenRedTable::ROW_1].Write<ApplicationSampleDB::MyData>(data);
		m_dataset[ProjectCommon::GreenRedTable2][ApplicationSampleDB::GreenRedTable::ROW_2].Write<int>(utils::random_int(100));

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


		AddRunnable<ConfigurationLoader::XmlStoreDB>(ProjectCommon::MonitorDBIndex::XML_StoreDBEnum,
			m_dataset,
			xmlFactorySetting,
			"",
			xmlUserSetting,
			m_handler,
			m_mainApp.AppName(),
			"");

		m_dataset[ProjectCommon::MonitorDBIndex::XML_StoreDBEnum][ParamStore::XML_StoreDBEnum::DEBUG_EN_MEM].Read<Common::CommonTypes::MonitorParams>(m_RemoteAgentDebugEnv);
		m_dataset[ProjectCommon::MonitorDBIndex::XML_StoreDBEnum][ParamStore::XML_StoreDBEnum::REMOTE_AGENT_DEBUG_ENV].Read<Common::CommonTypes::MonitorParams>(m_RemoteAgentEnv);

	}
protected:
	virtual void BuildEnvironment() override
	{
		// Initialize the application database
		m_dataset = Database::MemoryDatabase::Create("My Dataset");


		//Try to initialize the database from the dataset xml file

		if (false == Database::Schema::Load(m_dataset, m_mainApp.DataBasePath() + "/" + m_mainApp.AppName() + "DataSet.xml"))
		{
			Core::Console::ColorPrint(Core::Console::Colors::RED, "Failed to Build Environment from Data Set Build Normally\n");
		}
		else
		{
			Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Successfully Build Environment from XML Data Set \n");
		}

		ReadParamsStore();

		AddRunnable<Database::Monitor>(m_RemoteAgentDebugEnv.remoteAddress,
			m_RemoteAgentDebugEnv.remotePort,
			m_RemoteAgentDebugEnv.localAddress,
			m_RemoteAgentDebugEnv.localPort, m_dataset);


	}

	void AddRemoteAgentTable(int internalTableID, int externalTableID, Database::table_info::exported_or_imported_flag importExportFlag)
	{
		Database::table_info tableInfo;

		tableInfo.externalId = externalTableID;
		tableInfo.exportedOrImportedFlag = importExportFlag;

		m_tableInfoMap.insert(Database::table_info_pair(internalTableID, tableInfo));
	}
	virtual void BuildDispatchers() override
	{
		
		AddRemoteAgentTable(ProjectCommon::MonitorDBIndex::GreenRedTable1, ProjectCommon::MonitorDBIndex::GreenRedTable1, Database::table_info::exported_or_imported_flag::IMPORTED);
		AddRemoteAgentTable(ProjectCommon::MonitorDBIndex::GreenRedTable2, ProjectCommon::MonitorDBIndex::GreenRedTable2, Database::table_info::exported_or_imported_flag::IMPORTED);

		//InitializeTables();

		std::stringstream rad;
		rad << "Build RemoteAgentdebug environment" << m_RemoteAgentDebugEnv.remoteAddress << ":" << m_RemoteAgentDebugEnv.remotePort << " local address " << m_RemoteAgentDebugEnv.localAddress << ":" << m_RemoteAgentDebugEnv.localPort;
		LOG_INFO(LOGGER) << rad.str().c_str();
		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "%s\n", rad.str().c_str());


		std::stringstream ra;
		ra << "Build RemoteAgent remote address " << m_RemoteAgentEnv.remoteAddress << ":" << m_RemoteAgentEnv.remotePort << " local address " << m_RemoteAgentEnv.localAddress << ":" << m_RemoteAgentEnv.localPort;
		LOG_INFO(LOGGER) << ra.str().c_str();
		Core::Console::ColorPrint(Core::Console::Colors::WHITE, "%s\n", ra.str().c_str());

		AddRunnable<Database::RemoteAgent>(m_RemoteAgentEnv.remoteAddress,
			m_RemoteAgentEnv.remotePort,
			m_RemoteAgentEnv.localAddress,
			m_RemoteAgentEnv.localPort,
			m_dataset, m_dataset[ProjectCommon::MonitorDBIndex::RemoteAgentStatusDBEnum], m_tableInfoMap);

	}

};

Logging::Logger ApplicationBuilder::LOGGER = Core::Framework::CreateLogger("ApplicationBuilder", Logging::Severity::DEBUG);
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
