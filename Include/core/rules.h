#pragma once
#include <core/ref_count_interface.h>
#include <core/database.h>

namespace core
{
	namespace rules
	{
		class DLL_EXPORT rule_func_callback_interface : public core::ref_count_interface
		{
		public:
			virtual bool execute(double& retval) = 0;
			virtual const char* name() = 0;
		};

		/// rules_data_and_types_interface. delivered to a rules dispatcher in order to access rows or enumerations by name
		/// It is not expected that implementation need to be thread safe
		/// @date	09/01/2020
		class DLL_EXPORT rules_data_and_types_interface : public core::ref_count_interface
		{
		public:
			virtual ~rules_data_and_types_interface() = default;

			/// Queries row by string
			/// @date	06/01/2020
			/// @param 		   	row_string	The row name.
			/// @param [out]	row		  	If non-null, the row.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_row_by_string(const char* row_string, core::database::row_interface** row) = 0;

			/// Queries rule existence row
			/// @date	06/01/2020
			/// @param 		   	rule_id	Identifier for the rule.
			/// @param [in,out]	row	   	If non-null, the row.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_rule_existence_row(size_t rule_id, core::database::row_interface** row) = 0;

			/// Adds a rule existence row by 'row_name'. it is not expected the function to be thread safe
			/// @date	06/01/2020
			/// @param	rule_id 	Identifier for the rule.
			/// @param	row_name	Name of the row.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_rule_existence_row(size_t rule_id, const char* row_name) = 0;

			/// Queries rule enable row: retrive the enable row of the to allow setting the rule enable/disable
			/// @date	06/01/2020
			/// @param 		   	rule_id	Identifier for the rule.
			/// @param [out]	row	   	If non-null, the row.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_rule_enable_row(size_t rule_id, core::database::row_interface** row) = 0;

			/// Adds a rule enable row by 'row_name'. it is not expected the function to be thread safe
			/// @date	06/01/2020
			/// @param	rule_id 	Identifier for the rule.
			/// @param	row_name	Name of the row.
			/// @return	True if it succeeds, false if it fails.
			virtual bool add_rule_enable_row(size_t rule_id, const char* row_name) = 0;

			/// Queries reload rules row, this row is one for all rows in dataset, and trigger it will cause 
			/// the rule xml to be reloaded.
			/// @date	06/01/2020
			/// @param [in,out]	row	If non-null, the row.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_reload_rules_row(core::database::row_interface** row) = 0;

			/// Gets an enumeration by name
			/// @date	06/01/2020
			/// @param 		   	enum_string	The enum string.
			/// @param [out]	val		   	The value.
			/// @return	True if it succeeds, false if it fails.
			virtual bool get_enumeration(const char* enum_string, int64_t& val) = 0;

			/// Gets an enumeration by name and by the enum name (of the type)
			/// @date	06/01/2020
			/// @param 		   	enum_name_val 	The enum name value.
			/// @param 		   	enum_type_name	Name of the enum type.
			/// @param [out]	val			  	The value.
			/// @return	True if it succeeds, false if it fails.
			virtual bool get_enumeration(const char* enum_name_val,const char* enum_type_name, int64_t& val) = 0;

			/// Queries a function: Get functions that retrieve data from a specific row
			/// @date	06/01/2020
			/// @param 		   	func_string	The function string.
			/// @param [in,out]	callback   	If non-null, the callback.
			/// @return	True if it succeeds, false if it fails.
			virtual bool query_function(const char* func_string, rule_func_callback_interface** callback) = 0;
		};
	}
}