// VideoPublisher.cpp : Defines the entry point for the console application.
//

#ifdef _WIN32
#include "windows.h"
#endif // _WIN32

#include <files/xml_file_interface.h>
#include <imaging/image_utils.h>

#include <Strings.hpp>
#include <Video.hpp>
#include <Factories.hpp>

#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <map>
#include <algorithm>

std::map<std::string, std::string> m_arguments_map;
Video::VideoPublisher publisher;

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

        std::string key = pattern_match[1];
        std::string val = pattern_match[2];

        m_arguments_map[key] = val;
    }
}

std::string get_option(std::string option_name)
{
    auto it = m_arguments_map.find(option_name);
    if (it == m_arguments_map.end())
        return "";

    return it->second;
}

#ifdef _WIN32

BOOL ctrl_handler(DWORD event)
{
	if (event == CTRL_CLOSE_EVENT) 
	{
		publisher = nullptr;
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

	std::string name = get_option("name");
	if (name.empty() == true)
	{
		printf("Expected command line arguments: -name:<video_name>\n");
		return -1;
	}

    std::string source = get_option("source");
    if (source.empty() == true)
	{
        printf("Expected command line argument: -source:<source_name>\nValid sources: 'rtsp' 'file' 'raw' 'test'\n");
		return -1;		
	}	    

	uint32_t buffer_size = Video::Publishers::SharedMemoryVideoPublisher::DEFAULT_VIDEO_BUFFER_SIZE;
	std::string buffer_size_option = get_option("buffersize");
	if (buffer_size_option.empty() == false)
	{
		if (parse(buffer_size_option.c_str(), buffer_size) == false)
		{
			printf("Invalid command line argument: buffersize\n");
			return -1;
		}
	}

	uint32_t buffer_pool_size = Video::Publishers::SharedMemoryVideoPublisher::DEFAULT_VIDEO_BUFFER_POOL_SIZE;
	std::string buffer_pool_size_option = get_option("poolsize");
	if (buffer_pool_size_option.empty() == false)
	{
		if (parse(buffer_pool_size_option.c_str(), buffer_pool_size) == false)
		{
			printf("Invalid command line argument: poolsize\n");
			return -1;
		}
	}

	Imaging::ImageAlgorithm algo;
	
	std::string distortion_params_file_path;
	distortion_params_file_path += "./";
	distortion_params_file_path += name;
	distortion_params_file_path += "_Distortion.xml";
	utils::ref_count_ptr<files::xml_file_interface> xml_file;
	if (files::xml_file_interface::create(distortion_params_file_path.c_str(), &xml_file) == true)
	{
		auto load_func = [&]()
		{			
			std::regex number_pattern("[-+]?(([0-9]+.[0-9]+f?)|[0-9]+)");

			utils::ref_count_ptr<files::xml_element_interface> width_element;
			if (xml_file->query_element("/Distortion/Width", &width_element) == false)
				return false;

			utils::ref_count_ptr<files::xml_element_interface> height_element;
			if (xml_file->query_element("/Distortion/Height", &height_element) == false)
				return false;

			utils::ref_count_ptr<files::xml_element_interface> matrix_element;
			if (xml_file->query_element("/Distortion/Matrix", &matrix_element) == false)
				return false;			

			utils::ref_count_ptr<files::xml_element_interface> coeffs_element;
			if (xml_file->query_element("/Distortion/Coeffs", &coeffs_element) == false)
				return false;

			std::string width_str = width_element->value();
			std::string height_str = height_element->value();
			std::string matrix_str = matrix_element->value();
			std::string coeffs_str = coeffs_element->value();
			
			std::smatch pattern_match;						

			uint32_t width;
			uint32_t height;
			float matrix[3][3];
			float coeffs[5];

			if (std::regex_search(width_str, pattern_match, number_pattern) == false)
				return false;

			if (parse(pattern_match.str().c_str(), width) == false)
				return false;

			if (std::regex_search(height_str, pattern_match, number_pattern) == false)
				return false;

			if (parse(pattern_match.str().c_str(), height) == false)
				return false;			

			size_t match_count = 0;
			std::string::const_iterator searchStart(matrix_str.cbegin());
			while (std::regex_search(searchStart, matrix_str.cend(), pattern_match, number_pattern) == true)
			{
				if (match_count == 9)
					return false;

				float val = 0.0f;
				if (parse(pattern_match.str().c_str(), val) == false)
					return false;				

				matrix[match_count / 3][match_count % 3] = val;
				++match_count;

				searchStart = pattern_match.suffix().first;
			}		

			if (match_count < 9)
				return false;

			match_count = 0;
			searchStart = coeffs_str.cbegin();
			while (std::regex_search(searchStart, coeffs_str.cend(), pattern_match, number_pattern) == true)
			{
				if (match_count == 5)
					return false;

				float val = 0.0f;
				if (parse(pattern_match.str().c_str(), val) == false)
					return false;

				coeffs[match_count] = val;
				++match_count;

				searchStart = pattern_match.suffix().first;
			}

			if (match_count < 5)
				return false;			

			algo = Imaging::ImageUndistortAlgorithm::Create(width, height, matrix, coeffs);
			if (algo.Empty() == true)
				return false;

			return true;
		};

		printf("Found distorion params file, trying to parse...\n");
		if (load_func() == false)
			printf("Failed to parse distorion params.\n");
		else
			printf("Distorion params parsed successfuly! Adding algorithm.\n");
	}

	Video::VideoSourceFactory factory;

    if (source == "rtsp")
	{
        // rtsp source
        std::string url = get_option("url");
        if (url.empty() == true)
		{
            printf("Missing command line arguments: -url:<rtsp_address>\n");
			return -1;
		}

		bool live = false;
		std::string live_option = get_option("live");
		if (live_option == "true")
			live = true;

		bool multicast = false;
        std::string multicast_option = get_option("multicast");
        if (multicast_option == "true")
			multicast = true;

		bool rtp_timestamps = false;
		std::string rtp_timestamps_option = get_option("timestamps");
		if (rtp_timestamps_option == "true")
			rtp_timestamps = true;

        std::string nicname_option = get_option("nicname");

        core::imaging::pixel_format format = core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT;
        std::string format_option = get_option("format");
        if (format_option.empty() == false)
        {
            if (parse(format_option.c_str(), format) == false)
            {
                printf("Invalid command line argument: format\n");
                return -1;
            }
        }

		uint32_t stream_index = (std::numeric_limits<uint32_t>::max)();
		std::string stream_option = get_option("stream");
		if (stream_option.empty() == false)
		{
			if (parse(stream_option.c_str(), stream_index) == false)
			{
				printf("Invalid command line argument: stream\n");
				return -1;
			}
		}

        factory = Video::VideoSourceFactory::Get<Video::Sources::RtspVideoSourceFactory>(
			url.c_str(),
			live,
			multicast,
			rtp_timestamps,
			nicname_option.empty() == true ? nullptr : nicname_option.c_str(),
			format,
			algo,
			stream_index);
    }
    else if (source == "file")
	{
        std::string path = get_option("path");
        if (path.empty() == true)
        {
            printf("Missing command line arguments: -path:<video_file_path>\n");
			return -1;
		}

        factory = Video::VideoSourceFactory::Get<Video::Sources::FileVideoSourceFactory>(
			path.c_str(),
			algo);
	}
    else if (source == "raw")
	{        
        std::string path_option = get_option("path");
        if (path_option.empty() == true)
        {
            printf("Missing command line arguments: -path:<video_file_path>\n");
            return -1;
        }

        std::string width_option = get_option("width");
        if (width_option.empty() == true)
        {
            printf("Missing command line arguments: -width:<width>\n");
            return -1;
        }

        uint32_t width;
        if (parse(width_option.c_str(), width) == false)
        {
            printf("Invalid command line argument: width\n");
            return -1;
        }

        std::string height_option = get_option("height");
        if (height_option.empty() == true)
        {
            printf("Missing command line arguments: -height:<height>\n");
            return -1;
        }

        uint32_t height;
        if (parse(height_option.c_str(), height) == false)
        {
            printf("Invalid command line argument: height\n");
            return -1;
        }

        std::string format_option = get_option("format");
        if (format_option.empty() == true)
        {
            printf("Missing command line arguments: -format:<pixelformat>\n");
            return -1;
        }
        core::imaging::pixel_format format;
        if (parse(format_option.c_str(), format) == false)
        {
            printf("Invalid command line argument: format\n");
            return -1;
        }

        std::string framerate_option = get_option("framerate");
        if (framerate_option.empty() == true)
        {
            printf("Missing command line arguments: -framerate:<numerator/denominator>\n");
            return -1;
        }

        core::video::framerate framerate;
        if (parse(framerate_option.c_str(), framerate) == false)
        {
            printf("Invalid command line argument: framerate\n");
            return -1;
        }

        factory = Video::VideoSourceFactory::Get<Video::Sources::RawDataVideoSourceFactory>(path_option.c_str(), width, height, format, framerate, algo);
	}
    else if (source == "test")
	{
        std::string width_option = get_option("width");
        if (width_option.empty() == true)
        {
            printf("Missing command line arguments: -width:<width>\n");
            return -1;
        }

        uint32_t width;
        if (parse(width_option.c_str(), width) == false)
        {
            printf("Invalid command line argument: width\n");
            return -1;
        }

        std::string height_option = get_option("height");
        if (height_option.empty() == true)
        {
            printf("Missing command line arguments: -height:<height>\n");
            return -1;
        }

        uint32_t height;
        if (parse(height_option.c_str(), height) == false)
        {
            printf("Invalid command line argument: height\n");
            return -1;
        }

        std::string format_option = get_option("format");
        if (format_option.empty() == true)
        {
            printf("Missing command line arguments: -format:<pixelformat>\n");
            return -1;
        }
        core::imaging::pixel_format format;
        if (parse(format_option.c_str(), format) == false)
        {
            printf("Invalid command line argument: format\n");
            return -1;
        }

        std::string framerate_option = get_option("framerate");
        if (framerate_option.empty() == true)
        {
            printf("Missing command line arguments: -framerate:<numerator/denominator>\n");
            return -1;
        }

        core::video::framerate framerate;
        if (parse(framerate_option.c_str(), framerate) == false)
        {
            printf("Invalid command line argument: framerate\n");
            return -1;
        }

        factory = Video::VideoSourceFactory::Get<Video::Sources::TestVideoSourceFactory>(width, height, format, framerate, algo);
	}
	else if (source == "auto")
	{
		uint32_t width = 0;
		std::string width_option = get_option("width");
		if (width_option.empty() == false)
		{
			if (parse(width_option.c_str(), width) == false)
			{
				printf("Invalid command line argument: width\n");
				return -1;
			}
		}

		uint32_t height = 0;
		std::string height_option = get_option("height");
		if (height_option.empty() == false)
		{
			if (parse(height_option.c_str(), height) == false)
			{
				printf("Invalid command line argument: height\n");
				return -1;
			}
		}

		core::imaging::pixel_format format = core::imaging::pixel_format::UNDEFINED_PIXEL_FORMAT;
		std::string format_option = get_option("format");
		if (format_option.empty() == false)
		{
			if (parse(format_option.c_str(), format) == false)
			{
				printf("Invalid command line argument: format\n");
				return -1;
			}
		}

		bool sync = true;
		std::string sync_str = get_option("sync");
		if (sync_str.empty() == false)
		{
			if (strcmp(sync_str.c_str(), "true") == 0)
			{
				sync = true;
			}
			else if (strcmp(sync_str.c_str(), "false") == 0)
			{
				sync = false;
			}
			else
			{
				printf("Invalid command line argument, expected: -sync:<true>/<false>\n");
				return -1;
			}
		}

		factory = Video::VideoSourceFactory::Get<Video::Sources::AutoVideoSourceFactory>(sync, width, height, format, algo);
	}
	else if (source == "gstreamer")
	{
		std::string pipeline_option = get_option("pipeline");
		if (pipeline_option.empty() == true)
		{
			printf("Missing command line arguments: -pipeline:<gstreamer pipeline>\n");
			return -1;
		}

		std::string sink_option = get_option("sink");
		if (sink_option.empty() == true)
		{
			printf("Missing command line arguments: -sink:<appsink name>\n");
			return -1;
		}

		factory = Video::VideoSourceFactory::Get<Video::Sources::GStreamerCustomVideoSourceFactory>(pipeline_option.c_str(), sink_option.c_str(), algo);
	}
	else
	{
		printf("Unknown source...\n");
		return -1;
	}    

	std::function<void(int)> error_func;
	Utils::SignalToken error_token = Utils::SignalTokenUndefined;
    std::mutex start_publisher_mutex;
    bool shutting_down = false;
	auto start_publisher = [&]()
	{
        printf("Creating publisher...\nStream name is: %s\nMax buffer size is: %lu bytes\nShared pool size is: %lu\n", name.c_str(),
               static_cast<unsigned long>(buffer_size),
               static_cast<unsigned long>(buffer_pool_size));

        std::lock_guard<std::mutex> locker(start_publisher_mutex);
        if (shutting_down == true)
            return;

		if (publisher == nullptr)
		{
			publisher = Video::Publishers::SharedMemoryVideoPublisher::Create(name.c_str(), factory, buffer_size, buffer_pool_size);
			if (publisher.Empty() == true)
				throw;

			error_token = publisher.OnError() += error_func;
			if (error_token == Utils::SignalTokenUndefined)
				throw;
		}
		else
		{
			publisher.Stop();
		}

		publisher.Start();
		printf("press Enter to stop publishing...\n");
	};

	std::atomic<bool> running(true);
	error_func = [&](int error_code)
	{
		for (int i = 0; i < 20; i++)
		{
			if (running == false)
				return;

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		start_publisher();
	};

	start_publisher();	
	getchar();
	running = false;

    std::unique_lock<std::mutex> locker(start_publisher_mutex);
    shutting_down = true;
    locker.unlock();

	publisher = nullptr;
    return 0;
}

