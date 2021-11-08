#ifndef _BRING_TO_STATEMACHINE_H_
#define _BRING_TO_STATEMACHINE_H_

// ----------------------------------------------------------------------------------------------//
// ----------------------------------------------------------------------------------------------//
// - 
// -                    StateMachine - Bring To
// - 
// ----------------------------------------------------------------------------------------------//
// ----------------------------------------------------------------------------------------------//
class StateMachine_BringTo 
{
public:	

	enum BringTo_StatesEnum
	{
		NO_STATE					= -1, // A value for -1 must be used
		STATE_BRING_TO_INITIATE,
		STATE_BRING_TO_BRING_TO,
		STATE_BRING_TO_END_SUCCESS,
		STATE_BRING_TO_END_FAIL,
		
		NUM_OF_BRING_TO
	};

	//-------------------------------------//
	//      Transitions in State Machine   //
	//-------------------------------------//
	enum BringTo_TransitionsEnum
	{
		NO_TRANSITION		= -1, // A value for -1 must be used
		GO_TO_BRING_TO,
		GO_TO_END_SUCCESS,
		GO_TO_END_FAIL
	};

	enum BringTo_StatesTimeouts // mili
	{
		
	};

	/*---- For Debug Environment ----//
	struct StateMachineEntryStruct_ActivityBringTo  
	{
		ProcessStateMachine::StateMachineStatusEnum					nIsValid;			// t:int
		StateMachine::PROCESS_STATUS								nProcessStatus;		// t:int
		BringToActivity::BringToActivityOpcodesEnum					nLastOpcode;		// t:int
		StateMachine_BringTo::BringTo_TransitionsEnum				nLastTransition;	// t:int
		StateMachine_BringTo::BringTo_StatesEnum					nCurrentState;		// t:int
		ProcessStateMachine::TimeoutStatusEnum						nIsTimeout;			// t:int
		CommonDbDefs::OperatorTypesEnum								nOperator;			// t:int
	};*/

};

#endif // !defined(_BRING_TO_STATEMACHINE_H_)

