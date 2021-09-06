cmake_minimum_required(VERSION 3.0)

project(sysinfo)
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake NO_POLICY_SCOPE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro(check_err)
	if (NOT ecode EQUAL 0)
		message(FATAL_ERROR "error: ${err}")
	endif()
endmacro()

message("make check dir")
execute_process(
	COMMAND
		"${CMAKE_COMMAND}"
		-E make_directory "${CMAKE_CURRENT_BINARY_DIR}/check"
	ERROR_VARIABLE err
	RESULT_VARIABLE ecode)
check_err()

if (CMAKE_GENERATOR_PLATFORM)
	set(genopt "-DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}")
else()
	set(genopt "")
endif()
message("configuring check tool")
execute_process(
	COMMAND
		"${CMAKE_COMMAND}" "${CMAKE_CURRENT_LIST_DIR}/check"
		-G "${CMAKE_GENERATOR}" ${genopt}
		-DCMAKE_INSTALL_PREFIX=.
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/check"
	ERROR_VARIABLE err
	RESULT_VARIABLE ecode)
check_err()
message("Building check tool")
execute_process(
	COMMAND
		"${CMAKE_COMMAND}"
		--build "${CMAKE_CURRENT_BINARY_DIR}/check"
		--config "${CMAKE_BUILD_TYPE}"
		--target install
	ERROR_VARIABLE err
	RESULT_VARIABLE ecode)
check_err()

foreach(command IN ITEMS "ENDIAN" "INTREP" "CPFP32" "CPFP64")
	execute_process(
		COMMAND
			"${CMAKE_CURRENT_BINARY_DIR}/check/bin/sysinfo"
			"${command}"
		OUTPUT_VARIABLE "SYSINFO_${command}_"
		ERROR_VARIABLE err
		OUTPUT_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE ecode)
	check_err()
	message("${command}: ${SYSINFO_${command}_}")
	set(SYSINFO_${command} "${SYSINFO_${command}_}" CACHE STRING "${command}")
endforeach()

configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/include/.sysinfo/sysinfo.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/sysinfo/sysinfo.h"
	@ONLY)

add_library(sysinfo INTERFACE)
target_include_directories(
	sysinfo INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<INSTALL_INTERFACE:include>
)
install_project(${CMAKE_CURRENT_BINARY_DIR}/include/sysinfo)
