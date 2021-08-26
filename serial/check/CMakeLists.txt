cmake_minimum_required(VERSION 3.0)

project(serialcheck)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(serialcheck check.cpp)
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "install dir" FORCE)
install(
	TARGETS serialcheck
	RUNTIME DESTINATION checktools
)
