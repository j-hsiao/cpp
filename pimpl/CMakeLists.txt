cmake_minimum_required(VERSION 3.0)

project(pimpl)

option(ENABLE_IPO "lto" ON)
include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake")

add_library(pimpl INTERFACE)
target_include_directories(
	pimpl INTERFACE
	$<BUILD_INTERFACE:"${CMAKE_CURRENT_LIST_DIR}/include">
	$<INSTALL_INTERFACE:include>
)

install(
	TARGETS pimpl
	EXPORT pimpl_targets
)
install(
	DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/pimpl"
	DESTINATION include
)
install(
	EXPORT pimpl_targets
	FILE pimplConfig.cmake
	DESTINATION cmake
	NAMESPACE pimpl::
)

if (BUILD_TESTS)
	add_executable(pimpltest ./test.cpp)
	target_link_libraries(pimpltest PRIVATE pimpl)
	install(
		TARGETS pimpltest
		RUNTIME DESTINATION tests
	)
endif()
