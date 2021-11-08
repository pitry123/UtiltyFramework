if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
	if (NOT DEFINED CMAKE_PLATFORM)
            set(CMAKE_PLATFORM "x64")
	endif()
	MESSAGE("Detected Windows x64 Build configuration")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
	if (NOT DEFINED CMAKE_PLATFORM)
		set(CMAKE_PLATFORM "Win32")
	endif()
	MESSAGE("Detected Windows Win32 Build configuration")	
else()
	MESSAGE(FATAL "Script was not able to detect the compiler type. Please set CMAKE_PLATFORM to either x64 or Win32")
endif()

# Boost
if(NOT DEFINED BOOST_ROOT)
# Note that although slashes (/) are my prefered way of constructing a path
# I'm using backslashes here (\) because of a later pre-build script
# which copies a boost patch for windows (handles an issue with shared memory implementation)
set(BOOST_ROOT $ENV{DEVELOPMENT_ROOT}\\ThirdParty\\boost_1_64_0)
endif(NOT DEFINED BOOST_ROOT)
set(BOOST_INCLUDE_PATH ${BOOST_ROOT}/)
if (MSVC)
	if(CMAKE_PLATFORM STREQUAL "x64")
		set(BOOST_LIB_PATH ${BOOST_ROOT}/stage/x64/lib)
	elseif(CMAKE_PLATFORM STREQUAL "Win32")
		set(BOOST_LIB_PATH ${BOOST_ROOT}/stage/win32/lib)
	endif()	
elseif(CMAKE_COMPILER_IS_GNUCXX)
	if(CMAKE_PLATFORM STREQUAL "x64")
		set(BOOST_LIB_PATH ${BOOST_ROOT}/stage/mingw64/lib)
	elseif(CMAKE_PLATFORM STREQUAL "Win32")
		set(BOOST_LIB_PATH ${BOOST_ROOT}/stage/mingw32/lib)
	endif()
endif()

if (MSVC)
	add_definitions(-DBOOST_ALL_NO_LIB)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(BOOST_LIBS libboost_system-vc141-mt-gd-1_64)
		set(BOOST_LOG_LIBS libboost_filesystem-vc141-mt-gd-1_64 libboost_log-vc141-mt-gd-1_64 libboost_log_setup-vc141-mt-gd-1_64 libboost_thread-vc141-mt-gd-1_64 libboost_serialization-vc141-mt-gd-1_64)
		set(BOOST_OPTIONS_LIBS libboost_program_options-vc141-mt-gd-1_64)
	else()
		set(BOOST_LIBS libboost_system-vc141-mt-1_64)
		set(BOOST_LOG_LIBS libboost_filesystem-vc141-mt-1_64 libboost_log-vc141-mt-1_64 libboost_log_setup-vc141-mt-1_64 libboost_thread-vc141-mt-1_64 libboost_serialization-vc141-mt-1_64)
		set(BOOST_OPTIONS_LIBS libboost_program_options-vc141-mt-1_64)
	endif()
elseif(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-DBOOST_LOG_DYN_LINK=1)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(BOOST_LIBS boost_system-mgw81-mt-d-1_64 boost_filesystem-mgw81-mt-d-1_64)
		set(BOOST_LOG_LIBS boost_log-mgw81-mt-d-1_64 boost_log_setup-mgw81-mt-d-1_64 boost_thread-mgw81-mt-d-1_64 boost_serialization-mgw81-mt-d-1_64)
		set(BOOST_OPTIONS_LIBS boost_program_options-mgw81-mt-d-1_64)
	else()
		set(BOOST_LIBS boost_system-mgw81-mt-1_64 boost_filesystem-mgw81-mt-1_64)
		set(BOOST_LOG_LIBS boost_log-mgw81-mt-1_64 boost_log_setup-mgw81-mt-1_64 boost_thread-mgw81-mt-1_64 boost_serialization-mgw81-mt-1_64)
		set(BOOST_OPTIONS_LIBS boost_program_options-mgw81-mt-1_64)
	endif()	
