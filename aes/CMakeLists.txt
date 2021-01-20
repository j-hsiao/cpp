cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(aes)

option(ENABLE_IPO "lto" ON)
include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake")

option(AES_USE_SSLPACKAGE "use find_package to find openssl (OFF = manually specify)" ON)
option(AES_DEBUG "compile with aes debugging logging" OFF)
set(AES_FORCE "" CACHE STRING "force choosing plain B, MB, or SSL impl")
option(AES_BUILD_TESTS "build tests" ON)

add_library(
	aes
	./src/aes_plain.cpp
	./src/aes.cpp
	./src/aes_consts.cpp
	./src/aes_key.cpp)
if (NOT BUILD_SHARED_LIBS)
	target_compile_definitions(aes PUBLIC "AES_API=")
endif()
target_include_directories(
	aes PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

#------------------------------
# link to openssl if found
#------------------------------
if (AES_USE_SSLPACKAGE)
	find_package(OpenSSL)
else()
	set(OPENSSL_FOUND "")
endif()
if (NOT OPENSSL_FOUND)
	message("failed to find openssl")
	find_library(OPENSSL_CRYPTO_LIBRARY crypto)
	get_property(osslcached CACHE OPENSSL_INCLUDE_DIR PROPERTY TYPE)
	if (NOT osslcached)
		unset(OPENSSL_INCLUDE_DIR CACHE)
		set(OPENSSL_INCLUDE_DIR "" CACHE PATH "openssl include directory")
	endif()
endif()
if (OPENSSL_INCLUDE_DIR AND OPENSSL_CRYPTO_LIBRARY)
	message("linking to openssl libcrypto")
	target_compile_definitions(aes PRIVATE "AES_WITH_SSL")
	target_sources(aes PRIVATE ./src/aes_ssl.cpp)
	target_link_libraries(aes PRIVATE "${OPENSSL_CRYPTO_LIBRARY}")
	target_include_directories(aes PRIVATE "${OPENSSL_INCLUDE_DIR}")
else()
	message("to use OpenSSL, please specify OPENSSL_INCLUDE_DIR and OPENSSL_CRYPTO_LIBRARY")
endif()

#------------------------------
# dispatch to intrinsics if available
#------------------------------
if (AES_FORCE)
	target_compile_definitions(aes PRIVATE "AES_FORCE_${AES_FORCE}")
else()
	add_library(aes_aes OBJECT ./src/aes_aes.cpp)
	if (NOT BUILD_SHARED_LIBS)
		target_compile_definitions(aes_aes PUBLIC "AES_API=")
	endif()
	target_include_directories(aes_aes PRIVATE ./include)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(aes_aes PRIVATE -maes -msse2)
	endif()
	target_sources(aes PRIVATE $<TARGET_OBJECTS:aes_aes>)
	#need cpuinfo to dispatch if not force
	if (NOT TARGET cpuinfo)
		add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../cpuinfo" "cpuinfo")
	endif()
	target_link_libraries(aes PRIVATE cpuinfo)
endif()

if (AES_DEBUG)
	target_compile_definitions(aes PRIVATE "AES_DEBUG")
endif()

include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
	target_compile_definitions(aes PRIVATE BIG_ENDIAN)
else()
	target_compile_definitions(aes PRIVATE LITTLE_ENDIAN)
endif()

if (NOT TARGET argparse)
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../argparse" argparse)
endif()
add_executable(aestool ./src/main.cpp)
target_link_libraries(aestool PRIVATE aes argparse)
target_include_directories(aestool PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

install(
	TARGETS
		aes
	EXPORT
		aesTargets
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
)
install(
	TARGETS
		aestool
	RUNTIME DESTINATION bin
)
install(
	FILES
		"${CMAKE_CURRENT_LIST_DIR}/include/aes/aes.hpp"
	DESTINATION
		include/aes
)

# this creates a .cmake file that
# you can include to link to the headers/binaries
# built within the build tree
# otherwise, this does nothing
# docs say something about crosscompiling or something
# not really sure what that's all about
export(
	EXPORT aesTargets
	FILE "${CMAKE_BINARY_DIR}/exports/aes/aesExports.cmake"
	NAMESPACE aes::
)
# add the aesConfig.cmake to DESTINATION
# (after install, use this to link to lib)
install(
	EXPORT aesTargets
	DESTINATION cmake
	NAMESPACE aes::
	FILE aesConfig.cmake
)

#------------------------------
# tests
#------------------------------
if (AES_BUILD_TESTS)
	if (NOT TARGET timeutil)
		add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../timeutil" timeutil)
	endif()

	add_executable(
		aes_testkeys_b
		./tests/test_keys.cpp
		./src/aes_key.cpp
		./src/aes_consts.cpp)
	target_include_directories(
		aes_testkeys_b
		PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/include)
	target_compile_definitions(
		aes_testkeys_b PRIVATE AES_FORCE_B AES_DEBUG "AES_API=")


	add_executable(
		aes_testkeys_mb
		./tests/test_keys.cpp
		./src/aes_key.cpp
		./src/aes_consts.cpp)
	target_include_directories(
		aes_testkeys_mb
		PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/include)
	target_compile_definitions(
		aes_testkeys_mb PRIVATE AES_FORCE_MB AES_DEBUG "AES_API=")

	add_executable(
		aes_testplain
		./tests/test_plain.cpp
		./src/aes_plain.cpp
		./src/aes_key.cpp
		./src/aes_consts.cpp)
	target_include_directories(
		aes_testplain
		PRIVATE
		${CMAKE_CURRENT_LIST_DIR}/include)
	target_compile_definitions(
		aes_testplain PRIVATE AES_DEBUG "AES_API=")
	if (AES_FORCE)
		target_compile_definitions(
			aes_testplain PRIVATE AES_FORCE_${AES_FORCE})
	endif()

	add_executable(aes_test ./tests/test_aes.cpp)
	target_link_libraries(aes_test PRIVATE aes timeutil)

	install(
		TARGETS
			aes_test
			aes_testkeys_mb
			aes_testkeys_b
		RUNTIME DESTINATION tests
	)
endif()
