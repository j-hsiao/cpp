cmake_minimum_required(VERSION 3.1)
project(aes)
include (CTest)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
list(REMOVE_DUPLICATES CMAKE_INSTALL_RPATH)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

add_library(
	aes
	./src/plain.cpp
	./src/keys.cpp
	./src/consts.cpp
	./src/impl.cpp
)
target_include_directories(aes PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../tlog")

foreach(dep sha256 os)
	find_package(${dep} QUIET)
	if (NOT ${dep}_FOUND)
		build_dependency(${dep} "${CMAKE_CURRENT_LIST_DIR}/../${dep}")
	endif()
	find_export_package(${dep} REQUIRED)
endforeach()
target_link_libraries(
	aes
	PUBLIC sha256::sha256
	PRIVATE os::os)

#------------------------------
# link to ssl impl if found
#------------------------------
find_package(OpenSSL)
if (NOT OPENSSL_FOUND)
	set(tmp "${OPENSSL_INCLUDE_DIR}")
	unset(OPENSSL_INCLUDE_DIR CACHE)
	set(OPENSSL_INCLUDE_DIR "${tmp}" CACHE PATH "openssl include directory")

	set(tmp "${OPENSSL_CRYPTO_LIBRARY}")
	unset(OPENSSL_CRYPTO_LIBRARY CACHE)
	set(OPENSSL_CRYPTO_LIBRARY "${tmp}" CACHE PATH "openssl encryption lib")
endif()

if (OPENSSL_FOUND)
	target_sources(aes PRIVATE ./src/openssl.cpp)
	target_compile_definitions(aes PRIVATE "AES_USE_SSL=1")
	if (TARGET OpenSSL::Crypto)
		target_link_libraries(aes PRIVATE OpenSSL::Crypto)
	else()
		target_link_libraries(aes PRIVATE "${OPENSSL_CRYPTO_LIBRARY}")
		target_include_directories(aes PRIVATE "${OPENSSL_INCLUDE_DIR}")
	endif()
elseif(OPENSSL_CRYPTO_LIBRARY AND OPENSSL_INCLUDE_DIR)
	target_sources(aes PRIVATE ./src/openssl.cpp)
	target_link_libraries(aes PRIVATE "${OPENSSL_CRYPTO_LIBRARY}")
	target_include_directories(aes PRIVATE "${OPENSSL_INCLUDE_DIR}")
	target_compile_definitions(aes PRIVATE "AES_USE_SSL=1")
else()
	message("failed to find OpenSSL, set OPENSSL_INCLUDE_DIR:${OPENSSL_INCLUDE_DIR} and OPENSSL_CRYPTO_LIBRARY:${OPENSSL_CRYPTO_LIBRARY} to use ssl impl")
endif()

#------------------------------
# use x86/x64 aes intrinsics
#------------------------------

option(AES_USE_AESNI "use aesni intrinsics" ON)
option(AES_USE_SSE "use sse2/ssse3/avx2 intrinsics" ON)
if (AES_USE_AESNI)
	target_sources(aes PRIVATE ./src/aesintrin.cpp)
	target_compile_definitions(aes PRIVATE AES_USE_AESNI=1)
	if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
		target_compile_options(aes PRIVATE -maes -msse2)
	endif()
endif()

if (AES_USE_SSE)
	target_sources(aes PRIVATE ./src/sse.cpp)
	if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
		target_compile_options(aes PRIVATE -msse2 -mssse3 -mavx2 -msse4.1)
	endif()
	target_compile_definitions(aes PRIVATE AES_USE_SSE=1)
endif()
if (AES_USE_AESNI OR AES_USE_SSE)
	find_package(cpuinfo QUIET)
	if (NOT cpuinfo_FOUND)
		build_dependency(cpuinfo "${CMAKE_CURRENT_LIST_DIR}/../cpuinfo")
	endif()
	find_export_package(cpuinfo REQUIRED)
	target_link_libraries(aes PRIVATE cpuinfo::cpuinfo)
endif()


configure_dll()
install_project(include/aes)

#------------------------------
# use avx2, sse2/3, ssse3
#------------------------------
#TODO: implement this
#add_library(sse_intrin OBJECT ./src/sse.cpp)
#target_include_directories(
	#sse_intrin PRIVATE
	#"${CMAKE_CURRENT_LIST_DIR}/include"
	#"${CMAKE_CURRENT_BINARY_DIR}/include"
	#"${CMAKE_INSTALL_PREFIX}/include"
#)
#if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	#target_compile_options(sse_intrin PRIVATE -msse3 -msse2 -mssse3 -mavx2)
#endif()
#target_sources(aes PRIVATE $<TARGET_OBJECTS:sse_intrin>)

#------------------------------
# aes encrypt/decrypt tool
#------------------------------
find_package(argparse QUIET)
if (NOT argparse_FOUND)
	build_dependency(argparse "${CMAKE_CURRENT_LIST_DIR}/../argparse")
endif()
find_package(argparse REQUIRED)
find_package(timeutil QUIET)
if (NOT timeutil_FOUND)
	build_dependency(timeutil "${CMAKE_CURRENT_LIST_DIR}/../timeutil")
endif()
find_package(timeutil REQUIRED)

add_executable(aes_exe src/main.cpp)
set_target_properties(aes_exe PROPERTIES OUTPUT_NAME aes)
target_link_libraries(
	aes_exe
	PRIVATE
	aes
	sha256::sha256
	os::os
	argparse::argparse
	timeutil::timeutil
)
install(
	TARGETS aes_exe
	RUNTIME DESTINATION bin
)

if (BUILD_TESTING)

	add_executable(types tests/types.cpp)
	target_link_libraries(types PRIVATE aes)
	add_test(NAME types COMMAND types)

	add_executable(keys tests/keys.cpp)
	target_link_libraries(keys PRIVATE aes)
	add_test(NAME keys COMMAND keys)

	add_executable(plain tests/plain.cpp)
	target_link_libraries(plain PRIVATE aes timeutil::timeutil)
	add_test(NAME plain COMMAND plain)

	add_executable(aes_test tests/aes.cpp)
	target_link_libraries(aes_test PRIVATE aes timeutil::timeutil)
	add_test(NAME aes COMMAND aes_test)
endif()
