cmake_minimum_required(VERSION 2.8)
project(gstreamer_video)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

include_directories(${GSTREAMER_META_ROOT})

set(SOURCE_FILES
	common/gstreamer_base_video_controller.hpp
	common/gstreamer_base_video_source.hpp
	common/gstreamer_base_rtsp_server.hpp
	common/gstreamer_base_appsrc_pipeline.hpp
    gstreamer_rtsp_source.h
    gstreamer_rtsp_source.cpp
	gstreamer_file_source.h
	gstreamer_file_source.cpp
	gstreamer_raw_data_source.h
	gstreamer_raw_data_source.cpp
	gstreamer_test_source.h
	gstreamer_test_source.cpp
	gstreamer_auto_source.h
	gstreamer_auto_source.cpp
	gstreamer_custom_source.h
	gstreamer_custom_source.cpp
	gstreamer_h264_encoder.h
	gstreamer_h264_encoder.cpp
	gstreamer_rtsp_launch.h
	gstreamer_rtsp_launch.cpp
	gstreamer_rtsp_server.h
	gstreamer_rtsp_server.cpp
	gstreamer_h264_player.h
	gstreamer_h264_player.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(
${PROJECT_NAME}
${PTHREAD}
${GSTREAMER_LIBS}
gstatime)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
