cmake_minimum_required(VERSION 2.8.9)
project(Modules)

#-------------------- Lets make in-source-builds forbidden --------
MACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD MSG)
    STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
            "${CMAKE_BINARY_DIR}" insource)
    GET_FILENAME_COMPONENT(PARENTDIR ${CMAKE_SOURCE_DIR} PATH)
    STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
            "${PARENTDIR}" insourcesubdir)
    IF(insource OR insourcesubdir)
        file(REMOVE_RECURSE "${CMAKE_SOURCE_DIR}/CMakeCache.txt" "${CMAKE_SOURCE_DIR}/CMakeFiles")
        MESSAGE("============================================================")
        MESSAGE("======    ")
        MESSAGE("======    ${MSG}")
        MESSAGE("======    ")
        MESSAGE("============================================================")
        MESSAGE(FATAL_ERROR "Error occured")
    ENDIF(insource OR insourcesubdir)
ENDMACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)

MACRO_ENSURE_OUT_OF_SOURCE_BUILD(
        "${CMAKE_PROJECT_NAME} requires an out of source build."
)

add_definitions(${COMPILE_DEFINITIONS})
set(MODULES_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_COMPILER_IS_GNUCXX)
    #--------------------- suppprt for c++11 ----------------
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    #--------------- missing c++11 functions ----------------
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9.2)
        add_definitions(-DGCC_492)
    endif()

    #---------------------- Debug Builds --------------------
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		#-------------- No Optimizations -----------------
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 ")
    endif()

	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		#-------------- Add security options --------------------
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=1")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-strong")
	endif()

    MESSAGE("GNU compiler detected, adding flag: -Wl,--no-undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-undefined")    
endif()

set(INCLUDE_DIR ${ROOT_DIR}/Include/)
set(MODERN_API_DIR ${ROOT_DIR}/ModernAPI/)
set(COMMON_DIR ${ROOT_DIR}/Common/)

SET(INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/include/ezFramework_2.0/Infrastructure)
install(DIRECTORY ${INCLUDE_DIR} DESTINATION ${INSTALL_PATH}/Include)
install(DIRECTORY ${MODERN_API_DIR} DESTINATION ${INSTALL_PATH}/ModernAPI)
install(DIRECTORY ${COMMON_DIR} DESTINATION ${INSTALL_PATH}/Common)

#----------- Build shared or static library ------------
if(BUILD_STATIC)
	MESSAGE("Infrastructure modules: building STATIC libraries")
	set(SDK_LIB_TYPE "STATIC")
else()
	MESSAGE("Infrastructure modules: building SHARED libraries")
	set(SDK_LIB_TYPE "SHARED")
endif()

add_subdirectory(core)
add_subdirectory(communication)
add_subdirectory(database)
add_subdirectory(dispatchers)
add_subdirectory(files)
add_subdirectory(logging)
add_subdirectory(streams)
add_subdirectory(imaging)
add_subdirectory(video)
add_subdirectory(accessories)
add_subdirectory(parsers)
add_subdirectory(rules)
add_subdirectory(compression)
add_subdirectory(http)
add_subdirectory(websocket)
