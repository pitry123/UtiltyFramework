cmake_minimum_required(VERSION 2.8)
project(video)

set(SHM_COMMON_DIR ${CMAKE_CURRENT_LIST_DIR}/shm_common/)
set(GSTREAMER_META_ROOT ${CMAKE_CURRENT_LIST_DIR}/gstreamer_meta/)


add_subdirectory(video_messages)
add_subdirectory(shared_memory_video)

if(USE_GSTREAMER)
	add_subdirectory(gstreamer_meta)
	add_subdirectory(gstreamer_video)	
	add_subdirectory(gstreamer_elements)	
endif()