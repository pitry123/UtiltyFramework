#pragma once
#include "../RulesSample/BringTo_StateMachine.h"
#include "../RulesSample/CommonDbDefs.h"
#include "../RulesSample/ControllerDefs.h"
#include "../RulesSample/Operational_ActivitiesLogicDB.h"
#include "../RulesSample/ProjectCommon.h"
#include "../RulesSample/ProjectCommonDbDefs.h"
#include "../RulesSample/Scanning_StateMachine.h"
#include "../RulesSample/SystemStates_StateMachine.h"
#include "../RulesSample/UiDB.h"
#include <Database.hpp>
#include <Rules.hpp>
#include <map>
#include <functional>
#include <iostream>
#include "RulesDB.h"
namespace Rules
{
	class rules_data_and_types_impl : public utils::rules::rules_data_and_types_base<rules::rules_data_and_types>
	{
		private:
			// Memory Databases
			Database::DataSet m_dataset;
			Database::Table m_inputTable;
			Database::Table m_enableTable;
			Database::Table m_existenceTable;
			Database::Table m_outputTable;
			Database::Table m_managementDB;
			std::map<std::string, Database::Row> m_dbRowMap;
			// Rule_ID : DB_Index
			std::map<size_t, RulesDB::RulesExistenceDBEnum> m_ruleExistenceMap;
			std::map<size_t, RulesDB::RulesEnabledDBEnum> m_ruleEnabledMap;
		public:
			rules_data_and_types_impl(
				core::database::dataset_interface* dataset,
				core::database::table_interface* inputTable,
				core::database::table_interface* enableTable,
				core::database::table_interface* existenceTable,
				core::database::table_interface* outputTable,
				core::database::table_interface* managementTable) :
				m_dataset(dataset),
				m_inputTable(inputTable),
				m_enableTable(enableTable),
				m_existenceTable(existenceTable),
				m_outputTable(outputTable),
				m_managementDB(managementTable)
			{
				BuildDatabases();
				InitFunctionMap();
				InitEnumarationMap();
				InitDbRowMap();
				InitRuleEnabledMap();
				InitRuleExistenceMap();
			}
			~rules_data_and_types_impl() { }
			virtual bool query_row_by_string(const char* row_string, core::database::row_interface** row) override
			{
				if (row == nullptr)
					return false;
				m_dbRowMap[std::string(row_string)].UnderlyingObject(row);
				if (*row == nullptr)
				return false;
				return true;
			}
			virtual bool query_rule_existence_row(size_t rule_id, core::database::row_interface** row) override
			{
				if(row == nullptr)
					return false;
				m_existenceTable[m_ruleExistenceMap[rule_id]].UnderlyingObject(row);
				if (*row == nullptr)
				return false;
			return true;
			}
			virtual bool query_rule_enable_row(size_t rule_id, core::database::row_interface** row) override
			{
				if (row == nullptr)
					return false;
			m_enableTable[m_ruleEnabledMap[rule_id]].UnderlyingObject(row);
			if (*row == nullptr)
				return false;

				return true;
			}

			virtual bool  query_reload_rules_row(core::database::row_interface** row) override
			{
				if (row == nullptr)
					return false;
				auto Row = m_managementDB[RulesDB::RulesManagementDBEnum::RELOAD_RULES];
				if (Row.Empty())
					return false;
				Row.UnderlyingObject(row);
				return true;
			}

