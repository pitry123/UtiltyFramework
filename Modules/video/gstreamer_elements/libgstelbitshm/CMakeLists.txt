cmake_minimum_required(VERSION 2.8)
project(gstelbitshm)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	if (MSVC)
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:\"${CMAKE_CURRENT_LIST_DIR}/libgstelbitshm.def\"")
	elseif(CMAKE_COMPILER_IS_GNUCXX)
		add_definitions(-DMINGW_COMPILER)
	endif()
endif()

include_directories(${SHM_COMMON_DIR})
include_directories(${GSTREAMER_META_ROOT})

set(SOURCE_FILES
	gstshmmeta.h
	gstshmmeta.cpp
	gstshmbufferpool.h
	gstshmbufferpool.cpp	
	gstshmsink.c
	gstshmsink.h
	gstelbitshm.c
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(
${PROJECT_NAME}
${PTHREAD}
${GSTREAMER_LIBS}
${SHARED_OS_LIB})

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
