cmake_minimum_required(VERSION 3.0)

project(gittok)
include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(
	APPEND CMAKE_PREFIX_PATH
	"${CMAKE_INSTALL_PREFIX}/cmake"
	"${CMAKE_INSTALL_PREFIX}/lib/cmake"
)

foreach(dep os argparse sha256 aes)
	find_package(${dep} QUIET)
	if (NOT ${dep}_FOUND)
		build_dependency(${dep} "${CMAKE_CURRENT_LIST_DIR}/../${dep}")
	endif()
	find_export_package(${dep} REQUIRED)
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
