cmake_minimum_required(VERSION 3.0)

project(serial VERSION 1.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
include(../common.cmake NO_POLICY_SCOPE)
include(../tlog/tlog.cmake)


add_library(serial src/serial.cpp)
add_includes(serial PUBLIC BUILD include INSTALL include)
option(
	SERIAL_USE_CPOPT
	"Use sysinfo to use memcpy/byte copies instead of calculations, though in testing, this was actually slower, maybe there's some optimization associated with bitshift+store"
	OFF)
if(SERIAL_USE_CPOPT)
	add_package_or_subdir(packages ../sysinfo sysinfo 1.0)
	target_link_libraries(serial PUBLIC sysinfo::sysinfo)
	target_compile_definitions(serial PUBLIC SERIAL_USE_CPOPT=1)
endif()
configure_dll(serial)
make_tlog("" SERIAL_DEBUG)
option(SERIAL_DEBUG "serial debugging" OFF)
target_compile_definitions(
	serial PUBLIC "SERIAL_DEBUG=$<BOOL:${SERIAL_DEBUG}>")

default_install(TARGETS)
default_install(EXPORT)
default_install(DIRECTORY include/ DESTINATION include)
install_configfile(DEPENDS "${packages}")

add_aliases(serial serial)

if (BUILD_TESTING)
	add_package_or_subdir(ignored ../timeutil timeutil 1.0)
	add_executable(serialtest ./src/test.cpp)
	target_link_libraries(serialtest PRIVATE serial timeutil::timeutil)
	add_test(NAME serial COMMAND serialtest)
endif()
