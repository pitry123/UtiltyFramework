#ifndef _SCANNING_STATEMACHINE_H_
#define _SCANNING_STATEMACHINE_H_

// ----------------------------------------------------------------------------------------------//
// ----------------------------------------------------------------------------------------------//
// - 
// -                    StateMachine - Scanning
//
// ----------------------------------------------------------------------------------------------//
// ----------------------------------------------------------------------------------------------//
class StateMachine_Scanning
{
public:	

	enum Scanning_StatesEnum
	{
		NO_STATE					= -1, // A value for -1 must be used
		STATE_SCANNING_INITIATE,
		STATE_SCANNING_SCANNING,
		STATE_SCANNING_END_SUCCESS,
		STATE_SCANNING_END_FAIL,
		
		NUM_OF_SCANNING
	};

	//-------------------------------------//
	//      Transitions in State Machine   //
	//-------------------------------------//
	enum Scanning_TransitionsEnum
	{
		NO_TRANSITION		= -1, // A value for -1 must be used
		GO_TO_SCANNING,
		GO_TO_END_SUCCESS,
		GO_TO_END_FAIL
	};

	enum Scanning_StatesTimeouts // mili
	{
		
	};

	/*---- For Debug Environment ----//
	struct StateMachineEntryStruct_ActivityScanning  
	{
		ProcessStateMachine::StateMachineStatusEnum					nIsValid;			// t:int
		StateMachine::PROCESS_STATUS								nProcessStatus;		// t:int
		ScanningActivity::ScanningActivityOpcodesEnum				nLastOpcode;		// t:int
		StateMachine_Scanning::Scanning_TransitionsEnum				nLastTransition;	// t:int
		StateMachine_Scanning::Scanning_StatesEnum					nCurrentState;		// t:int
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

#endif // !defined(_SCANNING_STATEMACHINE_H_)

