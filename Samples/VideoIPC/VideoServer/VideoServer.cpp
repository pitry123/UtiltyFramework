// VideoServer.cpp : Defines the entry point for the console application.
//

#ifdef _WIN32
#include "windows.h"
#endif // _WIN32

#include <video/sources/gstreamer_test_source.h>
#include <video/sources/gstreamer_auto_source.h>
#include <video/publishers/gstreamer_rtsp_server.h>
#include <video/publishers/gstreamer_rtsp_launch.h>
#include <utils/ref_count_base.hpp>
#include <utils/ref_count_ptr.hpp>

#include <Video.hpp>
#include <Strings.hpp>
#include <Factories.hpp>

#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <map>
#include <cctype>

std::map<std::string, std::string> m_arguments_map;
utils::ref_count_ptr<core::video::video_publisher_interface> server;

std::string str_tolower(std::string s) 
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); } );
	return s;
}

void parse_command_line_options(int argc, const char* argv[])
{
    std::regex key_val_pattern("-([a-zA-Z0-9]+):(.*)");

    for (int i=1; i<argc; i++)
    {
        std::string key_val(argv[i]);
        std::smatch pattern_match;

        if (std::regex_match(key_val, pattern_match, key_val_pattern) == false)
            continue;

        if (pattern_match.size() != 3)
            continue;

		std::string key = str_tolower(pattern_match[1]);
        std::string val = pattern_match[2];

        m_arguments_map[key] = val;
    }
}

std::string get_option(std::string option_name)
{
    auto it = m_arguments_map.find(str_tolower(option_name));
    if (it == m_arguments_map.end())
        return "";

    return it->second;
}

#ifdef _WIN32

BOOL ctrl_handler(DWORD event)
{
	if (event == CTRL_CLOSE_EVENT) 
	{
		server.release();
		return TRUE;
	}

	return FALSE;
}

#endif // _WIN32

int main(int argc, const char* argv[])
{
    parse_command_line_options(argc, argv);

#ifdef _WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)(ctrl_handler), TRUE);
#endif // _WIN32	

	std::string uri = get_option("uri");
	if (uri.empty() == true)
	{
		printf("Expected command line arguments:\n-uri:<uri>\n[optional: -name:<shm_source_name>]\n[optional: -minMCAddr:<ip_addr> -maxMCAddr:<ip_addr> -minMCPort:<port> -maxMCPort:<port>]\n\nExample for multicast video server bounded to VideoPublisher source named suzi:\nVideoServer -uri:stream -name:suzi -minMCAddr:230.5.4.0 -maxMCAddr:230.5.4.10 -minMCPort:6000 -maxMCPort:6010\n");
		return -1;
	}   	    

	uint32_t port = 8554;
	std::string port_option = get_option("port");
	if (port_option.empty() == false)
	{
		if (parse(port_option.c_str(), port) == false)
		{
			printf("Invalid command line argument: port\n");
			return -1;
		}

		if (port > (std::numeric_limits<uint16_t>::max)())
		{
			printf("Invalid command line argument: port. Max value is 65535\n");
			return -1;
		}
	}

	std::string min_mc_addr = get_option("minMCAddr");
	std::string max_mc_addr = get_option("maxMCAddr");

	uint32_t min_mc_port = 0;
	std::string min_mc_port_option = get_option("minMCPort");
	if (min_mc_port_option.empty() == false)
	{
		if (parse(min_mc_port_option.c_str(), min_mc_port) == false)
		{
			printf("Invalid command line argument: minMCPort\n");
			return -1;
		}

		if (min_mc_port > (std::numeric_limits<uint16_t>::max)())
		{
			printf("Invalid command line argument: minMCPort. Max value is 65535\n");
			return -1;
		}
	}

	uint32_t max_mc_port = 0;
	std::string max_mc_port_option = get_option("maxMCPort");
	if (max_mc_port_option.empty() == false)
	{
		if (parse(max_mc_port_option.c_str(), max_mc_port) == false)
		{
			printf("Invalid command line argument: maxMCPort\n");
			return -1;
		}

		if (max_mc_port > (std::numeric_limits<uint16_t>::max)())
		{
			printf("Invalid command line argument: maxMCPort. Max value is 65535\n");
			return -1;
		}
	}

	std::function<void()> create_server = nullptr;

	std::string pipeline = get_option("pipeline");
	if (pipeline.empty() == false)
	{
		create_server = [&]()
		{
			server.release();

			printf("Creating server...\nURL is: %s\nPort is: %lu\n",
				uri.c_str(),
				static_cast<unsigned long>(port));

			if (video::publishers::gstreamer_rtsp_launch::create(
				uri.c_str(),
				static_cast<uint16_t>(port),
				pipeline.c_str(),
				min_mc_addr.empty() == true ? nullptr : min_mc_addr.c_str(),
				max_mc_addr.empty() == true ? nullptr : max_mc_addr.c_str(),
				static_cast<uint16_t>(min_mc_port),
				static_cast<uint16_t>(max_mc_port),
				&server) == false)
				throw;
		};
	}
	else
	{
		utils::ref_count_ptr<core::video::video_source_factory_interface> factory;
		std::string name = get_option("name");
		if (name.empty() == false)
		{
			factory = utils::make_ref_count_ptr<Video::Sources::SharedMemoryVideoSourceFactory>(name.c_str());
		}
		else
		{
			uint32_t width = 1920;
			uint32_t height = 1080;
			core::imaging::pixel_format format = core::imaging::pixel_format::YUY2;
			core::video::framerate framerate{ 30, 1 };
			factory = utils::make_ref_count_ptr<Video::Sources::TestVideoSourceFactory>(width, height, format, framerate, nullptr);
		}

		create_server = [&, factory]()
		{
			server.release();

			printf("Creating server...\nURL is: %s\nPort is: %lu\n",
				uri.c_str(),
				static_cast<unsigned long>(port));

			if (video::publishers::gstreamer_rtsp_server::create(
				uri.c_str(),
				static_cast<uint16_t>(port),
				min_mc_addr.empty() == true ? nullptr : min_mc_addr.c_str(),
				max_mc_addr.empty() == true ? nullptr : max_mc_addr.c_str(),
				static_cast<uint16_t>(min_mc_port),
				static_cast<uint16_t>(max_mc_port),
				factory,
				&server) == false)
				throw;						
		};		
	}	

	if (create_server == nullptr)
		throw std::runtime_error("Failed to create server creation logicv");
	
	create_server();
	server->start();

	printf("press Enter to stop the server...\n");
	getchar();	    

	server.release();	
    return 0;
}

