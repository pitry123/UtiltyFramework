#pragma once
#include <core/accessories.h>
#include <Application.hpp>

namespace Accessories
{
	namespace Controllers
	{
		class Joystick :
			public Application::Runnable
		{
		public:
			Joystick()
			{
				// Empty Joystick
			}

			Joystick(core::accessories::controllers::joystick_runnable* joystick) :
				Application::Runnable(joystick)
			{
			}			
		};
	}
}