#pragma once
#include <controller_base.hpp>

#include <Controllers/ComplexInputDeviceDefs.h>
#include <Controllers/ComplexInputDeviceDB.h>

#define DEAD_ZONE(val) \
((std::abs(val) <= m_deadzone) ? 0 : val)

namespace accessories
{
	namespace controllers
	{	
		template <class T>
		class complex_input_device_base : public controller_base<T>
		{
		private:
			uint16_t m_deadzone;

			void report_info(const Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo& info)
			{
				Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceStatusDBEnum info_row_key = 
					Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceStatusDBEnum::COMPLEX_INPUT_DEVICE_INFO;
				utils::ref_count_ptr<core::database::row_interface> info_row;
				if (m_status_table->query_row(utils::database::buffered_key(&info_row_key, sizeof(info_row_key)), &info_row) == false)
					return; // TODO: Log Error

				info_row->write_bytes(&info, sizeof(info), false, 0);
			}

		protected:
			void reset(Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo& info)
			{
				info = {};

				for (size_t i = 0; i < (sizeof(info.pointOfViewDirections) / sizeof(uint16_t)); i++)
					info.pointOfViewDirections[i] = POV_UNDEFINED;

				info.connectionStatus = Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceConnectionStatus::COMPLEX_INPUT_DEVICE_DISCONNECTED;
			}

			virtual bool get_info(Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo& info) = 0;

			virtual void on_start() override
			{
				controller_base<T>::on_start();				
			}

			virtual void on_stop() override
			{
				controller_base<T>::on_stop();

				Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo info;
				reset(info);
				report_info(info);
			}

			virtual void on_timer() override
			{
				Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo info;
				reset(info);

				bool success = false;
				utils::scope_guard reporter([&]()
				{
					if (success == true)
						info.connectionStatus = Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceConnectionStatus::COMPLEX_INPUT_DEVICE_CONNECTED;						
					else
						reset(info);						

					report_info(info);
				});

				success = get_info(info);

				info.xAxisPosition = DEAD_ZONE(info.xAxisPosition);
				info.yAxisPosition = DEAD_ZONE(info.yAxisPosition);
				info.zAxisPosition = DEAD_ZONE(info.zAxisPosition);

				info.xAxisRotation = DEAD_ZONE(info.xAxisRotation);
				info.yAxisRotation = DEAD_ZONE(info.yAxisRotation);
				info.zAxisRotation = DEAD_ZONE(info.zAxisRotation);

				for (size_t i = 0; i < (sizeof(info.extraAxesPositions) / sizeof(int16_t)); i++)
					info.extraAxesPositions[i] = DEAD_ZONE(info.extraAxesPositions[i]);		
			}
			
		public:
			complex_input_device_base(
				core::database::table_interface* status_table,
				core::database::table_interface* commands_table,
				uint16_t deadzone,
				double polling_interval_milliseconds,
				const char* dispatcher_name = "Complex Input Device Runnable") :
				controller_base<T>(status_table, commands_table, polling_interval_milliseconds, dispatcher_name),
				m_deadzone(deadzone)
			{
				Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceStatusDBEnum info_row_key = 
					Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceStatusDBEnum::COMPLEX_INPUT_DEVICE_INFO;
				status_table->add_row(utils::database::buffered_key(&info_row_key, sizeof(info_row_key)), sizeof(Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo));

				Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceCommandsDBEnum vibration_row_key =
					Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceCommandsDBEnum::COMPLEX_INPUT_DEVICE_VIBRATION;
				commands_table->add_row(utils::database::buffered_key(&vibration_row_key, sizeof(vibration_row_key)), sizeof(Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceVibration));

			}

			virtual ~complex_input_device_base()
			{
				Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceStatusDBEnum info_row_key = 
					Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceStatusDBEnum::COMPLEX_INPUT_DEVICE_INFO;
				m_status_table->remove_row(utils::database::buffered_key(&info_row_key, sizeof(info_row_key)), nullptr);

				Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceCommandsDBEnum vibration_row_key = 
					Accessories::Controllers::ComplexInputDeviceDB::ComplexInputDeviceCommandsDBEnum::COMPLEX_INPUT_DEVICE_VIBRATION;
				m_commands_table->remove_row(utils::database::buffered_key(&vibration_row_key, sizeof(vibration_row_key)), nullptr);
			}
		};
	}
}

