#pragma once


class ProjectCommon
{
public:
	enum MonitorDBIndex
	{
		InternalErrorsDB,													//g:DeviceX
		ErrorInfoDB,														//r:ErrorMetadata g:Errors/metadata
		ErrorGroupsDB,														//g:Errors
		ErrorStoreDBEnum,													//g:Errors
		ErrorDBEnum,														//g:Errors			
		RulesInputDBEnum,												    //g:Rules
		RulesOutputDBEnum,		                                            //g:Rules
		RulesExistenceDBEnum,	                                            //g:Rules
		RulesEnabledDBEnum,					  								//g:Rules
		RulesManagementDBEnum,				  								//g:Rules
		XML_StoreDBEnum,					  								//g:ParamStore
		NumberOfDatabases
	};

	enum ProjectLogID
	{
		NUM_OF_PROJECT_LOGS
	};

	
};
