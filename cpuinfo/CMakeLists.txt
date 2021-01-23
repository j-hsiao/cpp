cmake_minimum_required (VERSION 3.0)

project(cpuinfo)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CPUINFO_DEBUG "build with debug logs?" OFF)

if (MSVC OR MINGW)
	add_library(cpuinfo ./src/cpuinfo_win.cpp)
else()
	add_library(cpuinfo ./src/cpuinfo_gnu.cpp)
endif()
target_compile_definitions(
	cpuinfo
	PRIVATE
	"CPUINFO_DEBUG=$<BOOL:${CPUINFO_DEBUG}>"
)
configure_dll()
install_project(include/cpuinfo)
target_include_directories(cpuinfo PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../tlog")

add_executable(getcpuinfo ./src/main.cpp)
target_link_libraries(getcpuinfo PRIVATE cpuinfo)
if (BUILD_TESTING)
	install(
		TARGETS getcpuinfo
		RUNTIME DESTINATION tests
	)
endif()
