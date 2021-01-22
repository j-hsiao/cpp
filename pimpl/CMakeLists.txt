cmake_minimum_required(VERSION 3.0)

project(pimpl)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(BUILD_TESTS "build tests" ON)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

add_library(pimpl INTERFACE)
install_project("${CMAKE_CURRENT_LIST_DIR}/include/pimpl")

if (BUILD_TESTS)
	add_subdirectory(tests)
endif()
