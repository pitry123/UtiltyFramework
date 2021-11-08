#ifndef __PROJECT_COMMON_DEFS__H__
#define __PROJECT_COMMON_DEFS__H__

#include "CommonDbDefs.h"

class ProjectCommonDbDefs
{

public:
	

	//--------------------- Enums ---------------------------------------//
	
	enum VersionsTypeEnum
	{
		EMU_VERSION,
		
		NUM_OF_VERSION_TYPES
	};
	
	//--------------------- From DB's ---------------------------------------//

	//oleg
	enum LdrcStationsEnum
	{
		LDRC_NONE = 0,
		LDRC_DEFAULT = 1,
		LDRC_EXTRA_SHARPNESS_FOR_DEFAULT = 2,
		LDRC_BAD_WEATHER = 3,
		LDRC_BLOOMING = 4,
		LDRC_EXTRA_SHARPNESS_FOR_BLOOMING = 5
	};
	////oleg

	enum NucIntegrationModeEnum
	{
		LONG_INTEGRATION = 0,
		MEDIUM_INTEGRATION = 1,
		SHORT_INTEGRATION = 2
	};

	enum CamerasEnum
	{
		CAMERA_DAY,
		CAMERA_NIGHT,

		NUM_OF_CAMERAS
	};

	enum EnslavementModeEnum
	{
		ENSLAVEMENT_MODE_ERROR			= -2,
		ENSLAVEMENT_MODE_NONE 			= -1,
		ENSLAVEMENT_MODE_MECHANICAL		= 0,
		ENSLAVEMENT_MODE_GTS			= 1,
		ENSLAVEMENT_MODE_STG			= 2
	};

	enum PointerTypeEnum
	{
		POINTER_TYPE_CONTINUOUS,
		POINTER_TYPE_BLINKING,
	};

	enum CameraSelectionEnum
	{
		INPUT_SELECTION_NONE = -1,
		INPUT_SELECTION_VIDEO,
	};

	enum RangeAccuracyEnum
	{
		RANGE_ACCURACY_NONE,
		RANGE_ACCURACY_ESTIMATED,
		RANGE_ACCURACY_ACCURATE
	};

	enum WorkModesEnum
	{
		WORK_MODE_NONE,
		WORK_MODE_MANUAL,
		WORK_MODE_POWER,
		WORK_MODE_STAB
	};

	enum FireModeEnum
	{
		FIRE_MODE_UNINITIALIZED		= -1,
		FIRE_MODE_SINGLE			,
		FIRE_MODE_BURST				,
		FIRE_MODE_AUTOMATIC			,
		FIRE_MODE_ENUM_SIZE
	};

	enum ProcessStatusUpdatesEnum
	{
		PROCESS_STATUS_UPDATE_NOT_IN_PROCESS,
		PROCESS_STATUS_UPDATE_IN_PROCESS,
		PROCESS_STATUS_UPDATE_END_OK,
		PROCESS_STATUS_UPDATE_END_FAIL,
		PROCESS_STATUS_UPDATE_ABORTED_ON_REQUEST,
		PROCESS_STATUS_UPDATE_ABORTED_NO_CONDITIONS

	};

	enum RecordingStateEnum
	{
		RECORDING_STATE_NONE = -1,
		RECORDING_STATE_ON_ACTIVATED = 0,
		RECORDING_STATE_OFF_NOT_ACTIVATED,
		RECORDING_STATE_RECORDING_FAILED,
		RECORDING_STATE_RECORDING_ENABLED,
		RECORDING_STATE_RECORDING_DISABLED
	};

	enum RecordingKindEnum
	{
		RECORDING_KIND_NONE = -1,
		RECORDING_KIND_CONTINUOUS = 0,
		RECORDING_KIND_DISCONTINUOUS
	};
	enum PlayingStateEnum
	{
		PLAYING_STATE_NONE = -1,
		PLAYING_STATE_ON_ACTIVATED = 0,
		PLAYING_STATE_OFF_NOT_ACTIVATED,
		PLAYING_STATE_PLAYING_FAILED,
		PLAYING_STATE_PLAYING_ENABLED,
		PLAYING_STATE_PLAYING_DISABLED
	};

	enum PlayingRateEnum
	{
		PLAYING_RATE_NONE = -1,
		PLAYING_RATE_REGULAR = 0,
		PLAYING_RATE_FAST,
		PLAYING_RATE_STOP
	};

	enum PlayingDirectionEnum
	{
		PLAYING_DIRECTION_NONE = -1,
		PLAYING_DIRECTION_FORWARD = 0,
		PLAYING_DIRECTION_BACKWARD
	};

	enum SelectedSystemEnum
	{
		SYSTEM_NONE = -1,
		SYSTEM_ORCWS = 0,
		SYSTEM_OBSERVATION
	};

	enum BoreSightTypeEnum
	{
		BORE_SIGHT_TYPE_NONE,

		BORE_SIGHT_TYPE_LI_DAY,
		BORE_SIGHT_TYPE_LD_DAY,
		BORE_SIGHT_TYPE_LI_THERMAL,
		BORE_SIGHT_TYPE_LD_THERMAL,
		BORE_SIGHT_TYPE_DAY_FOV_CALIB,
		BORE_SIGHT_TYPE_THERMAL_FOV_CALIB,
	};

