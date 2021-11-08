#pragma once
#include <StateMachine.hpp>

class StateMachineLogic : public StateMachine
{
public:
	//--------------------------------------------------------------------
	//State machine states
	//--------------------------------------------------------------------	
	enum StateMachineLogic_StateEnum
	{
		NO_STATE = -1,
		STATE_FIRST,
		STATE_SECOND,
		STATE_THIRD,

		NUM_OF_EXAMPLE_STATES
	};
	//--------------------------------------------------------------------
	//State machine transitions
	//--------------------------------------------------------------------
	enum StateMachineLogic_TransitionsEnum
	{
		NO_TRANSITION = -1,           // A value for -1 must be used
		GO_TO_FIRST,
		GO_TO_SECOND,
		GO_TO_THIRD
	};	

	StateMachineLogic(
		const Database::Subscriber& subscriber, 
		Database::DataSet& dataset, 
		const Database::Row& rowStateMachineSts = nullptr);

	~StateMachineLogic();

	/*---- For Debug Environment ----//
	struct StateMachineEntryStruct_StateMachineLogic
	{
	StateMachine::StateMachineStatusEnum					   nIsValid;	     //t:int
	int											               nProcessIndex;
	StateMachineLogic::StateMachineLogic_TransitionsEnum	   nLastOpcode;		//t:int	 
	int														   nLastTransition;	
	StateMachineLogic::StateMachineLogic_StateEnum             nCurrentState;	 //t:int
	StateMachine::TimeoutStatusEnum							   nIsTimeout;		 //t:int
	};*/

protected:
	virtual void HandleStateMachineEvent(const Database::RowData& data, uint32_t opcode) override;
};

class State_Example_first : public State
{
public:
	State_Example_first(StateMachine& stateMachine, int nStateNum, int nodeTimeOut, StateType eStateType);

	virtual bool HandleData(const Database::RowData& data, uint32_t opcode) override;
	virtual bool HandleEnteredState() override;
	virtual void HandleExitState() override;
	virtual void OnPaused() override;
	virtual void OnResume() override;
};

class State_Example_Second : public State
{
public:
	State_Example_Second(StateMachine& stateMachine, int nStateNum, int nodeTimeOut, StateType eStateType);

	virtual bool HandleData(const Database::RowData& data, uint32_t opcode) override;
	virtual bool HandleEnteredState() override;
	virtual void HandleExitState() override;
	virtual void OnPaused() override;
	virtual void OnResume() override;

private:
	// Base class type definition
	typedef State Super;
};

class State_Example_Third : public State
{
public:
	State_Example_Third(StateMachine& stateMachine, int nStateNum, int nodeTimeOut, StateType eStateType);

	virtual bool HandleData(const Database::RowData& data, uint32_t opcode) override;
	virtual bool HandleEnteredState() override;
	virtual void HandleExitState() override;
	virtual bool HandleTimeout() override;
	virtual void OnPaused() override;
	virtual void OnResume() override;

private:
	typedef State Super;
};