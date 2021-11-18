cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(os VERSION 1.0)

include(CTest)
include(../common.cmake NO_POLICY_SCOPE)
include(../tlog/tlog.cmake)

add_library(os ./src/os.cpp)
option(OS_DEBUG "debug os" OFF)
make_tlog(os OS_DEBUG)
configure_dll(os)
add_includes(os PUBLIC BUILD include INSTALL include)
target_include_directories(os PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
target_compile_definitions(os PRIVATE "OS_DEBUG=$<BOOL:${OS_DEBUG}>")
if (MINGW)
	#add_link_options(os PUBLIC -luuid -lole32)
	target_link_libraries(os PUBLIC ole32)
endif()
default_install(TARGETS os)
default_install(EXPORT)
default_install(DIRECTORY include/os DESTINATION include)
install_configfile()
add_aliases(os os)


if (BUILD_TESTING)
	add_executable(wchar tests/wchar.cpp)
	add_package_or_subdir(ignore ../argparse argparse)
	add_package_or_subdir(ignore ../timeutil timeutil)

	add_executable(ftype tests/ftype.cpp)
	target_link_libraries(ftype PRIVATE os)

	add_executable(getenv tests/getenv.cpp)
	target_link_libraries(getenv PRIVATE os)

	add_executable(hide tests/hide.cpp)
	target_link_libraries(hide PRIVATE os timeutil::timeutil)

	add_executable(ls tests/ls.cpp)
	target_link_libraries(ls PRIVATE os argparse::argparse)

	add_executable(mkdir tests/mkdir.cpp)
	target_link_libraries(mkdir PRIVATE os)

	add_executable(mv tests/mv.cpp)
	target_link_libraries(mv PRIVATE os argparse::argparse)

	add_executable(paths tests/path.cpp)
	target_link_libraries(paths PRIVATE os)
	add_test(NAME paths COMMAND paths)

	add_executable(rm tests/rm.cpp)
	target_link_libraries(rm PRIVATE os argparse::argparse)

	add_executable(sysdir tests/sysdir.cpp)
	target_link_libraries(sysdir PRIVATE os)

	add_executable(term tests/term.cpp)
	target_link_libraries(term PRIVATE os argparse::argparse)

	foreach(item IN ITEMS ftype getenv hide ls mkdir mv paths rm sysdir term)
		target_compile_definitions("${item}" PRIVATE "TESTING_DLL=$<BOOL:${BUILD_SHARED_LIBS}>")
	endforeach()

endif()
