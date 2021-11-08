
#include <core/database.h>
#include <Application.hpp>
#include <Factories.hpp>
#include <Monitor.hpp>
#include <Rules.hpp>
#include "Common/RulesSample/RulesConcrete.hpp"
#include "Common/RulesSample/ProjectCommon.h"

//Should come from a config file in real applications
static constexpr char const* LOCAL_IP = "127.0.0.1";
static constexpr uint16_t	LOCAL_PORT = 23103;
static constexpr char const* REMOTE_IP = "127.0.0.1";
static constexpr uint16_t REMOTE_PORT = 23102;


class RulesSampleBuilder : public Application::Builder
{
public:
	RulesSampleBuilder()
	{}

	~RulesSampleBuilder()
	{

	}

private:
	Database::DataSet m_dataset;
	Rules::RulesDataAndTypes m_rulesData;
	Application::MainApp m_mainApp;

	void BuildRulesDbs()
	{
		Database::Table table;
		if(false == m_dataset.TryGet(ProjectCommon::MonitorDBIndex::RulesInputDBEnum, table))
				m_dataset.AddTable(ProjectCommon::MonitorDBIndex::RulesInputDBEnum);
		if (false == m_dataset.TryGet(ProjectCommon::MonitorDBIndex::RulesOutputDBEnum, table))
				m_dataset.AddTable(ProjectCommon::MonitorDBIndex::RulesOutputDBEnum);
		if (false == m_dataset.TryGet(ProjectCommon::MonitorDBIndex::RulesEnabledDBEnum, table))
				m_dataset.AddTable(ProjectCommon::MonitorDBIndex::RulesEnabledDBEnum);
		if (false == m_dataset.TryGet(ProjectCommon::MonitorDBIndex::RulesExistenceDBEnum, table))
				m_dataset.AddTable(ProjectCommon::MonitorDBIndex::RulesExistenceDBEnum);
		if (false == m_dataset.TryGet(ProjectCommon::MonitorDBIndex::RulesManagementDBEnum, table))
				m_dataset.AddTable(ProjectCommon::MonitorDBIndex::RulesManagementDBEnum);
	}

	void BuildRulesDataObject()
	{

		m_rulesData = Rules::RulesBuilder::CreateRulesDataAndType(
			m_dataset,
			m_dataset[ProjectCommon::MonitorDBIndex::RulesInputDBEnum],
			m_dataset[ProjectCommon::MonitorDBIndex::RulesEnabledDBEnum],
			m_dataset[ProjectCommon::MonitorDBIndex::RulesExistenceDBEnum],
			m_dataset[ProjectCommon::MonitorDBIndex::RulesOutputDBEnum],
			m_dataset[ProjectCommon::MonitorDBIndex::RulesManagementDBEnum]);
	}
protected:
	virtual void BuildEnvironment() override
	{
		// Initialize the application database
		m_dataset = Database::MemoryDatabase::Create("My Database");
	
		//Try to initialize the database from the dataset xml file
		BuildRulesDbs();

		// Add monitor
		AddRunnable<Database::Monitor>(REMOTE_IP, REMOTE_PORT, LOCAL_IP, LOCAL_PORT, m_dataset);	
	}

	virtual void BuildDispatchers() override
	{
		Application::MainApp mainApp;
		Parsers::BinaryMetadataStore store;
		BuildRulesDataObject();
		std::string path_to_rules_xml = mainApp.DataBasePath() + "Rules.xml";

		AddRunnable(Rules::RulesBuilder::Create(path_to_rules_xml.c_str(),
												m_rulesData));
	}
};

int main(int argc, const char* argv[])
{
	try
	{
		Application::MainApp mainApp(argc, argv);
		return mainApp.BuildAndRun<RulesSampleBuilder>();
	}
	catch (std::exception &e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}
}
