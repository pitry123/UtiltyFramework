#ifndef _COMMON__COMMON__H_
#define _COMMON__COMMON__H_

typedef unsigned long       DWORD;

class Common
{

public:

	typedef int UnitIDEnum;

	typedef enum 
	{
		COMM_CHAN_UNIT_ID			= -1,//IMPORTANT!! : Communication channels should be built with this callback ID
		CALLBACK_TIMER_UID			= 0,
		ATT_UID,
		CONTROL_UID,
		DATA_STORE_UID,
		XML_STORE_UID,
		DISPLAY_GUNNER_UID,
		DISPLAY_COMMANDER_UID,
		ERROR_MANAGER_UID,
		LRF_DB_UID,
		MONITOR_MANAGER_UID,
		RETICLE_MANAGER_UID,
		RULES_UID,
		SENSORS_UID,
		SOFTWARE_VERSION_UID,
		LOGGER_MANAGER_UID,
		IO_SIGNALS_UID,
		RULES_UPDATER_UID,
		SAGEM_UID,
		ALERT_MANAGER_UID,

		NUM_OF_GENERIC_UNITS,
		NUM_OF_UNIT_IDS = 100
	} GenericUnitID;
	

	typedef enum GenericLogID
	{
		LOG_SYSTEM_FAILURE,
		LOG_DATA_STORE,
		LOG_XML_STORE,
		LOG_RULES,
		LOG_RETICLE,
		LOG_SENSORS,
		LOG_SOFT_VER,
		LOG_ERROR_MANAGER,
		LOG_CONTROL,
		LOG_ATT,
		LOG_TRACKER,
		LOG_ASSERT_CHECKER,
		LOG_ELMO_MANAGER,
		LOG_COMMUNICATION,
		LOG_IO_SIGNALS,
		LOG_SCREENS,
		LOG_SAGEM,
		LOG_ALERTS,

		NUM_OF_GENERIC_LOGS

	}GenericLogID;
	
	typedef enum
	{
		LOG_UNIT_DISPLAY			= 11
	}REMOVE;


	typedef struct BspConvStruct
	{
		DWORD		eLocalReg;
		DWORD		eBspRegAddr;
		char		chName[64];
	} BSP_CONV_STRUCT;

	typedef enum
	{
		NA		= 0,	// Not Applicable
		TBD		= 1		// TO be Defined
	}
	GeneralStatusEnum;

	
	typedef enum
	{		
		COMMUNICATION_OK		= 0,
		COMMUNICATION_FAILURE	= 1,
	}
	CommStatusEnum;

	typedef enum
	{
		SENSOR_FAIL	= 0,
		SENSOR_OK	= 1
	}
	SensorStatusEnum;
	
	enum DiscreteStateEnum
	{
		DISCRETE_ERROR	= -1,
		DISCRETE_OFF	= 0,
		DISCRETE_ON		= 1
	};

	enum SwTriStatusEnum
	{
		TR_ERROR=-1,
		MIDDLE	= 0,
		UP		= 1,
		DOWN	= 2
	};

	enum SwBiStatusEnum
	{
		BI_ERROR=-1,
		RELEASED= 0,
		PRESSED	= 1
	};
	
	enum BitStatusEnum
	{
		BIT_FAIL,
		BIT_OK,
		BIT_NOT_VALID
	};

	enum EnableDisableEnum
	{
		DISABLE,
		ENABLE
	};

	enum OperatorsEnum
	{
		NO_OPERATOR=-1,
		GUNNER_OPERATOR=0,
		COMMANDER_OPERATOR=1,
		NUM_OF_OPERATORS=2

	};

	// Insert here only dispatcher ID (might be more than one per system/unit ID
	typedef enum
	{
		DATA_STORE_NUM_OF_DISPATCHERS
	}
	DataStoreDispatcherIdsEnum;

private:

	Common();

};

#endif

