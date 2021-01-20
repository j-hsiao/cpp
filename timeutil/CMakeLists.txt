cmake_minimum_required (VERSION 3.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(timeutil)

option(ENABLE_IPO "lto" ON)
include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake")

add_library(timeutil INTERFACE)
target_include_directories(
	timeutil
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
	$<INSTALL_INTERFACE:include>
)
install(
	TARGETS timeutil
	EXPORT timeutilTargets
)
install(
	FILES
		"${CMAKE_CURRENT_LIST_DIR}/include/timeutil/timeutil.hpp"
	DESTINATION
		include/timeutil
)

install(
	EXPORT
		timeutilTargets
	DESTINATION cmake
	NAMESPACE timeutil::
	FILE timeutilConfig.cmake)
