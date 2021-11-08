cmake_minimum_required(VERSION 2.8)
project(RulesSampleDynamic)
set(APP_ROOT ${CMAKE_CURRENT_LIST_DIR})

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
endif()

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

add_executable(${PROJECT_NAME}
		RulesSample.cpp
		${RULES_APP_ROOT}/Common/RulesSample/RulesDB.h
		${RULES_APP_ROOT}/Common/RulesSample/ProjectCommon.h
		${RULES_APP_ROOT}/Common/RulesSample/CommonDbDefs.h
		${RULES_APP_ROOT}/Common/RulesSample/ControllerDefs.h
		${RULES_APP_ROOT}/Common/RulesSample/SystemStates_StateMachine.h
		${RULES_APP_ROOT}/Common/RulesSample/ProjectCommonDbDefs.h
		${RULES_APP_ROOT}/Common/RulesSample/Scanning_StateMachine.h
		${RULES_APP_ROOT}/Common/RulesSample/BringTo_StateMachine.h
		${RULES_APP_ROOT}/Common/RulesSample/UiDB.h
		${RULES_APP_ROOT}/Common/RulesSample/Operational_ActivitiesLogicDB.h
        )


include_directories(${APP_ROOT}/Include)
include_directories(${APP_ROOT}/ModernAPI)
include_directories(${APP_ROOT}/Common)

include(${ROOT_DIR}/cmake_includes/version.cmake)
conan_cmake_version(${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
	${PROJECT_NAME}
	${CORE_LIBS}
	binary_parser
	memory_database
	ports
	protocols
	boost_logger
	common_files
	dynamic_rules
)

include(${ROOT_DIR}/cmake_includes/build_events.cmake)
post_build_event(${PROJECT_NAME} ${ROOT_DIR} ${RULES_APP_ROOT})

install(TARGETS ${PROJECT_NAME} DESTINATION ${BIN_DIR})
