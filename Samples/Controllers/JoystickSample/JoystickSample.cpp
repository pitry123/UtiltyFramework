#include "Tables.h"

#include <Application.hpp>
#include <Factories.hpp>
#include <Monitor.hpp>

#include <Controllers/JoystickDB.h>
#include <Controllers/JoystickDefs.h>

#include <limits>

//Should come from a config file in real applications
static constexpr char* HOST_IP = "127.0.0.1";
static constexpr uint16_t HOST_PORT = 4321;
static constexpr char* REMOTE_IP = "127.0.0.1";
static constexpr uint16_t REMOTE_PORT = 1234;

// Joystick config
static constexpr int16_t JOYSTICK_INDEX = 0;
static constexpr int16_t JOYSTICK_DEADZONE = 3000;

class JoystickDispatcher : public Database::Dispatcher
{
private:
	Database::Table m_statusTable;
	Database::Table m_commandsTable;
	Database::SubscriptionToken m_infoToken;

	Accessories::Controllers::JoystickDefs::JoystickDeviceConnectionStatus m_connectionStatus;

public:
	JoystickDispatcher(const Database::Table& statusTable, const Database::Table& commandsTable) :
		m_statusTable(statusTable, nullptr),
		m_commandsTable(commandsTable, nullptr),
		m_infoToken(Database::SubscriptionTokenUndefined),
		m_connectionStatus(Accessories::Controllers::JoystickDefs::JoystickDeviceConnectionStatus::JOYSTICK_DEVICE_DISCONNECTED)
	{
	}

	virtual void Stop() override
	{
		if (m_infoToken == Database::SubscriptionTokenUndefined)
			return;

		Unsubscribe(m_statusTable[Accessories::Controllers::JoystickDB::JoystickStatusDBEnum::JOYSTICK_INFO], m_infoToken);
	}

	virtual void Start() override
	{
		m_infoToken = Subscribe(
			m_statusTable[Accessories::Controllers::JoystickDB::JoystickStatusDBEnum::JOYSTICK_INFO],
			[this](const Database::RowData& data)
		{
			auto info = data.Read<Accessories::Controllers::JoystickDefs::JoystickInfo>();

			if (info.connectionStatus != m_connectionStatus)
			{
				if (info.connectionStatus == Accessories::Controllers::JoystickDefs::JoystickDeviceConnectionStatus::JOYSTICK_DEVICE_CONNECTED)
					Core::Console::ColorPrint(Core::Console::Colors::GREEN, "Joystick Connected!\n");
				else if (info.connectionStatus == Accessories::Controllers::JoystickDefs::JoystickDeviceConnectionStatus::JOYSTICK_DEVICE_DISCONNECTED)
					Core::Console::ColorPrint(Core::Console::Colors::RED, "Joystick Disconnected!\n");

				m_connectionStatus = info.connectionStatus;
			}

			if (info.connectionStatus == Accessories::Controllers::JoystickDefs::JoystickDeviceConnectionStatus::JOYSTICK_DEVICE_DISCONNECTED)
				return;

			Accessories::Controllers::JoystickDefs::JoystickVibration vibrationState = {};
			vibrationState.leftMotor = static_cast<uint16_t>(
				(static_cast<float>(info.leftTrigger) / static_cast<float>((std::numeric_limits<uint8_t>::max)())) * static_cast<float>((std::numeric_limits<uint16_t>::max)()));

			vibrationState.rightMotor = static_cast<uint16_t>(
				(static_cast<float>(info.rightTrigger) / static_cast<float>((std::numeric_limits<uint8_t>::max)())) * static_cast<float>((std::numeric_limits<uint16_t>::max)()));			

			m_commandsTable[Accessories::Controllers::JoystickDB::JoystickCommandsDBEnum::JOYSTICK_VIBRATION].Write(vibrationState, false);
		});
	}
};

class JoystickSampleBuilder : public Application::Builder
{
private:
	Database::DataSet m_dataset;

protected:
	virtual void BuildEnvironment() override
	{
		// Initialize the application database
		m_dataset = Database::MemoryDatabase::Create("My Database");
		m_dataset.AddTable(MonitorDBIndex::JoystickStatusTable);
		m_dataset.AddTable(MonitorDBIndex::JoystickCommandsTable);
		
		// Add monitor
		AddRunnable<Database::Monitor>(HOST_IP, HOST_PORT, REMOTE_IP, REMOTE_PORT, m_dataset);
	}

	virtual void BuildDispatchers() override
	{		
		/*AddRunnable(Accessories::Controllers::LogitechF310::Create(
			m_dataset[MonitorDBIndex::JoystickStatusTable],
			m_dataset[MonitorDBIndex::JoystickCommandsTable],
			JOYSTICK_DEADZONE, 
			JOYSTICK_INDEX));*/

		AddRunnable(Accessories::Controllers::XInputDevice::Create(			
			m_dataset[MonitorDBIndex::JoystickStatusTable],
			m_dataset[MonitorDBIndex::JoystickCommandsTable],
			JOYSTICK_DEADZONE,
			JOYSTICK_INDEX));

		AddRunnable<JoystickDispatcher>(
			m_dataset[MonitorDBIndex::JoystickStatusTable],
			m_dataset[MonitorDBIndex::JoystickCommandsTable]);
	}
};

int main(int argc, const char* argv[])
{	 
	Core::Console::ColorPrint(Core::Console::Colors::YELLOW,
		"Joystick Sample\n" \
		"Powered by ezFramework 2.0, version %s\n\n",
		Core::Framework::Version());

	Core::Console::ColorPrint(Core::Console::Colors::WHITE, 
		"Press 'Left Trigger' for left vibration\n" \
		"Press 'Right Trigger' for right vibration\n\n");

	Core::Console::ColorPrint(Core::Console::Colors::CYAN,	"Press 'Enter' key to exit\n");

	JoystickSampleBuilder builder;
	builder.Build();

	getchar();
	return 0;
}
