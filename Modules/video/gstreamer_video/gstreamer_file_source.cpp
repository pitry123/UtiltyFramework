#include "gstreamer_file_source.h"
#include "common/gstreamer_utils.hpp"

std::string video::sources::gstreamer_file_source_impl::pipeline_description()
{
	bool intel_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("mfxdecode");

	std::stringstream description_builder;
	
	// rtspcsrc
	description_builder <<
		"filesrc name=filesrc" << index() << " location=" << m_file_path.c_str() << " ! ";

	// parser and decoder
	description_builder <<
		"parsebin name=parsebin" << index() << " ! ";

	if (intel_hardware_accelerated)
	{
		description_builder <<
			"mfxdecode name=mfxdecode" << index() << " ! video/x-raw, format=BGRA ! ";
	}
	else
	{
		description_builder <<
			"avdec_h264 name=avdec_h264" << index() << " ! " <<
			"videoconvert name=videoconvert" << index() << " ! video/x-raw, format=RGBA ! ";
	}

	// appsink
	description_builder <<
		"appsink name=appsink" << index();

	return description_builder.str();
}

video::sources::gstreamer_file_source_impl::gstreamer_file_source_impl(const char* uri, core::imaging::image_algorithm_interface* algo) :
	video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_file_source>>(algo),
	m_file_path(uri)
{
}

video::sources::gstreamer_file_source_impl::~gstreamer_file_source_impl()
{
	stop();
}

bool video::sources::gstreamer_file_source::create(const char* file_path, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source)
{
	if (file_path == nullptr)
		return false;

	if (source == nullptr)
		return false;

	utils::ref_count_ptr<core::video::video_source_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<gstreamer_file_source_impl>(file_path, algo);
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

bool video::sources::gstreamer_file_source::create(const char* file_path, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_file_source::create(file_path, nullptr, source);
}
