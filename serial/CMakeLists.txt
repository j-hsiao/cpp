cmake_minimum_required(VERSION 3.0)

project(serial)
include(CTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

find_export_package(sysinfo)
if (NOT sysinfo_FOUND)
	message("sysinfo not found")
	build_dependency(sysinfo "${CMAKE_CURRENT_LIST_DIR}/../sysinfo")
	find_package(sysinfo REQUIRED)
endif()

add_library(serial src/serial.cpp)
target_link_libraries(serial PUBLIC sysinfo::sysinfo)
configure_dll()
install_project(include/serial ${CMAKE_CURRENT_BINARY_DIR}/include/serial)

if (BUILD_TESTING)
	add_subdirectory(
		"${CMAKE_CURRENT_SOURCE_DIR}/../timeutil"
		"${CMAKE_CURRENT_BINARY_DIR}/timeutil"
		EXCLUDE_FROM_ALL)
	add_executable(serialtest ./src/test.cpp)
	target_link_libraries(serialtest PRIVATE serial timeutil)
	add_test(NAME serial COMMAND serialtest)
endif()
