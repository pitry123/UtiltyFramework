#ifndef _OPERATIONAL_ACTIVITIES_LOGIC_DB_H_
#define _OPERATIONAL_ACTIVITIES_LOGIC_DB_H_

class OperationalActivitiesLogicDB
{
public:
	
	enum OperationalActivitiesLogic_StatusDBEnum
	{
		BRING_TO_STATE_STS,								// StateMachine_BringTo::BringTo_StatesEnum		 t:int
		BRING_TO_COORDINATES_STS,						// t:CommonDbDefs::CoordinatesStruct	
		BRING_TO_OUT_OF_LIMITS_STS,						// CommonDbDefs::BoolEnum						 t:int
		SCANNING_STATE_STS,								// StateMachine_Scanning::Scanning_StatesEnum	 t:int
		SCANNING_COORDINATES_START_STS,					// t:CommonDbDefs::CoordinatesStruct	
		SCANNING_COORDINATES_END_STS,					// t:CommonDbDefs::CoordinatesStruct	
		SCANNING_OUT_OF_LIMITS_STS,						// CommonDbDefs::BoolEnum						 t:int
		SCANNING_VELOCITY_STS,							// t:float
		SYSTEM_RANGE_STS,								// t:ProjectCommonDbDefs::SystemRangeStruct
		AUTO_TRACKING_STS,								// CommonDbDefs::TrackerOperationEnum			 t:int
		LI_LASING_EXECUTION_STS,						// ProjectCommonDbDefs::ProcessStatusUpdatesEnum t:int
		LI_LASING_ENABLE_STS,							// CommonDbDefs::EnabledEnum					 t:int
		LI_LASING_SELECTED_STS,							// CommonDbDefs::BoolEnum						 t:int
		LD_LASING_EXECUTION_STS,						// ProjectCommonDbDefs::ProcessStatusUpdatesEnum t:int
		LD_LASING_ENABLE_STS,							// CommonDbDefs::EnabledEnum					 t:int
		LD_LASING_SELECTED_STS,							// CommonDbDefs::BoolEnum						 t:int

		OPERATIONAL_ACTIVITIES_LOGIC_STATUS_DB_SIZE
	};


	enum OperationalActivitiesLogic_CmdDBEnum
	{
		
		OPERATIONAL_ACTIVITIES_LOGIC_CMD_DB_SIZE
	};

	enum OperationalActivityLogic_ErrorDBEnum
	{
		
		OPERATIONAL_ACTIVITY_LOGIC_ERROR_DB_SIZE
	};

	

};

#endif // _OPERATIONAL_ACTIVITIES_LOGIC_DB_H_
