cmake_minimum_required(VERSION 3.1)
project(aes VERSION 1.0)
include(CTest)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_CURRENT_LIST_DIR}/../common.cmake" NO_POLICY_SCOPE)

add_library(aes src/aes.cpp src/ssl.cpp src/sslaes.cpp src/sse.cpp src/aesni.cpp src/uil32.cpp src/ui32.cpp src/raw.cpp src/defs.cpp)
configure_dll(aes)
add_includes(
	aes PUBLIC
	BUILD include "${CMAKE_CURRENT_BINARY_DIR}/include"
	INSTALL include)

option(AES_USE_PLAIN "add plain unsigned char impl" OFF)
option(AES_USE_AESNI "use aesni intrinsics" ON)
option(AES_USE_SSE "use sse2/ssse3/avx2 intrinsics" OFF)
option(AES_UI32_BYTEFIRST "use byte-first implementation, smaller cpu cache = faster?" OFF)
option(AES_UI32_PASSTMP "pass tmp vs pointer vs allocate on stack, sometimes faster sometimes slower" OFF)
option(AES_UI32_INTO "dec/enclast is non-inplace = no need to swap tmp/working buffers in cbc (requires PASSTMP)" ON)
option(AES_UIL32_BYTEFIRST "use byte-first implementation, smaller cpu cache = faster?" ON)
option(AES_UIL32_PASSTMP "pass tmp vs pointer vs allocate on stack, sometimes faster sometimes slower" ON)
option(AES_UIL32_INTO "dec/enclast is non-inplace = no need to swap tmp/working buffers in cbc (requires PASSTMP)" ON)
set(AES_FALLBACK "Impl_RawUIL32" CACHE STRING "name of fallback to use")
set_property(CACHE AES_FALLBACK PROPERTY STRINGS "Impl_RawUIL32" "Impl_RawUI32")
target_compile_definitions(
	aes PRIVATE
	"AES_USE_AESNI=$<BOOL:${AES_USE_AESNI}>"
	"AES_USE_SSE=$<BOOL:${AES_USE_SSE}>"
	"AES_UIL32_BYTEFIRST=$<BOOL:${AES_UIL32_BYTEFIRST}>"
	"AES_UI32_BYTEFIRST=$<BOOL:${AES_UI32_BYTEFIRST}>"
	"AES_UIL32_PASSTMP=$<BOOL:${AES_UIL32_PASSTMP}>"
	"AES_UI32_PASSTMP=$<BOOL:${AES_UI32_PASSTMP}>"
	"AES_UIL32_INTO=$<BOOL:${AES_UIL32_INTO}>"
	"AES_UI32_INTO=$<BOOL:${AES_UI32_INTO}>"
	"AES_FALLBACK=${AES_FALLBACK}"
)

