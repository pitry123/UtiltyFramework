cmake_minimum_required(VERSION 2.8.9)
project(infrastructure)

if(MSVC)	
	if(NOT CMAKE_BUILD_TYPE)
		MESSAGE("MSVC: CMAKE_BUILD_TYPE is not configured, Trying to resolve...")
		string(TOUPPER ${CMAKE_CONFIGURATION_TYPES} MSVC_BUILD_TYPE)
		if (MSVC_BUILD_TYPE STREQUAL "DEBUG")
			set (CMAKE_BUILD_TYPE "Debug")
		elseif(MSVC_BUILD_TYPE STREQUAL "RELEASE")
			set (CMAKE_BUILD_TYPE "Release")
		elseif(MSVC_BUILD_TYPE STREQUAL "MINSIZEREL")
			set (CMAKE_BUILD_TYPE "Release")
		elseif(MSVC_BUILD_TYPE STREQUAL "RELWITHDEBINFO")
			set (CMAKE_BUILD_TYPE "Release")
		endif()
	endif()
endif()

if(NOT DEFINED BUILD_STATIC)
	set (BUILD_STATIC OFF)
endif(NOT DEFINED BUILD_STATIC)

if(NOT CMAKE_BUILD_TYPE)	
        MESSAGE(WARNING " Build Type is empty - Assuming Release")
        set(CMAKE_BUILD_TYPE "Release")
endif()

if(NOT DEFINED USE_CONAN)
	set (USE_CONAN OFF)
endif(NOT DEFINED USE_CONAN)

if(NOT DEFINED USE_GSTREAMER)
	set (USE_GSTREAMER ON)
	MESSAGE("GStreamer included ")
endif(NOT DEFINED USE_GSTREAMER)

if(NOT DEFINED USE_LZ4)
	set (USE_LZ4 ON)
	MESSAGE("LZ4 included ")
endif(NOT DEFINED USE_LZ4)

if(NOT DEFINED USE_OPENCV)
	set (USE_OPENCV ON)
	MESSAGE("OpenCV included ")
endif(NOT DEFINED USE_OPENCV)

if(NOT DEFINED USE_CURL)
	set (USE_CURL ON)
	MESSAGE("Curl included ")
endif(NOT DEFINED USE_CURL)

if(NOT DEFINED ENABLE_AVX2)
	set (ENABLE_AVX2 OFF)	
endif(NOT DEFINED ENABLE_AVX2)

if(NOT DEFINED LIB_DIR)
	set (LIB_DIR lib)	
endif(NOT DEFINED LIB_DIR)

if(NOT DEFINED BIN_DIR)
	set (BIN_DIR bin)	
endif(NOT DEFINED BIN_DIR)

if(USE_CONAN)
	include(cmake_includes/conan.cmake)
	conan_cmake_settings(settings)
	
	if( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		if (CMAKE_COMPILER_IS_GNUCXX)			
			if(CMAKE_SIZEOF_VOID_P EQUAL 4)
				# 32 bits
				set (settings "${settings};-s;arch=x86;-s;arch_build=x86")
			else()
				# Assumed 64 bits
				set (settings "${settings};-s;arch=x86_64;-s;arch_build=x86_64")			
			endif()
		endif()
		conan_cmake_install(CONANFILE conanfile_windows.txt SETTINGS ${settings})
	else()
		conan_cmake_install(CONANFILE conanfile_linux.txt SETTINGS ${settings})
	endif()

	include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
endif()

include(cmake_includes/check_os.cmake)

## Set preprocessors defines according to OS, to be used within header files
if( ${IS_CURRENT_SYSTEM_LINUX_OS})
    add_definitions(-DLINUX)
    MESSAGE("Added preprocessor definition: LINUX")
endif()

file (STRINGS "${CMAKE_CURRENT_LIST_DIR}/version.txt" VERSION)

string(REPLACE "." ";" VERSION_LIST ${VERSION})
list(GET VERSION_LIST 0 EZ_FRAMEWORK_VERSION_MAJOR)
list(GET VERSION_LIST 1 EZ_FRAMEWORK_VERSION_MINOR)
list(GET VERSION_LIST 2 EZ_FRAMEWORK_VERSION_PATCH)

set(LIBVERSION ${EZ_FRAMEWORK_VERSION_MAJOR}.${EZ_FRAMEWORK_VERSION_MINOR}.${EZ_FRAMEWORK_VERSION_PATCH})

add_definitions(-DEZ_FRAMEWORK_VERSION_MAJOR=${EZ_FRAMEWORK_VERSION_MAJOR})
add_definitions(-DEZ_FRAMEWORK_VERSION_MINOR=${EZ_FRAMEWORK_VERSION_MINOR})
add_definitions(-DEZ_FRAMEWORK_VERSION_PATCH=${EZ_FRAMEWORK_VERSION_PATCH})

set(EZ_FRAMEWORK_VERSION_BUILD_UNKNOWN 0xffffffff)
add_definitions(-DEZ_FRAMEWORK_VERSION_BUILD_UNKNOWN=${EZ_FRAMEWORK_VERSION_BUILD_UNKNOWN})

if(NOT DEFINED BUILD_NUMBER)	
	message("Infrastructure version is: ${VERSION}")
	add_definitions(-DEZ_FRAMEWORK_VERSION_BUILD=${EZ_FRAMEWORK_VERSION_BUILD_UNKNOWN})
else()
	message("Infrastructure version is: ${VERSION} (build: ${BUILD_NUMBER})")
	add_definitions(-DEZ_FRAMEWORK_VERSION_BUILD=${BUILD_NUMBER})
endif(NOT DEFINED BUILD_NUMBER)

set(LIBSOVERSION ${EZ_FRAMEWORK_VERSION_MAJOR})

set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
include_directories(${ROOT_DIR}/Include)
include_directories(${ROOT_DIR}/ModernAPI)
include_directories(${ROOT_DIR}/Common)

set(CORE_LIBS ezframework)

add_subdirectory(Modules)
add_subdirectory(Samples)
add_subdirectory(Plugins)
