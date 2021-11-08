cmake_minimum_required(VERSION 2.8)
project(ChannelDBSample)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

add_executable(${PROJECT_NAME}
		ChannelDBSample.cpp
        )

target_link_libraries(${PROJECT_NAME}
	${CORE_LIBS}
	ports
	protocols
    memory_database)

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
