cmake_minimum_required(VERSION 2.8)
project(SimpleParser)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

add_executable(${PROJECT_NAME}
		SimpleParser.cpp
        )

target_link_libraries(${PROJECT_NAME}
	${CORE_LIBS}
	memory_stream
	binary_parser
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})

add_test(NAME ${PROJECT_NAME}_test CONFIGURATIONS Release]
         WORKING_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/bin"
		 COMMAND all)
