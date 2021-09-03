cmake_minimum_required(VERSION 3.0)

project(sysinfo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("CMAKE_INSTALL_PREFIX is \"${CMAKE_INSTALL_PREFIX}\"")
add_executable(sysinfo check.cpp)
install(
	TARGETS sysinfo
	RUNTIME DESTINATION bin
)
