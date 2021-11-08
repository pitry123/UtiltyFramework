#ifndef __COMMON_DEFS__H__
#define __COMMON_DEFS__H__

class CommonDbDefs
{

public:
	
	//--------------------- From DB's ---------------------------------------//
	
	enum OperatorTypesEnum
	{
		OPERATOR_NONE = -1,
		OPERATOR_GUNNER,
		OPERATOR_COMMANDER,

		NUM_OF_OPERATOR_TYPES
	};
	
	enum UserRequestEnum
	{
		USER_REQUEST_NONE				= -1,
		USER_REQUEST_ENTER_PROCESS		= 0,
		USER_REQUEST_EXIT_PROCESS		= 1,
		USER_REQUEST_ABORT_PROCESS		= 2,
		USER_REQUEST_RESET_PROCESS		= 3
	};

	enum MobilityTypeEnum
	{
		MOBILITY_NONE,
		MOBILITY_VELOCITY,
		MOBILITY_ACCELERATION
	};

	enum ActivityStatusEnum
	{
		ACTIVITY_SUSPENDED,
		ACTIVITY_READY,
		ACTIVITY_IN_PROCESS,
		ACTIVITY_END_SUCCESS,
		ACTIVITY_END_FAIL,
		ACTIVITY_END_ABORT
	};

	enum ActivityCmdEnum
	{
		ACTIVITY_SUSPEND,
		ACTIVITY_WAKE,
		ACTIVITY_START,
		ACTIVITY_STOP_SUCCESS,
		ACTIVITY_STOP_FAIL,
		ACTIVITY_STOP_ABORT
	};

	enum CommunicationEnum
	{
		COMMUNICATION_NONE		= -1,
		COMMUNICATION_OK		= 0,
		COMMUNICATION_FAILURE	= 1
	};	
	
	enum BoolEnum
	{
		BOOL_NONE			= -1,
		BOOL_FALSE			= 0,
		BOOL_TRUE			= 1
	};

	enum StatusEnum 
	{
		STATUS_NONE			= -1,
		STATUS_FAIL			= 0,
		STATUS_OK			= 1
	};
	


	enum TriSwitchEnum
	{
		TRI_SWITCH_ERROR	= -2,
		TRI_SWITCH_NONE		= -1,
		TRI_SWITCH_MIDDLE	= 0,
		TRI_SWITCH_UP		= 1,
		TRI_SWITCH_DOWN		= 2
	};

	enum ButtonEnum
	{
		BUTTON_NONE			= -1,
		BUTTON_RELEASED		= 0,
		BUTTON_PRESSED		= 1
	};



	enum EnabledEnum
	{
		ENABLED_NONE 		= -1,
		ENABLED_FALSE		= 0,
		ENABLED_TRUE		= 1
	};

	enum MobileEnum
	{
		MOBILE_ERROR		= -2,
		MOBILE_NONE 		= -1,
		MOBILE_FALSE		= 0,
		MOBILE_TRUE			= 1
	};
	
	enum ModeEnum
	{
		MODE_ERROR			= -2,
		MODE_NONE 			= -1,
		MODE_MANUAL			= 0,
		MODE_POWER			= 1,
		MODE_STAB			= 2

	};

	enum ProcessEnum
	{
		PROCESS_ERROR				= -2,
		PROCESS_NONE 				= -1,
		PROCESS_STOPPED				= 0,
		PROCESS_RUNNING				= 1,
		PROCESS_FINISHED_FAIL		= 2,
		PROCESS_FINISHED_OK			= 3
	};

	enum BitEnum
	{
		BIT_NONE	= -1,
		BIT_FAIL	= 0,
		BIT_OK		= 1
	};

	enum BitTypeEnum
	{
		BIT_TYPE_NONE				= -1,
		BIT_TYPE_INITIALIZATION		= 0,
		BIT_TYPE_PERIODIC			= 1,
		BIT_TYPE_INITIATED			= 2
	};

