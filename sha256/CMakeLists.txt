cmake_minimum_required(VERSION 3.0)

project(sha256)
include(CTest)
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake NO_POLICY_SCOPE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


execute_process(
	COMMAND
		"${CMAKE_COMMAND}"
		-E make_directory "${CMAKE_CURRENT_BINARY_DIR}/checks"
	ERROR_VARIABLE err)
message("make check dir: ${err}")

if (CMAKE_GENERATOR_PLATFORM)
	execute_process(
		COMMAND
			"${CMAKE_COMMAND}" "${CMAKE_CURRENT_LIST_DIR}/checks"
			-G "${CMAKE_GENERATOR}"
			-DCMAKE_GENERATOR_PLATFORM="${CMAKE_GENERATOR_PLATFORM}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/checks"
		ERROR_VARIABLE err
	)
else()
	execute_process(
		COMMAND
			"${CMAKE_COMMAND}" "${CMAKE_CURRENT_LIST_DIR}/checks"
			-G "${CMAKE_GENERATOR}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/checks"
		ERROR_VARIABLE err
	)
endif()

message("configure check\n${err}")
execute_process(
	COMMAND
		"${CMAKE_COMMAND}"
		--build "${CMAKE_CURRENT_BINARY_DIR}/checks"
		--config "${CMAKE_BUILD_TYPE}"
		--target install
	ERROR_VARIABLE err
)
message("build checks\n${err}")

execute_process(
	COMMAND
		${CMAKE_CURRENT_BINARY_DIR}/checks/checktools/endiancheck
	OUTPUT_VARIABLE endianess
	ERROR_VARIABLE err
)
message("checking endian\n${err}")
message("endianess is ${endianess}")

if (
	endianess STREQUAL "LITTLE"
	OR endianess STREQUAL "BIG"
	OR endianess STREQUAL "MIXED"
)
	set(SHA256_ENDIAN "${endianess}" CACHE STRING "endianness")
else()
	message(WARNING "failed to detect endianess, set manually")
	set(SHA256_ENDIAN "" CACHE STRING "endianness")
endif()
set_property(CACHE SHA256_ENDIAN PROPERTY STRINGS "LITTLE" "BIG" "MIXED")

configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/include/.sha256/types_impl.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/include/sha256/types_impl.h"
	@ONLY
)

add_library(sha256 src/sha256.cpp)
configure_dll()
install_project(include/sha256 ${CMAKE_CURRENT_BINARY_DIR}/include/sha256)

if (NOT TARGET argparse)
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../argparse" argparse)
endif()
add_executable(sha256_exe src/main.cpp)
target_link_libraries(sha256_exe PRIVATE sha256 argparse)
set_target_properties(
	sha256_exe PROPERTIES
	RUNTIME_OUTPUT_NAME sha256)
install(
	TARGETS sha256_exe
	RUNTIME DESTINATION bin)

if (BUILD_TESTING)
	add_executable(t_types tests/types.cpp)
	target_link_libraries(t_types PRIVATE sha256)
	add_test(NAME types COMMAND t_types)

	add_executable(t_sha256 tests/sha256.cpp)
	target_link_libraries(t_sha256 PRIVATE sha256)
	add_test(NAME sha256 COMMAND t_sha256)
endif()
