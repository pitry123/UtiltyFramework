#pragma once
namespace Rules
{
using namespace rules;
	class RulesDB
	{
		public:
			// Enum for input data
			enum RulesInputDBEnum
			{

				iActSystemStatesStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iCurrentSystemState,			//StateMachine_SystemStates::SystemStates_StatesEnum			t:int						def:NO_STATE
				iRequestSystemState,			//StateMachine_SystemStates::SystemStates_StatesEnum			t:int						def:NO_STATE
				iEnterGeneral,			//CommonDbDefs::EnabledEnum			t:int						def:ENABLED_NONE
				iShaftEncoderTrvStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_NONE
				iShaftEncoderElvStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_NONE
				iDriverElvStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_OK
				iDriverTrvStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_OK
				iGyroTrvStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_NONE
				iGyroElvStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_NONE
				iVoltageStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_NONE
				iCommutationStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_NONE
				iMastOpened,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_NONE
				iWorkModeRequest,			//CommonDbDefs::ModeEnum			t:int						def:MODE_MANUAL
				iActWorkModeStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iCurrentWorkMode,			//CommonDbDefs::ModeEnum			t:int						def:MODE_MANUAL
				iMobileStatus,			//CommonDbDefs::MobileEnum			t:int						def:MOBILE_FALSE
				iActTrackingStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iActScanningStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iActBringToStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iActBoreSightStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iActNucStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iDesignatorStatus,			//CommonDbDefs::OnOffStateEnum			t:int						def:STATE_OFF
				iIlluminatorStatus,			//CommonDbDefs::OnOffStateEnum			t:int						def:STATE_OFF
				iActDriftStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iDriftCompleted,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_FALSE
				iHandlePalmStatus,			//CommonDbDefs::ButtonEnum			t:int						def:BUTTON_RELEASED
				iHandlePalmInitiator,			//CommonDbDefs::UserEnum			t:int						def:USER_NONE
				iHandleAnalogsInitiator,			//CommonDbDefs::UserEnum			t:int						def:USER_NONE
				iHandleFovInitiator,			//CommonDbDefs::UserEnum			t:int						def:USER_NONE
				iHandleFovStatus,			//CommonDbDefs::ButtonEnum			t:int						def:BUTTON_RELEASED
				iHandleTrackInitiator,			//CommonDbDefs::UserEnum			t:int						def:USER_NONE
				iHandleTrackStatus,			//CommonDbDefs::ButtonEnum			t:int						def:BUTTON_RELEASED
				iActControlStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iHandleAnalogsElvAboveThreshold,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_FALSE
				iHandleAnalogsTrvAboveThreshold,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_FALSE
				iHandleLaseInitiator,			//CommonDbDefs::UserEnum			t:int						def:USER_NONE
				iHandleLaseStatus,			//CommonDbDefs::ButtonEnum			t:int						def:BUTTON_RELEASED
				iProcessBringToInitiator,			//CommonDbDefs::UserEnum			t:int						def:USER_NONE
				iProcessBringToStatus,			//CommonDbDefs::UserRequestEnum			t:int						def:USER_REQUEST_NONE
				iControlBringToProcessStatus,			//CommonDbDefs::ProcessEnum			t:int						def:PROCESS_NONE
				iBringToState,			//StateMachine_BringTo::BringTo_StatesEnum			t:int						def:NO_STATE
				iScanningState,			//StateMachine_Scanning::Scanning_StatesEnum			t:int						def:NO_STATE
				iControlScanningProcessStatus,			//CommonDbDefs::ProcessEnum			t:int						def:PROCESS_NONE
				iProcessScanningInitiator,			//CommonDbDefs::UserEnum			t:int						def:USER_NONE
				iProcessScanningStatus,			//CommonDbDefs::UserRequestEnum			t:int						def:USER_REQUEST_NONE
				iActLaseStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iActCcdStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iActTiStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iActVmdStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iActReticleStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iActVideoSwitchingStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iAutoTrackingStatus,			//CommonDbDefs::TrackerOperationEnum			t:int						def:TRACKER_IS_IDLE
				iActSlaStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iDayCamCommStatus,			//CommonDbDefs::CommunicationEnum			t:int						def:COMMUNICATION_NONE
				iNightCamCommStatus,			//CommonDbDefs::CommunicationEnum			t:int						def:COMMUNICATION_NONE
				iLrfCamCommStatus,			//CommonDbDefs::CommunicationEnum			t:int						def:COMMUNICATION_NONE
				iActSeCalibrationStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iDiscreteLiReadySwitch,			//CommonDbDefs::OnOffStateEnum			t:int						def:STATE_NONE
				iDiscreteLiUnitStsSwitch,			//CommonDbDefs::OnOffStateEnum			t:int						def:STATE_NONE
				iLiSelectStatus,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_NONE
				iLdLiLasingPasswordSts,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_FALSE
				iLiOnOffRequest,			//CommonDbDefs::OnOffStateEnum			t:int						def:STATE_OFF
				iLdOnOffRequest,			//CommonDbDefs::OnOffStateEnum			t:int						def:STATE_OFF
				iLrfOnOffRequest,			//CommonDbDefs::OnOffStateEnum			t:int						def:STATE_OFF
				iLiCommStatus,			//CommonDbDefs::CommunicationEnum			t:int						def:COMMUNICATION_NONE
				iLdCommStatus,			//CommonDbDefs::CommunicationEnum			t:int						def:COMMUNICATION_NONE
				iCommanderSelectedSys,			//ProjectCommonDbDefs::SelectedSystemEnum			t:int						def:SYSTEM_OBSERVATION
				iGunnerSelectedSys,			//ProjectCommonDbDefs::SelectedSystemEnum			t:int						def:SYSTEM_ORCWS
				iHatchesCloseSts,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_FALSE
				iLiLdLasingSwitch,			//CommonDbDefs::OnOffStateEnum			t:int						def:STATE_OFF
				iLdNormalTemperature,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_FALSE
				iGunShaftEncoderElvStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_NONE
				iGunShaftEncoderTrvStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_NONE
				iNightCamReady,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_FALSE
				iActFusionStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iBoreSightType,			//ProjectCommonDbDefs::BoreSightTypeEnum			t:int						def:BORE_SIGHT_TYPE_NONE
				iTrackingUser,			//CommonDbDefs::UserEnum			t:int						def:USER_NONE
				iSlaCommStatus,			//CommonDbDefs::CommunicationEnum			t:int						def:COMMUNICATION_NONE
				iTrackingCamChange,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_FALSE
				iStabErrorGroupStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_OK
				iPowerErrorGroupStatus,			//CommonDbDefs::StatusEnum			t:int						def:STATUS_OK
				iTrackingCam,			//ProjectCommonDbDefs::CamerasEnum			t:int						def:NUM_OF_CAMERAS
				iActFovCalibStatus,			//CommonDbDefs::ActivityStatusEnum			t:int						def:ACTIVITY_SUSPENDED
				iLiBsOnOffRequest,			//CommonDbDefs::OnOffStateEnum			t:int						def:STATE_OFF
				iMastClosed,			//CommonDbDefs::BoolEnum			t:int						def:BOOL_NONE

