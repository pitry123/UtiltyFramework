cmake_minimum_required(VERSION 2.8)
project(ports)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

if(MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4834")
endif()

set(SOURCE_FILES
    serial_port_impl.h
    serial_port_impl.cpp
	udp_client_port_impl.h
    udp_client_port_impl.cpp
	tcp_client_port_impl.h
    tcp_client_port_impl.cpp
	tcp_server_port_impl.h
	tcp_server_port_impl.cpp
	can_port_impl.h
	can_port_impl.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(${PROJECT_NAME} ${PTHREAD} ${BOOST_LIBS}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
