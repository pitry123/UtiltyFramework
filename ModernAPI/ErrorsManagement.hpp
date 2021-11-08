#pragma once
#include <utils/types.hpp>
#include <BinaryParser.hpp>
#include <Database.hpp>
#include <XmlStoreLoader.hpp>
#include <Common.h>
#include <unordered_map>
#include <memory>

namespace Errors
{	
	using namespace Common;

	class ErrorsManager : public Database::Dispatcher
	{
	private:
		
		Database::DataSet m_dataset;
		std::string m_errorPath;
		Rules::RulesManager m_rulesManager;
		bool m_verbose;
		Logging::Logger ERRMNG_LOG;
		Database::Table m_errorGroupsTable;
		Database::Table m_errorsStoreTable;
		Database::Table m_errorsTable;
		Database::SubscriptionsCollector m_subscriptions;
		using VectorRowPtr = std::shared_ptr<std::vector<Database::Row>>;
		std::unordered_map<std::string, VectorRowPtr> m_errorGropsMap;
		Parsers::BinaryMetadataStore m_store;

		bool BuildErrorDB(Files::XmlElement& element)
		{
			Files::XmlElement dbElement = element.QueryChild("DATABASE");
			if (dbElement.Empty())
			{
				LOG_ERROR(ERRMNG_LOG) << "No 'DATABASE' defined in XML file:" << m_errorPath.c_str();
				throw std::runtime_error("No 'DATABASE' defined in XML file");
			}
			
			Files::XmlAttribute  attrib = dbElement.QueryAttribute("name");
			std::string name;
			if (false == attrib.Empty())
			{
				name = attrib.Value();
				m_errorsStoreTable = m_dataset.GetTableByName(name.c_str());
				if (m_errorsStoreTable.Empty())
				{
					LOG_ERROR(ERRMNG_LOG) << "BuildErrorDB ErrorStore Table" << name.c_str() << " does not exist";
					return false;
				}
				if (false == XmlStoreLoader::Load(m_errorsStoreTable, dbElement, m_verbose))
				{
					LOG_ERROR(ERRMNG_LOG) << "Failed to Load Error Store by GUID:" << name.c_str() << " to dataset ";
					return false;
				}
			}
			attrib = dbElement.QueryAttribute("errorsDB");
			if (false == attrib.Empty())
			{
				name = attrib.Value();
				m_errorsTable = m_dataset.GetTableByName(name.c_str());
				if (m_errorsTable.Empty())
				{
					LOG_ERROR(ERRMNG_LOG) << "BuildErrorDB ErrorsDB Table" << name.c_str() << " does not exist";
					return false;
				}
				Parsers::BinaryMetadataStore store;
				
				Database::RowInfo info;
				info.type = TypeEnum::COMPLEX;
				Parsers::BinaryMetaData metadata = store.Metadata("CommonTypes::ErrorStatusStruct");
				if (metadata.Empty())
					throw std::runtime_error("Common::CommonTypes::ErrorStatusStruct - metadata does not exist");
				STRCPY(info.type_name, sizeof(info.type_name), metadata.Namely());
				
				Parsers::BinaryParser parser;
			 	Common::CommonTypes::ErrorStatusStruct errorStatus = {
						Common::CommonTypes::ErrorStatusEnum::NOT_EXIST,
						std::chrono::system_clock::now().time_since_epoch()/std::chrono::milliseconds(1),
						Common::CommonTypes::BoolEnum::BOOL_FALSE,
						0,
						{0,true}
				};
				for (Database::Row& row : m_errorsStoreTable)
				{
					STRCPY(info.name, sizeof(info.name), row.Info().name);
					Database::AnyKey key = row.Key();
					
					utils::ref_count_ptr<core::buffer_interface> buffer = utils::make_ref_count_ptr<utils::ref_count_buffer>(m_errorsStoreTable[key].ParserMetadata().Size());
					Parsers::BinaryParser parserStore =  m_errorsStoreTable[key].ParserMetadata().CreateParser();
					m_errorsStoreTable[key].Read(buffer->data(),buffer->size());
					parserStore.Parse(buffer->data(), buffer->size());
		
					if (parserStore.Read<uint64_t>("DelayToSet") > 0)
						errorStatus.pendable = Common::CommonTypes::BOOL_TRUE;
					else
						errorStatus.pendable = Common::CommonTypes::BOOL_FALSE;
					
					errorStatus.errorKey = static_cast<int>(key);
					m_errorsTable.AddRow<Common::CommonTypes::ErrorStatusStruct>(key,errorStatus, info, metadata);
					m_errorsTable[key].Write<Common::CommonTypes::ErrorStatusStruct>(errorStatus);
				}
				return true;
			}
			return false;
		}

