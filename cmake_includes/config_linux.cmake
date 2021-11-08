set(COMPILE_DEFINITIONS -Wall -Wno-write-strings -Wno-comment -Wno-unknown-pragmas -Wno-unused-function -Wno-misleading-indentation -Wno-strict-aliasing -Werror)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS} -Wno-overloaded-virtual")
endif()

# GStreamer
if(USE_GSTREAMER)
	set(GSTREAMER_INCLUDE_PATH /usr/local/include/gstreamer-1.0/ /usr/lib/x86_64-linux-gnu/gstreamer-1.0/include/ /usr/lib/x86_64-linux-gnu/glib-2.0/include/ /usr/lib64/glib-2.0/include/ /usr/include/gstreamer-1.0/ /usr/include/glib-2.0/)

	if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "armv7")	
		set(GSTREAMER_INCLUDE_PATH ${GSTREAMER_INCLUDE_PATH} /usr/lib/arm-linux-gnueabihf/glib-2.0/include/)
	elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
		set(GSTREAMER_INCLUDE_PATH ${GSTREAMER_INCLUDE_PATH} /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include/ /usr/lib/aarch64-linux-gnu/glib-2.0/include/)
	endif()

	set (GSTREAMER_LIBS
		glib-2.0
		gstbase-1.0
		gstreamer-1.0
		gsttag-1.0
		gstpbutils-1.0
		gobject-2.0
		gstapp-1.0
		gstrtspserver-1.0		
		gstrtp-1.0		
		gstvideo-1.0)
endif() #USE_GSTREAMER

if (USE_CURL)
	set(CURL_LIBS curl)
endif()

# LZ4
if (USE_LZ4)
	set (LZ4_LIBS lz4)
endif()

# Boost (Shared Memory prerequisite)
set(SHARED_OS_LIB rt)
set(BOOST_LIBS boost_system)

add_definitions(-DBOOST_LOG_DYN_LINK=1)
set(BOOST_LOG_LIBS boost_log boost_log_setup boost_thread boost_serialization)
set(BOOST_OPTIONS_LIBS boost_program_options boost_filesystem)

# OpenGL
set(GLFW_LIBS GL glfw)

# Required by GCC when using std::thread
set(PTHREAD pthread)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 ")
endif()

if(USE_OPENCV)
        # Check for OpenCV4
        if(EXISTS /usr/include/opencv4/)
            include_directories(SYSTEM /usr/include/opencv4/)
            set(OPENCV_LIBS opencv_core opencv_imgproc opencv_calib3d)
        else()
            set(OPENCV_LIBS opencv_core opencv_imgproc)
        endif()
endif() #USE_OPENCV

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wformat -Wformat-security")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wconversion")
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64")
    if (ENABLE_AVX2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
        MESSAGE("AVX2 instructions are enabled")
    endif()
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -z noexecstack")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -z relro -z now")

include_directories(${GSTREAMER_INCLUDE_PATH})
