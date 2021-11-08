cmake_minimum_required(VERSION 2.8.9)
project(plugins)

add_definitions(${COMPILE_DEFINITIONS})
set(PLUGINS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

#----------- Build shared or static library ------------
if(BUILD_STATIC)
	MESSAGE("Infrastructure plugins: building STATIC libraries")
	set(SDK_LIB_TYPE "STATIC")
else()
	MESSAGE("Infrastructure plugins: building SHARED libraries")
	set(SDK_LIB_TYPE "SHARED")
endif()

# Define a MACRO for listing all subdirectories of a given folder
MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()

# Get the current folder
GET_FILENAME_COMPONENT(CURRENT_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

# Get all subdirectories of current folder (i.e all plugins directories)
SUBDIRLIST(SUBDIRS ${CURRENT_DIR})

# For each sub folder, add it to the project tree
FOREACH(subdir ${SUBDIRS})
    MESSAGE("Adding plugin directory to project: ${subdir}")
    ADD_SUBDIRECTORY(${subdir})
ENDFOREACH()