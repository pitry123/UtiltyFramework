///-------------------------------------------------------------------
/// @file	RemoteAgent.hpp.
///
/// @brief	Declares the remote agent class
///-------------------------------------------------------------------
#pragma once
#include <iostream>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <Core.hpp>
#include <Database.hpp>
#include <Communication.hpp>
#include <Utils.hpp>
#include <Common.h>

#ifndef _WIN32
#include <sys/time.h>
#endif

/// @brief	.
using namespace Communication;

#define RA_MAX_BUFF				1350
#define APPMSG_MAX_DATA			500

static constexpr size_t MAX_NAME = 256;
static constexpr size_t MAX_DESCRIPTION = 256;

/// @brief	Defines an alias representing an 8-bit unsigned integer
typedef uint8_t			byte;
/// @brief	Defines an alias representing the data[ appmsg maximum data]
typedef char			Data[APPMSG_MAX_DATA];


namespace Database
{
	///-------------------------------------------------------------------
	/// @brief	table_info - describe all the parameters which table 
	///			is needed for the remote Agent
	/// @date	23/12/2018
	///-------------------------------------------------------------------
	class table_info
	{
	public:

		///-------------------------------------------------------------------
		/// @brief	enum exported_or_imported_flag
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		enum exported_or_imported_flag
		{
			NONE,
			IMPORTED,
			EXPORTED
		};

		///-------------------------------------------------------------------
		/// @brief	constructor
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		table_info() :
			externalId(-1),
			exportedOrImportedFlag(exported_or_imported_flag::NONE)
		{
			memset(name, 0, MAX_NAME);
			memset(description, 0, MAX_DESCRIPTION);
			m_isAcked = false;
		}

		char name[MAX_NAME];
		char description[MAX_DESCRIPTION];
		int externalId; //if this row need to be exported\imported to other application it need the external Id
		exported_or_imported_flag exportedOrImportedFlag;
		bool m_isAcked;
	};

	///-------------------------------------------------------------------
	/// @brief	helpers to describe the table info map and its iterator
	///
	/// @date	23/12/2018
	///-------------------------------------------------------------------
	typedef std::map  <uint32_t, table_info> table_info_map;
	typedef std::pair <uint32_t, table_info> table_info_pair;
	typedef std::map  <uint32_t, table_info>::iterator table_info_iterator;


	///-------------------------------------------------------------------
	/// @brief	A remote agent.
	///
	/// @date	23/12/2018
	///-------------------------------------------------------------------
	class RemoteAgent : public Dispatcher
	{

	private:

		// ---- helper methods to convert data to framework1 -----------------------  

		/// @brief	Values that represent framework 1 cell type enums
		typedef enum FRAMWWORK1_CELL_TYPE_ENUM
		{
			DEBUG_TYPE = -1,
			NOT_A_TYPE = 0,	// Always match
			BYTE_TYPE = 1,
			SHORT_TYPE = 2,
			INT_TYPE = 3,
			FLOAT_TYPE = 4,
			STRUCT_TYPE = 5,
			EMPTY_TYPE = 6,
			BOOL_TYPE = 7,
			DOUBLE_TYPE = 8,
			STRING_TYPE = 9,
			DYNAMIC_LENGTH_TYPE = 10,
			POINTER_TYPE = 11
		} FRAMWWORK1_CELL_TYPE_ENUM;

