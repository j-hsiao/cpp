# provided helper functions/macros
# NOTE: In general, lists must be passed as expanded and in quotes.  This means no
#	items in the list should be a list itself. Use a different delimiter if this
#	is the case.
#
# adds options:
#	VERSIONED_INSTALL: projects that use the install methods here
#		will have the paths prefixed by <name>PROJECT_VERSION.
#		This can allow for concurrent installations of different versions.
#
# dset(var val)
#
#	Set a variable if falsey.
#
# list_get_keys(
#	<list> [-<flag>[,<key1>,<key2>...]]
#	[<lst>[,<key1>,<key2>...] [[-]<maxlength>]]
#	[+<unassigned>])
#
#	Parse <list> into flags or lists, or unassigned.
#	flags:
#		Flags are prefixed with a -.  If a flag output has only 1 key
#		associated with it, then its value will be 1 if the flag was
#		found and "" otherwise.  If a flag has multiple keys, then its
#		value will instead be a list of all the keys that were found.
#		To see if a particular key exists, use list(FIND <flag> <key> ...).
#	lists:
#		Values following after a list's keys will be appended and returned
#		in the outvar.  If multiple keys are given, then the keys will be
#		added as well (if <maxedlength> not yet reached) but will not count
#		towards maxlength.
#	unassigned:
#		This is a list of any values not assigned to flags/lists
#	Keys associated with an empty outvariable will be treated as
#	unassigned and will interrupt parsing of any list outputs.  (These would
#	tell when the list ends).  Empty keys will be ignored, but will
#	contribute to the key count.  If a flag/list has no keys (or are all empty)
#	then te flag/list will be used as the key.
#
#
# versioned_prefix(<out> [NAME <name>] [VERSION <version>])
#
#	Set <out> to <name><version>/ if VERSIONED_INSTALL cache variable
#	is set, else "".
#	<name> defaults to PROJECT_NAME.
#	<version> defaults to PROJECT_VERSION.
#
# versioned_install([NAME <name>] [VERSION version] ...)
#
#	Wrap install() command.  If VERSIONED_INSTALL, then any DESTINATION
#	arguments will be prefixed with the versioned prefix if relative.
#	NAME and VERSION are same as in versioned_prefix().  Destinations
#	should not be empty (though not sure if it'll make a difference...)
#	use "." instead of "". NOTE you cannot just change the CMAKE_INSTALL_PREFIX
#	because any find_or_add_subdirs would then be installed to the wrong location.
#
# default_package_install_dir(<out> [NAME <name>])
#	 
#	Return the default package installation location.  <name> will
#	default to PROJECT_NAME.  On windows, this will be "cmake".
#	Otherwise, "share/cmake/<name>"
#
# default_install(<mode> [NAME <name>] [VERSION version] ...)
#
#	Wrap install().  <mode> is the install mode (TARGETS, EXPORT, FILES, ...).
#	Add NAME and VERSION arguments (same as in versioned_prefix)
#	default_install() uses versioned_install() instead of install() (affects
#	all DESTINATION arguments).
#	These modes are given defaults:
#		TARGETS:
#			TARGETS: <name>
#			EXPORT: <name>_exports
#			RUNTIME/LIBRARY/ARCHIVE DESTINATION: bin/lib/lib
#		EXPORT:
#			EXPORT: <name>_exports
#			DESTINATION: see default_package_install_dir()
#			NAMESPACE: <name>::
#			FILE: <name>Targets.cmake
#
# default_export(EXPORT export NAMESPACE namespace FILE file)
#
#	export() with defaults:
#	EXPORT: PROJECT_NAME_exports
#	NAMESPACE: PROJECT_NAME::
#	FILE: ${CMAKE_CURRENT_BINARY_DIR}/cmake_packages/${PROJECT_NAME}Targets.cmake
#
# install_configfile(
#	[NAME <name>] [DEPENDS <depends>] [FILE <filename>]
#	[EXPORT_FILE <export_file] [DESTINATION <destination>]
#	[OUTDIR <output>]
#	[VERSION <version>] [COMPATIBILITY <compatibility>]
#	[VFILE <version_file>])
#
#	Configure and install a simple Config.cmake file.
#	It:
#		includes FindDependencyMacro
#		calls find_dependency (passed via <depends>)
#		include the installed export file
#
#	NAME (PROJECT_NAME) The name of the package.
#	DEPENDS ("") A string of find_dependency() calls
#	FILE (<name>Config.cmake) The output config file name.
#	EXPORT_FILE (<name>Targets.cmake) The installed export file to include.
#	DESTINATION (default_package_install_dir()) The destination to install
#		the config file to.
#	OUTDIR (CURRENT_BINARY_DIR/cmake_packages) The build-time output directory.
#	VERSION (PROJECT_VERSION) version of the project.  If non-empty,
#		a <name>ConfigVersion.cmake file will be created alongside the Config file.
#		(CMakePackageConfigHelpers -> write_basic_package_version_file)
#	VERSION_FILE (<FILE-no-extension>Version.cmake) The output version file name.
#	COMPATIBILITY (SameMajorVersion)
#
# add_package(<outvar> <packagename> ...)
#
#	Call find_package(<packagename> ...).
#	If successful, append a corresponding find_dependency() line to outvar.
#	The result is suitable for the <depends> argument of install_configfile.
#	For non-required packages, just use find_package instead of add_package.
#
# add_aliases(<namespace> ...)
#
#	add_library(<namespace>::target ALIAS target) for each target in ...
#
# add_package_or_subdir(<outvar> <src> <packagename> ...)
#
#	Try to add_package(<outvar> <packagename> ...).
#	If fail, then add_subdirectory(<src> ${packagename})
#	(NOTE: the package should call add_aliases so same name exists as
#	if find_package had succeeded)
#
# add_includes(
#	<target> <iface> [BUILD <paths>...] [INSTALL <paths>...]
#	[NAME <name>] [VERSION <version>])
#
#	call target_include_directories on target with iface
#	for corresponding $<BUILD_INTERFACE:path> or $<INSTALL_INTERFACE:path>.
#	For BUILD_INTERFACE, relative paths are relative to CMAKE_CURRENT_SOURCE_DIR.
#	INSTALL_INTERFACE paths may be will be prefixed via versioned_prefix()
#	NAME and VERSION are passed fo versioned_prefix if given.
#
# configure_dll(
#	<target> [NAME <name>] [VERSION <version>]
#	[OUT <out>] [DESTINATION <dest>] [FILE <file>])
#
#	Generate <file> at <out>/<file> and install it to <dest>/<file>.
#	Call add_includes(
#		<target> PUBLIC BUILD <out> INSTALL <dest>
#		NAME <name> VERSION <version>).
#
#	If <target> is a shared library, also call
#	target_compile_definitions(<target> PRIVATE UPPER(<name>)_EXPORTS=1)
#
#	The header will define UPPER(<name>)_API to the appropriate
#	__declspec(dll[import/export]).
#
#	<name> (<target>) The package name. Determines defaults for others.
#	<file> (<name>/<name>_dllcfg.h) is what you would use for a #include<<file>>
#		line to include the generated header.
#	<out> (include) The build-time generated output dir.  This will be added to
#		the $<BUILD_INTERFACE>.  If relative, it is relative to
#		CMAKE_CURRENT_BINARY_DIR.
#	<dest> (include) The install destination.  This will be added to the
#		$<INSTALL_INTERFACE> for <target>.
#	<version> (PROJECT_VERSION) version for add_includes

