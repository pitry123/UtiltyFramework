#pragma once
#include <rules/rules.h>
#include <utils/rules.hpp>
#include <files/xml_file_interface.h>
#include <Database.hpp>
#include <Application.hpp>

namespace Rules
{	
	using RulesCallbacFunc = utils::rules::rules_callback_func;

	class RulesDataAndTypes : public Common::CoreObjectWrapper<core::rules::rules_data_and_types_interface>
	{
	public:
		RulesDataAndTypes() {}

		RulesDataAndTypes(core::rules::rules_data_and_types_interface* rules_data) :
			Common::CoreObjectWrapper<core::rules::rules_data_and_types_interface>(rules_data)
		{
		}

		Database::Row GetRowByString(const char* rowString)
		{
			ThrowOnEmpty("RulesDataAndTypes");
			core::database::row_interface *row;
			if (false == m_core_object->query_row_by_string(rowString, &row))
			{
				throw std::runtime_error("row does not exits");
			}
			return Database::Row(row);
		}

		Database::Row GetRuleExistenceRow(size_t ruleID)
		{
			ThrowOnEmpty("RulesDataAndTypes");
			core::database::row_interface *row;
			if (false == m_core_object->query_rule_existence_row(ruleID, &row))
			{
				throw std::runtime_error("row does not exits");
			}
			return Database::Row(row);
		}

		Database::Row GetRuleEnableRow(size_t ruleID)
		{
			ThrowOnEmpty("RulesDataAndTypes");
			utils::ref_count_ptr<core::database::row_interface> row;
			if (false == m_core_object->query_rule_enable_row(ruleID, &row))
			{
				throw std::runtime_error("row does not exits");
			}
			return Database::Row(row);
		}

		Database::Row GetReloadRulesRow()
		{
			ThrowOnEmpty("RulesDataAndTypes");
			core::database::row_interface *row;
			if (false == m_core_object->query_reload_rules_row(&row))
			{
				throw std::runtime_error("row does not exits");
			}

			return Database::Row(row);
		}

		int64_t GetEnumeration(const char* enumString)
		{
			ThrowOnEmpty("RulesDataAndTypes");
			int64_t val = 0;
			if (false == m_core_object->get_enumeration(enumString, val))
				throw std::runtime_error("no enumeration");
			
			return val;
		}

		RulesCallbacFunc GetFunction(const char* func_string)
		{
			RulesCallbacFunc func = nullptr;
			utils::ref_count_ptr<core::rules::rule_func_callback_interface> callback;
			if (m_core_object->query_function(func_string, &callback))
			{
				func = ([callback]() {
					double retval;
					if (false == callback->execute(retval))
						throw std::runtime_error("execute failed");

					return retval;
				});
			}
			else
				throw std::runtime_error("function does not exist");

			return func;
			
		}
	};
	class RulesManager : public Application::Runnable
	{
	private:
		rules::rules_dispatcher *rules_dispatcher()
		{
			return (rules::rules_dispatcher *)static_cast<core::application::runnable_interface*>(*this);
		}
	public:
		RulesManager()
		{

		}

		RulesManager(rules::rules_dispatcher* instance):
			Application::Runnable(instance)

		{

		}

		bool AddRule(Files::XmlElement element)
		{
			ThrowOnEmpty("RulesManager");

			return rules_dispatcher()->add_rule(static_cast<files::xml_element_interface*>(element));
		}
	};

	class RulesBuilder :
		public Common::NonConstructible
	{
	public:
		static Rules::RulesManager Create(
			const char*				FilePath,
			RulesDataAndTypes rulesData)
		{

			utils::ref_count_ptr<rules::rules_dispatcher> instance;
			if (rules::rules_dispatcher::create(
				FilePath,
				static_cast<core::rules::rules_data_and_types_interface*>(rulesData),
				&instance) == false)
				throw std::runtime_error("Failed to create rules dispatcher");

			return Rules::RulesManager(instance);
		}

		static RulesManager Create(const char*		  filePath,
											const Database::Table& inputTable,
											const Database::Table& enableTable,
											const Database::Table& existenceTable,
											const Database::Table& outputTable,
											const Database::Table& managementTable,
											const Parsers::BinaryMetadataStore& store)
		{		
			utils::ref_count_ptr<rules::rules_dispatcher> instance;
			if (rules::rules_dispatcher::create(filePath,
												static_cast<core::database::table_interface*>(inputTable),
												static_cast<core::database::table_interface*>(enableTable),
												static_cast<core::database::table_interface*>(existenceTable),
												static_cast<core::database::table_interface*>(outputTable),
												static_cast<core::database::table_interface*>(managementTable),
												static_cast<core::parsers::binary_metadata_store_interface*>(store),
												&instance) == false)
				throw std::runtime_error("Failed to create rules dispatcher");

			return Rules::RulesManager(instance);
		}
		
		static RulesManager Create(const char*		  filePath,
			const Database::DataSet& dataset,
			const Parsers::BinaryMetadataStore& store)
		{
			utils::ref_count_ptr<rules::rules_dispatcher> instance;
			if (rules::rules_dispatcher::create(filePath,
				static_cast<core::database::dataset_interface*>(dataset),
				static_cast<core::parsers::binary_metadata_store_interface*>(store),
				&instance) == false)
				throw std::runtime_error("Failed to create rules dispatcher");

			return Rules::RulesManager(instance);

		}

		static RulesDataAndTypes  CreateRulesDataAndType(
			const Database::DataSet& dataset,
			const Database::Table& inputTable,
			const Database::Table& enableTable,
			const Database::Table& existenceTable,
			const Database::Table& outputTable,
			const Database::Table& managementTable)
		{
			utils::ref_count_ptr<core::rules::rules_data_and_types_interface> rule_data;
			if (false == rules::rules_data_and_types::create(
				static_cast<core::database::dataset_interface*>(dataset),
				static_cast<core::database::table_interface*>(inputTable),
				static_cast<core::database::table_interface*>(enableTable),
				static_cast<core::database::table_interface*>(existenceTable),
				static_cast<core::database::table_interface*>(outputTable),
				static_cast<core::database::table_interface*>(managementTable),
				&rule_data))
			{
				throw std::runtime_error("error on create RulesDataAndTypes");
			}

			return RulesDataAndTypes(rule_data);

		}
	};
}