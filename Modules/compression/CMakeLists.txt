cmake_minimum_required(VERSION 2.8)
project(compression)

# LZ4
if (USE_LZ4)
	add_subdirectory(lz4)
endif()

