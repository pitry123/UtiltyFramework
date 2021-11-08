#include "gstreamer_h264_encoder.h"
#include "common/gstreamer_utils.hpp"
#include <utils/strings.hpp>

#define MAX_PENDING_FRAMES 10

bool video::sinks::gstreamer_h264_encoder_impl::PushFrame(core::video::frame_interface* frame)
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
	
	bool success = true;
	utils::scope_guard buffer_releaser([&]()
	{
		if (success == false && gstbuffer != nullptr)
			gst_buffer_unref(gstbuffer);
	});

	frame->add_ref();
	gsize size = image_params.size;	
	gstbuffer = gst_buffer_new_wrapped_full((GstMemoryFlags)0, (gpointer)(buffer->data()), size, 0, size, frame,
		[](gpointer data)
	{
		core::video::frame_interface* pframe = static_cast<core::video::frame_interface*>(data);
		pframe->release();
	});

	GST_BUFFER_PTS(gstbuffer) = display_params.pts;
	GST_BUFFER_DURATION(gstbuffer) = display_params.duration;

	auto ret = gst_app_src_push_buffer(GST_APP_SRC(m_appsrc), gstbuffer);
	if (ret != GST_FLOW_OK)
	{
		// TODO: Log error...
		success = false;
		return false;
	}

	return true;
}

int video::sinks::gstreamer_h264_encoder_impl::PushFrames()
{
	std::unique_lock<std::mutex> locker(m_mutex);
	m_wait_handle.wait(locker, [this]()
	{
		return ((m_running == false) || (m_pending_frames.size() > 0));
	});

	if (m_running == false)
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
		if (PushFrame(frame) == true)
			++retval;
	}

	return retval;
}

std::string video::sinks::gstreamer_h264_encoder_impl::pipeline_description()
{
	bool intel_hardware_accelerated = video::helpers::gstreamer_helpers::has_element("mfxdecode");


	std::stringstream stream;

	if (intel_hardware_accelerated == true)
	{
		// mfxh264enc
		stream <<
			"appsrc name=appsrc" << m_index << " ! "
			"mfxh264enc name=mfxh264enc" << m_index << " " <<
			"rate-control=" << (m_bitrate == 0 ? 3 : 1) << " " <<
			"max-bitrate=" << m_bitrate << " " <<
			"bitrate=" << m_bitrate << " ! " <<
			"avimux name=avimux" << m_index << " ! " <<
			"filesink name=filesink" << m_index << " location=" << m_file_path.c_str();
	}
	else
	{
		stream <<
			"appsrc name=appsrc" << m_index << " ! "
			"videoconvert ! video/x-raw, format=I420 ! x264enc speed-preset=1 tune=zerolatency bitrate=" << ((m_bitrate == 0) ? 8192 : m_bitrate) << " ! " <<
			"avimux name=avimux" << m_index << " ! " <<
			"filesink name=filesink" << m_index << " location=" << m_file_path.c_str();		
	}

	return stream.str();
}

void video::sinks::gstreamer_h264_encoder_impl::appsrc_need_data(GstElement *appsrc, guint unused_size, gpointer user_data)
{
	gstreamer_h264_encoder_impl* impl = static_cast<video::sinks::gstreamer_h264_encoder_impl*>(user_data);

	while (impl->m_running == true)
	{
		if (impl->PushFrames() > 0)
			break;
	}
}

void video::sinks::gstreamer_h264_encoder_impl::on_pipeline_constructed(GstElement* pipeline)
{
	std::stringstream appsink_name;
	appsink_name << "appsrc" << m_index;
	m_appsrc = gst_bin_get_by_name(GST_BIN(pipeline), appsink_name.str().c_str());

	if (m_appsrc == nullptr)
		throw std::runtime_error("Failed to find the appsrc element");

	g_object_set(G_OBJECT(m_appsrc),
		"stream-type", 0, // GST_APP_STREAM_TYPE_STREAM
		"format", GST_FORMAT_TIME,
		"is-live", TRUE,
		NULL);

	g_signal_connect(m_appsrc, "need-data", G_CALLBACK(video::sinks::gstreamer_h264_encoder_impl::appsrc_need_data), this);
	gst_app_src_set_size(GST_APP_SRC(m_appsrc), MAX_PENDING_FRAMES * 2);

	m_running = true;
	m_first_frame = true;
}

void video::sinks::gstreamer_h264_encoder_impl::on_pipeline_completed()
{
	m_running = false;
	m_wait_handle.notify_one();
}

void video::sinks::gstreamer_h264_encoder_impl::on_pipeline_destroyed()
{
	if (m_appsrc != nullptr)
		gst_object_unref(m_appsrc);

	m_appsrc = nullptr;
}

video::sinks::gstreamer_h264_encoder_impl::gstreamer_h264_encoder_impl(const char* file_path, uint16_t bitrate) :
    m_file_path(file_path), m_bitrate(bitrate), m_running(false)
{
    static std::atomic<int> INSTANCE_COUNTER{0};
	m_index = ++INSTANCE_COUNTER;	

	m_pending_frames.reserve(MAX_PENDING_FRAMES);
	m_working_frames.reserve(MAX_PENDING_FRAMES);
}

video::sinks::gstreamer_h264_encoder_impl::~gstreamer_h264_encoder_impl()
{
	stop();
}

bool video::sinks::gstreamer_h264_encoder_impl::set_frame(core::video::frame_interface* frame)
{
	if (frame == nullptr)
		return false;

	if (m_running == false)
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

		std::stringstream interlace_mode;
        interlace_mode << video_params.interlace_mode;

		g_object_set(G_OBJECT(m_appsrc), "caps",
			gst_caps_new_simple("video/x-raw",
				"format", G_TYPE_STRING, format.str().c_str(),
				"width", G_TYPE_INT, width,
				"height", G_TYPE_INT, height,
				"framerate", GST_TYPE_FRACTION, fractionNumerator, fractionDenominator,
				"interlace-mode", G_TYPE_STRING, interlace_mode.str().c_str(),
				nullptr), nullptr);

		m_first_frame = false;
	}

	bool retval = false;
	std::unique_lock<std::mutex> locker(m_mutex);
	if (m_pending_frames.size() < MAX_PENDING_FRAMES)
	{
		m_pending_frames.emplace_back(frame);
		retval = true;
	}
	locker.unlock();

	m_wait_handle.notify_one();
	return retval;
}

bool video::sinks::gstreamer_mfx_h264enc::create(const char* file_path, uint16_t bitrate, core::video::video_sink_interface** sink)
{
	if (file_path == nullptr)
		return false;

	if (sink == nullptr)
		return false;

	utils::ref_count_ptr<core::video::video_sink_interface> instance;
	try
	{
        instance = utils::make_ref_count_ptr<gstreamer_h264_encoder_impl>(file_path, bitrate);
	}
	catch (...)
	{
		return false;
	}

    if (instance == nullptr)
        return false;

	*sink = instance;
	(*sink)->add_ref();

	return true;
}

bool video::sinks::gstreamer_mfx_h264enc::create(const char* file_path, core::video::video_sink_interface** sink)
{
    return video::sinks::gstreamer_mfx_h264enc::create(file_path, static_cast<uint16_t>(0), sink);
}
