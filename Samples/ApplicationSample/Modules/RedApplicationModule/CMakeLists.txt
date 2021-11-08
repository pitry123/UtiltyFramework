cmake_minimum_required(VERSION 2.8)
project(RedApplicationModule)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

include_directories(${APP_ROOT}/Include)
include_directories(${APP_ROOT}/Common)

set(SOURCE_FILES
    RedDispatcher.h
    RedDispatcher.cpp
)

add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(${PROJECT_NAME}    
	${CORE_LIBS}
    ${PTHREAD}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
