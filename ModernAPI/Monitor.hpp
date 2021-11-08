#pragma once
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <utils/database.hpp>

#include <Common.h>
#include <Core.hpp>
#include <Database.hpp>
#include <Communication.hpp>
#include <Utils.hpp>
#include <Factories.hpp>
#include <Logging.hpp>

#include <iostream>
#include <mutex>
#include <cstring>
#include <future>
#include <random>
#include <vector>
#include <limits>
#include <sstream>

#ifndef _WIN32
#include <sys/time.h>
#endif

using namespace Communication;

#ifdef _WIN32
static inline unsigned long GetTicks()
{
	return GetTickCount();
}
#else
static inline unsigned int GetTicks()
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL) != 0)
		return 0;

	return static_cast<unsigned int>((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}
#endif

namespace Database
{
	class Monitor : public Dispatcher
	{
	private:
		static constexpr size_t MAX_DATA_SIZE = (std::numeric_limits<short>::max)();

		static constexpr int MONITOR_COMMUNICATION_FAILURE_TIMEOUT_IN_TICK = 3000;
		static constexpr uint8_t HIGHEST_PRIORITY = (std::numeric_limits<uint8_t>::max)();
		static constexpr uint8_t LOWEST_PRIORITY = (std::numeric_limits<uint8_t>::min)();

		std::mutex m_mutex;
		CommClientChannel m_pCommChan;
		bool m_bGotMsgFromMonitorApp;
		unsigned int m_nTickGetFromLastArrivedAck;
		Utils::Timer m_timer;
		std::vector<DataSet> m_Datasets;

		Utils::SignalToken m_communicationToken;
		std::map < std::pair < int, int >, Database::SubscriptionToken> m_tokenMap;
		std::map <int, utils::ref_count_ptr<utils::database::smart_table_callback>> m_TableTokenMap;
		std::vector<uint8_t> m_dataQueryBuffer;
		Logging::Logger LOGGER = Core::Framework::CreateLogger("Monitor", Logging::Severity::ERROR);
		Parsers::BinaryMetadataStore m_store;
	public:
		Monitor(const char* remoteHostName,
			uint16_t remotePort,
			const char* localHostName,
			uint16_t localPort, const std::vector<DataSet>& datasets,
			bool legacyMode = false, core::parsers::binary_metadata_store_interface* store = nullptr) : Dispatcher("Monitor"),
			m_bGotMsgFromMonitorApp(false),
			m_Datasets(datasets),
			m_communicationToken(Utils::SignalTokenUndefined),
			m_store(store)
		{
			if (m_Datasets.size() == 0)
				throw std::runtime_error("Database::Monitor must be initiated with at least 1 dataset");

			if (legacyMode == false)
			{
				m_pCommChan = Communication::CommClientChannel(Communication::Protocols::VariableLengthProtocol::Create(
					Communication::Ports::UdpPort::Create(
						remoteHostName, 
						remotePort, 
						localHostName, 
						localPort), 
					2, 
					0, 
					false, 
					MAX_DATA_SIZE,
					0), MAX_DATA_SIZE, true);
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

		Monitor(const char* remoteHostName,
			uint16_t remotePort,
			const char* localHostName,
			uint16_t localPort, const DataSet& datasets,
			bool legacyMode = false,
			core::parsers::binary_metadata_store_interface* store = nullptr) :
			Monitor(remoteHostName, remotePort, localHostName, localPort, std::vector<DataSet>({ datasets }), legacyMode,store)
		{
		}
		
		~Monitor()
		{
			Disconnect();
		}

		void Disconnect()
		{
			if (m_communicationToken == Utils::SignalTokenUndefined)
				return;

			m_timer.Stop();

			m_pCommChan.OnData() -= m_communicationToken;
			m_pCommChan.Disconnect();

			m_communicationToken = Utils::SignalTokenUndefined;
			UnRegisterAll();
		}

		void Connect()
		{
			Disconnect();

			if (m_pCommChan.Connect() == false)
				throw std::runtime_error("Failed to connect communication channel");

			m_communicationToken = m_pCommChan.OnData() += [this](const DataReader& reader)
			{
				MessageHandler(reader);
			};

			m_timer.Start(1000);
		}

		virtual void Start() override
		{
			Connect();
		}

	private:
		void MessageHandler(const DataReader& reader)
		{
			int	OpCode(0), nTable(0), nRow(0), nDataLength(0);
			OpCode = GetOpcodeExt(reader);
			try
			{

				if (OpCode != MONITOR_GET_KEEP_ALIVE_OPCODE && OpCode != MONITOR_GET_DB_DATA_EX)
				{
					DeToMonitorDataMsgEx& msg = reader.Read<DeToMonitorDataMsgEx>();
                    nTable = static_cast<int>(msg.unTable);
                    nRow = static_cast<int>(msg.unRow);
					nDataLength = msg.shLength;
				}

				switch (OpCode)
				{
				case MONITOR_GET_SINGLE_DATA_EX:
					GetSingleData(nTable, nRow);
					break;

				case MONITOR_REGISTER_MULTI_SINGLE_DATA_OPCODE_EX:
				case MONITOR_GET_MULTI_SINGLE_DATA_OPCODE_EX:
				case MONITOR_UNREGISTER_MULTI_SINGLE_DATA_OPCODE_EX:
				case MONITOR_GET_DEBUG_MULTI_SINGLE_DATA_OPCODE_EX:
					HandleMultiSingleData(reader, OpCode);
					break;

				case MONITOR_GET_KEEP_ALIVE_OPCODE:
					m_nTickGetFromLastArrivedAck = GetTicks();
					break;
				case MONITOR_UPDATE_DATA_EX:
					UpdateDebugValue(nTable, nRow, reader, nDataLength);
					break;
				case MONITOR_REGISTER_SINGLE_DATA_OPCODE_EX:
					RegisterSingleData(nTable, nRow);
					break;

				case MONITOR_UNREGISTER_SINGLE_DATA_OPCODE_EX:
					UnRegisterSingleData(nTable, nRow);
					break;

				case MONITOR_UPDATE_DEBUG_STATE_OPCODE_EX:
					UpdateDebugValue(nTable, nRow, reader, nDataLength, true);
					break;
				case MONITOR_GET_DEBUG_MODE_OPCODE:
					GetDebugState(nTable, nRow);
					break;
				case MONITOR_GET_DB_DATA_EX:
					if (reader.Size() > 2)
					{
						DeToMonitorGetDataMsgEx msg = reader.Read<DeToMonitorGetDataMsgEx>();
                        nTable = static_cast<int>(msg.unTable);
						GetDBData(nTable);
						
					}
					break;
				case MONITOR_GET_ROWS_OPCODE:
					GetRowsForTable(nTable);
					break;
				case MONITOR_STOP_GET_ROWS_OPCODE:
					UnRegisterTableRowsUpdates(nTable);
					break;
				case MONITOR_GET_ROW_SCHEMA_OPCODE:
					GetRowsSchema(nTable, nRow);
					break;
				case MONITOR_GET_ENUM_SCHEMA_OPCODE:
				{
                    size_t len = static_cast<size_t>(nDataLength) - sizeof(DeToMonitorDataMsgEx) + sizeof(int16_t);
					char str[256];
					size_t index = sizeof(DeToMonitorDataMsgEx);
					MEMCPY(str, sizeof(str), ((uint8_t*)reader.Buffer()) + index, len);
					str[len] = 0;
					GetEnumSchema(str);
				}
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
			BuildMonitorKeepAliveMsg();
		}

		int GetOpcodeExt(const DataReader& reader)
		{
			uint8_t* buffer = (uint8_t*)reader.Buffer();
			return buffer[2];
		}

		void GetSingleData(int nTable, int nRow)
		{
			for (auto& dataset : m_Datasets)
			{
				Table table;
				if (dataset.TryGet(nTable, table) == false)
					continue;

				size_t dataSize = table[nRow].DataSize();
				
				if (dataSize > m_dataQueryBuffer.size())
					throw std::runtime_error("Row data size exceeds max message size");				

				std::lock_guard<std::mutex> locker(m_mutex);
				if(dataSize >0)
					table[nRow].Read(m_dataQueryBuffer.data(), dataSize);

                BuildMonitorSingleMsg((uint8_t*)m_dataQueryBuffer.data(), dataSize, static_cast<unsigned int>(nTable), static_cast<unsigned int>(nRow), MONITOR_SINGLE_MSG_OPCODE_EX);
				return;
			}
			throw std::runtime_error("table not found in any datasets");
		}

		void GetDBData(int nTable)
		{
			for (auto& dataset : m_Datasets)
			{
				Table table;
				if (dataset.TryGet(nTable, table) == false)
					continue;

				size_t nNumOfBytesCopied = 0;
				for (int i = 0; i < (int)table.Size(); i++)
				{
					// Assumption:
					// Row tables and rows keys are incremental integers starting from 0 (DE enum convention)
					size_t dataSize = table.GetAt(i).DataSize();
					if (dataSize + nNumOfBytesCopied > m_dataQueryBuffer.size())
						throw std::runtime_error("Row data size exceeds max message size");

					if (dataSize > 0)
					{
						std::lock_guard<std::mutex> locker(m_mutex);
						table.GetAt(i).Read(m_dataQueryBuffer.data() + nNumOfBytesCopied, dataSize);
						nNumOfBytesCopied += dataSize;
					}
				}

                BuildMonitorDBMsg(m_dataQueryBuffer.data(), nNumOfBytesCopied, static_cast<unsigned int>(nTable));
				return;
			}
			throw std::runtime_error("table not found in any datasets");
		}

		void BuildMonitorRowInfoMsg(uint8_t* Values, size_t Length, unsigned int unDB)
		{
			size_t message_length = Length + sizeof(MonitorDBDataMsgEx);
			if (message_length > static_cast<size_t>((std::numeric_limits<short>::max)()))
				throw std::runtime_error("Message length exceeds allowed message size (max short)");

			uint8_t msg[MAX_DATA_SIZE + sizeof(MonitorDBDataMsgEx)];
			MonitorDBDataMsgEx stMsg;

			stMsg.stMsgHeader.shLength = static_cast<short>(Length + sizeof(MonitorDBDataMsgEx));
			stMsg.stMsgHeader.byOpcode = MONITOR_SEND_ROWS_OPCODE;
			stMsg.unTimeTag = GetTicks();
			stMsg.unDBIndex = unDB;

			memcpy(&msg[0], &stMsg, sizeof(MonitorDBDataMsgEx));
			memcpy(&msg[sizeof(MonitorDBDataMsgEx)], Values, Length);
            m_pCommChan.Send((uint8_t*)&msg, static_cast<size_t>(stMsg.stMsgHeader.shLength));
		}
		
		size_t BuildSingleRowDataMsg(const Database::Row& row, std::vector<uint8_t>& buffer, size_t start_index)
		{
			int rowID = static_cast<unsigned int>(row.Key());
			size_t msgLen = start_index;
			int typeLen = 0;
			int nameLen = 0;
			int enumLen = 0;
			size_t rowMetaDataSize = 0;
			char type_name[256];
			char enum_name[256];

			nameLen = static_cast<int>(strlen(row.Info().name));
			if (row.Info().type == core::types::type_enum::ENUM)
			{
				core::types::type_enum type = utils::types::get_type_by_size(row.DataSize(), false);
				utils::types::get_type_name(type, type_name);
				typeLen = static_cast<int>(strlen(type_name));
				STRCPY(enum_name, sizeof(type_name), row.Info().type_name);
				enumLen = static_cast<int>(strlen(row.Info().type_name));
			}
			else
			{
				typeLen = static_cast<int>(strlen(row.Info().type_name));
				STRCPY(type_name, sizeof(type_name), row.Info().type_name);
			}
			rowMetaDataSize = sizeof(rowID) + sizeof(typeLen) + typeLen + sizeof(nameLen) + nameLen;
			if (rowMetaDataSize > buffer.size() -  msgLen)
				return 0;

			MEMCPY(buffer.data() + msgLen, (buffer.size() - msgLen), &rowID, sizeof(rowID));
			msgLen += sizeof(rowID);

			MEMCPY(buffer.data() + msgLen, buffer.size() - msgLen, &typeLen, sizeof(typeLen));
			msgLen += sizeof(typeLen);
			MEMCPY(buffer.data() + msgLen, buffer.size() - msgLen, type_name, typeLen);
			msgLen += typeLen;

			MEMCPY(buffer.data() + msgLen, buffer.size() - msgLen, &nameLen, sizeof(nameLen));
			msgLen += sizeof(nameLen);
			MEMCPY(buffer.data() + msgLen, buffer.size() - msgLen, row.Info().name, nameLen);
			msgLen += nameLen;
			if (enumLen > 0)
			{
				MEMCPY(buffer.data() + msgLen, buffer.size() - msgLen, &enumLen, sizeof(enumLen));
				msgLen += sizeof(enumLen);
				MEMCPY(buffer.data() + msgLen, buffer.size() - msgLen, enum_name, enumLen);
				msgLen += enumLen;
			}
			else
			{
				MEMCPY(buffer.data() + msgLen, buffer.size() - msgLen, &enumLen, sizeof(enumLen));
				msgLen += sizeof(enumLen);
			}

			return msgLen;
		}

		void BuildMonitorDeleteRowMsg(int tableID, int rowID)
		{
			size_t message_length = sizeof(MonitorSingleDataMsgEx);

			static constexpr size_t MAX_SIZE = MAX_DATA_SIZE + sizeof(MonitorSingleDataMsgEx);

			if (message_length > MAX_SIZE ||
				message_length > static_cast<size_t>((std::numeric_limits<short>::max)()))
				throw std::runtime_error("Message length exceeds allowed message size (max short)");

			int8_t msg[MAX_SIZE];
			MonitorSingleDataMsgEx stMsg;
			stMsg.stMsgHeader.shLength = static_cast<short>(sizeof(MonitorSingleDataMsgEx));
			stMsg.stMsgHeader.byOpcode = MONITOR_SEND_DELETE_ROW_OPCODE;
			stMsg.unTimeTag = GetTicks();
			stMsg.unTable = tableID;
			stMsg.unRow = rowID;
			memcpy(&msg[0], &stMsg, sizeof(MonitorSingleDataMsgEx));
	     	m_pCommChan.Send((uint8_t*)&msg, static_cast<size_t>(stMsg.stMsgHeader.shLength));

		}
		void SubscribeRowStatusUpdates(Database::Table& table)
		{
			utils::ref_count_ptr<core::database::table_interface> core_table;
			table.UnderlyingObject(&core_table);

			utils::ref_count_ptr<utils::database::smart_table_callback> data_callback =
				utils::make_ref_count_ptr<utils::database::smart_table_callback>();
			int tableID = static_cast<unsigned int>(table.Key());
			data_callback->row_added += [&,tableID](core::database::row_interface* core_row)
			{
				Database::Row row(core_row);
				Context().BeginInvoke([&,row, tableID]()
					{
						int tableCount = 1;
						std::lock_guard<std::mutex> locker(m_mutex);
						MEMCPY(m_dataQueryBuffer.data(), m_dataQueryBuffer.size(), &tableCount, sizeof(tableCount));
						size_t msgLen = BuildSingleRowDataMsg(row, m_dataQueryBuffer, sizeof(tableCount));
						BuildMonitorRowInfoMsg(m_dataQueryBuffer.data(), msgLen, static_cast<unsigned int>(tableID));
					});
			};

			data_callback->row_removed += [&, tableID](core::database::row_interface* core_row)
			{
				Database::Row row(core_row);
				Context().BeginInvoke([&,row, tableID]()
					{
						int rowID = static_cast<int>(row.Key());
						BuildMonitorDeleteRowMsg(tableID, rowID);
					});
			};

			core_table->subscribe_callback(data_callback);
			std::lock_guard<std::mutex> locker(m_mutex);
			m_TableTokenMap.emplace(tableID, data_callback);
		}

		void GetRowsForTable(int nTable)
		{
			Table table;

			for (auto& dataset : m_Datasets)
			{
				if (dataset.TryGet(nTable, table) == false)
					continue;
				
				int tableCount;
				size_t msgLen = 0;
				size_t index = 0;
				
				
				for (index = 0; index < table.Size(); index++)
				{
					msgLen = 0;
					Database::Row row = table.GetAt(index);
					
					{
						tableCount = 1;
						std::lock_guard<std::mutex> locker(m_mutex);
						msgLen += BuildSingleRowDataMsg(row, m_dataQueryBuffer, sizeof(tableCount));
						MEMCPY(m_dataQueryBuffer.data(), m_dataQueryBuffer.size(), &tableCount, sizeof(tableCount));
						BuildMonitorRowInfoMsg(m_dataQueryBuffer.data(), static_cast<size_t>(msgLen), static_cast<unsigned int>(nTable));
					}
				}
				
				auto it = m_TableTokenMap.find(nTable);
				if (it == m_TableTokenMap.end())
				{
					SubscribeRowStatusUpdates(table);
				}
			}
			
		}

		
		void BuildMonitorRowSchemaMsg(uint8_t* Values, size_t Length, unsigned int table,unsigned int row)
		{
			size_t message_length = Length + sizeof(MonitorSingleDataMsgEx);
			if (message_length > static_cast<size_t>((std::numeric_limits<short>::max)()))
				throw std::runtime_error("Message length exceeds allowed message size (max short)");

			uint8_t msg[MAX_DATA_SIZE + sizeof(MonitorSingleDataMsgEx)];
			MonitorSingleDataMsgEx stMsg;

			stMsg.stMsgHeader.shLength = static_cast<short>(Length + sizeof(MonitorSingleDataMsgEx));
			stMsg.stMsgHeader.byOpcode = MONITOR_SEND_ROW_SCHEMA_OPCODE;
			stMsg.unTimeTag = GetTicks();
			stMsg.unTable = table;
			stMsg.unRow = row;

			MEMCPY(&msg[0],sizeof(msg), &stMsg, sizeof(MonitorSingleDataMsgEx));
			MEMCPY(&msg[sizeof(MonitorSingleDataMsgEx)],sizeof(msg) - sizeof(MonitorSingleDataMsgEx), Values, Length);
            m_pCommChan.Send((uint8_t*)&msg, static_cast<size_t>(stMsg.stMsgHeader.shLength));
		}

		void GetRowsSchema(int TableID,int rowID)
		{
			Table table;
			for (auto& dataset : m_Datasets)
			{
				if (dataset.TryGet(TableID, table) == false)
					continue;
				Row row;
				if(table.TryGet(rowID,row) == false)
					continue;
				if (row.ParserMetadata().Empty())
					return;
				std::string json = row.ParserMetadata().ToJson();
				if (json.empty())
					return;
				uint32_t length = static_cast<uint32_t>(json.length() + 1);
				MEMCPY(m_dataQueryBuffer.data(), m_dataQueryBuffer.size(), &length, sizeof(length));
				MEMCPY(m_dataQueryBuffer.data()+sizeof(length), m_dataQueryBuffer.size(), json.c_str(), length);
                BuildMonitorRowSchemaMsg(m_dataQueryBuffer.data(), static_cast<size_t>(length) + sizeof(length), static_cast<unsigned int>(TableID), static_cast<unsigned int>(rowID));
				break;
			}

		}

		void BuildMonitorEnumSchemaMsg(uint8_t* Values, size_t Length)
		{
			size_t message_length = Length + sizeof(MonitorSingleDataMsgEx);
			if (message_length > static_cast<size_t>((std::numeric_limits<short>::max)()))
				throw std::runtime_error("Message length exceeds allowed message size (max short)");

			uint8_t msg[MAX_DATA_SIZE + sizeof(MonitorSingleDataMsgEx)];
			MonitorSingleDataMsgEx stMsg;

			stMsg.stMsgHeader.shLength = static_cast<short>(Length + sizeof(MonitorSingleDataMsgEx));
			stMsg.stMsgHeader.byOpcode = MONITOR_SEND_ENUM_SCHEMA_OPCODE;
			stMsg.unTimeTag = GetTicks();
			stMsg.unTable = (std::numeric_limits<uint32_t>::max)();
			stMsg.unRow = (std::numeric_limits<uint32_t>::max)();;

			MEMCPY(&msg[0], sizeof(msg), &stMsg, sizeof(MonitorSingleDataMsgEx));
			MEMCPY(&msg[sizeof(MonitorSingleDataMsgEx)], sizeof(msg) - sizeof(MonitorSingleDataMsgEx), Values, Length);
            m_pCommChan.Send((uint8_t*)&msg, static_cast<size_t>(stMsg.stMsgHeader.shLength));
		}

		void GetEnumSchema(const char* enumName)
		{
			if (m_store.Empty())
				return;
			Parsers::EnumData enumData =  m_store.Enum(enumName);
			if (enumData.Empty())
				return;
			std::string json = enumData.ToJson();
			uint32_t length = static_cast<uint32_t>(json.length() + 1);

			std::lock_guard<std::mutex> locker(m_mutex);
			MEMCPY(m_dataQueryBuffer.data(), m_dataQueryBuffer.size(), &length, sizeof(length));
			MEMCPY(m_dataQueryBuffer.data() + sizeof(length), m_dataQueryBuffer.size(), json.c_str(), length);
			BuildMonitorEnumSchemaMsg(m_dataQueryBuffer.data(), length + sizeof(length));
		}
		void HandleMultiSingleData(const DataReader& reader, int OpCode)
		{
			short shLength(0), shNumOfMultiData(0);
			memcpy(&shLength, reader.Buffer(), 2);
            shNumOfMultiData = static_cast<short>(shLength / static_cast<short>(2 * sizeof(int)));
			SingleData stData;

			for (int i(0); i < shNumOfMultiData; i++)
			{
                memcpy(&stData, (char*)(reader.Buffer()) + 3 + static_cast<size_t>(i) * sizeof(SingleData), sizeof(SingleData));

				switch (OpCode)
				{
				case MONITOR_REGISTER_MULTI_SINGLE_DATA_OPCODE_EX:
                    RegisterSingleData(static_cast<int>(stData.unTable), static_cast<int>(stData.unRow));
					break;

				case MONITOR_GET_MULTI_SINGLE_DATA_OPCODE_EX:
                    GetSingleData(static_cast<int>(stData.unTable), static_cast<int>(stData.unRow));
					break;

				case MONITOR_UNREGISTER_MULTI_SINGLE_DATA_OPCODE_EX:
                    UnRegisterSingleData(static_cast<int>(stData.unTable), static_cast<int>(stData.unRow));
					break;

				case MONITOR_GET_DEBUG_MULTI_SINGLE_DATA_OPCODE_EX:
                    GetDebugState(static_cast<int>(stData.unTable), static_cast<int>(stData.unRow));
					break;
				}
			}
		}

		void UpdateDebugValue(int nTable, int nRow, const DataReader & reader, int  nDataLength, bool bSetDebugState = false)
		{
			for (auto& dataset : m_Datasets)
			{
				Table table;
				if (dataset.TryGet(nTable, table) == false)
					continue;

				if (bSetDebugState == false)
				{
					// Debug reports are always reported in highest priority
					const void* buffer = (static_cast<const uint8_t*>(reader.Buffer()) + 11);
					table[nRow].Write(
						buffer,
                        static_cast<size_t>(nDataLength - 9),
						HIGHEST_PRIORITY);
				}
				else
				{
					// Debug lock
					if (table[nRow].WritePriority() == HIGHEST_PRIORITY)
						table[nRow].SetWritePriority(LOWEST_PRIORITY);
					else
						table[nRow].SetWritePriority(HIGHEST_PRIORITY);
				}

				return;
			}
			throw std::runtime_error("table not found in any datasets");
		}

		void RegisterSingleData(int nTable, int nRow)
		{
			std::pair <int, int> p;
			p.first = nTable;
			p.second = nRow;

			if (!IsTokenInMap(nTable, nRow))
			{
				for (auto& dataset : m_Datasets)
				{
					Table table;
					if (dataset.TryGet(nTable, table) == false)
						continue;

					Database::SubscriptionToken token = Subscribe(table[nRow], [this, nTable, nRow](const RowData& data)
					{
						static constexpr size_t MAX_SIZE = MAX_DATA_SIZE + sizeof(MonitorSingleDataMsgEx);

						try
						{
							if (data.DataSize() > MAX_SIZE)
								throw std::runtime_error("Message length exceeds allowed message size (max short)");
							std::lock_guard<std::mutex> locker(m_mutex);
							data.Read(m_dataQueryBuffer.data(), data.DataSize());
							HandleDataBaseUponEventMsg(m_dataQueryBuffer.data(), data.DataSize(), nTable, nRow);
						}
						catch (const std::exception &e)
						{
							LOG_ERROR(LOGGER) << "RegisterSingleData: " << e.what();
						}
					});
					m_tokenMap[p] = token;
					return;
				}
				throw std::runtime_error("table not found in any datasets");
			}
		}

		void HandleDataBaseUponEventMsg(uint8_t* myData, size_t t, int nTable, int nRow)
		{
			try
			{
                BuildMonitorSingleMsg((uint8_t*)myData, t, static_cast<unsigned int>(nTable), static_cast<unsigned int>(nRow), MONITOR_SINGLE_MSG_OPCODE_EX);
			}
			catch (const std::exception& e)
			{
				LOG_ERROR(LOGGER) << "HandleDataBaseUponEventMsg: " << e.what();
			}
		}

		void BuildMonitorKeepAliveMsg()
		{
			if (m_bGotMsgFromMonitorApp && m_pCommChan.Status() == CommStatus::CONNECTED) {

				MonitorKeepAliveMsg keepAliveMsgEx;
				keepAliveMsgEx.stMsgHeader.shLength = sizeof(keepAliveMsgEx);
				keepAliveMsgEx.stMsgHeader.byOpcode = 0x15;
				keepAliveMsgEx.unMonitorInfo = 2;
				m_pCommChan.Send(&keepAliveMsgEx, sizeof(keepAliveMsgEx));
			}
		}

		void BuildMonitorSingleMsg(uint8_t* Values, size_t Length, unsigned int nTable, unsigned int nRow, uint8_t nOpcodeToSend)
		{
			size_t message_length = Length + sizeof(MonitorSingleDataMsgEx);

			static constexpr size_t MAX_SIZE = MAX_DATA_SIZE + sizeof(MonitorSingleDataMsgEx);

			if (message_length > MAX_SIZE ||
				message_length > static_cast<size_t>((std::numeric_limits<short>::max)()))
				throw std::runtime_error("Message length exceeds allowed message size (max short)");

			int8_t msg[MAX_SIZE];
			MonitorSingleDataMsgEx stMsg;
			stMsg.stMsgHeader.shLength = static_cast<short>(Length + sizeof(MonitorSingleDataMsgEx));
			stMsg.stMsgHeader.byOpcode = nOpcodeToSend;
			stMsg.unTimeTag = GetTicks();
			stMsg.unTable = nTable;
			stMsg.unRow = nRow;
			memcpy(&msg[0], &stMsg, sizeof(MonitorSingleDataMsgEx));
			memcpy(&msg[sizeof(MonitorSingleDataMsgEx)], Values, Length);
            m_pCommChan.Send((uint8_t*)&msg, static_cast<size_t>(stMsg.stMsgHeader.shLength));
		}

		void BuildMonitorDBMsg(uint8_t* Values, size_t Length, unsigned int unDB)
		{
			size_t message_length = Length + sizeof(MonitorDBDataMsgEx);
			if (message_length > static_cast<size_t>((std::numeric_limits<short>::max)()))
				throw std::runtime_error("Message length exceeds allowed message size (max short)");

			uint8_t msg[MAX_DATA_SIZE + sizeof(MonitorDBDataMsgEx)];
			MonitorDBDataMsgEx stMsg;

			stMsg.stMsgHeader.shLength = static_cast<short>(Length + sizeof(MonitorDBDataMsgEx));
			stMsg.stMsgHeader.byOpcode = MONITOR_DB_MSG_OPCODE;
			stMsg.unTimeTag = GetTicks();
			stMsg.unDBIndex = (uint8_t)unDB;

			memcpy(&msg[0], &stMsg, sizeof(MonitorDBDataMsgEx));
			memcpy(&msg[sizeof(MonitorDBDataMsgEx)], Values, Length);
            m_pCommChan.Send((uint8_t*)&msg, static_cast<size_t>(stMsg.stMsgHeader.shLength));
		}
		
		void UnRegisterTableRowsUpdates(int tableID)
		{
			for (auto& dataset : m_Datasets)
			{
				Database::Table table;
				if (false == dataset.TryGet(tableID, table))
					continue;
				auto it = m_TableTokenMap.find(tableID);
				if (it != m_TableTokenMap.end())
				{
					utils::ref_count_ptr<core::database::table_interface> core_table;
					table.UnderlyingObject(&core_table);
					core_table->unsubscribe_callback(it->second);
					std::lock_guard<std::mutex> locker(m_mutex);
					m_TableTokenMap.erase(tableID);
					break;
				}
			}
		}
		
		void UnRegisterSingleData(int nTable, int nRow)
		{
			std::pair <int, int> p;
			p.first = nTable;
			p.second = nRow;
			if (IsTokenInMap(nTable, nRow))
			{
				for (auto& dataset : m_Datasets)
				{
					Table table;
					if (dataset.TryGet(nTable, table) == false)
						continue;

					if (Unsubscribe(table[nRow], m_tokenMap[p]) == false)
					{
						LOG_INFO(LOGGER) << "Unsubscribe Db" << nTable << "row =" << nRow << "with token =" << m_tokenMap[p] << "failed";
					}

					m_tokenMap.erase(p);
					return;
				}
				throw std::runtime_error("table not found in any datasets");
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

			while (m_TableTokenMap.size() > 0)
			{
				int tableID = m_TableTokenMap.begin()->first;

				try {
					UnRegisterTableRowsUpdates(tableID);
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

		}

		void GetDebugState(int nTable, int nRow)
		{
			for (auto& dataset : m_Datasets)
			{
				Table table;
				if (dataset.TryGet(nTable, table) == false)
					continue;

				bool bDebugState(false);

				// TODO: Check if MAX_DATA_SIZE is still required
				uint8_t msg[MAX_DATA_SIZE + sizeof(MonitorSingleDataMsgEx)];
				MonitorSingleDataMsgEx stMsg;

				if (table[nRow].WritePriority() == HIGHEST_PRIORITY)
					bDebugState = true;
				stMsg.stMsgHeader.shLength = sizeof(bDebugState) + sizeof(MonitorSingleDataMsgEx);
				stMsg.stMsgHeader.byOpcode = MONITOR_DEBUG_MODE_OPCODE;
				stMsg.unTimeTag = GetTicks();
                stMsg.unTable = static_cast<unsigned int>(nTable);
                stMsg.unRow = static_cast<unsigned int>(nRow);

				memcpy(&msg[0], &stMsg, sizeof(MonitorSingleDataMsgEx));
				msg[sizeof(MonitorSingleDataMsgEx)] = bDebugState;
                m_pCommChan.Send((uint8_t*)&msg, static_cast<size_t>(stMsg.stMsgHeader.shLength));
				return;
			}
			throw std::runtime_error("table not found in any datasets");
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
			MONITOR_SEND_KEEP_ALIVE_OPCODE_EX = 0x15,
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
			MONITOR_SEND_ENUM_SCHEMA_OPCODE = 0x23,
			MONITOR_SEND_DELETE_ROW_OPCODE = 0x24
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
			MONITOR_STOP_GET_ROWS_OPCODE = 0x24,
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
	
		// Database Logger
	class LoggerUnits : public Common::CoreObjectWrapper<core::ref_count_interface>
	{
	private:
		class db_logger : public utils::ref_count_base<core::logging::logger>
		{
		private:
			class empty_dispatcher : public utils::database::database_dispatcher_base<core::application::runnable_interface>
			{
			public:
				empty_dispatcher(utils::dispatcher* context = nullptr) :
					utils::database::database_dispatcher_base<core::application::runnable_interface>(context)
				{
				}
			};

			class dummy_logger : public utils::ref_count_base<core::logging::logger>
			{
			private:
				core::logging::severity m_filter;
				bool m_enabled;

			public:
				dummy_logger(core::logging::severity filter) :
					m_filter(filter),
					m_enabled(true)
				{
				}

				virtual const char* name() const override
				{
					return "DUMMY LOGGER";
				}

				virtual bool enabled() const override
				{
					return m_enabled;
				}

				virtual core::logging::severity filter() const override
				{
					return m_filter;
				}

				virtual void enabled(bool val) override
				{
					m_enabled = val;
				}

				virtual void filter(core::logging::severity severity) override
				{
					m_filter = severity;
				}

                virtual bool log(core::logging::severity, const char*) const override
				{
					return true;
				}

                virtual bool create_stream(core::logging::severity, core::logging::log_stream**) const override
				{
					return false;
				}
			};

			class db_log_stream : public utils::ref_count_base<core::logging::log_stream>
			{
			private:
				utils::ref_count_ptr<const core::logging::logger> m_logger;
				core::logging::severity m_severity;

				std::stringstream m_stream;

			public:
				db_log_stream(const core::logging::logger* logger, core::logging::severity severity) :
					m_logger(logger),
					m_severity(severity)
				{
					if (logger == nullptr)
						throw std::invalid_argument("logger");
				}

				virtual ~db_log_stream()
				{
					m_logger->log(m_severity, m_stream.str().c_str());
				}

				virtual core::logging::severity severity() const override
				{
					return m_severity;
				}

				virtual core::logging::log_stream& operator<< (char c) override
				{
					m_stream << c;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (const char* p) override
				{
					m_stream << p;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (bool value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (signed char value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (unsigned char value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (short value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (unsigned short value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (int value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (unsigned int value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (long value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (unsigned long value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (long long value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (unsigned long long value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (float value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (double value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (long double value) override
				{
					m_stream << value;
					return *this;
				}

				virtual core::logging::log_stream& operator<< (const void* value) override
				{
					m_stream << value;
					return *this;
				}
			};

			utils::ref_count_ptr<core::database::row_interface> m_configuration_row;
			utils::ref_count_ptr<core::database::row_interface> m_logging_row;

			utils::ref_count_ptr<utils::database::subscriber> m_subscriber;
			utils::database::auto_token m_configuration_token;

			utils::ref_count_ptr<core::logging::logger> m_logger;

			Common::CommonTypes::UnitLogConfiguration m_configuration;

			bool m_traceLevelEnabled;

			void init_configuration()
			{
#ifdef _WIN32
				strcpy_s(m_configuration.LOG_UNIT_NAME, name());
#else
                std::strcpy(&(m_configuration.LOG_UNIT_NAME[0]), name());
#endif	

				for (size_t i = 0; i < Common::CommonTypes::UnitLogSeverityEnum::MAX_SEVERITY_NUMBER; i++)
					m_configuration.TRACE_LEVEL_ENABLED[i] = m_traceLevelEnabled;

				m_configuration.SHELL_PRINT_TYPE = Common::CommonTypes::LoggingTypeEnum::HOST_LOG_NO_LOG;
				m_configuration.LOG_TO_UNIT_FILE = false;
				m_configuration.LOG_TO_SYSTEM_FILE = false;
				m_configuration.USE_PRESET_STRING = false;
				m_configuration.ENABLE_TIMESTAMP = false;
				m_configuration.ENABLE_DATE_TIME = false;
			}

			void write_configuration()
			{
				if (m_configuration_row->write_bytes(&m_configuration, sizeof(m_configuration), true, m_configuration_row->write_priority()) == false)
					throw std::runtime_error("Failed to set DB logger configuration");
			}

			void unregister_configuration_handler()
			{
				m_configuration_token.unregister();
			}

			void register_configuration_handler()
			{
				unregister_configuration_handler();
				write_configuration();

				m_configuration_token = m_subscriber->subscribe(m_configuration_row, [this](const utils::database::row_data& data)
				{
					m_configuration = data.read<Common::CommonTypes::UnitLogConfiguration>();

					if (m_configuration.SHELL_PRINT_TYPE != Common::CommonTypes::LoggingTypeEnum::HOST_LOG_NO_LOG ||
						m_configuration.LOG_TO_UNIT_FILE != false ||
						m_configuration.LOG_TO_SYSTEM_FILE != false ||
						m_configuration.USE_PRESET_STRING != false ||
						m_configuration.ENABLE_TIMESTAMP != false ||
						m_configuration.ENABLE_DATE_TIME != false)
					{
						m_configuration.SHELL_PRINT_TYPE = Common::CommonTypes::LoggingTypeEnum::HOST_LOG_NO_LOG;
						m_configuration.LOG_TO_UNIT_FILE = false;
						m_configuration.LOG_TO_SYSTEM_FILE = false;
						m_configuration.USE_PRESET_STRING = false;
						m_configuration.ENABLE_TIMESTAMP = false;
						m_configuration.ENABLE_DATE_TIME = false;

						write_configuration();
					}
				});
			}

		public:
			db_logger(
				core::database::row_interface* configuration_row,
				core::database::row_interface* logging_row,
				core::logging::logger* logger,
				utils::dispatcher* context = nullptr,
				bool traceLevelEnabled = true) :
				m_configuration_row(configuration_row),
				m_logging_row(logging_row),
				m_logger(logger),
				m_configuration({}),
				m_traceLevelEnabled(traceLevelEnabled)
			{
				if (configuration_row == nullptr)
					throw std::invalid_argument("configuration_row");

				if (logging_row == nullptr)
					throw std::invalid_argument("logging_row");

				if (logger == nullptr)
					throw std::invalid_argument("logger");

				m_subscriber = utils::make_ref_count_ptr<utils::database::subscriber>(
					utils::make_ref_count_ptr<empty_dispatcher>(context));

				init_configuration();
				register_configuration_handler();
			}

			db_logger(
				core::database::row_interface* configuration_row,
				core::database::row_interface* logging_row,
				core::logging::severity filter,
				utils::dispatcher* context = nullptr,
				bool traceLevelEnabled = true) :
				db_logger(configuration_row, logging_row, utils::make_ref_count_ptr<dummy_logger>(filter), context, traceLevelEnabled)
			{
			}

			virtual ~db_logger()
			{
				unregister_configuration_handler();
			}

			virtual const char* name() const override
			{
				return "DB LOGGER";
			}

			virtual bool enabled() const override
			{
				return m_logger->enabled();
			}

			virtual core::logging::severity filter() const override
			{
				return m_logger->filter();
			}

			virtual void enabled(bool val) override
			{
				m_logger->enabled(val);
			}

			virtual void filter(core::logging::severity severity) override
			{
				m_logger->filter(severity);
			}

			virtual bool log(core::logging::severity severity, const char* message) const override
			{
				if (message == nullptr)
					return false;

				if (enabled() == false)
					return false;

				if (severity < filter())
					return false;

				static Common::CommonTypes::UnitLogSeverityEnum severities[] =
				{
					Common::CommonTypes::UnitLogSeverityEnum::LOG_TRASH_MSGS,
					Common::CommonTypes::UnitLogSeverityEnum::LOG_DEBUG_MSGS,
					Common::CommonTypes::UnitLogSeverityEnum::LOG_INFO_MSGS,
					Common::CommonTypes::UnitLogSeverityEnum::LOG_WARNING_MSGS,
					Common::CommonTypes::UnitLogSeverityEnum::LOG_ERRORS_MSGS,
					Common::CommonTypes::UnitLogSeverityEnum::LOG_ERRORS_MSGS
				};

				if (m_configuration.TRACE_LEVEL_ENABLED[severities[severity]] == false)
					return false;

				size_t length = std::strlen(message);
				if (length >= sizeof(decltype(Common::CommonTypes::UnitLogDBNotifyData::TRACE_STR)))
					return false;

				Common::CommonTypes::UnitLogDBNotifyData record = {};

				record.TRACE_LEVEL_TYPE = severities[severity];
#ifdef _WIN32
				strcpy_s(record.TRACE_STR, message);
#else
				std::strcpy(&(record.TRACE_STR[0]), message);
#endif			

				if (m_logging_row->write_bytes(&record, sizeof(record), true, m_logging_row->write_priority()) == false)
					return false;

				return m_logger->log(severity, message);
			}

			virtual bool create_stream(core::logging::severity severity, core::logging::log_stream** stream) const override
			{
				if (stream == nullptr)
					return false;

				utils::ref_count_ptr<core::logging::log_stream> instance;

				try
				{
					instance = utils::make_ref_count_ptr<db_log_stream>(this, severity);
				}
				catch (...)
				{
					return false;
				}

				if (instance == nullptr)
					return false;

				*stream = instance;
				(*stream)->add_ref();
				return true;
			}
		};

		class db_logger_units : public utils::ref_count_base<core::ref_count_interface>
		{
		private:
			using loggers_vector = std::vector<utils::ref_count_ptr<core::logging::logger>>;
			utils::thread_safe_object<loggers_vector> m_loggers;

			void get_or_create_rows(
				core::database::table_interface* configuration_table,
				core::database::table_interface* logging_table,
				int index,
				core::database::row_interface** configuration_row,
				core::database::row_interface** logging_row)
			{
				utils::database::buffered_key key(&index, sizeof(index));

				if (configuration_table->query_row(key, configuration_row) == false)
				{
					configuration_table->add_row(key, sizeof(Common::CommonTypes::UnitLogConfiguration));
					if (configuration_table->query_row(key, configuration_row) == false)
						throw std::runtime_error("Failed to add db logger configuration row");
				}

				if (logging_table->query_row(key, logging_row) == false)
				{
					logging_table->add_row(key, sizeof(Common::CommonTypes::UnitLogDBNotifyData));
					if (logging_table->query_row(key, logging_row) == false)
						throw std::runtime_error("Failed to add db logger logging row");
				}
			}

		public:
			db_logger_units(
				core::database::table_interface* configuration_table,
				core::database::table_interface* logging_table,
				int unit_count,
				core::logging::logger* logger,
				bool traceLevelEnabled = true)
			{
				if (configuration_table == nullptr)
					throw std::invalid_argument("configuration_table");

				if (logging_table == nullptr)
					throw std::invalid_argument("logging_table");

				if (unit_count <= 0)
					throw std::invalid_argument("units_count");

				m_loggers.use([&](loggers_vector& loggers)
				{
					utils::ref_count_ptr<utils::dispatcher> context =
						utils::make_ref_count_ptr<utils::dispatcher>("Monitor Logger");

                    loggers.resize(static_cast<size_t>(unit_count));
					for (int i = 0; i < unit_count; i++)
					{
						utils::ref_count_ptr<core::database::row_interface> configuration_row;
						utils::ref_count_ptr<core::database::row_interface> logging_row;
						get_or_create_rows(configuration_table, logging_table, i, &configuration_row, &logging_row);

						utils::database::buffered_key key(&i, sizeof(i));
						loggers[static_cast<size_t>(i)] = utils::make_ref_count_ptr<db_logger>(
							configuration_row,
							logging_row,
							logger,
							context,
							traceLevelEnabled);
					}
				});
			}

			db_logger_units(
				core::database::table_interface* configuration_table,
				core::database::table_interface* logging_table,
				int unit_count,
				core::logging::severity filter,
				bool traceLevelEnabled = true)
			{
				if (configuration_table == nullptr)
					throw std::invalid_argument("configuration_table");

				if (logging_table == nullptr)
					throw std::invalid_argument("logging_table");

				if (unit_count <= 0)
					throw std::invalid_argument("unit_count");

				m_loggers.use([&](loggers_vector& loggers)
				{
					utils::ref_count_ptr<utils::dispatcher> context =
						utils::make_ref_count_ptr<utils::dispatcher>("Monitor Logger");

                    loggers.resize(static_cast<size_t>(unit_count));
					for (int i = 0; i < unit_count; i++)
					{
						utils::ref_count_ptr<core::database::row_interface> configuration_row;
						utils::ref_count_ptr<core::database::row_interface> logging_row;
						get_or_create_rows(configuration_table, logging_table, i, &configuration_row, &logging_row);

						loggers[static_cast<size_t>(i)] = utils::make_ref_count_ptr<db_logger>(
							configuration_row,
							logging_row,
							filter,
							context,
							traceLevelEnabled);
					}
				});
			}

			bool query(int index, core::logging::logger** logger) const
			{
				if (logger == nullptr)
					return false;

				if (index < 0)
					return false;

				return m_loggers.use<bool>([&](const loggers_vector& loggers)
				{
					if (static_cast<size_t>(index) >= loggers.size())
						return false;

					*logger = loggers[static_cast<size_t>(index)];;
					(*logger)->add_ref();
					return true;
				});
			}
		};

		db_logger_units* loggers() const
		{
			return static_cast<db_logger_units*>(static_cast<core::ref_count_interface*>(m_core_object));
		}

	public:
		LoggerUnits()
		{
			// Empty
		}

		LoggerUnits(
			const Database::Table& configurationTable,
			const Database::Table& loggingTable,
			int unitCount,
			const Logging::Logger& logger,
			bool traceLevelEnabled = true) :
			Common::CoreObjectWrapper<core::ref_count_interface>(
				utils::make_ref_count_ptr<db_logger_units>(
					static_cast<core::database::table_interface*>(configurationTable),
					static_cast<core::database::table_interface*>(loggingTable),
					unitCount,
					static_cast<core::logging::logger*>(logger),
					traceLevelEnabled))
		{
		}

		LoggerUnits(
			const Database::Table& configurationTable,
			const Database::Table& loggingTable,
			int unitCount,
			Logging::Severity severity = Logging::Severity::TRACE,
			bool traceLevelEnabled = true) :
			Common::CoreObjectWrapper<core::ref_count_interface>(
				utils::make_ref_count_ptr<db_logger_units>(
					static_cast<core::database::table_interface*>(configurationTable),
					static_cast<core::database::table_interface*>(loggingTable),
					unitCount,
					severity,
					traceLevelEnabled))
		{
		}

		Logging::Logger operator [](int index) const
		{
			ThrowOnEmpty("Database::DBLoggerUnits");

			utils::ref_count_ptr<core::logging::logger> logger;
			if (loggers()->query(index, &logger) == false)
				throw std::runtime_error("Invalid DB logging unit");

			return Logging::Logger(logger);
		}
	};
}