endif()

# GStreamer
if(USE_GSTREAMER)
	if(NOT DEFINED GSTREAMER_DIR)
		if(CMAKE_PLATFORM STREQUAL "x64")
			set(GSTREAMER_DIR $ENV{DEVELOPMENT_ROOT}/ThirdParty/GStreamer/1.0/mingw_x86_64)
		elseif(CMAKE_PLATFORM STREQUAL "Win32")
			set(GSTREAMER_DIR $ENV{DEVELOPMENT_ROOT}/ThirdParty/GStreamer/1.0/mingw_x86)
		endif()	
	endif(NOT DEFINED GSTREAMER_DIR)
	set(GSTREAMER_INCLUDE_PATH ${GSTREAMER_DIR}/include/gstreamer-1.0/ ${GSTREAMER_DIR}/include/glib-2.0/ ${GSTREAMER_DIR}/lib/glib-2.0/include)
	set(GSTREAMER_LIB_PATH ${GSTREAMER_DIR}/lib/)
	
	if (CMAKE_COMPILER_IS_GNUCXX)
		set (GSTREAMER_LIBS
			${GSTREAMER_LIB_PATH}/libglib-2.0.dll.a
			${GSTREAMER_LIB_PATH}/libgstbase-1.0.dll.a 
			${GSTREAMER_LIB_PATH}/libgstreamer-1.0.dll.a 
			${GSTREAMER_LIB_PATH}/libgsttag-1.0.dll.a
			${GSTREAMER_LIB_PATH}/libgstpbutils-1.0.dll.a
			${GSTREAMER_LIB_PATH}/libgobject-2.0.dll.a
			${GSTREAMER_LIB_PATH}/libgstapp-1.0.dll.a
			${GSTREAMER_LIB_PATH}/libgstrtspserver-1.0.dll.a
			${GSTREAMER_LIB_PATH}/libgstrtp-1.0.dll.a
			${GSTREAMER_LIB_PATH}/libgstvideo-1.0.dll.a)
	else()
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
	endif()
endif() #USE_GSTREAMER

if(NOT DEFINED GLFW_DIR)
	if(CMAKE_PLATFORM STREQUAL "x64")
		set(GLFW_DIR $ENV{DEVELOPMENT_ROOT}/ThirdParty/GLFW/x64)
	elseif(CMAKE_PLATFORM STREQUAL "Win32")
		set(GLFW_DIR $ENV{DEVELOPMENT_ROOT}/ThirdParty/GLFW/win32)
	endif()
endif(NOT DEFINED GLFW_DIR)

set(GLFW_INCLUDE_PATH ${GLFW_DIR}/include/)
set(OPENGL_LIBS OpenGL32)

if (MSVC)
	set(GLFW_LIB_PATH ${GLFW_DIR}/lib-vc2017/)
elseif(CMAKE_COMPILER_IS_GNUCXX)
	set(GLFW_LIB_PATH ${GLFW_DIR}/lib-mingw-w64/)	
endif()

set(GLFW_LIBS glfw3)

if(USE_OPENCV)
	if(NOT DEFINED OPENCV_DIR)
	set(OPENCV_DIR $ENV{DEVELOPMENT_ROOT}/ThirdParty/OpenCV)
	endif(NOT DEFINED OPENCV_DIR)

	set(OPENCV_INCLUDE_PATH ${OPENCV_DIR}/build/include/)

	if (MSVC)
		if(CMAKE_PLATFORM STREQUAL "x64")
			set(OPENCV_LIB_PATH ${OPENCV_DIR}/build/x64/vc15/lib)	
		elseif(CMAKE_PLATFORM STREQUAL "Win32")
			set(OPENCV_LIB_PATH ${OPENCV_DIR}/build/x86/vc15/lib)
		endif()
	elseif(CMAKE_COMPILER_IS_GNUCXX)
		if(CMAKE_PLATFORM STREQUAL "x64")
			set(OPENCV_LIB_PATH ${OPENCV_DIR}/build/x64/mingw/lib)	
		elseif(CMAKE_PLATFORM STREQUAL "Win32")
			set(OPENCV_LIB_PATH ${OPENCV_DIR}/build/x86/mingw/lib)
		endif()
	endif()

	set (OPENCV_VERSION "345")
	if (MSVC)
		set(OPENCV_LIBS opencv_world${OPENCV_VERSION})
	elseif(CMAKE_COMPILER_IS_GNUCXX)
		set(OPENCV_LIBS opencv_core${OPENCV_VERSION}.dll opencv_imgproc${OPENCV_VERSION}.dll)
	endif()
