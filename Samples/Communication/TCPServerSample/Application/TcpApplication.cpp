#include <TCPServerSample/Tables.h>
#include <TCPServerSample/DB_Defs.h>

#include <Application.hpp>
#include <Monitor.hpp>
#include <Dispatchers.hpp>
#include <Factories.hpp>

#include <sstream>

//Should come from a config file in real applications
static constexpr char const* HOST_IP = "127.0.0.1";
static constexpr uint16_t HOST_PORT = 4321;
static constexpr char const* REMOTE_IP = "127.0.0.1";
static constexpr uint16_t REMOTE_PORT = 1234;

static constexpr size_t MAX_DATA_SIZE = 2048;

class ApplicationBuilder : public Application::Builder
{
private:
	Database::DataSet m_dataset;

protected:
	virtual void BuildEnvironment() override
	{
		// Initialize the application database
		m_dataset = Database::MemoryDatabase::Create("My Database");
		m_dataset.AddTable(MonitorDBIndex::ClientIncommingTable);
		m_dataset.AddTable(MonitorDBIndex::ClientOutgoingTable);
		m_dataset.AddTable(MonitorDBIndex::ServersClientIncommingTable);
		m_dataset.AddTable(MonitorDBIndex::ServersClientOutgoingTable);
		
		// Add monitor
		AddRunnable<Database::Monitor>(REMOTE_IP, REMOTE_PORT, HOST_IP, HOST_PORT, m_dataset);
	}

	virtual void BuildDispatchers() override
	{
		Types::Endian endian = Types::Endian::LITTLE;
		
		// create the server comm
		Communication::ServerChannel serverChannel = Communication::Ports::TcpServerPort::Create("0.0.0.0", 2100);		
		AddRunnable<Communication::SingleClientTcpServer>(
			m_dataset[MonitorDBIndex::ServersClientIncommingTable],
			m_dataset[MonitorDBIndex::ServersClientOutgoingTable],
			MAX_DATA_SIZE,
			serverChannel,			
			[&, endian](const ClientChannel& client, size_t maxDataSize)
			{
				Communication::IPClientChannel ipChannel = Communication::IPClientChannel::FromClientChannel(client);
				IPEndPoint localEndPoint = ipChannel.LocalEndPoint();
				IPEndPoint remoteEndPoint = ipChannel.RemoteEndPoint();

				if (localEndPoint.Undefined() == true ||
					remoteEndPoint.Undefined() == true)
					throw std::runtime_error("TCP client with undefined local and/or remote endpoint(s)");

				std::stringstream stream;
				stream << "TCP Client connected: Local end point: " << localEndPoint << ", Remote end point: " << remoteEndPoint;
				Core::Console::ColorPrint(Core::Console::Colors::WHITE, "%s\n", stream.str().c_str());

				return Communication::Protocols::VariableLengthProtocol::Create(client, 2, 3, false, maxDataSize, endian);
			});

		// create the client comm
		Port port = Communication::Ports::TcpPort::Create("127.0.0.1", 2100, "127.0.0.1", 14321);
		Protocol protocol = Communication::Protocols::VariableLengthProtocol::Create(port, 2, 3, false, MAX_DATA_SIZE, endian);
		AddRunnable<DBChannelDispatcher>(
			m_dataset[MonitorDBIndex::ClientIncommingTable],
			m_dataset[MonitorDBIndex::ClientOutgoingTable],
			MAX_DATA_SIZE, 
			protocol);

		// create the server data handler
		AddRunnable<Dispatchers::ServerDataHandler>(m_dataset[MonitorDBIndex::ServersClientIncommingTable], m_dataset[MonitorDBIndex::ServersClientOutgoingTable], endian);
		
		// create the client data handler
		AddRunnable<Dispatchers::ClientDataHandler>(m_dataset[MonitorDBIndex::ClientIncommingTable], m_dataset[MonitorDBIndex::ClientOutgoingTable], endian);
	}
};

int main(int argc, const char* argv[])
{	
	Application::MainApp app(argc, argv);
	return app.BuildAndRun<ApplicationBuilder>();
}