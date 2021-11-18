cmake_minimum_required (VERSION 3.0)

project(cpuinfo VERSION 1.0)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CPUINFO_DEBUG "build with debug logs?" OFF)

if (MSVC)
	add_library(cpuinfo ./src/cpuinfo_win.cpp)
else()
	add_library(cpuinfo ./src/cpuinfo_gnu.cpp)
endif()
add_includes(cpuinfo PUBLIC BUILD include INSTALL include)
target_compile_definitions(
	cpuinfo
	PRIVATE
	"CPUINFO_DEBUG=$<BOOL:${CPUINFO_DEBUG}>"
)
configure_dll(cpuinfo)
default_install(TARGETS)
default_install(EXPORT)
install_configfile()
default_install(DIRECTORY include/cpuinfo DESTINATION include)

add_aliases(cpuinfo cpuinfo)

include(../tlog/tlog.cmake)
make_tlog(cpuinfo CPUINFO_DEBUG)

add_executable(getcpuinfo ./src/main.cpp)
target_link_libraries(getcpuinfo PRIVATE cpuinfo)