				RULES_INPUT_DB_SIZE
			};

			//Enum for output data
			enum RulesOutputDBEnum
			{
				oActSystemStatesCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oGoToSystemState,			//StateMachine_SystemStates::SystemStates_StatesEnum			t:int			def:NO_STATE
				oChangeWorkMode,			//CommonDbDefs::ModeEnum			t:int			def:MODE_MANUAL
				oActWorkModeCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oPalmConditionForDrift,			//CommonDbDefs::ConditionEnum			t:int			def:CONDITION_YES
				oActDriftCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oActiveOperator,			//CommonDbDefs::UserEnum			t:int			def:USER_NONE
				oCurrentMaster,			//CommonDbDefs::UserEnum			t:int			def:USER_NONE
				oHandlePalmPermission,			//UiDB::PermissionByUserEnum			t:int			def:PERMISSION_NONE
				oHandleAnalogsPermission,			//UiDB::PermissionByUserEnum			t:int			def:PERMISSION_NONE
				oHandleFovPermission,			//UiDB::PermissionByUserEnum			t:int			def:PERMISSION_NONE
				oHandleTrackPermission,			//UiDB::PermissionByUserEnum			t:int			def:PERMISSION_NONE
				oActControlCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oHandleLasePermission,			//UiDB::PermissionByUserEnum			t:int			def:PERMISSION_NONE
				oActBringToCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oGoToBringToState,			//StateMachine_BringTo::BringTo_StatesEnum			t:int			def:NO_STATE
				oProcessBringToPermission,			//UiDB::PermissionByUserEnum			t:int			def:PERMISSION_NONE
				oActScanningCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oGoToScanningState,			//StateMachine_Scanning::Scanning_StatesEnum			t:int			def:NO_STATE
				oActLaseCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oActCcdCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oActTiCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oActVmdCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oActReticleCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oActVideoSwitchingCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oActAutoTrackingCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oAutoTrackingRequest,			//CommonDbDefs::OnOffStateEnum			t:int			def:STATE_NONE
				oActSlaCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oBringToPermission,			//UiDB::PermissionByUserEnum			t:int			def:PERMISSION_NONE
				oScanningPermission,			//UiDB::PermissionByUserEnum			t:int			def:PERMISSION_NONE
				oActSeCalibrationCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oActBoreSightCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oLiLasingRequest,			//CommonDbDefs::OnOffStateEnum			t:int			def:STATE_OFF
				oLdLasingRequest,			//CommonDbDefs::OnOffStateEnum			t:int			def:STATE_OFF
				oLiPowerEnableCmd,			//CommonDbDefs::BoolEnum			t:int			def:BOOL_FALSE
				oLdPowerEnableCmd,			//CommonDbDefs::BoolEnum			t:int			def:BOOL_FALSE
				oLiProcessEnableCmd,			//CommonDbDefs::EnabledEnum			t:int			def:ENABLED_FALSE
				oLdProcessEnableCmd,			//CommonDbDefs::EnabledEnum			t:int			def:ENABLED_FALSE
				oConditionsForBringObsrvToGun,			//CommonDbDefs::ConditionEnum			t:int			def:CONDITION_NO
				oActFusionCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND
				oVmdPermission,			//UiDB::PermissionByUserEnum			t:int			def:PERMISSION_NONE
				oActFovCalibCmd,			//CommonDbDefs::ActivityCmdEnum			t:int			def:ACTIVITY_SUSPEND