		bool LoadErrorGroups(Files::XmlElement& dbElement)
		{
			for (auto& db_entry : dbElement.Children("ErrorGroup"))
			{
				Database::RowInfo info;
				Files::XmlAttribute attrib = db_entry.QueryAttribute("type");
				if (attrib.Empty())
					throw std::runtime_error("LoadErrorGroups - Type");

				info.type = utils::types::get_type(attrib.Value());
				Parsers::BinaryMetaDataBuilder metadata;;
				size_t size = 0;
				if (info.type == TypeEnum::COMPLEX)
				{
					Parsers::BinaryMetadataStore store;
					metadata = store.Metadata(attrib.Value());

					if (metadata.Empty())
						throw std::runtime_error("LoadErrorGroups Complex type does not exist");
					if (metadata.Size() != sizeof(Common::CommonTypes::ErrorGroupStruct))
						throw std::runtime_error("LoadErrorGroups Complex does not match CommonTypes::ErrorGroupStruct");
				}
				else
				{
					size = utils::types::sizeof_type(info.type);
					metadata = Parsers::BinaryMetaDataBuilder::Create();
					metadata.Simple("ErrorGroup", size, info.type);
				}
				
				STRCPY(info.name, sizeof(info.name), db_entry.Value());
				STRCPY(info.type_name, sizeof(info.type_name), attrib.Value());

				attrib = db_entry.QueryAttribute("id");
				int val = attrib.ValueAsInt(-1);
				if (val == -1)
					throw std::invalid_argument("Illegele Group Row ID");
				m_errorGroupsTable.AddRow(val, size, info, metadata);
			}

			return true;
		}

		bool BuildErrorGroupsTable(Files::XmlElement& rootElement)
		{
			Files::XmlElement dbElement = rootElement.QueryChild("ErrorGroupsDB");
			Files::XmlAttribute attrib =  dbElement.QueryAttribute("name");
			std::string name = attrib.Value();
			if (name.empty())
				return false;
			m_errorGroupsTable = m_dataset.GetTableByName(name.c_str());

			//If table does not exist create it and add the rows
			if (m_errorGroupsTable.Empty())
			{
				LOG_ERROR(ERRMNG_LOG) << "BuildErrorGroupsTable ErrorGroupTable" << name.c_str() << " does not exist";
				return false;
			}
			

			if(m_errorGroupsTable.Size() == 0)
				return LoadErrorGroups(dbElement);

			return true;

		}

		void UpdateError(Database::Row errorRow, Common::CommonTypes::StatusEnum errorSatus)
		{
			Common::CommonTypes::ErrorStatusStruct error;
			error = errorRow.Read<Common::CommonTypes::ErrorStatusStruct>();
            uint64_t currentTime = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch() /
                std::chrono::milliseconds(1));

			utils::ref_count_ptr<core::buffer_interface> buffer = utils::make_ref_count_ptr<utils::ref_count_buffer>(m_errorsStoreTable[errorRow.Key()].ParserMetadata().Size());
			Parsers::BinaryParser parser = m_errorsStoreTable[errorRow.Key()].ParserMetadata().CreateParser();
			m_errorsStoreTable[errorRow.Key()].Read(buffer->data(), buffer->size());
			parser.Parse(buffer->data(), buffer->size());
		
			bool changed = false;
			if (false == error.statistics.Valid)
			{
				LOG_WARNING(ERRMNG_LOG) << "Error:" << parser.ReadString("Description").c_str() << " Triggered but valid flag is false";
				return;
			}

