cmake_minimum_required(VERSION 2.8)
project(RemoteAgentSample)

set(REMOTEAGENT_ROOT ${CMAKE_CURRENT_LIST_DIR})

include_directories(${REMOTEAGENT_ROOT}/Common)
include_directories(${REMOTEAGENT_ROOT}/Include)
add_subdirectory(RemoteAgentPublisher)
add_subdirectory(RemoteAgentSubscriber)

