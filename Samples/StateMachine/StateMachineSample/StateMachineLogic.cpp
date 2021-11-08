#include "StateMachineLogic.h"
#include "Databases.h"
#include "Tables.h"

#include <Core.hpp>

StateMachineLogic::StateMachineLogic(
	const Database::Subscriber& subscriber, 
	Database::DataSet& dataset, 
	const Database::Row& rowStateMachineSts) : 
	StateMachine(subscriber, rowStateMachineSts)
{
	//------------- [STATE_FIRST]-----------------------------------
	

	AddState<State_Example_first>(*this, STATE_FIRST, 0, State::STATE_DEFAULT);
	GetState(STATE_FIRST)->AddTransition(GO_TO_SECOND, STATE_SECOND);

	//------------- [STATE_SECOND]  ---------------------------
	

	AddState<State_Example_Second>(*this, STATE_SECOND, 0, State::STATE_DEFAULT);
	GetState(STATE_SECOND)->AddTransition(GO_TO_THIRD, STATE_THIRD);

	//------------- [STATE_THIRD]  ---------------------------
	

	AddState<State_Example_Third>(*this, STATE_THIRD, 0, State::STATE_DEFAULT);
	GetState(STATE_THIRD)->AddTransition(GO_TO_FIRST, STATE_FIRST);

	// Register to relevant data
	RegisterHandler(dataset[ProjectCommon::MonitorDBIndex::Table1DBEnum][MonitorSample::Table1DBEnum::VALUE], 10);
	RegisterHandler(dataset[ProjectCommon::MonitorDBIndex::Table1DBEnum][MonitorSample::Table1DBEnum::STATUS], 20);
}

StateMachineLogic::~StateMachineLogic()
{
	// TODO: Unsubscribe all handlers
}

void StateMachineLogic::HandleStateMachineEvent(const Database::RowData& data, uint32_t opcode)
{
	Database::Table dataTable = data.DBRow().Parent();
	if (dataTable.Empty())
		return;

	Database::AnyKey table = dataTable.Key();
	Database::AnyKey row = data.Key();

	if ((ProjectCommon::MonitorDBIndex)table == ProjectCommon::Table1DBEnum && (MonitorSample::Table1DBEnum)row== MonitorSample::STATUS)
	{
		MonitorSample::CommunicationEnum status = data.Read<MonitorSample::CommunicationEnum>();
		if (status == MonitorSample::PAUSED)
			Pause();
		else
			Resume();		
	}
	else
	{		
		StateMachine::HandleStateMachineEvent(data, opcode);				
	}
}

//---------------- [STATE_FIRST] --------------------------------------------------
State_Example_first::State_Example_first(StateMachine& stateMachine, int nStateNum, int nodeTimeOut, StateType eStateType) :
	State(stateMachine, nStateNum, nodeTimeOut, eStateType)
{
}

bool State_Example_first::HandleData(const Database::RowData& data, uint32_t opcode)
{
	if (data.Key().Equals(MonitorSample::Table1DBEnum::VALUE) == false)
		return false;

	int val = data.Read<int>();
	if (val < 0 || val > 20)
		return m_stateMachine.PerformTransition(StateMachineLogic::GO_TO_SECOND);
	
	return false;
}

bool State_Example_first::HandleEnteredState()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Enter First======================\n");
	return false;
}

void State_Example_first::HandleExitState()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Exit First======================\n");
}

void State_Example_first::OnPaused()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Pause On First======================\n");
}

void State_Example_first::OnResume()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Resume On  First======================\n");
}

//---------------- [STATE_SECOND] --------------------------------------------------
State_Example_Second::State_Example_Second(StateMachine& stateMachine, int nStateNum, int nodeTimeOut, StateType eStateType) :
	State(stateMachine, nStateNum, nodeTimeOut, eStateType)
{
}

bool State_Example_Second::HandleData(const Database::RowData& data, uint32_t opcode)
{
	if (data.Key().Equals(MonitorSample::Table1DBEnum::VALUE) == false)
		return false;

	int val = data.Read<int>();
	if (val < 21 || val > 40)
		return m_stateMachine.PerformTransition(StateMachineLogic::GO_TO_THIRD);

	return false;	
}

bool State_Example_Second::HandleEnteredState()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Enter Second======================\n");
	return false;
}

void State_Example_Second::HandleExitState()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Exit Second======================\n");
}

void State_Example_Second::OnPaused()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Pause On Second======================\n");
}

void State_Example_Second::OnResume()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Resume On  Second======================\n");
}

//---------------- [STATE_THIRD] --------------------------------------------------

State_Example_Third::State_Example_Third(StateMachine& stateMachine, int nStateNum, int nodeTimeOut, StateType eStateType) :
	State(stateMachine, nStateNum, nodeTimeOut, eStateType)
{
}

bool State_Example_Third::HandleData(const Database::RowData& data, uint32_t opcode)
{
	if (data.Key().Equals(MonitorSample::Table1DBEnum::VALUE) == false)
		return false;

	int val = data.Read<int>();
	if (val < 41 || val > 60)
		return m_stateMachine.PerformTransition(StateMachineLogic::GO_TO_FIRST);

	return false;	
}

bool State_Example_Third::HandleEnteredState()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Enter Third======================\n");
	return false;
}

void State_Example_Third::HandleExitState()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Exit Third======================\n");
}

bool State_Example_Third::HandleTimeout()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================TimeOut Third======================\n");
	m_stateMachine.PerformTransition(StateMachineLogic::GO_TO_FIRST);
	return true;
}

void State_Example_Third::OnPaused()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Pause On Third======================\n");
}

void State_Example_Third::OnResume()
{
	Core::Console::ColorPrint(Core::Console::Colors::WHITE, "=======================Resume On  Third======================\n");
}
