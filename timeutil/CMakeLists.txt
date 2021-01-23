cmake_minimum_required (VERSION 3.0)
project(timeutil)

include(CTest)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

find_export_package(Threads MODULE)

add_library(timeutil INTERFACE)
target_link_libraries(timeutil INTERFACE Threads::Threads)
install_project(include/timeutil)

if (BUILD_TESTING)
	add_executable(t_timeutil tests/timeutil.cpp)
	target_link_libraries(t_timeutil PRIVATE timeutil)

	add_test(NAME timeutil COMMAND t_timeutil)
endif()
