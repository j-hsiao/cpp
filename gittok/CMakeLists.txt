cmake_minimum_required(VERSION 3.0)

project(gittok)
include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

foreach(dep os argparse sha256 aes)
	find_or_build_dep("${dep}" "${CMAKE_CURRENT_LIST_DIR}/../${dep}")
endforeach()


option(GITTOK_DEBUG "debugging" OFF)
add_executable(gittok ./src/gittok.cpp)
target_include_directories(
	gittok
	PRIVATE
		../tlog)
if (GITTOK_DEBUG)
	target_compile_definitions(gittok PRIVATE GITTOK_DEBUG=1)
endif()
target_link_libraries(
	gittok
	PRIVATE
		aes::aes
		os::os
		argparse::argparse
		sha256::sha256
)

install(
	TARGETS gittok
	RUNTIME DESTINATION bin)