	enum TrackEnum
	{
		TRACK_NONE 			= -1,
		TRACK_NOT_AQUIRE	= 0,
		TRACK_AQUIRE		= 1,
		TRACK_COAST			= 2
	};

	enum RangeTypeEnum
	{
		RANGE_TYPE_NONE 		= -1,
		RANGE_TYPE_MANUAL,
		RANGE_TYPE_LASER,

		NUM_OF_RANGE_TYPES
	};
	
	enum OnOffStateEnum
	{
		STATE_ERROR   =-2,
		STATE_NONE    =-1,
		STATE_OFF     =0,
		STATE_ON      =1
	};

	

	enum FireStatusEnum
	{
		FIRE_STATUS_NONE	 = -1,
		FIRE_STATUS_DISABLED =  0,
		FIRE_STATUS_ENABLED  =  1
	};

	enum UnitStatusEnum
	{
		UNIT_STATUS_NONE		 = -1,
		UNIT_STATUS_ON			 =	0,
		UNIT_STATUS_OPERATOR_OFF =	1,
		UNIT_STATUS_MANUAL		 =	2,
		UNIT_STATUS_COMPUTER_OFF =	3
	};



	enum LedModeEnum
	{
		LED_MODE_NONE	= -1,
		LED_MODE_OFF	= 0,
		LED_MODE_ON		= 1,
		LED_MODE_BLINK	= 2
	};

	enum OpenClosedStateEnum
	{
		OPEN_CLOSED_STATE_ERROR = -2, 
		OPEN_CLOSED_STATE_NONE = -1,
		STATE_CLOSED = 0,
		STATE_OPENED = 1
	};

	enum InputOutputModeEnum
	{
		IO_MODE_NONE= -1,
		IO_MODE_INPUT,
		IO_MODE_OUTPUT
	};

	enum AngleUnitsEnum
	{
		ANGLE_UNITS_UNDEFINED = -1,
		ANGLE_UNITS_RADS,
		ANGLE_UNITS_MILLS,
		ANGLE_UNITS_MRADS,
		ANGLE_UNITS_DEGS
	};

	enum PositionUnitsEnum
	{
		POSITION_UNITS_UNDEFINED = -1,
		POSITION_UNITS_PIXELS

	};

	enum BoresightModeEnum
	{
		BS_MODE_NOT_IN_PROGRESS,
		BS_MODE_IN_PROGRESS,
		BS_MODE_EXIT_SUCCESS,
		BS_MODE_EXIT_FAIL,
		BS_MODE_EXIT_FAIL_TRV,
		BS_MODE_EXIT_FAIL_ELV,
		BS_MODE_EXIT_ABORTED,
	};

	enum ReticleBoresightActionEnum
	{
		BS_ACTION_NONE,
		BS_ACTION_START,
		BS_ACTION_ENTER,
		BS_ACTION_ABORT,
		BS_ACTION_RESET,
		BS_ACTION_BACK,
	};

	enum ReticleMovementEnum
	{
		RETICLE_MOVEMENT_NO_MOVEMENT = -1,
		RETICLE_MOVEMENT_STEP_UP,
		RETICLE_MOVEMENT_STEP_DOWN,
		RETICLE_MOVEMENT_STEP_LEFT,
		RETICLE_MOVEMENT_STEP_RIGHT
	};

	enum ValidityEnum
	{
		VALIDITY_INDETERMINATE,
		VALID,
		NOT_VALID		
	};

	enum RangeMeasureStatusEnum
	{
		RANGE_MEASURE_NONE 			= -1,
		RANGE_MEASURE_NO_RANGE		= 0,
		RANGE_MEASURE_SINGLE		= 1,
		RANGE_MEASURE_MULTIPLE		= 2,
		RANGE_MEASURE_SKY_LASING	= 3,
		RANGE_MEASURE_ERROR			= 4,

		NUM_OF_RANGE_MEASURE_STATUSES
	};

	enum BrakesOpenCloseEnum
	{
		BRAKES_NONE				=	-1,
		BRAKES_CLOSE,
		BRAKES_OPEN
	};

