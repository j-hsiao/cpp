cmake_minimum_required(VERSION 3.0)

project(sysinfo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sysinfo_exe check.cpp)
set_target_properties(
	sysinfo_exe PROPERTIES OUTPUT_NAME "sysinfo")

install(
	TARGETS sysinfo_exe
	RUNTIME DESTINATION bin
)
