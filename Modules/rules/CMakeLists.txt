cmake_minimum_required(VERSION 2.8)
project(dynamic_rules)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

#include_directories(${APP_ROOT}/Include)
#include_directories(${APP_ROOT}/Common)

set(SOURCE_FILES
    rules_dispatcher_impl.h
    rules_dispatcher_impl.cpp
	rules_defs.h
	rules_parser.h
	rules_parser.cpp
	rule.h
	rule.cpp
	rules_concrete_generic.cpp
	rules_concrete_generic.h
	expression.h
	expression.cpp
	node.h
	node.cpp
	operands.h
	operands.cpp
	operators.h
	operators.cpp
)

include_directories(${PROJECT_NAME})
set(SDK_LIB_TYPE "SHARED")
add_library(${PROJECT_NAME} ${SDK_LIB_TYPE} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "${LIBVERSION}" SOVERSION "${LIBSOVERSION}")

target_link_libraries(${PROJECT_NAME}    
	                  ${CORE_LIBS}
                      memory_database
					  common_files
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_DIR})
