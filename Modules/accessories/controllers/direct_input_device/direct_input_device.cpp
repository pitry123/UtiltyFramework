#include "direct_input_device.h"
#include <limits>
#include <bitset>
#include <cmath>
#include <cstring>

#define DIRECT_INPUT_UNDEFINED_POD (std::numeric_limits<unsigned long>::max)()
#define TAKE_DEVICE_TIMOUT_ON_FAILURE 10000 // milliseconds

BOOL accessories::controllers::direct_input_device_impl::device_enumerator_callback(const DIDEVICEINSTANCE* instance, VOID* context)
{
	accessories::controllers::direct_input_device_impl* obj = static_cast<accessories::controllers::direct_input_device_impl*>(context);
	if (obj->m_device_name.empty() == false)
		if (std::strcmp(instance->tszProductName, obj->m_device_name.c_str()) != 0)
			return DIENUM_CONTINUE;

	CComPtr<IDirectInputDevice8> joystick;
	HRESULT hr = obj->m_direct_input->CreateDevice(instance->guidInstance, &joystick, NULL);
	
	if (FAILED(hr))
		return DIENUM_CONTINUE;

	if (obj->m_device_index_iterator > 0)
	{
		--obj->m_device_index_iterator;
		return DIENUM_CONTINUE;
	}

	obj->m_direct_input_device = joystick;
	return DIENUM_STOP;
}

BOOL accessories::controllers::direct_input_device_impl::axes_enumerator_callback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context)
{
	accessories::controllers::direct_input_device_impl* obj = static_cast<accessories::controllers::direct_input_device_impl*>(context);

	DIPROPRANGE propRange;
	propRange.diph.dwSize = sizeof(DIPROPRANGE);
	propRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	propRange.diph.dwHow = DIPH_BYID;
	propRange.diph.dwObj = instance->dwType;
	propRange.lMin = -MAX_AXIS_VAL;
	propRange.lMax = +MAX_AXIS_VAL;

	// Set the range for the axis
	if (FAILED(obj->m_direct_input_device->SetProperty(DIPROP_RANGE, &propRange.diph)))
		return DIENUM_STOP;	

	return DIENUM_CONTINUE;
}

BOOL accessories::controllers::direct_input_device_impl::effects_enumerator_callback(LPCDIEFFECTINFO effectInfo, VOID * context)
{
	if (std::memcmp(&effectInfo->guid, &GUID_ConstantForce, sizeof(GUID_ConstantForce)) != 0)
		return TRUE;	

	DIEFFECT dieffect = {};
	DWORD rgwAxes[1] = { DIJOFS_X };
	LONG rglDirection[2] = { 1000, 0 };
	DICONSTANTFORCE constantForce;

	constantForce.lMagnitude = 5; // DI_FFNOMINALMAX;
	
	dieffect.dwSize = sizeof(DIEFFECT);	
	dieffect.dwFlags = DIEFF_POLAR | DIEFF_OBJECTOFFSETS;

	if (effectInfo->dwStaticParams & DIEP_DURATION)
	{
		dieffect.dwDuration = INFINITE;
	}

	if (effectInfo->dwStaticParams & DIEP_SAMPLEPERIOD)
	{
		dieffect.dwSamplePeriod = 0;
	}

	if (effectInfo->dwStaticParams & DIEP_GAIN)
	{
		dieffect.dwGain = DI_FFNOMINALMAX;
	}

	if (effectInfo->dwStaticParams & DIEP_TRIGGERBUTTON)
	{
		dieffect.dwTriggerButton = DIEB_NOTRIGGER;
	}

	if (effectInfo->dwStaticParams & DIEP_TRIGGERREPEATINTERVAL)
	{
		dieffect.dwTriggerRepeatInterval = 0;
	}

	if (effectInfo->dwStaticParams & DIEP_AXES)
	{
		dieffect.cAxes = 2;
		dieffect.rgdwAxes = &rgwAxes[0];
	}

	if (effectInfo->dwStaticParams & DIEP_DIRECTION)
	{
		dieffect.rglDirection = &rglDirection[0];
	}

	dieffect.lpEnvelope = NULL;

	if (effectInfo->dwStaticParams & DIEP_TYPESPECIFICPARAMS)
	{
		dieffect.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
		dieffect.lpvTypeSpecificParams = &constantForce;
	}

	accessories::controllers::direct_input_device_impl* obj = static_cast<accessories::controllers::direct_input_device_impl*>(context);
	CComPtr<IDirectInputEffect> effect;
	HRESULT hr = obj->m_direct_input_device->CreateEffect(GUID_ConstantForce, &dieffect, &effect, NULL);
	auto val = GetLastError();
	(void)val;
	//HRESULT hr = obj->m_direct_input_device->CreateEffect(GUID_ConstantForce, NULL, &effect, NULL);
	if (FAILED(hr))
		return FALSE;	

	obj->m_direct_input_effect = effect;
	return FALSE;
}

