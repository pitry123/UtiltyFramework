#pragma once
#include <core/communication.h>

namespace websocket
{
	class DLL_EXPORT websocket_client : public core::communication::client_channel_interface
	{
		enum op_code_ws
		{
			ON_OPEN,
			ON_CLOSE,
			ON_MESSEGE,
			ON_ERROR,
			ON_PING,
			ON_PONG
		};

	public:
		virtual ~websocket_client() = default;

		static bool create(const char* uri, core::communication::client_channel_interface** client);
		static bool create(const char* uri, uint16_t port, core::communication::client_channel_interface** client);
	};
}