#include "gstreamer_custom_source.h"

std::string video::sources::gstreamer_custom_source_impl::appsink_name() const
{
	return m_appsink_name;
}

std::string video::sources::gstreamer_custom_source_impl::pipeline_description()
{
	return m_pipeline.c_str();
}

video::sources::gstreamer_custom_source_impl::gstreamer_custom_source_impl(const char* pipeline, const char* appsink_name, core::imaging::image_algorithm_interface* algo) :
	video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_custom_source>>(algo),
	m_pipeline(pipeline == nullptr ? "" : pipeline),
	m_appsink_name(appsink_name == nullptr ? "" : appsink_name)
{
	if (pipeline == nullptr)
		throw std::invalid_argument("pipeline");

	if (appsink_name == nullptr)
		throw std::invalid_argument("appsink_name");
}

video::sources::gstreamer_custom_source_impl::~gstreamer_custom_source_impl()
{
	stop();
}

bool video::sources::gstreamer_custom_source::create(const char* pipeline, const char* appsink_name, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source)
{
	if (pipeline == nullptr)
		return false;

	if (appsink_name == nullptr)
		return false;

	utils::ref_count_ptr<core::video::video_source_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<gstreamer_custom_source_impl>(pipeline, appsink_name, algo);
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

bool video::sources::gstreamer_custom_source::create(const char* pipeline, const char* appsink_name, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_custom_source::create(pipeline, appsink_name, nullptr, source);
}