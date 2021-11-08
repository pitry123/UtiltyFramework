cmake_minimum_required(VERSION 2.8)
project(VideoSubscriber)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

if (MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
	endif()
endif()

add_executable(${PROJECT_NAME}
		viewer.h
		VideoSubscriber.cpp
        )

target_link_libraries(${PROJECT_NAME}
${CORE_LIBS}
shared_memory_video
image_converter
${OPENGL_LIBS}
${GLFW_LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
