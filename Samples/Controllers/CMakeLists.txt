cmake_minimum_required(VERSION 2.8)
project(Controllers)

if((${CMAKE_SYSTEM_NAME} STREQUAL "Windows") AND (MSVC))
	add_subdirectory(JoystickSample)
	add_subdirectory(DirectInputSample)
endif()
