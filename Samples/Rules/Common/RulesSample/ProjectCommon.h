#pragma once

class ProjectCommon
{
public:

	// NEW_PROCESS_ADD_HERE : If the process has a DB then add it here
	enum MonitorDBIndex
	{

		// -------- [ Rules DBs ] -------- 
		RulesInputDBEnum,		            //                              g:Rules
		RulesOutputDBEnum,		            //                              g:Rules
		RulesExistenceDBEnum,	            //                              g:Rules
		RulesEnabledDBEnum,					//								g:Rules
		RulesManagementDBEnum,				//								g:Rules

		NumberOfDatabases
	};

	enum ProjectLogID
	{

		NUM_OF_PROJECT_LOGS
	};

};

