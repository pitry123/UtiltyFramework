#pragma once 

class MonitorSample
{
public: 
	enum Table1DBEnum
	{
		VALUE, //t:int
		STATUS,  //MonitorSample::CommunicationEnum  t:int
		STATE_MACHINE_STS,// t:StateMachineLogic.h::StateMachineEntryStruct_StateMachineLogic
		NUM_OF_TABLE1_ROWS
	};

	enum Table2DBEnum
	{
		VALUE2, //t:int
		STATUS2,  //MonitorSample::CommunicationEnum  t:int

		NUM_OF_TABLE2_ROWS
	};


	enum CommunicationEnum
	{
		RUNNING = 1,
		PAUSED = 2,
	};
};