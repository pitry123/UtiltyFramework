cmake_minimum_required(VERSION 2.8)
project(http)

if(USE_CURL)
	add_subdirectory(curl_http_client)
endif()
