cmake_minimum_required(VERSION 2.8)
project(MonitorSample)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

add_executable(${PROJECT_NAME}
		MonitorSample.cpp
		Tables.h
		Databases.h
        )

target_link_libraries(${PROJECT_NAME}
	${CORE_LIBS}
    memory_database
	ports
	protocols	
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
