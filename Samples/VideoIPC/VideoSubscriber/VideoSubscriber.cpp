// VideoSubscriber.cpp : Defines the entry point for the console application.
//

#ifdef _WIN32
#include "windows.h"
#endif // _WIN32

#include "viewer.h"
#include <video/sources/shared_memory_video_source.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <Core.hpp>
#include <Video.hpp>
#include <Factories.hpp>

#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>

#define SECONDS_TO_NANO_SECONDS 1000000000
#define MAX_RENDERED_STREAMS 4

std::vector<Video::VideoSource> sources;
std::shared_ptr<utils::viewer> viewer;

#ifdef _WIN32

BOOL ctrl_handler(DWORD event)
{
	if (event == CTRL_CLOSE_EVENT)
	{
		sources.clear();
		viewer.reset();
		return TRUE;
	}

	return FALSE;
}

#endif // _WIN32

// constexpr int VIDEO_DOMAIN = (std::numeric_limits<int>::max)();

int main(int argc, const char* argv[])
{
#ifdef _WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)(ctrl_handler), TRUE);
#endif // _WIN32

	if (argc < 2)
	{
		printf("Expected command line arguments: <video_name>\n");
		return -1;
	}	

	std::mutex viewerMutex;
	std::condition_variable viewerCondition;
	bool running = true;

    int streamCount = ((argc - 1) < MAX_RENDERED_STREAMS) ? (argc - 1) : MAX_RENDERED_STREAMS;
    viewer = std::make_shared<utils::viewer>(streamCount, 1024, 768, [&]()
	{
		std::unique_lock<std::mutex> locker(viewerMutex);
		running = false;
		locker.unlock();

		viewerCondition.notify_one();
	}, "Rendered By Suzi");

	struct FpsHelper
	{
		int incomingCounter = 0;
		std::chrono::high_resolution_clock::time_point incomingTimestamp = (std::chrono::high_resolution_clock::time_point::min)();
	};

	std::vector<FpsHelper> helpers;
    for (size_t i = 1; i < static_cast<size_t>(argc); i++)
	{	
		sources.emplace_back(Video::VideoSource(Video::VideoSourceFactory::Get<Video::Sources::SharedMemoryVideoSourceFactory>(argv[i])));
		helpers.emplace_back(FpsHelper());
        size_t source_index = i - 1;
		sources[source_index].OnFrame() += [source_index, &helpers](const Video::Frame& frame)
		{		
			if (source_index < MAX_RENDERED_STREAMS)
			{
				utils::ref_count_ptr<core::video::frame_interface> coreFrame;
				frame.UnderlyingObject(&coreFrame);
				viewer->show_frame(coreFrame, source_index);
			}

			auto now = std::chrono::high_resolution_clock::now();
			if (helpers[source_index].incomingTimestamp == (std::chrono::high_resolution_clock::time_point::min)())
			{
				helpers[source_index].incomingTimestamp = now;
				return;
			}

			++(helpers[source_index].incomingCounter);
			auto totalNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(now - helpers[source_index].incomingTimestamp).count();
			if (totalNanoseconds >= static_cast<long long>(static_cast<long long>(5) * SECONDS_TO_NANO_SECONDS))
			{
				double messagesPerSecond = (static_cast<double>(helpers[source_index].incomingCounter) / (static_cast<double>(totalNanoseconds) / static_cast<double>(SECONDS_TO_NANO_SECONDS)));

				Core::Console::ColorPrint(
					Core::Console::Colors::WHITE,
					"Source %d incoming throughput: %lf frames / sec\n", source_index, messagesPerSecond);

				helpers[source_index].incomingTimestamp = now;
				helpers[source_index].incomingCounter = 0;
			}
		};

		sources[source_index].Start();
	}	

	printf("Subscriber loaded...\n");
	std::unique_lock<std::mutex> locker(viewerMutex);
	viewerCondition.wait(locker, [&]()
	{
		return (running == false);
	});
	
	locker.unlock();

	sources.clear();
    return 0;
}

