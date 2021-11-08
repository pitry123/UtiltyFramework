#pragma once
#include <iostream>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <Factories.hpp>
#include <Core.hpp>
#include <Database.hpp>
#include <Communication.hpp>
#include <Utils.hpp>
#include <Monitor.hpp>


#include <mutex>
#include <cstring>
#include <future>
#include <random>
#include <vector>
#include <limits>

#ifndef _WIN32
#include <sys/time.h>
#endif

using namespace Communication;


namespace Database
{
	class MonitorClient : public Dispatcher
	{
	private:
		static constexpr size_t MAX_DATA_SIZE = (std::numeric_limits<short>::max)();

		static constexpr int MONITOR_COMMUNICATION_FAILURE_TIMEOUT_IN_TICK = 3000;
		static constexpr uint8_t HIGHEST_PRIORITY = (std::numeric_limits<uint8_t>::max)();
		static constexpr uint8_t LOWEST_PRIORITY = (std::numeric_limits<uint8_t>::min)();

		std::mutex m_mutex;
		CommClientChannel m_pCommChan;
		bool m_bGotMsgFromMonitorApp;
		unsigned int m_monitorVersion;
		unsigned int m_nTickGetFromLastArrivedAck;
		Utils::Timer m_timer;
		DataSet m_dataset;

		Utils::SignalToken m_communicationToken;
		std::map < std::pair < int, int >, Database::SubscriptionToken> m_tokenMap;
		std::map < std::pair < int, int >, bool> m_debugStateMap;
		std::vector<uint8_t> m_dataQueryBuffer;
		Logging::Logger LOGGER = Core::Framework::CreateLogger("MonitorClient", Logging::Severity::ERROR);
		std::function<void(bool Status)> m_connecFunc;
	public:
		MonitorClient(const char* remoteHostName,
			uint16_t remotePort,
			const char* localHostName,
			uint16_t localPort, const DataSet dataset,
			const std::function<void(bool)>& connFunc,
			bool legacyMode = false) :
			Dispatcher("Monitor"),			
			m_bGotMsgFromMonitorApp(false),
			m_dataset(dataset),
			m_communicationToken(Utils::SignalTokenUndefined),
			m_connecFunc(connFunc)
		{
			if (legacyMode == false)
			{
				m_pCommChan = Communication::CommClientChannel(Communication::Protocols::VariableLengthProtocol::Create(
					Communication::Ports::UdpPort::Create(
						remoteHostName, 
						remotePort, 
						localHostName, 
						localPort), 2, 0, true, MAX_DATA_SIZE), MAX_DATA_SIZE, true);
			}
			else
			{
				m_pCommChan = Communication::CommClientChannel(Communication::Protocols::UdpDatagramProtocol::Create(
					remoteHostName,
					remotePort,
					localHostName,
					localPort), MAX_DATA_SIZE, true);
			}

			// Data request buffer
			m_dataQueryBuffer.resize(m_pCommChan.MaxMessageSize());

			// set the function for the periodic timer
			m_timer.Elapsed() += [this]()
			{
				PeriodicMonitorDataCollection();
			};
		}

		~MonitorClient()
		{
			Disconnect();
		}

		bool Disconnect()
		{
			if (m_communicationToken == Utils::SignalTokenUndefined)
				return false;

			m_timer.Stop();

			m_pCommChan.OnData() -= m_communicationToken;
			m_pCommChan.Disconnect();
			m_bGotMsgFromMonitorApp = false;
			m_communicationToken = Utils::SignalTokenUndefined;
			UnRegisterAll();
			if (m_connecFunc != nullptr)
				m_connecFunc(false);
			return true;
		}

		bool Connect()
		{
			Disconnect();

			if (m_pCommChan.Connect() == false)
				return false;

			m_communicationToken = m_pCommChan.OnData() += [this](const DataReader& reader)
			{
				MessageClientHandler(reader);
			};

			m_timer.Start(500);

			return true;
		}

		bool IsConnected()
		{
			return m_bGotMsgFromMonitorApp;
		}

		virtual void Start() override
		{
			
		}

		DataSet Dataset()
		{
			return m_dataset;
		 }

