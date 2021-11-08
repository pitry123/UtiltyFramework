cmake_minimum_required(VERSION 2.8)
project(DirectInputSample)

set(APP_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(${ROOT_DIR}/cmake_includes/version.cmake)
conan_cmake_version(${CMAKE_CURRENT_LIST_DIR})

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

add_executable(${PROJECT_NAME}
		DirectInputSample.cpp
        )

include_directories(${APP_ROOT}/Include)
include_directories(${APP_ROOT}/ModernAPI)
include_directories(${APP_ROOT}/Common)

target_link_libraries(${PROJECT_NAME}
${CORE_LIBS}
direct_input_device
memory_database
ports
protocols)

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
