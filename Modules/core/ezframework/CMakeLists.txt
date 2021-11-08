cmake_minimum_required(VERSION 2.8)
project(ezframework)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

set(SOURCE_FILES
    ezframework.cpp
	application.cpp
	guid.cpp)

# This module is always built SHARED as it manages a unique resources (Build version and Console printing thread)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(${PROJECT_NAME}
	${BOOST_OPTIONS_LIBS}
	boost_logger)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