		void RegisterForEntry(int tableId, int entryId)
		{
			Table table;
			if (m_dataset.TryGet(tableId, table) == false)
				throw std::runtime_error("table not found in dataset");
			
			DeToMonitorDataMsgEx msg;
			msg.shLength = sizeof(DeToMonitorDataMsgEx) - sizeof(msg.shLength);
			msg.byOpcode = MONITOR_REGISTER_SINGLE_DATA_OPCODE_EX;
			msg.unTable = tableId;
			msg.unRow = entryId;
			m_pCommChan.Send((uint8_t*)&msg, sizeof(msg));


		}

		void SendRequestForSingleRow(int tableId, int entryId)
		{
			DeToMonitorDataMsgEx msg;
			msg.shLength = sizeof(DeToMonitorDataMsgEx) - sizeof(msg.shLength);
			msg.byOpcode = MONITOR_GET_SINGLE_DATA_EX;
			msg.unTable = tableId;
			msg.unRow = entryId;
			m_pCommChan.Send((uint8_t*)&msg, sizeof(msg));
		}

		void SendSingleData(int tableId,int entryId,size_t size, uint8_t* data)
		{
			size_t dataSize = size + sizeof(DeToMonitorDataMsgEx);
			if (dataSize > m_dataQueryBuffer.size())
			{
				Core::Console::ColorPrint(Core::Console::Colors::RED, "Row data Table:%d, Row:%d size exceeds max message size", tableId, entryId);
			}
			bool bDebugState = false;
			
			DeToMonitorDataMsgEx msg;
			msg.shLength = static_cast<short>(size);
			if (m_monitorVersion > 1)
			{
				int len = static_cast<int>(msg.shLength) + static_cast<int>(sizeof(msg)) - static_cast<int>(sizeof(msg.shLength));
				msg.shLength = static_cast<short>(len);
			}

			msg.unTable = tableId;
			msg.unRow = entryId;
			if (bDebugState)
				msg.byOpcode = MONITOR_UPDATE_DEBUG_STATE_OPCODE_EX;
			else
				msg.byOpcode = MONITOR_UPDATE_DATA_EX;

			memcpy(m_dataQueryBuffer.data(), &msg, sizeof(msg));
			memcpy(m_dataQueryBuffer.data() + sizeof(msg), data, size);
			m_pCommChan.Send(m_dataQueryBuffer.data(), sizeof(msg) + size);

		}
		void SendGetRequestForEntryDebugStateMessage(int tableId, int entryId)
		{
			DeToMonitorDataMsgEx msg;
			msg.shLength = sizeof(DeToMonitorDataMsgEx) - sizeof(msg.shLength);
			msg.byOpcode = MONITOR_GET_DEBUG_MODE_OPCODE;
			msg.unTable = tableId;
			msg.unRow = entryId;
			m_pCommChan.Send((uint8_t*)&msg, sizeof(msg));
		}

		bool EnterDebugState(int tableId, int entryId, bool debugState)
		{
			if (DebugState(tableId, entryId) == debugState)
				return false;

			DeToMonitorDataMsgEx msg;
			msg.byOpcode = MONITOR_UPDATE_DEBUG_STATE_OPCODE_EX;
			msg.shLength = sizeof(DeToMonitorDataMsgEx) - sizeof(msg.shLength);
			msg.unTable = tableId;
			msg.unRow = entryId;
			
			std::pair<int, int> key;
			key.first = tableId;
			key.second = entryId;
			auto it = m_debugStateMap.find(key);
			if (it == m_debugStateMap.end())
			{
				m_debugStateMap.emplace(key, debugState);
			}
			else
				it->second = debugState;

			bool retval = m_pCommChan.Send((uint8_t*)&msg, sizeof(msg));
			SendGetRequestForEntryDebugStateMessage(tableId, entryId);
			return retval;
		}
		
