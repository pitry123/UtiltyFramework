#pragma once
#include <core/ref_count_interface.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/timer.hpp>
#include <utils/scope_guard.hpp>

#include <Database.hpp>

#include <map>
#include <cstring>
#include <limits>
#include <cstdint>
#include <type_traits>

#define MAX_TRANSITIONS_COUNT_PER_STATE			500
#define MAX_NUM_OF_TRANSITIONS_PER_STATE		32

class StateMachine;

class State : 
	public utils::ref_count_base<core::ref_count_interface>
{
public:
	enum StateType
	{
		STATE_UNINITIALIZED = 0,
		STATE_DEFAULT = 1,
		STATE_END_SUCCESS = 2,
		STATE_END_FAIL = 3,
		STATE_END_ABORT = 4
	};

	struct transition_pair
	{
		short	opcode;
		short	nextState;
	};

	// A node in the state machine
	// each state node can be of type default or an End node
protected:
	StateMachine&				m_stateMachine;
	int							m_stateID;	
	StateType					m_stateType;

	transition_pair				m_transitionsArray[MAX_TRANSITIONS_COUNT_PER_STATE];
	int							m_numOfTransitions;
	int							m_nodeTimeOut;	
	bool						m_markedFlag;
	
public:
	State(StateMachine& stateMachine, 
		int stateID,
		int nodeTimeOut = 0,
		StateType stateType = StateType::STATE_DEFAULT) :
		m_stateMachine(stateMachine),
		m_stateID(stateID),
		m_stateType(stateType),			
		m_numOfTransitions(0),
		m_nodeTimeOut(nodeTimeOut),
		m_markedFlag(false)
	{		
		std::memset(m_transitionsArray, 0, sizeof(m_transitionsArray));
	}

	int StateID() const
	{
		return m_stateID;
	}

	virtual void Initialize()
	{
		// Do Nothing...
	}
	
    virtual bool HandleEnteredState() = 0;
	
    virtual void HandleExitState() = 0;

	virtual bool HandleData(const Database::RowData& data, uint32_t opcode) = 0;
	
    virtual bool HandleTimeout()
	{
		return false;
	}

    virtual void OnPaused()
	{
		// Do Nothing...
	}

    virtual void OnResume()
	{
		// Do Nothing...
	}

	int	Timeout()
	{
		return m_nodeTimeOut;
	}

	State::StateType GetStateType()
	{
		return m_stateType;
	}	

    bool AddTransition(short opcode, short nextState)
	{
		if (m_numOfTransitions >= MAX_NUM_OF_TRANSITIONS_PER_STATE)
			return false;

		m_transitionsArray[m_numOfTransitions].nextState = nextState;
		m_transitionsArray[m_numOfTransitions].opcode = opcode;
		m_numOfTransitions++;
		return true;
	}

    int FindTransition(short currentOpcode)
	{
		for (int i = 0; i < m_numOfTransitions; i++)
		{
			if (currentOpcode == m_transitionsArray[i].opcode)// checks if opcode matches
				return m_transitionsArray[i].nextState;// found opcode
		}

		return -1;
	}
};

class StateMachine
{
public:
	enum ProcessStatus
	{
		PROCESS_NOT_STARTED,
		PROCESS_IN_THE_MIDDLE,
		PROCESS_FINISHED_FAIL,
		PROCESS_FINISHED_TIMEOUT_ERROR,
		PROCESS_FINISHED_SUCCESSFULY,
		PROCESS_FINISHED_KILLED,
		PROCESS_FINISHED_ABORTED,
		PROCESS_PAUSED
	};

	enum StateMachineStatusEnum
	{
		STATE_MACHINE_NOT_VALID,
		STATE_MACHINE_VALID
	};

	enum TimeoutStatusEnum
	{
		TIMEOUT_NOT_EXPIRED,
		TIMEOUT_EXPIRED
	};

	struct StateMachineEntryStruct
	{
		StateMachineStatusEnum	nIsValid;			// t:int
		int						nProcessStatus;
		int						nLastOpcode;
		int						nLastTransition;
		int						nCurrentState;
		TimeoutStatusEnum		nIsTimeout;			// t:int
	};

private:
	// Holds the state machine (transitions move the current state from one to the other
	struct Graph_struct
	{
		std::map<int, utils::ref_count_ptr<State>> nodeTable;
		int										   currentSize;
	};

	Graph_struct	m_StateGraph;

	StateMachine(const StateMachine& other) = delete;       // non construction-copyable
	StateMachine& operator=(const StateMachine&) = delete;	// non copyable				
	StateMachine(const StateMachine&& other) = delete;      // non construction-movable
	StateMachine& operator=(const StateMachine&&) = delete;	// non movable

protected:
	ProcessStatus						m_processStatus;
	ProcessStatus						m_LastprocessStatus;

