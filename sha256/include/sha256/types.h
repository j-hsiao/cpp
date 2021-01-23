//standard types for sha256, conversions, operations
#ifndef SHA256_TYPES_H
#define SHA256_TYPES_H

#include <sha256/sha256_dllcfg.h>

#include <stddef.h>
#include <limits.h>
#include <stdint.h>

CPP_EXTERNC_BEGIN

static const size_t sha256__Byte_Bits = 8;
static const size_t sha256__Word_Bits = 32;
static const size_t sha256__Word_Bytes = sha256__Word_Bits / sha256__Byte_Bits;

//sha256__Byte is used to emphasize
//that the values in the unsigned char <= SHA256_Byte_Mask
//if user can guarantee values are <= 0xFF
//then can directly use unsigned char* as sha256__Byte*
typedef unsigned char sha256__Byte;
#ifdef UINT32_MAX
typedef uint32_t sha256__Word;
#else
typedef uint_least32_t sha256__Word;
#endif

static const sha256__Byte sha256__Byte_Mask = 0xFFu;
static const sha256__Word sha256__Word_Mask = 0xFFFFFFFFu;

//"normalize" unsigned char buffers so that each unsigned char contains 8 bits
//most significant bits are stored in the least significant bits of the next unsigned char
//lastly, padding is added
inline size_t sha256__normalized_size(size_t numChars);
inline size_t sha256__unnormalized_size(size_t numBytes);
inline sha256__Byte* sha256__normalize(
	sha256__Byte *normed, const unsigned char *raw, size_t numChars);
inline unsigned char* sha256__unnormalize(
	unsigned char *raw, const sha256__Byte *normed, size_t numChars);

//store 32 bits of data in each uint_least32_t
//sha256__Word should have size numChars / 4 rounded up
inline void sha256__load_words(
	sha256__Word *words, const sha256__Byte *normed, size_t numBytes);
inline void sha256__store_words(
	sha256__Byte *normed, const sha256__Word *words, size_t numBytes);
//get sha256__Byte of the word (0 = most sig, 3 = least sig)
//value must be between 0 and 3
inline sha256__Byte sha256__get_byte(sha256__Word word, size_t num);
inline void sha256__set_byte(sha256__Word *word, sha256__Byte val, size_t num);

//bits should be between 0 and sha256__Word_Bits inclusive
inline sha256__Word sha256__rrot(sha256__Word word, size_t bits);
inline sha256__Word sha256__lrot(sha256__Word word, size_t bits);

//buf should have size 2 * nbytes
inline void sha256__to_hex(char *buf, const void *data, size_t nbytes);
inline int sha256__from_hex(void *data, const char *buf, size_t nbytes);

CPP_EXTERNC_END

#include <sha256/types_impl.h>

#endif
