#include "frame_message.h"

bool video::messages::frame_message::create(
	core::buffer_allocator* buffer_allocator,
	core::messaging::message_interface** msg)
{
	if (msg == nullptr)
		return false;

	utils::ref_count_ptr<core::messaging::message_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<video::messages::frame_message_impl>(buffer_allocator);
	}
	catch (...)
	{
		return false;
	}

    if (instance == nullptr)
        return false;

	*msg = instance;
	(*msg)->add_ref();
	return true;
}

bool video::messages::frame_message::create(
	core::video::frame_interface* frame,	
	core::buffer_allocator* buffer_allocator,
	core::messaging::message_interface** msg)
{
	if (msg == nullptr)
		return false;

	utils::ref_count_ptr<core::messaging::message_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<video::messages::frame_message_impl>(frame, buffer_allocator);
	}
	catch (...)
	{
		return false;
	}

    if (instance == nullptr)
        return false;

	*msg = instance;
	(*msg)->add_ref();
	return true;
}
