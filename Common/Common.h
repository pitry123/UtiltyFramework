#pragma once
#include <stdint.h>

#define INF_COMMON_BUFFER_MAX_SIZE 256

//Common Structure for all infrastructure usages
namespace Common
{
	static constexpr uint16_t BUFFER_MAX_SIZE = INF_COMMON_BUFFER_MAX_SIZE;
	class CommonTypes
	{
	public:
		enum GenericLogID
		{
			NUM_OF_GENERIC_LOGS
		};

		enum GenericUnitID
		{
			NUM_OF_GENERIC_UNITS,
		};

		enum ActiveInactiveEnum
		{
			ACTIVE = 0,
			INACTIVE = 1,
		};

		enum BoolEnum
		{
			BOOL_NONE = -1,
			BOOL_FALSE = 0,
			BOOL_TRUE = 1
		};

		enum ButtonEnum
		{
			BUTTON_NONE = -1,
			BUTTON_RELEASED = 0,
			BUTTON_PRESSED = 1
		};

		enum BitEnum
		{
			BIT_NONE = -1,
			BIT_FAIL = 0,
			BIT_OK = 1
		};

		enum CommunicationEnum
		{
			COMMUNICATION_NONE = -1,
			COMMUNICATION_FAILURE = 0,
			COMMUNICATION_OK = 1
		};

		enum EnabledEnum
		{
			ENABLED_NONE = -1,
			ENABLED_FALSE = 0,
			ENABLED_TRUE = 1
		};

		enum LedModeEnum
		{
			LED_MODE_NONE = -1,
			LED_MODE_OFF = 0,
			LED_MODE_ON = 1,
			LED_MODE_BLINK = 2
		};

		enum OnOffStateEnum
		{
			STATE_ERROR = -2,
			STATE_NONE = -1,
			STATE_OFF = 0,
			STATE_ON = 1
		};

		enum OpenClosedStateEnum
		{
			OPEN_CLOSED_STATE_ERROR = -2,
			OPEN_CLOSED_STATE_NONE = -1,
			STATE_CLOSED = 0,
			STATE_OPENED = 1
		};

		enum ProcessEnum
		{
			PROCESS_ERROR = -2,
			PROCESS_NONE = -1,
			PROCESS_STOPPED = 0,
			PROCESS_RUNNING = 1,
			PROCESS_FINISHED_FAIL = 2,
			PROCESS_FINISHED_OK = 3
		};

		enum StatusEnum
		{
			STATUS_NONE = -1,
			STATUS_FAIL = 0,
			STATUS_OK = 1
		};

		enum StartStopCmdEnum
		{
			CMD_NONE = -1,
			CMD_STOP,
			CMD_START
		};

		enum SystemStatesEnum
		{
			SYSTEM_STATE_NONE = -1, // A value for -1 must be used
			SYSTEM_STATE_IDLE,
			SYSTEM_STATE_CONNECTING,
			SYSTEM_STATE_CONNECTED,
			SYSTEM_STATE_FAILURE,
			SYSTEM_STATE_DISCONNECTING,
			SYSTEM_STATE_DISCONNECTED,

			NUM_OF_SYSTEM_STATES
		};

		enum ValidityEnum
		{
			VALIDITY_INDETERMINATE,
			VALID,
			NOT_VALID
		};

		enum TimerEnum
		{
			PERIODIC_1_MILLI	= 1,
			PERIODIC_5_MILLI	= 5,
			PERIODIC_10_MILLI	= 10,
			PERIODIC_50_MILLI	= 50,
			PERIODIC_100_MILLI	= 100,
			PERIODIC_200_MILLI	= 200,
			PERIODIC_500_MILLI	= 500,
			PERIODIC_1_SECOND	= 1000,
			PERIODIC_2_SECOND	= 2000,
			PERIODIC_5_SECOND	= 5000
		};

		enum ParityEnum
		{
			NONE = 0,
			ODD = 1,
			EVEN = 2,
			MARK = 3,
			SPACE = 4,
		};

		enum StopBitsEnum
		{
			STOPBITSNONE = 0,
			ONE = 1,
			TWO = 2,
			ONEPOINTFIVE = 3,
		};

		enum LoggingTypeEnum
		{
			HOST_LOG_NO_LOG = 0,
			HOST_LOG_LOGMSG = 1,
			HOST_LOG_PRINTF = 2
		};

		enum UnitLogSeverityEnum
		{
			LOG_ERRORS_MSGS = 0,
			LOG_WARNING_MSGS = 1,
			LOG_INFO_MSGS = 2,
			LOG_DEBUG_MSGS = 3,
			LOG_TRASH_MSGS = 4,
			MAX_SEVERITY_NUMBER = 5
		};

		enum ErrorStatusEnum
		{
			NOT_EXIST = 0x01,
			EXIST = 0x02,
			PEND_TO_EXIST = 0x04,
			VANISHED = 0x08,
			REPEATED = 0x10,
			PEND_TO_REPEAT = 0x20
		};