			if (errorSatus == Common::CommonTypes::StatusEnum::STATUS_OK)
			{
				if (error.status == Common::CommonTypes::EXIST ||
					error.status == Common::CommonTypes::REPEATED)
				{
					error.status = Common::CommonTypes::VANISHED;
					changed = true;
				}
			}
			else if (errorSatus == Common::CommonTypes::StatusEnum::STATUS_FAIL)
			{
				error.statistics.currentCount++;
                if (static_cast<uint64_t>(error.timeStamp) + parser.Read<uint64_t>("DelayToSet") < currentTime)
				{
					error.pendable = Common::CommonTypes::BOOL_FALSE;
				}

				if (parser.Read<short>("RepeatsToSet") == 0||
					(error.statistics.currentCount % parser.Read<short>("RepeatsToSet") == 0 &&
					error.pendable == Common::CommonTypes::BOOL_FALSE))
				{

					if (error.status == Common::CommonTypes::ErrorStatusEnum::VANISHED)
					{
						error.status = Common::CommonTypes::ErrorStatusEnum::REPEATED;
						changed = true;
					}

					if (error.status == Common::CommonTypes::ErrorStatusEnum::NOT_EXIST)
					{
						error.status = Common::CommonTypes::ErrorStatusEnum::EXIST;
						changed = true;
					}

					if (error.status == Common::CommonTypes::ErrorStatusEnum::PEND_TO_EXIST)
					{
						error.status = Common::CommonTypes::EXIST;
						changed = true;
					}

					if (error.status == Common::CommonTypes::ErrorStatusEnum::PEND_TO_REPEAT)
					{
						error.status = Common::CommonTypes::REPEATED;
						changed = true;
					}
					
				}
				
				if (error.pendable == Common::CommonTypes::BOOL_TRUE)
				{
					if(error.status == Common::CommonTypes::ErrorStatusEnum::NOT_EXIST ||
					   (error.status == Common::CommonTypes::ErrorStatusEnum::VANISHED &&
						error.statistics.currentCount == 1))
						error.status = Common::CommonTypes::ErrorStatusEnum::PEND_TO_EXIST;
					else
						error.status = Common::CommonTypes::ErrorStatusEnum::PEND_TO_REPEAT;
					changed = true;
				}
			}
			else if (errorSatus == Common::CommonTypes::StatusEnum::STATUS_NONE)
			{
				error.status = Common::CommonTypes::ErrorStatusEnum::NOT_EXIST;
				changed = true;
			}

            error.timeStamp = static_cast<int64_t>(currentTime);
			if (changed)
				LOG_INFO(ERRMNG_LOG) << "Error Status:" << parser.ReadString("Description").c_str() << " changed to " << m_store.Enum<Common::CommonTypes::ErrorStatusEnum>().GetItemNameByValue(error.status);

			errorRow.Write<Common::CommonTypes::ErrorStatusStruct>(error);
		}

		void UpdateGroupError(Database::Row  errorGroupRow, const Common::CommonTypes::ErrorStatusStruct& error)
		{
			Common::CommonTypes::ErrorGroupStruct errorGroup;
			int count;
			if (errorGroupRow.Info().type == TypeEnum::COMPLEX)
			{
				errorGroup = errorGroupRow.Read<Common::CommonTypes::ErrorGroupStruct>();
				count = errorGroup.numOfExistingErrors;
				utils::ref_count_ptr<core::buffer_interface> buffer = utils::make_ref_count_ptr<utils::ref_count_buffer>(m_errorsStoreTable[error.errorKey].ParserMetadata().Size());
				Parsers::BinaryParser parser = m_errorsStoreTable[error.errorKey].ParserMetadata().CreateParser();
				m_errorsStoreTable[error.errorKey].Read(buffer->data(), buffer->size());
				parser.Parse(buffer->data(), buffer->size());
				STRCPY(errorGroup.lastError, sizeof(errorGroup.lastError), parser.ReadString("Description").c_str());
			}
			else
			{
				count = errorGroupRow.Read<int>();
			}
			
			if (error.status == Common::CommonTypes::ErrorStatusEnum::REPEATED ||
				error.status == Common::CommonTypes::ErrorStatusEnum::EXIST)
				count++;
			else if (error.status == Common::CommonTypes::VANISHED || 
				error.status == Common::CommonTypes::NOT_EXIST)
			{
				if (count > 0)
					count--;
				else
					return;
			}
			if (errorGroupRow.Info().type == TypeEnum::COMPLEX)
			{
				errorGroup.numOfExistingErrors = count;
				errorGroupRow.Write<Common::CommonTypes::ErrorGroupStruct>(errorGroup);
				LOG_INFO(ERRMNG_LOG) << "Error Group:" << errorGroupRow.Info().name << " count:" << errorGroup.numOfExistingErrors<<" Truggered by Error:" << errorGroup.lastError;
			}
			else
			{
				errorGroupRow.Write<int>(count);
				LOG_INFO(ERRMNG_LOG) << "Error Group:" << errorGroupRow.Info().name << " count:" << count;
			}
		}

