cmake_minimum_required(VERSION 2.8)
project(websocket_client)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

if(MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4834")
endif()

set(SOURCE_FILES
    websocket_client_impl.h
    websocket_client_impl.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(${PROJECT_NAME}
${PTHREAD}
${BOOST_LIBS}
)

#include_directories($ENV{DEVELOPMENT_ROOT}/ThirdParty/curl/include)
#link_directories($ENV{DEVELOPMENT_ROOT}/ThirdParty/curl/build/lib/x64)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