		enum GeneralEnum
		{
			STRING_ERROR_DESCRIPTION_LENGTH = 256,
			STRING_FILE_FULL_PATH_LENGTH = 50,
			MAX_NUMBER_OF_GROUPS = 32,
			STRING_NUMBER_OF_GROUPS_LENGTH = 256,
			STRING_EXISTING_ERRORS_IN_GROUP_LENGTH = 400
		};

#pragma pack(1)

		//Errors
		struct CounterStruct
		{
			uint16_t								currentCount;
			bool									Valid;
		};

		// The struct used in various error data bases
		struct ErrorStatusStruct
		{
			ErrorStatusEnum						status;			// t:int def:NOT_EXIST
			int64_t							  timeStamp;
			Common::CommonTypes::BoolEnum		pendable;			// t:int
			int								    errorKey;
			CounterStruct						statistics;
		};

		// The struct used in the 'ErrorManagerDB' as the data of the groups
		struct ErrorGroupStruct
		{
			int					numOfExistingErrors;
			char					lastError[STRING_ERROR_DESCRIPTION_LENGTH];
		};

		struct ErrorIndicationCounterStruct
		{
			uint16_t	counterForExist;
			uint16_t	incPerExistIndication;
			uint16_t	decPerExistIndication;
		};

		struct ExistingErrorsInGroupStruct
		{
			char	pchData[STRING_EXISTING_ERRORS_IN_GROUP_LENGTH];
		};

		// Stores a 128-bit UUID as 2 64-bit integers (low and high).
		struct uuidTypeStruct
		{
			int64_t  nMSB;
			int64_t  nLSB;
		};

		struct CHARBUFFER
		{
			char buffer[INF_COMMON_BUFFER_MAX_SIZE];
		};

		struct IPEndPointStruct
		{
			char IPAddress[16];
			uint16_t port;
		};

		struct DelimiterProtocolStruct
		{
			char startDelimiter[16];
			char endDelimiter[16];
		};

		struct FixedLengthProtocolStruct
		{
			int32_t nLength;
		};

		struct VariableLengthProtocolStruct
		{
			int32_t nSizeOfDataSize;
			int32_t nPlaceOfDataSize;
			int32_t nConstatSizeAddition;
			bool bAbsoluteDataSize;
			int32_t nMaxDataSize;
		};
		
		struct ClientStruct
		{
			CommonTypes::IPEndPointStruct stRemoteIP;
			CommonTypes::IPEndPointStruct stLocalIP;
		};

		struct ClientDelimiterProtocolStruct
		{
			CommonTypes::ClientStruct stClientStruct;
			CommonTypes::DelimiterProtocolStruct stDelimeterProtocol;
		};

		struct ClientFixedLengthProtocolStruct
		{
			CommonTypes::ClientStruct stClientStruct;
			CommonTypes::FixedLengthProtocolStruct stFixedLengthProtocol;
		};

		struct ClientVariableLengthProtocolStruct
		{
			CommonTypes::ClientStruct stClientStruct;
			CommonTypes::VariableLengthProtocolStruct stVariableLengthProtocol;
		};

		struct SerialPortStruct
		{
			char m_strPortName[32];
			uint32_t m_nBaudRate;
			CommonTypes::ParityEnum m_eParity;		//t:int
			int32_t m_nDataBits;
			CommonTypes::StopBitsEnum m_eStopBits;	//t:int
		};

		struct SerialPortDelimiterProtocolStruct
		{
			CommonTypes::SerialPortStruct stSerialPort;
			CommonTypes::DelimiterProtocolStruct stDelimiterProtocol;
		};

		struct SerialPortFixedLengthProtocolStruct
		{
			CommonTypes::SerialPortStruct stSerialPort;
			CommonTypes::FixedLengthProtocolStruct stFixedLengthProtocol;
		};
		
		struct SerialPortVariableLengthProtocolStruct
		{
			CommonTypes::SerialPortStruct stSerialPort;
			CommonTypes::VariableLengthProtocolStruct stVariableLengthProtocol;
		};

		struct ServerStruct
		{
			CommonTypes::IPEndPointStruct stLocalIP;
		};
		
		struct MonitorParams
		{
			char remoteAddress[16];
			uint16_t remotePort;
			char localAddress[16];
			uint16_t  localPort;
		};		

		struct UnitLogConfiguration
		{
			char				LOG_UNIT_NAME[32];
			LoggingTypeEnum     SHELL_PRINT_TYPE;		//t:int
			bool				TRACE_LEVEL_ENABLED[MAX_SEVERITY_NUMBER];
			bool				LOG_TO_UNIT_FILE;
			bool				LOG_TO_SYSTEM_FILE;
			bool				USE_PRESET_STRING;
			bool				ENABLE_TIMESTAMP;
			bool				ENABLE_DATE_TIME;
		};

		struct UnitLogDBNotifyData
		{
			UnitLogSeverityEnum TRACE_LEVEL_TYPE;  	//UnitLogSeverityEnum t:int
			unsigned long		LOG_TIMESTAMP_MILLI;
			char				TRACE_ADDITIONAL_INFO[32];
			char                TRACE_STR[400];
		};

#pragma pack()
	};
}