		///-------------------------------------------------------------------
		/// @brief	Convert type to framework 1
		///
		/// @date	23/12/2018
		///
		/// @param	type	type_enum.
		///
		/// @return	The type converted to framework 1.
		///-------------------------------------------------------------------
		FRAMWWORK1_CELL_TYPE_ENUM convertTypeToFramework1(core::types::type_enum type)
		{

			FRAMWWORK1_CELL_TYPE_ENUM ret;

			switch (type)
			{
			case core::types::type_enum::DEBUG_TYPE:
			case core::types::type_enum::UNKNOWN:
			case core::types::type_enum::BYTE:
			case core::types::type_enum::SHORT:
			case core::types::type_enum::INT32:
			case core::types::type_enum::FLOAT:
			case core::types::type_enum::COMPLEX:
			case core::types::type_enum::EMPTY_TYPE:
			case core::types::type_enum::BOOL:
			case core::types::type_enum::DOUBLE:
			case core::types::type_enum::STRING:
			case core::types::type_enum::BUFFER:
			case core::types::type_enum::POINTER:
			{
				ret = (FRAMWWORK1_CELL_TYPE_ENUM)type;
				break;
			}
			case core::types::type_enum::BITMAP:
			case core::types::type_enum::UINT8:
			case core::types::type_enum::UINT16:
			case core::types::type_enum::UINT32:
			case core::types::type_enum::UINT64:
			case core::types::type_enum::INT8:
			case core::types::type_enum::INT16:
			case core::types::type_enum::CHAR:
			case core::types::type_enum::USHORT:
			case core::types::type_enum::INT64:
			case core::types::type_enum::ENUM:
			case core::types::type_enum::ARRAY:
			default:
			{
				ret = (FRAMWWORK1_CELL_TYPE_ENUM)core::types::type_enum::UNKNOWN;
				break;
			}
			}
			return ret;
		}

		// ---- helper methods to convert data to framework1 End -----//
		
	private:

		/// @brief	The remote agent logger
		Logging::Logger REMOTE_AGENT_LOGGER = Core::Framework::CreateLogger("RemoteAgent", Logging::Severity::DEBUG);

		/// @brief	Size of the maximum data
		static constexpr size_t MAX_DATA_SIZE = (std::numeric_limits<short>::max)();

		/// @brief	The remote agent communication failure timeout in tick
		static constexpr int REMOTE_AGENT_COMMUNICATION_FAILURE_TIMEOUT_IN_TICK = 1000;

		/// @brief	The communications channel
		CommClientChannel m_pCommChan;
		
		/// @brief	True to got message from remote agent application
		bool m_bGotMsgFromRemoteAgentApp;
		
		/// @brief	The tick get from last arrived acknowledge
		unsigned int m_nTickGetFromLastArrivedAck;
		
		/// @brief	The timer
		Utils::Timer m_timer;

		/// @brief	The dataset
		Database::DataSet m_Dataset;
		
		/// @brief	The status database
		Database::Table m_status_db;

		/// @brief	The communication token
		Utils::SignalToken m_communicationToken;
		
		/// @brief	True to communication status
        // bool     m_bCommunicationStatus;
		
		/// @brief	The subscriptions
		Database::SubscriptionsCollector m_subscriptions;

		///-------------------------------------------------------------------
		/// @brief	this map translate from external key of a table to its internal
		/// 		key
		///-------------------------------------------------------------------
		table_info_map m_table_info_map;

		/// @brief	True to send events on 'Write' also when data was not changed
		bool m_force_write;

