cmake_minimum_required(VERSION 3.0)

project(gittok VERSION 1.0)
include("../common.cmake" NO_POLICY_SCOPE)
include(../tlog/tlog.cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

foreach(dep os argparse sha256 aes)
	add_package_or_subdir(
		packages "${CMAKE_CURRENT_LIST_DIR}/../${dep}" "${dep}" 1.0)
endforeach()

make_tlog(gittok GITTOK_DEBUG)
option(GITTOK_DEBUG "debugging" OFF)
add_executable(gittok ./src/gittok.cpp)
target_include_directories(
	gittok PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include")
target_compile_definitions(
	gittok PRIVATE "GITTOK_DEBUG=$<BOOL:${GITTOK_DEBUG}>")
target_link_libraries(
	gittok
	PRIVATE
		aes::aes
		os::os
		argparse::argparse
		sha256::sha256
)

default_install(
	TARGETS gittok
	RUNTIME DESTINATION bin)
