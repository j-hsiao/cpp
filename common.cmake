# default build type
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type" FORCE)
endif()
set(CMAKE_CONFIGURATION_TYPES "Release")

option(BUILD_TESTS "build tests" ON)
option(ENABLE_IPO "enable interprocedural optimization (lto)" OFF)
#enabling ipo
if (ENABLE_IPO)
	if("${CMAKE_VERSION}" VERSION_GREATER 3.9)
		cmake_policy(GET CMP0069 cmpcheck)
		if (NOT cmpcheck STREQUAL NEW)
			cmake_policy(SET CMP0069 NEW)
		endif()
		include(CheckIPOSupported)
		check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
		if (ipo_error)
			messsage(WARNING "ipo error: ${ipo_error}")
		elseif(ipo_supported)
			set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
		else()
			if (NOT warned_ipo_support)
				message(WARNING "IPO not supported")
			endif()
			set(warned_ipo_support ON STRING "already warned")
		endif()
	else()
		if (NOT warned_ipo_vs)
			message(WARNING "IPO requires cmake >= 3.9, current version is ${CMAKE_VERSION}")
		endif()
		set(warned_ipo_vs ON STRING "already warned")
	endif()
endif()

#configure a ${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/${PROJECT_NAME}_dllcfg.h
# header file containing appropriate ${PROJECT_NAME}_API definition
if (NOT COMMAND configure_dll)
	if (${CMAKE_VERSION} VERSION_GREATER 3.17)
		set(configure_dll_in "${CMAKE_CURRENT_LIST_DIR}/common.h.in")
	endif()
	function(configure_dll)
		set(${PROJECT_NAME}_SHARED "${BUILD_SHARED_LIBS}")
		set(out "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/${PROJECT_NAME}_dllcfg.h")
		if (${CMAKE_VERSION} VERSION_GREATER 3.17)
			configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/common.h.in" "${out}")
		else()
			configure_file("${configure_dll_in}" "${out}")
		endif()
		target_include_directories(
			${PROJECT_NAME} PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
		install(
			FILES "${out}"
			DESTINATION "include/${PROJECT_NAME}"
		)
	endfunction()
endif()

# install_project to install current project
# (excluding header files)
if (NOT COMMAND install_project)
	macro(install_project ...)
		get_target_property(libtype ${PROJECT_NAME} TYPE)
		if (libtype STREQUAL "INTERFACE_LIBRARY")
			target_include_directories(
				${PROJECT_NAME} INTERFACE
				$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
				$<INSTALL_INTERFACE:install>)
		else()
			target_include_directories(
				${PROJECT_NAME} PUBLIC
				$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
				$<INSTALL_INTERFACE:install>)
		endif()
		install(
			TARGETS ${PROJECT_NAME}
			EXPORT ${PROJECT_NAME}_targets
			RUNTIME DESTINATION bin
			LIBRARY DESTINATION lib
			ARCHIVE DESTINATION lib
		)
		install(
			EXPORT ${PROJECT_NAME}_targets
			FILE ${PROJECT_NAME}Config.cmake
			DESTINATION cmake
			NAMESPACE ${PROJECT_NAME}::
			EXPORT_LINK_INTERFACE_LIBRARIES
		)
		foreach (header ${ARGV})
			if (IS_DIRECTORY ${header})
				install(
					DIRECTORY "${header}"
					DESTINATION include
				)
			else()
				install(
					FILES "${header}"
					DESTINATION include/${PROJECT_NAME}
				)
			endif()
		endforeach()
	endmacro()
endif()
