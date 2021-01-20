cmake_minimum_required(VERSION 3.0)
project(argparse)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_IPO "lto" ON)
include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

add_library(argparse INTERFACE)
target_include_directories(
	argparse
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

install(
	TARGETS argparse
	EXPORT argparseTargets
)
install(
	FILES
		"${CMAKE_CURRENT_LIST_DIR}/include/argparse/argparse.hpp"
	DESTINATION
		include/argparse
)
install(
	EXPORT
		argparseTargets
	NAMESPACE argparse::
	FILE argparseConfig.cmake
	DESTINATION cmake
)

if (BUILD_TESTS)
	add_executable(argtest ${CMAKE_CURRENT_LIST_DIR}/test.cpp)
	target_link_libraries(argtest PRIVATE argparse)

	install(
		TARGETS argtest
		RUNTIME DESTINATION tests)
endif()
