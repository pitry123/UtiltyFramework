#include "logitech_f310.h"

#define LOGITECH_MANUFACTURER_ID 1133
#define D_MODE_PID 49686
#define X_MODE_PID 49693

#define POS2AXIS(pos) \
static_cast<int16_t>(((static_cast<float>(pos) / static_cast<float>((std::numeric_limits<WORD>::max)())) * static_cast<float>(MAX_AXIS_VAL * 2.0f)) - static_cast<float>(MAX_AXIS_VAL))

bool accessories::controllers::logitech_f310_impl::accept_device(const JOYCAPS & caps)
{
	if (caps.wMid != static_cast<WORD>(LOGITECH_MANUFACTURER_ID))
		return false;

	accessories::controllers::joystick_mode mode = accessories::controllers::joystick_mode::UNDEFINED;
	if (caps.wPid == static_cast<WORD>(D_MODE_PID))
		mode = accessories::controllers::joystick_mode::D_MODE;
	else if (caps.wPid == static_cast<WORD>(X_MODE_PID))
		mode = accessories::controllers::joystick_mode::X_MODE;

	if (mode == accessories::controllers::joystick_mode::UNDEFINED)
		return false;

	m_mode = mode;
	return true;
}

void accessories::controllers::logitech_f310_impl::convert(const JOYINFOEX & win_info, Accessories::Controllers::JoystickDefs::JoystickInfo & info)
{
	if (m_mode == accessories::controllers::joystick_mode::D_MODE)
	{
		info.leftAnalogX = POS2AXIS(win_info.dwXpos);
		info.leftAnalogY = POS2AXIS(win_info.dwYpos);
		info.rightAnalogX =POS2AXIS(win_info.dwZpos);
		info.rightAnalogY =POS2AXIS(win_info.dwRpos);

		info.pointOfView = static_cast<uint16_t>(win_info.dwPOV);

		info.buttons = static_cast<uint16_t>(win_info.dwButtons);
		info.pushedButtonsCount = static_cast<uint8_t>(win_info.dwButtonNumber);
	}
	else // if (m_mode == accessories::controllers::joystick_mode::X_MODE)
	{
		info.leftAnalogX = POS2AXIS(win_info.dwXpos);
		info.leftAnalogY = POS2AXIS(win_info.dwYpos);
		info.rightAnalogX = POS2AXIS(win_info.dwUpos);
		info.rightAnalogY = POS2AXIS(win_info.dwRpos);

		info.zAxis = -POS2AXIS(win_info.dwZpos);
		info.pointOfView = static_cast<uint16_t>(win_info.dwPOV);

		info.buttons = static_cast<uint16_t>(win_info.dwButtons);
		info.pushedButtonsCount = static_cast<uint8_t>(win_info.dwButtonNumber);
	}
}

accessories::controllers::logitech_f310_impl::logitech_f310_impl(
	core::database::table_interface* status_table,
	core::database::table_interface* commands_table,
	uint16_t deadzone,
	uint32_t device_index,
	double polling_interval_milliseconds) :
	base_class(status_table, commands_table, deadzone, device_index, polling_interval_milliseconds),
	m_mode(joystick_mode::UNDEFINED)
{	
}

bool accessories::controllers::logitech_f310::create(
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

	utils::ref_count_ptr<core::accessories::controllers::joystick_runnable> instance;
	try
	{
		instance = utils::make_ref_count_ptr<accessories::controllers::logitech_f310_impl>(status_table, commands_table, deadzone, device_index, polling_interval_milliseconds);
	}
	catch (.../*std::exception& e*/)
	{
		return false;
	}

	instance->add_ref();
	*joystick = instance;
	return true;
}
