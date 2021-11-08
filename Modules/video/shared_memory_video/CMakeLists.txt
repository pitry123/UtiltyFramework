cmake_minimum_required(VERSION 2.8)
project(shared_memory_video)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

include_directories(${SHM_COMMON_DIR})

set(SOURCE_FILES
    shared_memory_video_publisher.h
    shared_memory_video_publisher.cpp
    shared_memory_video_subscriber.h
    shared_memory_video_subscriber.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

if (NOT USE_CONAN)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		add_custom_target(
		copyBoostPatch
		COMMAND xcopy /E /R /Y .\\windows_boost_patch\\*.* ${BOOST_ROOT}\\
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		)
		add_dependencies(${PROJECT_NAME} copyBoostPatch)
	endif()
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(
    ${PROJECT_NAME}
    ${PTHREAD}
    ${SHARED_OS_LIB})

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
