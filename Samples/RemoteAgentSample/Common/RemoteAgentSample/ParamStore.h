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
		DEBUG_EN_MEM,              // t:CommonTypes::MonitorParams
		REMOTE_AGENT_DEBUG_ENV,     // t:CommonTypes::MonitorParams   
		NUM_OF_PARAM_STORE_DB_XML
	};


	enum INI_StoreDBEnum
	{
		FACTORY_SETTING_PATH,               // t:CommonTypes::CHARBUFFER
		DEVELOPER_SETTING_PATH,             // t:CommonTypes::CHARBUFFER
		USER_SETTING_PATH,                  // t:CommonTypes::CHARBUFFER
		
		NUM_OF_PARAM_STORE_DB_INI
	};

	struct DDSParams
	{
		int	 DomainId;
		char qosFilePathWin[256];
		char qosFilePathLinux[256];

		DDSParams() :
			DomainId(0)
		{}
	};

};
#endif // _PARAM_STORE_H_