	enum StartStopCmdEnum
	{
		CMD_NONE				=	-1,
		CMD_STOP,
		CMD_START
	};

	enum FunctionModeEnum
	{
		MODE_UNSUPPORTED		= -2,		
		MODE_UNKNOWN			= -1,
		MANUAL_MODE,
		AUTO_MODE
	};

	enum UserEnum
	{
		USER_NONE				= -1,
		USER_COMMANDER,
		USER_GUNNER,

	};

	enum InsModeTypeEnum
	{
		INS_NONE = -1,
		INS_OFF = 0x0,
		INS_STANDARD_ALIGN = 0x1,
		INS_STORED_HEADING_ALIGN = 0x2,
		INS_STANDBY = 0x3,
		INS_UPDATE_POSITION = 0x4,
		INS_GPS_ALIGNMENT = 0x5
	};

	enum ActionEnum
	{
		ACTION_NONE = -1,
		ACTION_STOP = 0x0,
		ACTION_EXIT = 0x1,
		ACTION_START = 0x2

	};

	enum VelocityEnum
	{
		VELOCITY_NONE		= -1,
		VERY_SLOW_VELOCITY	= 0x1,
		SLOW_VELOCITY		= 0x2,
		NORMAL_VELOCITY		= 0x3,
		FAST_VELOCITY		= 0x4,
		VERY_FAST_VELOCITY	= 0x5,

		NUM_OF_VELOCITIES
	};

	enum SystemTypeEnum
	{
		UNKNOWN_SYSTEM		= -1,
		OBSERVATION_SYSTEM	= 0x1,

	};

	enum DayImageColorEnum
	{
		IMAGE_COLOR_NONE	= -1,
		BW_IMAGE			= 0x0,
		COLOR_IMAGE			= 0x1
	};

	enum FreezeEnum
	{
		FREEZE_NONE			= -1,
		NO_FREEZE			= 0x0,
		FREEZE				= 0x1
	};

	enum FusionTiHotPartsColorEnum
	{
		FUSION_TI_NONE		= -1,
		FUSION_TI_BLUE		= 0x1,
		FUSION_TI_GREEN		= 0x2,
		FUSION_TI_RED		= 0x3,
		FUSION_TI_YELLOW	= 0x4,
		FUSION_TI_DEFAULT	= 0x5,
		FUSION_TI_NO_COLOR   = 0x6
	};

	enum InvertImageEnum
	{
		INVERT_NONE			= -1,
		NORMAL_INVERT		= 0x1,
		INVERT_UP_DOWN		= 0x2,
		INVERT_LEFT_RIGHT	= 0x3,
		INVERT_BOTH_AXES	= 0x4,
		INVERT_NOT_AVAILABLE = 0xff
	};

	enum AveragingEnum
	{
		AVERAGING_NONE				= -1,
		AVERAGING_OFF				= 0x0,
		AVERAGING_LOW_FREQUENCY		= 0x1,
		AVERAGING_HIGH_FREQUENCY	= 0x2,
		AVERAGING_NOT_AVAILABLE		= 0xff
	};

	enum CameraDisplayFunctionsEnum
	{
		FUNCTION_NONE			= -1,
		FUNCTION_ERASE			= 0x0,
		FUNCTION_DISPLAY		= 0x1,
		FUNCTION_KEEP			= 0x2,
	};

	enum VideoConfigurationEnum
	{
		VIDEO_CONFIGURATION_NONE = -1,
		VIDEO_CONFIGURATION_DAY,
		VIDEO_CONFIGURATION_TI,
		VIDEO_CONFIGURATION_LRF,
		VIDEO_CONFIGURATION_SLA,
		VIDEO_CONFIGURATION_ATT,

		NUM_OF_VIDEOS_CONFIGURATION
	};

	enum LanguageEnum
	{
		LANGUAGE_ENGLISH,
		LANGUAGE_OTHER
	};

	//--------------------- From Rules ---------------------------------------//

