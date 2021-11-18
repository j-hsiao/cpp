#ifndef SHA256_H
#define SHA256_H
#include <sha256/sha256_dllcfg.h>

#include <stddef.h>

CPP_EXTERNC_BEGIN

static const size_t sha256_Byte_Bits = 8;
static const size_t sha256_Word_Bits = 32;
static const size_t sha256_Hash_Bits = 256;
static const size_t sha256_Word_Bytes = sha256_Word_Bits / sha256_Byte_Bits;
static const size_t sha256_Hash_Bytes = sha256_Hash_Bits / sha256_Byte_Bits;
static const size_t sha256_Hash_Words = sha256_Hash_Bits / sha256_Word_Bits;

SHA256_API void sha256_hash(void *dst, const void *src, size_t numBytes);
CPP_EXTERNC_END

#endif
