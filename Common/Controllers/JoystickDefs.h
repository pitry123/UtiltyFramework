#pragma once
#include <Controllers/ControllerDefs.h>
#include <cstdint>

namespace Accessories
{
	namespace Controllers
	{
#pragma pack(1)
		class JoystickDefs
		{
		public:
			enum JoystickDeviceConnectionStatus
			{
				JOYSTICK_DEVICE_CONNECTED,
				JOYSTICK_DEVICE_DISCONNECTED
			};

			struct JoystickInfo
			{
				int16_t leftAnalogX;
				int16_t leftAnalogY;
				int16_t rightAnalogX;
				int16_t rightAnalogY;
				int16_t zAxis;

				uint16_t pointOfView;
				
				uint8_t leftTrigger;
				uint8_t rightTrigger;
				
				uint16_t buttons;
				uint8_t pushedButtonsCount;

				JoystickDeviceConnectionStatus connectionStatus; // JoystickDefs::JoystickDeviceConnectionStatus t:int
			};

			struct JoystickVibration
			{
				uint16_t leftMotor;
				uint16_t rightMotor;
			};
		};
#pragma pack()
	}
}