	enum BoreSightMovementEnum
	{
		POD_CALIB_NO_MOVEMENT = -1,
		POD_CALIB_MOVEMENT_STEP_UP,
		POD_CALIB_MOVEMENT_STEP_DOWN,
		POD_CALIB_MOVEMENT_STEP_LEFT,
		POD_CALIB_MOVEMENT_STEP_RIGHT,
		POD_CALIB_MOVEMENT_CONT_UP,
		POD_CALIB_MOVEMENT_CONT_DOWN,
		POD_CALIB_MOVEMENT_CONT_LEFT,
		POD_CALIB_MOVEMENT_CONT_RIGHT,
	};

	enum IlluminatorOperationModeEnum
	{
		LI_OPERATION_MODE_NONE = -1,
		LI_OPERATION_MODE_PULESE,
		LI_OPERATION_MODE_CONTINUOUS
	};

	enum MastStatusEnum
	{
		MAST_STATUS_NONE,
		MAST_STATUS_ERROR,
		MAST_STATUS_OPEN_READY,
		MAST_STATUS_MOVING_OPEN,
		MAST_STATUS_MOVING_CLOSE,
		MAST_STATUS_CLOSE_READY
	};

	enum HomingStatusEnum
	{
		HOMING_NONE,
		IN_HOMING_IN_PROCESS,
		IN_HOMING_END_FAIL,
		IN_HOMING_END_SUCCESS,
		IN_HOMING_END_SUCCESS_FORCED,
		OUT_OF_HOMING,
		OUT_OF_HOMING_POWER_OFF,
		OUT_OF_HOMING_FORCED,
	};

	enum HomingCmdEnum
	{
		HOMING_NONE_CMD,
		IN_HOMING_CMD,
		OUT_OF_HOMING_CMD,
	};

	enum ScreenSize
	{
		ScreenWidth = 701, //710,
		ScreenHeight = 576 //580
	};

	//--------------------- From Rules ---------------------------------------//
	
	enum SystemStatesEnum
	{
		STATE_NONE = -1,
		STATE_INITIALIZATION,
		STATE_OPERATIONAL,
		STATE_BORESIGHT,
		STATE_MAINTENANCE,
		STATE_TECHNICIAN,

		NUM_OF_SYSTEM_STATES
	};
	
	enum InvMovementStatesEnum
	{
		INV_MOVE_STATE_NONE = -1,
		STATE_CHECK_ENABLED,
		STATE_CHECK_DISABLED,
		STATE_FAILURE,
		
		NUM_OF_INV_MOVEMENT_STATES
	};
	
	enum OperatorTypesEnum
	{
		OPERATOR_NONE = -1,
		OPERATOR_GUNNER,
		OPERATOR_COMMANDER,
	
		NUM_OF_OPERATOR_TYPES
	};

	enum SystemArmEnum
	{
		SYSTEM_UNARMED,
		SYSTEM_ARMING_ERROR,
		SYSTEM_ARMED,
		SYSTEM_ARMED_NOT_READY
	};

	enum FourWayStepEnum
	{
		FOUR_WAY_STEP_NO_STEP = -1,
		FOUR_WAY_STEP_UP,
		FOUR_WAY_STEP_DOWN,
		FOUR_WAY_STEP_LEFT,
		FOUR_WAY_STEP_RIGHT
	};

	enum IntensityEnum
	{
		INTENSITY_NO_STEP = -1,
		INTENSITY_INCREASE,
		INTENSITY_DECREASE
	};

	enum StockValueEnum
	{
		STOCK_UNINITIALIZED		= -1,
		FULL_STOCK				,
		HALF_STOCK				,
		EMPTY_STOCK				,
		MANUAL_STOCK			,
		STOCK_VALUE_ENUM_SIZE
	};

	enum StationTypeEnum
	{
		DRAGON_BASIC			= 0,
		DRAGON_EXTENDED			= 1
	};

	enum SelectedWeaponEnum
	{
		WEAPON_NONE 			= -1,
		WEAPON_GUN05,
		WEAPON_MAG,
		WEAPON_MAKLAR
	};

	enum LrfFlipEnum
	{
		LRF_FLIP_NONE = -1,
		LRF_NO_FLIP = 0,
		LRF_FLIP = 1
	};

	enum LrfMirrorEnum
	{
		LRF_MIRROR_NONE = -1,
		LRF_NO_MIRROR = 0,
		LRF_MIRROR = 1
	};

	enum LrfOverlayEnum
	{
		LRF_OVERLAY_NONE = -1,
		LRF_OVERLAY_OFF = 0,
		LRF_OVERLAY_ON = 1
	};

	//--------------------- Structs ---------------------------------------//

#pragma pack(1)

	struct SystemRangeStruct
	{
		CommonDbDefs::RangeTypeEnum				eRangeType;				// t:int		
		CommonDbDefs::RangeMeasureStatusEnum	eMeasureType;			// t:int
		CommonDbDefs::RangeSelectionEnum		eFirstLastSelection;	// t:int
		float									fSelectedRange;			
		float									fFirstRange;			
		float									fLastRange;				
	};

#pragma pack()

};

#endif //__PROJECT_COMMON_DEFS__H__

