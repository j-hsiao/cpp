cmake_minimum_required(VERSION 3.0)
project(argparse)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

add_library(argparse INTERFACE)
install_project(
	"${CMAKE_CURRENT_LIST_DIR}/include/argparse/argparse.hpp"
)

if (BUILD_TESTS)
	add_subdirectory(tests)
endif()