bool accessories::controllers::direct_input_device_impl::initialize_direct_input()
{
	assert(m_direct_input == nullptr);

	// Create a DirectInput device
	if (FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_direct_input, NULL)))
		return false;

	return true;
}

bool accessories::controllers::direct_input_device_impl::set_device_properties()
{
	if (FAILED(m_direct_input_device->SetDataFormat(&c_dfDIJoystick2)))
		return false;

	if (FAILED(m_direct_input_device->SetCooperativeLevel(NULL, DISCL_NONEXCLUSIVE /*DISCL_EXCLUSIVE*/ | DISCL_BACKGROUND)))
	{
		printf("Failed to set Cooperative Level\n");
		// return false;
	}

	/*m_device_capabilities.dwSize = sizeof(DIDEVCAPS);
	if (FAILED(m_joystick->GetCapabilities(&m_device_capabilities)))
		return false;*/

	if (FAILED(m_direct_input_device->EnumObjects(axes_enumerator_callback, this, DIDFT_AXIS)))
		return false;

	return true;
}

bool accessories::controllers::direct_input_device_impl::select_device()
{
	assert(m_direct_input_device == nullptr);
	m_device_index_iterator = m_device_index;

	// Look for the first simple joystick we can find.
	if (FAILED(m_direct_input->EnumDevices(DI8DEVCLASS_GAMECTRL, device_enumerator_callback, this, DIEDFL_ATTACHEDONLY)))
		return false;

	// Make sure we got a device
	if (m_direct_input_device == NULL)
		return false;

	if (set_device_properties() == false)
	{
		m_direct_input_device = nullptr;
		return false;
	}

	if (FAILED(m_direct_input_device->EnumEffects(&effects_enumerator_callback, this, DIEFT_ALL)))
		return false;

	return true;
}

bool accessories::controllers::direct_input_device_impl::take_device()
{
	if (m_direct_input == nullptr)
		if (initialize_direct_input() == false)
			return false;

	if (m_direct_input_device == nullptr)
		if (select_device() == false)
			return false;

	HRESULT hr = m_direct_input_device->Poll();
	if (FAILED(hr))
	{
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = m_direct_input_device->Acquire();
		while (hr == DIERR_INPUTLOST)
		{
			hr = m_direct_input_device->Acquire();
		}

		// If we encounter a fatal error, return failure.
		if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED))
			return false;

		// If another application has control of this device, return.
		// We'll just have to wait our turn to use the joystick.
		if (hr == DIERR_OTHERAPPHASPRIO)
			return false;
	}

	return true;
}

