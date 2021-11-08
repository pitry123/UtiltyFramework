#pragma once
#include <Utils.hpp>
#include <Database.hpp>
#include <Application.hpp>
#include <Communication.hpp>
#include <TCPServerSample/DB_Defs.h>

#include <random>

namespace Dispatchers
{
	class RandomNumbersDispatcher : public Database::Dispatcher
	{
	protected:
		static inline int RandomInt(int maxValue)
		{
			return (std::rand() % maxValue);
		}

		static inline float RandomFloat(float maxValue)
		{
			return (static_cast<float>(std::rand()) / (static_cast<float>(static_cast<float>(RAND_MAX) / maxValue)));
		}

	public:
		RandomNumbersDispatcher(const char* name = nullptr, bool startSuspended = false) :
			Database::Dispatcher(name, startSuspended)
		{
		}

		virtual ~RandomNumbersDispatcher() = default;
	};

	class ServerDataHandler : public RandomNumbersDispatcher
	{
	private:
		Database::Table m_incommingTable;
		Database::Table m_outgoingTable;
		Database::AutoToken m_rowToken;
		Types::Endian m_endian;

		void HandleMyData(const SampleApplication::MyData1& data)
		{
			Core::Console::ColorPrint(
				Core::Console::Colors::GREEN,
				"Server side: Got data from Row 1 (MyData): {%d, %d, %f, %lf}\n",
				data.val1,
				data.val2,
				data.val3,
				data.val4);

			SampleApplication::MyData2 data2;
			data2.val1 = RandomInt(100);

			if (m_endian != Types::PlatformEndian())
			{
				Types::EndianSwap(data2.header.m_length);
			}

			m_outgoingTable[Communication::CommDB::CommOutgoingDB::OUTGOING_MESSAGE].Write(data2);
		}

	public:
		ServerDataHandler(const Database::Table& incommingTable, const Database::Table& outgoingTable, Types::Endian endian) :
			RandomNumbersDispatcher("Server Dispatcher", true),
			m_incommingTable(incommingTable),
			m_outgoingTable(outgoingTable),
			m_endian(endian)
		{
		}

		ServerDataHandler(const Database::Table& incommingTable, const Database::Table& outgoingTable) :
			ServerDataHandler(incommingTable, outgoingTable, Types::PlatformEndian())
		{
		}

		virtual ~ServerDataHandler()
		{
			Stop();
		}

	protected:
		virtual void Start() override
		{
			Stop();

			m_rowToken = Subscribe(m_incommingTable, Communication::CommDB::CommIncomingDB::INCOMING_MESSAGE, [this](const Database::RowData& data)
			{
				// Here is the event handler handling data update from Row 2
				HandleMyData(data.Read<SampleApplication::MyData1>());
			});
		}

		virtual void Stop() override
		{
			m_rowToken = nullptr;
		}
	};

	class ClientDataHandler : public RandomNumbersDispatcher
	{
	private:
		Database::Table m_incommingTable;
		Database::Table m_outgoingTable;
		Database::AutoToken m_rowToken;
		Utils::AutoTimerToken m_timerToken;
		Types::Endian m_endian;

		void HandleMyData(const SampleApplication::MyData2& data)
		{
			Core::Console::ColorPrint(
				Core::Console::Colors::RED, "Client side: Got data from Row 1 (MyData2): {%d}\n", data.val1);
		}

	public:
		ClientDataHandler(const Database::Table& incommingTable, const Database::Table& outgoingTable, Types::Endian endian) :
			RandomNumbersDispatcher("Client Dispatcher", true),
			m_incommingTable(incommingTable),
			m_outgoingTable(outgoingTable),
			m_endian(endian)
		{
		}

		ClientDataHandler(const Database::Table& incommingTable, const Database::Table& outgoingTable) :
			ClientDataHandler(incommingTable, outgoingTable, Types::PlatformEndian())
		{
		}

		virtual ~ClientDataHandler()
		{
			Stop();
		}

	protected:
		virtual void Start() override
		{
			Stop();

			// Subscribe to Row of incomming data in the data-table
			m_rowToken = Subscribe(m_incommingTable, Communication::CommDB::CommIncomingDB::INCOMING_MESSAGE, [this](const Database::RowData& data)
			{
				// Here is the event handler handling data update from Row 2
				HandleMyData(data.Read<SampleApplication::MyData2>());
			});

			// Start publishing to Row 1 periodically 
			m_timerToken = RegisterTimer(500, [this]()
			{				
				SampleApplication::MyData1 data;
				data.val1 = RandomInt(100);
				data.val2 = RandomInt(100);
				data.val3 = RandomFloat(100);
				data.val4 = RandomFloat(100);

				if (m_endian != Types::PlatformEndian())
				{
					Types::EndianSwap(data.header.m_length);
				}

				m_outgoingTable[Communication::CommDB::CommOutgoingDB::OUTGOING_MESSAGE].Write(data);
			});
		}

		virtual void Stop() override
		{
			m_timerToken = nullptr;
			m_rowToken = nullptr;
		}
	};	
}