	public:
		RemoteAgent(
			const char* remoteHostName,
			uint16_t remotePort,
			const char* localHostName,
			uint16_t localPort, 
			const Database::DataSet &datasets,
			const Database::Table& status_db,
			table_info_map &table_info_map,
			bool force_write = false) :
				Dispatcher("RemoteAgent"),
				m_pCommChan(Communication::Protocols::VariableLengthProtocol::Create(
					Communication::Ports::UdpPort::Create(remoteHostName, remotePort, localHostName, localPort), 2, 0, true, MAX_DATA_SIZE, 0), MAX_DATA_SIZE, true),
			m_bGotMsgFromRemoteAgentApp(false),
			m_Dataset(datasets),
			m_status_db(status_db, nullptr),
			m_communicationToken(Utils::SignalTokenUndefined),
			m_table_info_map(table_info_map),
			m_force_write(force_write)
		{

            // Status DB
           Database::Row dbrow;
            if(m_status_db.TryGet(RemoteAgentStatusDBEnum::REMOTE_CHANNEL_COMM_STATUS, dbrow) == false)
            {
                m_status_db.AddRow<int>(RemoteAgentStatusDBEnum::REMOTE_CHANNEL_COMM_STATUS);
                m_status_db.AddRow<int>(RemoteAgentStatusDBEnum::DATA_SYNCED_STS);
                m_status_db.AddRow<int>(RemoteAgentStatusDBEnum::REMOTE_CHANNEL_ENABLE_STS);
                m_status_db.AddRow<Common::CommonTypes::CHARBUFFER>(RemoteAgentStatusDBEnum::IMPORTED_DB_COUPLES_STS);
                m_status_db.AddRow<Common::CommonTypes::CHARBUFFER>(RemoteAgentStatusDBEnum::EXPORTED_DB_COUPLES_STS);
                m_status_db.AddRow<Common::CommonTypes::CHARBUFFER>(RemoteAgentStatusDBEnum::NOT_SYNCTED_DB_STS);
            }


			std::stringstream importedDBCouple;
			std::stringstream exportedDBCouple;

			for (auto& table : m_Dataset)
			{

				table_info_iterator itr = m_table_info_map.find(table.Key());

				if (itr == m_table_info_map.end()) continue;
				table_info tableInfo = itr->second;

				if (tableInfo.exportedOrImportedFlag == table_info::exported_or_imported_flag::NONE)
					continue;

				if (tableInfo.exportedOrImportedFlag == table_info::exported_or_imported_flag::IMPORTED)
					importedDBCouple << tableInfo.externalId << "<--" << (uint32_t)table.Key() << " | ";

				if (tableInfo.exportedOrImportedFlag == table_info::exported_or_imported_flag::EXPORTED)
					exportedDBCouple << tableInfo.externalId << "<--" << (uint32_t)table.Key() << " | ";

				for (auto& row : table)
				{					
					m_subscriptions += Subscribe(row, [this, table, row, tableInfo](const Database::RowData& stData)
					{
						Database::AnyKey rowKey = stData.Key();

						if (table.Key().length != sizeof(unsigned int) ||
							rowKey.length != sizeof(unsigned int))
						{
							return;
						}

						FRAMWWORK1_CELL_TYPE_ENUM rowType = convertTypeToFramework1(row.Info().type);

						RemoteAgentOpcodesEnum forceWrite = (m_force_write) ? RemoteAgentOpcodesEnum::DATA_UPDATE_AND_REPLAY_MSG_OPCODE : DATA_UPDATE_MSG_OPCODE;
                        BuildSingleMsg((uint8_t*)stData.Buffer(), (int32_t)stData.DataSize(), (uint32_t)tableInfo.externalId, (uint32_t)rowKey, rowType, forceWrite, true);
					});
				}
			};

			//insert to the DB the external Id map that we created:
			if (importedDBCouple.str().size() > 0)
			{
				m_status_db[RemoteAgentStatusDBEnum::IMPORTED_DB_COUPLES_STS].Write((void *)importedDBCouple.str().c_str(), importedDBCouple.str().size(), true, 0);
			}
			else
			{
				m_status_db[RemoteAgentStatusDBEnum::IMPORTED_DB_COUPLES_STS].Write("NONE", 4, true, 0);
			}

			if (exportedDBCouple.str().size() > 0)
			{	
				m_status_db[RemoteAgentStatusDBEnum::EXPORTED_DB_COUPLES_STS].Write((void *)exportedDBCouple.str().c_str(), exportedDBCouple.str().size(), true, 0);
			}
			else
			{
				m_status_db[RemoteAgentStatusDBEnum::EXPORTED_DB_COUPLES_STS].Write("NONE", 4, true, 0);
			}
			

			// set the function for the periodic timer
			m_timer.Elapsed() += [this]()
			{
				PeriodicRemoteAgentDataCollection();
			};

			m_status_db[RemoteAgentStatusDBEnum::REMOTE_CHANNEL_COMM_STATUS].Write<int>(EZ_COMMUNICATION_NONE);
		}

		///-------------------------------------------------------------------
		/// @brief	Destructor
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		~RemoteAgent()
		{
			Disconnect();
		}

		///-------------------------------------------------------------------
		/// @brief	Disconnects this object
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		void Disconnect()
		{
			if (m_communicationToken == Utils::SignalTokenUndefined)
				return;

			m_timer.Stop();

			m_pCommChan.OnData() -= m_communicationToken;
			m_pCommChan.Disconnect();

			m_communicationToken = Utils::SignalTokenUndefined;
		}

