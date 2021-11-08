cmake_minimum_required(VERSION 2.8)
project(curl_http_client)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

set(SOURCE_FILES
    curl_http_client.h
    curl_http_client.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(${PROJECT_NAME}
${CURL_LIBS}
${PTHREAD}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
