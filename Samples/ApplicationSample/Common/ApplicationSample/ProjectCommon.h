#pragma once

class DBLoggerCommon
{
public:
	enum LogsDBEnum
	{
		LOG_UNIT_1, //t:CommonTypes::UnitLogDBNotifyData
		LOG_UNIT_2, //t:CommonTypes::UnitLogDBNotifyData
		LOGGING_DATA_DB_SIZE
	};

	// Add here all the config log DB fields
	enum LoggerConfigDBEnum
	{
		CONF_LOG_UNIT_1, //t:CommonTypes::UnitLogConfiguration
		CONF_LOG_UNIT_2, //t:CommonTypes::UnitLogConfiguration
		CONFIG_LOGGING_DATA_DB_SIZE
	};
};

class ProjectCommon
{
public:
	enum MonitorDBIndex
	{
		LogsDBEnum,							  //g:Logger
		LoggerConfigDBEnum,					  //g:Logger

		GreenRedTable,						  //g:ApplicationSample
		XML_StoreDBEnum,					  //g:ParamStore
		NumberOfDatabases
	};

	enum ProjectLogID
	{
		NUM_OF_PROJECT_LOGS
	};
};