		///-------------------------------------------------------------------
		/// @brief	Connects this object
		///
		/// @date	23/12/2018
		///
		/// @exception	std::runtime_error	Raised when a runtime error condition
		/// 								occurs.
		///-------------------------------------------------------------------
		void Connect()
		{
			Disconnect();

			if (m_pCommChan.Connect() == false)
				throw std::runtime_error("Failed to connect communication channel");

			m_communicationToken = m_pCommChan.OnData() += [this](const DataReader& reader)
			{
				MessageHandler(reader);
			};

			m_timer.Start(500);
		}

		///-------------------------------------------------------------------
		/// @brief	Starts this object
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		virtual void Start() override
		{
			
			Database::table_info_iterator it;
			for (it = m_table_info_map.begin(); it != m_table_info_map.end(); ++it)
			{
				if (it->second.exportedOrImportedFlag == table_info::exported_or_imported_flag::EXPORTED)
					it->second.m_isAcked = true;
			}
			
			Subscribe(m_status_db[DATA_SYNCED_STS], [&](const Database::RowData& data) {
				Database::table_info_iterator itr;
				bool isSynced = static_cast<bool>(data.Read<int>());
				Common::CommonTypes::CHARBUFFER stData = { 0 };
				size_t nLength;
				if (false == isSynced)
				{
					for (itr = m_table_info_map.begin(); itr != m_table_info_map.end(); ++itr)
					{
						if (itr->second.m_isAcked == false)
						{
							nLength = strlen(stData.buffer);
							if (nLength + 12 < Common::BUFFER_MAX_SIZE)
#ifdef _WIN32
								sprintf_s(&stData.buffer[nLength], Common::BUFFER_MAX_SIZE -nLength, "%d|", itr->first);
#else
								std::sprintf(&stData.buffer[nLength], "%d|", itr->first);
#endif // _WIN32

								
							else if (nLength + 3 < Common::BUFFER_MAX_SIZE)
#ifdef _WIN32
								sprintf_s(&stData.buffer[nLength], Common::BUFFER_MAX_SIZE - nLength, "...");
#else
								std::sprintf(&stData.buffer[nLength], "...");
#endif // _WIN32

								
							
						}
					}
				}
				else
				{
#ifdef _WIN32
					strncpy_s(stData.buffer, "", Common::BUFFER_MAX_SIZE);
#else
					std::strncpy(stData.buffer, "", Common::BUFFER_MAX_SIZE);
#endif // _WIN32
					
				}
				m_status_db[NOT_SYNCTED_DB_STS].Write<Common::CommonTypes::CHARBUFFER>(stData);

			});

			m_status_db[DATA_SYNCED_STS].Write<int>(false, true);

			Connect();
		}

	private:

		///-------------------------------------------------------------------
		/// @brief	Handler, called when the message
		///
		/// @date	23/12/2018
		///
		/// @param	reader	The reader.
		///-------------------------------------------------------------------
		void MessageHandler(const DataReader& reader)
		{

			try 
			{
				int OpCode = 0;

				RemoteAgentDataMsg remoteagentdatamsg = reader.Read<RemoteAgentDataMsg>();
				OpCode = remoteagentdatamsg.stMsgHeader.byOpcode;
			
				switch (OpCode)
				{

				case KEEP_ALIVE_OPCODE:
					m_nTickGetFromLastArrivedAck = GetTicks();
					break;
				case REGISTER_DB_OPCODE:
					if (reader.Size() > 2)
					{
                        HandleRegisterDB(static_cast<int32_t>(remoteagentdatamsg.unDBIndex));
					}
					break;
				case DATA_UPDATE_AND_REPLAY_MSG_OPCODE:

					HandleSingleDataValue(reader);
					break;

				case DATA_UPDATE_MSG_OPCODE:
				case EXPORTED_DB_REPLAY_MSG_OPCODE:
					HandleSingleDataValue(reader);
					break;


				default:
					Core::Console::ColorPrint(Core::Console::Colors::BLUE, "receive unsupported opcode =%d\n", OpCode);
				}
			}
			catch (std::exception &e)
			{
				LOG_ERROR(REMOTE_AGENT_LOGGER) << "MessageHandler Exception on MessageHandler:" << e.what();
			}
		}

