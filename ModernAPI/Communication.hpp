#pragma once
#include <utils/communication.hpp>
#include <utils/strings.hpp>

#include <Common.h>
#include <Common.hpp>
#include <Database.hpp>
#include <Utils.hpp>

#include <array>
#include <algorithm>

namespace Communication
{
	using CommStatus = core::communication::communication_status;
	using CommError = core::communication::communication_error;

	class DataReader
	{
	private:
		const utils::communication::data_reader& m_reader;

		DataReader(const DataReader& other) = delete;			// non construction-copyable
		DataReader& operator=(const DataReader&) = delete;	// non copyable				
		DataReader(const DataReader&& other) = delete;		// non construction-movable
		DataReader& operator=(const DataReader&&) = delete;	// non movable				

	public:
		DataReader(const utils::communication::data_reader& reader) :
			m_reader(reader)
		{
		}

		const void* Buffer() const
		{
			return m_reader.buffer();
		}

		size_t Size() const
		{
			return m_reader.size();
		}

		template <typename T>
		void Read(T& val) const
		{
			m_reader.read<T>(val);
		}

		template <typename T>
		T& Read() const
		{
			return m_reader.read<T>();
		}
	};

	/// A client channel - Modern API wrapper for client_channel_interface. 
	/// Allow management of the client_channel_interface pointer and expose it functions
	/// @date	05/06/2018
	template <typename T>
	class ClientChannelInterface :
		public Common::CoreObjectWrapper<T>
	{
	public:
		ClientChannelInterface()
		{
			// Empty channel
		}

		ClientChannelInterface(T* channel) :
			Common::CoreObjectWrapper<T>(channel)
		{
		}

		ClientChannelInterface(const ClientChannelInterface<T>& other) :
			Common::CoreObjectWrapper<T>(other)
		{
		}

		Communication::CommStatus Status() const
		{
			if (this->Empty() == true)
				throw std::runtime_error("Empty Channel");

			return this->m_core_object->status();
		}

		bool Connect()
		{
			if (this->Empty() == true)
				throw std::runtime_error("Empty Channel");

			return this->m_core_object->connect();
		}

		bool Disconnect()
		{
			if (this->Empty() == true)
				throw std::runtime_error("Empty Channel");

			return this->m_core_object->disconnect();
		}

		size_t Send(const void* buffer, size_t size) const
		{
			if (this->Empty() == true)
				throw std::runtime_error("Empty Channel");

			return this->m_core_object->send(buffer, size);
		}

		size_t Recieve(void* buffer, size_t size, CommError* commError)
		{
			if (this->Empty() == true)
				throw std::runtime_error("Empty Channel");

			return this->m_core_object->recieve(buffer, size, commError);
		}
	};

	class ClientChannel : 
		public ClientChannelInterface<core::communication::client_channel_interface>
	{
	public:
		ClientChannel() :
			ClientChannelInterface<core::communication::client_channel_interface>()
		{
			// Empty channel
		}

		ClientChannel(core::communication::client_channel_interface* channel) :
			ClientChannelInterface<core::communication::client_channel_interface>(channel)
		{
		}

		ClientChannel(const ClientChannel& other) :
			ClientChannelInterface<core::communication::client_channel_interface>(other)
		{
		}
	};

	using IPAddressType = core::communication::ip_address_type;

	struct IPAddress
	{
	private:
		core::communication::ip_address m_address;

	public:
		IPAddress() :
			m_address({})
		{
			// Undefined
		}

		IPAddress(const core::communication::ip_address& address) :
			m_address(address)
		{
		}

		IPAddressType Type() const
		{
			return m_address.type;
		}

		std::array<uint8_t, sizeof(decltype(m_address.val))> Val() const
		{
			std::array<uint8_t, sizeof(decltype(m_address.val))> retval;
			std::copy(std::begin(m_address.val), std::end(m_address.val), retval.begin());
			
			return retval;
		}

		bool Undefined() const
		{
			return (Type() == IPAddressType::IP_ADDRESS_TYPE_UDEFINED);
		}

		core::communication::ip_address UnderlyingObject() const
		{
			return m_address;
		}
	};

	class IPEndPoint
	{
	private:
		core::communication::ip_endpoint m_endPoint;

	public:
		IPEndPoint() :
			m_endPoint({})
		{
			// Undefined
		}

		IPEndPoint(const core::communication::ip_endpoint& endPoint) :
			m_endPoint(endPoint)
		{
		}

