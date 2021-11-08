#pragma once
#include <Controllers/ControllerDefs.h>
#include <cstdint>

namespace Accessories
{
	namespace Controllers
	{
#pragma pack(1)
		class ComplexInputDeviceDefs
		{
		public:
			enum ComplexInputDeviceConnectionStatus
			{
				COMPLEX_INPUT_DEVICE_CONNECTED,
				COMPLEX_INPUT_DEVICE_DISCONNECTED
			};

			struct ComplexInputDeviceInfo
			{
				int16_t    xAxisPosition;
				int16_t    yAxisPosition;
				int16_t    zAxisPosition;
				int16_t    xAxisRotation;
				int16_t    yAxisRotation;
				int16_t    zAxisRotation;
				int16_t    extraAxesPositions[2];
				uint16_t   pointOfViewDirections[4];
				uint8_t    buttons[16];
				int16_t    xAxisVelocity;
				int16_t    yAxisVelocity;
				int16_t    zAxisVelocity;
				int16_t    xAxisAngularVelocity;
				int16_t    yAxisAngularVelocity;
				int16_t    zAxisAngularVelocity;
				int16_t    extraAxesVelocities[2];
				int16_t    xAxisAcceleration;
				int16_t    yAxisAcceleration;
				int16_t    zAxisAcceleration;
				int16_t    xAxisAngularAcceleration;
				int16_t    yAxisAngularAcceleration;
				int16_t    zAxisAngularAcceleration;
				int16_t    extraAxesAcceleration[2];
				int16_t    xAxisForce;
				int16_t    yAxisForce;
				int16_t    zAxisForce;
				int16_t    xAxisTorque;
				int16_t    yAxisTorque;
				int16_t    zAxisTorque;
				int16_t    extraAxesForces[2];

				ComplexInputDeviceConnectionStatus connectionStatus; // ComplexInputDeviceDefs::ComplexInputDeviceConnectionStatus t:int
			};

			struct ComplexInputDeviceVibration
			{
				uint16_t leftMotor;
				uint16_t rightMotor;
			};
		};
#pragma pack()
	}
}