		///-------------------------------------------------------------------
		/// @brief	Handles the single data value described by reader
		///
		/// @date	23/12/2018
		///
		/// @param	reader	The reader.
		///-------------------------------------------------------------------
		void HandleSingleDataValue(const DataReader& reader)
		{
			RemoteAgentDataMsg remoteagentdatamsg = reader.Read<RemoteAgentDataMsg>();

			Database::table_info_iterator itr;

			for (itr = m_table_info_map.begin(); itr != m_table_info_map.end(); ++itr)
			{
				uint32_t externalID = static_cast<uint32_t>(itr->second.externalId);
				if(externalID  == remoteagentdatamsg.unDBIndex)
					break;
			}

			if (itr == m_table_info_map.end())
				return;
			

			Database::Table table;
			if (!m_Dataset.TryGet(itr->first, table)) return;

			Database::Row row;
			if (!table.TryGet(remoteagentdatamsg.unDBEntryIndex, row)) return;

			size_t value_size = reader.Size() - sizeof(RemoteAgentDataMsg);
			
			uint8_t buffer[RA_MAX_BUFF];
			memset(buffer, 0, RA_MAX_BUFF);
			memcpy(buffer, reader.Buffer(), reader.Size());
			
			row.Write(&buffer[sizeof(RemoteAgentDataMsg)], value_size, m_force_write, 0);

			if (itr->second.exportedOrImportedFlag == table_info::exported_or_imported_flag::IMPORTED)
				itr->second.m_isAcked = true;
		}

		///-------------------------------------------------------------------
		/// @brief	Periodic remote agent data collection
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		void PeriodicRemoteAgentDataCollection()
		{
			unsigned int temp = GetTicks();

			if ((temp - m_nTickGetFromLastArrivedAck) > REMOTE_AGENT_COMMUNICATION_FAILURE_TIMEOUT_IN_TICK)
			{
				if (true == m_bGotMsgFromRemoteAgentApp)
				{
					HandleRemoteAgentCommunicationError();
				}
			}
			else
			{
				if (false == m_bGotMsgFromRemoteAgentApp)
				{
					HandleRemoteAgentCommunicationResume();
				}

				
				SendRegistrationRequests();
				
			}

			// Send Keep Alive msg
			BuildRemoteAgentKeepAliveMsg();
		}

		///-------------------------------------------------------------------
		/// @brief	Handles the register database described by nDB
		///
		/// @date	23/12/2018
		///
		/// @param	nDB	The database.
		///-------------------------------------------------------------------
		void HandleRegisterDB(int32_t nDB)
		{
			//std::cout << "\n HandleRegisterDB nDB:" << nDB;

			Database::table_info_iterator itr;

			for (itr = m_table_info_map.begin(); itr != m_table_info_map.end(); ++itr)
			{
				if (itr->second.externalId == nDB)
					break;
			}

			if (itr == m_table_info_map.end())
				return;

			for (auto& row : m_Dataset[itr->first])
			{
				uint8_t buffer[2048];
				size_t size = row.DataSize();

				row.Read(&buffer, size);			

				FRAMWWORK1_CELL_TYPE_ENUM rowType = convertTypeToFramework1(row.Info().type);

				RemoteAgentOpcodesEnum forceWrite = (m_force_write) ? RemoteAgentOpcodesEnum::DATA_UPDATE_AND_REPLAY_MSG_OPCODE : DATA_UPDATE_MSG_OPCODE;
                BuildSingleMsg(buffer, (int32_t)row.DataSize(), (uint32_t)nDB, (uint32_t)row.Key(), rowType , forceWrite, true);
			}
		}

		///-------------------------------------------------------------------
		/// @brief	Builds remote agent keep alive message
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		void BuildRemoteAgentKeepAliveMsg()
		{
 			RemoteAgentDataMsg keepAliveMsg;
 			memset(&keepAliveMsg, 0, sizeof(RemoteAgentDataMsg));
 			 
 			keepAliveMsg.stMsgHeader.shLength = sizeof(RemoteAgentDataMsg);
 			keepAliveMsg.stMsgHeader.byOpcode = KEEP_ALIVE_OPCODE;
 			keepAliveMsg.unTimeTag = 0;
 			keepAliveMsg.nCellType = 1;
 
 			m_pCommChan.Send(&keepAliveMsg, sizeof(RemoteAgentDataMsg));
		}

