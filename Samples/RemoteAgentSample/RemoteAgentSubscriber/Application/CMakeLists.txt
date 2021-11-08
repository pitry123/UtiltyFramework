cmake_minimum_required(VERSION 2.8)
project(RemoteAgentSubscriber)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

add_executable(${PROJECT_NAME}
		RemoteAgentSubscriber.cpp
        )

include(${ROOT_DIR}/cmake_includes/build_events.cmake)
post_build_event(${PROJECT_NAME} ${ROOT_DIR} ${APP_ROOT})

target_link_libraries(${PROJECT_NAME}
${CORE_LIBS}
ports
protocols
common_files
binary_parser
memory_database)

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
