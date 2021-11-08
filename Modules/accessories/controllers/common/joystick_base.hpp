#pragma once
#include <controller_base.hpp>

#include <Controllers/JoystickDefs.h>
#include <Controllers/JoystickDB.h>

#define DEAD_ZONE(val) \
((std::abs(val) <= m_deadzone) ? 0 : val)

namespace accessories
{
	namespace controllers
	{	
		template <class T>
		class joystick_base : public controller_base<T>
		{
		private:
			uint16_t m_deadzone;

			void report_info(const Accessories::Controllers::JoystickDefs::JoystickInfo& info)
			{
				Accessories::Controllers::JoystickDB::JoystickStatusDBEnum info_row_key = Accessories::Controllers::JoystickDB::JoystickStatusDBEnum::JOYSTICK_INFO;
				utils::ref_count_ptr<core::database::row_interface> info_row;
				if (m_status_table->query_row(utils::database::buffered_key(&info_row_key, sizeof(info_row_key)), &info_row) == false)
					return; // TODO: Log Error

				info_row->write_bytes(&info, sizeof(info), false, 0);
			}

		protected:
			void reset(Accessories::Controllers::JoystickDefs::JoystickInfo& info)
			{
				info = {};
				info.pointOfView = POV_UNDEFINED;
			}

			virtual bool get_info(Accessories::Controllers::JoystickDefs::JoystickInfo& info) = 0;

			virtual void on_start() override
			{
				controller_base<T>::on_start();				
			}

			virtual void on_stop() override
			{
				controller_base<T>::on_stop();

				Accessories::Controllers::JoystickDefs::JoystickInfo info;
				reset(info);
				report_info(info);
			}

			virtual void on_timer() override
			{
				Accessories::Controllers::JoystickDefs::JoystickInfo info;
				reset(info);

				bool success = false;
				utils::scope_guard reporter([&]()
				{
					if (success == false)
						reset(info);

					report_info(info);
				});

				success = get_info(info);

				info.leftAnalogX = DEAD_ZONE(info.leftAnalogX);
				info.leftAnalogY = DEAD_ZONE(info.leftAnalogY);
				info.rightAnalogX = DEAD_ZONE(info.rightAnalogX);
				info.rightAnalogY = DEAD_ZONE(info.rightAnalogY);
			}
			
		public:
			joystick_base(
				core::database::table_interface* status_table,
				core::database::table_interface* commands_table,
				uint16_t deadzone,
				double polling_interval_milliseconds,
				const char* dispatcher_name = "Joystick Runnable") :
				controller_base<T>(status_table, commands_table, polling_interval_milliseconds, dispatcher_name),
				m_deadzone(deadzone)
			{
				Accessories::Controllers::JoystickDB::JoystickStatusDBEnum info_row_key = Accessories::Controllers::JoystickDB::JoystickStatusDBEnum::JOYSTICK_INFO;
				status_table->add_row(utils::database::buffered_key(&info_row_key, sizeof(info_row_key)), sizeof(Accessories::Controllers::JoystickDefs::JoystickInfo));

				Accessories::Controllers::JoystickDB::JoystickCommandsDBEnum vibration_row_key = Accessories::Controllers::JoystickDB::JoystickCommandsDBEnum::JOYSTICK_VIBRATION;
				commands_table->add_row(utils::database::buffered_key(&vibration_row_key, sizeof(vibration_row_key)), sizeof(Accessories::Controllers::JoystickDefs::JoystickVibration));

			}

			virtual ~joystick_base()
			{
				Accessories::Controllers::JoystickDB::JoystickStatusDBEnum info_row_key = Accessories::Controllers::JoystickDB::JoystickStatusDBEnum::JOYSTICK_INFO;
				m_status_table->remove_row(utils::database::buffered_key(&info_row_key, sizeof(info_row_key)), nullptr);

				Accessories::Controllers::JoystickDB::JoystickCommandsDBEnum vibration_row_key = Accessories::Controllers::JoystickDB::JoystickCommandsDBEnum::JOYSTICK_VIBRATION;
				m_commands_table->remove_row(utils::database::buffered_key(&vibration_row_key, sizeof(vibration_row_key)), nullptr);
			}
		};
	}
}

