cmake_minimum_required(VERSION 3.0)
project(argparse VERSION 1.0)
include(CTest)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../common.cmake NO_POLICY_SCOPE)

add_library(argparse INTERFACE)
add_includes(argparse INTERFACE BUILD include INSTALL include)

default_install(TARGETS)
default_install(EXPORT)
install_configfile()
default_install(DIRECTORY include/argparse DESTINATION include)
add_aliases(argparse argparse)

if (BUILD_TESTING)
	add_executable(argtest tests/test.cpp)
	target_link_libraries(argtest PRIVATE argparse)
endif()
