cmake_minimum_required(VERSION 2.8)
project(DynamicApplication) 

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

add_executable(${PROJECT_NAME}
		Application.cpp
        )

include_directories(${APP_ROOT}/Include)
include_directories(${APP_ROOT}/ModernAPI)
include_directories(${APP_ROOT}/Common)

target_link_libraries(${PROJECT_NAME}
${CORE_LIBS}
ports
protocols
common_files
memory_database
dynamic_rules
GreenDynamicModule
RedDynamicModule)

include(${ROOT_DIR}/cmake_includes/build_events.cmake)
post_build_event(${PROJECT_NAME} ${ROOT_DIR} ${APP_ROOT})


install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
