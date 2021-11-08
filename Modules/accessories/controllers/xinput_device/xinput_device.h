#pragma once
#include <joystick_base.hpp>
#include <accessories/controllers/xinput_device.h>

#include <Xinput.h>

namespace accessories
{
	namespace controllers
	{
		using base_class = accessories::controllers::joystick_base<accessories::controllers::xinput_device>;

		class xinput_device_impl : public base_class
		{
		private:
			uint32_t m_device_index;
			bool m_set_vibration;
			uint16_t m_left_motor_vibration;
			uint16_t m_right_motor_vibration;
			utils::database::subscription_token m_vibration_token;

			void convert(const XINPUT_STATE& xinfo, Accessories::Controllers::JoystickDefs::JoystickInfo& info);
			void vibrate(uint16_t left_motor, uint16_t right_motor);

		protected:		
			virtual void on_start() override;
			virtual void on_stop() override;

			virtual bool get_info(Accessories::Controllers::JoystickDefs::JoystickInfo& info) override;

		public:
			xinput_device_impl(
				core::database::table_interface* status_table,
				core::database::table_interface* commands_table,
				uint16_t deadzone,
				uint32_t device_index,
				double polling_interval_milliseconds);
		};
	}
}

