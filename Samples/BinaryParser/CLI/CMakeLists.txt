cmake_minimum_required(VERSION 2.8)
project(BinaryParserCLI)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

add_definitions(-DEXT_MODE)

if(${IS_CURRENT_SYSTEM_LINUX_OS})
    set(CONTROL_DIR ${ROOT_DIR}/Samples/Control/Matlab/Linux)
else()
    set(CONTROL_DIR ${ROOT_DIR}/Samples/Control/Matlab/Windows)
endif()




add_executable(${PROJECT_NAME}
		
                mainapp.cpp
                )

#here we stete the project from the inclue dir

target_link_libraries(${PROJECT_NAME}
			${PTHREAD}
                       ${CORE_LIBS}
			common_files
			ports
			protocols
			dynamic_rules
			memory_database
			binary_parser
			boost_logger)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_SYSTEM_NAME})
