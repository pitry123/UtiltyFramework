#include "gstreamer_auto_source.h"
#include "utils/strings.hpp"

std::string video::sources::gstreamer_auto_source_impl::pipeline_description()
{
	std::stringstream description_builder;
	
	// autovideosrc
	description_builder <<
		"autovideosrc name=autovideosrc" << index() << " ! video/x-raw";

	if (m_width > 0)
		description_builder << ", width=" << m_width;

	if (m_height > 0)
		description_builder << ", height=" << m_height;

	if (m_format != core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT)
		description_builder << ", format=" << m_format;

	description_builder << " ! ";

	// appsink
	description_builder <<
		"appsink name=appsink" << index();
	
	// No sync
	description_builder <<
		" sync=" << ((m_sync == true) ? "true" : "false");	

	return description_builder.str();
}

video::sources::gstreamer_auto_source_impl::gstreamer_auto_source_impl(bool sync, uint32_t width, uint32_t height, core::imaging::pixel_format format, core::imaging::image_algorithm_interface* algo) :
	video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_auto_source>>(algo),
	m_sync(sync),
	m_width(width),
	m_height(height),
	m_format(format)
{
}

video::sources::gstreamer_auto_source_impl::~gstreamer_auto_source_impl()
{
	stop();
}

bool video::sources::gstreamer_auto_source::create(bool sync, uint32_t width, uint32_t height, core::imaging::pixel_format format, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source)
{

	if (source == nullptr)
		return false;

	utils::ref_count_ptr<core::video::video_source_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<gstreamer_auto_source_impl>(sync, width, height, format, algo);
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

bool video::sources::gstreamer_auto_source::create(bool sync, uint32_t width, uint32_t height, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_auto_source::create(sync, width, height, core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT, algo, source);
}

bool video::sources::gstreamer_auto_source::create(bool sync, uint32_t width, uint32_t height, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_auto_source::create(sync, width, height, nullptr, source);
}

bool video::sources::gstreamer_auto_source::create(bool sync, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_auto_source::create(sync, 0, 0, algo, source);
}

bool video::sources::gstreamer_auto_source::create(bool sync, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_auto_source::create(sync, nullptr, source);
}

bool video::sources::gstreamer_auto_source::create(core::video::video_source_interface** source)
{
	return video::sources::gstreamer_auto_source::create(true, source);
}
