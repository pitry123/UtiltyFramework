#pragma once
#include <joystick_base_windows.hpp>
#include <accessories/controllers/logitech_f310.h>

namespace accessories
{
	namespace controllers
	{
		enum joystick_mode
		{
			UNDEFINED,
			D_MODE,
			X_MODE,
		};

		using base_class = accessories::controllers::joystick_base_winmm<accessories::controllers::logitech_f310>;

		class logitech_f310_impl : public base_class
		{
		private:			
			joystick_mode m_mode;

		protected:
			virtual bool accept_device(const JOYCAPS& caps) override;
			virtual void convert(const JOYINFOEX& win_info, Accessories::Controllers::JoystickDefs::JoystickInfo& info) override;

		public:
			logitech_f310_impl(
				core::database::table_interface* status_table,
				core::database::table_interface* commands_table,
				uint16_t deadzone,
				uint32_t device_index,
				double polling_interval_milliseconds);
		};
	}
}

