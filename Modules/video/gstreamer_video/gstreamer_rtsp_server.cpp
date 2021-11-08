#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable: 4244) // conversion
#endif

#include "gstreamer_rtsp_server.h"
#include "common/gstreamer_utils.hpp"

#include <utils/video.hpp>
#include <utils/strings.hpp>
#include <libgstatime/gstatimemeta.h>

#include <iostream>
#include <sstream>

#define MAX_PENDING_FRAMES 2
#define APPSRC_NAME "9E0866AE-9869-4297-A3AB-71A8B4B4C3EA"

bool video::publishers::gstreamer_rtsp_server_impl::push_frame(core::video::frame_interface* frame)
{
	if (frame == nullptr)
		return false;

	core::imaging::image_params image_params;
	if (frame->query_image_params(image_params) == false)
		return false;

	core::video::display_params display_params;
	if (frame->query_display_params(display_params) == false)
		return false;

	core::video::video_params video_params;
	if (frame->query_video_params(video_params) == false)
		return false;

	utils::ref_count_ptr<core::buffer_interface> buffer;
	if (frame->query_buffer(&buffer) == false)
		return false;

	GstBuffer* gstbuffer = nullptr;

	frame->add_ref();
	gsize size = image_params.size;
	gstbuffer = gst_buffer_new_wrapped_full((GstMemoryFlags)0, (gpointer)(buffer->data()), size, 0, size, frame,
		[](gpointer data)
	{
		core::video::frame_interface* pframe = static_cast<core::video::frame_interface*>(data);
		pframe->release();
	});

	if (m_pts_offset == 0)
	{		
        gst_app_src_set_size(GST_APP_SRC(m_appsrc), static_cast<gint64>((MAX_PENDING_FRAMES * 2) * size));
		m_pts_offset = display_params.pts;
	}

	GST_BUFFER_PTS(gstbuffer) = (display_params.pts - m_pts_offset);
	GST_BUFFER_DURATION(gstbuffer) = display_params.duration;

	if (display_params.timestamp > 0)
	{
		gst_buffer_add_atime_meta(gstbuffer, static_cast<GstClockTime>(display_params.timestamp));
	}

	GstFlowReturn ret;
	//ret = gst_app_src_push_buffer(GST_APP_SRC(m_appsrc), gstbuffer);	
	g_signal_emit_by_name(m_appsrc, "push-buffer", gstbuffer, &ret);
	gst_buffer_unref(gstbuffer);
	if (ret != GST_FLOW_OK)
	{
		// TODO: Log error...
		//gst_buffer_unref(gstbuffer);
		return false;
	}

	return true;
}

int video::publishers::gstreamer_rtsp_server_impl::push_frames()
{
	m_stop_feed = false;

	std::unique_lock<std::mutex> locker(m_frames_mutex);
	bool initiated = true;
	m_wait_handle.wait(locker, [&]()
	{
		initiated = m_initiated.load();
		return ((initiated == false) || (m_pending_frames.size() > 0));
	});

	if (initiated == false)
		return 0;

	m_working_frames.swap(m_pending_frames);
	locker.unlock();

	utils::scope_guard clear_working_frames([this]()
	{
		m_working_frames.clear();
	});

	int retval = 0;
	for (auto& frame : m_working_frames)
	{
		if (m_stop_feed == true)
		{
			//printf("FRAMES DROPPED!!!\n");
			break;
		}

		if (push_frame(frame) == true)
		{
			++retval;
		}

		frame.release();		
	}

	return retval;
}

