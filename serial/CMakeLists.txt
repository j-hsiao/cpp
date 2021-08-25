cmake_minimum_required(VERSION 3.0)

project(serial)
include(CTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

add_library(serial)
install_project(include/serial)

if (BUILD_TESTING)
	add_executable(serialtest ./tests/test.cpp)
	target_link_libraries(serialtest PRIVATE serial)
	add_test(NAME serial COMMAND serialtest)
endif()
