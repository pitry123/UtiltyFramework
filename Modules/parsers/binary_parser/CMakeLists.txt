cmake_minimum_required(VERSION 2.8)
project(binary_parser)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

if(MSVC)
	if ("${CMAKE_CXX_FLAGS}" MATCHES "/W[1-4]")
		STRING(REGEX REPLACE "/W[1-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endif()

	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

set(SOURCE_FILES
	binary_parser_impl.h
	binary_parser_impl.cpp
	binary_metadata_store_impl.h
	binary_metadata_store_impl.cpp
	binary_metadata_impl.cpp
	enum_data_imp.h
	enum_data_imp.cpp
)

add_definitions(${COMPILE_DEFINITIONS})

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(
    ${PROJECT_NAME}
    ${PTHREAD})

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
