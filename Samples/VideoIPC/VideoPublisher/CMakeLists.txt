cmake_minimum_required(VERSION 2.8)
project(VideoPublisher)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

add_executable(${PROJECT_NAME}
        VideoPublisher.cpp
        )

target_link_libraries(${PROJECT_NAME} gstreamer_video shared_memory_video common_files image_undistort)

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