		uint16_t Port() const
		{
			return m_endPoint.port;
		}

		IPAddress Address() const
		{
			return m_endPoint.address;
		}

		bool Undefined() const
		{
			return Address().Undefined();
		}

		core::communication::ip_endpoint UnderlyingObject() const
		{
			return m_endPoint;
		}
	};

	class IPClientChannel :
		public ClientChannelInterface<core::communication::ip_client_channel_interface>
	{
	public:
		IPClientChannel() :
			ClientChannelInterface<core::communication::ip_client_channel_interface>()
		{
			// Empty channel
		}

		IPClientChannel(core::communication::ip_client_channel_interface* channel) :
			ClientChannelInterface<core::communication::ip_client_channel_interface>(channel)
		{
		}

		IPClientChannel(const IPClientChannel& other) :
			ClientChannelInterface<core::communication::ip_client_channel_interface>(other)
		{
		}

		IPEndPoint LocalEndPoint()
		{
			ThrowOnEmpty("IPClientChannel");

			core::communication::ip_endpoint endPoint;
			if (m_core_object->query_local_endpoint(endPoint) == false)
				return IPEndPoint(); // Undeined;

			return IPEndPoint(endPoint);
		}

		IPEndPoint RemoteEndPoint()
		{
			ThrowOnEmpty("IPClientChannel");

			core::communication::ip_endpoint endPoint;
			if (m_core_object->query_remote_endpoint(endPoint) == false)
				return IPEndPoint(); // Undeined;

			return IPEndPoint(endPoint);
		}

		static Communication::IPClientChannel FromClientChannel(const Communication::ClientChannel& client)
		{
			return Communication::IPClientChannel(
				static_cast<core::communication::ip_client_channel_interface*>(
					static_cast<core::communication::client_channel_interface*>(client)));
		}
	};

	/// protocol==ClientChannel and Port ==ClientChannel - just help to distinguish between the two and maintain similar terminology as Framework 1.0
	using Protocol = ClientChannel;
	using Port = ClientChannel;

	/// A server channel - Modern API wrapper for async_server_channel.
	/// Allow management of the client_channel_interface pointer and expose it functions
	/// @date	05/06/2018
	class ServerChannel : public Common::CoreObjectWrapper<utils::communication::async_server_channel>
	{
	public:
		using AcceptSignal = Utils::SignalAdapter<
			std::function<void(const ClientChannel&)>,
			utils::communication::async_server_channel,
			core::communication::client_channel_interface*>;

		ServerChannel()
		{
			// Empty Server
		}

		ServerChannel(core::communication::server_channel_interface* server) :
			Common::CoreObjectWrapper<utils::communication::async_server_channel>(utils::make_ref_count_ptr<utils::communication::async_server_channel>(server))
		{
		}

        virtual void UnderlyingObject(utils::communication::async_server_channel** async_server) const override
        {
            Common::CoreObjectWrapper<utils::communication::async_server_channel>::UnderlyingObject(async_server);
        }

        void UnderlyingObject(core::communication::server_channel_interface** core_object) const
        {
            if (m_core_object->query_server(core_object) == false)
				throw std::runtime_error("Failed to query core server");
		}

		AcceptSignal OnAccept()
		{
			ThrowOnEmpty("ServerChannel");
			return AcceptSignal(m_core_object->on_accept);
		}

		bool Connect()
		{
			ThrowOnEmpty("ServerChannel");
			return m_core_object->connect();
		}

		bool Disconnect()
		{
			ThrowOnEmpty("ServerChannel");
			return m_core_object->disconnect();
		}
	};

	/// The communications client channel - Manage the ClientChannel to allow simple interface to sunscribe and unsubscribe 
	/// function to the recive thread, Communication status and communication errors
	///
	/// @date	05/06/2018
	class CommClientChannel : public Common::CoreObjectWrapper<utils::communication::comm_client_channel>
	{
	public:
		using DataReadSignal = utils::signal<utils::communication::comm_client_channel, const utils::communication::data_reader&>;
		using CommStatusSignal = utils::signal<utils::communication::comm_client_channel, const core::communication::communication_status&>;
		using CommErrorSignal = utils::signal<utils::communication::comm_client_channel, const core::communication::communication_error&>;

		CommClientChannel()
		{
			// Empty CommClientChannel
		}

		CommClientChannel(utils::communication::comm_client_channel* channel) :
			CoreObjectWrapper<utils::communication::comm_client_channel>(channel)
		{
		}

