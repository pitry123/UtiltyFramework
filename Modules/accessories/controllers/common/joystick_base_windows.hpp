#pragma once
#include <joystick_base.hpp>
#include <utils/scope_guard.hpp>
#include <limits>

#include <windows.h>
#include <XInput.h>
#include <joystickapi.h>

#define SELECTED_DEVICE_UNDEFINED (std::numeric_limits<UINT>::max)()

namespace accessories
{
	namespace controllers
	{
		template <class T>
		class joystick_base_winmm : public accessories::controllers::joystick_base<T>
		{
		private:
			uint32_t m_device_index;
			UINT m_selected_device;

		protected:
			virtual bool accept_device(const JOYCAPS& caps) = 0;
			virtual void convert(const JOYINFOEX& win_info, Accessories::Controllers::JoystickDefs::JoystickInfo& info) = 0;
			
			virtual void on_start() override
			{
				m_selected_device = SELECTED_DEVICE_UNDEFINED;
			}

			virtual bool get_info(Accessories::Controllers::JoystickDefs::JoystickInfo& info)
			{
				UINT dev_count = joyGetNumDevs();
				if (dev_count == 0)
					return false;

				JOYCAPS caps;
				if ((m_selected_device == SELECTED_DEVICE_UNDEFINED) ||
					(joyGetDevCaps(m_selected_device, &caps, sizeof(JOYCAPS)) != JOYERR_NOERROR) ||
					(accept_device(caps) == false))
				{
					m_selected_device = SELECTED_DEVICE_UNDEFINED;
					uint32_t dev_index = m_device_index;
					for (UINT i = 0; i < dev_count; i++)
					{
						if (joyGetDevCaps(i, &caps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
							continue;

						if (accept_device(caps) == false)
							continue;

						if (dev_index != 0)
						{
							--dev_index;
							continue;
						}

						m_selected_device = i;
						break;
					}
				}

				if (m_selected_device == SELECTED_DEVICE_UNDEFINED)
					return false;				

				JOYINFOEX joyinfo;
				if (joyGetPosEx(m_selected_device, &joyinfo) == JOYERR_UNPLUGGED)
				{
					m_selected_device = SELECTED_DEVICE_UNDEFINED;
					return false;
				}
				
				convert(joyinfo, info);
				return true;
			}		
			
		public:
			joystick_base_winmm(
				core::database::table_interface* status_table,
				core::database::table_interface* commands_table,
				uint16_t deadzone,
				uint32_t device_index,
				double polling_interval_milliseconds) :
				accessories::controllers::joystick_base<T>(status_table, commands_table, deadzone, polling_interval_milliseconds),
				m_device_index(device_index),
				m_selected_device(SELECTED_DEVICE_UNDEFINED)
			{				
			}						
		};
	}
}

