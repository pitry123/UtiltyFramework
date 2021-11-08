macro (post_build_event PROJECT PROJECT_ROOT PROJECT_APP_ROOT)
  if( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  
   if(EXISTS ${PROJECT_APP_ROOT}/Configuration)
	 add_custom_command(TARGET ${PROJECT} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${PROJECT_APP_ROOT}/Configuration"
			"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/Configuration")
	endif()
   
   if(EXISTS ${PROJECT_APP_ROOT}/DE)
    MESSAGE("${PROJECT_APP_ROOT}/DE -> ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/DE" ) 
	add_custom_command(TARGET ${PROJECT} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${PROJECT_APP_ROOT}/DE"
			"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/DE")
   endif() 
  elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

        file(RELATIVE_PATH RELATIVE ${PROJECT_ROOT} ${CMAKE_CURRENT_LIST_DIR})
	    MESSAGE("${CMAKE_BINARY_DIR}/${RELATIVE}")
	   if(EXISTS ${PROJECT_APP_ROOT}/Configuration)
     	   add_custom_command(TARGET ${PROJECT} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                            "${PROJECT_APP_ROOT}/Configuration"
                            "${CMAKE_BINARY_DIR}/${RELATIVE}/Configuration")
		endif()
	  if(EXISTS ${PROJECT_APP_ROOT}/DE)
	    add_custom_command(TARGET ${PROJECT} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                            "${PROJECT_APP_ROOT}/DE"
                            "${CMAKE_BINARY_DIR}/${RELATIVE}/DE")
      endif()

endif()

endmacro()
