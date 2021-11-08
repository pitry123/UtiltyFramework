cmake_minimum_required(VERSION 2.8)
project(imaging)

if (USE_OPENCV)
	add_subdirectory(image_undistort)
    add_subdirectory(image_converter)
endif()