	Database::Subscriber				m_subscriber;	
	Database::SubscriptionsCollector	m_subscriptions;

	utils::timer						m_pStateTimer;
	utils::timer						m_pProcessTimer;
	int									m_nProcessTimeOut;
	int									m_loggerUnit;

	// If the state machine is implemented inside a state that is part of another state machine
	// then this pointer is to the "Father" state machine.
	StateMachine*	m_pOwnerStateMachine;

	Database::Row m_rowStateMachineSts;
	StateMachineEntryStruct m_stStateMachineSts;

public:
	StateMachine(const Database::Subscriber& subscriber, const Database::Row& rowStateMachineSts = nullptr) :
		m_processStatus(ProcessStatus::PROCESS_NOT_STARTED),
		m_LastprocessStatus(ProcessStatus::PROCESS_NOT_STARTED),
		m_subscriber(subscriber),
		m_rowStateMachineSts(rowStateMachineSts),
		m_stStateMachineSts({})
	{
		if (m_subscriber.Empty() == true)
			m_subscriber = Database::Subscriber(utils::make_ref_count_ptr<Database::Dispatcher>());

		m_pStateTimer.elapsed += [this]()
		{
			HandleStateTimeOut();
		};
	}

	StateMachine(const Database::Row& rowStateMachineSts = nullptr) :
		StateMachine(nullptr, rowStateMachineSts)
	{
	}

	virtual ~StateMachine()
	{
		UnsubscribeAll();
		m_subscriber.Context().Sync();
	}

	// Must be called each time the a state machine process is started
	virtual void InitializeResetRun(int nProcessTimeOut, int nFirstStateToRun)
	{
		m_subscriber.Context().BeginInvoke([this, nProcessTimeOut, nFirstStateToRun]()
		{
			// Go over each node and make sure it is Initialized and all it's fields are reset
			for (auto& pair : (m_StateGraph.nodeTable))
			{
				pair.second->Initialize();
			}

			// Should be called each time the state machine should be run
			if (m_processStatus != PROCESS_NOT_STARTED)
			{
				m_pProcessTimer.stop();
				StopStateTimer();
			}

			m_nProcessTimeOut = nProcessTimeOut;
			m_stStateMachineSts.nCurrentState = nFirstStateToRun;
			m_stStateMachineSts.nIsValid = StateMachineStatusEnum::STATE_MACHINE_VALID;

			if (m_nProcessTimeOut)
			{
				m_stStateMachineSts.nIsTimeout = TimeoutStatusEnum::TIMEOUT_NOT_EXPIRED;
				m_pProcessTimer.start(m_nProcessTimeOut);
			}

			m_processStatus = PROCESS_IN_THE_MIDDLE;

			// Lets the first state do some logic 
			SwitchToNextState();
		});
	}

	void Pause()
	{
		m_subscriber.Context().BeginInvoke([this]()
		{
			if (m_processStatus != PROCESS_PAUSED)
			{
				StopStateTimer();
				m_LastprocessStatus = m_processStatus;
				m_processStatus = PROCESS_PAUSED;
				m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->OnPaused();
			}
		});
	}

	void Resume()
	{
		m_subscriber.Context().BeginInvoke([this]()
		{
			if (m_processStatus == PROCESS_PAUSED)
			{
				StartStateTimer();
				m_processStatus = m_LastprocessStatus;
				m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->OnResume();
			}
		});
	}

	virtual bool PerformTransition(short transitionOpcode)
	{
		int	nTempNextState = -1;
		m_stStateMachineSts.nLastOpcode = transitionOpcode;
		if (-1 == (nTempNextState = m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->FindTransition(transitionOpcode)))
		{
			return false;
		}

		// check if next state to move to is Legal
		if ((m_StateGraph.nodeTable.find(nTempNextState) != m_StateGraph.nodeTable.end()) &&
			(State::STATE_UNINITIALIZED != m_StateGraph.nodeTable[nTempNextState]->GetStateType()))
		{
			// State is being left, call HandleExitState
			m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->HandleExitState();
			m_stStateMachineSts.nCurrentState = nTempNextState;
			return true;
		}

		return false;
	}


private:
	void UnsubscribeAll()
	{
		m_subscriptions.Clear();
	}

	bool CallState(const Database::RowData& data, int opcode)
	{
		bool isMovedToNext = false;

		if (PROCESS_IN_THE_MIDDLE != m_processStatus)
		{
			return false;
		}

		// Allow current State to handle the new Opcode. If opcode causes a transition then 
		// return value is True
        isMovedToNext = m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->HandleData(data, static_cast<uint32_t>(opcode));

		if (true == isMovedToNext)
		{
			//cancel previous state timer
			StopStateTimer();

			// call new state to do some logic
			SwitchToNextState();
		}

		return true;
	}

protected:
	virtual void HandleStateMachineEvent(const Database::RowData& data, uint32_t opcode)
	{
        CallState(data, static_cast<int>(opcode));
	}

