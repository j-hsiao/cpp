#ifndef SHA256_H
#define SHA256_H
#include <sha256/sha256_dllcfg.h>
#include <stddef.h>
#include <sha256/types.h>

CPP_EXTERNC_BEGIN

static const size_t sha256__Hash_Bits = 256;
static const size_t sha256__Hash_Bytes = sha256__Hash_Bits / sha256__Byte_Bits;

SHA256_API void sha256__hash(
	sha256__Byte dst[sha256__Hash_Bytes], const sha256__Byte *src, size_t numBytes);

CPP_EXTERNC_END
#endif