endif() #USE_OPENCV

if (USE_LZ4)
	if(NOT DEFINED LZ4_DIR)
		set(LZ4_DIR $ENV{DEVELOPMENT_ROOT}/ThirdParty/lz4)
	endif(NOT DEFINED LZ4_DIR)

	set(LZ4_INCLUDE_PATH ${LZ4_DIR}/include/)

	if(CMAKE_PLATFORM STREQUAL "x64")
		set(LZ4_LIB_PATH ${LZ4_DIR}/x64/dll)
	elseif(CMAKE_PLATFORM STREQUAL "Win32")
		set(LZ4_LIB_PATH ${LZ4_DIR}/x86/dll)
	endif()

	set (LZ4_LIBS liblz4)
endif()

if (USE_CURL)
	if(NOT DEFINED CURL_DIR)
		if(CMAKE_PLATFORM STREQUAL "x64")
			set(CURL_DIR $ENV{DEVELOPMENT_ROOT}/ThirdParty/curl/x64)
		elseif(CMAKE_PLATFORM STREQUAL "Win32")
			set(CURL_DIR $ENV{DEVELOPMENT_ROOT}/ThirdParty/curl/Win32)
		endif()
	endif(NOT DEFINED CURL_DIR)


	set(CURL_INCLUDE_PATH ${CURL_DIR}/include)
	set(CURL_LIB_PATH ${CURL_DIR}/lib)
	set(CURL_LIBS libcurl_imp)
endif()

if (MSVC)
	set(COMPILE_DEFINITIONS /WX /wd4100 /wd4505 /wd4706)

	if ("${CMAKE_CXX_FLAGS}" MATCHES "/W[1-4]")
		  STRING(REGEX REPLACE "/W[1-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endif()

	if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64")
		if (ENABLE_AVX2)
			set(COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS} /arch:AVX2")
			MESSAGE("AVX2 instructions are enabled")
		endif()
	endif()
elseif(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_STANDARD_LIBRARIES "-lgcc -lstdc++ -lwsock32 -lws2_32 -lgdi32 ${CMAKE_CSS_STANDARD_LIBRARIES}")
	set(CMAKE_CXX_FLAGS "-Wa,-mbig-obj -mstack-protector-guard=global ${CMAKE_CXX_FLAGS}")
	if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "AMD64")
		if (ENABLE_AVX2)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
			MESSAGE("AVX2 instructions are enabled")
		endif()
	endif()

	set(COMPILE_DEFINITIONS -Wall -Wno-write-strings -Wno-comment -Wno-unknown-pragmas -Wno-unused-function -Wno-misleading-indentation -Wno-strict-aliasing -Wno-parentheses -Werror)
endif()

include_directories(${BOOST_INCLUDE_PATH} ${GSTREAMER_INCLUDE_PATH} ${GLFW_INCLUDE_PATH} ${OPENCV_INCLUDE_PATH} ${LZ4_INCLUDE_PATH} ${CURL_INCLUDE_PATH})
link_directories(${BOOST_LIB_PATH} ${GLFW_LIB_PATH} ${OPENCV_LIB_PATH} ${LZ4_LIB_PATH} ${CURL_LIB_PATH})
if (NOT CMAKE_COMPILER_IS_GNUCXX)
	link_directories(${GSTREAMER_LIB_PATH})
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
