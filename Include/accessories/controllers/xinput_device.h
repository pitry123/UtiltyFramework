#pragma once
#include <core/accessories.h>
#include <core/database.h>

namespace accessories
{
	namespace controllers
	{
		class DLL_EXPORT xinput_device : public core::accessories::controllers::joystick_runnable
		{
		public:
			virtual ~xinput_device() = default;
			
			static bool create(				
				core::database::table_interface* status_table,
				core::database::table_interface* commands_table,				
				uint16_t deadzone,
				uint32_t device_index,
				double polling_interval_milliseconds,
				core::accessories::controllers::joystick_runnable** joystick);
		};
	}
}