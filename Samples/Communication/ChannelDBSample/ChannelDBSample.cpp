// CommunicationSample.cpp : Defines the entry point for the console application.

#include "Tables.h"
#include <Common.h>

#include <Application.hpp>
#include <Utils.hpp>
#include <Factories.hpp>
#include <Monitor.hpp>

#include <iostream>

using namespace Core;
using namespace Utils;
using namespace Database;
using namespace Communication;
using namespace Communication::Ports;
using namespace Communication::Protocols;

int RandomInt(int max_value)
{
	return (std::rand() % max_value);
}

struct Message
{
	int length;
	int sourceId;
	int msgId;
	int randomVal1;
	int randomVal2;
};

class GreenDispatcher : public Dispatcher
{

	Table m_incommongDB;
	Table m_outgoingDB;
	SubscriptionToken m_token;
	Timer m_timer;
	int m_messageCounter = 0;

private:
	void HandleIntData(Message msg)
	{

		Console::ColorPrint(
			msg.sourceId == 1 ? Console::Colors::RED : Console::Colors::GREEN,
			"%s: Got message from %s channel. ID: %d, Size: %d, Values: {%d, %d}\n",
			msg.sourceId == 1 ? "RED" : "GREEN", msg.sourceId == 1 ? "GREEN" : "RED", msg.msgId, msg.length, msg.randomVal1, msg.randomVal2);
	}

public:
	GreenDispatcher(Table incommongDB, Table outgoingDB) :
		Dispatcher(),
		m_incommongDB(incommongDB),
		m_outgoingDB(outgoingDB)
	{		
	}

	virtual void Start() override
	{
        m_token = Subscribe(m_incommongDB, CommDB::CommIncomingDB::INCOMING_MESSAGE, [this](const RowData& data)
        {
            Message msg = data.Read<Message>();
            HandleIntData(msg);
        });

        m_timer.Elapsed() += [this]()
        {
            Message msg = { sizeof(Message), 1, ++m_messageCounter , RandomInt(100), RandomInt(100) };
            m_outgoingDB[CommDB::CommOutgoingDB::OUTGOING_MESSAGE].Write<Message>(msg);
        };

		m_timer.Start(1000);
	}

    virtual void Stop() override
    {
        Unsubscribe(m_incommongDB, CommDB::CommIncomingDB::INCOMING_MESSAGE, m_token);
    }
};

class RedDispatcher : public Dispatcher
{
	//DBChannel m_channel;
	Table m_incommongDB;
	Table m_outgoingDB;
	SubscriptionToken m_token;
	Timer m_timer;
	int m_messageCounter = 0;

private:
	void HandleIntData(Message msg)
	{

		Console::ColorPrint(
			msg.sourceId == 1 ? Console::Colors::RED : Console::Colors::GREEN,
			"%s: Got message from %s channel. ID: %d, Size: %d, Values: {%d, %d}\n",
			msg.sourceId == 1 ? "RED" : "GREEN", msg.sourceId == 1 ? "GREEN" : "RED", msg.msgId, msg.length, msg.randomVal1, msg.randomVal2);
	}

public:
	RedDispatcher(Table incommongDB, Table outgoingDB) :
		Dispatcher(),
		m_incommongDB(incommongDB),
		m_outgoingDB(outgoingDB)
	{		
	}

	virtual void Start() override
	{
        m_token = Subscribe(m_incommongDB, CommDB::CommIncomingDB::INCOMING_MESSAGE, [this](const RowData& data)
        {
            Message msg = data.Read<Message>();
            HandleIntData(msg);
        });

        m_timer.Elapsed() += [this]()
        {
            Message msg = { sizeof(Message), 2, ++m_messageCounter , RandomInt(100), RandomInt(100) };
            m_outgoingDB[CommDB::CommOutgoingDB::OUTGOING_MESSAGE].Write<Message>(msg);
        };

		m_timer.Start(1000);
	}

    virtual void Stop() override
    {
        Unsubscribe(m_incommongDB, CommDB::CommIncomingDB::INCOMING_MESSAGE, m_token);
    }
};

//Should come from a config file in real applications
static constexpr size_t MAX_DATA_SIZE = 1024;
static constexpr char const* HOST_IP = "127.0.0.1";
static constexpr uint16_t HOST_PORT = 8765;
static constexpr char const* REMOTE_IP = "127.0.0.1";
static constexpr uint16_t REMOTE_PORT = 5678;

class CommDBBuilder : public Application::Builder
{
private:
	DataSet m_dataset;

public:
	virtual void BuildEnvironment() override
	{
		// Initialize the application database
		m_dataset = MemoryDatabase::Create("My Database");
		// Add monitor
		AddRunnable<Monitor>(HOST_IP, HOST_PORT, REMOTE_IP, REMOTE_PORT, m_dataset);
	}

	virtual void BuildDispatchers() override
	{
		//Setting up the database tables
		m_dataset.AddTable(MonitorDBIndex::GREEN_INCOMMING_TABLE);
		m_dataset.AddTable(MonitorDBIndex::GREEN_OUTGOING_TABLE);

		//Setting up the Communication Channel for green dispatcher
		Port port1 = UdpPort::Create("127.0.0.1", 4321, "127.0.0.1", 1234);
		
		//Example for multicast
        //Port port1 = UdpPort::Create("235.5.5.5", 4321, "0.0.0.0", 1234, true);


		Protocol protocol1 = VariableLengthProtocol::Create(port1, 4, 0, true, MAX_DATA_SIZE);


		//Add Communication Channel as Runnable
		AddRunnable<DBChannelDispatcher>(m_dataset[MonitorDBIndex::GREEN_INCOMMING_TABLE], m_dataset[MonitorDBIndex::GREEN_OUTGOING_TABLE], MAX_DATA_SIZE, protocol1);

		//Create redDispatcher and initiate it with its communication channel
		AddRunnable<GreenDispatcher>(m_dataset[MonitorDBIndex::GREEN_INCOMMING_TABLE], m_dataset[MonitorDBIndex::GREEN_OUTGOING_TABLE]);
		//Setting up the database tables
		m_dataset.AddTable(MonitorDBIndex::RED_INCOMMING_TABLE);
		m_dataset.AddTable(MonitorDBIndex::RED_OUTGOING_TABLE);

		//Setting up the Communication Channel for red dispatcher
		Port port2 = UdpPort::Create("127.0.0.1", 1234, "127.0.0.1", 4321);
		
		//Example for multicast
        //Port port2 = UdpPort::Create("235.5.5.5", 1234, "0.0.0.0", 4321, true);

		Protocol protocol2 = VariableLengthProtocol::Create(port2, 4, 0, true, MAX_DATA_SIZE);


		//Add Communication Channel as Runnable
		AddRunnable<DBChannelDispatcher>(m_dataset[MonitorDBIndex::RED_INCOMMING_TABLE], m_dataset[MonitorDBIndex::RED_OUTGOING_TABLE], MAX_DATA_SIZE, protocol2);

		//Create redDispatcher and initiate it with its communication channel
		AddRunnable<RedDispatcher>(m_dataset[MonitorDBIndex::RED_INCOMMING_TABLE], m_dataset[MonitorDBIndex::RED_OUTGOING_TABLE]);

	}
};

int main(int argc, const char* argv[])
{

	CommDBBuilder builder;
	builder.Build();

	Console::ColorPrint(
		Console::Colors::WHITE, "Waiting for messages, press any key to exit...\n");

	// wait for any key...
	getchar();
	return 0;
}
