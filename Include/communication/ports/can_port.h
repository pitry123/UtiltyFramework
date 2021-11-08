/// @file	ports/udp_client_port.h.
/// @brief	Declares the UDP client port class
#pragma once
#include <core/communication.h>
namespace communication
{
	namespace ports
	{

#pragma pack(1)

		struct can_msg
		{
			int id;
			uint8_t data[8];
		};

#pragma pack()

		enum can_interface
		{
			socket_can,
			slcan
		};

		enum can_mode
		{
#ifndef _WIN32
			MODE_CAN_MTU = CAN_MTU,
			MODE_CANFD_MTU = CANFD_MTU

#else
			MODE_CAN_MTU = 16,
			MODE_CANFD_MTU = 16

#endif
		};

		class DLL_EXPORT can_port_adapter : public core::communication::client_channel_interface
		{
		public:

			virtual ~can_port_adapter() = default;

			static bool create(
				communication::ports::can_interface ican,
				communication::ports::can_mode mode,
				const char* channel_name, 
				uint32_t buadrate,
				bool extension_mode,
				core::communication::client_channel_interface** client);

			static bool create(
				communication::ports::can_port_interface* can_port_interface,
				core::communication::client_channel_interface** client);

		};

		class can_port_interface : public core::ref_count_interface
		{
		public:

			~can_port_interface() = default;

			virtual bool can_open() = 0;
			virtual bool can_close() = 0;
			virtual size_t can_read(void* buffer, size_t size) = 0;
			virtual size_t can_send(const void* buffer, size_t size) const = 0;
			virtual core::communication::communication_status communication_status() const = 0;
		};
	}
}