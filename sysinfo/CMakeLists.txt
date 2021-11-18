cmake_minimum_required(VERSION 3.3)

project(sysinfo VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
include(../common.cmake NO_POLICY_SCOPE)

add_executable(header_generator check.cpp)
set(generated_header "${CMAKE_CURRENT_BINARY_DIR}/include/sysinfo/sysinfo.h")
add_custom_command(
	OUTPUT "${generated_header}"
	COMMAND
		"${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/include/sysinfo"
	COMMAND
		header_generator "${generated_header}"
	DEPENDS header_generator)
add_custom_target(
	generate_headers
	ALL
	DEPENDS "${generated_header}")

add_library(sysinfo INTERFACE)
add_includes(sysinfo INTERFACE BUILD "${CMAKE_CURRENT_BINARY_DIR}/include" INSTALL include)
add_dependencies(sysinfo generate_headers)

add_aliases(sysinfo sysinfo)
default_install(TARGETS)
default_install(EXPORT)
install_configfile()
default_install(
	FILES "${generated_header}"
	DESTINATION include/sysinfo)


if (BUILD_TESTING)
	add_executable(sysinfo_test src/t.cpp)
	target_link_libraries(sysinfo_test PRIVATE sysinfo)
	add_test(NAME sysinfo COMMAND sysinfo_test)
endif()
