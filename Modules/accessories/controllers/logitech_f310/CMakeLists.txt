cmake_minimum_required(VERSION 2.8)
project(logitech_f310)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

include_directories(${CONTROLLERS_COMMON_DIR})

set(SOURCE_FILES
    logitech_f310.h
    logitech_f310.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(
	${PROJECT_NAME}
	${PTHREAD}
	Winmm)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
