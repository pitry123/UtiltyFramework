cmake_minimum_required(VERSION 2.8)
project(boost_logger)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

set(SOURCE_FILES
    logger.h
    logger.cpp
)

# Boost logger is always built SHARED as it manages a unique resource to the log file (the actual logger instance)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(${PROJECT_NAME}
    ${BOOST_LIBS}
    ${BOOST_LOG_LIBS}
    ${PTHREAD}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
