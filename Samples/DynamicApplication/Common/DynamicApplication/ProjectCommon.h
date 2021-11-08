#pragma once
class ProjectCommon
{
public:
	enum MonitorDBIndex
	{
		GreenRedTable,			//g:ApplicationSample
		XML_StoreDBEnum,		//g:ParamStore
		DynamicTable,			//g:dynamic
		RulesInputDBEnum,		//g:rules
		RulesOutputDBEnum,		//g:rules
		RulesExistenceDBEnum,	//g:rules
		RulesEnabledDBEnum,		//g:rules
		RulesManagementDBEnum,  //g:rules
		NumberOfDatabases
	};

	enum ProjectLogID
	{
		NUM_OF_PROJECT_LOGS
	};
};