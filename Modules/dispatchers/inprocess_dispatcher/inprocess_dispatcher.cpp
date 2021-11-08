// inprocess_dispatcher.cpp : Defines the exported functions for the DLL application.
//

#include "inprocess_dispatcher.h"

bool dispatchers::internal::inprocess_dispatcher_impl::post(core::messaging::message_interface* message)
{
	post_message(message);
	return true;
}

bool dispatchers::internal::inprocess_dispatcher::create(core::messaging::message_dispatcher** dispatcher)
{
	if (dispatcher == nullptr)
		return false;

	utils::ref_count_ptr<core::messaging::message_dispatcher> instance;
	try
	{
		instance = utils::make_ref_count_ptr<inprocess_dispatcher_impl>();
	}
	catch (...)
	{
		return false;
	}

    if (instance == nullptr)
        return false;

	*dispatcher = instance;
	(*dispatcher)->add_ref();
	return true;
}