		CommClientChannel(const Protocol& client, size_t maxMessageSize, bool automaticReconnect = false) :
			CoreObjectWrapper<utils::communication::comm_client_channel>(utils::make_ref_count_ptr<utils::communication::comm_client_channel>(
				maxMessageSize,
				static_cast<core::communication::client_channel_interface*>(client),
				automaticReconnect))
		{
		}

		size_t MaxMessageSize() const
		{
			ThrowOnEmpty("CommClientChannel");
			return m_core_object->max_message_size();
		}

		Communication::CommStatus Status() const
		{
			ThrowOnEmpty("CommClientChannel");
			return m_core_object->status();
		}

		bool Connect()
		{
			ThrowOnEmpty("CommClientChannel");
			return m_core_object->connect();
		}

		bool Disconnect()
		{
			ThrowOnEmpty("CommClientChannel");
			return m_core_object->disconnect();
		}

		bool Send(const void* buffer, size_t size) const
		{
			ThrowOnEmpty("CommClientChannel");
			return m_core_object->send(buffer, size);
		}

		DataReadSignal& OnData()
		{
			ThrowOnEmpty("CommClientChannel");
			return m_core_object->data_read;
		}

		CommStatusSignal& OnCommStatus()
		{
			ThrowOnEmpty("CommClientChannel");
			return m_core_object->comm_stat;
		}

		CommErrorSignal& OnCommError()
		{
			ThrowOnEmpty("CommClientChannel");
			return m_core_object->comm_err;
		}
	};

	class CommServerChannel :
		public Common::CoreObjectWrapper<utils::communication::comm_server_channel>
	{
	public:
		using AcceptSignal = utils::signal<utils::communication::comm_server_channel, core::communication::client_channel_interface*, size_t>;

		CommServerChannel()
		{
			// Empty Server
		}

		CommServerChannel(utils::communication::comm_server_channel* channel) :
			CoreObjectWrapper<utils::communication::comm_server_channel>(channel)
		{
		}

		CommServerChannel(
			const ServerChannel& server,
			size_t maxMessageSize) :
			Common::CoreObjectWrapper<utils::communication::comm_server_channel>(utils::make_ref_count_ptr<utils::communication::comm_server_channel>(
				static_cast<utils::communication::async_server_channel*>(server), maxMessageSize))
		{
		}

		AcceptSignal& OnAccept() const
		{
			ThrowOnEmpty("CommServerChannel");
			return m_core_object->on_accept;
		}

		bool Connect()
		{
			ThrowOnEmpty("CommServerChannel");
			return m_core_object->connect();
		}

		bool Disconnect()
		{
			ThrowOnEmpty("CommServerChannel");
			return m_core_object->disconnect();
		}
	};

	class CommDB
	{
	public:
		enum CommIncomingDB
		{
			ERROR_CODE,					//communication.h::communication_error	  t:int
			COMM_STATUS,				//communication.h::communication_status    t:int
			INCOMING_MESSAGE,			//t:charbuff[2048]
			INCOMING_DB_SIZE
		};

		enum CommOutgoingDB
		{
			OUTGOINT_CONNECTION_CMD,	//CommonTypes::ActiveInactiveEnum  t:int c:Active or INACTIVE
			OUTGOING_MESSAGE,			//t:charbuff[2048]
			OUTGOING_DB_SIZE
		};

		static void Init(Database::Table& incomingDB, Database::Table& outgoingDB, size_t maxDataSize)
		{
			if (incomingDB.Empty() == true)
				throw std::invalid_argument("incomingDB");

			if (outgoingDB.Empty() == true)
				throw std::invalid_argument("outgoingDB");

			Database::Row row;
			//Config Incoming DB
			if (incomingDB.TryGet(CommDB::CommIncomingDB::INCOMING_MESSAGE, row) == false)
				incomingDB.AddRow(CommDB::CommIncomingDB::INCOMING_MESSAGE, maxDataSize);
			if (incomingDB.TryGet(CommDB::CommIncomingDB::COMM_STATUS, row) == false)
				incomingDB.AddRow<CommStatus>(CommDB::CommIncomingDB::COMM_STATUS);
			if (incomingDB.TryGet(CommDB::CommIncomingDB::ERROR_CODE, row) == false)
				incomingDB.AddRow<CommError>(CommDB::CommIncomingDB::ERROR_CODE);

			//Config Outgoing DB
			if (outgoingDB.TryGet(CommDB::CommOutgoingDB::OUTGOING_MESSAGE, row) == false)
				outgoingDB.AddRow(CommDB::CommOutgoingDB::OUTGOING_MESSAGE, maxDataSize);
			if (outgoingDB.TryGet(CommDB::CommOutgoingDB::OUTGOINT_CONNECTION_CMD, row) == false)
				outgoingDB.AddRow<Common::CommonTypes::ActiveInactiveEnum>(CommDB::CommOutgoingDB::OUTGOINT_CONNECTION_CMD);
		}
	};