	enum ConditionEnum
	{
		CONDITION_ERROR			= -2,
		CONDITION_NONE			= -1,
		CONDITION_NO			= 0,
		CONDITION_YES			= 1
	};

	enum RangeSelectionEnum
	{
		RANGE_SELECTION_NONE = -1,
		RANGE_SELECTION_FIRST = 0,
		RANGE_SELECTION_LAST = 1
	};

	enum ReticleColorEnum
	{
		RETICLE_COLOR_NONE = -1,
		RETICLE_COLOR_WHITE,
		RETICLE_COLOR_BLACK,
		RETICLE_COLOR_GREEN
	};

	enum Misc
	{
		CAMERA_NOT_RELEVANT_VALUE = 0xFF
	};

	enum SlaSensitivityTypeEnum
	{
		SLA_SENSITIVITY_TYPE_NONE = -1,
		LARGE_TARGET_MTD_THRESHOLD = 1,
		SMALL_TARGET_MTI_SENSITIVITY = 2
	};

	enum TrackerOperationEnum
	{
		TRACKER_IS_IDLE,
		TRACKING,
		COASTING
	};

	enum LrfCamCmdTypeEnum
	{
		LRF_CAM_CMD_RETICLE_GREY_LEVEL	= 0,
		LRF_CAM_CMD_ZOOM				= 1,
		LRF_CAM_CMD_OVERLAY_SHOW		= 2,
		LRF_CAM_CMD_COLOR_GAIN			= 3
	};
	
#pragma pack(1)

	struct RangeStruct
	{
		CommonDbDefs::RangeTypeEnum eRangeType; // t:int
		float fRange;
	};

	struct StringStruct
	{
		char StringValue[256];
	};
	
	struct CoordinatesStruct
	{
		float	fCordX;
		float	fCordY;
	};

	struct DimensionStruct
	{
		float	fWidthX;
		float	fHeightY;
	};

	struct CompensationStruct
	{
		float fTrvCompensation; // u:Angle: Radian
		float fElvCompensation; // u:Angle: Radian
	};

	struct PodPositionStruct
	{
		float fElevation;			// u:Radian
		float fAzimuth;				// u:Radian
	};

	struct MinAndMaxStruct
	{
		float fMin;
		float fMax;
	};

	struct OrientationStruct
	{
		float fElvation;
		float fTraverse;
	};

	struct InsModeTransitionStruct
	{
		CommonDbDefs::InsModeTypeEnum	eModeType;								//t:int
		float							m_fLatitude;							//c: rad, -PI/2 , +PI/2
		float							m_fLongitude;							//c: rad, -PI, +PI 
		float							m_fAltitude;							//c: meter, -9999.00, +9999.99
	};

	struct InsCalibrationStruct
	{
		float					fPointingDeviceBorisightDataAlpha;	//c: rad, 0-2PI
		float					fPointingDeviceBorisightDataBeta;	//c: rad, -PI/2 , +PI/2
		float					fPointingDeviceBorisightDataGamma;	//c: rad, -PI, +PI
	};

	struct ScanningStruct
	{
		CommonDbDefs::UserRequestEnum	eAction;					//t:int
		float							fAzimuthStartPoint;			//c: mRad, 0 - 2PI  rad (0-360 degrees) 
		float							fElevationStartPoint;		//c: mRad, -PI/6 - PI/4 rad (-30 - 45 degrees )
		float							fAzimuthStopPoint;			//c: mRad, 0 - 2PI  rad (0-360 degrees)	
		float							fElevationStopPoint;		//c: mRad, -PI/6 - PI/4 rad (-30 - 45 degrees )
		CommonDbDefs::VelocityEnum		ePredefinedVelocity;		//t:int
		float							fUserVelocity;
	};

	struct PositionRequestStruct
	{
		CommonDbDefs::UserRequestEnum	eAction;		// t:int
		float							fElevation;		// c:0 to PI in units of  mrad, An offset of (-PI/2) should be added to the angle value in order to transform the value to the (-PI/2, +PI/2) range
		float							fAzimuth;		// c:0 to 2PI in units of mrad, An offset of (-PI) should be added to the angle value in order to transform the value to the (-PI, +PI) range.
	};

