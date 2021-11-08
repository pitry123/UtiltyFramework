#ifndef _SYSTEM_STATES_STATEMACHINE_H_
#define _SYSTEM_STATES_STATEMACHINE_H_

//#include <Generic/Infrastructure/infrastructure_ProcessStateMachine.h>
//#include <Common/ProjectCommonDbDefs.h>
//#include <Generic/Infrastructure/infrastructure_EnumConvert.h>


// ----------------------------------------------------------------------------------------------//
// ----------------------------------------------------------------------------------------------//
// - 
// -                    StateMachine - System States
// - 
// ----------------------------------------------------------------------------------------------//
// ----------------------------------------------------------------------------------------------//
class StateMachine_SystemStates 
{
public:	

	enum SystemStates_StatesEnum
	{
		NO_STATE					= -1, // A value for -1 must be used
		STATE_INITIALIZATION,
		STATE_OPERATIONAL,
		STATE_BORESIGHT,
		STATE_CALIBRATION,
		STATE_IBIT,
		STATE_MAINTENANCE,

		NUM_OF_SYSTEM_STATES
	};

	//-------------------------------------//
	//      Transitions in State Machine   //
	//-------------------------------------//
	enum SystemStates_TransitionsEnum
	{
		NO_TRANSITION		= -1, // A value for -1 must be used
		GO_TO_INITIALIZATION,
		GO_TO_OPERATIONAL,
		GO_TO_BORESIGHT,
		GO_TO_CALIBRATION,
		GO_TO_IBIT,
		GO_TO_MAINTENANCE,

		NUM_OF_SYSTEM_STATES_TRANSITION
	};

	enum SystemStates_StatesTimeouts // mili
	{
		SYSTEM_INIT_TIMEOUT				= 3500
	};

	/*---- For Debug Environment ----//
	struct StateMachineEntryStruct_ActivitySystemStates  
	{
		ProcessStateMachine::StateMachineStatusEnum					nIsValid;			// t:int
		StateMachine::PROCESS_STATUS								nProcessStatus;		// t:int
		SystemStatesActivity::SystemStatesActivityOpcodesEnum				nLastOpcode;		// t:int
		StateMachine_SystemStates::SystemStates_TransitionsEnum		nLastTransition;	// t:int
		StateMachine_SystemStates::SystemStates_StatesEnum			nCurrentState;		// t:int
		ProcessStateMachine::TimeoutStatusEnum						nIsTimeout;			// t:int
		CommonDbDefs::OperatorTypesEnum								nOperator;			// t:int
	};*/

private:

};


// ----------------------------------------------------------------------------------------------//
// ----------------------------------------------------------------------------------------------//
// -																							 //
// -                   States																	 //
// ----------------------------------------------------------------------------------------------//
// ----------------------------------------------------------------------------------------------//

//---------------- [PROGRAMS_STATE_BASE] -----------------------


#endif // !defined(_SYSTEM_STATES_STATEMACHINE_H_)

