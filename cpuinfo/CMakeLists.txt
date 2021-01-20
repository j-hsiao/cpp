cmake_minimum_required (VERSION 3.0)

project(cpuinfo)

option(ENABLE_IPO "lto" ON)
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

target_include_directories(
	cpuinfo PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
	$<INSTALL_INTERFACE:include>)
if (NOT BUILD_SHARED_LIBS)
	target_compile_definitions(cpuinfo PUBLIC "CPUINFO_API=")
endif()
if (CPUINFO_DEBUG)
	target_compile_definitions(cpuinfo PRIVATE CPUINFO_DEBUG)
endif()

add_executable(cpuinfotest ./src/main.cpp)
target_link_libraries(cpuinfotest PRIVATE cpuinfo)

install(
	TARGETS
		cpuinfo
	EXPORT
		cpuinfoTargets
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
)
install(
	FILES
		"${CMAKE_CURRENT_LIST_DIR}/include/cpuinfo/cpuinfo.hpp"
	DESTINATION
		include/cpuinfo
)

if (BUILD_TESTS)
	install(
		TARGETS
			cpuinfotest
		RUNTIME DESTINATION tests
	)
endif()

message("exporting cpuinfo")
export(
	EXPORT cpuinfoTargets
	FILE "${CMAKE_BINARY_DIR}/exports/cpuinfo/cpuinfoExports.cmake"
	NAMESPACE cpuinfo::
)
message("cpuinfo exported")
install(
	EXPORT cpuinfoTargets
	DESTINATION cmake
	NAMESPACE cpuinfo::
	FILE cpuinfoConfig.cmake
)
