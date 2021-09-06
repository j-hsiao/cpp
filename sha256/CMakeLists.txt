cmake_minimum_required(VERSION 3.0)

project(sha256)
include(CTest)
include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake NO_POLICY_SCOPE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_or_build_dep(sysinfo "${CMAKE_CURRENT_LIST_DIR}/../sysinfo")
find_or_build_dep(argparse "${CMAKE_CURRENT_LIST_DIR}/../argparse")

add_library(sha256 src/sha256.cpp)
target_link_libraries(sha256 PUBLIC sysinfo::sysinfo)
configure_dll()
install_project(include/sha256 ${CMAKE_CURRENT_BINARY_DIR}/include/sha256)

add_executable(sha256_exe src/main.cpp)
target_link_libraries(sha256_exe PRIVATE sha256 argparse::argparse)
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
