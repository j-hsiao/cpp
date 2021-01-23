cmake_minimum_required(VERSION 3.0)
project(argparse)
include(CTest)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

add_library(argparse INTERFACE)
install_project(include/argparse)

if (BUILD_TESTING)
	add_executable(argparsetest tests/test.cpp)
	target_link_libraries(argparsetest PRIVATE argparse)
	add_test(NAME argparse COMMAND argparsetest)
endif()