if (NOT COMMAND install_configfile)

	# if CMAKE_BUILD_TYPE not set, default to Release
	if (NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type" FORCE)
	endif()
	set(CMAKE_CONFIGURATION_TYPES "Release")

	string(REGEX MATCH ".*/[^/]*mingw[^/]*" isMingw "${CMAKE_CXX_COMPILER}")
	set(MINGW "${isMingw}" CACHE STRING "using mingw compiler mingw")

	option(ENABLE_IPO "enable interprocedural optimization (lto)" OFF)
	if (ENABLE_IPO)
		if(CMAKE_VERSION VERSION_LESS 3.9)
			if (NOT warned_ipo_vs)
				message(WARNING "IPO requires cmake >= 3.9, current version is ${CMAKE_VERSION}")
			endif()
			set(warned_ipo_vs ON STRING "already warned")
		else()
			cmake_policy(SET CMP0069 NEW)
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
		endif()
	endif()


	function(dset var val)
		if (NOT ${var})
			set(${var} ${val} PARENT_SCOPE)
		endif()
	endfunction()


	function(list_get_keys lst)
		# parse keys
		set(keys "") # Out
		set(flags "") # Values, NumKeys Length (-1 always)
		set(lsts "") # Values, Force, Length, NumKeys
		set(curout "")
		set(unassigned "")
		set(unassigneds "")
		foreach(out IN LISTS ARGN)
			if (out MATCHES "^-?[0-9]+$")
				string(REGEX MATCH "^-" "${curout}Force" "${out}")
				string(REGEX MATCH "[0-9]+" "${curout}Length" "${out}")
				if (NOT ${curout}Length)
					set("${curout}Length" -1)
				endif()
				set(curout "")
			elseif(out MATCHES "^\\+.*$")
				string(REGEX MATCH "[^+].*$" unassigned "${out}")
			else()
				string(REPLACE "," ";" out "${out}")
				string(REGEX MATCH "^-" isflag "${out}")
				string(REGEX MATCH "[^-].*$" ks "${out}")
				list(GET ks 0 outname)
				list(REMOVE_AT ks 0)
				set(added 0)
				foreach(key IN LISTS ks)
					if (NOT key STREQUAL "")
						set(added 1)
						set("${key}Out" "${outname}")
						list(APPEND keys "${key}")
					endif()
				endforeach()
				if (NOT outname STREQUAL "")
					if (NOT added)
						list(APPEND keys "${outname}")
						set("${outname}Out" "${outname}")
					endif()
					list(LENGTH ks "${outname}NumKeys")
					set("${outname}Values" "")
					set("${outname}Length" -1)
					if (isflag)
						list(APPEND flags "${outname}")
					else()
						list(APPEND lsts "${outname}")
						set("${outname}Force" 0)
						set(curout "${outname}")
					endif()
				endif()
			endif()
		endforeach()

		# parse list
		# use string(CONCAT) here because list(APPEND) will give wrong
		# result if val is empty string
		set(curout "")
		foreach(val IN LISTS lst)
			list(FIND keys "${val}" idx)
			if (idx LESS 0)
				if (NOT curout STREQUAL "" AND ${curout}Length)
					string(CONCAT "${curout}Values" "${${curout}Values}" "${val};")
					if (${curout}Length GREATER 0)
						math(EXPR "${curout}Length" "${${curout}Length} - 1")
					endif()
				else()
					string(CONCAT unassigneds "${unassigneds}" "${val};")
					set(curout "")
				endif()
			else()
				set(outname "${${val}Out}")
				if (outname STREQUAL "")
					# ignored key
					set(curout "")
					string(CONCAT unassigneds "${unassigneds}" "${val};")
				else()
					list(FIND flags "${outname}" idx)
					if (idx LESS 0)
						if (${outname}Length)
							set(curout "${outname}")
							if (${outname}NumKeys GREATER 1)
								string(
									CONCAT "${outname}Values"
									"${${outname}Values}" "${val};")
							endif()
						else()
							set(curout "")
							string(CONCAT unassigneds "${unassigneds}" "${val};")
						endif()
					else()
						# flag
						if (${outname}NumKeys GREATER 1)
							string(
								CONCAT "${outname}Values"
								"${${outname}Values}" "${val};")
						else()
							set("${outname}Values" 1)
						endif()
						set(curout "")
					endif()
				endif()
			endif()
		endforeach()

		# set return values
		foreach(flag IN LISTS flags)
			string(REGEX REPLACE ";$" "" retval "${${flag}Values}")
			set("${flag}" "${retval}" PARENT_SCOPE)
		endforeach()
		if(NOT unassigned STREQUAL "")
			string(REGEX REPLACE ";$" "" retval "${unassigneds}")
			set(${unassigned} "${retval}" PARENT_SCOPE)
		endif()
		foreach(lstout IN LISTS lsts)
			if(NOT "${${lstout}Values}" STREQUAL "" OR ${lstout}Force)
				string(REGEX REPLACE ";$" "" retval "${${lstout}Values}")
				set("${lstout}" "${retval}" PARENT_SCOPE)
			endif()
		endforeach()
	endfunction()


	option(VERSIONED_INSTALL "install into prefix install locations with <name><version>" ON)

	function(versioned_prefix out)
		set(packagename "${PROJECT_NAME}")
		set(version "${PROJECT_VERSION}")
		list_get_keys("${ARGN}" packagename,NAME 1 version,VERSION 1)
		if(VERSIONED_INSTALL)
			set(${out} "${packagename}${version}/" PARENT_SCOPE)
		else()
			set(${out} "" PARENT_SCOPE)
		endif()
	endfunction()


	# NAME and VERSION are not valid install(...) keys
	# so it is okay to use them.
	function(versioned_install)
		list_get_keys("${ARGV}" packagename,NAME 1 version,VERSION 1 +remain)
		versioned_prefix(prefix ${ARGV})
		if (VERSIONED_INSTALL)
			set(args "")
			set(isdest 0)
			foreach(arg IN LISTS remain)
				if (arg STREQUAL "DESTINATION")
					set(isdest 1)
					list(APPEND args "${arg}")
				else()
					if (isdest AND NOT IS_ABSOLUTE "${arg}")
						list(APPEND args "${prefix}${arg}")
					else()
						list(APPEND args "${arg}")
					endif()
					set(isdest 0)
				endif()
			endforeach()
		else()
			set(args ${remain})
		endif()
		install(${args})
	endfunction()


	function(default_package_install_dir outvar)
		set(packagename "${PROJECT_NAME}")
		list_get_keys("${ARGN}" packagename,NAME 1)
		if (CMAKE_SYSTEM_NAME STREQUAL "WINDOWS")
			set(${outvar} "cmake" PARENT_SCOPE)
		else()
			set(${outvar} "share/cmake/${packagename}" PARENT_SCOPE)
		endif()
	endfunction()


	function(default_install mode)
		set(packagename "${PROJECT_NAME}")
		set(version "${PROJECT_VERSION}")
		list_get_keys("${ARGN}" packagename,NAME 1 version,VERSION 1 +remain)

		set(namev "NAME" "${packagename}" "VERSION" "${version}")
		list(APPEND args "${mode}" ${remain})
		if (mode STREQUAL "TARGETS")
			set(export "${packagename}_exports")
			set(targets "${packagename}")
			list_get_keys(
				"${args}"
				targets,TARGETS 0
				export,EXPORT 1
				,RUNTIME_DEPENDENCIES,RUNTIME_DEPENDENCY_SET
				,ARCHIVE,LIBRARY,RUNTIME,OBJECTS,FRAMEWORK,BUNDLE
				,PRIVATE_HEADER,PUBLIC_HEADER,RESOURCE
				,INCLUDES
				+remain)
			# later RUNTIME/LIBRARY/ARCHIVE arguments seem to override
			# previous arguments so just hardcoding these destinations
			# results in a "default-like" behavior
			versioned_install(
				TARGETS ${targets}
				EXPORT "${export}"
				RUNTIME DESTINATION bin
				LIBRARY DESTINATION lib
				ARCHIVE DESTINATION lib
				${remain} ${namev})
		elseif (mode STREQUAL "EXPORT")
			default_package_install_dir(
				destination NAME "${packagename}" VERSION "${version}")
			set(export "${packagename}_exports")
			set(fname "${packagename}Targets.cmake")
			set(namespace "${packagename}::")
			list_get_keys(
				"${args}"
				export,EXPORT 1
				destination,DESTINATION 1
				fname,FILE 1
				namespace,NAMESPACE 1
				+remain)
			versioned_install(
				EXPORT "${export}"
				DESTINATION "${destination}"
				NAMESPACE "${namespace}"
				FILE "${fname}"
				${remain} ${namev})
		else()
			versioned_install(${args} ${namev})
		endif()
	endfunction()

	function(default_export)
		set(export "${PROJECT_NAME}_exports")
		set(namespace "${PROJECT_NAME}::")
		set(fname "${CMAKE_CURRENT_BINARY_DIR}/cmake_packages/${PROJECT_NAME}Targets.cmake")
		list_get_keys("${ARGV}" export,EXPORT 1 namespace,NAMESPACE 1 fname,FILE 1)
		export(EXPORT "${export}" NAMESPACE "${namespace}" FILE "${fname}")
	endfunction()

	if (CMAKE_VERSION VERSION_LESS 3.17)
		set(_common_dllin "${CMAKE_CURRENT_LIST_DIR}/dllcfg.h.in")
		set(_common_cfgin "${CMAKE_CURRENT_LIST_DIR}/config.cmake.in")
	endif()
	include(CMakePackageConfigHelpers)
	function(install_configfile)
		set(packagename ${PROJECT_NAME})
		set(outdir "${CMAKE_CURRENT_BINARY_DIR}/cmake_packages")
		set(version "${PROJECT_VERSION}")
		set(compatibility SameMajorVersion)
		list_get_keys(
			"${ARGV}"
			packagename,NAME 1
			depends,DEPENDS -1
			fname,FILE -1
			exportfile,EXPORT_FILE -1
			destination,DESTINATION -1
			outdir,OUTDIR 1
			version,VERSION 1
			vfile,VERSION_FILE -1
			compatibility,COMPATIBILITY 1)
		dset(fname "${packagename}Config.cmake")
		dset(exportfile "${packagename}Targets.cmake")
		if (destination STREQUAL "")
			default_package_install_dir(destination NAME "${packagename}")
		endif()
		if (CMAKE_VERSION VERSION_LESS 3.17)
			configure_file("${_common_cfgin}" "${outdir}/${fname}" @ONLY)
		else()
			configure_file(
				"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/config.cmake.in"
				"${outdir}/${fname}" @ONLY)
		endif()
		versioned_install(
			FILES "${outdir}/${fname}" DESTINATION "${destination}"
			NAME "${packagename}" VERSION "${version}")
		if (NOT version STREQUAL "")
			if (vfile STREQUAL "")
				get_filename_component(vfile_ext "${fname}" EXT)
				get_filename_component(vfile_name "${fname}" NAME_WE)
				set(vfile "${vfile_name}Version${vfile_ext}")
			endif()
			write_basic_package_version_file(
				"${outdir}/${vfile}" VERSION "${version}"
				COMPATIBILITY "${compatibility}")
			versioned_install(
				FILES "${outdir}/${vfile}" DESTINATION "${destination}"
				NAME "${packagename}" VERSION "${version}")
		endif()
	endfunction()


	macro(add_package outvar packagename)
		find_package("${packagename}" ${ARGN})
		if (${packagename}_FOUND)
			add_package_("${outvar}" "${packagename}" ${ARGN})
		endif()
	endmacro()
	function(add_package_ outvar)
		# find_dependency is a little vague. After reading up, it seems that
		# find_dependency actually means that that REQUIRED and QUIET are
		# forwarded from find_package in a CMakeLists.txt searching for the
		# package that find_dependency is called for.
		#
		# ie: liba, libb, dep
		#	libbConfig.cmake has "find_dependency(dep)"
		#	CMakeLists.txt for liba has "find_package(libb ...)"
		#	then if liba calls it as find_package(libb REQUIRED)
		#	then the find_dependency in libbConfig.cmake will also add REQUIRED
		#	as a result, REQUIRED and QUIET should NOT be added to the
		#	find_dependency line.
		list(REMOVE_ITEM ARGN REQUIRED QUIET)
		string(REPLACE ";" " " depline "find_dependency(${ARGN})\n")
		string(CONCAT total "${${outvar}}" "${depline}")
		set("${outvar}" "${total}" PARENT_SCOPE)
	endfunction()


	macro(add_aliases namespace)
		foreach(v IN ITEMS ${ARGN})
			add_library(${namespace}::${v} ALIAS ${v})
		endforeach()
	endmacro()


	macro(add_package_or_subdir outvar src packagename)
		if (CMAKE_FIND_DEBUG_MODE)
			find_package(${packagename} ${ARGN})
		else()
			find_package(${packagename} ${ARGN} QUIET)
		endif()
		if (${packagename}_FOUND)
			message("${packagename} found")
		else()
			if (NOT TARGET ${packagename})
				message("${packagename} not found, adding subdir ${src}")
				add_subdirectory("${src}" ${packagename})
			else()
				message("${packagename} is already a target")
			endif()
		endif()
		# After some testing, it seems the find_dependency in the *Config.cmake
		# file is only necessary if the targets are not in the same export set.
		# But if they are in the same export set, add_package_or_subdir wouldn't
		# have been called anyways, so always add_package_
		add_package_(${outvar} ${packagename} ${ARGN})
	endmacro()


	function(add_includes target iface)
		set(packagename "${PROJECT_NAME}")
		set(version "${PROJECT_VERSION}")
		list_get_keys(
			"${ARGN}"
			buildpaths,BUILD -0
			installpaths,INSTALL -0
			packagename,NAME 1
			version,VERSION 1)

		foreach(idir IN LISTS buildpaths)
			if (NOT IS_ABSOLUTE "${idir}")
				set(idir "${CMAKE_CURRENT_SOURCE_DIR}/${idir}")
			endif()
			target_include_directories(
				"${target}" "${iface}" "$<BUILD_INTERFACE:${idir}>")
		endforeach()
		versioned_prefix(prefix NAME "${packagename}" VERSION "${version}")
		foreach(idir IN LISTS installpaths)
			target_include_directories(
				"${target}" "${iface}" "$<INSTALL_INTERFACE:${prefix}${idir}>")
		endforeach()
	endfunction()


	function(configure_dll target)
		set(packagename "${target}")
		set(out "include")
		set(destination "include")
		list_get_keys(
			"${ARGN}"
			packagename,NAME 1
			out,OUT 1
			destination,DESTINATION 1
			fname,FILE -1
			+remain)
		if (NOT IS_ABSOLUTE "${out}")
			set(out "${CMAKE_CURRENT_BINARY_DIR}/${out}")
		endif()
		dset(fname "${packagename}/${packagename}_dllcfg.h")

		string(TOUPPER "${packagename}" PACKAGENAME)
		get_property(result TARGET "${target}" PROPERTY TYPE)
		if (result STREQUAL "SHARED_LIBRARY")
			set(is_shared 1)
			target_compile_definitions("${target}" PRIVATE "${PACKAGENAME}_EXPORTS=1")
		else()
			set(is_shared 0)
		endif()

		if (CMAKE_VERSION VERSION_LESS 3.17)
			configure_file("${_common_dllin}" "${out}/${fname}" @ONLY)
		else()
			configure_file(
				"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/dllcfg.h.in"
				"${out}/${fname}" @ONLY)
		endif()
		add_includes(
			"${target}" PUBLIC BUILD "${out}" INSTALL "${destination}"
			NAME "${packagename}" ${remain})
		get_filename_component(idest "${destination}/${fname}" DIRECTORY)
		versioned_install(FILES "${out}/${fname}" DESTINATION "${idest}")
	endfunction()

endif()
