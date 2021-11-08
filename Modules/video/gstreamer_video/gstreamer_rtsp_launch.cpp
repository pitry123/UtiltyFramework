#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4244) // conversion
#endif

#include "gstreamer_rtsp_launch.h"
#include <utils/strings.hpp>
#include <utils/ref_count_ptr.hpp>

std::string video::publishers::gstreamer_rtsp_launch_impl::pipeline_description()
{	
	return m_launch_pipeline;
}

video::publishers::gstreamer_rtsp_launch_impl::gstreamer_rtsp_launch_impl(
	const char* stream_name,
	uint16_t port, const char* launch_pipeline,
	const char* minmum_multicast_address,
	const char* maximum_multicast_address,
	uint16_t minmum_multicast_port,
	uint16_t maximum_multicast_port) :
	base_class(stream_name, port, minmum_multicast_address, maximum_multicast_address, minmum_multicast_port, maximum_multicast_port),
	m_launch_pipeline(launch_pipeline)
{
}

bool video::publishers::gstreamer_rtsp_launch::create(
	const char* stream_name,
	uint16_t port,
	const char* launch_pipeline,
	const char* minmum_multicast_address,
	const char* maximum_multicast_address,
	uint16_t minmum_multicast_port,
	uint16_t maximum_multicast_port,
	core::video::video_publisher_interface** publisher)
{
	if (publisher == nullptr)
		return false;

	if (stream_name == nullptr)
		return false;

	if (launch_pipeline == nullptr)
		return false;	

	utils::ref_count_ptr<core::video::video_publisher_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<gstreamer_rtsp_launch_impl>(
			stream_name, 
			port, 
			launch_pipeline,
			minmum_multicast_address,
			maximum_multicast_address,
			minmum_multicast_port,
			maximum_multicast_port);
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

bool video::publishers::gstreamer_rtsp_launch::create(
	const char* stream_name, uint16_t port,
	const char* launch_pipeline,
	core::video::video_publisher_interface** publisher)
{
	return video::publishers::gstreamer_rtsp_launch::create(stream_name, port, launch_pipeline, nullptr, nullptr, 0, 0, publisher);
}

// Intel C++ does not properly keep warning state for function templates,
// so popping warning state at the end of translation unit leads to warnings in the middle.
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#	pragma warning(pop)
#endif