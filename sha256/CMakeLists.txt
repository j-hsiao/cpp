cmake_minimum_required(VERSION 3.0)

project(sha256 VERSION 1.0)
include(CTest)
include(../common.cmake NO_POLICY_SCOPE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_package_or_subdir(packages ../serial serial 1.0)
add_package_or_subdir(packages ../sysinfo sysinfo 1.0)
add_library(sha256 src/sha256.cpp)
target_link_libraries(sha256 PRIVATE serial::serial INTERFACE sysinfo::sysinfo)
configure_dll(sha256)
add_includes(sha256 PUBLIC BUILD include INSTALL include)

add_package_or_subdir(ignored ../argparse argparse 1.0)
add_executable(sha256_exe src/main.cpp)
target_link_libraries(sha256_exe PRIVATE sha256 argparse::argparse)
set_target_properties(
	sha256_exe PROPERTIES
	RUNTIME_OUTPUT_NAME sha256)

default_install(TARGETS sha256 sha256_exe)
default_install(EXPORT)
install_configfile(DEPENDS "${packages}")
default_install(DIRECTORY include/ DESTINATION include)
add_aliases(sha256 sha256)

if (BUILD_TESTING)
	add_executable(testsha256 tests/sha256.cpp)
	target_link_libraries(testsha256 PRIVATE sha256)
	add_test(NAME sha256 COMMAND testsha256)
endif()