		bool DebugState(uint32_t tableId, uint32_t rowId)
		{
			if (false == IsConnected())
				return false;

			SendGetRequestForEntryDebugStateMessage(tableId, rowId);

			return Context().Invoke<bool>([&, tableId, rowId]() {
				std::pair<int, int> key;
				key.first = tableId;
				key.second = rowId;
				auto it = m_debugStateMap.find(key);
				if (it == m_debugStateMap.end())
					return false;

				return it->second;
			});
			
		}
	private:
		void MessageClientHandler(const DataReader& reader)
		{
			int	OpCode(0);
			OpCode = GetOpcodeExt(reader);
			try
			{

				switch (OpCode)
				{
				case MONITOR_GET_SINGLE_DATA_EX:
				case MONITOR_SINGLE_MSG_OPCODE:
				case MONITOR_SINGLE_MSG_OPCODE_EX:
					ReceivedSingleRow(reader);
					break;
				case MONITOR_DB_MSG_OPCODE:
				case MONITOR_DB_MSG_OPCODE_EX:
					WriteDBData(reader);
					break;
				case MONITOR_SERVER_KEEP_ALIVE_OPCODE_EX:
					MonitorKeepAliveMsg keepalive;
					reader.Read<MonitorKeepAliveMsg>(keepalive);
					m_monitorVersion = keepalive.unMonitorInfo;
					m_nTickGetFromLastArrivedAck = GetTicks();
					break;
				case MONITOR_GET_KEEP_ALIVE_OPCODE:
					break;
				case MONITOR_SEND_LOGGER_MSG_OPCODE:
					Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "MONITOR_SEND_LOGGER_MSG_OPCODE unsupported");
					break;
				case MONITOR_SNIFFER_MSG_OPCODE:
				case MONITOR_SNIFFER_MSG_OPCODE_EX:
					Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "MONITOR_SNIFFER_MSG_OPCODE unsupported");
					break;
				case MONITOR_DEBUG_MODE_OPCODE:
					UpdateDebugState(reader);
					break;

				default:
					Core::Console::ColorPrint(Core::Console::Colors::BLUE, "receive unsupported opcode =%d\n", OpCode);
				}
			}
			catch (std::exception &e)
			{
				LOG_ERROR(LOGGER) << "MessageHandler Exception on MessageHandler:" << e.what();
			}


		}
		
		void UpdateDebugState(const DataReader& reader)
		{
			bool debugState;
			MonitorSingleDataMsgEx msg;
			memcpy(&msg, (uint8_t*)reader.Buffer() , sizeof(MonitorSingleDataMsgEx));
			memcpy(&debugState, (uint8_t*)reader.Buffer() + sizeof(MonitorSingleDataMsgEx), sizeof(bool));
			std::pair<int, int> key;
			key.first = msg.unTable;
			key.second = msg.unRow;
			auto it = m_debugStateMap.find(key);
			if (it == m_debugStateMap.end())
			{
				m_debugStateMap.emplace(key, debugState);
			}
			else
				it->second = debugState;

			//TODO: Publish debug state to application
		}

		void ReceivedSingleRow(const DataReader& reader)
		{
			MonitorSingleDataMsgEx& msg = reader.Read<MonitorSingleDataMsgEx>();
			Table table;
			if (false == m_dataset.TryGet(msg.unTable, table))
			{
				Core::Console::ColorPrint(Core::Console::Colors::BLUE, "unknown tableID received=%d\n", msg.unTable);
				LOG_INFO(LOGGER) << "unknown tableID received" << msg.unTable;
				return;
			}

			Row row;
			if (false == table.TryGet(msg.unRow, row))
			{
				Core::Console::ColorPrint(Core::Console::Colors::BLUE, "unknown rowID received=%d\n", msg.unRow);
				LOG_INFO(LOGGER) << "unknown rowID received" << msg.unRow;
				return;
			}
			size_t offset = sizeof(MonitorSingleDataMsgEx);
			size_t length = msg.stMsgHeader.shLength - sizeof(MonitorSingleDataMsgEx);
			if (length > reader.Size())
			{
				LOG_INFO(LOGGER) <<"received length mismatch with message length (" << msg.stMsgHeader.shLength << ")";
				return;
			}
			row.Write((const void*)(((uint8_t*)reader.Buffer())+offset), length);
		}

		void PeriodicMonitorDataCollection()
		{
			unsigned int temp = GetTicks();
			if ((temp - m_nTickGetFromLastArrivedAck) > MONITOR_COMMUNICATION_FAILURE_TIMEOUT_IN_TICK)
			{
				if (true == m_bGotMsgFromMonitorApp)
				{
					HandleMonitorCommunicationError();
				}
			}
			else
			{
				if (false == m_bGotMsgFromMonitorApp)
				{
					HandleMonitorCommunicationResume();
				}
			}

			// Send Keep Alive msg
			BuildMonitorClientKeepAliveMsg();
		}

		int GetOpcodeExt(const DataReader& reader)
		{
			uint8_t* buffer = (uint8_t*)reader.Buffer();
			return buffer[2];
		}

		bool WriteDBData(const DataReader& reader)
		{
			MonitorDBDataMsgEx msg;
			reader.Read<MonitorDBDataMsgEx>(msg);
			int nTable = msg.unDBIndex;

			Table table;
			if (m_dataset.TryGet(nTable, table) == false)
				return false;

			size_t nNumOfBytesCopied = sizeof(MonitorDBDataMsgEx);
			for (int i = 0; i < (int)table.Size(); i++)
			{
				// Assumption:
				// Row tables and rows keys are incremental integers starting from 0 (DE enum convention)
				size_t dataSize = table[i].DataSize();
				if (dataSize + nNumOfBytesCopied > reader.Size())
				{
					Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Row data size exceeds max message size");
					return false;
				}
				memcpy((void*)m_dataQueryBuffer.data(), (uint16_t*)reader.Buffer() + nNumOfBytesCopied, dataSize);
				table[i].Write((const void*)m_dataQueryBuffer.data(), dataSize);
				nNumOfBytesCopied += dataSize;

			}
			return true;
		}
		
		void BuildMonitorClientKeepAliveMsg()
		{
			if (m_pCommChan.Status() == CommStatus::CONNECTED) {

				MonitorMsgHeader keepAliveMsg;
				keepAliveMsg.shLength = sizeof(MonitorMsgHeader) - sizeof(keepAliveMsg.shLength);
				keepAliveMsg.byOpcode = MONITOR_GET_KEEP_ALIVE_OPCODE;
				m_pCommChan.Send(&keepAliveMsg, sizeof(keepAliveMsg));
			}
			else
			{
				Connect();
			}
		}

		void UnRegisterSingleData(int nTable, int nRow)
		{
			std::pair <int, int> p;
			p.first = nTable;
			p.second = nRow;
			if (IsTokenInMap(nTable, nRow))
			{

				Table table;
				if (m_dataset.TryGet(nTable, table) == false)
					throw std::runtime_error("table not found in dataset");

				if (Unsubscribe(table[nRow], m_tokenMap[p]) == false)
				{
					LOG_INFO(LOGGER) << "Unsubscribe Db" << nTable << "row =" << nRow << "with token =" << m_tokenMap[p] << "failed";
				}

				m_tokenMap.erase(p);
				return;


			}
		}

		void UnRegisterAll()
		{
			while (m_tokenMap.size() > 0)
			{
				std::pair <int, int> p = m_tokenMap.begin()->first;

				try {
					UnRegisterSingleData(p.first, p.second);
				}
				catch (...)
				{
					//TODO - Update to logger
					return;
				}
			}
		}

		void HandleMonitorCommunicationError()
		{
			m_bGotMsgFromMonitorApp = false;
			m_connecFunc(m_bGotMsgFromMonitorApp);
			// TODO: Use logger instead of printings...
			Core::Console::ColorPrint(Core::Console::Colors::BLUE, "Monitoring Communication Error\n");
			UnRegisterAll();

			m_pCommChan.Disconnect();
			m_pCommChan.Connect();
		}

		void HandleMonitorCommunicationResume()
		{
			// TODO: Use logger instead of printings...
			Core::Console::ColorPrint(Core::Console::Colors::BLUE, "Monitoring Communication Resume\n");
			m_bGotMsgFromMonitorApp = true;
			m_connecFunc(m_bGotMsgFromMonitorApp);

		}

		bool IsTokenInMap(int nTable, int nRow)
		{
			std::pair <int, int> p;
			p.first = nTable;
			p.second = nRow;

			if ((m_tokenMap.find(p) == m_tokenMap.cend()))
				return false;
			return true;
		}

