#ifndef _UI_DB_H_
#define _UI_DB_H_


class UiDB
{

public:

	
	enum PermissionByUserEnum
	{
		PERMISSION_NONE			= -3,
		PERMISSION_ALL			= -2,
		PERMISSION_AUTHORITY	= -1,
		PERMISSION_ACTIVE_USER	= 0,
		PERMISSION_COMMANDER	= 1,
		PERMISSION_GUNNER		= 2,

		PERMISSION_DEFAULT
	};

	enum DbEnum
	{
		DB_NONE = -1,
		
		DB_EXECUTION,

		DB_COMMANDER,
		DB_GUNNER,

		NUM_OF_DBS
	};

	

	enum UserInputsDBEnum
	{
		// HANDLES
		HANDLES_ANALOGS_CMD,					// t:UiDB::UserHandleCommandStruct
		HANDLES_PALM_CMD,						// t:UiDB::UserOnOffStruct		
		HANDLES_LASE_CMD,						// t:UiDB::UserOnOffStruct		
		HANDLES_FOV_CMD,						// t:UiDB::UserOnOffStruct	
		HANDLES_TRACK_CMD,						// t:UiDB::UserOnOffStruct		
			
		// PROCESS
		PROCESS_BRING_TO_CMD,					// t:UiDB::BringToCommandStruct
		PROCESS_AUTOMATIC_TRACKING_CMD,			// t:UiDB::UserOnOffStruct
		PROCESS_SCANNING_CMD,					// t:UiDB::ScanningCommandStruct
		PROCESS_DRIFT_CANCELLATION_CMD,			// t:UiDB::UserOnOffStruct
		PROCESS_VMD_CMD,						// t:UiDB::UserOnOffStruct
		PROCESS_LI_TIMER_SEC_CMD,				// t:int
		PROCESS_MAINTENANCE_CMD,				// t:UiDB::UserOnOffStruct

		// DESIGNATOR
		DESIGNATOR_SELECT_CMD,					// t:UiDB::UserOnOffStruct
		DESIGNATOR_ARM_CMD,						// t:UiDB::UserOnOffStruct
		DESIGNATOR_LASE_CMD,					// t:UiDB::UserOnOffStruct
		DESIGNATOR_CODE_CMD,					// t:UiDB::IntStruct

		// ILLUMINATOR
		ILLUMINATOR_SELECT_CMD,					// t:UiDB::UserOnOffStruct
		ILLUMINATOR_ARM_CMD,					// t:UiDB::UserOnOffStruct
		ILLUMINATOR_LASE_CMD,					// t:UiDB::UserOnOffStruct
		ILLUMINATOR_OPERATION_MODE_CMD,			// t:UiDB::LiOperationModeStruct
		ILLUMINATOR_MAX_LASING_TIME_CMD,		// t:UiDB::LiMaxLaseTimeStruct

		LD_LI_LASING_PASSWORD_RECEIVED_CMD,		// t:UiDB::BoolStruct

		// LRF
		LRF_MIN_MAX_RANGE_CMD,					// t:UiDB::MinMaxRangeStruct
		LRF_RETICLE_GRAY_LEVEL_CMD,				// t:UiDB::IntStruct c:0-255, 0=black, 255=white, 128=default
		LRF_RETICLE_ON_CMD,						// t:UiDB::UserOnOffStruct
		LRF_ZOOM_CMD,							// t:UiDB::IntStruct c:1-255, 1 = zoom X1, 255 = zoom X12
		LRF_POWER_CMD,							// t:UiDB::UserOnOffStruct
		LRF_ROTATE_FLIP_CMD,					// t:UiDB::LrfFlipStruct
		LRF_ROTATE_MIRROR_CMD,					// t:UiDB::LrfMirrorStruct
		LRF_OVERLAY_SHOW_CMD,					// t:UiDB::LrfOverlayStruct
		LRF_COLOR_GAIN_CMD,						// t:UiDB::IntStruct c:0-255, 255 = max color
		LRF_FIRST_LAST_RANGE_CMD,				// t:UiDB::RangeSelectionStruct

		// MANUAL RANGE
		MANUAL_RANGE_CMD,						// t:UiDB::NumericStruct

		// --- CAMERAS ---
		CAMERA_SELECT_CMD,						//t:UiDB::CameraSelectionCommandStruct

