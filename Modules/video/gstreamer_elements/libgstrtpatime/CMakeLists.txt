cmake_minimum_required(VERSION 2.8)
project(gstrtpatime)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

include_directories(${GSTREAMER_META_ROOT})

set(SOURCE_FILES	
	gstrtpatime.c
	gstrtpatimeparse.h
	gstrtpatimeparse.c
	gstrtpatimetimestamp.h
	gstrtpatimetimestamp.c
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(
${PROJECT_NAME}
${PTHREAD}
${GSTREAMER_LIBS}
gstatime)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