void accessories::controllers::direct_input_device_impl::convert(const DIJOYSTATE2& dinfo, Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo & info)
{
	info.xAxisPosition = static_cast<int16_t>(dinfo.lX);
	info.yAxisPosition = static_cast<int16_t>(dinfo.lY);
	info.zAxisPosition = static_cast<int16_t>(dinfo.lZ);

	info.xAxisRotation = static_cast<int16_t>(dinfo.lRx);
	info.yAxisRotation = static_cast<int16_t>(dinfo.lRy);
	info.zAxisRotation = static_cast<int16_t>(dinfo.lRz);

	info.extraAxesPositions[0] = static_cast<int16_t>(dinfo.rglASlider[0]);
	info.extraAxesPositions[1] = static_cast<int16_t>(dinfo.rglASlider[1]);

	for (size_t i = 0; i < 4; i++)
		if (dinfo.rgdwPOV[i] != DIRECT_INPUT_UNDEFINED_POD)
			info.pointOfViewDirections[i] = static_cast<uint16_t>(dinfo.rgdwPOV[i]);

	for (size_t i = 0; i < 128; i++)
	{
		size_t index = (i / 8);
		size_t bit = (i % 8);
		if (dinfo.rgbButtons[i] > 0)
			info.buttons[index] = info.buttons[index] | static_cast<uint8_t>(pow(2, bit));
	}		

	info.xAxisVelocity = static_cast<int16_t>(dinfo.lVX);
	info.yAxisVelocity = static_cast<int16_t>(dinfo.lVY);
	info.zAxisVelocity = static_cast<int16_t>(dinfo.lVZ);

	info.xAxisAngularVelocity = static_cast<int16_t>(dinfo.lVRx);
	info.yAxisAngularVelocity = static_cast<int16_t>(dinfo.lVRy);
	info.zAxisAngularVelocity = static_cast<int16_t>(dinfo.lVRz);

	info.extraAxesVelocities[0] = static_cast<int16_t>(dinfo.rglVSlider[0]);
	info.extraAxesVelocities[1] = static_cast<int16_t>(dinfo.rglVSlider[1]);

	info.xAxisAcceleration = static_cast<int16_t>(dinfo.lAX);
	info.yAxisAcceleration = static_cast<int16_t>(dinfo.lAY);
	info.zAxisAcceleration = static_cast<int16_t>(dinfo.lAZ);

	info.xAxisAngularAcceleration = static_cast<int16_t>(dinfo.lARx);
	info.yAxisAngularAcceleration = static_cast<int16_t>(dinfo.lARy);
	info.zAxisAngularAcceleration = static_cast<int16_t>(dinfo.lARz);

	info.extraAxesAcceleration[0] = static_cast<int16_t>(dinfo.rglASlider[0]);
	info.extraAxesAcceleration[1] = static_cast<int16_t>(dinfo.rglASlider[1]);

	info.xAxisForce = static_cast<int16_t>(dinfo.lFX);
	info.yAxisForce = static_cast<int16_t>(dinfo.lFY);
	info.zAxisForce = static_cast<int16_t>(dinfo.lFZ);

	info.xAxisTorque = static_cast<int16_t>(dinfo.lFRx);
	info.yAxisTorque = static_cast<int16_t>(dinfo.lFRy);
	info.zAxisTorque = static_cast<int16_t>(dinfo.lFRz);

	info.extraAxesForces[0] = static_cast<int16_t>(dinfo.rglFSlider[0]);
	info.extraAxesForces[1] = static_cast<int16_t>(dinfo.rglFSlider[1]);
}

void accessories::controllers::direct_input_device_impl::vibrate(uint16_t left_motor, uint16_t right_motor)
{	
	if (take_device() == false)
		return;	

	if (FAILED(m_direct_input_device->EnumEffects(&effects_enumerator_callback, this, /*DIEFT_ALL*/DIEFT_CONSTANTFORCE)))
		return;

	if (m_direct_input_effect == nullptr)
		return;

	m_direct_input_effect->Start(INFINITE, 0);
}

