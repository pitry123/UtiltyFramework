cmake_minimum_required(VERSION 2.8)
project(VideoEncoder)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

add_executable(${PROJECT_NAME}
		VideoEncoder.cpp
        )

target_link_libraries(${PROJECT_NAME}
shared_memory_video
gstreamer_video)

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
