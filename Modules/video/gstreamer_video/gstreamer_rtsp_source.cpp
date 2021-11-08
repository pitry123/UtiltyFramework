#include "gstreamer_rtsp_source.h"
#include "common/gstreamer_utils.hpp"
#include <utils/strings.hpp>

#include <limits>

gboolean video::sources::gstreamer_rtsp_source_impl::on_select_stream(GstElement* rtspsrc, guint idx, GstCaps* caps, gpointer user_data)
{
	return static_cast<video::sources::gstreamer_rtsp_source_impl*>(user_data)->is_streams_allowed(idx);
}

inline gboolean video::sources::gstreamer_rtsp_source_impl::is_streams_allowed(guint index)
{
	if (m_stream_index == (std::numeric_limits<uint32_t>::max)())
		return TRUE;

	return (index == static_cast<guint>(m_stream_index)) ? TRUE : FALSE;
}

std::string video::sources::gstreamer_rtsp_source_impl::pipeline_description()
{
	std::stringstream description_builder;
	
	// rtspcsrc
	description_builder <<
		"rtspsrc name=rtspsrc" << index() << " location=" << m_uri.c_str();	

	if (m_live == true)
		description_builder << " latency=" << m_latency;

	if (m_multicast == true)
	{
		description_builder << " protocols=\"udp-mcast\"";
		if (m_nic_name.empty() == false)
		{
			description_builder << " multicast-iface=\"" << m_nic_name.c_str() << "\"";
		}
	}

	if (m_rtp_timestamps == true)
		description_builder << " ! rtpatimeparse name=rtpatimeparse" << index();

	if (m_data_type == core::video::video_data_type::RAW)
	{
		description_builder << " ! rtph264depay name=rtph264depay" << index() << " ! ";

		// parser and decoder
		description_builder <<
			"h264parse name=h264parse" << index() << " ! ";

		bool intel_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("mfxdecode");
		bool omx_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("omxh264dec");
		bool d3d11_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("d3d11h264dec");
		bool nvidia_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("nvh264sldec");		

		if (intel_hardware_accelerated)
		{
			description_builder <<
				"mfxdecode name=mfxdecode" << index();

			if (m_live == true)
				description_builder << " live_mode=true";

			description_builder << " ! ";
		}
        else if (omx_hardware_accelerated)
        {
            description_builder <<
                "omxh264dec name=omxh264dec" << index() << " ! ";
        }
		else if (d3d11_hardware_accelerated)
		{
			description_builder <<
				"d3d11h264dec name=d3d11h264dec" << index() << " ! video/x-raw ! ";
		}
		else if (nvidia_hardware_accelerated)
		{
			description_builder <<
				"nvh264sldec name=nvh264sldec" << index() << " ! video/x-raw ! ";
		}		
		else
		{
			description_builder <<
				"avdec_h264 name=avdec_h264" << index() << " ! ";
		}

		if (m_output_format != core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT)
		{
			if (intel_hardware_accelerated == true && m_output_format == core::imaging::pixel_format::BGRA)
			{
				// mfxh264dec supports hardware conversion to BGRA
				description_builder << "video/x-raw, format=" << m_output_format << " ! ";
			}
			else
			{
				description_builder <<
					"videoconvert name=videoconvert" << index() << " ! video/x-raw, format=" << m_output_format << " ! ";
			}
		}

		// appsink
		description_builder <<
			"appsink name=appsink" << index();

		// Currently low latency doesn't play well with mfxdecode
		// So we remove timestamps sync
		if (m_live == true)
			description_builder << " sync=false";
	}
	else
	{
		description_builder << " ! rtph264depay name=rtph264depay" << index() << " ! ";

		description_builder << "h264parse name=h264parse" << index() << " ! video/x-h264, alignment=au, stream-format=byte-stream ! ";
		// appsink
		description_builder <<
			"appsink name=appsink" << index() << " sync=false";
	}

	return description_builder.str();
}