		void LoadTriggers(Files::XmlElement rootElement)
		{
			Files::XmlElement dbElement = rootElement.QueryChild("ErrorsMetadata");
			for (auto& dbEntry : dbElement.Children("Error"))
			{
				Files::XmlAttribute  attrib = dbEntry.QueryAttribute("name");
				if (false == attrib.Empty())
				{
					std::string errorRowName = attrib.Value();
					Database::Row errorRow = m_errorsTable.GetRowByName(errorRowName.c_str());
					if (errorRow.Empty())
					{
						LOG_ERROR(ERRMNG_LOG) << "LoadTriggers - Error Row:" << errorRowName.c_str() << " Row is Missing";
						throw std::runtime_error("LoadTriggers - Error getting Error Row DB");
					}

					auto element = dbEntry.QueryChild("Triggers");
					if (element.Empty())
					{
						LOG_ERROR(ERRMNG_LOG) << "LoadTriggers - Missing Triggers element";
						throw std::runtime_error("LoadTriggers - Missing Triggers element");
					}
					std::string trigger;
					for (auto& child : element.Children())
					{
						std::string childName(child.Name());
						if (childName == "Trigger")
						{
							Files::XmlElement dbData;
							std::string tableStr, rowStr;
							dbData = child.QueryChild("DataTable");
							if(dbData.Empty())
								throw std::runtime_error("LoadTriggers - Error Getting Table Element");

							tableStr = dbData.Value();

							dbData = child.QueryChild("DataRow");
							if (dbData.Empty())
								throw std::runtime_error("LoadTriggers - Error Getting Row Element");
							rowStr = dbData.Value();

							Database::Table triggerTable = m_dataset.GetTableByName(tableStr.c_str());
							if(triggerTable.Empty())
								throw std::runtime_error("LoadTriggers - Error Getting Table by Name");

							Database::Row triggerRow = triggerTable.GetRowByName(rowStr.c_str());
							if (triggerRow.Empty())
								throw std::runtime_error("LoadTriggers - Error Getting Row by Name");

							m_subscriptions += Subscribe(triggerRow, [this,errorRow](const Database::RowData& rowData) 
							{
								Common::CommonTypes::StatusEnum data = rowData.Read<Common::CommonTypes::StatusEnum>();
								UpdateError(errorRow,data);
								
							});

						}
						else if (childName == "Rule")
						{
							//m_rulesManager.AddRule(child);
						}
					}

					element = dbEntry.QueryChild("ErrorGroups");
					if (element.Empty())
					{
						LOG_WARNING(ERRMNG_LOG) << "LoadTriggers - Missing ErrorGroups element";
					}
					std::string errorsGroup;
					std::shared_ptr<std::vector<Database::Row>> errorGroupVector = 
						std::make_shared<std::vector<Database::Row>>();

					for (auto& child : element.Children())
					{
						errorsGroup = child.Value();
						Database::Row groupRow = m_errorGroupsTable.GetRowByName(errorsGroup.c_str());
						if (groupRow.Empty())
						{
							LOG_ERROR(ERRMNG_LOG) << "LoadTriggers - Missing Error:"<< errorsGroup.c_str()<< " in ErrorGroups table";
							throw std::runtime_error("LoadTriggers - Missing Error in ErrorGroups table");
						}

						errorGroupVector->emplace_back(groupRow);
					}
					m_errorGropsMap[errorRowName] = errorGroupVector;
					m_subscriptions += Subscribe(errorRow, [&, errorRowName](const Database::RowData& rowData)
					{
						Common::CommonTypes::ErrorStatusStruct error = rowData.Read<Common::CommonTypes::ErrorStatusStruct>();
						//Update status of Errorgroups
						for (auto& item : *m_errorGropsMap[errorRowName])
						{
							UpdateGroupError(item,error);
						}
					});
				}
			}
		}
	public:
		enum ErrorMetadata
		{			
			ErrorStatus,	 //t:CommonTypes::ErrorStatusStruct
			ErrorGroups,	 //t:CommonTypes::ErrorGroupStruct
			NUM_OF_GENERAL_INFO
		};
		ErrorsManager(Database::DataSet dataset, const char* errorPath,Rules::RulesManager& ruleManager, bool verbose):
			m_dataset(dataset),
			m_errorPath(errorPath),
			m_rulesManager(ruleManager),
			m_verbose(verbose),
			m_store()
		{
			Logging::Severity sevirity = Logging::Severity::WARNING;
			if (verbose)
			{
				sevirity = Logging::Severity::DEBUG;
			}
			ERRMNG_LOG = Core::Framework::CreateLogger("ErrorManager", sevirity);
		}

		void Init() override
		{
			Files::XmlFile xmlFile;

			try
			{
				xmlFile = Files::XmlFile::Create(m_errorPath.c_str());
			}
			catch (std::exception &e)
			{
				LOG_ERROR(ERRMNG_LOG) << "Error Reading dataset file:" << m_errorPath.c_str() << "error:" << e.what();
				throw std::runtime_error("Error Reading dataset");
			}

			Files::XmlElement rootElement = xmlFile.QueryElement("ERROR_STORE");
			BuildErrorGroupsTable(rootElement);
			
			if(false == BuildErrorDB(rootElement))
				throw std::runtime_error("ErrorsManager XmlStoreLoader failed");

			LoadTriggers(rootElement);
		}
	};
}