std::string video::publishers::gstreamer_rtsp_server_impl::pipeline_description()
{
	// Intel GPU
	bool intel_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("mfxh264enc");
	
	// nvidia ARM based
	bool nvidia_converter = video::helpers::gstreamer_helpers::has_element("nvvidconv");
	bool nvidia_omx_hardware_accelerated = nvidia_converter && video::helpers::gstreamer_helpers::has_element("omxh264enc");

	// nvidia GPU
	bool nvidia_gpu_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("nvh264enc");

	bool has_rtp_timestamp_adding = video::helpers::gstreamer_helpers::has_element("rtpatimetimestamp");

	std::stringstream stream;
	
	if (intel_hardware_accelerated == true)
	{
		stream <<
			"appsrc name=" << APPSRC_NAME << " is-live=true do-timestamp=true ! " <<
			"queue max-size-buffers=1 ! " <<
			"mfxh264enc preset=veryfast rate-control=cbr async-depth=1 gop-distance=1 ! " <<
			"queue max-size-buffers=1 ! ";
	}
    else if (nvidia_omx_hardware_accelerated == true)
    {
		stream <<
			"appsrc name=" << APPSRC_NAME << " is-live=true do-timestamp=true ! " <<
			"queue max-size-buffers=1 ! " <<
			"nvvidconv ! " <<
			"omxh264enc bitrate=0 ! " <<
			"queue max-size-buffers=1 ! ";
    }
	else if (nvidia_gpu_hardware_accelerated == true)
	{
		stream <<
			"appsrc name=" << APPSRC_NAME << " is-live=true do-timestamp=true ! " <<
			"queue max-size-buffers=1 ! " <<
			"glupload ! glcolorconvert ! " <<
			"nvh264enc preset=5 zerolatency=true ! video/x-h264, alignment=au, stream-format=byte-stream, profile=baseline ! " <<
			"queue max-size-buffers=1 ! ";
	}
	else
	{
		stream <<
			"appsrc name=" << APPSRC_NAME << " is-live=true do-timestamp=true ! " <<
			"queue max-size-buffers=1 ! " <<
			"videoconvert ! video/x-raw, format=I420 ! x264enc bitrate=8192 speed-preset=1 tune=zerolatency ! " <<
			"queue max-size-buffers=1 ! ";
	}

	if (has_rtp_timestamp_adding == true)
	{
		stream << "rtph264pay config-interval=1 pt=96 ! rtpatimetimestamp name=pay0";
	}
	else
	{
		stream << "rtph264pay config-interval=1 name=pay0 pt=96";
	}
	return stream.str();
}

void video::publishers::gstreamer_rtsp_server_impl::on_started()
{
	m_pts_offset = 0;
	m_source->start();
}

void video::publishers::gstreamer_rtsp_server_impl::on_stop()
{
	m_source->stop();
}

void video::publishers::gstreamer_rtsp_server_impl::appsrc_need_data(GstElement *appsrc, guint unused_size, gpointer user_data)
{
	gstreamer_rtsp_server_impl* impl = static_cast<video::publishers::gstreamer_rtsp_server_impl*>(user_data);
	impl->push_frames();
}

void video::publishers::gstreamer_rtsp_server_impl::stop_feed(GstElement *pipeline, gpointer user_data)
{
	static_cast<video::publishers::gstreamer_rtsp_server_impl*>(user_data)->m_stop_feed = true;
}

void video::publishers::gstreamer_rtsp_server_impl::on_pipeline_constructed(GstElement* pipeline)
{
	std::lock_guard<std::mutex> configure_lock(m_configure_mutex);

	GstElement* appsrc = gst_bin_get_by_name(GST_BIN(pipeline), APPSRC_NAME);
	if (appsrc == nullptr)
		throw std::runtime_error("Failed to find the appsrc element");

	if (m_appsrc == appsrc)
		return;

	m_pending_frames.clear();
	m_working_frames.clear();

	m_first_frame = true;
	m_initiated = false;
	m_pts_offset = 0;

	if (m_appsrc != nullptr)	
		gst_object_unref(m_appsrc);			

	m_appsrc = appsrc;	

	g_object_set(G_OBJECT(m_appsrc),
		"stream-type", 0, // GST_APP_STREAM_TYPE_STREAM
		"format", GST_FORMAT_TIME,
		"is-live", TRUE,
		NULL);	

	g_signal_connect(m_appsrc, "need-data", G_CALLBACK(video::publishers::gstreamer_rtsp_server_impl::appsrc_need_data), this);
	g_signal_connect(m_appsrc, "enough-data", G_CALLBACK(video::publishers::gstreamer_rtsp_server_impl::stop_feed), this);
	//gst_app_src_set_latency(GST_APP_SRC(m_appsrc), 0, 0);

	m_first_frame = true;
	m_initiated = true;
}

void video::publishers::gstreamer_rtsp_server_impl::on_pipeline_completed()
{
	base_class::on_pipeline_completed();

	m_initiated = false;
	m_wait_handle.notify_one();
}

void video::publishers::gstreamer_rtsp_server_impl::on_pipeline_destroyed()
{
	base_class::on_pipeline_destroyed();

	if (m_appsrc != nullptr)
		gst_object_unref(m_appsrc);

	m_appsrc = nullptr;
}