	void RegisterHandler(const Database::Row& row, uint32_t opcode = (std::numeric_limits<uint32_t>::max)())
	{
		if (row.Empty() == true)
			throw std::invalid_argument("row");

		m_subscriptions += m_subscriber.Subscribe(row, [this, opcode](const Database::RowData& data)
		{
			HandleStateMachineEvent(data, opcode);
		});
	}

	template<typename T, typename... Args>
	bool AddState(Args&&... args)
	{
		static_assert(std::is_base_of<State, T>() == true, "AddState assumed to be template with a 'State' generalization");		

		utils::ref_count_ptr<State> instance;		
		instance = utils::make_ref_count_ptr<T>(std::forward<Args>(args)...);
		
		int id = instance->StateID();
		if (m_StateGraph.nodeTable.find(id) != m_StateGraph.nodeTable.end())
			throw std::runtime_error("a state with same id already exists");

		m_StateGraph.nodeTable[id] = instance;
		return true;
	}

	State* GetState(int id)
	{
		auto it = m_StateGraph.nodeTable.find(id);
		if (it == m_StateGraph.nodeTable.end())
			return nullptr;

		return it->second;
	}

	void StopStateTimer()
	{
		m_pStateTimer.stop();
	}

	void StartStateTimer()
	{
		int time = m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->Timeout();
		if (time > 0)
		{
			m_pStateTimer.start(m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->Timeout());
		}
	}

	virtual void HandleStateTimeOut()
	{
		// called if a timeout was set for the state

		bool isMovedToNext = false;

		// checks if the state has a handle timeout function
		// TODO: isMovedToNext should be out parameter (ref)
		isMovedToNext = m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->HandleTimeout();
		m_stStateMachineSts.nIsTimeout = TIMEOUT_EXPIRED;

		if (m_rowStateMachineSts != nullptr)
		{
			m_rowStateMachineSts.Write<StateMachineEntryStruct>(m_stStateMachineSts, true);
		}
	
		// if state moved to a different state because of timeout, call new state to do logic
		if (true == isMovedToNext)
		{
			StopStateTimer();
			SwitchToNextState();
		}
	}

	virtual void HandleProcessTimeOut()
	{
		StopStateTimer();
		m_pProcessTimer.stop();
		m_processStatus = PROCESS_FINISHED_TIMEOUT_ERROR;
	}

	void SwitchToNextState()
	{
		State::StateType	newStateType;
		bool isMovedToNext = false;

		// The new state can call other states immediately
		do
		{
			// [-------------Try this------------>

			newStateType = m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->GetStateType();

			if (State::STATE_END_SUCCESS == newStateType)
			{
				m_pProcessTimer.stop();
				m_processStatus = PROCESS_FINISHED_SUCCESSFULY;
			}
			else if (State::STATE_END_FAIL == newStateType)
			{
				m_pProcessTimer.stop();
				m_processStatus = PROCESS_FINISHED_FAIL;
			}
			else if (State::STATE_END_ABORT == newStateType)
			{
				m_pProcessTimer.stop();
				m_processStatus = PROCESS_FINISHED_ABORTED;
			}
			// >---------End - try this----------]

			// Allow derived state-machine to do some logic upon changing state			
			OnSwitchingState();
			if (m_rowStateMachineSts != nullptr)
			{
				m_rowStateMachineSts.Write<StateMachineEntryStruct>(m_stStateMachineSts);
			}

			// Call function of new state to do any logic that it needs
			isMovedToNext = m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->HandleEnteredState();

		} while ((true == isMovedToNext) &&
			(m_processStatus != PROCESS_FINISHED_SUCCESSFULY) &&
			(m_processStatus != PROCESS_FINISHED_FAIL) &&
			(m_processStatus != PROCESS_FINISHED_ABORTED));

		if ((true == isMovedToNext) && ((m_processStatus == PROCESS_FINISHED_SUCCESSFULY) || (m_processStatus == PROCESS_FINISHED_FAIL) || (m_processStatus == PROCESS_FINISHED_ABORTED)))
		{
			// We moved to an End state so the while is finished, but still we want to 
			// call the LocalEnterNewState for this last state
			OnSwitchingState();
		}

		// -----------moved from here-------------

		// If new state has a timer then start it
		if (m_StateGraph.nodeTable[m_stStateMachineSts.nCurrentState]->Timeout() > 0)
		{
			StartStateTimer();
		}

		return;
	}

	virtual void OnSwitchingState()
	{
	}
};