		///-------------------------------------------------------------------
		/// @brief	Builds register message
		///
		/// @date	23/12/2018
		///
		/// @param	unDB	The database Id.
		///-------------------------------------------------------------------
		void BuildRegisterMsg(uint32_t unDB)
		{
			std::cout << "\n BuildRegisterMsg unDB:" << unDB;

			RemoteAgentDataMsg registerDataMsg;
			memset(&registerDataMsg, 0, sizeof(registerDataMsg));

			registerDataMsg.stMsgHeader.shLength = sizeof(RemoteAgentDataMsg);
			registerDataMsg.stMsgHeader.byOpcode = REGISTER_DB_OPCODE;
			registerDataMsg.unDBIndex = unDB;
			registerDataMsg.unTimeTag = 0;

			m_pCommChan.Send(&registerDataMsg, sizeof(RemoteAgentDataMsg));
		}

		///-------------------------------------------------------------------
		/// @brief	Builds single message
		///
		/// @date	23/12/2018
		///
		/// @param [in,out]	Values		 	If non-null, the values.
		/// @param 		   	Length		 	The length.
		/// @param 		   	unDB		 	The un database.
		/// @param 		   	unEntry		 	The un entry.
		/// @param 		   	rowType		 	Type of the row.
		/// @param 		   	nOpcodeToSend	The opcode to send.
		/// @param 		   	bExported	 	True if exported.
		///-------------------------------------------------------------------
		void BuildSingleMsg(uint8_t* Values, int32_t Length, uint32_t unDB, uint32_t unEntry, FRAMWWORK1_CELL_TYPE_ENUM rowType,int32_t nOpcodeToSend, bool bExported)
		{
		//	if (true == m_bCommunicationStatus)
			{
				uint8_t msg[RA_MAX_BUFF + sizeof(RemoteAgentDataMsg)];
				RemoteAgentDataMsg stMsg;

                stMsg.stMsgHeader.shLength = (short)(static_cast<size_t>(Length) + sizeof(RemoteAgentDataMsg));
				stMsg.stMsgHeader.byOpcode = static_cast<byte>(nOpcodeToSend);
				stMsg.unTimeTag = 0; // CallbackTimer::Instance()->GetMills();
				stMsg.unDBIndex = unDB;
				stMsg.unDBEntryIndex = unEntry;
                stMsg.nCellType = static_cast<unsigned int>(rowType);

				memcpy(&msg[0], &stMsg, sizeof(RemoteAgentDataMsg));
                memcpy(&msg[sizeof(RemoteAgentDataMsg)], Values, static_cast<size_t>(Length));
			
                m_pCommChan.Send(&msg, static_cast<size_t>(stMsg.stMsgHeader.shLength));
			}
		}

		///-------------------------------------------------------------------
		/// @brief	Handles the remote agent communication error
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		void HandleRemoteAgentCommunicationError()
		{
			m_bGotMsgFromRemoteAgentApp = false;

			// TODO: Use logger instead of printings...
			Core::Console::ColorPrint(Core::Console::Colors::BLUE, "RemoteAgent Communication Error\n");
			//UnRegisterAll();

			m_pCommChan.Disconnect();
			m_pCommChan.Connect();

			Database::table_info_iterator itr;
			for (itr = m_table_info_map.begin(); itr != m_table_info_map.end(); ++itr)
			{
				if (itr->second.exportedOrImportedFlag == table_info::exported_or_imported_flag::IMPORTED)
					itr->second.m_isAcked = false;
				
			}
			m_status_db[RemoteAgentStatusDBEnum::REMOTE_CHANNEL_COMM_STATUS].Write<int>(EZ_COMMUNICATION_FAILURE);
			m_status_db[DATA_SYNCED_STS].Write<int>(false, true);
			//build registration request to all table that need to be imported
		

		}