	// Forward declaration of DBChannel (friend of DBChannelDispatcher)
	class DBChannel;

	/// A database channel 
	/// This Database allows interaction with communication using a dispatcher
	/// It expect two databases 
	/// IncommingDB - Database to put incoming data in
	/// OutgoingDB - Database to put outgoing data from the logic 
	/// @date	05/06/2018
	class DBChannelDispatcher : public Database::Dispatcher
	{
		friend class DBChannel;

	private:
		Database::Table m_incomingDB;
		Database::Table m_outgoingDB;
		CommClientChannel m_commChannel;

		Utils::SignalToken m_incomingToken;
		Utils::SignalToken m_statusToken;
		Utils::SignalToken m_errorToken;	

		Database::AutoToken m_cmdToken;
		Database::AutoToken m_outgoingToken;
		
		size_t m_maxDataSize;

		void UnregisterHandlers()
		{
			m_outgoingToken.Unregister();
			m_cmdToken.Unregister();

			if (m_errorToken != Utils::SignalTokenUndefined)
				m_commChannel.OnCommError() -= m_errorToken;

			if (m_statusToken != Utils::SignalTokenUndefined)
				m_commChannel.OnCommStatus() -= m_statusToken;

			if (m_incomingToken != Utils::SignalTokenUndefined)
				m_commChannel.OnData() -= m_incomingToken;			

			m_errorToken = Utils::SignalTokenUndefined;
			m_statusToken = Utils::SignalTokenUndefined;
			m_incomingToken = Utils::SignalTokenUndefined;			
		}

		void RegisterHandlers()
		{
			m_outgoingToken = m_outgoingDB[CommDB::CommOutgoingDB::OUTGOING_MESSAGE].Subscribe([this](const Database::RowData& reader)
			{
				size_t data_size = reader.DataSize();
				if (data_size > m_commChannel.MaxMessageSize())
					throw std::runtime_error("Unexpected data size");
				if (m_commChannel.Status() == CommStatus::DISCONNECTED)
				{
					//TODO:Write to log
				}
				else
				{
					if (m_commChannel.Send(reader.Buffer(), data_size) == false)
					{
						//set status to fail due to transmission failure
						m_incomingDB[CommDB::CommIncomingDB::COMM_STATUS].Write<Common::CommonTypes::StatusEnum>(Common::CommonTypes::StatusEnum::STATUS_FAIL);
					}
				}
			});

			m_incomingToken = m_commChannel.OnData() += [this](const DataReader& reader)
			{
				if (reader.Size() > m_commChannel.MaxMessageSize())
				{
					m_incomingDB[CommDB::CommIncomingDB::COMM_STATUS].Write<Common::CommonTypes::StatusEnum>(Common::CommonTypes::StatusEnum::STATUS_FAIL);
					return;
				}

				m_incomingDB[CommDB::CommIncomingDB::INCOMING_MESSAGE].Write(reader.Buffer(), reader.Size());
			};

			m_statusToken = m_commChannel.OnCommStatus() += [this](const CommStatus& status)
			{
				m_incomingDB[CommDB::CommIncomingDB::COMM_STATUS].Write(status, false);
			};

			m_errorToken = m_commChannel.OnCommError() += [this](const CommError& error)
			{
				m_incomingDB[CommDB::CommIncomingDB::ERROR_CODE].Write(error, false);
			};

			m_cmdToken = m_outgoingDB[CommDB::CommOutgoingDB::OUTGOINT_CONNECTION_CMD].Subscribe([this](const Database::RowData& reader)
			{
				Common::CommonTypes::ActiveInactiveEnum cmd = reader.Read<Common::CommonTypes::ActiveInactiveEnum>();

				if (cmd == Common::CommonTypes::ActiveInactiveEnum::ACTIVE)
				{
					m_commChannel.Connect();
				}
				else
				{
					m_commChannel.Disconnect();
				}

				m_incomingDB[CommDB::CommIncomingDB::COMM_STATUS].Write(m_commChannel.Status());
			});
		}
		
