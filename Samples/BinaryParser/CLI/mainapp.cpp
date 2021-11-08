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
#include <ErrorsManagement.hpp>


class CliBuilder : public Application::Builder
{
public:
	CliBuilder() :
		Builder()
	{

	}


	using dataset_map = std::map<std::string, Database::DataSet>;

private:
	dataset_map m_datasetMap;
	Application::Runnable m_runnableMonitor;
	Application::Runnable m_runnableDebugEnvMonitor; // add another monitor connection with the same dataSet
	Rules::RulesManager m_runnableRule;

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

	}

	bool LoadDataSet(const char* path, const char* name, Database::DataSet& dataset)
	{
		try
		{
			dataset = Database::MemoryDatabase::Create("DbgServer");
		}
		catch (std::exception &e)
		{
			Core::Console::ColorPrint(Core::Console::Colors::RED, "Error creating dataset : %s, error %s\n", path, e.what());
			return false;
		}

		if (Database::Schema::Load(dataset, path, Application::MainApp().Verbose(),true))
		{
			Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Dataset %s Loaded\n", name);
			return true;
		}
		else
		{
			Core::Console::ColorPrint(Core::Console::Colors::RED, "Dataset %s Load failed\n", name);
		}
		return false;
	}

	bool LoadRules(const char* rulePath, Database::Table& input, Database::Table& output, Database::Table& existence, Database::Table& enabled, Database::Table& management)
	{
		if (input.Empty() ||
			output.Empty() ||
			existence.Empty() ||
			enabled.Empty())
		{
			Core::Console::ColorPrint(Core::Console::Colors::RED, "Error on Rows for rules\n");
			return false;
		}
		Parsers::BinaryMetadataStore store;
		m_runnableRule = Rules::RulesBuilder::Create(rulePath, input, enabled, existence, output, management, store);
		AddRunnable(m_runnableRule);
		m_runnableRule.Init();
		m_runnableRule.Start();
		m_runnableRule.Started();
		return true;
	}

	bool Load(const char* debugEnvPath)
	{
		try
		{

			std::string remoteIP;
			std::string localIP;
			uint16_t remotePort, localPort;
			std::string projectName, datasetName, path;
			Files::XmlFile xmlFile;

			try
			{
				xmlFile = Files::XmlFile::Create(debugEnvPath);
			}
			catch (std::exception &e)
			{
				Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Error Reading dataset file : %s, error %s\n", debugEnvPath, e.what());
				return false;
			}
			Files::XmlElement element = xmlFile.QueryElement("/Configuration");
			Files::XmlElement child = element.QueryChild("Project.Name");
			projectName = child.Value();
			child = element.QueryChild("Project.Folder");
			//datasetName = child.Value();
			if (datasetName.empty())
			{
				path = ExtractPath(debugEnvPath);
			}
			else
				path = datasetName;

			datasetName = path + "/" + projectName + "DataSet.xml";
			child = element.QueryChild("Project.Communication");
			if (false == child.Empty())
			{
				Files::XmlElement internalElement;
				internalElement = child.QueryChild("Listen.Port");
				remotePort = static_cast<uint16_t>(std::stoi(internalElement.Value()));
				internalElement = child.QueryChild("Transmit.Port");
				localPort = static_cast<uint16_t>(std::stoi(internalElement.Value()));
				internalElement = child.QueryChild("Transmit.Ip");
				remoteIP = "127.0.0.1";
				localIP = "127.0.0.1";
			}
			else
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Error Reading project file : %s, error %s\n", debugEnvPath);
				return false;
			}
			Database::DataSet dataset;
			if (LoadDataSet(datasetName.c_str(), projectName.c_str(), dataset))
			{
				m_runnableMonitor = Application::Runnable::Create<Database::Monitor>(remoteIP.c_str(), remotePort, localIP.c_str(), localPort, dataset);
				AddRunnable(m_runnableMonitor);
				m_runnableMonitor.Start();
				m_datasetMap.emplace(projectName, dataset);

				OpenConstantConnectionToMonitor(dataset);
			}
			else
				return false;

			std::string rulesPath;
			child = element.QueryChild("Rules");
			if (false == child.Empty())
			{
				element = child.QueryChild("Rules.Xml0");
				rulesPath = path + "/" + element.Value();
				Database::Table input = dataset.GetTableByName("RulesInputDBEnum");
				Database::Table output = dataset.GetTableByName("RulesOutputDBEnum");
				Database::Table existence = dataset.GetTableByName("RulesExistenceDBEnum");
				Database::Table enabled = dataset.GetTableByName("RulesEnabledDBEnum");
				Database::Table management = dataset.GetTableByName("RulesManagementDBEnum");
				if (false == LoadRules(rulesPath.c_str(), input, output, existence, enabled, management))
					Core::Console::ColorPrint(Core::Console::Colors::RED, "Error on loading Rules\n");
			}

		}
		catch (std::exception& e)
		{
			Core::Console::ColorPrint(Core::Console::Colors::RED, "Error on load: %s", e.what());
			return false;
		}
		return true;
	}
	bool VerifyRow(Database::DataSet dataset, const char* tableName, const char* rowName,bool printJson, Parsers::JsonDetailsLevel detalsLevel)
	{
		Database::Table table = dataset.GetTableByName(tableName);
		if (table.Empty())
		{
			Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown table\n");
		}

		Database::Row row = table.GetRowByName(rowName);
		if (row.Empty())
		{
			Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown row %s\n",rowName);
			return false;
		}
		if (row.Info().type == TypeEnum::EMPTY_TYPE)
		{
			Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Empty (none) row %s\n", rowName);
			return true;
		}
		std::string jsonStr;
		
		if (false == row.TryGetJson(jsonStr,detalsLevel,false))
		{
			Core::Console::ColorPrint(false, true,Core::Console::Colors::RED, "\n");
			std::cout << jsonStr.c_str()<<"\n";
			return false;
		}
		
		std::string rowNameStr(row.Info().name);
		if (row.FromJson(jsonStr.c_str()))
		{
			if (printJson)
			{
				std::cout << jsonStr.c_str() << "\n";
			}
			return true;
		}
		return false;
	}

	void OpenConstantConnectionToMonitor(Database::DataSet dataset)
	{
        constexpr char const* LOCAL_IP = "127.0.0.1";
        constexpr uint16_t	LOCAL_PORT = 3333;
        constexpr char const* REMOTE_IP = "127.0.0.1";
        constexpr uint16_t REMOTE_PORT = 4444;

        m_runnableDebugEnvMonitor = Application::Runnable::Create<Database::Monitor>(REMOTE_IP, REMOTE_PORT, LOCAL_IP, LOCAL_PORT, dataset);
        AddRunnable(m_runnableDebugEnvMonitor);
        m_runnableDebugEnvMonitor.Start();
	}

	virtual void BuildDispatchers() override
	{
		Application::MainApp mainApp;
		std::string depath = mainApp.DataBasePath();
		if (depath.find(".xml") != std::string::npos) //there is a file
		{
			if(false == Load(depath.c_str()))
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Failed to load - %s\n");
		}

		mainApp.AddCLICmd("load", "load a debug environment\n   Syntax: loadde <debug env file> <path to debug environement>", [&](const char *cmd, size_t size, const char* params[])
		{
			if (size < 1)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To Few parameters\n");
				return;
			}

			Load(params[0]);
			Application::MainApp main;
		});

		mainApp.AddCLICmd("load_dataset", "load a data set\n   Syntax: load <dataset_name> <path to dataset>", [&](const char *cmd, size_t size, const char* params[])
		{
			if (size < 2)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To Few parameters\n");
				return;
			}
			Database::DataSet dataset;
			if (LoadDataSet(params[1],params[0], dataset))
			{

				OpenConstantConnectionToMonitor(dataset);
				m_datasetMap.emplace(params[0], dataset);
			}
		});

		mainApp.AddCLICmd("loadrules", "load a rules xml\n   Syntax: load <dataset_name> <path to rule xml> <input table> <output table> <existence table> <enabled table>", [&](const char *cmd, size_t size, const char* params[])
		{
			if (size < 7)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To Few parameters\n");
				return;
			}

			auto it = m_datasetMap.find(params[0]);
			if (it == m_datasetMap.end())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "wrong dataset parameters\n");
				return;
			}
			Database::DataSet dataset = it->second;
			
			Parsers::BinaryMetadataStore stroe;
			Database::Table input, output, existence, enabled, management;
			
			input = dataset.GetTableByName(params[2]);
			output = dataset.GetTableByName(params[3]);
			existence = dataset.GetTableByName(params[4]);
			enabled = dataset.GetTableByName(params[5]);
			management = dataset.GetTableByName(params[6]);
			
			if(false ==LoadRules(params[1], input, output, existence, enabled, management))
				Core::Console::ColorPrint(Core::Console::Colors::RED, "failed to load rules files %s\n", params[1]);

		});

		mainApp.AddCLICmd("load_errors", "load a Error xml\n   Syntax: load_errors <dataset_name> <path errors xml>", [&](const char* cmd, size_t size, const char* params[])
		{
			if (size < 2)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To Few parameters\n");
				return;
			}

			auto it = m_datasetMap.find(params[0]);
			if (it == m_datasetMap.end())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "wrong dataset parameters\n");
				return;
			}
			Database::DataSet dataset = it->second;

			utils::ref_count_ptr<Errors::ErrorsManager> errorDispatcher = utils::make_ref_count_ptr<Errors::ErrorsManager>(dataset, params[1], m_runnableRule, Application::MainApp().Verbose());
			AddRunnable(errorDispatcher);
			errorDispatcher->Init();
			

		});
        mainApp.AddCLICmd("unload", "unload a project\n   Syntax: unload <dataset_name>", [&](const char *cmd, size_t size, const char* params[])
        {
            (void)cmd;
			if (size < 1)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To Few parameters\n");
				return;
			}

			auto it = m_datasetMap.find(params[0]);
			if (it != m_datasetMap.end())
			{
				m_datasetMap.erase(it);
				Core::Console::ColorPrint(Core::Console::Colors::GREEN, "dataset %s unloaded\n",params[0]);
			}
			if (false == m_runnableMonitor.Empty())
			{
				m_runnableMonitor.Stop();
				m_runnableMonitor = nullptr;
			}
            if (false == m_runnableDebugEnvMonitor.Empty())
            {
                m_runnableDebugEnvMonitor.Stop();
                m_runnableDebugEnvMonitor = nullptr;
            }
			if (false == m_runnableRule.Empty())
			{
				m_runnableRule.Stop();
				m_runnableRule.Stopped();
				m_runnableRule = nullptr;
			}
		});
		
		mainApp.AddCLICmd("dataset", "list of datasets\n   Syntax: dataset", [this](const char *cmd, size_t size, const char* params[])
		{
			
			for (auto& it : m_datasetMap)
			{
				Core::Console::ColorPrint(Core::Console::Colors::GREEN, "%s: loaded\n",it.first.c_str());
			}

		});
		mainApp.AddCLICmd("schematest", "test schema of a dataset", [this](const char *cmd, size_t size, const char* params[])
		{
			if (size < 1)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To few parameters\n");
				return;
			}
			
			auto it = m_datasetMap.find(params[0]);
			if (it == m_datasetMap.end())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "unknown dataset\n");
				return;
			}

			Database::DataSet dataset = it->second;
			
			for (Database::Table table : dataset)
			{
				Core::Console::ColorPrint(Core::Console::Colors::WHITE, "Verify Table %s\n", table.Name());
				Parsers::BinaryMetadataStore store = Parsers::BinaryMetadataStore::Create();
				for (Database::Row row : table)
				{
					if (row.ParserMetadata().Empty())
					{
						if(row.Info().type == TypeEnum::EMPTY_TYPE)
							Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "     Row %s -OK: Empty Type No Parser\n", row.Info().name);
						else
							Core::Console::ColorPrint(Core::Console::Colors::RED, "     Row %s - Failed:No Parser\n", row.Info().name);

					}
					else
					{
						Parsers::BinaryMetaDataBuilder metadata = Parsers::BinaryMetaDataBuilder::Create(row.ParserMetadata().ToJson(), store);
						if (metadata.Size() == row.ParserMetadata().Size())
							Core::Console::ColorPrint(Core::Console::Colors::GREEN, "    Row %s - OK\n", row.Info().name);
						else
							Core::Console::ColorPrint(Core::Console::Colors::RED, "     Row %s - Failed\n", row.Info().name);
					}
				}
			}
			
		});
		mainApp.AddCLICmd("schema", "Get the schema of a specific row\n   Syntax: schema <dataset_name> <table_name> <row_name>", [&](const char *cmd, size_t size, const char* params[])
		{
	
			if (size < 3)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To few parameters\n");
				return;
			}
			
			auto it = m_datasetMap.find(params[0]);
			if (it == m_datasetMap.end())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "unknown dataset\n");
			}

			Database::DataSet dataset = it->second;
			Database::Table table = dataset.GetTableByName(params[1]);
			if (table.Empty())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown table\n");
			}

			Database::Row row = table.GetRowByName(params[2]);
			if (row.Empty())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown row\n");
			}

			Parsers::BinaryMetaData metadata = row.ParserMetadata();
			std::cout << metadata.ToJson(false);


		});
		mainApp.AddCLICmd("verify", "verify a specific row by reading and writing a json to it\n   Syntax: verify <dataset_name> <table_name> <row_name>", [&](const char *cmd, size_t size, const char* params[])
		{
			if (size < 3)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To few parameters\n");
				return;
			}
			std::string key_str(params[0]);
			auto it = m_datasetMap.find(key_str);
			if (it == m_datasetMap.end())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown dataset\n");
				return;
			}
			else
			{
				Database::DataSet dataset = it->second;
				if (VerifyRow(it->second, params[1], params[2],true, Parsers::JsonDetailsLevel::JSON_ENUM_FULL))
					Core::Console::ColorPrint(Core::Console::Colors::GREEN, "%s/%s/%s - OK\n", params[0], params[1], params[2]);
				else
					Core::Console::ColorPrint(Core::Console::Colors::RED, "%s/%s/%s - Failed\n", params[0], params[1], params[2]);
			}
		});
		mainApp.AddCLICmd("verifyall", "test all dataset", [this](const char *cmd, size_t size, const char* params[])
		{
			try
			{

			Parsers::JsonDetailsLevel detalsLevel = Parsers::JsonDetailsLevel::JSON_ENUM_FULL;
			if (size < 1)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To few parameters\n");
				return;
			}
			bool toPrintJson = false;
			if (size >= 2)
			{
				std::string print = params[1];
				if (print == "print")
					toPrintJson = true;

				if (size == 3)
				{
					std::string detailsLevelStr = params[2];

					if (detailsLevelStr == "lables")
						detalsLevel = Parsers::JsonDetailsLevel::JSON_ENUM_LABLES;
					if (detailsLevelStr == "values")
						detalsLevel = Parsers::JsonDetailsLevel::JSON_ENUM_VALUES;
					if (detailsLevelStr == "full")
						detalsLevel = Parsers::JsonDetailsLevel::JSON_ENUM_FULL;


				}
			}

			
			auto it = m_datasetMap.find(params[0]);
			if (it == m_datasetMap.end())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "unknown dataset\n");
				return;
			}

			Database::DataSet dataset = it->second;

			for (Database::Table table : dataset)
			{
				Core::Console::ColorPrint(Core::Console::Colors::WHITE, "Verify Table %s\n", table.Name());
				for (Database::Row row : table)
				{
					if (VerifyRow(it->second, table.Name(), row.Info().name,toPrintJson, detalsLevel))
					{
						Core::Console::ColorPrint(Core::Console::Colors::GREEN, "%s/%s/%s - OK\n", params[0], table.Name(), row.Info().name);
					}
					else
						Core::Console::ColorPrint(Core::Console::Colors::RED, "%s/%s/%s - Failed\n", params[0], table.Name(), row.Info().name);

				}
			}
			}
			catch (const std::exception& e)
			{
				std::cout << "verifyall exception:" << e.what();
			}
		});
		mainApp.AddCLICmd("read", "subscribe to a specific row\n   Syntax: subscribe <dataset_name> <table_name> <row_name>", [&](const char *cmd, size_t size, const char* params[])
		{
			if (size < 3)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To few parameters\n");
				return;
			}
			std::string key_str(params[0]);
			auto it = m_datasetMap.find(key_str);
			if (it == m_datasetMap.end())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown dataset\n");
				return;
			}
			else
			{
				Database::DataSet dataset = it->second; 
				
				Database::Table table = dataset.GetTableByName(params[1]);
				if (table.Empty())
				{
					Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown table\n");
				}

				Database::Row row = table.GetRowByName(params[2]);
				if (row.Empty())
				{
					Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown row\n");
				}
				Buffers::Buffer buffer(utils::make_ref_count_ptr<utils::ref_count_buffer>(row.DataSize()));
				row.Read(buffer.Data(), buffer.Size());
				Parsers::BinaryParser parser = row.ParserMetadata().CreateParser();
				parser.Parse(buffer.Data(), buffer.Size());
				std::string jsonStr;
				bool retval = parser.TryGetJson(jsonStr, Parsers::JsonDetailsLevel::JSON_ENUM_FULL, false);
				if (false == retval)
				{
					Core::Console::ColorPrint(Core::Console::Colors::RED, "JSON with bad values\n");
					std::cout << jsonStr.c_str()<<std::endl;
				}
				std::cout << jsonStr.c_str() << std::endl;
			}

		});
		mainApp.AddCLICmd("write", "write to a specific row\n   Syntax: write <dataset_name> <table_name> <row_name>", [&](const char *cmd, size_t size, const char* params[])
		{
			if (size < 4)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To few parameters\n");
				return;
			}
			std::string key_str(params[0]);
			auto it = m_datasetMap.find(key_str);
			if (it == m_datasetMap.end())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown dataset\n");
				return;
			}
			else
			{
				Database::DataSet dataset = it->second;

				Database::Table table = dataset.GetTableByName(params[1]);
				if (table.Empty())
				{
					Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown table\n");
				}

				Database::Row row = table.GetRowByName(params[2]);
				if (row.Empty())
				{
					Core::Console::ColorPrint(Core::Console::Colors::RED, "Unknown row\n");
				}
				
				if(row.FromJson(params[3]) == false)
					Core::Console::ColorPrint(Core::Console::Colors::RED, "Failed to write to row from JSON\n");
			}

		});
		mainApp.AddCLICmd("enum", "read enum\n   Syntax: read <enum name>", [&](const char *cmd, size_t size, const char* params[])
		{
			if (size < 1)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To few parameters\n");
				return;
			}

			Parsers::BinaryMetadataStore store;
			Parsers::EnumData enumData = store.Enum(params[0]);
			if (enumData.Empty())
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Enum %s not found\n", params[0]);
			else
				std::cout << enumData.ToJson(false) << "\n";
		});

		mainApp.AddCLICmd("reset", "reset a row, table or a whole dataset,\n syntax: reset dataset_name [table_name] [row_name]", [this](const char *cmd, size_t size, const char* params[])
		{
			if (size < 1)
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "To few parameters\n");
				return;
			}

			auto it = m_datasetMap.find(params[0]);
			if (it == m_datasetMap.end())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "unknown dataset\n");
				return;
			}

			Database::DataSet dataset = it->second;
			if (size == 1)
				dataset.Reset();
			else if (size == 2)
			{
				Database::Table table = dataset.GetTableByName(params[1]);
				table.Reset();
			}
			else if (size == 3)
			{
				Database::Table table = dataset.GetTableByName(params[1]);
				Database::Row row = table.GetRowByName(params[2]);
				row.Reset();
				
			}

		});
	}
public:
	virtual ~CliBuilder()
	{
		
	}

	
};
int main(int argc, const char* argv[])	
{

	try
	{
		Application::MainApp mainApp(argc, argv);
		return mainApp.BuildAndRun<CliBuilder>();
	}
	catch (std::exception &e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}
	
	getchar();

	return 0;
}
