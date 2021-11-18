cmake_minimum_required(VERSION 3.0)

project(pimpl)
include(CTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

add_library(pimpl INTERFACE)
install_project(include/pimpl)



if (BUILD_TESTING)
	add_executable(pimpltest ./tests/test.cpp)
	target_link_libraries(pimpltest PRIVATE pimpl)
	add_test(NAME pimpl COMMAND pimpltest)
endif()
