cmake_minimum_required(VERSION 2.8)
project(direct_input_device)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

include_directories(${CONTROLLERS_COMMON_DIR})

set(SOURCE_FILES
    direct_input_device.h
    direct_input_device.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(
	${PROJECT_NAME}
	${PTHREAD}
	dxguid
	dinput8)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
