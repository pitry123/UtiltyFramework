#pragma once
#include <core/application.h>
#include <core/database.h>
#include <core/rules.h>
#include <core/parser.h>
#include <files/xml_file_interface.h>

namespace rules
{

	class DLL_EXPORT rules_data_and_types : public core::rules::rules_data_and_types_interface
	 {
	 public:
		virtual ~rules_data_and_types() = default;

		/// Creates a rules data and type interface externally, This function should be implemented 
		/// by the generated code. If generated code is used. if not calling to this function will cause unresolved error
		/// @date	25/12/2019
		/// @param [in]	input_table				If non-null, the input table.
		/// @param [in]	enable_table			If non-null, the enable table.
		/// @param [in]	existence_table			If non-null, the existence table.
		/// @param [in]	output_table			If non-null, the output table.
		/// @param [in]	management_table		If non-null, the management
		/// 	table.
		/// @param [out]	rules_data_and_types	If non-null, instance to rules data and type class
		/// 	the rules data ands.
		/// @return	True if it succeeds, false if it fails.
		static bool create(
			core::database::dataset_interface*		dataset,
			core::database::table_interface*		input_table,
			core::database::table_interface*		enable_table,
			core::database::table_interface*		existence_table,
			core::database::table_interface*		output_table,
			core::database::table_interface*		management_table,
			core::rules::rules_data_and_types_interface**		rules_data_and_types);
		
	};

	class DLL_EXPORT rules_dispatcher : public core::application::runnable_interface
	{
	public:
		virtual ~rules_dispatcher() = default;

		virtual int add_rule(files::xml_element_interface* rule_element) = 0;
		/// Creates a new rules_dispatcher, this function assumes the rules_data_and_types_interface was created externally (probebly by a generated code)
		/// @date	25/12/2019
		/// @param 		   	file_path				Full pathname of the file.
		/// @param [in,out]	rules_data_and_types	If non-null, list of types of
		/// 	the rules data ands.
		/// @param [in,out]	runnable				If non-null, the runnable.
		/// @return	True if it succeeds, false if it fails.
		static bool create(
			const char* file_path,
			core::rules::rules_data_and_types_interface*	rules_data_and_types,
			rules_dispatcher** runnable);

		/// Creates a new rules_dispatcher. This function creates its own generic rules_data_and_types_interface internally.
		/// it is only valid if load schema is used, otherwise use the generated code 
		/// @date	25/12/2019
		/// @param 		   	file_path			Full pathname of the file.
		/// @param [in]	input_table			If non-null, the input table.
		/// @param [in]	enable_table		If non-null, the enable table.
		/// @param [in]	existence_table 	If non-null, the existence table.
		/// @param [in]	output_table		If non-null, the output table.
		/// @param [in]	management_table	If non-null, the management table.
		/// @param [in]	store				If non-null, the store.
		/// @param [out]	runnable			If non-null, return the runnable.
		/// @return	True if it succeeds, false if it fails.
		static bool create(
			const char* file_path,
			core::database::table_interface * input_table,
			core::database::table_interface * enable_table,
			core::database::table_interface * existence_table,
			core::database::table_interface * output_table,
			core::database::table_interface * management_table,
			core::parsers::binary_metadata_store_interface* store,
			rules_dispatcher** runnable);

		static bool create(const char* file_path,
			core::database::dataset_interface* dataset,
			core::parsers::binary_metadata_store_interface* store,
			rules_dispatcher** runnable);

	};

	class RulesDefs
	{
	public:

		enum RulesExistence
		{
			RULE_TRUE,
			RULE_FALSE,
			RULE_NOT_VALID
		};

		enum RulesEnabled
		{
			RULE_ENABLE,
			RULE_DISABLE,
			RULE_ENABLE_NOT_VALID
		};
	};
}