	struct MinMaxRangeStruct
	{
        uint16_t        				wMinRange;		// c:50 - Infinity in units of meters.
        uint16_t						wMaxRange;		// c:50 - Infinity in units of meters.
	};

	struct UnitsOnOffStruct
	{
		CommonDbDefs::SystemTypeEnum 	eSystemType;					//CommonDbDefs::SystemTypeEnum t:int
		CommonDbDefs::OnOffStateEnum 	eIlluminatorState;				//CommonDbDefs::OnOffStateEnum t:int		
		CommonDbDefs::OnOffStateEnum 	eDesignatorState;				//CommonDbDefs::OnOffStateEnum t:int	
		CommonDbDefs::OnOffStateEnum	ePodState;						//CommonDbDefs::OnOffStateEnum t:int		
		CommonDbDefs::OnOffStateEnum	eLrfLasingState;				//CommonDbDefs::OnOffStateEnum t:int			
		CommonDbDefs::OnOffStateEnum	eDayCameraState;				//CommonDbDefs::OnOffStateEnum t:int		
		CommonDbDefs::OnOffStateEnum	eTiCameraState;					//CommonDbDefs::OnOffStateEnum t:int	
		CommonDbDefs::OnOffStateEnum	eLwsState;						//CommonDbDefs::OnOffStateEnum t:int	
		CommonDbDefs::OnOffStateEnum	eLrfPowerState;					//CommonDbDefs::OnOffStateEnum t:int	
	};

	struct GeneralCameraCommandsWithoutSetValueStruct
	{
		CommonDbDefs::FunctionModeEnum eMode;					// CommonDbDefs::FunctionModeEnum t:int
		//CamerasDefs::FunctionStepCmdEnum eStep;					// CamerasDefs::FunctionStepCmdEnum t:int
	};

	struct GeneralCameraCommandStruct
	{
		CommonDbDefs::FunctionModeEnum eMode;					// CommonDbDefs::FunctionModeEnum t:int
		//CamerasDefs::FunctionStepCmdEnum eStep;					// CamerasDefs::FunctionStepCmdEnum t:int
		int nValue;												// c:0-100, 0xFF - Not relevant
	};

	struct FocusCommandStruct
	{
		CommonDbDefs::FunctionModeEnum eMode;					// CommonDbDefs::FunctionModeEnum t:int
		//CamerasDefs::FunctionStepCmdEnum eStep;					// CamerasDefs::FunctionStepCmdEnum t:int
		//CamerasDefs::FocusSpecialModeEnum eFocusSpecialMode;	// CamerasDefs::FocusSpecialModeEnum t:int
		int nValue;												// c:0-100, 0xFF - Not relevant
	};

	struct ButtonEventStruct
	{
		int							nButtonIndex;				// c:counting from 0
		CommonDbDefs::ButtonEnum	eButtonStatus;				// CommonDbDefs::ButtonEnum t:int
	};

	struct SlaSenstivityStruct
	{
		CommonDbDefs::SlaSensitivityTypeEnum eSensitivityType;	// CommonDbDefs::SlaSensitivityTypeEnum t:int
		int nValue;												// c: 0,1-10 for large targets, 0-4 for small targets	
	};

	struct LrfCamCmdStruct
	{
		CommonDbDefs::LrfCamCmdTypeEnum eLrfCamCmdType;			// CommonDbDefs::LrfCamCmdTypeEnum t:int
		int nValue;												
	};	

#pragma pack()

	//static float 		FLOAT_NONE 	= 999999.999999999; // TODO update min or max
	//static int 			INT_NONE 	= -11111111111;		// TODO update min or max
	//static unsigned int UINT_NONE 	= 0xFFFFFFFFFF;		// TODO update min or max
	
};

#endif //__COMMON_DEFS__H__

