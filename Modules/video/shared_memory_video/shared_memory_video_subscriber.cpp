#include "shared_memory_video_subscriber.h"

bool video::sources::shared_memory_video_source::create(const char* video_name, core::video::video_source_interface** source)
{
	if (source == nullptr)
		return false;

	utils::ref_count_ptr<core::video::video_source_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<shared_memory_video_subscriber>(video_name);
	}
	catch (...)
	{
		return false;
	}

    if (instance == nullptr)
        return false;

	*source = instance;
	(*source)->add_ref();
	return true;
}
