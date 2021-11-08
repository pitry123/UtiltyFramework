#pragma once
class ProjectCommon
{
public:
	enum MonitorDBIndex
	{
		GreenRedTable1,	  //r:GreenRedTable g:ApplicationSample
		GreenRedTable2,	  //r:GreenRedTable g:ApplicationSample
		RemoteAgentStatusDBEnum,       //g:RemoteAgent
		XML_StoreDBEnum,  //g:ParamStore
		NumberOfDatabases
	};

	enum ProjectLogID
	{
		NUM_OF_PROJECT_LOGS
	};
};