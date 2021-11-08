#pragma once
#include <complex_input_device_base.hpp>
#include <accessories/controllers/direct_input_device.h>

#define DIRECTINPUT_VERSION 0x0800
#include <atlbase.h>
#include <dinput.h>

#include <chrono>
#include <string>

namespace accessories
{
	namespace controllers
	{
		using base_class = accessories::controllers::complex_input_device_base<accessories::controllers::direct_input_device>;

		class direct_input_device_impl : public base_class
		{
		private:
			static BOOL CALLBACK device_enumerator_callback(const DIDEVICEINSTANCE* instance, VOID* context);
			static BOOL CALLBACK axes_enumerator_callback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context);
			static BOOL CALLBACK effects_enumerator_callback(LPCDIEFFECTINFO effectInfo, VOID* context);

			std::string m_device_name;
			uint32_t m_device_index;
			uint32_t m_device_index_iterator;
			uint16_t m_left_motor_vibration;
			uint16_t m_right_motor_vibration;
			utils::database::subscription_token m_vibration_token;

			CComPtr<IDirectInput8> m_direct_input;
			CComPtr<IDirectInputDevice8> m_direct_input_device;
			CComPtr<IDirectInputEffect> m_direct_input_effect;

			std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFailureTime;

			bool initialize_direct_input();
			bool set_device_properties();
			bool select_device();
			bool take_device();

			void convert(const DIJOYSTATE2& dinfo, Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo& info);
			void vibrate(uint16_t left_motor, uint16_t right_motor);

		protected:		
			virtual void on_start() override;
			virtual void on_stop() override;

			virtual bool get_info(Accessories::Controllers::ComplexInputDeviceDefs::ComplexInputDeviceInfo& info) override;

		public:
			direct_input_device_impl(
				core::database::table_interface* status_table,
				core::database::table_interface* commands_table,
				uint16_t deadzone,
				const char* device_name,
				uint32_t device_index,
				double polling_interval_milliseconds);
		};
	}
}