		// CCD
		CCD_FOCUS_CMD,							// t:UiDB::FocusCommandStruct
		CCD_FOV_CMD,							// t:UiDB::FovCommandStruct
		CCD_ZOOM_CMD,							// t:UiDB::GeneralCameraCommandStruct
		CCD_IRIS_CMD,							// t:UiDB::GeneralCameraCommandStruct
		CCD_BRIGHTNESS_CMD,						// t:UiDB::GeneralCameraCommandStruct
		CCD_CAMERA_COLOR_CMD,					// t:UiDB::CameraColorSelectionStruct
		CCD_IMAGE_ENHANCEMENT_CMD,				// t:UiDB::UserOnOffStruct
		CCD_DRC_CMD,							// t:UiDB::DrcCommandStruct
		CCD_FREEZE_CMD,							// t:UiDB::FreezeCommandStruct
		CCD_ALPHA_NUMERIC_DISPLAY_CMD,			// t:UiDB::UserOnOffStruct
		CCD_GAMMA_CMD,							// t:UiDB::GeneralCameraCommandStruct
		CCD_GAIN_CMD,							// t:UiDB::GeneralCameraCommandStruct
		CCD_LOW_LIGHT_CMD,						// CommonDbDefs::OnOffStateEnum t:int

		// TI
		TI_FOCUS_CMD,							// t:UiDB::FocusCommandStruct
		TI_FOV_CMD,								// t:UiDB::FovCommandStruct
		TI_ZOOM_CMD,							// t:UiDB::GeneralCameraCommandStruct
		TI_EZOOM_CMD,							// t:UiDB::GeneralCameraCommandStruct
		TI_POLARITY_CMD,						// t:UiDB::PolaritySelectionStruct
		TI_SYMBOLS_DISPLAY_CMD,					// t:UiDB::DisplayFunctionsStruct
		TI_SYNTHETIC_IMAGE_CMD,					// t:UiDB::DisplayFunctionsStruct
		TI_GAMMA_CMD,							// t:UiDB::GeneralCameraCommandStruct
		TI_LEVEL_CMD,							// t:UiDB::GeneralCameraCommandStruct
		TI_GAIN_CMD,							// t:UiDB::GeneralCameraCommandStruct
		TI_DRC_CMD,								// t:UiDB::DrcCommandStruct
		TI_NUC_CMD,								// t:UiDB::NucCommandStruct
		TI_FREEZE_CMD,							// t:UiDB::FreezeCommandStruct	
		TI_LDRC_CMD,							// ProjectCommonDbDefs::LdrcStationsEnum t:int
		TI_INTEGRATION_TIME_STATE_CMD,          // CommonDbDefs::FunctionModeEnum t:int
		TI_INTEGRATION_TIME_TYPE_CMD,           // ProjectCommonDbDefs::NucIntegrationModeEnum t:int

		// GUNNER
		GUNNER_CONFIRM_ACTIVATING_OBSRV_HW_CMD, // t:UiDB::EmptyEntryStruct

		BORESIGHT_INC_DEC_REQ_CMD,					// t:UiDB::BoreSightStepStruct
		BORESIGHT_ENTER_PRESSED_CMD,				// t:UiDB::EmptyEntryStruct
		BORESIGHT_CANCEL_PRESSED_CMD,				// t:UiDB::EmptyEntryStruct
		BORESIGHT_RESET_REQ_CMD,					// t:UiDB::EmptyEntryStruct
		BORESIGHT_LI_DAY_PROCESS_REQ_CMD,			// t:UiDB::UserRequestStruct
		BORESIGHT_LD_DAY_PROCESS_REQ_CMD,			// t:UiDB::UserRequestStruct
		BORESIGHT_LI_THERMAL_PROCESS_REQ_CMD,		// t:UiDB::UserRequestStruct
		BORESIGHT_LD_THERMAL_PROCESS_REQ_CMD,		// t:UiDB::UserRequestStruct
		FOV_CALIB_DAY_PROCESS_REQ_CMD,				// t:UiDB::UserRequestStruct
		FOV_CALIB_THERMAL_PROCESS_REQ_CMD,			// t:UiDB::UserRequestStruct
		// Oleg bs
		BORESIGHT_BACK_PRESSED_CMD,				    // t:none

		// WORK MODE
		WORK_MODE_REQ_CMD,							// t:UiDB::WorkModeStruct

		// INS
		INS_MODE_TRANSITION_REQUEST_CMD,			// t:UiDB::InsModeTransitionStruct
		
		// Homing
		TO_HOME_REQUEST_STS,						// t:UiDB::EmptyEntryStruct
		OUT_OF_HOME_REQUEST_STS,					// t:UiDB::EmptyEntryStruct