inline void video::sources::gstreamer_rtsp_source_impl::on_pipeline_constructed(GstElement * pipeline)
{
	video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_rtsp_source>>::on_pipeline_constructed(pipeline);

	std::stringstream rtspsrc_name;
	rtspsrc_name << "rtspsrc" << index();
	m_rtspsrc_element = gst_bin_get_by_name(GST_BIN(pipeline), rtspsrc_name.str().c_str());

	if (m_rtspsrc_element == nullptr)
		throw std::runtime_error("Failed to find the rtspsrc element");

	m_rtspsrc_stream_select_signal_handler = g_signal_connect(m_rtspsrc_element, "select-stream", G_CALLBACK(video::sources::gstreamer_rtsp_source_impl::on_select_stream),
		this);
}

inline void video::sources::gstreamer_rtsp_source_impl::on_pipeline_completed()
{
	if (m_rtspsrc_element != nullptr)
	{
		g_signal_handler_disconnect(m_rtspsrc_element, m_rtspsrc_stream_select_signal_handler);
	}

	video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_rtsp_source>>::on_pipeline_completed();
}

video::sources::gstreamer_rtsp_source_impl::gstreamer_rtsp_source_impl(const char* uri, bool live, bool multicast, bool rtp_timestamps, const char* nic_name, core::imaging::pixel_format output_format, core::imaging::image_algorithm_interface* algo, uint32_t stream_index, core::video::video_data_type data_type) :
	video::sources::gstreamer_base_video_source<utils::video::video_source_base<video::sources::gstreamer_rtsp_source>>(algo, (data_type == core::video::video_data_type::H264) ? false : true),
	m_rtspsrc_element(nullptr), m_rtspsrc_stream_select_signal_handler(0), m_stream_index(stream_index), m_data_type(data_type),
    m_uri(uri), m_live(live), m_multicast(multicast), m_rtp_timestamps(rtp_timestamps), m_nic_name((nic_name != nullptr) ? nic_name : ""), m_output_format(output_format), m_latency(0)
{
}

video::sources::gstreamer_rtsp_source_impl::~gstreamer_rtsp_source_impl()
{
	stop();
}

bool video::sources::gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, bool rtp_timestamps, const char* network_device_name, core::imaging::pixel_format output_format, core::imaging::image_algorithm_interface* algo, uint32_t stream_index, core::video::video_data_type data_type, core::video::video_source_interface** source)
{
	if (url == nullptr)
		return false;

	if (source == nullptr)
		return false;

	utils::ref_count_ptr<core::video::video_source_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<gstreamer_rtsp_source_impl>(url, live, multicast, rtp_timestamps, network_device_name, output_format, algo, stream_index, data_type);
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

bool video::sources::gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, bool rtp_timestamps, const char* nic_name, core::imaging::pixel_format output_format, core::imaging::image_algorithm_interface* algo, uint32_t stream_index, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_rtsp_source::create(url, live, multicast, rtp_timestamps, nic_name, output_format, algo, stream_index, core::video::video_data_type::RAW, source);
}

bool video::sources::gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, bool rtp_timestamps, const char* nic_name, core::imaging::pixel_format output_format, core::imaging::image_algorithm_interface* algo, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_rtsp_source::create(url, live, multicast, rtp_timestamps, nic_name, output_format, algo, (std::numeric_limits<uint32_t>::max)(), source);
}

bool video::sources::gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, bool rtp_timestamps, const char* nic_name, core::imaging::pixel_format output_format, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_rtsp_source::create(url, live, multicast, rtp_timestamps, nic_name, output_format, nullptr, source);
}

bool video::sources::gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, bool rtp_timestamps, const char* nic_name, core::video::video_source_interface** source)
{
    return video::sources::gstreamer_rtsp_source::create(url, live, multicast, rtp_timestamps, nullptr, core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT, source);
}

bool video::sources::gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, bool rtp_timestamps, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_rtsp_source::create(url, live, multicast, rtp_timestamps, nullptr, source);
}

bool video::sources::gstreamer_rtsp_source::create(const char* url, bool live, bool multicast, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_rtsp_source::create(url, live, multicast, false, nullptr, source);
}

bool video::sources::gstreamer_rtsp_source::create(const char* url, core::video::video_source_interface** source)
{
    return video::sources::gstreamer_rtsp_source::create(url, false, false, source);
}
