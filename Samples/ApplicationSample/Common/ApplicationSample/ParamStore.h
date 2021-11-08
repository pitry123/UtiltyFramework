#ifndef _PARAM_STORE_H_
#define _PARAM_STORE_H_

#include <iostream>
#include <sstream>
#include <Common.h>
#define PARAM_ARRAY_LENGTH 128

class ParamStore 
{
public:
	enum XML_StoreDBEnum
	{
		DEBUG_ENV,                  // t:CommonTypes::MonitorParams
		LOG1,						//t:ParamStore::LogData
		LOG2,						//t:ParamStore::LogData
		GENERAL_LOG,				//t:ParamStore::LogData
		NUM_OF_PARAM_STORE_DB_XML
	};

	struct LogData
	{
		char logName[256];
		char logSevirty[16];
	};
};
#endif // _PARAM_STORE_H_