		Database::Table& IncomingDB()
		{
			return m_incomingDB;
		}

		Database::Table& OutgoingDB()
		{
			return m_outgoingDB;
		}

	public:
		DBChannelDispatcher(Database::Table incomingDB, Database::Table outgoingDB, size_t maxDataSize, const CommClientChannel& commChannel) :
			m_incomingDB(incomingDB),
			m_outgoingDB(outgoingDB),
			m_commChannel(commChannel),
			m_incomingToken(Utils::SignalTokenUndefined),
			m_statusToken(Utils::SignalTokenUndefined),
			m_errorToken(Utils::SignalTokenUndefined),			
			m_maxDataSize(maxDataSize)
		{
			if (incomingDB.Empty() == true)
				throw std::invalid_argument("incomingDB");

			if (outgoingDB.Empty() == true)
				throw std::invalid_argument("outgoingDB");

			if (maxDataSize == 0)
				throw std::invalid_argument("maxDataSize");			
		}

		/// Constructor for DB channel
		///
		/// @date	05/06/2018
		///
		/// @exception	std::invalid_argument	Thrown when an IcommingDB or outgoingDB are empty or maxDataSize is 0
		/// 	error condition occurs.
		/// @exception	std::runtime_error   	Raised when a runtime error
		/// 	condition occurs.
		///
		/// @param	incomingDB 	The incoming database.
		/// @param	outgoingDB 	The outgoing database.
		/// @param	maxDataSize	Size of the maximum data.
		/// @param	commChannel	The communications channel.
		DBChannelDispatcher(Database::Table incomingDB, Database::Table outgoingDB, size_t maxDataSize, const ClientChannel& commChannel, bool automaticReconnect = true) :
			DBChannelDispatcher(incomingDB, outgoingDB, maxDataSize, CommClientChannel(commChannel, maxDataSize, automaticReconnect))
		{
		}

		~DBChannelDispatcher()
		{
			Stop();
		}

		virtual void Init() override
		{
			Communication::CommDB::Init(m_incomingDB, m_outgoingDB, m_maxDataSize);
		}

		/// Starts The Dispatcher - Call Connect (override of Runnable)
		///
		/// @date	12/06/2018
		virtual void Start() override
		{			
			RegisterHandlers();			
		}

		virtual void Started() override
		{			
			if (m_commChannel.Connect() == false)
				throw std::runtime_error("Failed to connect");
		}


		/// Stops The Dispatcher - Call Connect (override of Runnable)
		///
		/// @date	12/06/2018
		virtual void Stop() override
		{
			UnregisterHandlers();
			if (m_commChannel.Disconnect() == false)
				throw std::runtime_error("Failed to disconnect");

			Context().Sync();
		}
	};

	class DBChannel : public Common::CoreObjectWrapper<DBChannelDispatcher>
	{
	public:
		DBChannel()
		{
			// Empty DB Channel
		}

		DBChannel(DBChannelDispatcher* channel) :
			Common::CoreObjectWrapper<DBChannelDispatcher>(channel)
		{
			if (Empty() == false)
				m_core_object->Init();
		}

		DBChannel(
			Database::Table incomingDB,
			Database::Table outgoingDB,
			size_t maxDataSize,
			const CommClientChannel& commChannel) :
			DBChannel(utils::make_ref_count_ptr<DBChannelDispatcher>(incomingDB, outgoingDB, maxDataSize, commChannel))
		{
		}

		DBChannel(
			Database::Table incomingDB,
			Database::Table outgoingDB,
			size_t maxDataSize,
			const ClientChannel& commChannel,
			bool automaticReconnect = false) :
			DBChannel(incomingDB, outgoingDB, maxDataSize, CommClientChannel(commChannel, maxDataSize, automaticReconnect))
		{
		}		

		Database::Table& IncomingDB()
		{
			ThrowOnEmpty("DBChannel");
			return m_core_object->IncomingDB();
		}

		Database::Table& OutgoingDB()
		{
			ThrowOnEmpty("DBChannel");
			return m_core_object->OutgoingDB();
		}

