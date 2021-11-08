#pragma once
namespace Accessories
{
	namespace Controllers
	{
		class JoystickDB
		{
		public:
			enum JoystickStatusDBEnum
			{
				JOYSTICK_INFO, //t:JoystickDefs::JoystickInfo
				NUM_OF_JOYSTICK_STATUS_ROWS
			};

			enum JoystickCommandsDBEnum
			{
				JOYSTICK_VIBRATION, //t:JoystickDefs::JoystickVibration
				NUM_OF_JOYSTICK_COMMANDS_ROWS
			};
		};
	}
}



