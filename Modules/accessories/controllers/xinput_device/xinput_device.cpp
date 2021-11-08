#include "xinput_device.h"
#include <limits>
#include <bitset>

#define DIGITAL_UP 1
#define DIGITAL_DOWN 2
#define DIGITAL_LEFT 4
#define DIGITAL_RIGHT 8

#define POS2AXIS(val) \
((val < 0) ? \
-static_cast<int16_t>((static_cast<float>(val) / static_cast<float>((std::numeric_limits<SHORT>::min)())) * static_cast<float>(MAX_AXIS_VAL)) : \
static_cast<int16_t>((static_cast<float>(val) / static_cast<float>((std::numeric_limits<SHORT>::max)())) * static_cast<float>(MAX_AXIS_VAL)));

void accessories::controllers::xinput_device_impl::convert(const XINPUT_STATE & xinfo, Accessories::Controllers::JoystickDefs::JoystickInfo & info)
{
	info.leftAnalogX = POS2AXIS(xinfo.Gamepad.sThumbLX);
	info.leftAnalogY = -POS2AXIS(xinfo.Gamepad.sThumbLY);
	info.rightAnalogX = POS2AXIS(xinfo.Gamepad.sThumbRX);
	info.rightAnalogY = -POS2AXIS(xinfo.Gamepad.sThumbRY);

	uint16_t buttons = static_cast<uint16_t>(xinfo.Gamepad.wButtons);
	uint16_t pov_buttons = ((DIGITAL_UP | DIGITAL_DOWN | DIGITAL_LEFT | DIGITAL_RIGHT) & buttons);
	if (pov_buttons > 0)
	{
		uint32_t pov = 0;
		uint32_t direction_count = 0;
		if ((pov_buttons & DIGITAL_UP) > 0)
		{
			pov += 0;
			++direction_count;
		}
		else if ((pov_buttons & DIGITAL_DOWN) > 0)
		{
			pov += 18000;
			++direction_count;
		}

		if ((pov_buttons & DIGITAL_LEFT) > 0)
		{
			if ((pov_buttons & DIGITAL_UP) > 0)
				pov += 36000;

			pov += 27000;
			++direction_count;
		}
		else if ((pov_buttons & DIGITAL_RIGHT) > 0)
		{
			pov += 9000;
			++direction_count;
		}

		info.pointOfView = static_cast<uint16_t>((pov / direction_count) % 36000);
		buttons ^= pov_buttons;
	}

	info.leftTrigger = static_cast<uint8_t>(xinfo.Gamepad.bLeftTrigger);
	info.rightTrigger = static_cast<uint8_t>(xinfo.Gamepad.bRightTrigger);

	info.buttons = buttons;
	info.pushedButtonsCount = static_cast<uint8_t>(std::bitset<16>(buttons).count());
}

void accessories::controllers::xinput_device_impl::vibrate(uint16_t left_motor, uint16_t right_motor)
{
	XINPUT_VIBRATION vibration = {};
	vibration.wLeftMotorSpeed = left_motor;
	vibration.wRightMotorSpeed = right_motor;

	// Vibrate the controller
	XInputSetState(m_device_index, &vibration);
}

void accessories::controllers::xinput_device_impl::on_start()
{
	m_set_vibration = true;

	Accessories::Controllers::JoystickDB::JoystickCommandsDBEnum vibration_row_key = Accessories::Controllers::JoystickDB::JoystickCommandsDBEnum::JOYSTICK_VIBRATION;
	m_vibration_token = subscribe(
		m_commands_table, 
		utils::database::buffered_key(&vibration_row_key, sizeof(vibration_row_key)), 
		[this](const utils::database::row_data& data)
	{
		Accessories::Controllers::JoystickDefs::JoystickVibration vibration = 
			data.read<Accessories::Controllers::JoystickDefs::JoystickVibration>();

		m_set_vibration = true;
		m_left_motor_vibration = vibration.leftMotor;
		m_right_motor_vibration = vibration.rightMotor;
	});
}

void accessories::controllers::xinput_device_impl::on_stop()
{
	if (m_vibration_token == utils::database::subscription_token_undefined)
		return;	

	Accessories::Controllers::JoystickDB::JoystickCommandsDBEnum vibration_row_key = Accessories::Controllers::JoystickDB::JoystickCommandsDBEnum::JOYSTICK_VIBRATION;
	unsubscribe(
		m_commands_table, 
		utils::database::buffered_key(&vibration_row_key, sizeof(vibration_row_key)), 
		m_vibration_token);

	vibrate(0, 0);
}

bool accessories::controllers::xinput_device_impl::get_info(Accessories::Controllers::JoystickDefs::JoystickInfo & info)
{
	// Get the state
	XINPUT_STATE state = {};
	DWORD result = XInputGetState(static_cast<DWORD>(m_device_index), &state);
	if (FAILED(result))
		return false;

	if (result == ERROR_DEVICE_NOT_CONNECTED)
	{
		info = {};
		info.connectionStatus = Accessories::Controllers::JoystickDefs::JOYSTICK_DEVICE_DISCONNECTED;
		return true;
	}
	else
	{
		info.connectionStatus = Accessories::Controllers::JoystickDefs::JOYSTICK_DEVICE_CONNECTED;
	}

	if (m_set_vibration == true)
	{
		m_set_vibration = false;
		vibrate(m_left_motor_vibration, m_right_motor_vibration);
	}

	convert(state, info);
	return true;
}

accessories::controllers::xinput_device_impl::xinput_device_impl(
	core::database::table_interface* status_table,
	core::database::table_interface* commands_table,
	uint16_t deadzone,
	uint32_t device_index,
	double polling_interval_milliseconds) :
	accessories::controllers::joystick_base<accessories::controllers::xinput_device>(status_table, commands_table, deadzone, polling_interval_milliseconds),
	m_device_index(device_index),
	m_set_vibration(false),
	m_left_motor_vibration(0),
	m_right_motor_vibration(0),
	m_vibration_token(utils::database::subscription_token_undefined)
{
}

bool accessories::controllers::xinput_device::create(
	core::database::table_interface* status_table,
	core::database::table_interface* commands_table,
	uint16_t deadzone,
	uint32_t device_index,
	double polling_interval_milliseconds,
	core::accessories::controllers::joystick_runnable** joystick)
{
	if (joystick == nullptr)
		return false;

	if (status_table == nullptr)
		return false;

	if (commands_table == nullptr)
		return false;

	if (device_index > 3)
		return false;

	utils::ref_count_ptr<core::accessories::controllers::joystick_runnable> instance;
	try
	{
		instance = utils::make_ref_count_ptr<accessories::controllers::xinput_device_impl>(status_table, commands_table, deadzone, device_index, polling_interval_milliseconds);
	}
	catch (.../*std::exception& e*/)
	{
		return false;
	}

	instance->add_ref();
	*joystick = instance;
	return true;
}
