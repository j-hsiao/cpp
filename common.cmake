# default build type
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type" FORCE)
endif()
set(CMAKE_CONFIGURATION_TYPES "Release")

string(REGEX MATCH ".*/[^/]*mingw[^/]*" isMingw "${CMAKE_CXX_COMPILER}")
set(MINGW "${isMingw}" CACHE STRING "is mingw")

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
	# maybe work for function, but not for macros?
	function(build_dependency dep depsrc)
		# even if a dependency is private only
		# it'll still show up in the generated ${PROJECT_NAME}Targets.cmake
		# so just always use the CMAKE_INSTALL_PREFIX i guess
		set(dependency_install_dir "${CMAKE_INSTALL_PREFIX}")

		execute_process(
			COMMAND
				"${CMAKE_COMMAND}"
				-E make_directory
				"${CMAKE_CURRENT_BINARY_DIR}/${dep}")
		execute_process(
			COMMAND
				"${CMAKE_COMMAND}"
				"${depsrc}"
				"-DCMAKE_INSTALL_PREFIX=${dependency_install_dir}"
				"-DBUILD_TESTING=OFF"
				"-DMSVC=${MSVC}"
				"-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}"
				-G "${CMAKE_GENERATOR}"
			WORKING_DIRECTORY
				"${CMAKE_CURRENT_BINARY_DIR}/${dep}"
		)
		execute_process(
			COMMAND
				"${CMAKE_COMMAND}" --build .  --target install --config Release
			WORKING_DIRECTORY
				"${CMAKE_CURRENT_BINARY_DIR}/${dep}"
		)

		
	endfunction()
	set(commondir "${CMAKE_CURRENT_LIST_DIR}")
	function(configure_dll)
		set(${PROJECT_NAME}_SHARED "${BUILD_SHARED_LIBS}")
		string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)
		option("${PROJECT_NAME_UPPER}_DEBUG" "debugging for ${PROJECT_NAME}" OFF)
		message("${PROJECT_NAME} debugging: ${${PROJECT_NAME_UPPER}_DEBUG}")
		if ("${${PROJECT_NAME_UPPER}_DEBUG}")
			target_compile_definitions(
				${PROJECT_NAME} PRIVATE "${PROJECT_NAME_UPPER}_DEBUG=1")
		else()
			target_compile_definitions(
				${PROJECT_NAME} PRIVATE "${PROJECT_NAME_UPPER}_DEBUG=0")
		endif()
		set(out "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/${PROJECT_NAME}_dllcfg.h")
		if (${CMAKE_VERSION} VERSION_LESS 3.17)
			configure_file(
				"${commondir}/common.h.in"
				"${out}"
				@ONLY)
		else()
			configure_file(
				"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/common.h.in"
				"${out}"
				@ONLY)
		endif()
		target_include_directories(
			${PROJECT_NAME} PUBLIC
			$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
		install(
			FILES "${out}"
			DESTINATION "include/${PROJECT_NAME}"
		)
	endfunction()

	# use this to find_package
	macro(find_export_package ...)
		find_package(${ARGV})
		string(REPLACE ";" " " argstr "${ARGV}")
		
		string(APPEND ${PROJECT_NAME}_find_package_commands "find_dependency(${argstr})\n")
		#TODO
		# call find_package
		# add args to find_package_commands for the config file
	endmacro()

	# install_project to install current project
	# (excluding header files)
	macro(install_project ...)
		get_target_property(libtype ${PROJECT_NAME} TYPE)
		if (libtype STREQUAL "INTERFACE_LIBRARY")
			target_include_directories(
				${PROJECT_NAME} INTERFACE
				$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
				$<INSTALL_INTERFACE:include>)
		else()
			target_include_directories(
				${PROJECT_NAME} PUBLIC
				$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
				$<INSTALL_INTERFACE:include>)
		endif()
		# after reading the cmake packages page a bit more, seems like it should be:
		# export target to a blahblahTargets.cmake
		# then create a blahblahConfig.cmake file that:
		#   includes the blahblahTargets.cmake
		#   find_package for required packages
		# include(CMakePackageConfigHelpers)
		message("exporting ${PROJECT_NAME}")
		install(
			TARGETS ${PROJECT_NAME}
			EXPORT ${PROJECT_NAME}_exports
			RUNTIME DESTINATION bin
			LIBRARY DESTINATION lib
			ARCHIVE DESTINATION lib
		)
		# not sure why, but without this export
		# then a parent dir that used add_subdirectory
		# will error out saying:
		#      export called with target "targetname"
		#      which requires target "subdir lib" that is not in any export set
		# even though definitely in an export set ${PROJECT_NAME}_exports
		# but the even weirder thing is
		# making a small simple lib that wraps subdirlib and uses add_subdirectory
		# had no issues
		export(
			EXPORT ${PROJECT_NAME}_exports
			NAMESPACE ${PROJECT_NAME}::
			FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake")

		set(find_package_commands "${${PROJECT_NAME}_find_package_commands}")
		configure_file(
			"${commondir}/config.cmake.in"
			"${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
			@ONLY)
		if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
			set(configdst cmake)
		else()
			set(configdst lib/cmake/${PROJECT_NAME})
		endif()
		install(
			EXPORT ${PROJECT_NAME}_exports
			FILE ${PROJECT_NAME}Targets.cmake
			DESTINATION "${configdst}"
			NAMESPACE ${PROJECT_NAME}::
		)
		install(
			FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
			DESTINATION "${configdst}")

			# after reading a bit more on this
			# seems like EXPORT_LINK_INTERFACE_LIBRARIES
			# only links dlls to dependencies explicitly instead
			# of implicitly and is ignored for static libs
			#EXPORT_LINK_INTERFACE_LIBRARIES
		foreach (header ${ARGV})
			get_filename_component(headpath "${header}" ABSOLUTE)
			if (IS_DIRECTORY ${headpath})
				install(
					DIRECTORY "${headpath}"
					DESTINATION include
				)
			else()
				install(
					FILES "${headpath}"
					DESTINATION include/${PROJECT_NAME}
				)
			endif()
		endforeach()
	endmacro()
endif()
