#include "can_port_impl.h"



bool communication::ports::can_port_adapter::create(
	communication::ports::can_interface ican,
	communication::ports::can_mode mode,
	const char* channel_name,
	uint32_t buadrate,
	bool extension_mode,
	core::communication::client_channel_interface** client)
{
	if (client == nullptr)
		return false;

	utils::ref_count_ptr<core::communication::client_channel_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<can_port_adapter_impl>(ican, mode, channel_name, buadrate, extension_mode);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*client = instance;
	(*client)->add_ref();
	return true;
}
