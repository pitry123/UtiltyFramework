# GStreamer
if(USE_GSTREAMER)
	set(GSTREAMER_INCLUDE_PATH ${CONAN_GSTREAMER_ROOT}/include/gstreamer-1.0/ ${CONAN_GSTREAMER_ROOT}/include/glib-2.0/ ${CONAN_GSTREAMER_ROOT}/lib/glib-2.0/include)

	if (CMAKE_COMPILER_IS_GNUCXX)
		set (GSTREAMER_LIBS
			${CONAN_LIB_DIRS_GSTREAMER}/libglib-2.0.dll.a
			${CONAN_LIB_DIRS_GSTREAMER}/libgstbase-1.0.dll.a 
			${CONAN_LIB_DIRS_GSTREAMER}/libgstreamer-1.0.dll.a 
			${CONAN_LIB_DIRS_GSTREAMER}/libgsttag-1.0.dll.a
			${CONAN_LIB_DIRS_GSTREAMER}/libgstpbutils-1.0.dll.a
			${CONAN_LIB_DIRS_GSTREAMER}/libgobject-2.0.dll.a
			${CONAN_LIB_DIRS_GSTREAMER}/libgstapp-1.0.dll.a
			${CONAN_LIB_DIRS_GSTREAMER}/libgstrtspserver-1.0.dll.a
			${CONAN_LIB_DIRS_GSTREAMER}/libgstrtp-1.0.dll.a
			${CONAN_LIB_DIRS_GSTREAMER}/libgstvideo-1.0.dll.a)
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

if (USE_LZ4)
	set(LZ4_INCLUDE_PATH ${CONAN_INCLUDE_DIRS_LZ4})
	set(LZ4_LIB_PATH ${CONAN_LIB_DIRS_LZ4})	

	set(LZ4_LIBS liblz4)
endif()

if (USE_CURL)
	set(CURL_LIBS libcurl_imp)
endif()

# Boost
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

# GLFW
set(OPENGL_LIBS OpenGL32)
set(GLFW_LIBS glfw3)

# OpenCV
if(USE_OPENCV)
	set (OPENCV_VERSION "345")
	if (MSVC)
		set(OPENCV_LIBS opencv_world${OPENCV_VERSION})
	elseif(CMAKE_COMPILER_IS_GNUCXX)
		set(OPENCV_LIBS opencv_core${OPENCV_VERSION}.dll opencv_imgproc${OPENCV_VERSION}.dll)
	endif()
endif() #USE_OPENCV

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

include_directories(${CONAN_INCLUDE_DIRS_BOOST} ${GSTREAMER_INCLUDE_PATH} ${CONAN_INCLUDE_DIRS_GLFW3} ${CONAN_INCLUDE_DIRS_OPENCV} ${LZ4_INCLUDE_PATH} ${CONAN_INCLUDE_DIRS_CURL})
link_directories(${CONAN_LIB_DIRS_BOOST} ${CONAN_LIB_DIRS_GLFW3} ${CONAN_LIB_DIRS_OPENCV} ${LZ4_LIB_PATH} ${CONAN_LIB_DIRS_CURL})
if (NOT CMAKE_COMPILER_IS_GNUCXX)
	link_directories(${CONAN_LIB_DIRS_GSTREAMER})
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
