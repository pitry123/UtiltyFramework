cmake_minimum_required(VERSION 2.8)
project(image_converter)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

set(SOURCE_FILES
    image_converter.h
    image_converter.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(
	${PROJECT_NAME}
        ${PTHREAD}
	${OPENCV_LIBS}
	boost_logger
	ezframework
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