		// Reticle
		RETICLE_ON_OFF_CMD,							// t:UiDB::UserOnOffStruct
		RETICLE_COLOR_CMD,							// t:UiDB::ReticleColorStruct

		// Homing
		HOMING_CALIB_REQUEST_CMD,					// t:UiDB::UserRequestStruct

		// Fusion
		FUSION_PERCENT_DAY_VS_TI_REQUEST_CMD,		// t:UiDB::GeneralCameraCommandStruct
		FUSION_HOT_PARTS_COLOR_CMD,					// t:UiDB::FusionHotPartsStruct

		// SLA
		SLA_SHARPNESS_REQUEST_CMD,					// t:UiDB::GeneralCameraCommandStruct
		SLA_BRIGHTNESS_REQUEST_CMD,					// t:UiDB::GeneralCameraCommandStruct
		SLA_CONTRAST_REQUEST_CMD,					// t:UiDB::GeneralCameraCommandStruct
		SLA_SATURATION_REQUEST_CMD,					// t:UiDB::GeneralCameraCommandStruct
		SLA_RESET_REQUEST_CMD,						// t:UiDB::GeneralCameraCommandStruct
		SLA_BLEND_PARAMS_REQUEST_CMD,				// t:UiDB::GeneralCameraCommandStruct

		USER_INPUTS_DB_SIZE
	};


	enum PermissionDBEnum
	{
		// HANDLES
		PERMISSION_HANDLES_ANALOGS_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_HANDLES_PALM_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_HANDLES_LASE_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_HANDLES_FOV_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_HANDLES_TRACK_CMD,			// ControllerDefs::PermissionTypeEnum		t:int

		// PROCESS
		PERMISSION_PROCESS_BRING_TO_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_PROCESS_AUTOMATIC_TRACKING_CMD,	// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_PROCESS_SCANNING_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_PROCESS_DRIFT_CANCELLATION_CMD,	// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_PROCESS_VMD_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_PROCESS_LI_TIMER_SEC_CMD,		// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_PROCESS_MAINTENANCE_CMD,			// ControllerDefs::PermissionTypeEnum		t:int

		// DESIGNATOR
		PERMISSION_DESIGNATOR_SELECT_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_DESIGNATOR_ARM_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_DESIGNATOR_LASE_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_DESIGNATOR_CODE,					// ControllerDefs::PermissionTypeEnum		t:int

		// ILLUMINATOR
		PERMISSION_ILLUMINATOR_SELECT_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_ILLUMINATOR_ARM_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_ILLUMINATOR_LASE_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_ILLUMINATOR_OPERATION_MODE_CMD,  // ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_ILLUMINATOR_MAX_LASING_TIME_CMD, // ControllerDefs::PermissionTypeEnum		t:int

		PERMISSION_LD_LI_LASING_PASSWORD_RECEIVED_CMD,	// ControllerDefs::PermissionTypeEnum		t:int

		// LRF
		PERMISSION_LRF_MIN_MAX_RANGE_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_LRF_RETICLE_GRAY_LEVEL_CMD,		// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_LRF_RETICLE_ON_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_LRF_ZOOM_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_LRF_POWER_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_LRF_ROTATE_FLIP_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_LRF_ROTATE_MIRROR_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_LRF_OVERLAY_SHOW_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_LRF_COLOR_GAIN_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_LRF_FIRST_LAST_RANGE_CMD,		// ControllerDefs::PermissionTypeEnum		t:int
		
		// MANUAL RANGE
		PERMISSION_MANUAL_RANGE_CMD,				// ControllerDefs::PermissionTypeEnum		t:int

		// --- CAMERAS ---
		PERMISSION_CAMERA_SELECT_CMD,				// ControllerDefs::PermissionTypeEnum		t:int

		// CCD
		PERMISSION_CCD_FOCUS_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_FOV_CMD,						// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_ZOOM_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_IRIS_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_BRIGHTNESS_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_CAMERA_COLOR_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_IMAGE_ENHANCEMENT_CMD,		// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_DRC_CMD,						// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_FREEZE_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_ALPHA_NUMERIC_DISPLAY_CMD,	// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_GAMMA_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_GAIN_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_CCD_LOW_LIGHT_CMD,				// ControllerDefs::PermissionTypeEnum		t:int

		// TI
		PERMISSION_TI_FOCUS_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_FOV_CMD,						// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_ZOOM_CMD,						// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_POLARITY_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_SYMBOLS_DISPLAY_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_SYNTHETIC_IMAGE_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_GAMMA_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_LEVEL_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_GAIN_CMD,						// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_DRC_CMD,						// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_NUC_CMD,						// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_FREEZE_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		//oleg
		PERMISSION_TI_LDRC_CMD,                     // ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_EZOOM_CMD,                    // ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_INTEGRATION_TIME_MODE_CMD,    // ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_TI_INTEGRATION_TIME_TYPE_CMD,    // ControllerDefs::PermissionTypeEnum		t:int

