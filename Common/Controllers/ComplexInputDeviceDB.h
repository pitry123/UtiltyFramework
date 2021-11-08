#pragma once
namespace Accessories
{
	namespace Controllers
	{
		class ComplexInputDeviceDB
		{
		public:
			enum ComplexInputDeviceStatusDBEnum
			{
				COMPLEX_INPUT_DEVICE_INFO, //t:ComplexInputDeviceDefs::ComplexInputDeviceInfo
				NUM_OF_COMPLEX_INPUT_DEVICE_STATUS_ROWS
			};

			enum ComplexInputDeviceCommandsDBEnum
			{
				COMPLEX_INPUT_DEVICE_VIBRATION, //t:ComplexInputDeviceDefs::ComplexInputDeviceVibration
				NUM_OF_COMPLEX_INPUT_DEVICE_COMMANDS_ROWS
			};
		};
	}
}



