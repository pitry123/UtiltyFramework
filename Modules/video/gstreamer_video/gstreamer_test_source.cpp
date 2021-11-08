#include "gstreamer_test_source.h"
#include <utils/strings.hpp>

#include <algorithm>

std::string video::sources::gstreamer_test_source_impl::pipeline_description()
{
	std::stringstream description_builder;
	
	// rtspcsrc
	description_builder <<
		"videotestsrc name=videotestsrc" << index() << " ! ";	

	description_builder <<
		"video/x-raw," <<
		" width=" << m_width << ", " <<
		" height=" << m_height << ", " <<
		" format=" << m_format << ", " <<
		" framerate=" << m_framerate.numerator << "/" << m_framerate.denominator << " ! ";

	// appsink
	description_builder <<
		"appsink name=appsink" << index();

	return description_builder.str();
}

video::sources::gstreamer_test_source_impl::gstreamer_test_source_impl(uint32_t width, uint32_t height, core::imaging::pixel_format format, const core::video::framerate& framerate, core::imaging::image_algorithm_interface* algo) :
	video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_test_source>>(algo),
	m_width(width), m_height(height), m_format(format), m_framerate(framerate)
{
	if (width == 0 || height == 0)
		throw std::invalid_argument("width and/or height");
}

video::sources::gstreamer_test_source_impl::~gstreamer_test_source_impl()
{
	stop();
}

bool video::sources::gstreamer_test_source::create(uint32_t width, uint32_t height, core::imaging::pixel_format format, const core::video::framerate& framerate, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source)
{
	if (source == nullptr)
		return false;

	utils::ref_count_ptr<core::video::video_source_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<gstreamer_test_source_impl>(width, height, format, framerate, algo);
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

bool video::sources::gstreamer_test_source::create(uint32_t width, uint32_t height, core::imaging::pixel_format format, const core::video::framerate& framerate, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_test_source::create(width, height, format, framerate, nullptr, source);
}