		private:
			void InitFunctionMap()
			{
				add_function_map("GetInputActSystemStatesStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActSystemStatesStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputCurrentSystemState",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iCurrentSystemState].Read<StateMachine_SystemStates::SystemStates_StatesEnum>());});
				add_function_map("GetInputRequestSystemState",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iRequestSystemState].Read<StateMachine_SystemStates::SystemStates_StatesEnum>());});
				add_function_map("GetInputEnterGeneral",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iEnterGeneral].Read<CommonDbDefs::EnabledEnum>());});
				add_function_map("GetInputShaftEncoderTrvStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iShaftEncoderTrvStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputShaftEncoderElvStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iShaftEncoderElvStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputDriverElvStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iDriverElvStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputDriverTrvStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iDriverTrvStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputGyroTrvStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iGyroTrvStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputGyroElvStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iGyroElvStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputVoltageStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iVoltageStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputCommutationStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iCommutationStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputMastOpened",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iMastOpened].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetInputWorkModeRequest",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iWorkModeRequest].Read<CommonDbDefs::ModeEnum>());});
				add_function_map("GetInputActWorkModeStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActWorkModeStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputCurrentWorkMode",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iCurrentWorkMode].Read<CommonDbDefs::ModeEnum>());});
				add_function_map("GetInputMobileStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iMobileStatus].Read<CommonDbDefs::MobileEnum>());});
				add_function_map("GetInputActTrackingStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActTrackingStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputActScanningStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActScanningStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputActBringToStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActBringToStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputActBoreSightStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActBoreSightStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputActNucStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActNucStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputDesignatorStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iDesignatorStatus].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetInputIlluminatorStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iIlluminatorStatus].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetInputActDriftStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActDriftStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputDriftCompleted",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iDriftCompleted].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetInputHandlePalmStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandlePalmStatus].Read<CommonDbDefs::ButtonEnum>());});
				add_function_map("GetInputHandlePalmInitiator",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandlePalmInitiator].Read<CommonDbDefs::UserEnum>());});
				add_function_map("GetInputHandleAnalogsInitiator",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandleAnalogsInitiator].Read<CommonDbDefs::UserEnum>());});
				add_function_map("GetInputHandleFovInitiator",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandleFovInitiator].Read<CommonDbDefs::UserEnum>());});
				add_function_map("GetInputHandleFovStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandleFovStatus].Read<CommonDbDefs::ButtonEnum>());});
				add_function_map("GetInputHandleTrackInitiator",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandleTrackInitiator].Read<CommonDbDefs::UserEnum>());});
				add_function_map("GetInputHandleTrackStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandleTrackStatus].Read<CommonDbDefs::ButtonEnum>());});
				add_function_map("GetInputActControlStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActControlStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputHandleAnalogsElvAboveThreshold",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandleAnalogsElvAboveThreshold].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetInputHandleAnalogsTrvAboveThreshold",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandleAnalogsTrvAboveThreshold].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetInputHandleLaseInitiator",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandleLaseInitiator].Read<CommonDbDefs::UserEnum>());});
				add_function_map("GetInputHandleLaseStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHandleLaseStatus].Read<CommonDbDefs::ButtonEnum>());});
				add_function_map("GetInputProcessBringToInitiator",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iProcessBringToInitiator].Read<CommonDbDefs::UserEnum>());});
				add_function_map("GetInputProcessBringToStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iProcessBringToStatus].Read<CommonDbDefs::UserRequestEnum>());});
				add_function_map("GetInputControlBringToProcessStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iControlBringToProcessStatus].Read<CommonDbDefs::ProcessEnum>());});
				add_function_map("GetInputBringToState",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iBringToState].Read<StateMachine_BringTo::BringTo_StatesEnum>());});
				add_function_map("GetInputScanningState",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iScanningState].Read<StateMachine_Scanning::Scanning_StatesEnum>());});
				add_function_map("GetInputControlScanningProcessStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iControlScanningProcessStatus].Read<CommonDbDefs::ProcessEnum>());});
				add_function_map("GetInputProcessScanningInitiator",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iProcessScanningInitiator].Read<CommonDbDefs::UserEnum>());});
				add_function_map("GetInputProcessScanningStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iProcessScanningStatus].Read<CommonDbDefs::UserRequestEnum>());});
				add_function_map("GetInputActLaseStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActLaseStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputActCcdStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActCcdStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputActTiStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActTiStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputActVmdStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActVmdStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputActReticleStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActReticleStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputActVideoSwitchingStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActVideoSwitchingStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputAutoTrackingStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iAutoTrackingStatus].Read<CommonDbDefs::TrackerOperationEnum>());});
				add_function_map("GetInputActSlaStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActSlaStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputDayCamCommStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iDayCamCommStatus].Read<CommonDbDefs::CommunicationEnum>());});
				add_function_map("GetInputNightCamCommStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iNightCamCommStatus].Read<CommonDbDefs::CommunicationEnum>());});
				add_function_map("GetInputLrfCamCommStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLrfCamCommStatus].Read<CommonDbDefs::CommunicationEnum>());});
				add_function_map("GetInputActSeCalibrationStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActSeCalibrationStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputDiscreteLiReadySwitch",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iDiscreteLiReadySwitch].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetInputDiscreteLiUnitStsSwitch",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iDiscreteLiUnitStsSwitch].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetInputLiSelectStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLiSelectStatus].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetInputLdLiLasingPasswordSts",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLdLiLasingPasswordSts].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetInputLiOnOffRequest",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLiOnOffRequest].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetInputLdOnOffRequest",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLdOnOffRequest].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetInputLrfOnOffRequest",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLrfOnOffRequest].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetInputLiCommStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLiCommStatus].Read<CommonDbDefs::CommunicationEnum>());});
				add_function_map("GetInputLdCommStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLdCommStatus].Read<CommonDbDefs::CommunicationEnum>());});
				add_function_map("GetInputCommanderSelectedSys",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iCommanderSelectedSys].Read<ProjectCommonDbDefs::SelectedSystemEnum>());});
				add_function_map("GetInputGunnerSelectedSys",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iGunnerSelectedSys].Read<ProjectCommonDbDefs::SelectedSystemEnum>());});
				add_function_map("GetInputHatchesCloseSts",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iHatchesCloseSts].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetInputLiLdLasingSwitch",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLiLdLasingSwitch].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetInputLdNormalTemperature",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLdNormalTemperature].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetInputGunShaftEncoderElvStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iGunShaftEncoderElvStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputGunShaftEncoderTrvStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iGunShaftEncoderTrvStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputNightCamReady",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iNightCamReady].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetInputActFusionStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActFusionStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputBoreSightType",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iBoreSightType].Read<ProjectCommonDbDefs::BoreSightTypeEnum>());});
				add_function_map("GetInputTrackingUser",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iTrackingUser].Read<CommonDbDefs::UserEnum>());});
				add_function_map("GetInputSlaCommStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iSlaCommStatus].Read<CommonDbDefs::CommunicationEnum>());});
				add_function_map("GetInputTrackingCamChange",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iTrackingCamChange].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetInputStabErrorGroupStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iStabErrorGroupStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputPowerErrorGroupStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iPowerErrorGroupStatus].Read<CommonDbDefs::StatusEnum>());});
				add_function_map("GetInputTrackingCam",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iTrackingCam].Read<ProjectCommonDbDefs::CamerasEnum>());});
				add_function_map("GetInputActFovCalibStatus",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iActFovCalibStatus].Read<CommonDbDefs::ActivityStatusEnum>());});
				add_function_map("GetInputLiBsOnOffRequest",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iLiBsOnOffRequest].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetInputMastClosed",[this](){ return static_cast<double>(m_inputTable[RulesDB::RulesInputDBEnum::iMastClosed].Read<CommonDbDefs::BoolEnum>());});


				add_function_map("GetOutputActSystemStatesCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActSystemStatesCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputGoToSystemState",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oGoToSystemState].Read<StateMachine_SystemStates::SystemStates_StatesEnum>());});
				add_function_map("GetOutputChangeWorkMode",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oChangeWorkMode].Read<CommonDbDefs::ModeEnum>());});
				add_function_map("GetOutputActWorkModeCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActWorkModeCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputPalmConditionForDrift",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oPalmConditionForDrift].Read<CommonDbDefs::ConditionEnum>());});
				add_function_map("GetOutputActDriftCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActDriftCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputActiveOperator",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActiveOperator].Read<CommonDbDefs::UserEnum>());});
				add_function_map("GetOutputCurrentMaster",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oCurrentMaster].Read<CommonDbDefs::UserEnum>());});
				add_function_map("GetOutputHandlePalmPermission",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oHandlePalmPermission].Read<UiDB::PermissionByUserEnum>());});
				add_function_map("GetOutputHandleAnalogsPermission",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oHandleAnalogsPermission].Read<UiDB::PermissionByUserEnum>());});
				add_function_map("GetOutputHandleFovPermission",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oHandleFovPermission].Read<UiDB::PermissionByUserEnum>());});
				add_function_map("GetOutputHandleTrackPermission",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oHandleTrackPermission].Read<UiDB::PermissionByUserEnum>());});
				add_function_map("GetOutputActControlCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActControlCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputHandleLasePermission",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oHandleLasePermission].Read<UiDB::PermissionByUserEnum>());});
				add_function_map("GetOutputActBringToCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActBringToCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputGoToBringToState",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oGoToBringToState].Read<StateMachine_BringTo::BringTo_StatesEnum>());});
				add_function_map("GetOutputProcessBringToPermission",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oProcessBringToPermission].Read<UiDB::PermissionByUserEnum>());});
				add_function_map("GetOutputActScanningCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActScanningCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputGoToScanningState",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oGoToScanningState].Read<StateMachine_Scanning::Scanning_StatesEnum>());});
				add_function_map("GetOutputActLaseCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActLaseCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputActCcdCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActCcdCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputActTiCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActTiCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputActVmdCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActVmdCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputActReticleCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActReticleCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputActVideoSwitchingCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActVideoSwitchingCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputActAutoTrackingCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActAutoTrackingCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputAutoTrackingRequest",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oAutoTrackingRequest].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetOutputActSlaCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActSlaCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputBringToPermission",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oBringToPermission].Read<UiDB::PermissionByUserEnum>());});
				add_function_map("GetOutputScanningPermission",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oScanningPermission].Read<UiDB::PermissionByUserEnum>());});
				add_function_map("GetOutputActSeCalibrationCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActSeCalibrationCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputActBoreSightCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActBoreSightCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputLiLasingRequest",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oLiLasingRequest].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetOutputLdLasingRequest",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oLdLasingRequest].Read<CommonDbDefs::OnOffStateEnum>());});
				add_function_map("GetOutputLiPowerEnableCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oLiPowerEnableCmd].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetOutputLdPowerEnableCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oLdPowerEnableCmd].Read<CommonDbDefs::BoolEnum>());});
				add_function_map("GetOutputLiProcessEnableCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oLiProcessEnableCmd].Read<CommonDbDefs::EnabledEnum>());});
				add_function_map("GetOutputLdProcessEnableCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oLdProcessEnableCmd].Read<CommonDbDefs::EnabledEnum>());});
				add_function_map("GetOutputConditionsForBringObsrvToGun",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oConditionsForBringObsrvToGun].Read<CommonDbDefs::ConditionEnum>());});
				add_function_map("GetOutputActFusionCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActFusionCmd].Read<CommonDbDefs::ActivityCmdEnum>());});
				add_function_map("GetOutputVmdPermission",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oVmdPermission].Read<UiDB::PermissionByUserEnum>());});
				add_function_map("GetOutputActFovCalibCmd",[this](){ return static_cast<double>(m_outputTable[RulesDB::RulesOutputDBEnum::oActFovCalibCmd].Read<CommonDbDefs::ActivityCmdEnum>());});


				add_function_map("GetResConditionForLdDayBoreSight",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLdDayBoreSight].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForLiDayBoreSight",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLiDayBoreSight].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForThermalBoreSight",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForThermalBoreSight].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForDrift",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForDrift].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResEndDriftFail",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndDriftFail].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResEndDriftSuccess",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndDriftSuccess].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetStartDrift",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartDrift].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResPalm_pressed",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rPalm_pressed].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForDayFovCalib",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForDayFovCalib].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForThermalFovCalib",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForThermalFovCalib].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResEnable_General_Activities",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rEnable_General_Activities].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResActiveOperatorCommander",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rActiveOperatorCommander].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResActiveOperatorGunner",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rActiveOperatorGunner].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResActiveOperatorNone",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rActiveOperatorNone].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResBoresightToMaintenance",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rBoresightToMaintenance].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResBoresightToOperational",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rBoresightToOperational].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResCalibrationToMaintenance",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rCalibrationToMaintenance].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResCalibrationToOperational",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rCalibrationToOperational].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResIbitToMaintenance",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rIbitToMaintenance].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResIBitToOperational",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rIBitToOperational].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResInitToOperational",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rInitToOperational].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetMaintenanceToBoresight",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rMaintenanceToBoresight].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetMaintenanceToCalibration",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rMaintenanceToCalibration].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetMaintenanceToIBit",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rMaintenanceToIBit].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResMaintenanceToOperational",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rMaintenanceToOperational].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResOperationalToMaintenance",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rOperationalToMaintenance].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResChangeToManualMode",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rChangeToManualMode].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResChangeToPowerMode",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rChangeToPowerMode].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForPowerMode",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForPowerMode].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResChangeToStabMode",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rChangeToStabMode].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForStabMode",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForStabMode].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForNuc",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForNuc].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForTracking",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForTracking].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResStartAutoTracking",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartAutoTracking].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResStopAutoTrackingByFail",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rStopAutoTrackingByFail].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResStopAutoTrackingByUser",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rStopAutoTrackingByUser].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResBringToInitiateEnded",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rBringToInitiateEnded].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForBringObsrvToGun",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForBringObsrvToGun].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForBringTo",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForBringTo].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResEndBringToFail",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndBringToFail].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResEndBringToSuccess",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndBringToSuccess].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResStartBringTo",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartBringTo].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResPerformLasing",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rPerformLasing].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResWakeLasingActivity",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rWakeLasingActivity].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForLdEnable",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLdEnable].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForLdPower",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLdPower].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResStartLdProcess",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartLdProcess].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResStopLdProcess",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rStopLdProcess].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForLiEnable",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLiEnable].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForLiPower",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLiPower].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResStartLiProcess",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartLiProcess].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResStopLiProcess",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rStopLiProcess].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForScanning",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForScanning].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResEndScanningFail",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndScanningFail].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResEndScanningSuccess",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndScanningSuccess].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResScanningInitiateEnded",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rScanningInitiateEnded].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResStartScanning",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartScanning].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResDisableHandles",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rDisableHandles].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResEnableLasing",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rEnableLasing].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetPermissionMasterCommander",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rPermissionMasterCommander].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResPermissionMasterGunner",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rPermissionMasterGunner].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetPermissionMasterNone",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rPermissionMasterNone].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResEnableVMD",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rEnableVMD].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResConditionForOpticButton",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForOpticButton].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResMasterCommander",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rMasterCommander].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetResMasterGunner",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rMasterGunner].Read<RulesDefs::RulesExistence>()); });
				add_function_map("GetMasterNone",[this](){ return static_cast<double>(m_existenceTable[RulesDB::RulesExistenceDBEnum::rMasterNone].Read<RulesDefs::RulesExistence>()); });


				add_function_map("GetEnabledConditionForLdDayBoreSight",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForLdDayBoreSight].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForLiDayBoreSight",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForLiDayBoreSight].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForThermalBoreSight",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForThermalBoreSight].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForDrift",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForDrift].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledEndDriftFail",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enEndDriftFail].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledEndDriftSuccess",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enEndDriftSuccess].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledStartDrift",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enStartDrift].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledPalm_pressed",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enPalm_pressed].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForDayFovCalib",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForDayFovCalib].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForThermalFovCalib",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForThermalFovCalib].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledEnable_General_Activities",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enEnable_General_Activities].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledActiveOperatorCommander",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enActiveOperatorCommander].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledActiveOperatorGunner",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enActiveOperatorGunner].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledActiveOperatorNone",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enActiveOperatorNone].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledBoresightToMaintenance",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enBoresightToMaintenance].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledBoresightToOperational",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enBoresightToOperational].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledCalibrationToMaintenance",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enCalibrationToMaintenance].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledCalibrationToOperational",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enCalibrationToOperational].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledIbitToMaintenance",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enIbitToMaintenance].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledIBitToOperational",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enIBitToOperational].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledInitToOperational",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enInitToOperational].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledMaintenanceToBoresight",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enMaintenanceToBoresight].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledMaintenanceToCalibration",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enMaintenanceToCalibration].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledMaintenanceToIBit",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enMaintenanceToIBit].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledMaintenanceToOperational",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enMaintenanceToOperational].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledOperationalToMaintenance",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enOperationalToMaintenance].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledChangeToManualMode",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enChangeToManualMode].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledChangeToPowerMode",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enChangeToPowerMode].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForPowerMode",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForPowerMode].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledChangeToStabMode",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enChangeToStabMode].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForStabMode",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForStabMode].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForNuc",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForNuc].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForTracking",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForTracking].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledStartAutoTracking",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enStartAutoTracking].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledStopAutoTrackingByFail",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enStopAutoTrackingByFail].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledStopAutoTrackingByUser",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enStopAutoTrackingByUser].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledBringToInitiateEnded",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enBringToInitiateEnded].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForBringObsrvToGun",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForBringObsrvToGun].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForBringTo",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForBringTo].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledEndBringToFail",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enEndBringToFail].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledEndBringToSuccess",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enEndBringToSuccess].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledStartBringTo",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enStartBringTo].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledPerformLasing",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enPerformLasing].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledWakeLasingActivity",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enWakeLasingActivity].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForLdEnable",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForLdEnable].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForLdPower",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForLdPower].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledStartLdProcess",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enStartLdProcess].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledStopLdProcess",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enStopLdProcess].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForLiEnable",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForLiEnable].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForLiPower",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForLiPower].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledStartLiProcess",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enStartLiProcess].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledStopLiProcess",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enStopLiProcess].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForScanning",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForScanning].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledEndScanningFail",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enEndScanningFail].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledEndScanningSuccess",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enEndScanningSuccess].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledScanningInitiateEnded",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enScanningInitiateEnded].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledStartScanning",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enStartScanning].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledDisableHandles",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enDisableHandles].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledEnableLasing",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enEnableLasing].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledPermissionMasterCommander",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enPermissionMasterCommander].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledPermissionMasterGunner",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enPermissionMasterGunner].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledPermissionMasterNone",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enPermissionMasterNone].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledEnableVMD",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enEnableVMD].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledConditionForOpticButton",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enConditionForOpticButton].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledMasterCommander",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enMasterCommander].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledMasterGunner",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enMasterGunner].Read<RulesDefs::RulesEnabled>()); });
				add_function_map("GetEnabledMasterNone",[this](){ return static_cast<double>(m_enableTable[RulesDB::RulesEnabledDBEnum::enMasterNone].Read<RulesDefs::RulesEnabled>()); });
			}
			void InitEnumarationMap()
			{
				m_enumMap.emplace("RULE_TRUE", RulesDefs::RulesExistence::RULE_TRUE);
				m_enumMap.emplace("RULE_FALSE", RulesDefs::RulesExistence::RULE_FALSE);
				m_enumMap.emplace("RULE_NOT_VALID", RulesDefs::RulesExistence::RULE_NOT_VALID);
				m_enumMap.emplace("RULE_ENABLE", RulesDefs::RulesEnabled::RULE_ENABLE);
				m_enumMap.emplace("RULE_DISABLE", RulesDefs::RulesEnabled::RULE_DISABLE);
				m_enumMap.emplace("RULE_ENABLE_NOT_VALID", RulesDefs::RulesEnabled::RULE_ENABLE_NOT_VALID);
				m_enumMap.emplace("NO_TRANSITION",StateMachine_BringTo::BringTo_TransitionsEnum::NO_TRANSITION);
				m_enumMap.emplace("GO_TO_BRING_TO",StateMachine_BringTo::BringTo_TransitionsEnum::GO_TO_BRING_TO);
				m_enumMap.emplace("GO_TO_END_SUCCESS",StateMachine_BringTo::BringTo_TransitionsEnum::GO_TO_END_SUCCESS);
				m_enumMap.emplace("GO_TO_END_FAIL",StateMachine_BringTo::BringTo_TransitionsEnum::GO_TO_END_FAIL);
				m_enumMap.emplace("OPERATOR_NONE",CommonDbDefs::OperatorTypesEnum::OPERATOR_NONE);
				m_enumMap.emplace("OPERATOR_GUNNER",CommonDbDefs::OperatorTypesEnum::OPERATOR_GUNNER);
				m_enumMap.emplace("OPERATOR_COMMANDER",CommonDbDefs::OperatorTypesEnum::OPERATOR_COMMANDER);
				m_enumMap.emplace("NUM_OF_OPERATOR_TYPES",CommonDbDefs::OperatorTypesEnum::NUM_OF_OPERATOR_TYPES);
				m_enumMap.emplace("USER_REQUEST_NONE",CommonDbDefs::UserRequestEnum::USER_REQUEST_NONE);
				m_enumMap.emplace("USER_REQUEST_ENTER_PROCESS",CommonDbDefs::UserRequestEnum::USER_REQUEST_ENTER_PROCESS);
				m_enumMap.emplace("USER_REQUEST_EXIT_PROCESS",CommonDbDefs::UserRequestEnum::USER_REQUEST_EXIT_PROCESS);
				m_enumMap.emplace("USER_REQUEST_ABORT_PROCESS",CommonDbDefs::UserRequestEnum::USER_REQUEST_ABORT_PROCESS);
				m_enumMap.emplace("USER_REQUEST_RESET_PROCESS",CommonDbDefs::UserRequestEnum::USER_REQUEST_RESET_PROCESS);
				m_enumMap.emplace("MOBILITY_NONE",CommonDbDefs::MobilityTypeEnum::MOBILITY_NONE);
				m_enumMap.emplace("MOBILITY_VELOCITY",CommonDbDefs::MobilityTypeEnum::MOBILITY_VELOCITY);
				m_enumMap.emplace("MOBILITY_ACCELERATION",CommonDbDefs::MobilityTypeEnum::MOBILITY_ACCELERATION);
				m_enumMap.emplace("ACTIVITY_SUSPENDED",CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				m_enumMap.emplace("ACTIVITY_READY",CommonDbDefs::ActivityStatusEnum::ACTIVITY_READY);
				m_enumMap.emplace("ACTIVITY_IN_PROCESS",CommonDbDefs::ActivityStatusEnum::ACTIVITY_IN_PROCESS);
				m_enumMap.emplace("ACTIVITY_END_SUCCESS",CommonDbDefs::ActivityStatusEnum::ACTIVITY_END_SUCCESS);
				m_enumMap.emplace("ACTIVITY_END_FAIL",CommonDbDefs::ActivityStatusEnum::ACTIVITY_END_FAIL);
				m_enumMap.emplace("ACTIVITY_END_ABORT",CommonDbDefs::ActivityStatusEnum::ACTIVITY_END_ABORT);
				m_enumMap.emplace("ACTIVITY_SUSPEND",CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				m_enumMap.emplace("ACTIVITY_WAKE",CommonDbDefs::ActivityCmdEnum::ACTIVITY_WAKE);
				m_enumMap.emplace("ACTIVITY_START",CommonDbDefs::ActivityCmdEnum::ACTIVITY_START);
				m_enumMap.emplace("ACTIVITY_STOP_SUCCESS",CommonDbDefs::ActivityCmdEnum::ACTIVITY_STOP_SUCCESS);
				m_enumMap.emplace("ACTIVITY_STOP_FAIL",CommonDbDefs::ActivityCmdEnum::ACTIVITY_STOP_FAIL);
				m_enumMap.emplace("ACTIVITY_STOP_ABORT",CommonDbDefs::ActivityCmdEnum::ACTIVITY_STOP_ABORT);
				m_enumMap.emplace("COMMUNICATION_NONE",CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);
				m_enumMap.emplace("COMMUNICATION_OK",CommonDbDefs::CommunicationEnum::COMMUNICATION_OK);
				m_enumMap.emplace("COMMUNICATION_FAILURE",CommonDbDefs::CommunicationEnum::COMMUNICATION_FAILURE);
				m_enumMap.emplace("BOOL_NONE",CommonDbDefs::BoolEnum::BOOL_NONE);
				m_enumMap.emplace("BOOL_FALSE",CommonDbDefs::BoolEnum::BOOL_FALSE);
				m_enumMap.emplace("BOOL_TRUE",CommonDbDefs::BoolEnum::BOOL_TRUE);
				m_enumMap.emplace("STATUS_NONE",CommonDbDefs::StatusEnum::STATUS_NONE);
				m_enumMap.emplace("STATUS_FAIL",CommonDbDefs::StatusEnum::STATUS_FAIL);
				m_enumMap.emplace("STATUS_OK",CommonDbDefs::StatusEnum::STATUS_OK);
				m_enumMap.emplace("TRI_SWITCH_ERROR",CommonDbDefs::TriSwitchEnum::TRI_SWITCH_ERROR);
				m_enumMap.emplace("TRI_SWITCH_NONE",CommonDbDefs::TriSwitchEnum::TRI_SWITCH_NONE);
				m_enumMap.emplace("TRI_SWITCH_MIDDLE",CommonDbDefs::TriSwitchEnum::TRI_SWITCH_MIDDLE);
				m_enumMap.emplace("TRI_SWITCH_UP",CommonDbDefs::TriSwitchEnum::TRI_SWITCH_UP);
				m_enumMap.emplace("TRI_SWITCH_DOWN",CommonDbDefs::TriSwitchEnum::TRI_SWITCH_DOWN);
				m_enumMap.emplace("BUTTON_NONE",CommonDbDefs::ButtonEnum::BUTTON_NONE);
				m_enumMap.emplace("BUTTON_RELEASED",CommonDbDefs::ButtonEnum::BUTTON_RELEASED);
				m_enumMap.emplace("BUTTON_PRESSED",CommonDbDefs::ButtonEnum::BUTTON_PRESSED);
				m_enumMap.emplace("ENABLED_NONE",CommonDbDefs::EnabledEnum::ENABLED_NONE);
				m_enumMap.emplace("ENABLED_FALSE",CommonDbDefs::EnabledEnum::ENABLED_FALSE);
				m_enumMap.emplace("ENABLED_TRUE",CommonDbDefs::EnabledEnum::ENABLED_TRUE);
				m_enumMap.emplace("MOBILE_ERROR",CommonDbDefs::MobileEnum::MOBILE_ERROR);
				m_enumMap.emplace("MOBILE_NONE",CommonDbDefs::MobileEnum::MOBILE_NONE);
				m_enumMap.emplace("MOBILE_FALSE",CommonDbDefs::MobileEnum::MOBILE_FALSE);
				m_enumMap.emplace("MOBILE_TRUE",CommonDbDefs::MobileEnum::MOBILE_TRUE);
				m_enumMap.emplace("MODE_ERROR",CommonDbDefs::ModeEnum::MODE_ERROR);
				m_enumMap.emplace("MODE_NONE",CommonDbDefs::ModeEnum::MODE_NONE);
				m_enumMap.emplace("MODE_MANUAL",CommonDbDefs::ModeEnum::MODE_MANUAL);
				m_enumMap.emplace("MODE_POWER",CommonDbDefs::ModeEnum::MODE_POWER);
				m_enumMap.emplace("MODE_STAB",CommonDbDefs::ModeEnum::MODE_STAB);
				m_enumMap.emplace("PROCESS_ERROR",CommonDbDefs::ProcessEnum::PROCESS_ERROR);
				m_enumMap.emplace("PROCESS_NONE",CommonDbDefs::ProcessEnum::PROCESS_NONE);
				m_enumMap.emplace("PROCESS_STOPPED",CommonDbDefs::ProcessEnum::PROCESS_STOPPED);
				m_enumMap.emplace("PROCESS_RUNNING",CommonDbDefs::ProcessEnum::PROCESS_RUNNING);
				m_enumMap.emplace("PROCESS_FINISHED_FAIL",CommonDbDefs::ProcessEnum::PROCESS_FINISHED_FAIL);
				m_enumMap.emplace("PROCESS_FINISHED_OK",CommonDbDefs::ProcessEnum::PROCESS_FINISHED_OK);
				m_enumMap.emplace("BIT_NONE",CommonDbDefs::BitEnum::BIT_NONE);
				m_enumMap.emplace("BIT_FAIL",CommonDbDefs::BitEnum::BIT_FAIL);
				m_enumMap.emplace("BIT_OK",CommonDbDefs::BitEnum::BIT_OK);
				m_enumMap.emplace("BIT_TYPE_NONE",CommonDbDefs::BitTypeEnum::BIT_TYPE_NONE);
				m_enumMap.emplace("BIT_TYPE_INITIALIZATION",CommonDbDefs::BitTypeEnum::BIT_TYPE_INITIALIZATION);
				m_enumMap.emplace("BIT_TYPE_PERIODIC",CommonDbDefs::BitTypeEnum::BIT_TYPE_PERIODIC);
				m_enumMap.emplace("BIT_TYPE_INITIATED",CommonDbDefs::BitTypeEnum::BIT_TYPE_INITIATED);
				m_enumMap.emplace("TRACK_NONE",CommonDbDefs::TrackEnum::TRACK_NONE);
				m_enumMap.emplace("TRACK_NOT_AQUIRE",CommonDbDefs::TrackEnum::TRACK_NOT_AQUIRE);
				m_enumMap.emplace("TRACK_AQUIRE",CommonDbDefs::TrackEnum::TRACK_AQUIRE);
				m_enumMap.emplace("TRACK_COAST",CommonDbDefs::TrackEnum::TRACK_COAST);
				m_enumMap.emplace("RANGE_TYPE_NONE",CommonDbDefs::RangeTypeEnum::RANGE_TYPE_NONE);
				m_enumMap.emplace("RANGE_TYPE_MANUAL",CommonDbDefs::RangeTypeEnum::RANGE_TYPE_MANUAL);
				m_enumMap.emplace("RANGE_TYPE_LASER",CommonDbDefs::RangeTypeEnum::RANGE_TYPE_LASER);
				m_enumMap.emplace("NUM_OF_RANGE_TYPES",CommonDbDefs::RangeTypeEnum::NUM_OF_RANGE_TYPES);
				m_enumMap.emplace("STATE_ERROR",CommonDbDefs::OnOffStateEnum::STATE_ERROR);
				m_enumMap.emplace("STATE_NONE",CommonDbDefs::OnOffStateEnum::STATE_NONE);
				m_enumMap.emplace("STATE_OFF",CommonDbDefs::OnOffStateEnum::STATE_OFF);
				m_enumMap.emplace("STATE_ON",CommonDbDefs::OnOffStateEnum::STATE_ON);
				m_enumMap.emplace("FIRE_STATUS_NONE",CommonDbDefs::FireStatusEnum::FIRE_STATUS_NONE);
				m_enumMap.emplace("FIRE_STATUS_DISABLED",CommonDbDefs::FireStatusEnum::FIRE_STATUS_DISABLED);
				m_enumMap.emplace("FIRE_STATUS_ENABLED",CommonDbDefs::FireStatusEnum::FIRE_STATUS_ENABLED);
				m_enumMap.emplace("UNIT_STATUS_NONE",CommonDbDefs::UnitStatusEnum::UNIT_STATUS_NONE);
				m_enumMap.emplace("UNIT_STATUS_ON",CommonDbDefs::UnitStatusEnum::UNIT_STATUS_ON);
				m_enumMap.emplace("UNIT_STATUS_OPERATOR_OFF",CommonDbDefs::UnitStatusEnum::UNIT_STATUS_OPERATOR_OFF);
				m_enumMap.emplace("UNIT_STATUS_MANUAL",CommonDbDefs::UnitStatusEnum::UNIT_STATUS_MANUAL);
				m_enumMap.emplace("UNIT_STATUS_COMPUTER_OFF",CommonDbDefs::UnitStatusEnum::UNIT_STATUS_COMPUTER_OFF);
				m_enumMap.emplace("LED_MODE_NONE",CommonDbDefs::LedModeEnum::LED_MODE_NONE);
				m_enumMap.emplace("LED_MODE_OFF",CommonDbDefs::LedModeEnum::LED_MODE_OFF);
				m_enumMap.emplace("LED_MODE_ON",CommonDbDefs::LedModeEnum::LED_MODE_ON);
				m_enumMap.emplace("LED_MODE_BLINK",CommonDbDefs::LedModeEnum::LED_MODE_BLINK);
				m_enumMap.emplace("OPEN_CLOSED_STATE_ERROR",CommonDbDefs::OpenClosedStateEnum::OPEN_CLOSED_STATE_ERROR);
				m_enumMap.emplace("OPEN_CLOSED_STATE_NONE",CommonDbDefs::OpenClosedStateEnum::OPEN_CLOSED_STATE_NONE);
				m_enumMap.emplace("STATE_CLOSED",CommonDbDefs::OpenClosedStateEnum::STATE_CLOSED);
				m_enumMap.emplace("STATE_OPENED",CommonDbDefs::OpenClosedStateEnum::STATE_OPENED);
				m_enumMap.emplace("IO_MODE_NONE",CommonDbDefs::InputOutputModeEnum::IO_MODE_NONE);
				m_enumMap.emplace("IO_MODE_INPUT",CommonDbDefs::InputOutputModeEnum::IO_MODE_INPUT);
				m_enumMap.emplace("IO_MODE_OUTPUT",CommonDbDefs::InputOutputModeEnum::IO_MODE_OUTPUT);
				m_enumMap.emplace("ANGLE_UNITS_UNDEFINED",CommonDbDefs::AngleUnitsEnum::ANGLE_UNITS_UNDEFINED);
				m_enumMap.emplace("ANGLE_UNITS_RADS",CommonDbDefs::AngleUnitsEnum::ANGLE_UNITS_RADS);
				m_enumMap.emplace("ANGLE_UNITS_MILLS",CommonDbDefs::AngleUnitsEnum::ANGLE_UNITS_MILLS);
				m_enumMap.emplace("ANGLE_UNITS_MRADS",CommonDbDefs::AngleUnitsEnum::ANGLE_UNITS_MRADS);
				m_enumMap.emplace("ANGLE_UNITS_DEGS",CommonDbDefs::AngleUnitsEnum::ANGLE_UNITS_DEGS);
				m_enumMap.emplace("POSITION_UNITS_UNDEFINED",CommonDbDefs::PositionUnitsEnum::POSITION_UNITS_UNDEFINED);
				m_enumMap.emplace("POSITION_UNITS_PIXELS",CommonDbDefs::PositionUnitsEnum::POSITION_UNITS_PIXELS);
				m_enumMap.emplace("BS_MODE_NOT_IN_PROGRESS",CommonDbDefs::BoresightModeEnum::BS_MODE_NOT_IN_PROGRESS);
				m_enumMap.emplace("BS_MODE_IN_PROGRESS",CommonDbDefs::BoresightModeEnum::BS_MODE_IN_PROGRESS);
				m_enumMap.emplace("BS_MODE_EXIT_SUCCESS",CommonDbDefs::BoresightModeEnum::BS_MODE_EXIT_SUCCESS);
				m_enumMap.emplace("BS_MODE_EXIT_FAIL",CommonDbDefs::BoresightModeEnum::BS_MODE_EXIT_FAIL);
				m_enumMap.emplace("BS_MODE_EXIT_FAIL_TRV",CommonDbDefs::BoresightModeEnum::BS_MODE_EXIT_FAIL_TRV);
				m_enumMap.emplace("BS_MODE_EXIT_FAIL_ELV",CommonDbDefs::BoresightModeEnum::BS_MODE_EXIT_FAIL_ELV);
				m_enumMap.emplace("BS_MODE_EXIT_ABORTED",CommonDbDefs::BoresightModeEnum::BS_MODE_EXIT_ABORTED);
				m_enumMap.emplace("BS_ACTION_NONE",CommonDbDefs::ReticleBoresightActionEnum::BS_ACTION_NONE);
				m_enumMap.emplace("BS_ACTION_START",CommonDbDefs::ReticleBoresightActionEnum::BS_ACTION_START);
				m_enumMap.emplace("BS_ACTION_ENTER",CommonDbDefs::ReticleBoresightActionEnum::BS_ACTION_ENTER);
				m_enumMap.emplace("BS_ACTION_ABORT",CommonDbDefs::ReticleBoresightActionEnum::BS_ACTION_ABORT);
				m_enumMap.emplace("BS_ACTION_RESET",CommonDbDefs::ReticleBoresightActionEnum::BS_ACTION_RESET);
				m_enumMap.emplace("BS_ACTION_BACK",CommonDbDefs::ReticleBoresightActionEnum::BS_ACTION_BACK);
				m_enumMap.emplace("RETICLE_MOVEMENT_NO_MOVEMENT",CommonDbDefs::ReticleMovementEnum::RETICLE_MOVEMENT_NO_MOVEMENT);
				m_enumMap.emplace("RETICLE_MOVEMENT_STEP_UP",CommonDbDefs::ReticleMovementEnum::RETICLE_MOVEMENT_STEP_UP);
				m_enumMap.emplace("RETICLE_MOVEMENT_STEP_DOWN",CommonDbDefs::ReticleMovementEnum::RETICLE_MOVEMENT_STEP_DOWN);
				m_enumMap.emplace("RETICLE_MOVEMENT_STEP_LEFT",CommonDbDefs::ReticleMovementEnum::RETICLE_MOVEMENT_STEP_LEFT);
				m_enumMap.emplace("RETICLE_MOVEMENT_STEP_RIGHT",CommonDbDefs::ReticleMovementEnum::RETICLE_MOVEMENT_STEP_RIGHT);
				m_enumMap.emplace("VALIDITY_INDETERMINATE",CommonDbDefs::ValidityEnum::VALIDITY_INDETERMINATE);
				m_enumMap.emplace("VALID",CommonDbDefs::ValidityEnum::VALID);
				m_enumMap.emplace("NOT_VALID",CommonDbDefs::ValidityEnum::NOT_VALID);
				m_enumMap.emplace("RANGE_MEASURE_NONE",CommonDbDefs::RangeMeasureStatusEnum::RANGE_MEASURE_NONE);
				m_enumMap.emplace("RANGE_MEASURE_NO_RANGE",CommonDbDefs::RangeMeasureStatusEnum::RANGE_MEASURE_NO_RANGE);
				m_enumMap.emplace("RANGE_MEASURE_SINGLE",CommonDbDefs::RangeMeasureStatusEnum::RANGE_MEASURE_SINGLE);
				m_enumMap.emplace("RANGE_MEASURE_MULTIPLE",CommonDbDefs::RangeMeasureStatusEnum::RANGE_MEASURE_MULTIPLE);
				m_enumMap.emplace("RANGE_MEASURE_SKY_LASING",CommonDbDefs::RangeMeasureStatusEnum::RANGE_MEASURE_SKY_LASING);
				m_enumMap.emplace("RANGE_MEASURE_ERROR",CommonDbDefs::RangeMeasureStatusEnum::RANGE_MEASURE_ERROR);
				m_enumMap.emplace("NUM_OF_RANGE_MEASURE_STATUSES",CommonDbDefs::RangeMeasureStatusEnum::NUM_OF_RANGE_MEASURE_STATUSES);
				m_enumMap.emplace("BRAKES_NONE",CommonDbDefs::BrakesOpenCloseEnum::BRAKES_NONE);
				m_enumMap.emplace("BRAKES_CLOSE",CommonDbDefs::BrakesOpenCloseEnum::BRAKES_CLOSE);
				m_enumMap.emplace("BRAKES_OPEN",CommonDbDefs::BrakesOpenCloseEnum::BRAKES_OPEN);
				m_enumMap.emplace("CMD_NONE",CommonDbDefs::StartStopCmdEnum::CMD_NONE);
				m_enumMap.emplace("CMD_STOP",CommonDbDefs::StartStopCmdEnum::CMD_STOP);
				m_enumMap.emplace("CMD_START",CommonDbDefs::StartStopCmdEnum::CMD_START);
				m_enumMap.emplace("MODE_UNSUPPORTED",CommonDbDefs::FunctionModeEnum::MODE_UNSUPPORTED);
				m_enumMap.emplace("MODE_UNKNOWN",CommonDbDefs::FunctionModeEnum::MODE_UNKNOWN);
				m_enumMap.emplace("MANUAL_MODE",CommonDbDefs::FunctionModeEnum::MANUAL_MODE);
				m_enumMap.emplace("AUTO_MODE",CommonDbDefs::FunctionModeEnum::AUTO_MODE);
				m_enumMap.emplace("USER_NONE",CommonDbDefs::UserEnum::USER_NONE);
				m_enumMap.emplace("USER_COMMANDER",CommonDbDefs::UserEnum::USER_COMMANDER);
				m_enumMap.emplace("USER_GUNNER",CommonDbDefs::UserEnum::USER_GUNNER);
				m_enumMap.emplace("INS_NONE",CommonDbDefs::InsModeTypeEnum::INS_NONE);
				m_enumMap.emplace("INS_OFF",CommonDbDefs::InsModeTypeEnum::INS_OFF);
				m_enumMap.emplace("INS_STANDARD_ALIGN",CommonDbDefs::InsModeTypeEnum::INS_STANDARD_ALIGN);
				m_enumMap.emplace("INS_STORED_HEADING_ALIGN",CommonDbDefs::InsModeTypeEnum::INS_STORED_HEADING_ALIGN);
				m_enumMap.emplace("INS_STANDBY",CommonDbDefs::InsModeTypeEnum::INS_STANDBY);
				m_enumMap.emplace("INS_UPDATE_POSITION",CommonDbDefs::InsModeTypeEnum::INS_UPDATE_POSITION);
				m_enumMap.emplace("INS_GPS_ALIGNMENT",CommonDbDefs::InsModeTypeEnum::INS_GPS_ALIGNMENT);
				m_enumMap.emplace("ACTION_NONE",CommonDbDefs::ActionEnum::ACTION_NONE);
				m_enumMap.emplace("ACTION_STOP",CommonDbDefs::ActionEnum::ACTION_STOP);
				m_enumMap.emplace("ACTION_EXIT",CommonDbDefs::ActionEnum::ACTION_EXIT);
				m_enumMap.emplace("ACTION_START",CommonDbDefs::ActionEnum::ACTION_START);
				m_enumMap.emplace("VELOCITY_NONE",CommonDbDefs::VelocityEnum::VELOCITY_NONE);
				m_enumMap.emplace("VERY_SLOW_VELOCITY",CommonDbDefs::VelocityEnum::VERY_SLOW_VELOCITY);
				m_enumMap.emplace("SLOW_VELOCITY",CommonDbDefs::VelocityEnum::SLOW_VELOCITY);
				m_enumMap.emplace("NORMAL_VELOCITY",CommonDbDefs::VelocityEnum::NORMAL_VELOCITY);
				m_enumMap.emplace("FAST_VELOCITY",CommonDbDefs::VelocityEnum::FAST_VELOCITY);
				m_enumMap.emplace("VERY_FAST_VELOCITY",CommonDbDefs::VelocityEnum::VERY_FAST_VELOCITY);
				m_enumMap.emplace("NUM_OF_VELOCITIES",CommonDbDefs::VelocityEnum::NUM_OF_VELOCITIES);
				m_enumMap.emplace("UNKNOWN_SYSTEM",CommonDbDefs::SystemTypeEnum::UNKNOWN_SYSTEM);
				m_enumMap.emplace("OBSERVATION_SYSTEM",CommonDbDefs::SystemTypeEnum::OBSERVATION_SYSTEM);
				m_enumMap.emplace("IMAGE_COLOR_NONE",CommonDbDefs::DayImageColorEnum::IMAGE_COLOR_NONE);
				m_enumMap.emplace("BW_IMAGE",CommonDbDefs::DayImageColorEnum::BW_IMAGE);
				m_enumMap.emplace("COLOR_IMAGE",CommonDbDefs::DayImageColorEnum::COLOR_IMAGE);
				m_enumMap.emplace("FREEZE_NONE",CommonDbDefs::FreezeEnum::FREEZE_NONE);
				m_enumMap.emplace("NO_FREEZE",CommonDbDefs::FreezeEnum::NO_FREEZE);
				m_enumMap.emplace("FREEZE",CommonDbDefs::FreezeEnum::FREEZE);
				m_enumMap.emplace("FUSION_TI_NONE",CommonDbDefs::FusionTiHotPartsColorEnum::FUSION_TI_NONE);
				m_enumMap.emplace("FUSION_TI_BLUE",CommonDbDefs::FusionTiHotPartsColorEnum::FUSION_TI_BLUE);
				m_enumMap.emplace("FUSION_TI_GREEN",CommonDbDefs::FusionTiHotPartsColorEnum::FUSION_TI_GREEN);
				m_enumMap.emplace("FUSION_TI_RED",CommonDbDefs::FusionTiHotPartsColorEnum::FUSION_TI_RED);
				m_enumMap.emplace("FUSION_TI_YELLOW",CommonDbDefs::FusionTiHotPartsColorEnum::FUSION_TI_YELLOW);
				m_enumMap.emplace("FUSION_TI_DEFAULT",CommonDbDefs::FusionTiHotPartsColorEnum::FUSION_TI_DEFAULT);
				m_enumMap.emplace("FUSION_TI_NO_COLOR",CommonDbDefs::FusionTiHotPartsColorEnum::FUSION_TI_NO_COLOR);
				m_enumMap.emplace("INVERT_NONE",CommonDbDefs::InvertImageEnum::INVERT_NONE);
				m_enumMap.emplace("NORMAL_INVERT",CommonDbDefs::InvertImageEnum::NORMAL_INVERT);
				m_enumMap.emplace("INVERT_UP_DOWN",CommonDbDefs::InvertImageEnum::INVERT_UP_DOWN);
				m_enumMap.emplace("INVERT_LEFT_RIGHT",CommonDbDefs::InvertImageEnum::INVERT_LEFT_RIGHT);
				m_enumMap.emplace("INVERT_BOTH_AXES",CommonDbDefs::InvertImageEnum::INVERT_BOTH_AXES);
				m_enumMap.emplace("INVERT_NOT_AVAILABLE",CommonDbDefs::InvertImageEnum::INVERT_NOT_AVAILABLE);
				m_enumMap.emplace("AVERAGING_NONE",CommonDbDefs::AveragingEnum::AVERAGING_NONE);
				m_enumMap.emplace("AVERAGING_OFF",CommonDbDefs::AveragingEnum::AVERAGING_OFF);
				m_enumMap.emplace("AVERAGING_LOW_FREQUENCY",CommonDbDefs::AveragingEnum::AVERAGING_LOW_FREQUENCY);
				m_enumMap.emplace("AVERAGING_HIGH_FREQUENCY",CommonDbDefs::AveragingEnum::AVERAGING_HIGH_FREQUENCY);
				m_enumMap.emplace("AVERAGING_NOT_AVAILABLE",CommonDbDefs::AveragingEnum::AVERAGING_NOT_AVAILABLE);
				m_enumMap.emplace("FUNCTION_NONE",CommonDbDefs::CameraDisplayFunctionsEnum::FUNCTION_NONE);
				m_enumMap.emplace("FUNCTION_ERASE",CommonDbDefs::CameraDisplayFunctionsEnum::FUNCTION_ERASE);
				m_enumMap.emplace("FUNCTION_DISPLAY",CommonDbDefs::CameraDisplayFunctionsEnum::FUNCTION_DISPLAY);
				m_enumMap.emplace("FUNCTION_KEEP",CommonDbDefs::CameraDisplayFunctionsEnum::FUNCTION_KEEP);
				m_enumMap.emplace("VIDEO_CONFIGURATION_NONE",CommonDbDefs::VideoConfigurationEnum::VIDEO_CONFIGURATION_NONE);
				m_enumMap.emplace("VIDEO_CONFIGURATION_DAY",CommonDbDefs::VideoConfigurationEnum::VIDEO_CONFIGURATION_DAY);
				m_enumMap.emplace("VIDEO_CONFIGURATION_TI",CommonDbDefs::VideoConfigurationEnum::VIDEO_CONFIGURATION_TI);
				m_enumMap.emplace("VIDEO_CONFIGURATION_LRF",CommonDbDefs::VideoConfigurationEnum::VIDEO_CONFIGURATION_LRF);
				m_enumMap.emplace("VIDEO_CONFIGURATION_SLA",CommonDbDefs::VideoConfigurationEnum::VIDEO_CONFIGURATION_SLA);
				m_enumMap.emplace("VIDEO_CONFIGURATION_ATT",CommonDbDefs::VideoConfigurationEnum::VIDEO_CONFIGURATION_ATT);
				m_enumMap.emplace("NUM_OF_VIDEOS_CONFIGURATION",CommonDbDefs::VideoConfigurationEnum::NUM_OF_VIDEOS_CONFIGURATION);
				m_enumMap.emplace("LANGUAGE_ENGLISH",CommonDbDefs::LanguageEnum::LANGUAGE_ENGLISH);
				m_enumMap.emplace("LANGUAGE_OTHER",CommonDbDefs::LanguageEnum::LANGUAGE_OTHER);
				m_enumMap.emplace("CONDITION_ERROR",CommonDbDefs::ConditionEnum::CONDITION_ERROR);
				m_enumMap.emplace("CONDITION_NONE",CommonDbDefs::ConditionEnum::CONDITION_NONE);
				m_enumMap.emplace("CONDITION_NO",CommonDbDefs::ConditionEnum::CONDITION_NO);
				m_enumMap.emplace("CONDITION_YES",CommonDbDefs::ConditionEnum::CONDITION_YES);
				m_enumMap.emplace("RANGE_SELECTION_NONE",CommonDbDefs::RangeSelectionEnum::RANGE_SELECTION_NONE);
				m_enumMap.emplace("RANGE_SELECTION_FIRST",CommonDbDefs::RangeSelectionEnum::RANGE_SELECTION_FIRST);
				m_enumMap.emplace("RANGE_SELECTION_LAST",CommonDbDefs::RangeSelectionEnum::RANGE_SELECTION_LAST);
				m_enumMap.emplace("RETICLE_COLOR_NONE",CommonDbDefs::ReticleColorEnum::RETICLE_COLOR_NONE);
				m_enumMap.emplace("RETICLE_COLOR_WHITE",CommonDbDefs::ReticleColorEnum::RETICLE_COLOR_WHITE);
				m_enumMap.emplace("RETICLE_COLOR_BLACK",CommonDbDefs::ReticleColorEnum::RETICLE_COLOR_BLACK);
				m_enumMap.emplace("RETICLE_COLOR_GREEN",CommonDbDefs::ReticleColorEnum::RETICLE_COLOR_GREEN);
				m_enumMap.emplace("CAMERA_NOT_RELEVANT_VALUE",CommonDbDefs::Misc::CAMERA_NOT_RELEVANT_VALUE);
				m_enumMap.emplace("SLA_SENSITIVITY_TYPE_NONE",CommonDbDefs::SlaSensitivityTypeEnum::SLA_SENSITIVITY_TYPE_NONE);
				m_enumMap.emplace("LARGE_TARGET_MTD_THRESHOLD",CommonDbDefs::SlaSensitivityTypeEnum::LARGE_TARGET_MTD_THRESHOLD);
				m_enumMap.emplace("SMALL_TARGET_MTI_SENSITIVITY",CommonDbDefs::SlaSensitivityTypeEnum::SMALL_TARGET_MTI_SENSITIVITY);
				m_enumMap.emplace("TRACKER_IS_IDLE",CommonDbDefs::TrackerOperationEnum::TRACKER_IS_IDLE);
				m_enumMap.emplace("TRACKING",CommonDbDefs::TrackerOperationEnum::TRACKING);
				m_enumMap.emplace("COASTING",CommonDbDefs::TrackerOperationEnum::COASTING);
				m_enumMap.emplace("LRF_CAM_CMD_RETICLE_GREY_LEVEL",CommonDbDefs::LrfCamCmdTypeEnum::LRF_CAM_CMD_RETICLE_GREY_LEVEL);
				m_enumMap.emplace("LRF_CAM_CMD_ZOOM",CommonDbDefs::LrfCamCmdTypeEnum::LRF_CAM_CMD_ZOOM);
				m_enumMap.emplace("LRF_CAM_CMD_OVERLAY_SHOW",CommonDbDefs::LrfCamCmdTypeEnum::LRF_CAM_CMD_OVERLAY_SHOW);
				m_enumMap.emplace("LRF_CAM_CMD_COLOR_GAIN",CommonDbDefs::LrfCamCmdTypeEnum::LRF_CAM_CMD_COLOR_GAIN);
				m_enumMap.emplace("OPERATOR_NONE",ControllerDefs::OperatorsEnum::OPERATOR_NONE);
				m_enumMap.emplace("OPERATOR_1",ControllerDefs::OperatorsEnum::OPERATOR_1);
				m_enumMap.emplace("OPERATOR_2",ControllerDefs::OperatorsEnum::OPERATOR_2);
				m_enumMap.emplace("OPERATOR_3",ControllerDefs::OperatorsEnum::OPERATOR_3);
				m_enumMap.emplace("OPERATOR_4",ControllerDefs::OperatorsEnum::OPERATOR_4);
				m_enumMap.emplace("OPERATOR_5",ControllerDefs::OperatorsEnum::OPERATOR_5);
				m_enumMap.emplace("OPERATOR_6",ControllerDefs::OperatorsEnum::OPERATOR_6);
				m_enumMap.emplace("PERMISSION_NONE",ControllerDefs::PermissionTypeEnum::PERMISSION_NONE);
				m_enumMap.emplace("PERMISSION_ALL",ControllerDefs::PermissionTypeEnum::PERMISSION_ALL);
				m_enumMap.emplace("PERMISSION_AUTHORITY",ControllerDefs::PermissionTypeEnum::PERMISSION_AUTHORITY);
				m_enumMap.emplace("PERMISSION_USER_ACTIVE",ControllerDefs::PermissionTypeEnum::PERMISSION_USER_ACTIVE);
				m_enumMap.emplace("PERMISSION_USER_1",ControllerDefs::PermissionTypeEnum::PERMISSION_USER_1);
				m_enumMap.emplace("PERMISSION_USER_2",ControllerDefs::PermissionTypeEnum::PERMISSION_USER_2);
				m_enumMap.emplace("PERMISSION_USER_3",ControllerDefs::PermissionTypeEnum::PERMISSION_USER_3);
				m_enumMap.emplace("PERMISSION_USER_4",ControllerDefs::PermissionTypeEnum::PERMISSION_USER_4);
				m_enumMap.emplace("PERMISSION_USER_5",ControllerDefs::PermissionTypeEnum::PERMISSION_USER_5);
				m_enumMap.emplace("PERMISSION_USER_6",ControllerDefs::PermissionTypeEnum::PERMISSION_USER_6);
				m_enumMap.emplace("BRING_TO_STATE_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::BRING_TO_STATE_STS);
				m_enumMap.emplace("BRING_TO_COORDINATES_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::BRING_TO_COORDINATES_STS);
				m_enumMap.emplace("BRING_TO_OUT_OF_LIMITS_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::BRING_TO_OUT_OF_LIMITS_STS);
				m_enumMap.emplace("SCANNING_STATE_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::SCANNING_STATE_STS);
				m_enumMap.emplace("SCANNING_COORDINATES_START_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::SCANNING_COORDINATES_START_STS);
				m_enumMap.emplace("SCANNING_COORDINATES_END_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::SCANNING_COORDINATES_END_STS);
				m_enumMap.emplace("SCANNING_OUT_OF_LIMITS_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::SCANNING_OUT_OF_LIMITS_STS);
				m_enumMap.emplace("SCANNING_VELOCITY_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::SCANNING_VELOCITY_STS);
				m_enumMap.emplace("SYSTEM_RANGE_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::SYSTEM_RANGE_STS);
				m_enumMap.emplace("AUTO_TRACKING_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::AUTO_TRACKING_STS);
				m_enumMap.emplace("LI_LASING_EXECUTION_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::LI_LASING_EXECUTION_STS);
				m_enumMap.emplace("LI_LASING_ENABLE_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::LI_LASING_ENABLE_STS);
				m_enumMap.emplace("LI_LASING_SELECTED_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::LI_LASING_SELECTED_STS);
				m_enumMap.emplace("LD_LASING_EXECUTION_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::LD_LASING_EXECUTION_STS);
				m_enumMap.emplace("LD_LASING_ENABLE_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::LD_LASING_ENABLE_STS);
				m_enumMap.emplace("LD_LASING_SELECTED_STS",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::LD_LASING_SELECTED_STS);
				m_enumMap.emplace("OPERATIONAL_ACTIVITIES_LOGIC_STATUS_DB_SIZE",OperationalActivitiesLogicDB::OperationalActivitiesLogic_StatusDBEnum::OPERATIONAL_ACTIVITIES_LOGIC_STATUS_DB_SIZE);
				m_enumMap.emplace("OPERATIONAL_ACTIVITIES_LOGIC_CMD_DB_SIZE",OperationalActivitiesLogicDB::OperationalActivitiesLogic_CmdDBEnum::OPERATIONAL_ACTIVITIES_LOGIC_CMD_DB_SIZE);
				m_enumMap.emplace("OPERATIONAL_ACTIVITY_LOGIC_ERROR_DB_SIZE",OperationalActivitiesLogicDB::OperationalActivityLogic_ErrorDBEnum::OPERATIONAL_ACTIVITY_LOGIC_ERROR_DB_SIZE);
				m_enumMap.emplace("RulesInputDBEnum",ProjectCommon::MonitorDBIndex::RulesInputDBEnum);
				m_enumMap.emplace("RulesOutputDBEnum",ProjectCommon::MonitorDBIndex::RulesOutputDBEnum);
				m_enumMap.emplace("RulesExistenceDBEnum",ProjectCommon::MonitorDBIndex::RulesExistenceDBEnum);
				m_enumMap.emplace("RulesEnabledDBEnum",ProjectCommon::MonitorDBIndex::RulesEnabledDBEnum);
				m_enumMap.emplace("RulesManagementDBEnum",ProjectCommon::MonitorDBIndex::RulesManagementDBEnum);
				m_enumMap.emplace("NumberOfDatabases",ProjectCommon::MonitorDBIndex::NumberOfDatabases);
				m_enumMap.emplace("NUM_OF_PROJECT_LOGS",ProjectCommon::ProjectLogID::NUM_OF_PROJECT_LOGS);
				m_enumMap.emplace("EMU_VERSION",ProjectCommonDbDefs::VersionsTypeEnum::EMU_VERSION);
				m_enumMap.emplace("NUM_OF_VERSION_TYPES",ProjectCommonDbDefs::VersionsTypeEnum::NUM_OF_VERSION_TYPES);
				m_enumMap.emplace("LDRC_NONE",ProjectCommonDbDefs::LdrcStationsEnum::LDRC_NONE);
				m_enumMap.emplace("LDRC_DEFAULT",ProjectCommonDbDefs::LdrcStationsEnum::LDRC_DEFAULT);
				m_enumMap.emplace("LDRC_EXTRA_SHARPNESS_FOR_DEFAULT",ProjectCommonDbDefs::LdrcStationsEnum::LDRC_EXTRA_SHARPNESS_FOR_DEFAULT);
				m_enumMap.emplace("LDRC_BAD_WEATHER",ProjectCommonDbDefs::LdrcStationsEnum::LDRC_BAD_WEATHER);
				m_enumMap.emplace("LDRC_BLOOMING",ProjectCommonDbDefs::LdrcStationsEnum::LDRC_BLOOMING);
				m_enumMap.emplace("LDRC_EXTRA_SHARPNESS_FOR_BLOOMING",ProjectCommonDbDefs::LdrcStationsEnum::LDRC_EXTRA_SHARPNESS_FOR_BLOOMING);
				m_enumMap.emplace("LONG_INTEGRATION",ProjectCommonDbDefs::NucIntegrationModeEnum::LONG_INTEGRATION);
				m_enumMap.emplace("MEDIUM_INTEGRATION",ProjectCommonDbDefs::NucIntegrationModeEnum::MEDIUM_INTEGRATION);
				m_enumMap.emplace("SHORT_INTEGRATION",ProjectCommonDbDefs::NucIntegrationModeEnum::SHORT_INTEGRATION);
				m_enumMap.emplace("CAMERA_DAY",ProjectCommonDbDefs::CamerasEnum::CAMERA_DAY);
				m_enumMap.emplace("CAMERA_NIGHT",ProjectCommonDbDefs::CamerasEnum::CAMERA_NIGHT);
				m_enumMap.emplace("NUM_OF_CAMERAS",ProjectCommonDbDefs::CamerasEnum::NUM_OF_CAMERAS);
				m_enumMap.emplace("ENSLAVEMENT_MODE_ERROR",ProjectCommonDbDefs::EnslavementModeEnum::ENSLAVEMENT_MODE_ERROR);
				m_enumMap.emplace("ENSLAVEMENT_MODE_NONE",ProjectCommonDbDefs::EnslavementModeEnum::ENSLAVEMENT_MODE_NONE);
				m_enumMap.emplace("ENSLAVEMENT_MODE_MECHANICAL",ProjectCommonDbDefs::EnslavementModeEnum::ENSLAVEMENT_MODE_MECHANICAL);
				m_enumMap.emplace("ENSLAVEMENT_MODE_GTS",ProjectCommonDbDefs::EnslavementModeEnum::ENSLAVEMENT_MODE_GTS);
				m_enumMap.emplace("ENSLAVEMENT_MODE_STG",ProjectCommonDbDefs::EnslavementModeEnum::ENSLAVEMENT_MODE_STG);
				m_enumMap.emplace("POINTER_TYPE_CONTINUOUS",ProjectCommonDbDefs::PointerTypeEnum::POINTER_TYPE_CONTINUOUS);
				m_enumMap.emplace("POINTER_TYPE_BLINKING",ProjectCommonDbDefs::PointerTypeEnum::POINTER_TYPE_BLINKING);
				m_enumMap.emplace("INPUT_SELECTION_NONE",ProjectCommonDbDefs::CameraSelectionEnum::INPUT_SELECTION_NONE);
				m_enumMap.emplace("INPUT_SELECTION_VIDEO",ProjectCommonDbDefs::CameraSelectionEnum::INPUT_SELECTION_VIDEO);
				m_enumMap.emplace("RANGE_ACCURACY_NONE",ProjectCommonDbDefs::RangeAccuracyEnum::RANGE_ACCURACY_NONE);
				m_enumMap.emplace("RANGE_ACCURACY_ESTIMATED",ProjectCommonDbDefs::RangeAccuracyEnum::RANGE_ACCURACY_ESTIMATED);
				m_enumMap.emplace("RANGE_ACCURACY_ACCURATE",ProjectCommonDbDefs::RangeAccuracyEnum::RANGE_ACCURACY_ACCURATE);
				m_enumMap.emplace("WORK_MODE_NONE",ProjectCommonDbDefs::WorkModesEnum::WORK_MODE_NONE);
				m_enumMap.emplace("WORK_MODE_MANUAL",ProjectCommonDbDefs::WorkModesEnum::WORK_MODE_MANUAL);
				m_enumMap.emplace("WORK_MODE_POWER",ProjectCommonDbDefs::WorkModesEnum::WORK_MODE_POWER);
				m_enumMap.emplace("WORK_MODE_STAB",ProjectCommonDbDefs::WorkModesEnum::WORK_MODE_STAB);
				m_enumMap.emplace("FIRE_MODE_UNINITIALIZED",ProjectCommonDbDefs::FireModeEnum::FIRE_MODE_UNINITIALIZED);
				m_enumMap.emplace("FIRE_MODE_SINGLE",ProjectCommonDbDefs::FireModeEnum::FIRE_MODE_SINGLE);
				m_enumMap.emplace("FIRE_MODE_BURST",ProjectCommonDbDefs::FireModeEnum::FIRE_MODE_BURST);
				m_enumMap.emplace("FIRE_MODE_AUTOMATIC",ProjectCommonDbDefs::FireModeEnum::FIRE_MODE_AUTOMATIC);
				m_enumMap.emplace("FIRE_MODE_ENUM_SIZE",ProjectCommonDbDefs::FireModeEnum::FIRE_MODE_ENUM_SIZE);
				m_enumMap.emplace("PROCESS_STATUS_UPDATE_NOT_IN_PROCESS",ProjectCommonDbDefs::ProcessStatusUpdatesEnum::PROCESS_STATUS_UPDATE_NOT_IN_PROCESS);
				m_enumMap.emplace("PROCESS_STATUS_UPDATE_IN_PROCESS",ProjectCommonDbDefs::ProcessStatusUpdatesEnum::PROCESS_STATUS_UPDATE_IN_PROCESS);
				m_enumMap.emplace("PROCESS_STATUS_UPDATE_END_OK",ProjectCommonDbDefs::ProcessStatusUpdatesEnum::PROCESS_STATUS_UPDATE_END_OK);
				m_enumMap.emplace("PROCESS_STATUS_UPDATE_END_FAIL",ProjectCommonDbDefs::ProcessStatusUpdatesEnum::PROCESS_STATUS_UPDATE_END_FAIL);
				m_enumMap.emplace("PROCESS_STATUS_UPDATE_ABORTED_ON_REQUEST",ProjectCommonDbDefs::ProcessStatusUpdatesEnum::PROCESS_STATUS_UPDATE_ABORTED_ON_REQUEST);
				m_enumMap.emplace("PROCESS_STATUS_UPDATE_ABORTED_NO_CONDITIONS",ProjectCommonDbDefs::ProcessStatusUpdatesEnum::PROCESS_STATUS_UPDATE_ABORTED_NO_CONDITIONS);
				m_enumMap.emplace("RECORDING_STATE_NONE",ProjectCommonDbDefs::RecordingStateEnum::RECORDING_STATE_NONE);
				m_enumMap.emplace("RECORDING_STATE_ON_ACTIVATED",ProjectCommonDbDefs::RecordingStateEnum::RECORDING_STATE_ON_ACTIVATED);
				m_enumMap.emplace("RECORDING_STATE_OFF_NOT_ACTIVATED",ProjectCommonDbDefs::RecordingStateEnum::RECORDING_STATE_OFF_NOT_ACTIVATED);
				m_enumMap.emplace("RECORDING_STATE_RECORDING_FAILED",ProjectCommonDbDefs::RecordingStateEnum::RECORDING_STATE_RECORDING_FAILED);
				m_enumMap.emplace("RECORDING_STATE_RECORDING_ENABLED",ProjectCommonDbDefs::RecordingStateEnum::RECORDING_STATE_RECORDING_ENABLED);
				m_enumMap.emplace("RECORDING_STATE_RECORDING_DISABLED",ProjectCommonDbDefs::RecordingStateEnum::RECORDING_STATE_RECORDING_DISABLED);
				m_enumMap.emplace("RECORDING_KIND_NONE",ProjectCommonDbDefs::RecordingKindEnum::RECORDING_KIND_NONE);
				m_enumMap.emplace("RECORDING_KIND_CONTINUOUS",ProjectCommonDbDefs::RecordingKindEnum::RECORDING_KIND_CONTINUOUS);
				m_enumMap.emplace("RECORDING_KIND_DISCONTINUOUS",ProjectCommonDbDefs::RecordingKindEnum::RECORDING_KIND_DISCONTINUOUS);
				m_enumMap.emplace("PLAYING_STATE_NONE",ProjectCommonDbDefs::PlayingStateEnum::PLAYING_STATE_NONE);
				m_enumMap.emplace("PLAYING_STATE_ON_ACTIVATED",ProjectCommonDbDefs::PlayingStateEnum::PLAYING_STATE_ON_ACTIVATED);
				m_enumMap.emplace("PLAYING_STATE_OFF_NOT_ACTIVATED",ProjectCommonDbDefs::PlayingStateEnum::PLAYING_STATE_OFF_NOT_ACTIVATED);
				m_enumMap.emplace("PLAYING_STATE_PLAYING_FAILED",ProjectCommonDbDefs::PlayingStateEnum::PLAYING_STATE_PLAYING_FAILED);
				m_enumMap.emplace("PLAYING_STATE_PLAYING_ENABLED",ProjectCommonDbDefs::PlayingStateEnum::PLAYING_STATE_PLAYING_ENABLED);
				m_enumMap.emplace("PLAYING_STATE_PLAYING_DISABLED",ProjectCommonDbDefs::PlayingStateEnum::PLAYING_STATE_PLAYING_DISABLED);
				m_enumMap.emplace("PLAYING_RATE_NONE",ProjectCommonDbDefs::PlayingRateEnum::PLAYING_RATE_NONE);
				m_enumMap.emplace("PLAYING_RATE_REGULAR",ProjectCommonDbDefs::PlayingRateEnum::PLAYING_RATE_REGULAR);
				m_enumMap.emplace("PLAYING_RATE_FAST",ProjectCommonDbDefs::PlayingRateEnum::PLAYING_RATE_FAST);
				m_enumMap.emplace("PLAYING_RATE_STOP",ProjectCommonDbDefs::PlayingRateEnum::PLAYING_RATE_STOP);
				m_enumMap.emplace("PLAYING_DIRECTION_NONE",ProjectCommonDbDefs::PlayingDirectionEnum::PLAYING_DIRECTION_NONE);
				m_enumMap.emplace("PLAYING_DIRECTION_FORWARD",ProjectCommonDbDefs::PlayingDirectionEnum::PLAYING_DIRECTION_FORWARD);
				m_enumMap.emplace("PLAYING_DIRECTION_BACKWARD",ProjectCommonDbDefs::PlayingDirectionEnum::PLAYING_DIRECTION_BACKWARD);
				m_enumMap.emplace("SYSTEM_NONE",ProjectCommonDbDefs::SelectedSystemEnum::SYSTEM_NONE);
				m_enumMap.emplace("SYSTEM_ORCWS",ProjectCommonDbDefs::SelectedSystemEnum::SYSTEM_ORCWS);
				m_enumMap.emplace("SYSTEM_OBSERVATION",ProjectCommonDbDefs::SelectedSystemEnum::SYSTEM_OBSERVATION);
				m_enumMap.emplace("BORE_SIGHT_TYPE_NONE",ProjectCommonDbDefs::BoreSightTypeEnum::BORE_SIGHT_TYPE_NONE);
				m_enumMap.emplace("BORE_SIGHT_TYPE_LI_DAY",ProjectCommonDbDefs::BoreSightTypeEnum::BORE_SIGHT_TYPE_LI_DAY);
				m_enumMap.emplace("BORE_SIGHT_TYPE_LD_DAY",ProjectCommonDbDefs::BoreSightTypeEnum::BORE_SIGHT_TYPE_LD_DAY);
				m_enumMap.emplace("BORE_SIGHT_TYPE_LI_THERMAL",ProjectCommonDbDefs::BoreSightTypeEnum::BORE_SIGHT_TYPE_LI_THERMAL);
				m_enumMap.emplace("BORE_SIGHT_TYPE_LD_THERMAL",ProjectCommonDbDefs::BoreSightTypeEnum::BORE_SIGHT_TYPE_LD_THERMAL);
				m_enumMap.emplace("BORE_SIGHT_TYPE_DAY_FOV_CALIB",ProjectCommonDbDefs::BoreSightTypeEnum::BORE_SIGHT_TYPE_DAY_FOV_CALIB);
				m_enumMap.emplace("BORE_SIGHT_TYPE_THERMAL_FOV_CALIB",ProjectCommonDbDefs::BoreSightTypeEnum::BORE_SIGHT_TYPE_THERMAL_FOV_CALIB);
				m_enumMap.emplace("POD_CALIB_NO_MOVEMENT",ProjectCommonDbDefs::BoreSightMovementEnum::POD_CALIB_NO_MOVEMENT);
				m_enumMap.emplace("POD_CALIB_MOVEMENT_STEP_UP",ProjectCommonDbDefs::BoreSightMovementEnum::POD_CALIB_MOVEMENT_STEP_UP);
				m_enumMap.emplace("POD_CALIB_MOVEMENT_STEP_DOWN",ProjectCommonDbDefs::BoreSightMovementEnum::POD_CALIB_MOVEMENT_STEP_DOWN);
				m_enumMap.emplace("POD_CALIB_MOVEMENT_STEP_LEFT",ProjectCommonDbDefs::BoreSightMovementEnum::POD_CALIB_MOVEMENT_STEP_LEFT);
				m_enumMap.emplace("POD_CALIB_MOVEMENT_STEP_RIGHT",ProjectCommonDbDefs::BoreSightMovementEnum::POD_CALIB_MOVEMENT_STEP_RIGHT);
				m_enumMap.emplace("POD_CALIB_MOVEMENT_CONT_UP",ProjectCommonDbDefs::BoreSightMovementEnum::POD_CALIB_MOVEMENT_CONT_UP);
				m_enumMap.emplace("POD_CALIB_MOVEMENT_CONT_DOWN",ProjectCommonDbDefs::BoreSightMovementEnum::POD_CALIB_MOVEMENT_CONT_DOWN);
				m_enumMap.emplace("POD_CALIB_MOVEMENT_CONT_LEFT",ProjectCommonDbDefs::BoreSightMovementEnum::POD_CALIB_MOVEMENT_CONT_LEFT);
				m_enumMap.emplace("POD_CALIB_MOVEMENT_CONT_RIGHT",ProjectCommonDbDefs::BoreSightMovementEnum::POD_CALIB_MOVEMENT_CONT_RIGHT);
				m_enumMap.emplace("LI_OPERATION_MODE_NONE",ProjectCommonDbDefs::IlluminatorOperationModeEnum::LI_OPERATION_MODE_NONE);
				m_enumMap.emplace("LI_OPERATION_MODE_PULESE",ProjectCommonDbDefs::IlluminatorOperationModeEnum::LI_OPERATION_MODE_PULESE);
				m_enumMap.emplace("LI_OPERATION_MODE_CONTINUOUS",ProjectCommonDbDefs::IlluminatorOperationModeEnum::LI_OPERATION_MODE_CONTINUOUS);
				m_enumMap.emplace("MAST_STATUS_NONE",ProjectCommonDbDefs::MastStatusEnum::MAST_STATUS_NONE);
				m_enumMap.emplace("MAST_STATUS_ERROR",ProjectCommonDbDefs::MastStatusEnum::MAST_STATUS_ERROR);
				m_enumMap.emplace("MAST_STATUS_OPEN_READY",ProjectCommonDbDefs::MastStatusEnum::MAST_STATUS_OPEN_READY);
				m_enumMap.emplace("MAST_STATUS_MOVING_OPEN",ProjectCommonDbDefs::MastStatusEnum::MAST_STATUS_MOVING_OPEN);
				m_enumMap.emplace("MAST_STATUS_MOVING_CLOSE",ProjectCommonDbDefs::MastStatusEnum::MAST_STATUS_MOVING_CLOSE);
				m_enumMap.emplace("MAST_STATUS_CLOSE_READY",ProjectCommonDbDefs::MastStatusEnum::MAST_STATUS_CLOSE_READY);
				m_enumMap.emplace("HOMING_NONE",ProjectCommonDbDefs::HomingStatusEnum::HOMING_NONE);
				m_enumMap.emplace("IN_HOMING_IN_PROCESS",ProjectCommonDbDefs::HomingStatusEnum::IN_HOMING_IN_PROCESS);
				m_enumMap.emplace("IN_HOMING_END_FAIL",ProjectCommonDbDefs::HomingStatusEnum::IN_HOMING_END_FAIL);
				m_enumMap.emplace("IN_HOMING_END_SUCCESS",ProjectCommonDbDefs::HomingStatusEnum::IN_HOMING_END_SUCCESS);
				m_enumMap.emplace("IN_HOMING_END_SUCCESS_FORCED",ProjectCommonDbDefs::HomingStatusEnum::IN_HOMING_END_SUCCESS_FORCED);
				m_enumMap.emplace("OUT_OF_HOMING",ProjectCommonDbDefs::HomingStatusEnum::OUT_OF_HOMING);
				m_enumMap.emplace("OUT_OF_HOMING_POWER_OFF",ProjectCommonDbDefs::HomingStatusEnum::OUT_OF_HOMING_POWER_OFF);
				m_enumMap.emplace("OUT_OF_HOMING_FORCED",ProjectCommonDbDefs::HomingStatusEnum::OUT_OF_HOMING_FORCED);
				m_enumMap.emplace("HOMING_NONE_CMD",ProjectCommonDbDefs::HomingCmdEnum::HOMING_NONE_CMD);
				m_enumMap.emplace("IN_HOMING_CMD",ProjectCommonDbDefs::HomingCmdEnum::IN_HOMING_CMD);
				m_enumMap.emplace("OUT_OF_HOMING_CMD",ProjectCommonDbDefs::HomingCmdEnum::OUT_OF_HOMING_CMD);
				m_enumMap.emplace("ScreenWidth",ProjectCommonDbDefs::ScreenSize::ScreenWidth);
				m_enumMap.emplace("ScreenHeight",ProjectCommonDbDefs::ScreenSize::ScreenHeight);
				m_enumMap.emplace("STATE_NONE",ProjectCommonDbDefs::SystemStatesEnum::STATE_NONE);
				m_enumMap.emplace("STATE_INITIALIZATION",ProjectCommonDbDefs::SystemStatesEnum::STATE_INITIALIZATION);
				m_enumMap.emplace("STATE_OPERATIONAL",ProjectCommonDbDefs::SystemStatesEnum::STATE_OPERATIONAL);
				m_enumMap.emplace("STATE_BORESIGHT",ProjectCommonDbDefs::SystemStatesEnum::STATE_BORESIGHT);
				m_enumMap.emplace("STATE_MAINTENANCE",ProjectCommonDbDefs::SystemStatesEnum::STATE_MAINTENANCE);
				m_enumMap.emplace("STATE_TECHNICIAN",ProjectCommonDbDefs::SystemStatesEnum::STATE_TECHNICIAN);
				m_enumMap.emplace("NUM_OF_SYSTEM_STATES",ProjectCommonDbDefs::SystemStatesEnum::NUM_OF_SYSTEM_STATES);
				m_enumMap.emplace("INV_MOVE_STATE_NONE",ProjectCommonDbDefs::InvMovementStatesEnum::INV_MOVE_STATE_NONE);
				m_enumMap.emplace("STATE_CHECK_ENABLED",ProjectCommonDbDefs::InvMovementStatesEnum::STATE_CHECK_ENABLED);
				m_enumMap.emplace("STATE_CHECK_DISABLED",ProjectCommonDbDefs::InvMovementStatesEnum::STATE_CHECK_DISABLED);
				m_enumMap.emplace("STATE_FAILURE",ProjectCommonDbDefs::InvMovementStatesEnum::STATE_FAILURE);
				m_enumMap.emplace("NUM_OF_INV_MOVEMENT_STATES",ProjectCommonDbDefs::InvMovementStatesEnum::NUM_OF_INV_MOVEMENT_STATES);
				m_enumMap.emplace("OPERATOR_NONE",ProjectCommonDbDefs::OperatorTypesEnum::OPERATOR_NONE);
				m_enumMap.emplace("OPERATOR_GUNNER",ProjectCommonDbDefs::OperatorTypesEnum::OPERATOR_GUNNER);
				m_enumMap.emplace("OPERATOR_COMMANDER",ProjectCommonDbDefs::OperatorTypesEnum::OPERATOR_COMMANDER);
				m_enumMap.emplace("NUM_OF_OPERATOR_TYPES",ProjectCommonDbDefs::OperatorTypesEnum::NUM_OF_OPERATOR_TYPES);
				m_enumMap.emplace("SYSTEM_UNARMED",ProjectCommonDbDefs::SystemArmEnum::SYSTEM_UNARMED);
				m_enumMap.emplace("SYSTEM_ARMING_ERROR",ProjectCommonDbDefs::SystemArmEnum::SYSTEM_ARMING_ERROR);
				m_enumMap.emplace("SYSTEM_ARMED",ProjectCommonDbDefs::SystemArmEnum::SYSTEM_ARMED);
				m_enumMap.emplace("SYSTEM_ARMED_NOT_READY",ProjectCommonDbDefs::SystemArmEnum::SYSTEM_ARMED_NOT_READY);
				m_enumMap.emplace("FOUR_WAY_STEP_NO_STEP",ProjectCommonDbDefs::FourWayStepEnum::FOUR_WAY_STEP_NO_STEP);
				m_enumMap.emplace("FOUR_WAY_STEP_UP",ProjectCommonDbDefs::FourWayStepEnum::FOUR_WAY_STEP_UP);
				m_enumMap.emplace("FOUR_WAY_STEP_DOWN",ProjectCommonDbDefs::FourWayStepEnum::FOUR_WAY_STEP_DOWN);
				m_enumMap.emplace("FOUR_WAY_STEP_LEFT",ProjectCommonDbDefs::FourWayStepEnum::FOUR_WAY_STEP_LEFT);
				m_enumMap.emplace("FOUR_WAY_STEP_RIGHT",ProjectCommonDbDefs::FourWayStepEnum::FOUR_WAY_STEP_RIGHT);
				m_enumMap.emplace("INTENSITY_NO_STEP",ProjectCommonDbDefs::IntensityEnum::INTENSITY_NO_STEP);
				m_enumMap.emplace("INTENSITY_INCREASE",ProjectCommonDbDefs::IntensityEnum::INTENSITY_INCREASE);
				m_enumMap.emplace("INTENSITY_DECREASE",ProjectCommonDbDefs::IntensityEnum::INTENSITY_DECREASE);
				m_enumMap.emplace("STOCK_UNINITIALIZED",ProjectCommonDbDefs::StockValueEnum::STOCK_UNINITIALIZED);
				m_enumMap.emplace("FULL_STOCK",ProjectCommonDbDefs::StockValueEnum::FULL_STOCK);
				m_enumMap.emplace("HALF_STOCK",ProjectCommonDbDefs::StockValueEnum::HALF_STOCK);
				m_enumMap.emplace("EMPTY_STOCK",ProjectCommonDbDefs::StockValueEnum::EMPTY_STOCK);
				m_enumMap.emplace("MANUAL_STOCK",ProjectCommonDbDefs::StockValueEnum::MANUAL_STOCK);
				m_enumMap.emplace("STOCK_VALUE_ENUM_SIZE",ProjectCommonDbDefs::StockValueEnum::STOCK_VALUE_ENUM_SIZE);
				m_enumMap.emplace("DRAGON_BASIC",ProjectCommonDbDefs::StationTypeEnum::DRAGON_BASIC);
				m_enumMap.emplace("DRAGON_EXTENDED",ProjectCommonDbDefs::StationTypeEnum::DRAGON_EXTENDED);
				m_enumMap.emplace("WEAPON_NONE",ProjectCommonDbDefs::SelectedWeaponEnum::WEAPON_NONE);
				m_enumMap.emplace("WEAPON_GUN05",ProjectCommonDbDefs::SelectedWeaponEnum::WEAPON_GUN05);
				m_enumMap.emplace("WEAPON_MAG",ProjectCommonDbDefs::SelectedWeaponEnum::WEAPON_MAG);
				m_enumMap.emplace("WEAPON_MAKLAR",ProjectCommonDbDefs::SelectedWeaponEnum::WEAPON_MAKLAR);
				m_enumMap.emplace("LRF_FLIP_NONE",ProjectCommonDbDefs::LrfFlipEnum::LRF_FLIP_NONE);
				m_enumMap.emplace("LRF_NO_FLIP",ProjectCommonDbDefs::LrfFlipEnum::LRF_NO_FLIP);
				m_enumMap.emplace("LRF_FLIP",ProjectCommonDbDefs::LrfFlipEnum::LRF_FLIP);
				m_enumMap.emplace("LRF_MIRROR_NONE",ProjectCommonDbDefs::LrfMirrorEnum::LRF_MIRROR_NONE);
				m_enumMap.emplace("LRF_NO_MIRROR",ProjectCommonDbDefs::LrfMirrorEnum::LRF_NO_MIRROR);
				m_enumMap.emplace("LRF_MIRROR",ProjectCommonDbDefs::LrfMirrorEnum::LRF_MIRROR);
				m_enumMap.emplace("LRF_OVERLAY_NONE",ProjectCommonDbDefs::LrfOverlayEnum::LRF_OVERLAY_NONE);
				m_enumMap.emplace("LRF_OVERLAY_OFF",ProjectCommonDbDefs::LrfOverlayEnum::LRF_OVERLAY_OFF);
				m_enumMap.emplace("LRF_OVERLAY_ON",ProjectCommonDbDefs::LrfOverlayEnum::LRF_OVERLAY_ON);
				m_enumMap.emplace("NO_TRANSITION",StateMachine_Scanning::Scanning_TransitionsEnum::NO_TRANSITION);
				m_enumMap.emplace("GO_TO_SCANNING",StateMachine_Scanning::Scanning_TransitionsEnum::GO_TO_SCANNING);
				m_enumMap.emplace("GO_TO_END_SUCCESS",StateMachine_Scanning::Scanning_TransitionsEnum::GO_TO_END_SUCCESS);
				m_enumMap.emplace("GO_TO_END_FAIL",StateMachine_Scanning::Scanning_TransitionsEnum::GO_TO_END_FAIL);
				m_enumMap.emplace("NO_TRANSITION",StateMachine_SystemStates::SystemStates_TransitionsEnum::NO_TRANSITION);
				m_enumMap.emplace("GO_TO_INITIALIZATION",StateMachine_SystemStates::SystemStates_TransitionsEnum::GO_TO_INITIALIZATION);
				m_enumMap.emplace("GO_TO_OPERATIONAL",StateMachine_SystemStates::SystemStates_TransitionsEnum::GO_TO_OPERATIONAL);
				m_enumMap.emplace("GO_TO_BORESIGHT",StateMachine_SystemStates::SystemStates_TransitionsEnum::GO_TO_BORESIGHT);
				m_enumMap.emplace("GO_TO_CALIBRATION",StateMachine_SystemStates::SystemStates_TransitionsEnum::GO_TO_CALIBRATION);
				m_enumMap.emplace("GO_TO_IBIT",StateMachine_SystemStates::SystemStates_TransitionsEnum::GO_TO_IBIT);
				m_enumMap.emplace("GO_TO_MAINTENANCE",StateMachine_SystemStates::SystemStates_TransitionsEnum::GO_TO_MAINTENANCE);
				m_enumMap.emplace("NUM_OF_SYSTEM_STATES_TRANSITION",StateMachine_SystemStates::SystemStates_TransitionsEnum::NUM_OF_SYSTEM_STATES_TRANSITION);
				m_enumMap.emplace("SYSTEM_INIT_TIMEOUT",StateMachine_SystemStates::SystemStates_StatesTimeouts::SYSTEM_INIT_TIMEOUT);
				m_enumMap.emplace("DB_NONE",UiDB::DbEnum::DB_NONE);
				m_enumMap.emplace("DB_EXECUTION",UiDB::DbEnum::DB_EXECUTION);
				m_enumMap.emplace("DB_COMMANDER",UiDB::DbEnum::DB_COMMANDER);
				m_enumMap.emplace("DB_GUNNER",UiDB::DbEnum::DB_GUNNER);
				m_enumMap.emplace("NUM_OF_DBS",UiDB::DbEnum::NUM_OF_DBS);
				m_enumMap.emplace("HANDLES_ANALOGS_CMD",UiDB::UserInputsDBEnum::HANDLES_ANALOGS_CMD);
				m_enumMap.emplace("HANDLES_PALM_CMD",UiDB::UserInputsDBEnum::HANDLES_PALM_CMD);
				m_enumMap.emplace("HANDLES_LASE_CMD",UiDB::UserInputsDBEnum::HANDLES_LASE_CMD);
				m_enumMap.emplace("HANDLES_FOV_CMD",UiDB::UserInputsDBEnum::HANDLES_FOV_CMD);
				m_enumMap.emplace("HANDLES_TRACK_CMD",UiDB::UserInputsDBEnum::HANDLES_TRACK_CMD);
				m_enumMap.emplace("PROCESS_BRING_TO_CMD",UiDB::UserInputsDBEnum::PROCESS_BRING_TO_CMD);
				m_enumMap.emplace("PROCESS_AUTOMATIC_TRACKING_CMD",UiDB::UserInputsDBEnum::PROCESS_AUTOMATIC_TRACKING_CMD);
				m_enumMap.emplace("PROCESS_SCANNING_CMD",UiDB::UserInputsDBEnum::PROCESS_SCANNING_CMD);
				m_enumMap.emplace("PROCESS_DRIFT_CANCELLATION_CMD",UiDB::UserInputsDBEnum::PROCESS_DRIFT_CANCELLATION_CMD);
				m_enumMap.emplace("PROCESS_VMD_CMD",UiDB::UserInputsDBEnum::PROCESS_VMD_CMD);
				m_enumMap.emplace("PROCESS_LI_TIMER_SEC_CMD",UiDB::UserInputsDBEnum::PROCESS_LI_TIMER_SEC_CMD);
				m_enumMap.emplace("PROCESS_MAINTENANCE_CMD",UiDB::UserInputsDBEnum::PROCESS_MAINTENANCE_CMD);
				m_enumMap.emplace("DESIGNATOR_SELECT_CMD",UiDB::UserInputsDBEnum::DESIGNATOR_SELECT_CMD);
				m_enumMap.emplace("DESIGNATOR_ARM_CMD",UiDB::UserInputsDBEnum::DESIGNATOR_ARM_CMD);
				m_enumMap.emplace("DESIGNATOR_LASE_CMD",UiDB::UserInputsDBEnum::DESIGNATOR_LASE_CMD);
				m_enumMap.emplace("DESIGNATOR_CODE_CMD",UiDB::UserInputsDBEnum::DESIGNATOR_CODE_CMD);
				m_enumMap.emplace("ILLUMINATOR_SELECT_CMD",UiDB::UserInputsDBEnum::ILLUMINATOR_SELECT_CMD);
				m_enumMap.emplace("ILLUMINATOR_ARM_CMD",UiDB::UserInputsDBEnum::ILLUMINATOR_ARM_CMD);
				m_enumMap.emplace("ILLUMINATOR_LASE_CMD",UiDB::UserInputsDBEnum::ILLUMINATOR_LASE_CMD);
				m_enumMap.emplace("ILLUMINATOR_OPERATION_MODE_CMD",UiDB::UserInputsDBEnum::ILLUMINATOR_OPERATION_MODE_CMD);
				m_enumMap.emplace("ILLUMINATOR_MAX_LASING_TIME_CMD",UiDB::UserInputsDBEnum::ILLUMINATOR_MAX_LASING_TIME_CMD);
				m_enumMap.emplace("LD_LI_LASING_PASSWORD_RECEIVED_CMD",UiDB::UserInputsDBEnum::LD_LI_LASING_PASSWORD_RECEIVED_CMD);
				m_enumMap.emplace("LRF_MIN_MAX_RANGE_CMD",UiDB::UserInputsDBEnum::LRF_MIN_MAX_RANGE_CMD);
				m_enumMap.emplace("LRF_RETICLE_GRAY_LEVEL_CMD",UiDB::UserInputsDBEnum::LRF_RETICLE_GRAY_LEVEL_CMD);
				m_enumMap.emplace("LRF_RETICLE_ON_CMD",UiDB::UserInputsDBEnum::LRF_RETICLE_ON_CMD);
				m_enumMap.emplace("LRF_ZOOM_CMD",UiDB::UserInputsDBEnum::LRF_ZOOM_CMD);
				m_enumMap.emplace("LRF_POWER_CMD",UiDB::UserInputsDBEnum::LRF_POWER_CMD);
				m_enumMap.emplace("LRF_ROTATE_FLIP_CMD",UiDB::UserInputsDBEnum::LRF_ROTATE_FLIP_CMD);
				m_enumMap.emplace("LRF_ROTATE_MIRROR_CMD",UiDB::UserInputsDBEnum::LRF_ROTATE_MIRROR_CMD);
				m_enumMap.emplace("LRF_OVERLAY_SHOW_CMD",UiDB::UserInputsDBEnum::LRF_OVERLAY_SHOW_CMD);
				m_enumMap.emplace("LRF_COLOR_GAIN_CMD",UiDB::UserInputsDBEnum::LRF_COLOR_GAIN_CMD);
				m_enumMap.emplace("LRF_FIRST_LAST_RANGE_CMD",UiDB::UserInputsDBEnum::LRF_FIRST_LAST_RANGE_CMD);
				m_enumMap.emplace("MANUAL_RANGE_CMD",UiDB::UserInputsDBEnum::MANUAL_RANGE_CMD);
				m_enumMap.emplace("CAMERA_SELECT_CMD",UiDB::UserInputsDBEnum::CAMERA_SELECT_CMD);
				m_enumMap.emplace("CCD_FOCUS_CMD",UiDB::UserInputsDBEnum::CCD_FOCUS_CMD);
				m_enumMap.emplace("CCD_FOV_CMD",UiDB::UserInputsDBEnum::CCD_FOV_CMD);
				m_enumMap.emplace("CCD_ZOOM_CMD",UiDB::UserInputsDBEnum::CCD_ZOOM_CMD);
				m_enumMap.emplace("CCD_IRIS_CMD",UiDB::UserInputsDBEnum::CCD_IRIS_CMD);
				m_enumMap.emplace("CCD_BRIGHTNESS_CMD",UiDB::UserInputsDBEnum::CCD_BRIGHTNESS_CMD);
				m_enumMap.emplace("CCD_CAMERA_COLOR_CMD",UiDB::UserInputsDBEnum::CCD_CAMERA_COLOR_CMD);
				m_enumMap.emplace("CCD_IMAGE_ENHANCEMENT_CMD",UiDB::UserInputsDBEnum::CCD_IMAGE_ENHANCEMENT_CMD);
				m_enumMap.emplace("CCD_DRC_CMD",UiDB::UserInputsDBEnum::CCD_DRC_CMD);
				m_enumMap.emplace("CCD_FREEZE_CMD",UiDB::UserInputsDBEnum::CCD_FREEZE_CMD);
				m_enumMap.emplace("CCD_ALPHA_NUMERIC_DISPLAY_CMD",UiDB::UserInputsDBEnum::CCD_ALPHA_NUMERIC_DISPLAY_CMD);
				m_enumMap.emplace("CCD_GAMMA_CMD",UiDB::UserInputsDBEnum::CCD_GAMMA_CMD);
				m_enumMap.emplace("CCD_GAIN_CMD",UiDB::UserInputsDBEnum::CCD_GAIN_CMD);
				m_enumMap.emplace("CCD_LOW_LIGHT_CMD",UiDB::UserInputsDBEnum::CCD_LOW_LIGHT_CMD);
				m_enumMap.emplace("TI_FOCUS_CMD",UiDB::UserInputsDBEnum::TI_FOCUS_CMD);
				m_enumMap.emplace("TI_FOV_CMD",UiDB::UserInputsDBEnum::TI_FOV_CMD);
				m_enumMap.emplace("TI_ZOOM_CMD",UiDB::UserInputsDBEnum::TI_ZOOM_CMD);
				m_enumMap.emplace("TI_EZOOM_CMD",UiDB::UserInputsDBEnum::TI_EZOOM_CMD);
				m_enumMap.emplace("TI_POLARITY_CMD",UiDB::UserInputsDBEnum::TI_POLARITY_CMD);
				m_enumMap.emplace("TI_SYMBOLS_DISPLAY_CMD",UiDB::UserInputsDBEnum::TI_SYMBOLS_DISPLAY_CMD);
				m_enumMap.emplace("TI_SYNTHETIC_IMAGE_CMD",UiDB::UserInputsDBEnum::TI_SYNTHETIC_IMAGE_CMD);
				m_enumMap.emplace("TI_GAMMA_CMD",UiDB::UserInputsDBEnum::TI_GAMMA_CMD);
				m_enumMap.emplace("TI_LEVEL_CMD",UiDB::UserInputsDBEnum::TI_LEVEL_CMD);
				m_enumMap.emplace("TI_GAIN_CMD",UiDB::UserInputsDBEnum::TI_GAIN_CMD);
				m_enumMap.emplace("TI_DRC_CMD",UiDB::UserInputsDBEnum::TI_DRC_CMD);
				m_enumMap.emplace("TI_NUC_CMD",UiDB::UserInputsDBEnum::TI_NUC_CMD);
				m_enumMap.emplace("TI_FREEZE_CMD",UiDB::UserInputsDBEnum::TI_FREEZE_CMD);
				m_enumMap.emplace("TI_LDRC_CMD",UiDB::UserInputsDBEnum::TI_LDRC_CMD);
				m_enumMap.emplace("TI_INTEGRATION_TIME_STATE_CMD",UiDB::UserInputsDBEnum::TI_INTEGRATION_TIME_STATE_CMD);
				m_enumMap.emplace("TI_INTEGRATION_TIME_TYPE_CMD",UiDB::UserInputsDBEnum::TI_INTEGRATION_TIME_TYPE_CMD);
				m_enumMap.emplace("GUNNER_CONFIRM_ACTIVATING_OBSRV_HW_CMD",UiDB::UserInputsDBEnum::GUNNER_CONFIRM_ACTIVATING_OBSRV_HW_CMD);
				m_enumMap.emplace("BORESIGHT_INC_DEC_REQ_CMD",UiDB::UserInputsDBEnum::BORESIGHT_INC_DEC_REQ_CMD);
				m_enumMap.emplace("BORESIGHT_ENTER_PRESSED_CMD",UiDB::UserInputsDBEnum::BORESIGHT_ENTER_PRESSED_CMD);
				m_enumMap.emplace("BORESIGHT_CANCEL_PRESSED_CMD",UiDB::UserInputsDBEnum::BORESIGHT_CANCEL_PRESSED_CMD);
				m_enumMap.emplace("BORESIGHT_RESET_REQ_CMD",UiDB::UserInputsDBEnum::BORESIGHT_RESET_REQ_CMD);
				m_enumMap.emplace("BORESIGHT_LI_DAY_PROCESS_REQ_CMD",UiDB::UserInputsDBEnum::BORESIGHT_LI_DAY_PROCESS_REQ_CMD);
				m_enumMap.emplace("BORESIGHT_LD_DAY_PROCESS_REQ_CMD",UiDB::UserInputsDBEnum::BORESIGHT_LD_DAY_PROCESS_REQ_CMD);
				m_enumMap.emplace("BORESIGHT_LI_THERMAL_PROCESS_REQ_CMD",UiDB::UserInputsDBEnum::BORESIGHT_LI_THERMAL_PROCESS_REQ_CMD);
				m_enumMap.emplace("BORESIGHT_LD_THERMAL_PROCESS_REQ_CMD",UiDB::UserInputsDBEnum::BORESIGHT_LD_THERMAL_PROCESS_REQ_CMD);
				m_enumMap.emplace("FOV_CALIB_DAY_PROCESS_REQ_CMD",UiDB::UserInputsDBEnum::FOV_CALIB_DAY_PROCESS_REQ_CMD);
				m_enumMap.emplace("FOV_CALIB_THERMAL_PROCESS_REQ_CMD",UiDB::UserInputsDBEnum::FOV_CALIB_THERMAL_PROCESS_REQ_CMD);
				m_enumMap.emplace("BORESIGHT_BACK_PRESSED_CMD",UiDB::UserInputsDBEnum::BORESIGHT_BACK_PRESSED_CMD);
				m_enumMap.emplace("WORK_MODE_REQ_CMD",UiDB::UserInputsDBEnum::WORK_MODE_REQ_CMD);
				m_enumMap.emplace("INS_MODE_TRANSITION_REQUEST_CMD",UiDB::UserInputsDBEnum::INS_MODE_TRANSITION_REQUEST_CMD);
				m_enumMap.emplace("TO_HOME_REQUEST_STS",UiDB::UserInputsDBEnum::TO_HOME_REQUEST_STS);
				m_enumMap.emplace("OUT_OF_HOME_REQUEST_STS",UiDB::UserInputsDBEnum::OUT_OF_HOME_REQUEST_STS);
				m_enumMap.emplace("RETICLE_ON_OFF_CMD",UiDB::UserInputsDBEnum::RETICLE_ON_OFF_CMD);
				m_enumMap.emplace("RETICLE_COLOR_CMD",UiDB::UserInputsDBEnum::RETICLE_COLOR_CMD);
				m_enumMap.emplace("HOMING_CALIB_REQUEST_CMD",UiDB::UserInputsDBEnum::HOMING_CALIB_REQUEST_CMD);
				m_enumMap.emplace("FUSION_PERCENT_DAY_VS_TI_REQUEST_CMD",UiDB::UserInputsDBEnum::FUSION_PERCENT_DAY_VS_TI_REQUEST_CMD);
				m_enumMap.emplace("FUSION_HOT_PARTS_COLOR_CMD",UiDB::UserInputsDBEnum::FUSION_HOT_PARTS_COLOR_CMD);
				m_enumMap.emplace("SLA_SHARPNESS_REQUEST_CMD",UiDB::UserInputsDBEnum::SLA_SHARPNESS_REQUEST_CMD);
				m_enumMap.emplace("SLA_BRIGHTNESS_REQUEST_CMD",UiDB::UserInputsDBEnum::SLA_BRIGHTNESS_REQUEST_CMD);
				m_enumMap.emplace("SLA_CONTRAST_REQUEST_CMD",UiDB::UserInputsDBEnum::SLA_CONTRAST_REQUEST_CMD);
				m_enumMap.emplace("SLA_SATURATION_REQUEST_CMD",UiDB::UserInputsDBEnum::SLA_SATURATION_REQUEST_CMD);
				m_enumMap.emplace("SLA_RESET_REQUEST_CMD",UiDB::UserInputsDBEnum::SLA_RESET_REQUEST_CMD);
				m_enumMap.emplace("SLA_BLEND_PARAMS_REQUEST_CMD",UiDB::UserInputsDBEnum::SLA_BLEND_PARAMS_REQUEST_CMD);
				m_enumMap.emplace("USER_INPUTS_DB_SIZE",UiDB::UserInputsDBEnum::USER_INPUTS_DB_SIZE);
				m_enumMap.emplace("PERMISSION_HANDLES_ANALOGS_CMD",UiDB::PermissionDBEnum::PERMISSION_HANDLES_ANALOGS_CMD);
				m_enumMap.emplace("PERMISSION_HANDLES_PALM_CMD",UiDB::PermissionDBEnum::PERMISSION_HANDLES_PALM_CMD);
				m_enumMap.emplace("PERMISSION_HANDLES_LASE_CMD",UiDB::PermissionDBEnum::PERMISSION_HANDLES_LASE_CMD);
				m_enumMap.emplace("PERMISSION_HANDLES_FOV_CMD",UiDB::PermissionDBEnum::PERMISSION_HANDLES_FOV_CMD);
				m_enumMap.emplace("PERMISSION_HANDLES_TRACK_CMD",UiDB::PermissionDBEnum::PERMISSION_HANDLES_TRACK_CMD);
				m_enumMap.emplace("PERMISSION_PROCESS_BRING_TO_CMD",UiDB::PermissionDBEnum::PERMISSION_PROCESS_BRING_TO_CMD);
				m_enumMap.emplace("PERMISSION_PROCESS_AUTOMATIC_TRACKING_CMD",UiDB::PermissionDBEnum::PERMISSION_PROCESS_AUTOMATIC_TRACKING_CMD);
				m_enumMap.emplace("PERMISSION_PROCESS_SCANNING_CMD",UiDB::PermissionDBEnum::PERMISSION_PROCESS_SCANNING_CMD);
				m_enumMap.emplace("PERMISSION_PROCESS_DRIFT_CANCELLATION_CMD",UiDB::PermissionDBEnum::PERMISSION_PROCESS_DRIFT_CANCELLATION_CMD);
				m_enumMap.emplace("PERMISSION_PROCESS_VMD_CMD",UiDB::PermissionDBEnum::PERMISSION_PROCESS_VMD_CMD);
				m_enumMap.emplace("PERMISSION_PROCESS_LI_TIMER_SEC_CMD",UiDB::PermissionDBEnum::PERMISSION_PROCESS_LI_TIMER_SEC_CMD);
				m_enumMap.emplace("PERMISSION_PROCESS_MAINTENANCE_CMD",UiDB::PermissionDBEnum::PERMISSION_PROCESS_MAINTENANCE_CMD);
				m_enumMap.emplace("PERMISSION_DESIGNATOR_SELECT_CMD",UiDB::PermissionDBEnum::PERMISSION_DESIGNATOR_SELECT_CMD);
				m_enumMap.emplace("PERMISSION_DESIGNATOR_ARM_CMD",UiDB::PermissionDBEnum::PERMISSION_DESIGNATOR_ARM_CMD);
				m_enumMap.emplace("PERMISSION_DESIGNATOR_LASE_CMD",UiDB::PermissionDBEnum::PERMISSION_DESIGNATOR_LASE_CMD);
				m_enumMap.emplace("PERMISSION_DESIGNATOR_CODE",UiDB::PermissionDBEnum::PERMISSION_DESIGNATOR_CODE);
				m_enumMap.emplace("PERMISSION_ILLUMINATOR_SELECT_CMD",UiDB::PermissionDBEnum::PERMISSION_ILLUMINATOR_SELECT_CMD);
				m_enumMap.emplace("PERMISSION_ILLUMINATOR_ARM_CMD",UiDB::PermissionDBEnum::PERMISSION_ILLUMINATOR_ARM_CMD);
				m_enumMap.emplace("PERMISSION_ILLUMINATOR_LASE_CMD",UiDB::PermissionDBEnum::PERMISSION_ILLUMINATOR_LASE_CMD);
				m_enumMap.emplace("PERMISSION_ILLUMINATOR_OPERATION_MODE_CMD",UiDB::PermissionDBEnum::PERMISSION_ILLUMINATOR_OPERATION_MODE_CMD);
				m_enumMap.emplace("PERMISSION_ILLUMINATOR_MAX_LASING_TIME_CMD",UiDB::PermissionDBEnum::PERMISSION_ILLUMINATOR_MAX_LASING_TIME_CMD);
				m_enumMap.emplace("PERMISSION_LD_LI_LASING_PASSWORD_RECEIVED_CMD",UiDB::PermissionDBEnum::PERMISSION_LD_LI_LASING_PASSWORD_RECEIVED_CMD);
				m_enumMap.emplace("PERMISSION_LRF_MIN_MAX_RANGE_CMD",UiDB::PermissionDBEnum::PERMISSION_LRF_MIN_MAX_RANGE_CMD);
				m_enumMap.emplace("PERMISSION_LRF_RETICLE_GRAY_LEVEL_CMD",UiDB::PermissionDBEnum::PERMISSION_LRF_RETICLE_GRAY_LEVEL_CMD);
				m_enumMap.emplace("PERMISSION_LRF_RETICLE_ON_CMD",UiDB::PermissionDBEnum::PERMISSION_LRF_RETICLE_ON_CMD);
				m_enumMap.emplace("PERMISSION_LRF_ZOOM_CMD",UiDB::PermissionDBEnum::PERMISSION_LRF_ZOOM_CMD);
				m_enumMap.emplace("PERMISSION_LRF_POWER_CMD",UiDB::PermissionDBEnum::PERMISSION_LRF_POWER_CMD);
				m_enumMap.emplace("PERMISSION_LRF_ROTATE_FLIP_CMD",UiDB::PermissionDBEnum::PERMISSION_LRF_ROTATE_FLIP_CMD);
				m_enumMap.emplace("PERMISSION_LRF_ROTATE_MIRROR_CMD",UiDB::PermissionDBEnum::PERMISSION_LRF_ROTATE_MIRROR_CMD);
				m_enumMap.emplace("PERMISSION_LRF_OVERLAY_SHOW_CMD",UiDB::PermissionDBEnum::PERMISSION_LRF_OVERLAY_SHOW_CMD);
				m_enumMap.emplace("PERMISSION_LRF_COLOR_GAIN_CMD",UiDB::PermissionDBEnum::PERMISSION_LRF_COLOR_GAIN_CMD);
				m_enumMap.emplace("PERMISSION_LRF_FIRST_LAST_RANGE_CMD",UiDB::PermissionDBEnum::PERMISSION_LRF_FIRST_LAST_RANGE_CMD);
				m_enumMap.emplace("PERMISSION_MANUAL_RANGE_CMD",UiDB::PermissionDBEnum::PERMISSION_MANUAL_RANGE_CMD);
				m_enumMap.emplace("PERMISSION_CAMERA_SELECT_CMD",UiDB::PermissionDBEnum::PERMISSION_CAMERA_SELECT_CMD);
				m_enumMap.emplace("PERMISSION_CCD_FOCUS_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_FOCUS_CMD);
				m_enumMap.emplace("PERMISSION_CCD_FOV_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_FOV_CMD);
				m_enumMap.emplace("PERMISSION_CCD_ZOOM_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_ZOOM_CMD);
				m_enumMap.emplace("PERMISSION_CCD_IRIS_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_IRIS_CMD);
				m_enumMap.emplace("PERMISSION_CCD_BRIGHTNESS_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_BRIGHTNESS_CMD);
				m_enumMap.emplace("PERMISSION_CCD_CAMERA_COLOR_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_CAMERA_COLOR_CMD);
				m_enumMap.emplace("PERMISSION_CCD_IMAGE_ENHANCEMENT_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_IMAGE_ENHANCEMENT_CMD);
				m_enumMap.emplace("PERMISSION_CCD_DRC_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_DRC_CMD);
				m_enumMap.emplace("PERMISSION_CCD_FREEZE_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_FREEZE_CMD);
				m_enumMap.emplace("PERMISSION_CCD_ALPHA_NUMERIC_DISPLAY_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_ALPHA_NUMERIC_DISPLAY_CMD);
				m_enumMap.emplace("PERMISSION_CCD_GAMMA_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_GAMMA_CMD);
				m_enumMap.emplace("PERMISSION_CCD_GAIN_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_GAIN_CMD);
				m_enumMap.emplace("PERMISSION_CCD_LOW_LIGHT_CMD",UiDB::PermissionDBEnum::PERMISSION_CCD_LOW_LIGHT_CMD);
				m_enumMap.emplace("PERMISSION_TI_FOCUS_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_FOCUS_CMD);
				m_enumMap.emplace("PERMISSION_TI_FOV_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_FOV_CMD);
				m_enumMap.emplace("PERMISSION_TI_ZOOM_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_ZOOM_CMD);
				m_enumMap.emplace("PERMISSION_TI_POLARITY_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_POLARITY_CMD);
				m_enumMap.emplace("PERMISSION_TI_SYMBOLS_DISPLAY_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_SYMBOLS_DISPLAY_CMD);
				m_enumMap.emplace("PERMISSION_TI_SYNTHETIC_IMAGE_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_SYNTHETIC_IMAGE_CMD);
				m_enumMap.emplace("PERMISSION_TI_GAMMA_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_GAMMA_CMD);
				m_enumMap.emplace("PERMISSION_TI_LEVEL_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_LEVEL_CMD);
				m_enumMap.emplace("PERMISSION_TI_GAIN_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_GAIN_CMD);
				m_enumMap.emplace("PERMISSION_TI_DRC_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_DRC_CMD);
				m_enumMap.emplace("PERMISSION_TI_NUC_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_NUC_CMD);
				m_enumMap.emplace("PERMISSION_TI_FREEZE_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_FREEZE_CMD);
				m_enumMap.emplace("PERMISSION_TI_LDRC_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_LDRC_CMD);
				m_enumMap.emplace("PERMISSION_TI_EZOOM_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_EZOOM_CMD);
				m_enumMap.emplace("PERMISSION_TI_INTEGRATION_TIME_MODE_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_INTEGRATION_TIME_MODE_CMD);
				m_enumMap.emplace("PERMISSION_TI_INTEGRATION_TIME_TYPE_CMD",UiDB::PermissionDBEnum::PERMISSION_TI_INTEGRATION_TIME_TYPE_CMD);
				m_enumMap.emplace("PERMISSION_GUNNER_CONFIRM_ACTIVATING_OBSRV_HW_CMD",UiDB::PermissionDBEnum::PERMISSION_GUNNER_CONFIRM_ACTIVATING_OBSRV_HW_CMD);
				m_enumMap.emplace("PERMISSION_BORESIGHT_INC_DEC_REQ_CMD",UiDB::PermissionDBEnum::PERMISSION_BORESIGHT_INC_DEC_REQ_CMD);
				m_enumMap.emplace("PERMISSION_BORESIGHT_ENTER_PRESSED_CMD",UiDB::PermissionDBEnum::PERMISSION_BORESIGHT_ENTER_PRESSED_CMD);
				m_enumMap.emplace("PERMISSION_BORESIGHT_CANCEL_PRESSED_CMD",UiDB::PermissionDBEnum::PERMISSION_BORESIGHT_CANCEL_PRESSED_CMD);
				m_enumMap.emplace("PERMISSION_BORESIGHT_RESET_REQ_CMD",UiDB::PermissionDBEnum::PERMISSION_BORESIGHT_RESET_REQ_CMD);
				m_enumMap.emplace("PERMISSION_BORESIGHT_LI_DAY_PROCESS_REQ_CMD",UiDB::PermissionDBEnum::PERMISSION_BORESIGHT_LI_DAY_PROCESS_REQ_CMD);
				m_enumMap.emplace("PERMISSION_BORESIGHT_LD_DAY_PROCESS_REQ_CMD",UiDB::PermissionDBEnum::PERMISSION_BORESIGHT_LD_DAY_PROCESS_REQ_CMD);
				m_enumMap.emplace("PERMISSION_BORESIGHT_LI_THERMAL_PROCESS_REQ_CMD",UiDB::PermissionDBEnum::PERMISSION_BORESIGHT_LI_THERMAL_PROCESS_REQ_CMD);
				m_enumMap.emplace("PERMISSION_BORESIGHT_LD_THERMALPROCESS_REQ_CMD",UiDB::PermissionDBEnum::PERMISSION_BORESIGHT_LD_THERMALPROCESS_REQ_CMD);
				m_enumMap.emplace("PERMISSION_FOV_CALIB_DAY_PROCESS_REQ_CMD",UiDB::PermissionDBEnum::PERMISSION_FOV_CALIB_DAY_PROCESS_REQ_CMD);
				m_enumMap.emplace("PERMISSION_FOV_CALIB_THERMAL_PROCESS_REQ_CMD",UiDB::PermissionDBEnum::PERMISSION_FOV_CALIB_THERMAL_PROCESS_REQ_CMD);
				m_enumMap.emplace("PERMISSION_WORK_MODE_REQ_CMD",UiDB::PermissionDBEnum::PERMISSION_WORK_MODE_REQ_CMD);
				m_enumMap.emplace("PERMISSION_INS_MODE_TRANSITION_REQUEST_CMD",UiDB::PermissionDBEnum::PERMISSION_INS_MODE_TRANSITION_REQUEST_CMD);
				m_enumMap.emplace("PERMISSION_TO_HOME_REQUEST_STS",UiDB::PermissionDBEnum::PERMISSION_TO_HOME_REQUEST_STS);
				m_enumMap.emplace("PERMISSION_OUT_OF_HOME_REQUEST_STS",UiDB::PermissionDBEnum::PERMISSION_OUT_OF_HOME_REQUEST_STS);
				m_enumMap.emplace("PERMISSION_RETICLE_ON_OFF_CMD",UiDB::PermissionDBEnum::PERMISSION_RETICLE_ON_OFF_CMD);
				m_enumMap.emplace("PERMISSION_RETICLE_COLOR_CMD",UiDB::PermissionDBEnum::PERMISSION_RETICLE_COLOR_CMD);
				m_enumMap.emplace("PERMISSION_HOMING_CALIB_REQUEST_CMD",UiDB::PermissionDBEnum::PERMISSION_HOMING_CALIB_REQUEST_CMD);
				m_enumMap.emplace("PERMISSION_FUSION_PERCENT_DAY_VS_TI_REQUEST_CMD",UiDB::PermissionDBEnum::PERMISSION_FUSION_PERCENT_DAY_VS_TI_REQUEST_CMD);
				m_enumMap.emplace("PERMISSION_FUSION_HOT_PARTS_COLOR_CMD",UiDB::PermissionDBEnum::PERMISSION_FUSION_HOT_PARTS_COLOR_CMD);
				m_enumMap.emplace("PERMISSION_SLA_SHARPNESS_REQUEST_CMD",UiDB::PermissionDBEnum::PERMISSION_SLA_SHARPNESS_REQUEST_CMD);
				m_enumMap.emplace("PERMISSION_SLA_BRIGHTNESS_REQUEST_CMD",UiDB::PermissionDBEnum::PERMISSION_SLA_BRIGHTNESS_REQUEST_CMD);
				m_enumMap.emplace("PERMISSION_SLA_CONTRAST_REQUEST_CMD",UiDB::PermissionDBEnum::PERMISSION_SLA_CONTRAST_REQUEST_CMD);
				m_enumMap.emplace("PERMISSION_SLA_SATURATION_REQUEST_CMD",UiDB::PermissionDBEnum::PERMISSION_SLA_SATURATION_REQUEST_CMD);
				m_enumMap.emplace("PERMISSION_SLA_RESET_REQUEST_CMD",UiDB::PermissionDBEnum::PERMISSION_SLA_RESET_REQUEST_CMD);
				m_enumMap.emplace("PERMISSION_SLA_BLEND_REQUEST_CMD",UiDB::PermissionDBEnum::PERMISSION_SLA_BLEND_REQUEST_CMD);
				m_enumMap.emplace("PERMISSION_DB_SIZE",UiDB::PermissionDBEnum::PERMISSION_DB_SIZE);
				m_enumMap.emplace("UNIT_ON_OFF_ILLUMINATOR_CMD",UiDB::UnitsOnOffDBEnum::UNIT_ON_OFF_ILLUMINATOR_CMD);
				m_enumMap.emplace("UNIT_ON_OFF_DESIGNATOR_CMD",UiDB::UnitsOnOffDBEnum::UNIT_ON_OFF_DESIGNATOR_CMD);
				m_enumMap.emplace("UNIT_ON_OFF_POD_CMD",UiDB::UnitsOnOffDBEnum::UNIT_ON_OFF_POD_CMD);
				m_enumMap.emplace("UNIT_ON_OFF_LRF_CMD",UiDB::UnitsOnOffDBEnum::UNIT_ON_OFF_LRF_CMD);
				m_enumMap.emplace("UNIT_ON_OFF_DAY_CAM_CMD",UiDB::UnitsOnOffDBEnum::UNIT_ON_OFF_DAY_CAM_CMD);
				m_enumMap.emplace("UNIT_ON_OFF_TI_CAM_CMD",UiDB::UnitsOnOffDBEnum::UNIT_ON_OFF_TI_CAM_CMD);
				m_enumMap.emplace("UNIT_ON_OFF_LWS_CMD",UiDB::UnitsOnOffDBEnum::UNIT_ON_OFF_LWS_CMD);
				m_enumMap.emplace("UNITS_ON_OFF_DB_SIZE",UiDB::UnitsOnOffDBEnum::UNITS_ON_OFF_DB_SIZE);
				m_enumMap.emplace("BUTTON_0_STS",UiDB::ButtonsStsDBEnum::BUTTON_0_STS);
				m_enumMap.emplace("BUTTON_1_STS",UiDB::ButtonsStsDBEnum::BUTTON_1_STS);
				m_enumMap.emplace("BUTTON_2_STS",UiDB::ButtonsStsDBEnum::BUTTON_2_STS);
				m_enumMap.emplace("BUTTON_3_STS",UiDB::ButtonsStsDBEnum::BUTTON_3_STS);
				m_enumMap.emplace("BUTTON_4_STS",UiDB::ButtonsStsDBEnum::BUTTON_4_STS);
				m_enumMap.emplace("BUTTON_5_STS",UiDB::ButtonsStsDBEnum::BUTTON_5_STS);
				m_enumMap.emplace("BUTTON_6_STS",UiDB::ButtonsStsDBEnum::BUTTON_6_STS);
				m_enumMap.emplace("BUTTON_7_STS",UiDB::ButtonsStsDBEnum::BUTTON_7_STS);
				m_enumMap.emplace("BUTTON_8_STS",UiDB::ButtonsStsDBEnum::BUTTON_8_STS);
				m_enumMap.emplace("BUTTON_9_STS",UiDB::ButtonsStsDBEnum::BUTTON_9_STS);
				m_enumMap.emplace("BUTTON_10_STS",UiDB::ButtonsStsDBEnum::BUTTON_10_STS);
				m_enumMap.emplace("BUTTON_11_STS",UiDB::ButtonsStsDBEnum::BUTTON_11_STS);
				m_enumMap.emplace("BUTTON_12_STS",UiDB::ButtonsStsDBEnum::BUTTON_12_STS);
				m_enumMap.emplace("BUTTON_13_STS",UiDB::ButtonsStsDBEnum::BUTTON_13_STS);
				m_enumMap.emplace("BUTTON_14_STS",UiDB::ButtonsStsDBEnum::BUTTON_14_STS);
				m_enumMap.emplace("BUTTON_15_STS",UiDB::ButtonsStsDBEnum::BUTTON_15_STS);
				m_enumMap.emplace("BUTTON_16_STS",UiDB::ButtonsStsDBEnum::BUTTON_16_STS);
				m_enumMap.emplace("BUTTON_17_STS",UiDB::ButtonsStsDBEnum::BUTTON_17_STS);
				m_enumMap.emplace("BUTTON_18_STS",UiDB::ButtonsStsDBEnum::BUTTON_18_STS);
				m_enumMap.emplace("BUTTONS_STS_DB_SIZE",UiDB::ButtonsStsDBEnum::BUTTONS_STS_DB_SIZE);
				m_enumMap.emplace("BUTTON_NOT_PRESSED",UiDB::ButtonPressStatusEnum::BUTTON_NOT_PRESSED);
				m_enumMap.emplace("BUTTON_PRESSED",UiDB::ButtonPressStatusEnum::BUTTON_PRESSED);
				m_enumMap.emplace("BUTTON_CONT_PRESSED",UiDB::ButtonPressStatusEnum::BUTTON_CONT_PRESSED);
				m_enumMap.emplace("NO_STATE",StateMachine_BringTo::BringTo_StatesEnum::NO_STATE);
				m_enumMap.emplace("STATE_BRING_TO_INITIATE",StateMachine_BringTo::BringTo_StatesEnum::STATE_BRING_TO_INITIATE);
				m_enumMap.emplace("STATE_BRING_TO_BRING_TO",StateMachine_BringTo::BringTo_StatesEnum::STATE_BRING_TO_BRING_TO);
				m_enumMap.emplace("STATE_BRING_TO_END_SUCCESS",StateMachine_BringTo::BringTo_StatesEnum::STATE_BRING_TO_END_SUCCESS);
				m_enumMap.emplace("STATE_BRING_TO_END_FAIL",StateMachine_BringTo::BringTo_StatesEnum::STATE_BRING_TO_END_FAIL);
				m_enumMap.emplace("NUM_OF_BRING_TO",StateMachine_BringTo::BringTo_StatesEnum::NUM_OF_BRING_TO);
				m_enumMap.emplace("NO_STATE",StateMachine_Scanning::Scanning_StatesEnum::NO_STATE);
				m_enumMap.emplace("STATE_SCANNING_INITIATE",StateMachine_Scanning::Scanning_StatesEnum::STATE_SCANNING_INITIATE);
				m_enumMap.emplace("STATE_SCANNING_SCANNING",StateMachine_Scanning::Scanning_StatesEnum::STATE_SCANNING_SCANNING);
				m_enumMap.emplace("STATE_SCANNING_END_SUCCESS",StateMachine_Scanning::Scanning_StatesEnum::STATE_SCANNING_END_SUCCESS);
				m_enumMap.emplace("STATE_SCANNING_END_FAIL",StateMachine_Scanning::Scanning_StatesEnum::STATE_SCANNING_END_FAIL);
				m_enumMap.emplace("NUM_OF_SCANNING",StateMachine_Scanning::Scanning_StatesEnum::NUM_OF_SCANNING);
				m_enumMap.emplace("NO_STATE",StateMachine_SystemStates::SystemStates_StatesEnum::NO_STATE);
				m_enumMap.emplace("STATE_INITIALIZATION",StateMachine_SystemStates::SystemStates_StatesEnum::STATE_INITIALIZATION);
				m_enumMap.emplace("STATE_OPERATIONAL",StateMachine_SystemStates::SystemStates_StatesEnum::STATE_OPERATIONAL);
				m_enumMap.emplace("STATE_BORESIGHT",StateMachine_SystemStates::SystemStates_StatesEnum::STATE_BORESIGHT);
				m_enumMap.emplace("STATE_CALIBRATION",StateMachine_SystemStates::SystemStates_StatesEnum::STATE_CALIBRATION);
				m_enumMap.emplace("STATE_IBIT",StateMachine_SystemStates::SystemStates_StatesEnum::STATE_IBIT);
				m_enumMap.emplace("STATE_MAINTENANCE",StateMachine_SystemStates::SystemStates_StatesEnum::STATE_MAINTENANCE);
				m_enumMap.emplace("NUM_OF_SYSTEM_STATES",StateMachine_SystemStates::SystemStates_StatesEnum::NUM_OF_SYSTEM_STATES);
				m_enumMap.emplace("PERMISSION_NONE",UiDB::PermissionByUserEnum::PERMISSION_NONE);
				m_enumMap.emplace("PERMISSION_ALL",UiDB::PermissionByUserEnum::PERMISSION_ALL);
				m_enumMap.emplace("PERMISSION_AUTHORITY",UiDB::PermissionByUserEnum::PERMISSION_AUTHORITY);
				m_enumMap.emplace("PERMISSION_ACTIVE_USER",UiDB::PermissionByUserEnum::PERMISSION_ACTIVE_USER);
				m_enumMap.emplace("PERMISSION_COMMANDER",UiDB::PermissionByUserEnum::PERMISSION_COMMANDER);
				m_enumMap.emplace("PERMISSION_GUNNER",UiDB::PermissionByUserEnum::PERMISSION_GUNNER);
				m_enumMap.emplace("PERMISSION_DEFAULT",UiDB::PermissionByUserEnum::PERMISSION_DEFAULT);
			}
			void InitDbRowMap()
			{
				m_dbRowMap.emplace("iActSystemStatesStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActSystemStatesStatus]);
				m_dbRowMap.emplace("iCurrentSystemState",m_inputTable[RulesDB::RulesInputDBEnum::iCurrentSystemState]);
				m_dbRowMap.emplace("iRequestSystemState",m_inputTable[RulesDB::RulesInputDBEnum::iRequestSystemState]);
				m_dbRowMap.emplace("iEnterGeneral",m_inputTable[RulesDB::RulesInputDBEnum::iEnterGeneral]);
				m_dbRowMap.emplace("iShaftEncoderTrvStatus",m_inputTable[RulesDB::RulesInputDBEnum::iShaftEncoderTrvStatus]);
				m_dbRowMap.emplace("iShaftEncoderElvStatus",m_inputTable[RulesDB::RulesInputDBEnum::iShaftEncoderElvStatus]);
				m_dbRowMap.emplace("iDriverElvStatus",m_inputTable[RulesDB::RulesInputDBEnum::iDriverElvStatus]);
				m_dbRowMap.emplace("iDriverTrvStatus",m_inputTable[RulesDB::RulesInputDBEnum::iDriverTrvStatus]);
				m_dbRowMap.emplace("iGyroTrvStatus",m_inputTable[RulesDB::RulesInputDBEnum::iGyroTrvStatus]);
				m_dbRowMap.emplace("iGyroElvStatus",m_inputTable[RulesDB::RulesInputDBEnum::iGyroElvStatus]);
				m_dbRowMap.emplace("iVoltageStatus",m_inputTable[RulesDB::RulesInputDBEnum::iVoltageStatus]);
				m_dbRowMap.emplace("iCommutationStatus",m_inputTable[RulesDB::RulesInputDBEnum::iCommutationStatus]);
				m_dbRowMap.emplace("iMastOpened",m_inputTable[RulesDB::RulesInputDBEnum::iMastOpened]);
				m_dbRowMap.emplace("iWorkModeRequest",m_inputTable[RulesDB::RulesInputDBEnum::iWorkModeRequest]);
				m_dbRowMap.emplace("iActWorkModeStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActWorkModeStatus]);
				m_dbRowMap.emplace("iCurrentWorkMode",m_inputTable[RulesDB::RulesInputDBEnum::iCurrentWorkMode]);
				m_dbRowMap.emplace("iMobileStatus",m_inputTable[RulesDB::RulesInputDBEnum::iMobileStatus]);
				m_dbRowMap.emplace("iActTrackingStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActTrackingStatus]);
				m_dbRowMap.emplace("iActScanningStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActScanningStatus]);
				m_dbRowMap.emplace("iActBringToStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActBringToStatus]);
				m_dbRowMap.emplace("iActBoreSightStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActBoreSightStatus]);
				m_dbRowMap.emplace("iActNucStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActNucStatus]);
				m_dbRowMap.emplace("iDesignatorStatus",m_inputTable[RulesDB::RulesInputDBEnum::iDesignatorStatus]);
				m_dbRowMap.emplace("iIlluminatorStatus",m_inputTable[RulesDB::RulesInputDBEnum::iIlluminatorStatus]);
				m_dbRowMap.emplace("iActDriftStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActDriftStatus]);
				m_dbRowMap.emplace("iDriftCompleted",m_inputTable[RulesDB::RulesInputDBEnum::iDriftCompleted]);
				m_dbRowMap.emplace("iHandlePalmStatus",m_inputTable[RulesDB::RulesInputDBEnum::iHandlePalmStatus]);
				m_dbRowMap.emplace("iHandlePalmInitiator",m_inputTable[RulesDB::RulesInputDBEnum::iHandlePalmInitiator]);
				m_dbRowMap.emplace("iHandleAnalogsInitiator",m_inputTable[RulesDB::RulesInputDBEnum::iHandleAnalogsInitiator]);
				m_dbRowMap.emplace("iHandleFovInitiator",m_inputTable[RulesDB::RulesInputDBEnum::iHandleFovInitiator]);
				m_dbRowMap.emplace("iHandleFovStatus",m_inputTable[RulesDB::RulesInputDBEnum::iHandleFovStatus]);
				m_dbRowMap.emplace("iHandleTrackInitiator",m_inputTable[RulesDB::RulesInputDBEnum::iHandleTrackInitiator]);
				m_dbRowMap.emplace("iHandleTrackStatus",m_inputTable[RulesDB::RulesInputDBEnum::iHandleTrackStatus]);
				m_dbRowMap.emplace("iActControlStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActControlStatus]);
				m_dbRowMap.emplace("iHandleAnalogsElvAboveThreshold",m_inputTable[RulesDB::RulesInputDBEnum::iHandleAnalogsElvAboveThreshold]);
				m_dbRowMap.emplace("iHandleAnalogsTrvAboveThreshold",m_inputTable[RulesDB::RulesInputDBEnum::iHandleAnalogsTrvAboveThreshold]);
				m_dbRowMap.emplace("iHandleLaseInitiator",m_inputTable[RulesDB::RulesInputDBEnum::iHandleLaseInitiator]);
				m_dbRowMap.emplace("iHandleLaseStatus",m_inputTable[RulesDB::RulesInputDBEnum::iHandleLaseStatus]);
				m_dbRowMap.emplace("iProcessBringToInitiator",m_inputTable[RulesDB::RulesInputDBEnum::iProcessBringToInitiator]);
				m_dbRowMap.emplace("iProcessBringToStatus",m_inputTable[RulesDB::RulesInputDBEnum::iProcessBringToStatus]);
				m_dbRowMap.emplace("iControlBringToProcessStatus",m_inputTable[RulesDB::RulesInputDBEnum::iControlBringToProcessStatus]);
				m_dbRowMap.emplace("iBringToState",m_inputTable[RulesDB::RulesInputDBEnum::iBringToState]);
				m_dbRowMap.emplace("iScanningState",m_inputTable[RulesDB::RulesInputDBEnum::iScanningState]);
				m_dbRowMap.emplace("iControlScanningProcessStatus",m_inputTable[RulesDB::RulesInputDBEnum::iControlScanningProcessStatus]);
				m_dbRowMap.emplace("iProcessScanningInitiator",m_inputTable[RulesDB::RulesInputDBEnum::iProcessScanningInitiator]);
				m_dbRowMap.emplace("iProcessScanningStatus",m_inputTable[RulesDB::RulesInputDBEnum::iProcessScanningStatus]);
				m_dbRowMap.emplace("iActLaseStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActLaseStatus]);
				m_dbRowMap.emplace("iActCcdStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActCcdStatus]);
				m_dbRowMap.emplace("iActTiStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActTiStatus]);
				m_dbRowMap.emplace("iActVmdStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActVmdStatus]);
				m_dbRowMap.emplace("iActReticleStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActReticleStatus]);
				m_dbRowMap.emplace("iActVideoSwitchingStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActVideoSwitchingStatus]);
				m_dbRowMap.emplace("iAutoTrackingStatus",m_inputTable[RulesDB::RulesInputDBEnum::iAutoTrackingStatus]);
				m_dbRowMap.emplace("iActSlaStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActSlaStatus]);
				m_dbRowMap.emplace("iDayCamCommStatus",m_inputTable[RulesDB::RulesInputDBEnum::iDayCamCommStatus]);
				m_dbRowMap.emplace("iNightCamCommStatus",m_inputTable[RulesDB::RulesInputDBEnum::iNightCamCommStatus]);
				m_dbRowMap.emplace("iLrfCamCommStatus",m_inputTable[RulesDB::RulesInputDBEnum::iLrfCamCommStatus]);
				m_dbRowMap.emplace("iActSeCalibrationStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActSeCalibrationStatus]);
				m_dbRowMap.emplace("iDiscreteLiReadySwitch",m_inputTable[RulesDB::RulesInputDBEnum::iDiscreteLiReadySwitch]);
				m_dbRowMap.emplace("iDiscreteLiUnitStsSwitch",m_inputTable[RulesDB::RulesInputDBEnum::iDiscreteLiUnitStsSwitch]);
				m_dbRowMap.emplace("iLiSelectStatus",m_inputTable[RulesDB::RulesInputDBEnum::iLiSelectStatus]);
				m_dbRowMap.emplace("iLdLiLasingPasswordSts",m_inputTable[RulesDB::RulesInputDBEnum::iLdLiLasingPasswordSts]);
				m_dbRowMap.emplace("iLiOnOffRequest",m_inputTable[RulesDB::RulesInputDBEnum::iLiOnOffRequest]);
				m_dbRowMap.emplace("iLdOnOffRequest",m_inputTable[RulesDB::RulesInputDBEnum::iLdOnOffRequest]);
				m_dbRowMap.emplace("iLrfOnOffRequest",m_inputTable[RulesDB::RulesInputDBEnum::iLrfOnOffRequest]);
				m_dbRowMap.emplace("iLiCommStatus",m_inputTable[RulesDB::RulesInputDBEnum::iLiCommStatus]);
				m_dbRowMap.emplace("iLdCommStatus",m_inputTable[RulesDB::RulesInputDBEnum::iLdCommStatus]);
				m_dbRowMap.emplace("iCommanderSelectedSys",m_inputTable[RulesDB::RulesInputDBEnum::iCommanderSelectedSys]);
				m_dbRowMap.emplace("iGunnerSelectedSys",m_inputTable[RulesDB::RulesInputDBEnum::iGunnerSelectedSys]);
				m_dbRowMap.emplace("iHatchesCloseSts",m_inputTable[RulesDB::RulesInputDBEnum::iHatchesCloseSts]);
				m_dbRowMap.emplace("iLiLdLasingSwitch",m_inputTable[RulesDB::RulesInputDBEnum::iLiLdLasingSwitch]);
				m_dbRowMap.emplace("iLdNormalTemperature",m_inputTable[RulesDB::RulesInputDBEnum::iLdNormalTemperature]);
				m_dbRowMap.emplace("iGunShaftEncoderElvStatus",m_inputTable[RulesDB::RulesInputDBEnum::iGunShaftEncoderElvStatus]);
				m_dbRowMap.emplace("iGunShaftEncoderTrvStatus",m_inputTable[RulesDB::RulesInputDBEnum::iGunShaftEncoderTrvStatus]);
				m_dbRowMap.emplace("iNightCamReady",m_inputTable[RulesDB::RulesInputDBEnum::iNightCamReady]);
				m_dbRowMap.emplace("iActFusionStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActFusionStatus]);
				m_dbRowMap.emplace("iBoreSightType",m_inputTable[RulesDB::RulesInputDBEnum::iBoreSightType]);
				m_dbRowMap.emplace("iTrackingUser",m_inputTable[RulesDB::RulesInputDBEnum::iTrackingUser]);
				m_dbRowMap.emplace("iSlaCommStatus",m_inputTable[RulesDB::RulesInputDBEnum::iSlaCommStatus]);
				m_dbRowMap.emplace("iTrackingCamChange",m_inputTable[RulesDB::RulesInputDBEnum::iTrackingCamChange]);
				m_dbRowMap.emplace("iStabErrorGroupStatus",m_inputTable[RulesDB::RulesInputDBEnum::iStabErrorGroupStatus]);
				m_dbRowMap.emplace("iPowerErrorGroupStatus",m_inputTable[RulesDB::RulesInputDBEnum::iPowerErrorGroupStatus]);
				m_dbRowMap.emplace("iTrackingCam",m_inputTable[RulesDB::RulesInputDBEnum::iTrackingCam]);
				m_dbRowMap.emplace("iActFovCalibStatus",m_inputTable[RulesDB::RulesInputDBEnum::iActFovCalibStatus]);
				m_dbRowMap.emplace("iLiBsOnOffRequest",m_inputTable[RulesDB::RulesInputDBEnum::iLiBsOnOffRequest]);
				m_dbRowMap.emplace("iMastClosed",m_inputTable[RulesDB::RulesInputDBEnum::iMastClosed]);
				m_dbRowMap.emplace("oActSystemStatesCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActSystemStatesCmd]);
				m_dbRowMap.emplace("oGoToSystemState",m_outputTable[RulesDB::RulesOutputDBEnum::oGoToSystemState]);
				m_dbRowMap.emplace("oChangeWorkMode",m_outputTable[RulesDB::RulesOutputDBEnum::oChangeWorkMode]);
				m_dbRowMap.emplace("oActWorkModeCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActWorkModeCmd]);
				m_dbRowMap.emplace("oPalmConditionForDrift",m_outputTable[RulesDB::RulesOutputDBEnum::oPalmConditionForDrift]);
				m_dbRowMap.emplace("oActDriftCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActDriftCmd]);
				m_dbRowMap.emplace("oActiveOperator",m_outputTable[RulesDB::RulesOutputDBEnum::oActiveOperator]);
				m_dbRowMap.emplace("oCurrentMaster",m_outputTable[RulesDB::RulesOutputDBEnum::oCurrentMaster]);
				m_dbRowMap.emplace("oHandlePalmPermission",m_outputTable[RulesDB::RulesOutputDBEnum::oHandlePalmPermission]);
				m_dbRowMap.emplace("oHandleAnalogsPermission",m_outputTable[RulesDB::RulesOutputDBEnum::oHandleAnalogsPermission]);
				m_dbRowMap.emplace("oHandleFovPermission",m_outputTable[RulesDB::RulesOutputDBEnum::oHandleFovPermission]);
				m_dbRowMap.emplace("oHandleTrackPermission",m_outputTable[RulesDB::RulesOutputDBEnum::oHandleTrackPermission]);
				m_dbRowMap.emplace("oActControlCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActControlCmd]);
				m_dbRowMap.emplace("oHandleLasePermission",m_outputTable[RulesDB::RulesOutputDBEnum::oHandleLasePermission]);
				m_dbRowMap.emplace("oActBringToCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActBringToCmd]);
				m_dbRowMap.emplace("oGoToBringToState",m_outputTable[RulesDB::RulesOutputDBEnum::oGoToBringToState]);
				m_dbRowMap.emplace("oProcessBringToPermission",m_outputTable[RulesDB::RulesOutputDBEnum::oProcessBringToPermission]);
				m_dbRowMap.emplace("oActScanningCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActScanningCmd]);
				m_dbRowMap.emplace("oGoToScanningState",m_outputTable[RulesDB::RulesOutputDBEnum::oGoToScanningState]);
				m_dbRowMap.emplace("oActLaseCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActLaseCmd]);
				m_dbRowMap.emplace("oActCcdCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActCcdCmd]);
				m_dbRowMap.emplace("oActTiCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActTiCmd]);
				m_dbRowMap.emplace("oActVmdCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActVmdCmd]);
				m_dbRowMap.emplace("oActReticleCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActReticleCmd]);
				m_dbRowMap.emplace("oActVideoSwitchingCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActVideoSwitchingCmd]);
				m_dbRowMap.emplace("oActAutoTrackingCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActAutoTrackingCmd]);
				m_dbRowMap.emplace("oAutoTrackingRequest",m_outputTable[RulesDB::RulesOutputDBEnum::oAutoTrackingRequest]);
				m_dbRowMap.emplace("oActSlaCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActSlaCmd]);
				m_dbRowMap.emplace("oBringToPermission",m_outputTable[RulesDB::RulesOutputDBEnum::oBringToPermission]);
				m_dbRowMap.emplace("oScanningPermission",m_outputTable[RulesDB::RulesOutputDBEnum::oScanningPermission]);
				m_dbRowMap.emplace("oActSeCalibrationCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActSeCalibrationCmd]);
				m_dbRowMap.emplace("oActBoreSightCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActBoreSightCmd]);
				m_dbRowMap.emplace("oLiLasingRequest",m_outputTable[RulesDB::RulesOutputDBEnum::oLiLasingRequest]);
				m_dbRowMap.emplace("oLdLasingRequest",m_outputTable[RulesDB::RulesOutputDBEnum::oLdLasingRequest]);
				m_dbRowMap.emplace("oLiPowerEnableCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oLiPowerEnableCmd]);
				m_dbRowMap.emplace("oLdPowerEnableCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oLdPowerEnableCmd]);
				m_dbRowMap.emplace("oLiProcessEnableCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oLiProcessEnableCmd]);
				m_dbRowMap.emplace("oLdProcessEnableCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oLdProcessEnableCmd]);
				m_dbRowMap.emplace("oConditionsForBringObsrvToGun",m_outputTable[RulesDB::RulesOutputDBEnum::oConditionsForBringObsrvToGun]);
				m_dbRowMap.emplace("oActFusionCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActFusionCmd]);
				m_dbRowMap.emplace("oVmdPermission",m_outputTable[RulesDB::RulesOutputDBEnum::oVmdPermission]);
				m_dbRowMap.emplace("oActFovCalibCmd",m_outputTable[RulesDB::RulesOutputDBEnum::oActFovCalibCmd]);
				m_dbRowMap.emplace("rConditionForLdDayBoreSight",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLdDayBoreSight]);
				m_dbRowMap.emplace("rConditionForLiDayBoreSight",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLiDayBoreSight]);
				m_dbRowMap.emplace("rConditionForThermalBoreSight",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForThermalBoreSight]);
				m_dbRowMap.emplace("rConditionForDrift",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForDrift]);
				m_dbRowMap.emplace("rEndDriftFail",m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndDriftFail]);
				m_dbRowMap.emplace("rEndDriftSuccess",m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndDriftSuccess]);
				m_dbRowMap.emplace("rStartDrift",m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartDrift]);
				m_dbRowMap.emplace("rPalm_pressed",m_existenceTable[RulesDB::RulesExistenceDBEnum::rPalm_pressed]);
				m_dbRowMap.emplace("rConditionForDayFovCalib",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForDayFovCalib]);
				m_dbRowMap.emplace("rConditionForThermalFovCalib",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForThermalFovCalib]);
				m_dbRowMap.emplace("rEnable_General_Activities",m_existenceTable[RulesDB::RulesExistenceDBEnum::rEnable_General_Activities]);
				m_dbRowMap.emplace("rActiveOperatorCommander",m_existenceTable[RulesDB::RulesExistenceDBEnum::rActiveOperatorCommander]);
				m_dbRowMap.emplace("rActiveOperatorGunner",m_existenceTable[RulesDB::RulesExistenceDBEnum::rActiveOperatorGunner]);
				m_dbRowMap.emplace("rActiveOperatorNone",m_existenceTable[RulesDB::RulesExistenceDBEnum::rActiveOperatorNone]);
				m_dbRowMap.emplace("rBoresightToMaintenance",m_existenceTable[RulesDB::RulesExistenceDBEnum::rBoresightToMaintenance]);
				m_dbRowMap.emplace("rBoresightToOperational",m_existenceTable[RulesDB::RulesExistenceDBEnum::rBoresightToOperational]);
				m_dbRowMap.emplace("rCalibrationToMaintenance",m_existenceTable[RulesDB::RulesExistenceDBEnum::rCalibrationToMaintenance]);
				m_dbRowMap.emplace("rCalibrationToOperational",m_existenceTable[RulesDB::RulesExistenceDBEnum::rCalibrationToOperational]);
				m_dbRowMap.emplace("rIbitToMaintenance",m_existenceTable[RulesDB::RulesExistenceDBEnum::rIbitToMaintenance]);
				m_dbRowMap.emplace("rIBitToOperational",m_existenceTable[RulesDB::RulesExistenceDBEnum::rIBitToOperational]);
				m_dbRowMap.emplace("rInitToOperational",m_existenceTable[RulesDB::RulesExistenceDBEnum::rInitToOperational]);
				m_dbRowMap.emplace("rMaintenanceToBoresight",m_existenceTable[RulesDB::RulesExistenceDBEnum::rMaintenanceToBoresight]);
				m_dbRowMap.emplace("rMaintenanceToCalibration",m_existenceTable[RulesDB::RulesExistenceDBEnum::rMaintenanceToCalibration]);
				m_dbRowMap.emplace("rMaintenanceToIBit",m_existenceTable[RulesDB::RulesExistenceDBEnum::rMaintenanceToIBit]);
				m_dbRowMap.emplace("rMaintenanceToOperational",m_existenceTable[RulesDB::RulesExistenceDBEnum::rMaintenanceToOperational]);
				m_dbRowMap.emplace("rOperationalToMaintenance",m_existenceTable[RulesDB::RulesExistenceDBEnum::rOperationalToMaintenance]);
				m_dbRowMap.emplace("rChangeToManualMode",m_existenceTable[RulesDB::RulesExistenceDBEnum::rChangeToManualMode]);
				m_dbRowMap.emplace("rChangeToPowerMode",m_existenceTable[RulesDB::RulesExistenceDBEnum::rChangeToPowerMode]);
				m_dbRowMap.emplace("rConditionForPowerMode",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForPowerMode]);
				m_dbRowMap.emplace("rChangeToStabMode",m_existenceTable[RulesDB::RulesExistenceDBEnum::rChangeToStabMode]);
				m_dbRowMap.emplace("rConditionForStabMode",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForStabMode]);
				m_dbRowMap.emplace("rConditionForNuc",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForNuc]);
				m_dbRowMap.emplace("rConditionForTracking",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForTracking]);
				m_dbRowMap.emplace("rStartAutoTracking",m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartAutoTracking]);
				m_dbRowMap.emplace("rStopAutoTrackingByFail",m_existenceTable[RulesDB::RulesExistenceDBEnum::rStopAutoTrackingByFail]);
				m_dbRowMap.emplace("rStopAutoTrackingByUser",m_existenceTable[RulesDB::RulesExistenceDBEnum::rStopAutoTrackingByUser]);
				m_dbRowMap.emplace("rBringToInitiateEnded",m_existenceTable[RulesDB::RulesExistenceDBEnum::rBringToInitiateEnded]);
				m_dbRowMap.emplace("rConditionForBringObsrvToGun",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForBringObsrvToGun]);
				m_dbRowMap.emplace("rConditionForBringTo",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForBringTo]);
				m_dbRowMap.emplace("rEndBringToFail",m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndBringToFail]);
				m_dbRowMap.emplace("rEndBringToSuccess",m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndBringToSuccess]);
				m_dbRowMap.emplace("rStartBringTo",m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartBringTo]);
				m_dbRowMap.emplace("rPerformLasing",m_existenceTable[RulesDB::RulesExistenceDBEnum::rPerformLasing]);
				m_dbRowMap.emplace("rWakeLasingActivity",m_existenceTable[RulesDB::RulesExistenceDBEnum::rWakeLasingActivity]);
				m_dbRowMap.emplace("rConditionForLdEnable",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLdEnable]);
				m_dbRowMap.emplace("rConditionForLdPower",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLdPower]);
				m_dbRowMap.emplace("rStartLdProcess",m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartLdProcess]);
				m_dbRowMap.emplace("rStopLdProcess",m_existenceTable[RulesDB::RulesExistenceDBEnum::rStopLdProcess]);
				m_dbRowMap.emplace("rConditionForLiEnable",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLiEnable]);
				m_dbRowMap.emplace("rConditionForLiPower",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForLiPower]);
				m_dbRowMap.emplace("rStartLiProcess",m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartLiProcess]);
				m_dbRowMap.emplace("rStopLiProcess",m_existenceTable[RulesDB::RulesExistenceDBEnum::rStopLiProcess]);
				m_dbRowMap.emplace("rConditionForScanning",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForScanning]);
				m_dbRowMap.emplace("rEndScanningFail",m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndScanningFail]);
				m_dbRowMap.emplace("rEndScanningSuccess",m_existenceTable[RulesDB::RulesExistenceDBEnum::rEndScanningSuccess]);
				m_dbRowMap.emplace("rScanningInitiateEnded",m_existenceTable[RulesDB::RulesExistenceDBEnum::rScanningInitiateEnded]);
				m_dbRowMap.emplace("rStartScanning",m_existenceTable[RulesDB::RulesExistenceDBEnum::rStartScanning]);
				m_dbRowMap.emplace("rDisableHandles",m_existenceTable[RulesDB::RulesExistenceDBEnum::rDisableHandles]);
				m_dbRowMap.emplace("rEnableLasing",m_existenceTable[RulesDB::RulesExistenceDBEnum::rEnableLasing]);
				m_dbRowMap.emplace("rPermissionMasterCommander",m_existenceTable[RulesDB::RulesExistenceDBEnum::rPermissionMasterCommander]);
				m_dbRowMap.emplace("rPermissionMasterGunner",m_existenceTable[RulesDB::RulesExistenceDBEnum::rPermissionMasterGunner]);
				m_dbRowMap.emplace("rPermissionMasterNone",m_existenceTable[RulesDB::RulesExistenceDBEnum::rPermissionMasterNone]);
				m_dbRowMap.emplace("rEnableVMD",m_existenceTable[RulesDB::RulesExistenceDBEnum::rEnableVMD]);
				m_dbRowMap.emplace("rConditionForOpticButton",m_existenceTable[RulesDB::RulesExistenceDBEnum::rConditionForOpticButton]);
				m_dbRowMap.emplace("rMasterCommander",m_existenceTable[RulesDB::RulesExistenceDBEnum::rMasterCommander]);
				m_dbRowMap.emplace("rMasterGunner",m_existenceTable[RulesDB::RulesExistenceDBEnum::rMasterGunner]);
				m_dbRowMap.emplace("rMasterNone",m_existenceTable[RulesDB::RulesExistenceDBEnum::rMasterNone]);
				m_dbRowMap.emplace("enConditionForLdDayBoreSight",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForLdDayBoreSight]);
				m_dbRowMap.emplace("enConditionForLiDayBoreSight",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForLiDayBoreSight]);
				m_dbRowMap.emplace("enConditionForThermalBoreSight",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForThermalBoreSight]);
				m_dbRowMap.emplace("enConditionForDrift",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForDrift]);
				m_dbRowMap.emplace("enEndDriftFail",m_enableTable[ RulesDB::RulesEnabledDBEnum::enEndDriftFail]);
				m_dbRowMap.emplace("enEndDriftSuccess",m_enableTable[ RulesDB::RulesEnabledDBEnum::enEndDriftSuccess]);
				m_dbRowMap.emplace("enStartDrift",m_enableTable[ RulesDB::RulesEnabledDBEnum::enStartDrift]);
				m_dbRowMap.emplace("enPalm_pressed",m_enableTable[ RulesDB::RulesEnabledDBEnum::enPalm_pressed]);
				m_dbRowMap.emplace("enConditionForDayFovCalib",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForDayFovCalib]);
				m_dbRowMap.emplace("enConditionForThermalFovCalib",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForThermalFovCalib]);
				m_dbRowMap.emplace("enEnable_General_Activities",m_enableTable[ RulesDB::RulesEnabledDBEnum::enEnable_General_Activities]);
				m_dbRowMap.emplace("enActiveOperatorCommander",m_enableTable[ RulesDB::RulesEnabledDBEnum::enActiveOperatorCommander]);
				m_dbRowMap.emplace("enActiveOperatorGunner",m_enableTable[ RulesDB::RulesEnabledDBEnum::enActiveOperatorGunner]);
				m_dbRowMap.emplace("enActiveOperatorNone",m_enableTable[ RulesDB::RulesEnabledDBEnum::enActiveOperatorNone]);
				m_dbRowMap.emplace("enBoresightToMaintenance",m_enableTable[ RulesDB::RulesEnabledDBEnum::enBoresightToMaintenance]);
				m_dbRowMap.emplace("enBoresightToOperational",m_enableTable[ RulesDB::RulesEnabledDBEnum::enBoresightToOperational]);
				m_dbRowMap.emplace("enCalibrationToMaintenance",m_enableTable[ RulesDB::RulesEnabledDBEnum::enCalibrationToMaintenance]);
				m_dbRowMap.emplace("enCalibrationToOperational",m_enableTable[ RulesDB::RulesEnabledDBEnum::enCalibrationToOperational]);
				m_dbRowMap.emplace("enIbitToMaintenance",m_enableTable[ RulesDB::RulesEnabledDBEnum::enIbitToMaintenance]);
				m_dbRowMap.emplace("enIBitToOperational",m_enableTable[ RulesDB::RulesEnabledDBEnum::enIBitToOperational]);
				m_dbRowMap.emplace("enInitToOperational",m_enableTable[ RulesDB::RulesEnabledDBEnum::enInitToOperational]);
				m_dbRowMap.emplace("enMaintenanceToBoresight",m_enableTable[ RulesDB::RulesEnabledDBEnum::enMaintenanceToBoresight]);
				m_dbRowMap.emplace("enMaintenanceToCalibration",m_enableTable[ RulesDB::RulesEnabledDBEnum::enMaintenanceToCalibration]);
				m_dbRowMap.emplace("enMaintenanceToIBit",m_enableTable[ RulesDB::RulesEnabledDBEnum::enMaintenanceToIBit]);
				m_dbRowMap.emplace("enMaintenanceToOperational",m_enableTable[ RulesDB::RulesEnabledDBEnum::enMaintenanceToOperational]);
				m_dbRowMap.emplace("enOperationalToMaintenance",m_enableTable[ RulesDB::RulesEnabledDBEnum::enOperationalToMaintenance]);
				m_dbRowMap.emplace("enChangeToManualMode",m_enableTable[ RulesDB::RulesEnabledDBEnum::enChangeToManualMode]);
				m_dbRowMap.emplace("enChangeToPowerMode",m_enableTable[ RulesDB::RulesEnabledDBEnum::enChangeToPowerMode]);
				m_dbRowMap.emplace("enConditionForPowerMode",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForPowerMode]);
				m_dbRowMap.emplace("enChangeToStabMode",m_enableTable[ RulesDB::RulesEnabledDBEnum::enChangeToStabMode]);
				m_dbRowMap.emplace("enConditionForStabMode",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForStabMode]);
				m_dbRowMap.emplace("enConditionForNuc",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForNuc]);
				m_dbRowMap.emplace("enConditionForTracking",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForTracking]);
				m_dbRowMap.emplace("enStartAutoTracking",m_enableTable[ RulesDB::RulesEnabledDBEnum::enStartAutoTracking]);
				m_dbRowMap.emplace("enStopAutoTrackingByFail",m_enableTable[ RulesDB::RulesEnabledDBEnum::enStopAutoTrackingByFail]);
				m_dbRowMap.emplace("enStopAutoTrackingByUser",m_enableTable[ RulesDB::RulesEnabledDBEnum::enStopAutoTrackingByUser]);
				m_dbRowMap.emplace("enBringToInitiateEnded",m_enableTable[ RulesDB::RulesEnabledDBEnum::enBringToInitiateEnded]);
				m_dbRowMap.emplace("enConditionForBringObsrvToGun",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForBringObsrvToGun]);
				m_dbRowMap.emplace("enConditionForBringTo",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForBringTo]);
				m_dbRowMap.emplace("enEndBringToFail",m_enableTable[ RulesDB::RulesEnabledDBEnum::enEndBringToFail]);
				m_dbRowMap.emplace("enEndBringToSuccess",m_enableTable[ RulesDB::RulesEnabledDBEnum::enEndBringToSuccess]);
				m_dbRowMap.emplace("enStartBringTo",m_enableTable[ RulesDB::RulesEnabledDBEnum::enStartBringTo]);
				m_dbRowMap.emplace("enPerformLasing",m_enableTable[ RulesDB::RulesEnabledDBEnum::enPerformLasing]);
				m_dbRowMap.emplace("enWakeLasingActivity",m_enableTable[ RulesDB::RulesEnabledDBEnum::enWakeLasingActivity]);
				m_dbRowMap.emplace("enConditionForLdEnable",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForLdEnable]);
				m_dbRowMap.emplace("enConditionForLdPower",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForLdPower]);
				m_dbRowMap.emplace("enStartLdProcess",m_enableTable[ RulesDB::RulesEnabledDBEnum::enStartLdProcess]);
				m_dbRowMap.emplace("enStopLdProcess",m_enableTable[ RulesDB::RulesEnabledDBEnum::enStopLdProcess]);
				m_dbRowMap.emplace("enConditionForLiEnable",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForLiEnable]);
				m_dbRowMap.emplace("enConditionForLiPower",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForLiPower]);
				m_dbRowMap.emplace("enStartLiProcess",m_enableTable[ RulesDB::RulesEnabledDBEnum::enStartLiProcess]);
				m_dbRowMap.emplace("enStopLiProcess",m_enableTable[ RulesDB::RulesEnabledDBEnum::enStopLiProcess]);
				m_dbRowMap.emplace("enConditionForScanning",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForScanning]);
				m_dbRowMap.emplace("enEndScanningFail",m_enableTable[ RulesDB::RulesEnabledDBEnum::enEndScanningFail]);
				m_dbRowMap.emplace("enEndScanningSuccess",m_enableTable[ RulesDB::RulesEnabledDBEnum::enEndScanningSuccess]);
				m_dbRowMap.emplace("enScanningInitiateEnded",m_enableTable[ RulesDB::RulesEnabledDBEnum::enScanningInitiateEnded]);
				m_dbRowMap.emplace("enStartScanning",m_enableTable[ RulesDB::RulesEnabledDBEnum::enStartScanning]);
				m_dbRowMap.emplace("enDisableHandles",m_enableTable[ RulesDB::RulesEnabledDBEnum::enDisableHandles]);
				m_dbRowMap.emplace("enEnableLasing",m_enableTable[ RulesDB::RulesEnabledDBEnum::enEnableLasing]);
				m_dbRowMap.emplace("enPermissionMasterCommander",m_enableTable[ RulesDB::RulesEnabledDBEnum::enPermissionMasterCommander]);
				m_dbRowMap.emplace("enPermissionMasterGunner",m_enableTable[ RulesDB::RulesEnabledDBEnum::enPermissionMasterGunner]);
				m_dbRowMap.emplace("enPermissionMasterNone",m_enableTable[ RulesDB::RulesEnabledDBEnum::enPermissionMasterNone]);
				m_dbRowMap.emplace("enEnableVMD",m_enableTable[ RulesDB::RulesEnabledDBEnum::enEnableVMD]);
				m_dbRowMap.emplace("enConditionForOpticButton",m_enableTable[ RulesDB::RulesEnabledDBEnum::enConditionForOpticButton]);
				m_dbRowMap.emplace("enMasterCommander",m_enableTable[ RulesDB::RulesEnabledDBEnum::enMasterCommander]);
				m_dbRowMap.emplace("enMasterGunner",m_enableTable[ RulesDB::RulesEnabledDBEnum::enMasterGunner]);
				m_dbRowMap.emplace("enMasterNone",m_enableTable[ RulesDB::RulesEnabledDBEnum::enMasterNone]);
			}
			void InitRuleEnabledMap()
			{
				m_ruleEnabledMap.emplace(0,RulesDB::RulesEnabledDBEnum::enConditionForLdDayBoreSight);
				m_ruleEnabledMap.emplace(1,RulesDB::RulesEnabledDBEnum::enConditionForLiDayBoreSight);
				m_ruleEnabledMap.emplace(2,RulesDB::RulesEnabledDBEnum::enConditionForThermalBoreSight);
				m_ruleEnabledMap.emplace(3,RulesDB::RulesEnabledDBEnum::enConditionForDrift);
				m_ruleEnabledMap.emplace(4,RulesDB::RulesEnabledDBEnum::enEndDriftFail);
				m_ruleEnabledMap.emplace(5,RulesDB::RulesEnabledDBEnum::enEndDriftSuccess);
				m_ruleEnabledMap.emplace(6,RulesDB::RulesEnabledDBEnum::enStartDrift);
				m_ruleEnabledMap.emplace(7,RulesDB::RulesEnabledDBEnum::enPalm_pressed);
				m_ruleEnabledMap.emplace(8,RulesDB::RulesEnabledDBEnum::enConditionForDayFovCalib);
				m_ruleEnabledMap.emplace(9,RulesDB::RulesEnabledDBEnum::enConditionForThermalFovCalib);
				m_ruleEnabledMap.emplace(10,RulesDB::RulesEnabledDBEnum::enEnable_General_Activities);
				m_ruleEnabledMap.emplace(11,RulesDB::RulesEnabledDBEnum::enActiveOperatorCommander);
				m_ruleEnabledMap.emplace(12,RulesDB::RulesEnabledDBEnum::enActiveOperatorGunner);
				m_ruleEnabledMap.emplace(13,RulesDB::RulesEnabledDBEnum::enActiveOperatorNone);
				m_ruleEnabledMap.emplace(14,RulesDB::RulesEnabledDBEnum::enBoresightToMaintenance);
				m_ruleEnabledMap.emplace(15,RulesDB::RulesEnabledDBEnum::enBoresightToOperational);
				m_ruleEnabledMap.emplace(16,RulesDB::RulesEnabledDBEnum::enCalibrationToMaintenance);
				m_ruleEnabledMap.emplace(17,RulesDB::RulesEnabledDBEnum::enCalibrationToOperational);
				m_ruleEnabledMap.emplace(18,RulesDB::RulesEnabledDBEnum::enIbitToMaintenance);
				m_ruleEnabledMap.emplace(19,RulesDB::RulesEnabledDBEnum::enIBitToOperational);
				m_ruleEnabledMap.emplace(20,RulesDB::RulesEnabledDBEnum::enInitToOperational);
				m_ruleEnabledMap.emplace(21,RulesDB::RulesEnabledDBEnum::enMaintenanceToBoresight);
				m_ruleEnabledMap.emplace(22,RulesDB::RulesEnabledDBEnum::enMaintenanceToCalibration);
				m_ruleEnabledMap.emplace(23,RulesDB::RulesEnabledDBEnum::enMaintenanceToIBit);
				m_ruleEnabledMap.emplace(24,RulesDB::RulesEnabledDBEnum::enMaintenanceToOperational);
				m_ruleEnabledMap.emplace(25,RulesDB::RulesEnabledDBEnum::enOperationalToMaintenance);
				m_ruleEnabledMap.emplace(26,RulesDB::RulesEnabledDBEnum::enChangeToManualMode);
				m_ruleEnabledMap.emplace(27,RulesDB::RulesEnabledDBEnum::enChangeToPowerMode);
				m_ruleEnabledMap.emplace(28,RulesDB::RulesEnabledDBEnum::enConditionForPowerMode);
				m_ruleEnabledMap.emplace(29,RulesDB::RulesEnabledDBEnum::enChangeToStabMode);
				m_ruleEnabledMap.emplace(30,RulesDB::RulesEnabledDBEnum::enConditionForStabMode);
				m_ruleEnabledMap.emplace(31,RulesDB::RulesEnabledDBEnum::enConditionForNuc);
				m_ruleEnabledMap.emplace(32,RulesDB::RulesEnabledDBEnum::enConditionForTracking);
				m_ruleEnabledMap.emplace(33,RulesDB::RulesEnabledDBEnum::enStartAutoTracking);
				m_ruleEnabledMap.emplace(34,RulesDB::RulesEnabledDBEnum::enStopAutoTrackingByFail);
				m_ruleEnabledMap.emplace(35,RulesDB::RulesEnabledDBEnum::enStopAutoTrackingByUser);
				m_ruleEnabledMap.emplace(36,RulesDB::RulesEnabledDBEnum::enBringToInitiateEnded);
				m_ruleEnabledMap.emplace(37,RulesDB::RulesEnabledDBEnum::enConditionForBringObsrvToGun);
				m_ruleEnabledMap.emplace(38,RulesDB::RulesEnabledDBEnum::enConditionForBringTo);
				m_ruleEnabledMap.emplace(39,RulesDB::RulesEnabledDBEnum::enEndBringToFail);
				m_ruleEnabledMap.emplace(40,RulesDB::RulesEnabledDBEnum::enEndBringToSuccess);
				m_ruleEnabledMap.emplace(41,RulesDB::RulesEnabledDBEnum::enStartBringTo);
				m_ruleEnabledMap.emplace(42,RulesDB::RulesEnabledDBEnum::enPerformLasing);
				m_ruleEnabledMap.emplace(43,RulesDB::RulesEnabledDBEnum::enWakeLasingActivity);
				m_ruleEnabledMap.emplace(44,RulesDB::RulesEnabledDBEnum::enConditionForLdEnable);
				m_ruleEnabledMap.emplace(45,RulesDB::RulesEnabledDBEnum::enConditionForLdPower);
				m_ruleEnabledMap.emplace(46,RulesDB::RulesEnabledDBEnum::enStartLdProcess);
				m_ruleEnabledMap.emplace(47,RulesDB::RulesEnabledDBEnum::enStopLdProcess);
				m_ruleEnabledMap.emplace(48,RulesDB::RulesEnabledDBEnum::enConditionForLiEnable);
				m_ruleEnabledMap.emplace(49,RulesDB::RulesEnabledDBEnum::enConditionForLiPower);
				m_ruleEnabledMap.emplace(50,RulesDB::RulesEnabledDBEnum::enStartLiProcess);
				m_ruleEnabledMap.emplace(51,RulesDB::RulesEnabledDBEnum::enStopLiProcess);
				m_ruleEnabledMap.emplace(52,RulesDB::RulesEnabledDBEnum::enConditionForScanning);
				m_ruleEnabledMap.emplace(53,RulesDB::RulesEnabledDBEnum::enEndScanningFail);
				m_ruleEnabledMap.emplace(54,RulesDB::RulesEnabledDBEnum::enEndScanningSuccess);
				m_ruleEnabledMap.emplace(55,RulesDB::RulesEnabledDBEnum::enScanningInitiateEnded);
				m_ruleEnabledMap.emplace(56,RulesDB::RulesEnabledDBEnum::enStartScanning);
				m_ruleEnabledMap.emplace(57,RulesDB::RulesEnabledDBEnum::enDisableHandles);
				m_ruleEnabledMap.emplace(58,RulesDB::RulesEnabledDBEnum::enEnableLasing);
				m_ruleEnabledMap.emplace(59,RulesDB::RulesEnabledDBEnum::enPermissionMasterCommander);
				m_ruleEnabledMap.emplace(60,RulesDB::RulesEnabledDBEnum::enPermissionMasterGunner);
				m_ruleEnabledMap.emplace(61,RulesDB::RulesEnabledDBEnum::enPermissionMasterNone);
				m_ruleEnabledMap.emplace(62,RulesDB::RulesEnabledDBEnum::enEnableVMD);
				m_ruleEnabledMap.emplace(63,RulesDB::RulesEnabledDBEnum::enConditionForOpticButton);
				m_ruleEnabledMap.emplace(64,RulesDB::RulesEnabledDBEnum::enMasterCommander);
				m_ruleEnabledMap.emplace(65,RulesDB::RulesEnabledDBEnum::enMasterGunner);
				m_ruleEnabledMap.emplace(66,RulesDB::RulesEnabledDBEnum::enMasterNone);
			}
			void InitRuleExistenceMap()
			{
				m_ruleExistenceMap.emplace(0,RulesDB::RulesExistenceDBEnum::rConditionForLdDayBoreSight);
				m_ruleExistenceMap.emplace(1,RulesDB::RulesExistenceDBEnum::rConditionForLiDayBoreSight);
				m_ruleExistenceMap.emplace(2,RulesDB::RulesExistenceDBEnum::rConditionForThermalBoreSight);
				m_ruleExistenceMap.emplace(3,RulesDB::RulesExistenceDBEnum::rConditionForDrift);
				m_ruleExistenceMap.emplace(4,RulesDB::RulesExistenceDBEnum::rEndDriftFail);
				m_ruleExistenceMap.emplace(5,RulesDB::RulesExistenceDBEnum::rEndDriftSuccess);
				m_ruleExistenceMap.emplace(6,RulesDB::RulesExistenceDBEnum::rStartDrift);
				m_ruleExistenceMap.emplace(7,RulesDB::RulesExistenceDBEnum::rPalm_pressed);
				m_ruleExistenceMap.emplace(8,RulesDB::RulesExistenceDBEnum::rConditionForDayFovCalib);
				m_ruleExistenceMap.emplace(9,RulesDB::RulesExistenceDBEnum::rConditionForThermalFovCalib);
				m_ruleExistenceMap.emplace(10,RulesDB::RulesExistenceDBEnum::rEnable_General_Activities);
				m_ruleExistenceMap.emplace(11,RulesDB::RulesExistenceDBEnum::rActiveOperatorCommander);
				m_ruleExistenceMap.emplace(12,RulesDB::RulesExistenceDBEnum::rActiveOperatorGunner);
				m_ruleExistenceMap.emplace(13,RulesDB::RulesExistenceDBEnum::rActiveOperatorNone);
				m_ruleExistenceMap.emplace(14,RulesDB::RulesExistenceDBEnum::rBoresightToMaintenance);
				m_ruleExistenceMap.emplace(15,RulesDB::RulesExistenceDBEnum::rBoresightToOperational);
				m_ruleExistenceMap.emplace(16,RulesDB::RulesExistenceDBEnum::rCalibrationToMaintenance);
				m_ruleExistenceMap.emplace(17,RulesDB::RulesExistenceDBEnum::rCalibrationToOperational);
				m_ruleExistenceMap.emplace(18,RulesDB::RulesExistenceDBEnum::rIbitToMaintenance);
				m_ruleExistenceMap.emplace(19,RulesDB::RulesExistenceDBEnum::rIBitToOperational);
				m_ruleExistenceMap.emplace(20,RulesDB::RulesExistenceDBEnum::rInitToOperational);
				m_ruleExistenceMap.emplace(21,RulesDB::RulesExistenceDBEnum::rMaintenanceToBoresight);
				m_ruleExistenceMap.emplace(22,RulesDB::RulesExistenceDBEnum::rMaintenanceToCalibration);
				m_ruleExistenceMap.emplace(23,RulesDB::RulesExistenceDBEnum::rMaintenanceToIBit);
				m_ruleExistenceMap.emplace(24,RulesDB::RulesExistenceDBEnum::rMaintenanceToOperational);
				m_ruleExistenceMap.emplace(25,RulesDB::RulesExistenceDBEnum::rOperationalToMaintenance);
				m_ruleExistenceMap.emplace(26,RulesDB::RulesExistenceDBEnum::rChangeToManualMode);
				m_ruleExistenceMap.emplace(27,RulesDB::RulesExistenceDBEnum::rChangeToPowerMode);
				m_ruleExistenceMap.emplace(28,RulesDB::RulesExistenceDBEnum::rConditionForPowerMode);
				m_ruleExistenceMap.emplace(29,RulesDB::RulesExistenceDBEnum::rChangeToStabMode);
				m_ruleExistenceMap.emplace(30,RulesDB::RulesExistenceDBEnum::rConditionForStabMode);
				m_ruleExistenceMap.emplace(31,RulesDB::RulesExistenceDBEnum::rConditionForNuc);
				m_ruleExistenceMap.emplace(32,RulesDB::RulesExistenceDBEnum::rConditionForTracking);
				m_ruleExistenceMap.emplace(33,RulesDB::RulesExistenceDBEnum::rStartAutoTracking);
				m_ruleExistenceMap.emplace(34,RulesDB::RulesExistenceDBEnum::rStopAutoTrackingByFail);
				m_ruleExistenceMap.emplace(35,RulesDB::RulesExistenceDBEnum::rStopAutoTrackingByUser);
				m_ruleExistenceMap.emplace(36,RulesDB::RulesExistenceDBEnum::rBringToInitiateEnded);
				m_ruleExistenceMap.emplace(37,RulesDB::RulesExistenceDBEnum::rConditionForBringObsrvToGun);
				m_ruleExistenceMap.emplace(38,RulesDB::RulesExistenceDBEnum::rConditionForBringTo);
				m_ruleExistenceMap.emplace(39,RulesDB::RulesExistenceDBEnum::rEndBringToFail);
				m_ruleExistenceMap.emplace(40,RulesDB::RulesExistenceDBEnum::rEndBringToSuccess);
				m_ruleExistenceMap.emplace(41,RulesDB::RulesExistenceDBEnum::rStartBringTo);
				m_ruleExistenceMap.emplace(42,RulesDB::RulesExistenceDBEnum::rPerformLasing);
				m_ruleExistenceMap.emplace(43,RulesDB::RulesExistenceDBEnum::rWakeLasingActivity);
				m_ruleExistenceMap.emplace(44,RulesDB::RulesExistenceDBEnum::rConditionForLdEnable);
				m_ruleExistenceMap.emplace(45,RulesDB::RulesExistenceDBEnum::rConditionForLdPower);
				m_ruleExistenceMap.emplace(46,RulesDB::RulesExistenceDBEnum::rStartLdProcess);
				m_ruleExistenceMap.emplace(47,RulesDB::RulesExistenceDBEnum::rStopLdProcess);
				m_ruleExistenceMap.emplace(48,RulesDB::RulesExistenceDBEnum::rConditionForLiEnable);
				m_ruleExistenceMap.emplace(49,RulesDB::RulesExistenceDBEnum::rConditionForLiPower);
				m_ruleExistenceMap.emplace(50,RulesDB::RulesExistenceDBEnum::rStartLiProcess);
				m_ruleExistenceMap.emplace(51,RulesDB::RulesExistenceDBEnum::rStopLiProcess);
				m_ruleExistenceMap.emplace(52,RulesDB::RulesExistenceDBEnum::rConditionForScanning);
				m_ruleExistenceMap.emplace(53,RulesDB::RulesExistenceDBEnum::rEndScanningFail);
				m_ruleExistenceMap.emplace(54,RulesDB::RulesExistenceDBEnum::rEndScanningSuccess);
				m_ruleExistenceMap.emplace(55,RulesDB::RulesExistenceDBEnum::rScanningInitiateEnded);
				m_ruleExistenceMap.emplace(56,RulesDB::RulesExistenceDBEnum::rStartScanning);
				m_ruleExistenceMap.emplace(57,RulesDB::RulesExistenceDBEnum::rDisableHandles);
				m_ruleExistenceMap.emplace(58,RulesDB::RulesExistenceDBEnum::rEnableLasing);
				m_ruleExistenceMap.emplace(59,RulesDB::RulesExistenceDBEnum::rPermissionMasterCommander);
				m_ruleExistenceMap.emplace(60,RulesDB::RulesExistenceDBEnum::rPermissionMasterGunner);
				m_ruleExistenceMap.emplace(61,RulesDB::RulesExistenceDBEnum::rPermissionMasterNone);
				m_ruleExistenceMap.emplace(62,RulesDB::RulesExistenceDBEnum::rEnableVMD);
				m_ruleExistenceMap.emplace(63,RulesDB::RulesExistenceDBEnum::rConditionForOpticButton);
				m_ruleExistenceMap.emplace(64,RulesDB::RulesExistenceDBEnum::rMasterCommander);
				m_ruleExistenceMap.emplace(65,RulesDB::RulesExistenceDBEnum::rMasterGunner);
				m_ruleExistenceMap.emplace(66,RulesDB::RulesExistenceDBEnum::rMasterNone);
			}
			void BuildDatabases()
			{
				Database::Row row;

				// ===========================
				//		input
				//===========================
				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActSystemStatesStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActSystemStatesStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iCurrentSystemState, row))
					row.Write<StateMachine_SystemStates::SystemStates_StatesEnum>(StateMachine_SystemStates::SystemStates_StatesEnum::NO_STATE);
				else
					m_inputTable.AddRow<StateMachine_SystemStates::SystemStates_StatesEnum>(RulesDB::RulesInputDBEnum::iCurrentSystemState,StateMachine_SystemStates::SystemStates_StatesEnum::NO_STATE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iRequestSystemState, row))
					row.Write<StateMachine_SystemStates::SystemStates_StatesEnum>(StateMachine_SystemStates::SystemStates_StatesEnum::NO_STATE);
				else
					m_inputTable.AddRow<StateMachine_SystemStates::SystemStates_StatesEnum>(RulesDB::RulesInputDBEnum::iRequestSystemState,StateMachine_SystemStates::SystemStates_StatesEnum::NO_STATE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iEnterGeneral, row))
					row.Write<CommonDbDefs::EnabledEnum>(CommonDbDefs::EnabledEnum::ENABLED_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::EnabledEnum>(RulesDB::RulesInputDBEnum::iEnterGeneral,CommonDbDefs::EnabledEnum::ENABLED_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iShaftEncoderTrvStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iShaftEncoderTrvStatus,CommonDbDefs::StatusEnum::STATUS_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iShaftEncoderElvStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iShaftEncoderElvStatus,CommonDbDefs::StatusEnum::STATUS_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iDriverElvStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_OK);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iDriverElvStatus,CommonDbDefs::StatusEnum::STATUS_OK);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iDriverTrvStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_OK);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iDriverTrvStatus,CommonDbDefs::StatusEnum::STATUS_OK);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iGyroTrvStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iGyroTrvStatus,CommonDbDefs::StatusEnum::STATUS_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iGyroElvStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iGyroElvStatus,CommonDbDefs::StatusEnum::STATUS_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iVoltageStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iVoltageStatus,CommonDbDefs::StatusEnum::STATUS_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iCommutationStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iCommutationStatus,CommonDbDefs::StatusEnum::STATUS_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iMastOpened, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iMastOpened,CommonDbDefs::BoolEnum::BOOL_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iWorkModeRequest, row))
					row.Write<CommonDbDefs::ModeEnum>(CommonDbDefs::ModeEnum::MODE_MANUAL);
				else
					m_inputTable.AddRow<CommonDbDefs::ModeEnum>(RulesDB::RulesInputDBEnum::iWorkModeRequest,CommonDbDefs::ModeEnum::MODE_MANUAL);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActWorkModeStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActWorkModeStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iCurrentWorkMode, row))
					row.Write<CommonDbDefs::ModeEnum>(CommonDbDefs::ModeEnum::MODE_MANUAL);
				else
					m_inputTable.AddRow<CommonDbDefs::ModeEnum>(RulesDB::RulesInputDBEnum::iCurrentWorkMode,CommonDbDefs::ModeEnum::MODE_MANUAL);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iMobileStatus, row))
					row.Write<CommonDbDefs::MobileEnum>(CommonDbDefs::MobileEnum::MOBILE_FALSE);
				else
					m_inputTable.AddRow<CommonDbDefs::MobileEnum>(RulesDB::RulesInputDBEnum::iMobileStatus,CommonDbDefs::MobileEnum::MOBILE_FALSE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActTrackingStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActTrackingStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActScanningStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActScanningStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActBringToStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActBringToStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActBoreSightStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActBoreSightStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActNucStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActNucStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iDesignatorStatus, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_OFF);
				else
					m_inputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesInputDBEnum::iDesignatorStatus,CommonDbDefs::OnOffStateEnum::STATE_OFF);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iIlluminatorStatus, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_OFF);
				else
					m_inputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesInputDBEnum::iIlluminatorStatus,CommonDbDefs::OnOffStateEnum::STATE_OFF);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActDriftStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActDriftStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iDriftCompleted, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_FALSE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iDriftCompleted,CommonDbDefs::BoolEnum::BOOL_FALSE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandlePalmStatus, row))
					row.Write<CommonDbDefs::ButtonEnum>(CommonDbDefs::ButtonEnum::BUTTON_RELEASED);
				else
					m_inputTable.AddRow<CommonDbDefs::ButtonEnum>(RulesDB::RulesInputDBEnum::iHandlePalmStatus,CommonDbDefs::ButtonEnum::BUTTON_RELEASED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandlePalmInitiator, row))
					row.Write<CommonDbDefs::UserEnum>(CommonDbDefs::UserEnum::USER_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::UserEnum>(RulesDB::RulesInputDBEnum::iHandlePalmInitiator,CommonDbDefs::UserEnum::USER_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandleAnalogsInitiator, row))
					row.Write<CommonDbDefs::UserEnum>(CommonDbDefs::UserEnum::USER_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::UserEnum>(RulesDB::RulesInputDBEnum::iHandleAnalogsInitiator,CommonDbDefs::UserEnum::USER_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandleFovInitiator, row))
					row.Write<CommonDbDefs::UserEnum>(CommonDbDefs::UserEnum::USER_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::UserEnum>(RulesDB::RulesInputDBEnum::iHandleFovInitiator,CommonDbDefs::UserEnum::USER_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandleFovStatus, row))
					row.Write<CommonDbDefs::ButtonEnum>(CommonDbDefs::ButtonEnum::BUTTON_RELEASED);
				else
					m_inputTable.AddRow<CommonDbDefs::ButtonEnum>(RulesDB::RulesInputDBEnum::iHandleFovStatus,CommonDbDefs::ButtonEnum::BUTTON_RELEASED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandleTrackInitiator, row))
					row.Write<CommonDbDefs::UserEnum>(CommonDbDefs::UserEnum::USER_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::UserEnum>(RulesDB::RulesInputDBEnum::iHandleTrackInitiator,CommonDbDefs::UserEnum::USER_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandleTrackStatus, row))
					row.Write<CommonDbDefs::ButtonEnum>(CommonDbDefs::ButtonEnum::BUTTON_RELEASED);
				else
					m_inputTable.AddRow<CommonDbDefs::ButtonEnum>(RulesDB::RulesInputDBEnum::iHandleTrackStatus,CommonDbDefs::ButtonEnum::BUTTON_RELEASED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActControlStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActControlStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandleAnalogsElvAboveThreshold, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_FALSE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iHandleAnalogsElvAboveThreshold,CommonDbDefs::BoolEnum::BOOL_FALSE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandleAnalogsTrvAboveThreshold, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_FALSE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iHandleAnalogsTrvAboveThreshold,CommonDbDefs::BoolEnum::BOOL_FALSE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandleLaseInitiator, row))
					row.Write<CommonDbDefs::UserEnum>(CommonDbDefs::UserEnum::USER_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::UserEnum>(RulesDB::RulesInputDBEnum::iHandleLaseInitiator,CommonDbDefs::UserEnum::USER_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHandleLaseStatus, row))
					row.Write<CommonDbDefs::ButtonEnum>(CommonDbDefs::ButtonEnum::BUTTON_RELEASED);
				else
					m_inputTable.AddRow<CommonDbDefs::ButtonEnum>(RulesDB::RulesInputDBEnum::iHandleLaseStatus,CommonDbDefs::ButtonEnum::BUTTON_RELEASED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iProcessBringToInitiator, row))
					row.Write<CommonDbDefs::UserEnum>(CommonDbDefs::UserEnum::USER_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::UserEnum>(RulesDB::RulesInputDBEnum::iProcessBringToInitiator,CommonDbDefs::UserEnum::USER_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iProcessBringToStatus, row))
					row.Write<CommonDbDefs::UserRequestEnum>(CommonDbDefs::UserRequestEnum::USER_REQUEST_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::UserRequestEnum>(RulesDB::RulesInputDBEnum::iProcessBringToStatus,CommonDbDefs::UserRequestEnum::USER_REQUEST_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iControlBringToProcessStatus, row))
					row.Write<CommonDbDefs::ProcessEnum>(CommonDbDefs::ProcessEnum::PROCESS_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::ProcessEnum>(RulesDB::RulesInputDBEnum::iControlBringToProcessStatus,CommonDbDefs::ProcessEnum::PROCESS_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iBringToState, row))
					row.Write<StateMachine_BringTo::BringTo_StatesEnum>(StateMachine_BringTo::BringTo_StatesEnum::NO_STATE);
				else
					m_inputTable.AddRow<StateMachine_BringTo::BringTo_StatesEnum>(RulesDB::RulesInputDBEnum::iBringToState,StateMachine_BringTo::BringTo_StatesEnum::NO_STATE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iScanningState, row))
					row.Write<StateMachine_Scanning::Scanning_StatesEnum>(StateMachine_Scanning::Scanning_StatesEnum::NO_STATE);
				else
					m_inputTable.AddRow<StateMachine_Scanning::Scanning_StatesEnum>(RulesDB::RulesInputDBEnum::iScanningState,StateMachine_Scanning::Scanning_StatesEnum::NO_STATE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iControlScanningProcessStatus, row))
					row.Write<CommonDbDefs::ProcessEnum>(CommonDbDefs::ProcessEnum::PROCESS_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::ProcessEnum>(RulesDB::RulesInputDBEnum::iControlScanningProcessStatus,CommonDbDefs::ProcessEnum::PROCESS_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iProcessScanningInitiator, row))
					row.Write<CommonDbDefs::UserEnum>(CommonDbDefs::UserEnum::USER_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::UserEnum>(RulesDB::RulesInputDBEnum::iProcessScanningInitiator,CommonDbDefs::UserEnum::USER_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iProcessScanningStatus, row))
					row.Write<CommonDbDefs::UserRequestEnum>(CommonDbDefs::UserRequestEnum::USER_REQUEST_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::UserRequestEnum>(RulesDB::RulesInputDBEnum::iProcessScanningStatus,CommonDbDefs::UserRequestEnum::USER_REQUEST_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActLaseStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActLaseStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActCcdStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActCcdStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActTiStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActTiStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActVmdStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActVmdStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActReticleStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActReticleStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActVideoSwitchingStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActVideoSwitchingStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iAutoTrackingStatus, row))
					row.Write<CommonDbDefs::TrackerOperationEnum>(CommonDbDefs::TrackerOperationEnum::TRACKER_IS_IDLE);
				else
					m_inputTable.AddRow<CommonDbDefs::TrackerOperationEnum>(RulesDB::RulesInputDBEnum::iAutoTrackingStatus,CommonDbDefs::TrackerOperationEnum::TRACKER_IS_IDLE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActSlaStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActSlaStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iDayCamCommStatus, row))
					row.Write<CommonDbDefs::CommunicationEnum>(CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::CommunicationEnum>(RulesDB::RulesInputDBEnum::iDayCamCommStatus,CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iNightCamCommStatus, row))
					row.Write<CommonDbDefs::CommunicationEnum>(CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::CommunicationEnum>(RulesDB::RulesInputDBEnum::iNightCamCommStatus,CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLrfCamCommStatus, row))
					row.Write<CommonDbDefs::CommunicationEnum>(CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::CommunicationEnum>(RulesDB::RulesInputDBEnum::iLrfCamCommStatus,CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActSeCalibrationStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActSeCalibrationStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iDiscreteLiReadySwitch, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesInputDBEnum::iDiscreteLiReadySwitch,CommonDbDefs::OnOffStateEnum::STATE_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iDiscreteLiUnitStsSwitch, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesInputDBEnum::iDiscreteLiUnitStsSwitch,CommonDbDefs::OnOffStateEnum::STATE_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLiSelectStatus, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iLiSelectStatus,CommonDbDefs::BoolEnum::BOOL_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLdLiLasingPasswordSts, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_FALSE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iLdLiLasingPasswordSts,CommonDbDefs::BoolEnum::BOOL_FALSE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLiOnOffRequest, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_OFF);
				else
					m_inputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesInputDBEnum::iLiOnOffRequest,CommonDbDefs::OnOffStateEnum::STATE_OFF);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLdOnOffRequest, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_OFF);
				else
					m_inputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesInputDBEnum::iLdOnOffRequest,CommonDbDefs::OnOffStateEnum::STATE_OFF);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLrfOnOffRequest, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_OFF);
				else
					m_inputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesInputDBEnum::iLrfOnOffRequest,CommonDbDefs::OnOffStateEnum::STATE_OFF);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLiCommStatus, row))
					row.Write<CommonDbDefs::CommunicationEnum>(CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::CommunicationEnum>(RulesDB::RulesInputDBEnum::iLiCommStatus,CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLdCommStatus, row))
					row.Write<CommonDbDefs::CommunicationEnum>(CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::CommunicationEnum>(RulesDB::RulesInputDBEnum::iLdCommStatus,CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iCommanderSelectedSys, row))
					row.Write<ProjectCommonDbDefs::SelectedSystemEnum>(ProjectCommonDbDefs::SelectedSystemEnum::SYSTEM_OBSERVATION);
				else
					m_inputTable.AddRow<ProjectCommonDbDefs::SelectedSystemEnum>(RulesDB::RulesInputDBEnum::iCommanderSelectedSys,ProjectCommonDbDefs::SelectedSystemEnum::SYSTEM_OBSERVATION);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iGunnerSelectedSys, row))
					row.Write<ProjectCommonDbDefs::SelectedSystemEnum>(ProjectCommonDbDefs::SelectedSystemEnum::SYSTEM_ORCWS);
				else
					m_inputTable.AddRow<ProjectCommonDbDefs::SelectedSystemEnum>(RulesDB::RulesInputDBEnum::iGunnerSelectedSys,ProjectCommonDbDefs::SelectedSystemEnum::SYSTEM_ORCWS);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iHatchesCloseSts, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_FALSE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iHatchesCloseSts,CommonDbDefs::BoolEnum::BOOL_FALSE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLiLdLasingSwitch, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_OFF);
				else
					m_inputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesInputDBEnum::iLiLdLasingSwitch,CommonDbDefs::OnOffStateEnum::STATE_OFF);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLdNormalTemperature, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_FALSE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iLdNormalTemperature,CommonDbDefs::BoolEnum::BOOL_FALSE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iGunShaftEncoderElvStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iGunShaftEncoderElvStatus,CommonDbDefs::StatusEnum::STATUS_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iGunShaftEncoderTrvStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iGunShaftEncoderTrvStatus,CommonDbDefs::StatusEnum::STATUS_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iNightCamReady, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_FALSE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iNightCamReady,CommonDbDefs::BoolEnum::BOOL_FALSE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActFusionStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActFusionStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iBoreSightType, row))
					row.Write<ProjectCommonDbDefs::BoreSightTypeEnum>(ProjectCommonDbDefs::BoreSightTypeEnum::BORE_SIGHT_TYPE_NONE);
				else
					m_inputTable.AddRow<ProjectCommonDbDefs::BoreSightTypeEnum>(RulesDB::RulesInputDBEnum::iBoreSightType,ProjectCommonDbDefs::BoreSightTypeEnum::BORE_SIGHT_TYPE_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iTrackingUser, row))
					row.Write<CommonDbDefs::UserEnum>(CommonDbDefs::UserEnum::USER_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::UserEnum>(RulesDB::RulesInputDBEnum::iTrackingUser,CommonDbDefs::UserEnum::USER_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iSlaCommStatus, row))
					row.Write<CommonDbDefs::CommunicationEnum>(CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::CommunicationEnum>(RulesDB::RulesInputDBEnum::iSlaCommStatus,CommonDbDefs::CommunicationEnum::COMMUNICATION_NONE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iTrackingCamChange, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_FALSE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iTrackingCamChange,CommonDbDefs::BoolEnum::BOOL_FALSE);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iStabErrorGroupStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_OK);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iStabErrorGroupStatus,CommonDbDefs::StatusEnum::STATUS_OK);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iPowerErrorGroupStatus, row))
					row.Write<CommonDbDefs::StatusEnum>(CommonDbDefs::StatusEnum::STATUS_OK);
				else
					m_inputTable.AddRow<CommonDbDefs::StatusEnum>(RulesDB::RulesInputDBEnum::iPowerErrorGroupStatus,CommonDbDefs::StatusEnum::STATUS_OK);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iTrackingCam, row))
					row.Write<ProjectCommonDbDefs::CamerasEnum>(ProjectCommonDbDefs::CamerasEnum::NUM_OF_CAMERAS);
				else
					m_inputTable.AddRow<ProjectCommonDbDefs::CamerasEnum>(RulesDB::RulesInputDBEnum::iTrackingCam,ProjectCommonDbDefs::CamerasEnum::NUM_OF_CAMERAS);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iActFovCalibStatus, row))
					row.Write<CommonDbDefs::ActivityStatusEnum>(CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);
				else
					m_inputTable.AddRow<CommonDbDefs::ActivityStatusEnum>(RulesDB::RulesInputDBEnum::iActFovCalibStatus,CommonDbDefs::ActivityStatusEnum::ACTIVITY_SUSPENDED);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iLiBsOnOffRequest, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_OFF);
				else
					m_inputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesInputDBEnum::iLiBsOnOffRequest,CommonDbDefs::OnOffStateEnum::STATE_OFF);

				if (m_inputTable.TryGet(RulesDB::RulesInputDBEnum::iMastClosed, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_NONE);
				else
					m_inputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesInputDBEnum::iMastClosed,CommonDbDefs::BoolEnum::BOOL_NONE);

				// ===========================
				//		output
				//===========================
				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActSystemStatesCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActSystemStatesCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oGoToSystemState, row))
					row.Write<StateMachine_SystemStates::SystemStates_StatesEnum>(StateMachine_SystemStates::SystemStates_StatesEnum::NO_STATE);
				else
					m_outputTable.AddRow<StateMachine_SystemStates::SystemStates_StatesEnum>(RulesDB::RulesOutputDBEnum::oGoToSystemState,StateMachine_SystemStates::SystemStates_StatesEnum::NO_STATE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oChangeWorkMode, row))
					row.Write<CommonDbDefs::ModeEnum>(CommonDbDefs::ModeEnum::MODE_MANUAL);
				else
					m_outputTable.AddRow<CommonDbDefs::ModeEnum>(RulesDB::RulesOutputDBEnum::oChangeWorkMode,CommonDbDefs::ModeEnum::MODE_MANUAL);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActWorkModeCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActWorkModeCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oPalmConditionForDrift, row))
					row.Write<CommonDbDefs::ConditionEnum>(CommonDbDefs::ConditionEnum::CONDITION_YES);
				else
					m_outputTable.AddRow<CommonDbDefs::ConditionEnum>(RulesDB::RulesOutputDBEnum::oPalmConditionForDrift,CommonDbDefs::ConditionEnum::CONDITION_YES);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActDriftCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActDriftCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActiveOperator, row))
					row.Write<CommonDbDefs::UserEnum>(CommonDbDefs::UserEnum::USER_NONE);
				else
					m_outputTable.AddRow<CommonDbDefs::UserEnum>(RulesDB::RulesOutputDBEnum::oActiveOperator,CommonDbDefs::UserEnum::USER_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oCurrentMaster, row))
					row.Write<CommonDbDefs::UserEnum>(CommonDbDefs::UserEnum::USER_NONE);
				else
					m_outputTable.AddRow<CommonDbDefs::UserEnum>(RulesDB::RulesOutputDBEnum::oCurrentMaster,CommonDbDefs::UserEnum::USER_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oHandlePalmPermission, row))
					row.Write<UiDB::PermissionByUserEnum>(UiDB::PermissionByUserEnum::PERMISSION_NONE);
				else
					m_outputTable.AddRow<UiDB::PermissionByUserEnum>(RulesDB::RulesOutputDBEnum::oHandlePalmPermission,UiDB::PermissionByUserEnum::PERMISSION_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oHandleAnalogsPermission, row))
					row.Write<UiDB::PermissionByUserEnum>(UiDB::PermissionByUserEnum::PERMISSION_NONE);
				else
					m_outputTable.AddRow<UiDB::PermissionByUserEnum>(RulesDB::RulesOutputDBEnum::oHandleAnalogsPermission,UiDB::PermissionByUserEnum::PERMISSION_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oHandleFovPermission, row))
					row.Write<UiDB::PermissionByUserEnum>(UiDB::PermissionByUserEnum::PERMISSION_NONE);
				else
					m_outputTable.AddRow<UiDB::PermissionByUserEnum>(RulesDB::RulesOutputDBEnum::oHandleFovPermission,UiDB::PermissionByUserEnum::PERMISSION_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oHandleTrackPermission, row))
					row.Write<UiDB::PermissionByUserEnum>(UiDB::PermissionByUserEnum::PERMISSION_NONE);
				else
					m_outputTable.AddRow<UiDB::PermissionByUserEnum>(RulesDB::RulesOutputDBEnum::oHandleTrackPermission,UiDB::PermissionByUserEnum::PERMISSION_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActControlCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActControlCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oHandleLasePermission, row))
					row.Write<UiDB::PermissionByUserEnum>(UiDB::PermissionByUserEnum::PERMISSION_NONE);
				else
					m_outputTable.AddRow<UiDB::PermissionByUserEnum>(RulesDB::RulesOutputDBEnum::oHandleLasePermission,UiDB::PermissionByUserEnum::PERMISSION_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActBringToCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActBringToCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oGoToBringToState, row))
					row.Write<StateMachine_BringTo::BringTo_StatesEnum>(StateMachine_BringTo::BringTo_StatesEnum::NO_STATE);
				else
					m_outputTable.AddRow<StateMachine_BringTo::BringTo_StatesEnum>(RulesDB::RulesOutputDBEnum::oGoToBringToState,StateMachine_BringTo::BringTo_StatesEnum::NO_STATE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oProcessBringToPermission, row))
					row.Write<UiDB::PermissionByUserEnum>(UiDB::PermissionByUserEnum::PERMISSION_NONE);
				else
					m_outputTable.AddRow<UiDB::PermissionByUserEnum>(RulesDB::RulesOutputDBEnum::oProcessBringToPermission,UiDB::PermissionByUserEnum::PERMISSION_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActScanningCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActScanningCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oGoToScanningState, row))
					row.Write<StateMachine_Scanning::Scanning_StatesEnum>(StateMachine_Scanning::Scanning_StatesEnum::NO_STATE);
				else
					m_outputTable.AddRow<StateMachine_Scanning::Scanning_StatesEnum>(RulesDB::RulesOutputDBEnum::oGoToScanningState,StateMachine_Scanning::Scanning_StatesEnum::NO_STATE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActLaseCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActLaseCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActCcdCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActCcdCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActTiCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActTiCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActVmdCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActVmdCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActReticleCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActReticleCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActVideoSwitchingCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActVideoSwitchingCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActAutoTrackingCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActAutoTrackingCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oAutoTrackingRequest, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_NONE);
				else
					m_outputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesOutputDBEnum::oAutoTrackingRequest,CommonDbDefs::OnOffStateEnum::STATE_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActSlaCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActSlaCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oBringToPermission, row))
					row.Write<UiDB::PermissionByUserEnum>(UiDB::PermissionByUserEnum::PERMISSION_NONE);
				else
					m_outputTable.AddRow<UiDB::PermissionByUserEnum>(RulesDB::RulesOutputDBEnum::oBringToPermission,UiDB::PermissionByUserEnum::PERMISSION_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oScanningPermission, row))
					row.Write<UiDB::PermissionByUserEnum>(UiDB::PermissionByUserEnum::PERMISSION_NONE);
				else
					m_outputTable.AddRow<UiDB::PermissionByUserEnum>(RulesDB::RulesOutputDBEnum::oScanningPermission,UiDB::PermissionByUserEnum::PERMISSION_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActSeCalibrationCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActSeCalibrationCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActBoreSightCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActBoreSightCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oLiLasingRequest, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_OFF);
				else
					m_outputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesOutputDBEnum::oLiLasingRequest,CommonDbDefs::OnOffStateEnum::STATE_OFF);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oLdLasingRequest, row))
					row.Write<CommonDbDefs::OnOffStateEnum>(CommonDbDefs::OnOffStateEnum::STATE_OFF);
				else
					m_outputTable.AddRow<CommonDbDefs::OnOffStateEnum>(RulesDB::RulesOutputDBEnum::oLdLasingRequest,CommonDbDefs::OnOffStateEnum::STATE_OFF);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oLiPowerEnableCmd, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_FALSE);
				else
					m_outputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesOutputDBEnum::oLiPowerEnableCmd,CommonDbDefs::BoolEnum::BOOL_FALSE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oLdPowerEnableCmd, row))
					row.Write<CommonDbDefs::BoolEnum>(CommonDbDefs::BoolEnum::BOOL_FALSE);
				else
					m_outputTable.AddRow<CommonDbDefs::BoolEnum>(RulesDB::RulesOutputDBEnum::oLdPowerEnableCmd,CommonDbDefs::BoolEnum::BOOL_FALSE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oLiProcessEnableCmd, row))
					row.Write<CommonDbDefs::EnabledEnum>(CommonDbDefs::EnabledEnum::ENABLED_FALSE);
				else
					m_outputTable.AddRow<CommonDbDefs::EnabledEnum>(RulesDB::RulesOutputDBEnum::oLiProcessEnableCmd,CommonDbDefs::EnabledEnum::ENABLED_FALSE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oLdProcessEnableCmd, row))
					row.Write<CommonDbDefs::EnabledEnum>(CommonDbDefs::EnabledEnum::ENABLED_FALSE);
				else
					m_outputTable.AddRow<CommonDbDefs::EnabledEnum>(RulesDB::RulesOutputDBEnum::oLdProcessEnableCmd,CommonDbDefs::EnabledEnum::ENABLED_FALSE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oConditionsForBringObsrvToGun, row))
					row.Write<CommonDbDefs::ConditionEnum>(CommonDbDefs::ConditionEnum::CONDITION_NO);
				else
					m_outputTable.AddRow<CommonDbDefs::ConditionEnum>(RulesDB::RulesOutputDBEnum::oConditionsForBringObsrvToGun,CommonDbDefs::ConditionEnum::CONDITION_NO);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActFusionCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActFusionCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oVmdPermission, row))
					row.Write<UiDB::PermissionByUserEnum>(UiDB::PermissionByUserEnum::PERMISSION_NONE);
				else
					m_outputTable.AddRow<UiDB::PermissionByUserEnum>(RulesDB::RulesOutputDBEnum::oVmdPermission,UiDB::PermissionByUserEnum::PERMISSION_NONE);

				if (m_outputTable.TryGet(RulesDB::RulesOutputDBEnum::oActFovCalibCmd, row))
					row.Write<CommonDbDefs::ActivityCmdEnum>(CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);
				else
					m_outputTable.AddRow<CommonDbDefs::ActivityCmdEnum>(RulesDB::RulesOutputDBEnum::oActFovCalibCmd,CommonDbDefs::ActivityCmdEnum::ACTIVITY_SUSPEND);

				// ===========================
				//		result
				//===========================
				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForLdDayBoreSight, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForLdDayBoreSight,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForLiDayBoreSight, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForLiDayBoreSight,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForThermalBoreSight, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForThermalBoreSight,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForDrift, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForDrift,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rEndDriftFail, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rEndDriftFail,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rEndDriftSuccess, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rEndDriftSuccess,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rStartDrift, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rStartDrift,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rPalm_pressed, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rPalm_pressed,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForDayFovCalib, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForDayFovCalib,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForThermalFovCalib, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForThermalFovCalib,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rEnable_General_Activities, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rEnable_General_Activities,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rActiveOperatorCommander, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rActiveOperatorCommander,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rActiveOperatorGunner, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rActiveOperatorGunner,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rActiveOperatorNone, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rActiveOperatorNone,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rBoresightToMaintenance, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rBoresightToMaintenance,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rBoresightToOperational, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rBoresightToOperational,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rCalibrationToMaintenance, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rCalibrationToMaintenance,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rCalibrationToOperational, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rCalibrationToOperational,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rIbitToMaintenance, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rIbitToMaintenance,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rIBitToOperational, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rIBitToOperational,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rInitToOperational, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rInitToOperational,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rMaintenanceToBoresight, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rMaintenanceToBoresight,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rMaintenanceToCalibration, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rMaintenanceToCalibration,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rMaintenanceToIBit, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rMaintenanceToIBit,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rMaintenanceToOperational, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rMaintenanceToOperational,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rOperationalToMaintenance, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rOperationalToMaintenance,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rChangeToManualMode, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rChangeToManualMode,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rChangeToPowerMode, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rChangeToPowerMode,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForPowerMode, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForPowerMode,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rChangeToStabMode, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rChangeToStabMode,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForStabMode, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForStabMode,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForNuc, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForNuc,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForTracking, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForTracking,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rStartAutoTracking, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rStartAutoTracking,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rStopAutoTrackingByFail, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rStopAutoTrackingByFail,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rStopAutoTrackingByUser, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rStopAutoTrackingByUser,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rBringToInitiateEnded, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rBringToInitiateEnded,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForBringObsrvToGun, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForBringObsrvToGun,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForBringTo, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForBringTo,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rEndBringToFail, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rEndBringToFail,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rEndBringToSuccess, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rEndBringToSuccess,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rStartBringTo, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rStartBringTo,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rPerformLasing, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rPerformLasing,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rWakeLasingActivity, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rWakeLasingActivity,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForLdEnable, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForLdEnable,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForLdPower, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForLdPower,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rStartLdProcess, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rStartLdProcess,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rStopLdProcess, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rStopLdProcess,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForLiEnable, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForLiEnable,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForLiPower, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForLiPower,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rStartLiProcess, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rStartLiProcess,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rStopLiProcess, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rStopLiProcess,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForScanning, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForScanning,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rEndScanningFail, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rEndScanningFail,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rEndScanningSuccess, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rEndScanningSuccess,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rScanningInitiateEnded, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rScanningInitiateEnded,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rStartScanning, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rStartScanning,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rDisableHandles, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rDisableHandles,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rEnableLasing, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rEnableLasing,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rPermissionMasterCommander, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rPermissionMasterCommander,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rPermissionMasterGunner, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rPermissionMasterGunner,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rPermissionMasterNone, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rPermissionMasterNone,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rEnableVMD, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rEnableVMD,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rConditionForOpticButton, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rConditionForOpticButton,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rMasterCommander, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rMasterCommander,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rMasterGunner, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rMasterGunner,RulesDefs::RulesExistence::RULE_NOT_VALID);

				if (m_existenceTable.TryGet(RulesDB::RulesExistenceDBEnum::rMasterNone, row))
					row.Write<RulesDefs::RulesExistence>(RulesDefs::RulesExistence::RULE_NOT_VALID);
				else
					m_existenceTable.AddRow<RulesDefs::RulesExistence>(RulesDB::RulesExistenceDBEnum::rMasterNone,RulesDefs::RulesExistence::RULE_NOT_VALID);

				// ===========================
				//		enabled
				//===========================
				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForLdDayBoreSight, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForLdDayBoreSight,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForLiDayBoreSight, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForLiDayBoreSight,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForThermalBoreSight, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForThermalBoreSight,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForDrift, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForDrift,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enEndDriftFail, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enEndDriftFail,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enEndDriftSuccess, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enEndDriftSuccess,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enStartDrift, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enStartDrift,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enPalm_pressed, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enPalm_pressed,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForDayFovCalib, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForDayFovCalib,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForThermalFovCalib, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForThermalFovCalib,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enEnable_General_Activities, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enEnable_General_Activities,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enActiveOperatorCommander, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enActiveOperatorCommander,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enActiveOperatorGunner, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enActiveOperatorGunner,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enActiveOperatorNone, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enActiveOperatorNone,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enBoresightToMaintenance, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enBoresightToMaintenance,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enBoresightToOperational, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enBoresightToOperational,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enCalibrationToMaintenance, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enCalibrationToMaintenance,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enCalibrationToOperational, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enCalibrationToOperational,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enIbitToMaintenance, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enIbitToMaintenance,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enIBitToOperational, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enIBitToOperational,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enInitToOperational, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enInitToOperational,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enMaintenanceToBoresight, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enMaintenanceToBoresight,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enMaintenanceToCalibration, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enMaintenanceToCalibration,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enMaintenanceToIBit, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enMaintenanceToIBit,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enMaintenanceToOperational, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enMaintenanceToOperational,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enOperationalToMaintenance, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enOperationalToMaintenance,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enChangeToManualMode, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enChangeToManualMode,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enChangeToPowerMode, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enChangeToPowerMode,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForPowerMode, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForPowerMode,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enChangeToStabMode, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enChangeToStabMode,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForStabMode, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForStabMode,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForNuc, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForNuc,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForTracking, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForTracking,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enStartAutoTracking, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enStartAutoTracking,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enStopAutoTrackingByFail, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enStopAutoTrackingByFail,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enStopAutoTrackingByUser, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enStopAutoTrackingByUser,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enBringToInitiateEnded, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enBringToInitiateEnded,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForBringObsrvToGun, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForBringObsrvToGun,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForBringTo, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForBringTo,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enEndBringToFail, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enEndBringToFail,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enEndBringToSuccess, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enEndBringToSuccess,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enStartBringTo, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enStartBringTo,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enPerformLasing, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enPerformLasing,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enWakeLasingActivity, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enWakeLasingActivity,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForLdEnable, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForLdEnable,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForLdPower, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForLdPower,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enStartLdProcess, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enStartLdProcess,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enStopLdProcess, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enStopLdProcess,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForLiEnable, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForLiEnable,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForLiPower, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForLiPower,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enStartLiProcess, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enStartLiProcess,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enStopLiProcess, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enStopLiProcess,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForScanning, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForScanning,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enEndScanningFail, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enEndScanningFail,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enEndScanningSuccess, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enEndScanningSuccess,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enScanningInitiateEnded, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enScanningInitiateEnded,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enStartScanning, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enStartScanning,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enDisableHandles, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enDisableHandles,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enEnableLasing, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enEnableLasing,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enPermissionMasterCommander, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enPermissionMasterCommander,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enPermissionMasterGunner, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enPermissionMasterGunner,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enPermissionMasterNone, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enPermissionMasterNone,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enEnableVMD, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enEnableVMD,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enConditionForOpticButton, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enConditionForOpticButton,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enMasterCommander, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enMasterCommander,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enMasterGunner, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enMasterGunner,RulesDefs::RulesEnabled::RULE_ENABLE);

				if (m_enableTable.TryGet(RulesDB::RulesEnabledDBEnum::enMasterNone, row))
					row.Write<RulesDefs::RulesEnabled>(RulesDefs::RulesEnabled::RULE_ENABLE);
				else
					m_enableTable.AddRow<RulesDefs::RulesEnabled>(RulesDB::RulesEnabledDBEnum::enMasterNone,RulesDefs::RulesEnabled::RULE_ENABLE);

				// ===========================
				//		management
				//===========================
				if (m_managementDB.TryGet(RulesDB::RulesManagementDBEnum::RELOAD_RULES, row))
					row.Write<bool>(false);
				else
					m_managementDB.AddRow<bool>(RulesDB::RulesManagementDBEnum::RELOAD_RULES);
			}
	};
};


	bool rules::rules_data_and_types::create(
		core::database::dataset_interface* dataset,
		core::database::table_interface* inputTable,
		core::database::table_interface* enableTable,
		core::database::table_interface* existenceTable,
		core::database::table_interface* outputTable,
		core::database::table_interface* managementTable,
		core::rules::rules_data_and_types_interface** _rules_data_and_types)
	{
		if (dataset == nullptr ||inputTable == nullptr ||
			enableTable == nullptr ||
			existenceTable == nullptr ||
			outputTable == nullptr ||
			managementTable == nullptr ||
			_rules_data_and_types == nullptr)
		{
			return false;
		}

		utils::ref_count_ptr<core::rules::rules_data_and_types_interface> instance;
		try
		{
		instance = utils::make_ref_count_ptr<Rules::rules_data_and_types_impl>(
			dataset,
			inputTable,
			enableTable,
			existenceTable,
			outputTable,
			managementTable);
		}
		catch (...)
		{
			return false;
		}

		if (instance == nullptr)
			return false;

		instance->add_ref();
		*_rules_data_and_types = instance;
		return true;
	}
