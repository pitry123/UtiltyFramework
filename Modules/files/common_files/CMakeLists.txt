cmake_minimum_required(VERSION 2.8)
project(common_files)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

set(SOURCE_FILES
    bin_file_impl.h
    bin_file_impl.cpp
    ini_file_impl.h
    ini_file_impl.cpp
    log_file_impl.h
    log_file_impl.cpp
    pugixml/pugixml.hpp
    pugixml/pugixml.cpp
    pugixml/pugiconfig.hpp
    xml_file_impl.h
    xml_file_impl.cpp
	files_handler_impl.h
	files_handler_impl.cpp
	filessystem.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(${PROJECT_NAME}
${BOOST_LIBS}
${BOOST_OPTIONS_LIBS}
${BOOST_LOG_LIBS}
file_stream
${PTHREAD}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})