cmake_minimum_required (VERSION 3.0)

project(cpuinfo)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "build tests?" ON)
option(CPUINFO_DEBUG "build with debug logs?" OFF)

if (MSVC)
	add_library(cpuinfo ./src/cpuinfo_MSVC.cpp)
else()
	add_library(cpuinfo ./src/cpuinfo_GNU.cpp)
endif()
if (CPUINFO_DEBUG)
	target_compile_definitions(cpuinfo PRIVATE CPUINFO_DEBUG)
endif()
configure_dll()
install_project(
	"${CMAKE_CURRENT_LIST_DIR}/include/cpuinfo/cpuinfo.h"
	"${CMAKE_CURRENT_LIST_DIR}/include/cpuinfo/cpuinfo.hpp"
)

if (BUILD_TESTS)
	add_subdirectory(tests)
endif()
