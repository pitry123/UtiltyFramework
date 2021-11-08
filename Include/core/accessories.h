#pragma once
#include <core/application.h>

namespace core
{
	namespace accessories
	{
		namespace controllers
		{
			class DLL_EXPORT joystick_runnable : public virtual core::application::runnable_interface
			{
			public:
				virtual ~joystick_runnable() = default;
			};
		}
	}
}