configure_file(src/plain.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/include/aes/plain.hpp")
if (AES_USE_PLAIN)
	target_sources(aes PRIVATE src/plain.cpp)
endif()
set(INTRINSICS_FLAGS "")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if (AES_USE_AESNI)
		list(APPEND INTRINSICS_FLAGS -msse2 -maes)
	endif()
	if (AES_USE_SSE)
		list(APPEND INTRINSICS_FLAGS -msse2 -mssse3 -mavx2)
	endif()
endif()
if (AES_USE_AESNI OR AES_USE_SSE)
	add_package_or_subdir(packages ../cpuinfo cpuinfo 1.0)
	target_link_libraries(aes PRIVATE cpuinfo::cpuinfo)
endif()
if (INTRINSICS_FLAGS)
	target_compile_options(aes PRIVATE ${INTRINSICS_FLAGS})
endif()

option(AES_USE_SSL "use ssl library" ON)
if (AES_USE_SSL)
	add_package(packages OpenSSL)
	message("linking to OpenSSL crypto")
	if (OPENSSL_FOUND)
		target_compile_definitions(aes PRIVATE AES_USE_SSL=1)
		if (TARGET OpenSSL::Crypto)
			target_link_libraries(aes PRIVATE OpenSSL::Crypto)
		else()
			target_include_directories(aes PRIVATE "${OPENSSL_INCLUDE_DIR}")
			if(OPENSSL_CRYPTO_LIBRARY)
				target_link_libraries(aes PRIVATE "${OPENSSL_CRYPTO_LIBRARY}")
			else()
				target_link_libraries(aes PRIVATE ${OPENSSL_LIBRARIES})
			endif()
		endif()
	elseif(OPENSSL_CRYPTO_LIBRARY AND OPENSSL_INCLUDE_DIR)
		target_compile_definitions(aes PRIVATE AES_USE_SSL=1)
		target_link_libraries(aes PRIVATE "${OPENSSL_CRYPTO_LIBRARY}")
		target_include_directories(aes PRIVATE "${OPENSSL_INCLUDE_DIR}")
	else()
		message(
			"could not find OpenSSL, set OPENSSL_CRYPTO_LIBRARY"
			" and OPENSSL_INCLUDE_DIR to use")
		message("OPENSSL_CRYPTO_LIBRARY: ${OPENSSL_CRYPTO_LIBRARY}")
		message("OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
		target_compile_definitions(aes PRIVATE AES_USE_SSL=0)
	endif()
else()
	message("skipping OpenSSL")
	target_compile_definitions(aes PRIVATE AES_USE_SSL=0)
endif()


add_executable(aestool src/aestool.cpp)

add_package_or_subdir(ignored ../argparse argparse 1.0)
add_package_or_subdir(ignored ../os os 1.0)
add_package_or_subdir(ignored ../sha256 sha256 1.0)
target_link_libraries(aestool PRIVATE aes argparse::argparse os::os sha256::sha256)


default_install(TARGETS aes aestool)
default_install(EXPORT)
default_install(
	DIRECTORY include/aes "${CMAKE_CURRENT_BINARY_DIR}/include/aes"
	DESTINATION include)
install_configfile(DEPENDS "${packages}")
add_aliases(aes aes)



if (BUILD_TESTING)
	add_executable(testarr tests/array.cpp)
	target_include_directories(testarr PRIVATE include)
	target_compile_definitions(testarr PRIVATE arr_normal arr_crtp arr_adl arr_sfinae)

	add_executable(testarrnormal tests/array.cpp)
	target_include_directories(testarrnormal PRIVATE include)
	target_compile_definitions(testarrnormal PRIVATE arr_normal)

	add_executable(testarrcrtp tests/array.cpp)
	target_include_directories(testarrcrtp PRIVATE include)
	target_compile_definitions(testarrcrtp PRIVATE arr_crtp)

	add_executable(testarradl tests/array.cpp)
	target_include_directories(testarradl PRIVATE include)
	target_compile_definitions(testarradl PRIVATE arr_adl)

	add_executable(testarrsfinae tests/array.cpp)
	target_include_directories(testarrsfinae PRIVATE include)
	target_compile_definitions(testarrsfinae PRIVATE arr_sfinae)

	add_custom_target(
		array DEPENDS testarr testarrnormal testarrcrtp testarradl testarrsfinae
		 COMMAND testarr
		 COMMAND testarrnormal
		 COMMAND testarrcrtp
		 COMMAND testarradl
		 COMMAND testarrsfinae
	)

	add_executable(testkeys tests/keys.cpp)
	target_link_libraries(testkeys PRIVATE aes)
	add_test(NAME keys COMMAND testkeys)

	add_executable(testaes tests/aes.cpp)
	target_link_libraries(testaes PRIVATE aes)
	add_test(NAME aes COMMAND testaes)

	foreach(item IN ITEMS testaes testkeys)
		target_compile_definitions(${item} PRIVATE "TESTING_DLL=$<BOOL:${BUILD_SHARED_LIBS}>")
	endforeach()

# 	add_executable(testcompat tests/aescompat.cpp)
# 	target_link_libraries(testcompat PRIVATE aes hexlog)
# 	add_test(NAME aescompat COMMAND testcompat)
endif()


# 
# 
# add_library(aes src/aes.cpp)
# add_includes(
# 	aes PUBLIC
# 	BUILD include ${CMAKE_CURRENT_BINARY_DIR}/include
# 	INSTALL include)
# 
# configure_dll()
# install_project(
# 	${CMAKE_CURRENT_LIST_DIR}/include/aes/aes.h
# 	${CMAKE_CURRENT_LIST_DIR}/include/aes/aes.hpp
# )
# 
# if (BUILD_TESTING)
# 	add_executable(testkeys tests/keys.cpp)
# 	target_link_libraries(testkeys PRIVATE aesinline)
# 	add_test(NAME keys COMMAND testkeys)
# 
# 	add_executable(testaesinline tests/aesinline.cpp)
# 	target_link_libraries(testaesinline PRIVATE aesinline)
# 	add_test(NAME aesinline COMMAND testaesinline)
# 
# 	add_executable(testaes tests/aes.cpp)
# 	target_link_libraries(testaes PRIVATE aes)
# 	add_test(NAME aes COMMAND testaes)
# endif()
