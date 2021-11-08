// CommunicationSample.cpp : Defines the entry point for the console application.
//
#include <Core.hpp>
#include <Factories.hpp>
#include <Utils.hpp>

static constexpr size_t MAX_DATA_SIZE = 1024;

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

using namespace Communication;
using namespace Communication::Ports;
using namespace Communication::Protocols;

int main(int argc, const char* argv[])
{	  
	ClientChannel port1 = UdpPort::Create("127.0.0.1", 4321, "127.0.0.1", 1234);
	ClientChannel protocol1 = VariableLengthProtocol::Create(port1, 4, 0, true, MAX_DATA_SIZE);
	ClientChannel port2 = UdpPort::Create("127.0.0.1", 1234, "127.0.0.1", 4321);
	ClientChannel protocol2 = VariableLengthProtocol::Create(port2, 4, 0, true, MAX_DATA_SIZE);

	CommClientChannel greenChannel(protocol1, MAX_DATA_SIZE, true);
	CommClientChannel redChannel(protocol2, MAX_DATA_SIZE, true);
	
	auto message_handler = [&](const DataReader& reader)
	{
		Message& msg = reader.Read<Message>();
		
		Core::Console::ColorPrint(
			msg.sourceId == 1 ? Core::Console::Colors::RED : Core::Console::Colors::GREEN,
			"%s: Got message from %s channel. ID: %d, Size: %d, Values: {%d, %d}\n", 
			msg.sourceId == 1 ? "RED" : "GREEN", msg.sourceId == 1 ? "GREEN" : "RED", msg.msgId, msg.length, msg.randomVal1, msg.randomVal2);
	};

	greenChannel.OnData() += message_handler;
	redChannel.OnData() += message_handler;

	if (greenChannel.Connect() == false || redChannel.Connect() == false)
		throw std::runtime_error("Failed to connect communication channel");

	Utils::Timer timer;
	int message1Counter = 0;
	int message2Counter = 0;
	timer.Elapsed() += [&]()
	{
		Message msg1 = { sizeof(Message), 1, ++message1Counter , RandomInt(100), RandomInt(100) };
		greenChannel.Send(&msg1, sizeof(Message));

		Message msg2 = { sizeof(Message), 2, ++message2Counter , RandomInt(100), RandomInt(100) };
		redChannel.Send(&msg2, sizeof(Message));
	};

	timer.Start(1000);
	Core::Console::ColorPrint(
		Core::Console::Colors::WHITE, "Waiting for messages, press any key to exit...\n");

	// wait for any key...
	getchar();
	return 0;
}
