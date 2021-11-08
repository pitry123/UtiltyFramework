set(COMPILE_DEFINITIONS -Wall -Wno-weak-vtables -Wno-unused-private-field -Wno-unused-lambda-capture -Wno-unused-function -Wno-overloaded-virtual -Werror)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MACOSX_RPATH 1)

set(USR_LOCAL_INCLUDE_PATH /usr/local/include/)
set(USR_LOCAL_LIB_PATH /usr/local/lib/)

# GStreamer
if (USE_GSTREAMER)
	set(GSTREAMER_INCLUDE_PATH /Library/Frameworks/GStreamer.framework/Versions/1.0/Headers/ /Library/Frameworks/GStreamer.framework/Versions/1.0/include/gstreamer-1.0/)
	set(GSTREAMER_LIB_PATH /Library/Frameworks/GStreamer.framework/Versions/1.0/lib/)
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

# LZ4
if (USE_CURL)
        set (CURL_LIBS curl)
endif()

# LZ4
if (USE_LZ4)
        set (LZ4_LIBS lz4)
endif()

# Boost (Shared Memory prerequisite)
set(BOOST_LIBS boost_system-mt)

add_definitions(-DBOOST_LOG_DYN_LINK=1)
set(BOOST_LOG_LIBS boost_log-mt boost_log_setup-mt boost_thread-mt boost_serialization-mt)
set(BOOST_OPTIONS_LIBS boost_program_options-mt boost_filesystem-mt)

# OpenGL
set(GLFW_LIBS "-framework OpenGL" glfw)

# Required by GCC when using std::thread
set(PTHREAD pthread)

if (USE_OPENCV)
    # Check for OpenCV4
    if(EXISTS /usr/local/include/opencv4/)
        include_directories(SYSTEM /usr/local/include/opencv4/)
        set(OPENCV_LIBS opencv_core opencv_imgproc opencv_calib3d)
    else()
	set(OPENCV_LIBS opencv_core opencv_imgproc)
    endif()
endif() #USE_OPENCV

include_directories(${USR_LOCAL_INCLUDE_PATH} ${GSTREAMER_INCLUDE_PATH})
link_directories(${USR_LOCAL_LIB_PATH} ${GSTREAMER_LIB_PATH})
