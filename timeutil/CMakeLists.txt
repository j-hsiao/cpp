cmake_minimum_required (VERSION 3.0)
project(timeutil VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
include(../common.cmake NO_POLICY_SCOPE)

add_package(packages Threads MODULE)

add_library(timeutil INTERFACE)
target_link_libraries(timeutil INTERFACE Threads::Threads)
add_includes(timeutil INTERFACE BUILD include INSTALL include)

default_install(TARGETS)
default_install(EXPORT)
install_configfile(DEPENDS "${packages}")
default_install(DIRECTORY include/timeutil DESTINATION include)

add_aliases(timeutil timeutil)

if (BUILD_TESTING)
	add_executable(t_timeutil tests/timeutil.cpp)
	target_link_libraries(t_timeutil PRIVATE timeutil)
	add_test(NAME timeutil COMMAND t_timeutil)
endif()