		bool Connect()
		{
			ThrowOnEmpty("DBChannel");

			try
			{
				m_core_object->Start();
				m_core_object->Started();
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		/// Disconnects communication
		///
		/// @date	12/06/2018
		///
		/// @return	True if it succeeds, false if it fails.
		bool Disconnect()
		{			
			ThrowOnEmpty("DBChannel");

			try
			{
				m_core_object->Stop();
				m_core_object->Stopped();
			}
			catch (...)
			{
				return false;
			}

			return true;
		}
	};

	class SingleClientTcpServer : public Application::RunnableBase
	{
	private:
		Database::Table m_incoming;
		Database::Table m_outgoing;

		size_t m_maxDataSize;
		Communication::ServerChannel m_serverChannel;		
		std::function<ClientChannel(const ClientChannel&, size_t)> m_onClient;

		Communication::CommServerChannel m_server;
		Communication::DBChannel  m_client;

	public:
		SingleClientTcpServer(
			const Database::Table& incoming,
			const Database::Table& outgoing,
			size_t maxDataSize,
			const Communication::ServerChannel& serverChannel,			
			const std::function<ClientChannel(const ClientChannel&, size_t)>& onClient) :
			m_incoming(incoming),
			m_outgoing(outgoing),
			m_maxDataSize(maxDataSize),
			m_serverChannel(serverChannel),			
			m_onClient(onClient)
		{
			if (serverChannel.Empty() == true)
				throw std::invalid_argument("serverChannel");

			if (maxDataSize == 0)
				throw std::invalid_argument("maxDataSize");

			if (onClient == nullptr)
				throw std::invalid_argument("onClient");
		}

		virtual void Init() override
		{
			Communication::CommDB::Init(m_incoming, m_outgoing, m_maxDataSize);
			m_server = Communication::CommServerChannel(m_serverChannel, m_maxDataSize);

			m_server.OnAccept() += [this](const Communication::ClientChannel& client, size_t maxMessageSize)
			{
				Communication::ClientChannel protocol = m_onClient(client, maxMessageSize);
				m_client = Communication::DBChannel(m_incoming, m_outgoing, maxMessageSize, protocol);
				m_client.Connect();
			};
		}

		virtual void Start() override
		{
			m_server.Connect();
		}

		virtual void Stop() override
		{
			m_server.Disconnect();
		}		
	};

	// ModernAPI helpers/translators

	/// @class	ClientChannelBase
	/// @brief	Modern API implementation for client_channel_interface
	/// @date	12/08/2019
	class ClientChannelBase : public utils::ref_count_base<core::communication::client_channel_interface>
	{
	public:
		/// @brief	Default Destructor
		virtual ~ClientChannelBase() = default;

		/// @brief	Redefined 'status' method as modern API
		/// @return	Communication Status.
		virtual Communication::CommStatus Status() const = 0;

		/// @brief	Redefined 'connect' method as modern API
		/// @return	True if it succeeds, false if it fails.
		virtual bool Connect() = 0;

		/// @brief	Redefined 'disconnect' method as modern API
		/// @return	True if it succeeds, false if it fails.
		virtual bool Disconnect() = 0;

		/// @brief	Redefined 'send' method as modern API
		/// @param	buffer	Buffer to send
		/// @param	size  	Buffer size to send
		/// @return	Actual size of data that was sent
		virtual size_t Send(const void* buffer, size_t size) const = 0;

		/// @brief	Redefined 'receive' method as modern API
		/// @param [in,out]	buffer   	Buffet to fill with message data
		/// @param 		   	size	 	Max size of buffer
		/// @param [in,out]	commError	Communication Error to set/reset
		/// @return	Actual received message size
		virtual size_t Recieve(void* buffer, size_t size, Communication::CommError* commError) = 0;

	private:
		// --------------------------------------------------------------------
		// Implement abstract API of base class to call my abstract API
		// --------------------------------------------------------------------
		virtual core::communication::communication_status status() const override
		{
			return Status();
		}

		virtual bool connect() override
		{
			return Connect();
		}

		virtual bool disconnect() override
		{
			return Disconnect();
		}

		virtual size_t send(const void* buffer, size_t size) const override
		{
			return Send(buffer, size);
		}

		virtual size_t recieve(void* buffer, size_t size, core::communication::communication_error* commError) override
		{
			return Recieve(buffer, size, commError);
		}
	};
}

template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const Communication::IPAddress& address)
{
	return (os << address.UnderlyingObject());
}

template <typename CharT>
inline std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const Communication::IPEndPoint& endPoint)
{
	return (os << endPoint.UnderlyingObject());
}