		// GUNNER
		PERMISSION_GUNNER_CONFIRM_ACTIVATING_OBSRV_HW_CMD,	// ControllerDefs::PermissionTypeEnum		t:int

		PERMISSION_BORESIGHT_INC_DEC_REQ_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_BORESIGHT_ENTER_PRESSED_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_BORESIGHT_CANCEL_PRESSED_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_BORESIGHT_RESET_REQ_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_BORESIGHT_LI_DAY_PROCESS_REQ_CMD,		// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_BORESIGHT_LD_DAY_PROCESS_REQ_CMD,		// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_BORESIGHT_LI_THERMAL_PROCESS_REQ_CMD,	// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_BORESIGHT_LD_THERMALPROCESS_REQ_CMD,		// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_FOV_CALIB_DAY_PROCESS_REQ_CMD,			// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_FOV_CALIB_THERMAL_PROCESS_REQ_CMD,		// ControllerDefs::PermissionTypeEnum		t:int

		// WORK MODE
		PERMISSION_WORK_MODE_REQ_CMD,						// ControllerDefs::PermissionTypeEnum		t:int

		// INS
		PERMISSION_INS_MODE_TRANSITION_REQUEST_CMD,			// ControllerDefs::PermissionTypeEnum		t:int

		// Homing
		PERMISSION_TO_HOME_REQUEST_STS,						// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_OUT_OF_HOME_REQUEST_STS,					// ControllerDefs::PermissionTypeEnum		t:int

		// Reticle
		PERMISSION_RETICLE_ON_OFF_CMD,						// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_RETICLE_COLOR_CMD,						// ControllerDefs::PermissionTypeEnum		t:int

		// Homing
		PERMISSION_HOMING_CALIB_REQUEST_CMD,				// ControllerDefs::PermissionTypeEnum		t:int

		// Fusion
		PERMISSION_FUSION_PERCENT_DAY_VS_TI_REQUEST_CMD,	// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_FUSION_HOT_PARTS_COLOR_CMD,				// ControllerDefs::PermissionTypeEnum		t:int

		// SLA
		PERMISSION_SLA_SHARPNESS_REQUEST_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_SLA_BRIGHTNESS_REQUEST_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_SLA_CONTRAST_REQUEST_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_SLA_SATURATION_REQUEST_CMD,				// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_SLA_RESET_REQUEST_CMD,					// ControllerDefs::PermissionTypeEnum		t:int
		PERMISSION_SLA_BLEND_REQUEST_CMD,					// ControllerDefs::PermissionTypeEnum		t:int

		PERMISSION_DB_SIZE			
	};

	enum UnitsOnOffDBEnum
	{
		UNIT_ON_OFF_ILLUMINATOR_CMD,		// CommonDbDefs::OnOffStateEnum		t:int
		UNIT_ON_OFF_DESIGNATOR_CMD,			// CommonDbDefs::OnOffStateEnum		t:int
		UNIT_ON_OFF_POD_CMD,				// CommonDbDefs::OnOffStateEnum		t:int
		UNIT_ON_OFF_LRF_CMD,				// CommonDbDefs::OnOffStateEnum		t:int
		UNIT_ON_OFF_DAY_CAM_CMD,			// CommonDbDefs::OnOffStateEnum		t:int
		UNIT_ON_OFF_TI_CAM_CMD,				// CommonDbDefs::OnOffStateEnum		t:int
		UNIT_ON_OFF_LWS_CMD,				// CommonDbDefs::OnOffStateEnum		t:int

		UNITS_ON_OFF_DB_SIZE
	};


	enum ButtonsStsDBEnum
	{
		BUTTON_0_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_1_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_2_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_3_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_4_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_5_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_6_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_7_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_8_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_9_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_10_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_11_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_12_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_13_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_14_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_15_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_16_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_17_STS,				// UiDB.H::ButtonPressStatusEnum t:int
		BUTTON_18_STS,				// UiDB.H::ButtonPressStatusEnum t:int

		BUTTONS_STS_DB_SIZE
	};

	enum ButtonPressStatusEnum
	{
		BUTTON_NOT_PRESSED,
		BUTTON_PRESSED,
		BUTTON_CONT_PRESSED
	};

};

#endif	// _UI_DB_H_