void accessories::controllers::direct_input_device_impl::on_start()
{
	Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceCommandsDBEnum vibration_row_key = 
		Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceCommandsDBEnum::COMPLEX_INPUT_DEVICE_VIBRATION;
	m_vibration_token = subscribe(
		m_commands_table, 
		utils::database::buffered_key(&vibration_row_key, sizeof(vibration_row_key)), 
		[this](const utils::database::row_data& data)
	{
		Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceVibration vibration = 
			data.read<Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceVibration>();

		if (m_left_motor_vibration != vibration.leftMotor ||
			m_right_motor_vibration != vibration.rightMotor)
		{
			m_left_motor_vibration = vibration.leftMotor;
			m_right_motor_vibration = vibration.rightMotor;
			vibrate(m_left_motor_vibration, m_right_motor_vibration);
		}
	});

	
	initialize_direct_input();
	select_device();
	m_lastFailureTime = std::chrono::high_resolution_clock::now();
}

void accessories::controllers::direct_input_device_impl::on_stop()
{
	m_direct_input_device = nullptr;
	m_direct_input = nullptr;

	if (m_vibration_token == utils::database::subscription_token_undefined)
		return;	

	Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceCommandsDBEnum vibration_row_key = 
		Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceCommandsDBEnum::COMPLEX_INPUT_DEVICE_VIBRATION;
	unsubscribe(
		m_commands_table, 
		utils::database::buffered_key(&vibration_row_key, sizeof(vibration_row_key)), 
		m_vibration_token);
}

bool accessories::controllers::direct_input_device_impl::get_info(Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo& info)
{
	auto now = std::chrono::high_resolution_clock::now();
	if (m_lastFailureTime != (std::chrono::time_point<std::chrono::high_resolution_clock>::min)() &&
		std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFailureTime).count() < TAKE_DEVICE_TIMOUT_ON_FAILURE)
		return false;

	if (take_device() == false)
	{
		m_lastFailureTime = now;
		return false;
	}

	m_lastFailureTime = (std::chrono::time_point<std::chrono::high_resolution_clock>::min)();

	// Get the input's device state
	DIJOYSTATE2 dinfo;
	if (FAILED(m_direct_input_device->GetDeviceState(sizeof(DIJOYSTATE2), &dinfo)))
		return false;

	convert(dinfo, info);
	return true;
}

accessories::controllers::direct_input_device_impl::direct_input_device_impl(
	core::database::table_interface* status_table,
	core::database::table_interface* commands_table,
	uint16_t deadzone,
	const char* device_name,
	uint32_t device_index,
	double polling_interval_milliseconds) :
	accessories::controllers::complex_input_device_base<accessories::controllers::direct_input_device>(status_table, commands_table, deadzone, polling_interval_milliseconds),
	m_device_name(device_name != nullptr ? device_name : ""),
	m_device_index(device_index),
	m_left_motor_vibration(0),
	m_right_motor_vibration(0),
	m_vibration_token(utils::database::subscription_token_undefined),
	m_lastFailureTime((std::chrono::time_point<std::chrono::high_resolution_clock>::min)())
{
}

bool accessories::controllers::direct_input_device::create(
	core::database::table_interface* status_table,
	core::database::table_interface* commands_table,
	uint16_t deadzone,
	const char* device_name,
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
		instance = utils::make_ref_count_ptr<accessories::controllers::direct_input_device_impl>(status_table, commands_table, deadzone, device_name, device_index, polling_interval_milliseconds);
	}
	catch (.../*std::exception& e*/)
	{
		return false;
	}

	instance->add_ref();
	*joystick = instance;
	return true;
}

bool accessories::controllers::direct_input_device::create(
	core::database::table_interface* status_table,
	core::database::table_interface* commands_table,
	uint16_t deadzone,
	uint32_t device_index,
	double polling_interval_milliseconds,
	core::accessories::controllers::joystick_runnable** joystick)
{
	return accessories::controllers::direct_input_device::create(
		status_table,
		commands_table,
		deadzone,
		nullptr,
		device_index,
		polling_interval_milliseconds,
		joystick);
}
