cmake_minimum_required(VERSION 3.0)

project(sysinfo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sysinfo check.cpp)
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "install dir" FORCE)
install(
	TARGETS sysinfo
	RUNTIME DESTINATION bin
)
