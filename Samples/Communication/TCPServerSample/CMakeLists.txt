cmake_minimum_required(VERSION 2.8)
project(TCPServerSamples)

set(APP_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(${ROOT_DIR}/cmake_includes/version.cmake)
conan_cmake_version(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(Application)