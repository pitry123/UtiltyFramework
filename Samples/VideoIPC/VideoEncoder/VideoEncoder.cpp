// VideoEncoder.cpp : Defines the entry point for the console application.
//

#ifdef _WIN32
#include "windows.h"
#endif // _WIN32

#include <video/sinks/gstreamer_mfx_h264enc.h>
#include <video/sources/shared_memory_video_source.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>
#include <Video.hpp>
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>

#define SECONDS_TO_NANO_SECONDS 1000000000
#define MAX_RENDERED_STREAMS 4

using namespace Video;

std::shared_ptr<VideoSource> source;
utils::ref_count_ptr<core::video::video_sink_interface> sink;

#ifdef _WIN32

BOOL ctrl_handler(DWORD event)
{
	if (event == CTRL_CLOSE_EVENT)
	{
		source->Stop();
		sink->stop();

		source.reset();
		sink.release();

		return TRUE;
	}

	return FALSE;
}

#endif // _WIN32

class SharedMemorySourceFactory :
	public utils::ref_count_base<core::video::video_source_factory_interface>
{
private:
	std::string m_video_name;
public:
	SharedMemorySourceFactory(const char* video_name) :
		m_video_name(video_name)
	{
	}

	virtual bool create(core::video::video_source_interface** video_source) const override
	{
		return video::sources::shared_memory_video_source::create(m_video_name.c_str(), video_source);
	}
};

int main(int argc, const char* argv[])
{
#ifdef _WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)(ctrl_handler), TRUE);
#endif // _WIN32

    if (argc < 3)
	{
        printf("Expected command line arguments: <video_name> <output_path> <optinal:bitrate>\n");
		return -1;
    }
    uint16_t bitrate = 0;
    if(argc > 3)
    {
        try
        {
            bitrate = static_cast<uint16_t>(std::stoul(argv[3]));
        }
        catch(...)
        {
            printf("Unable to parse bitrate. range is  0-65535\n");
            return -1;
        }
    }

	sink.release();
    if (video::sinks::gstreamer_mfx_h264enc::create(argv[2], bitrate, &sink) == false)
	{
		printf("Failed to create video sink\n");
		return -1;
	}	

	utils::ref_count_ptr<core::video::video_source_factory_interface> factory =
		utils::make_ref_count_ptr<SharedMemorySourceFactory>(argv[1]);

	source = std::make_shared<VideoSource>(factory);	
	source->OnFrame() += [](const Frame& frame)
	{	
		utils::ref_count_ptr<core::video::frame_interface> coreFrame;
		frame.UnderlyingObject(&coreFrame);
		sink->set_frame(coreFrame);
	};

	sink->start();
	source->Start();		

	printf("Encoder loaded...\npress Enter to stop encoding...");
	getchar();
	printf("\n");

	source->Stop();
	sink->stop();

    return 0;
}

