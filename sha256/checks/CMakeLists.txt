cmake_minimum_required(VERSION 3.0)

project(endiancheck)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(endiancheck endian.cpp)
set_target_properties(
	endiancheck
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "install dir" FORCE)

install(
	TARGETS endiancheck
	RUNTIME DESTINATION checktools
)
