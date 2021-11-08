#include "shared_memory_video_publisher.h"

bool video::publishers::shared_memory_video_publisher::create(
	const char* video_name,
	core::video::video_source_interface* source,
	uint32_t buffer_size,
	uint32_t buffer_pool_size,
	core::video::video_publisher_interface** publisher)
{
	if (source == nullptr)
		return false;

	if (publisher == nullptr)
		return false;

	if (buffer_size == 0)
		return false;

	if (buffer_pool_size == 0)
		return false;	

	utils::ref_count_ptr<core::video::video_publisher_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<shared_memory_video_publisher_impl>(video_name, source, buffer_size, buffer_pool_size);
	}
	catch (...)
	{
		return false;
	}

	if (instance == nullptr)
		return false;

	*publisher = instance;
	(*publisher)->add_ref();
	return true;
}

bool video::publishers::shared_memory_video_publisher::create(
	const char* video_name,
	core::video::video_source_interface* source,
	core::video::video_publisher_interface** publisher)
{
	return create(video_name, source, VIDEO_CHANNEL_BUFFER_SIZE, VIDEO_CHANNEL_POOL_SIZE, publisher);
}

bool video::publishers::shared_memory_video_publisher::create(
	const char* video_name, 
	const core::video::video_source_factory_interface* source_factory,
	uint32_t buffer_size,
	uint32_t buffer_pool_size,
	core::video::video_publisher_interface** publisher)
{
	if (source_factory == nullptr)
		return false;

	if (publisher == nullptr)
		return false;

	if (buffer_size == 0)
		return false;

	if (buffer_pool_size == 0)
		return false;
	
	utils::ref_count_ptr<core::video::video_source_interface> source;
	if (source_factory->create(&source) == false)
		return false;

	return video::publishers::shared_memory_video_publisher::create(video_name, source, buffer_size, buffer_pool_size, publisher);
}

bool video::publishers::shared_memory_video_publisher::create(
	const char* video_name,
	const core::video::video_source_factory_interface* source_factory,
	core::video::video_publisher_interface** publisher)
{
	return create(video_name, source_factory, VIDEO_CHANNEL_BUFFER_SIZE, VIDEO_CHANNEL_POOL_SIZE, publisher);
}
