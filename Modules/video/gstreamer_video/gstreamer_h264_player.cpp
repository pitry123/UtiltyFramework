#include "gstreamer_h264_player.h"

bool video::sources::gstreamer_h264_player::create(core::video::video_source_interface* h264_source, bool live, core::imaging::pixel_format output_format, bool async, core::video::video_source_interface** source)
{
	if (source == nullptr)
		return false;

	utils::ref_count_ptr<core::video::video_source_interface> instance;
	try
	{
		instance = utils::make_ref_count_ptr<gstreamer_h264_player_impl>(h264_source, live, output_format, async);
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

bool video::sources::gstreamer_h264_player::create(core::video::video_source_interface* h264_source, bool live, core::imaging::pixel_format output_format, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_h264_player::create(h264_source, live, output_format, true, source);
}

bool video::sources::gstreamer_h264_player::create(core::video::video_source_interface* h264_source, bool live, core::video::video_source_interface** source)
{
	return video::sources::gstreamer_h264_player::create(h264_source, live, core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT, source);
}