bool video::publishers::gstreamer_rtsp_server_impl::set_frame(core::video::frame_interface* frame)
{
	if (frame == nullptr)
		return false;

	if (m_initiated == false)
		return false;	

	if (m_first_frame == true)
	{
		core::imaging::image_params image_params;
		if (frame->query_image_params(image_params) == false)
			return false;

		core::video::display_params display_params;
		if (frame->query_display_params(display_params) == false)
			return false;

		core::video::video_params video_params;
		if (frame->query_video_params(video_params) == false)
			return false;

		std::stringstream format;
		format << image_params.format;

		int width = static_cast<int>(image_params.width);
		int height = static_cast<int>(image_params.height);
		int fractionNumerator = static_cast<int>(video_params.framerate.numerator);
		int fractionDenominator = static_cast<int>(video_params.framerate.denominator);

		std::string gstreamer_format = image_params.format == core::imaging::pixel_format::BGRA ?
			std::string("BGRx") :
			format.str();

		g_object_set(G_OBJECT(m_appsrc), "caps",
			gst_caps_new_simple("video/x-raw",
				"format", G_TYPE_STRING, gstreamer_format.c_str(),
				"width", G_TYPE_INT, width,
				"height", G_TYPE_INT, height,
				"framerate", GST_TYPE_FRACTION, fractionNumerator, fractionDenominator,
				nullptr), nullptr);

		m_first_frame = false;
	}

	bool retval = true;
	std::unique_lock<std::mutex> locker(m_frames_mutex);
	if (m_pending_frames.size() > MAX_PENDING_FRAMES)
	{
		retval = false;
	}

	if (retval == true)
		m_pending_frames.emplace_back(frame);

	locker.unlock();

	m_wait_handle.notify_one();
	return retval;
}

video::publishers::gstreamer_rtsp_server_impl::gstreamer_rtsp_server_impl(	
	const char* stream_name,
	uint16_t port,
	const char* minmum_multicast_address,
	const char* maximum_multicast_address,
	uint16_t minmum_multicast_port,
	uint16_t maximum_multicast_port,
	core::video::video_source_factory_interface* source_factory) :
	base_class(stream_name, port, minmum_multicast_address, maximum_multicast_address, minmum_multicast_port, maximum_multicast_port),
	m_initiated(false), m_stop_feed(false), m_first_frame(true), m_pts_offset(0), m_appsrc(nullptr)
{
	if (source_factory == nullptr || source_factory->create(&m_source) == false)
		throw std::invalid_argument("source_factory");

	m_frame_callback = utils::make_ref_count_ptr<utils::video::smart_frame_callback>([this](core::video::frame_interface* frame)
	{
		std::lock_guard<std::mutex> configure_lock(m_configure_mutex);
		set_frame(frame);
	});

	m_source->add_frame_callback(m_frame_callback);

	m_pending_frames.reserve(MAX_PENDING_FRAMES);
	m_working_frames.reserve(MAX_PENDING_FRAMES);
}

video::publishers::gstreamer_rtsp_server_impl::~gstreamer_rtsp_server_impl()
{
	stop();
	m_source->remove_frame_callback(m_frame_callback);
}

bool video::publishers::gstreamer_rtsp_server::create(
	const char* stream_name,
	uint16_t port,
	const char* minmum_multicast_address,
	const char* maximum_multicast_address,
	uint16_t minmum_multicast_port,
	uint16_t maximum_multicast_port,
	core::video::video_source_factory_interface* source_factory,
	core::video::video_publisher_interface** publisher)
{
	if (publisher == nullptr)
		return false;

	if (stream_name == nullptr)
		return false;

	if (source_factory == nullptr)
		return false;

	utils::ref_count_ptr<core::video::video_publisher_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<gstreamer_rtsp_server_impl>(
			stream_name, 
			port, 
			minmum_multicast_address,
			maximum_multicast_address, 
			minmum_multicast_port, 
			maximum_multicast_port, 
			source_factory);
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

bool video::publishers::gstreamer_rtsp_server::create(
	const char* stream_name, uint16_t port,
	core::video::video_source_factory_interface* source_factory,
	core::video::video_publisher_interface** publisher)
{
	return video::publishers::gstreamer_rtsp_server::create(stream_name, port, nullptr, nullptr, 0, 0, source_factory, publisher);
}

// Intel C++ does not properly keep warning state for function templates,
// so popping warning state at the end of translation unit leads to warnings in the middle.
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#	pragma warning(pop)
#endif