#pragma pack(1)

		struct MonitorMsgHeader
		{
			short shLength;
			uint8_t  byOpcode;
		};

		struct MonitorKeepAliveMsg
		{
			MonitorMsgHeader stMsgHeader;
			unsigned int	 unMonitorInfo;

		};

		struct DeToMonitorDataMsgEx
		{
			short            shLength;
			uint8_t			 byOpcode;
			unsigned int	 unTable;
			unsigned int     unRow;
		};

		struct DeToMonitorGetDataMsgEx
		{
			short            shLength;
			uint8_t			 byOpcode;
			unsigned int	 unTable;
		};

		struct MonitorSingleDataMsgEx
		{
			MonitorMsgHeader stMsgHeader;
			unsigned int	 unTimeTag;
			unsigned int	 unTable;
			unsigned int     unRow;

		};

		struct MonitorDBDataMsgEx
		{
			MonitorMsgHeader stMsgHeader;
			unsigned int	 unTimeTag;
			unsigned int	 unDBIndex;
		};

		struct SingleData
		{
			unsigned int	 unTable;
			unsigned int     unRow;
		};

#pragma pack()

		enum ToMonitorOpcodesEnum
		{

			MONITOR_SNIFFER_MSG_OPCODE_EX = 0x14,
			MONITOR_SERVER_KEEP_ALIVE_OPCODE_EX = 0x15,
			MONITOR_SINGLE_MSG_OPCODE_EX = 0x18,
			MONITOR_DB_MSG_OPCODE_EX = 0x19,
			MONITOR_SEND_LOGGER_MSG_OPCODE_EX = 0x1a,

			MONITOR_SNIFFER_MSG_OPCODE = 0x04,
			MONITOR_SEND_KEEP_ALIVE_OPCODE = 0x05,
			MONITOR_SINGLE_MSG_OPCODE = 0x08,
			MONITOR_DB_MSG_OPCODE = 0x09,
			MONITOR_SEND_LOGGER_MSG_OPCODE = 0x0a,

			MONITOR_DEBUG_MODE_OPCODE = 0x20,
			MONITOR_SEND_ROWS_OPCODE = 0x21,
			MONITOR_SEND_ROW_SCHEMA_OPCODE = 0x22,
			MONITOR_SEND_ENUM_SCHEMA_OPCODE = 0x23
		};

		enum FromMonitorOpcodesEnum
		{

			MONITOR_GET_SINGLE_DATA_EX = 0x11,
			MONITOR_GET_DB_DATA_EX = 0x12,
			MONITOR_UPDATE_DATA_EX = 0x13,
			MONITOR_REGISTER_SNIFFER_OPCODE_EX = 0x14,
			MONITOR_GET_KEEP_ALIVE_OPCODE = 0x05,
			MONITOR_SET_LOG_UNITS_MSG_OPCODE = 0x06,
			MONITOR_UPDATE_DEBUG_STATE_OPCODE_EX = 0x17,
			MONITOR_REGISTER_SINGLE_DATA_OPCODE_EX = 0x18,
			MONITOR_UNREGISTER_SINGLE_DATA_OPCODE_EX = 0x19,
			MONITOR_UPDATE_SYSTEM_CLOCK_OPCODE_EX = 0x1a,
			MONITOR_UNREGISTER_SNIFFER_OPCODE_EX = 0x1b,
			MONITOR_REGISTER_MULTI_SINGLE_DATA_OPCODE_EX = 0x1c,
			MONITOR_UNREGISTER_MULTI_SINGLE_DATA_OPCODE_EX = 0x1d,
			MONITOR_GET_MULTI_SINGLE_DATA_OPCODE_EX = 0x1e,
			MONITOR_GET_DEBUG_MULTI_SINGLE_DATA_OPCODE_EX = 0x1f,
			MONITOR_GET_ROWS_OPCODE = 0x21,
			MONITOR_GET_ROW_SCHEMA_OPCODE = 0x22,
			MONITOR_GET_ENUM_SCHEMA_OPCODE = 0x23,
			MONITOR_GET_SINGLE_DATA = 0x01,
			MONITOR_GET_DB_DATA = 0x02,
			MONITOR_UPDATE_DATA = 0x03,
			MONITOR_REGISTER_SNIFFER_OPCODE = 0x04,
			MONITOR_UPDATE_DEBUG_STATE_OPCODE = 0x07,
			MONITOR_REGISTER_SINGLE_DATA_OPCODE = 0x08,
			MONITOR_UNREGISTER_SINGLE_DATA_OPCODE = 0x09,
			MONITOR_UPDATE_SYSTEM_CLOCK_OPCODE = 0x0a,
			MONITOR_UNREGISTER_SNIFFER_OPCODE = 0x0b,


			MONITOR_GET_DEBUG_MODE_OPCODE = 0x20,


			NUMBER_OF_OPCODES
		};

	};
}