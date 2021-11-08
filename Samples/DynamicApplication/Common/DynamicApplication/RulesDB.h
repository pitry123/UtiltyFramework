#pragma once
namespace Rules
{
using namespace rules;
	class RulesDB
	{
		public:
			// Enum for input data
			enum RulesInputDBEnum
			{

				iTestData,			//ApplicationSampleDB::inputEnum			t:int						def:INTPUT1
				iOptions,			//ApplicationSampleDB::MyEnum			t:int						def:OPTION1

				RULES_INPUT_DB_SIZE
			};

			//Enum for output data
			enum RulesOutputDBEnum
			{

				RULES_OUTPUT_DB_SIZE
			};

			//Enum for Rules Existence data
			enum RulesExistenceDBEnum
			{
				rRule1,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rDynamic_Rule1,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rDynamic_Rule2,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rDynamic_Rule3,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rDynamic_Rule4,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID

				RULES_EXISTENCE_DB_SIZE
			};

			//Enum for Rules Enabled data
			enum RulesEnabledDBEnum
			{
				enRule1,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enDynamic_Rule1,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enDynamic_Rule2,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enDynamic_Rule3,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enDynamic_Rule4,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE

				RULES_ENABLED_DB_SIZE
			};

			enum RulesManagementDBEnum
			{
				RELOAD_RULES,                //t:bool
				RULES_MANAGEMENT_DB_SIZE
			};
	};
}