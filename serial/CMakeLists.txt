cmake_minimum_required(VERSION 3.0)

project(serial)
include(CTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

execute_process(
	COMMAND
		"${CMAKE_COMMAND}"
		-E make_directory "${CMAKE_CURRENT_BINARY_DIR}/check"
	ERROR_VARIABLE err)
message("make check dir\n${err}")

if (CMAKE_GENERATOR_PLATFORM)
	set(genopt "-DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}")
else()
	set(genopt "")
endif()
execute_process(
	COMMAND
		"${CMAKE_COMMAND}" "${CMAKE_CURRENT_LIST_DIR}/check"
		-G "${CMAKE_GENERATOR}" ${genopt}
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/check"
	ERROR_VARIABLE err)
message("configure check\n${err}")
execute_process(
	COMMAND
		"${CMAKE_COMMAND}"
		--build "${CMAKE_CURRENT_BINARY_DIR}/check"
		--config "${CMAKE_BUILD_TYPE}"
		--target install
	ERROR_VARIABLE err)
message("build checks\n${err}")

execute_process(
	COMMAND
		"${CMAKE_CURRENT_BINARY_DIR}/check/checktools/serialcheck"
		endian
	OUTPUT_VARIABLE endianness
	ERROR_VARIABLE err
	OUTPUT_STRIP_TRAILING_WHITESPACE)
message("checking endian\n${err}")
message("endianness is ${endianness}")

execute_process(
	COMMAND
		"${CMAKE_CURRENT_BINARY_DIR}/check/checktools/serialcheck"
		rep
	OUTPUT_VARIABLE irep
	ERROR_VARIABLE err
	OUTPUT_STRIP_TRAILING_WHITESPACE)
message("checking irep\n${err}")
message("int rep is ${irep}")

if (
	endianness STREQUAL "LITTLE"
	OR endianness STREQUAL "BIG"
	OR endianness STREQUAL "UNKNOWN"
)
	set(SERIAL_ENDIAN "${endianness}" CACHE STRING "endianness")
else()
	message(WARNING "failed to detect endianness was \"${endianness}\", set manually")
	set(SERIAL_ENDIAN "" CACHE STRING "endianness")
endif()
set_property(CACHE SERIAL_ENDIAN PROPERTY STRINGS "LITTLE" "BIG" "UNKNOWN")

if (
	irep STREQUAL "TWO"
	OR irep STREQUAL "ONE"
	OR irep STREQUAL "SMAG"
	OR irep STREQUAL "UNKNOWN"
)
	set(SERIAL_REP "${irep}" CACHE STRING "integer rep")
else()
	message(WARNING "failed to detect rep, was \"${irep}\", set manually")
	set(SERIAL_REP "" CACHE STRING "integer representation")
endif()
set_property(CACHE SERIAL_REP PROPERTY STRINGS "TWO" "ONE" "SMAG" "UNKNOWN")

configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/include/.serial/serial_impl.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/serial/serial_impl.h"
	@ONLY)



add_library(serial src/serial.cpp)
configure_dll()
install_project(include/serial ${CMAKE_CURRENT_BINARY_DIR}/include/serial)

if (BUILD_TESTING)
	add_executable(serialtest ./src/test.cpp)
	target_link_libraries(serialtest PRIVATE serial)
	add_test(NAME serial COMMAND serialtest)
endif()
