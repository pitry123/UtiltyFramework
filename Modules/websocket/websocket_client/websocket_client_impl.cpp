#include "websocket_client_impl.h"

static constexpr uint16_t DEFAULT_PORT = 80;

bool websocket::websocket_client::create(const char* uri, uint16_t port, core::communication::client_channel_interface** client)
{
	if (client == nullptr)
		return false;

	if ((uri == nullptr) || (std::strcmp(uri, "") == 0))
		return false;

	utils::ref_count_ptr<core::communication::client_channel_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<websocket::websocket_client_impl>(uri, port);
	}
	catch (const std::exception&)
	{
		return false;
	}

	instance->add_ref();
	*client = instance;
	return true;
}

bool websocket::websocket_client::create(const char* uri, core::communication::client_channel_interface** client)
{
	return websocket::websocket_client::create(uri, DEFAULT_PORT, client);
}