				RULES_OUTPUT_DB_SIZE
			};

			//Enum for Rules Existence data
			enum RulesExistenceDBEnum
			{
				rConditionForLdDayBoreSight,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForLiDayBoreSight,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForThermalBoreSight,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForDrift,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rEndDriftFail,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rEndDriftSuccess,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rStartDrift,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rPalm_pressed,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForDayFovCalib,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForThermalFovCalib,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rEnable_General_Activities,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rActiveOperatorCommander,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rActiveOperatorGunner,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rActiveOperatorNone,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rBoresightToMaintenance,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rBoresightToOperational,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rCalibrationToMaintenance,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rCalibrationToOperational,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rIbitToMaintenance,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rIBitToOperational,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rInitToOperational,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rMaintenanceToBoresight,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rMaintenanceToCalibration,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rMaintenanceToIBit,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rMaintenanceToOperational,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rOperationalToMaintenance,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rChangeToManualMode,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rChangeToPowerMode,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForPowerMode,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rChangeToStabMode,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForStabMode,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForNuc,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForTracking,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rStartAutoTracking,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rStopAutoTrackingByFail,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rStopAutoTrackingByUser,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rBringToInitiateEnded,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForBringObsrvToGun,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForBringTo,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rEndBringToFail,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rEndBringToSuccess,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rStartBringTo,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rPerformLasing,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rWakeLasingActivity,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForLdEnable,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForLdPower,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rStartLdProcess,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rStopLdProcess,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForLiEnable,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForLiPower,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rStartLiProcess,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rStopLiProcess,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForScanning,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rEndScanningFail,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rEndScanningSuccess,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rScanningInitiateEnded,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rStartScanning,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rDisableHandles,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rEnableLasing,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rPermissionMasterCommander,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rPermissionMasterGunner,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rPermissionMasterNone,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rEnableVMD,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rConditionForOpticButton,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rMasterCommander,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rMasterGunner,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID
				rMasterNone,			//RulesDefs::RulesExistence			t:int			def:RULE_NOT_VALID

				RULES_EXISTENCE_DB_SIZE
			};

			//Enum for Rules Enabled data
			enum RulesEnabledDBEnum
			{
				enConditionForLdDayBoreSight,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForLiDayBoreSight,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForThermalBoreSight,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForDrift,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enEndDriftFail,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enEndDriftSuccess,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enStartDrift,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enPalm_pressed,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForDayFovCalib,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForThermalFovCalib,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enEnable_General_Activities,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enActiveOperatorCommander,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enActiveOperatorGunner,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enActiveOperatorNone,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enBoresightToMaintenance,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enBoresightToOperational,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enCalibrationToMaintenance,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enCalibrationToOperational,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enIbitToMaintenance,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enIBitToOperational,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enInitToOperational,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enMaintenanceToBoresight,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enMaintenanceToCalibration,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enMaintenanceToIBit,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enMaintenanceToOperational,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enOperationalToMaintenance,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enChangeToManualMode,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enChangeToPowerMode,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForPowerMode,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enChangeToStabMode,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForStabMode,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForNuc,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForTracking,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enStartAutoTracking,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enStopAutoTrackingByFail,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enStopAutoTrackingByUser,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enBringToInitiateEnded,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForBringObsrvToGun,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForBringTo,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enEndBringToFail,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enEndBringToSuccess,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enStartBringTo,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enPerformLasing,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enWakeLasingActivity,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForLdEnable,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForLdPower,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enStartLdProcess,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enStopLdProcess,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForLiEnable,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForLiPower,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enStartLiProcess,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enStopLiProcess,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForScanning,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enEndScanningFail,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enEndScanningSuccess,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enScanningInitiateEnded,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enStartScanning,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enDisableHandles,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enEnableLasing,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enPermissionMasterCommander,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enPermissionMasterGunner,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enPermissionMasterNone,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enEnableVMD,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enConditionForOpticButton,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enMasterCommander,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enMasterGunner,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE
				enMasterNone,			//RulesDefs::RulesEnabled			t:int			def:RULE_ENABLE

				RULES_ENABLED_DB_SIZE
			};

			enum RulesManagementDBEnum
			{
				RELOAD_RULES,                //t:bool
				RULES_MANAGEMENT_DB_SIZE
			};
	};
}