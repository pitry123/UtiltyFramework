cmake_minimum_required(VERSION 2.8)
project(controllers)

if((${CMAKE_SYSTEM_NAME} STREQUAL "Windows") AND (MSVC))
	set(CONTROLLERS_COMMON_DIR ${CMAKE_CURRENT_LIST_DIR}/common/)
	add_subdirectory(logitech_f310)
	add_subdirectory(xinput_device)	
	add_subdirectory(direct_input_device)
endif()
