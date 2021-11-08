cmake_minimum_required(VERSION 2.8)
project(IniFileSample)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

add_executable(${PROJECT_NAME}
		IniFileSample.cpp
        )

target_link_libraries(${PROJECT_NAME}
common_files)

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})