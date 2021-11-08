cmake_minimum_required(VERSION 2.8)
project(protocols)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

set(SOURCE_FILES
    fixed_length_protocol_impl.h
    fixed_length_protocol_impl.cpp
	variable_length_protocol_impl.cpp
	variable_length_protocol_impl.h
	delimiter_protocol_impl.cpp
	delimiter_protocol_impl.h
	udp_datagram_protocol.h
	udp_datagram_protocol.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(${PROJECT_NAME}
ports
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