		///-------------------------------------------------------------------
		/// @brief	Handles the remote agent communication resume
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		void HandleRemoteAgentCommunicationResume()
		{
			// TODO: Use logger instead of printings...
			
			m_status_db[RemoteAgentStatusDBEnum::REMOTE_CHANNEL_COMM_STATUS].Write<int>(EZ_COMMUNICATION_OK);
			Core::Console::ColorPrint(Core::Console::Colors::BLUE, "REMOTE_AGENT Communication Resume\n");
			m_bGotMsgFromRemoteAgentApp = true;

		}

		///-------------------------------------------------------------------
		/// @brief	Sends the registration requests
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		void SendRegistrationRequests()
		{
			bool isInsync = true;
			for (auto& table : m_Dataset)
			{

				table_info_iterator itr = m_table_info_map.find(table.Key());
				if(itr == m_table_info_map.end())
					continue;

				table_info tableInfo = itr->second;
				if(tableInfo.exportedOrImportedFlag != table_info::exported_or_imported_flag::IMPORTED)
					continue;

				int externalId = tableInfo.externalId;

				if(externalId == -1)
					continue;

				if (tableInfo.m_isAcked == false)
				{
					isInsync = false;
					BuildRegisterMsg((unsigned int)externalId);
				}
			}

			m_status_db[DATA_SYNCED_STS].Write<int>(isInsync,false);
		}


	
#pragma pack(1)

		///-------------------------------------------------------------------
		/// @brief	A remote agent message header.
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		typedef struct RemoteAgentMsgHeader
		{
			/// @brief	Length of the sh
			short shLength;
			/// @brief	The by opcode
			byte  byOpcode;

		}RemoteAgentMsgHeader;

		///-------------------------------------------------------------------
		/// @brief	A remote agent data message.
		///
		/// @date	23/12/2018
		///-------------------------------------------------------------------
		typedef struct RemoteAgentDataMsg
		{
			/// @brief	The message header
			RemoteAgentMsgHeader stMsgHeader;
			/// @brief	Zero-based index of the un database
			uint32_t			 unDBIndex;
			/// @brief	Zero-based index of the un database entry
			uint32_t			 unDBEntryIndex;
			/// @brief	The un time tag
			uint32_t			 unTimeTag;
			/// @brief	Type of the cell
			uint32_t			 nCellType;

		}RemoteAgentDataMsg;


#pragma pack()

		/// @brief	Values that represent remote agent opcodes enums
		typedef enum RemoteAgentOpcodesEnum
		{
			KEEP_ALIVE_OPCODE = 0x05,
			DATA_UPDATE_MSG_OPCODE = 0x06,
			DATA_UPDATE_AND_REPLAY_MSG_OPCODE = 0x07,
			DB_MSG_OPCODE = 0x08,
			REGISTER_DB_OPCODE = 0x09,
			EXPORTED_DB_REPLAY_MSG_OPCODE = 0x0A,

			NUMBER_OF_REMOTE_AGENT_OPCODE

		}RemoteAgentOpcodesEnum;
		
		enum RemoteAgentStatusDBEnum
		{
			REMOTE_CHANNEL_COMM_STATUS,			// t:int
			DATA_SYNCED_STS,					// t:int
			REMOTE_CHANNEL_ENABLE_STS,			// t:int
			IMPORTED_DB_COUPLES_STS,			// t:CommonTypes::CHARBUFFER
			EXPORTED_DB_COUPLES_STS,			// t:CommonTypes::CHARBUFFER
			NOT_SYNCTED_DB_STS,					// t:CommonTypes::CHARBUFFER

			NUM_OF_REMOTE_AGENT_STS
		};

		enum ezCommunicationEnum
		{
			EZ_COMMUNICATION_NONE = -1,
			EZ_COMMUNICATION_FAILURE = 0,
			EZ_COMMUNICATION_OK = 1
		};

		enum ezBoolEnum
		{
			EZ_BOOL_NONE = -1,
			EZ_BOOL_FALSE = 0,
			EZ_BOOL_TRUE = 1
		};

		enum ezStatusEnum
		{
			EZ_STATUS_NONE = -1,
			EZ_STATUS_FAIL = 0,
			EZ_STATUS_OK = 1
		};
	};

}
