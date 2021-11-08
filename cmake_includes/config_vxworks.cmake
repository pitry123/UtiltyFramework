set(COMPILE_DEFINITIONS -Wall -Wno-weak-vtables -Wno-unused-private-field -Wno-unused-lambda-capture -Wno-unused-function -Wno-overloaded-virtual -Werror)
set(CMAKE_CXX_STANDARD 11)

add_definitions(-DVXWORKS)
MESSAGE("Added preprocessor definition: VXWORKS")

set(UTILS_UNIX_ROOT $ENV{WINDRIVER_ROOT}/Ver7.0/workspace/xes_platform/usr/h/published/UTILS_UNIX/)
set(UTILS_UNIX_INCLUDE_PATH ${UTILS_UNIX_ROOT}/)
include_directories(${UTILS_UNIX_INCLUDE_PATH})