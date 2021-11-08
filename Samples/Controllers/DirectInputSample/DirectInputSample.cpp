#include "Tables.h"

#include <Application.hpp>
#include <Factories.hpp>
#include <Monitor.hpp>

#include <Controllers/ComplexInputDeviceDB.h>
#include <Controllers/ComplexInputDeviceDefs.h>

#include <limits>

//Should come from a config file in real applications
static constexpr char* HOST_IP = "127.0.0.1";
static constexpr uint16_t HOST_PORT = 4321;
static constexpr char* REMOTE_IP = "127.0.0.1";
static constexpr uint16_t REMOTE_PORT = 1234;

// Joystick config
static constexpr int16_t DEVICE_INDEX = 0;
static constexpr int16_t DEVICE_DEADZONE = 200;

class InputDeviceDispatcher : public Database::Dispatcher
{
private:
	Database::Table m_statusTable;
	Database::Table m_commandsTable;
	Database::SubscriptionToken m_infoToken;

public:
	InputDeviceDispatcher(const Database::Table& statusTable, const Database::Table& commandsTable) :
		m_statusTable(statusTable, nullptr),
		m_commandsTable(commandsTable, nullptr),
		m_infoToken(Database::SubscriptionTokenUndefined)
	{
	}

	virtual void Stop() override
	{
		if (m_infoToken == Database::SubscriptionTokenUndefined)
			return;

		Unsubscribe(m_statusTable[Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceStatusDBEnum::COMPLEX_INPUT_DEVICE_INFO], m_infoToken);
	}

	virtual void Start() override
	{
		m_infoToken = Subscribe(
			m_statusTable[Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceStatusDBEnum::COMPLEX_INPUT_DEVICE_INFO],
			[this](const Database::RowData& data)
		{
			auto info = data.Read<Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo>();			
			Core::Console::ColorPrint(Core::Console::Colors::YELLOW, "Axes state: X: %d, Y: %d, Z: %d\n",
				static_cast<int>(info.xAxisPosition),
				static_cast<int>(info.yAxisPosition),
				static_cast<int>(info.zAxisRotation));
		});
	}
};

class DirectInputSampleBuilder : public Application::Builder
{
private:
	Database::DataSet m_dataset;

protected:
	virtual void BuildEnvironment() override
	{
		// Initialize the application database
		m_dataset = Database::MemoryDatabase::Create("My Database");
		m_dataset.AddTable(MonitorDBIndex::ControllerStatusTable);
		m_dataset.AddTable(MonitorDBIndex::ControllerCommandsTable);
		
		// Add monitor
		AddRunnable<Database::Monitor>(HOST_IP, HOST_PORT, REMOTE_IP, REMOTE_PORT, m_dataset);
	}

	virtual void BuildDispatchers() override
	{
		AddRunnable(Accessories::Controllers::DirectInputDevice::Create(
			m_dataset[MonitorDBIndex::ControllerStatusTable],
			m_dataset[MonitorDBIndex::ControllerCommandsTable],
			DEVICE_DEADZONE,
			DEVICE_INDEX));

		AddRunnable<InputDeviceDispatcher>(
			m_dataset[MonitorDBIndex::ControllerStatusTable],
			m_dataset[MonitorDBIndex::ControllerCommandsTable]);
	}
};

int main(int argc, const char* argv[])
{	 
	Core::Console::ColorPrint(Core::Console::Colors::YELLOW,
		"DirectInput Device Sample\n" \
		"Powered by ezFramework 2.0, version %s\n\n",
		Core::Framework::Version());

	Core::Console::ColorPrint(Core::Console::Colors::WHITE, 
		"Press 'Left Trigger' for left vibration\n" \
		"Press 'Right Trigger' for right vibration\n\n");

	Core::Console::ColorPrint(Core::Console::Colors::CYAN,	"Press 'Enter' key to exit\n");	

	Application::MainApp application(argc, argv);
	return application.BuildAndRun< DirectInputSampleBuilder>();	
}