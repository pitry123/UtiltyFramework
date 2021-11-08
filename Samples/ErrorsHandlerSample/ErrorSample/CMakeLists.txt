cmake_minimum_required(VERSION 2.8)
project(ErrorSample)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()
set(APP_ROOT ${CMAKE_CURRENT_LIST_DIR})
#include_directories(${APP_ROOT}/Include)
#include_directories(${APP_ROOT}/ModernAPI)
include_directories(${APP_ROOT}/Common)

add_executable(${PROJECT_NAME}
		ErrorSample.cpp
        )

target_link_libraries(${PROJECT_NAME}
	${CORE_LIBS}
	memory_stream
	memory_database
	binary_parser
	ports
    protocols
    common_files
	dynamic_rules
)

include(${ROOT_DIR}/cmake_includes/build_events.cmake)
post_build_event(${PROJECT_NAME} ${ROOT_DIR} ${APP_ROOT})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
