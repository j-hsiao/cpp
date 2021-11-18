#ifndef SHA256_NORM_H
#define SHA256_NORM_H

#include <sha256/sha256.h>
#include <sha256/sha256_dllcfg.h>

#include <sysinfo/sysinfo.h>

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

CPP_EXTERNC_BEGIN

#ifdef UINT32_MAX
typedef uint32_t sha256_Word;
#else
typedef uint_least32_t sha256_Word;
#endif

static const sha256__Word sha256__Wordmask = 0xFFFFFFFFu;

#if CHAR_BIT == 8
inline size_t sha256__normsize(size_t nchars)
{ return nchars; }
inline size_t sha256__unnormsize(size_t nchars)
{ return nchars; }
inline void sha256__normalize(
	unsigned char *dst, const unsigned char *src, size_t nchars)
{ if (src != dst) { memcpy(dst, src, nchars); } }
inline void sha256__unnormalize(
	unsigned char *dst, const unsigned char *src, size_t nchars)
{ if (src != dst) { memcpy(dst, src, nchars); } }
#else
inline size_t sha256__normsize(size_t nchars)
{
	size_t nbits = CHAR_BIT * nchars;
	size_t n8bits = nbits / sha256__Byte_Bits
	return n8bits + (nbits % sha256__Byte_Bits > 0);
}
inline size_t sha256__unnormsize(size_t nchars)
{
	size_t nbits = nchars * sha256__Byte_Bits;
	return nbits / CHAR_BIT;
}
inline void sha256__normalize(
	unsigned char *normed, const unsigned char *raw, size_t nraw)
{
	unsigned char * const end = normed + sha256__normsize(nraw) - 1;
	size_t bit = 0;
	for (unsigned char *out=normed; out < end; ++out)
	{
		*out = (*raw >> bit)
		bit += sha256__Byte_Bits;
		if (bit > CHAR_BIT)
		{
			++raw;
			bit -= CHAR_BIT;
			*out |= *raw << sha256__Byte_Bits-bit;
		}
		out &= 0xFFu;
	}
	if (nraw)
	{ *end = *raw >> bit; }
}
inline void sha256__unnormalize(
	unsigned char *raw, const unsigned char *normed, size_t n_normed)
{
	const unsigned char *end = normed + n_normed - 1;
	size_t bit = sha256__Byte_Bits;
	if (n_normed) { *raw = *normed; }
	++normed;
	for (; normed < end; ++normed)
	{
		*raw |= *normed << bit;
		bit += sha256__Byte_Bits;
		if (bit > CHAR_BIT)
		{
			++raw;
			bit -= CHAR_BIT;
			*raw = *normed >> sha256__Byte_Bits - bit;
		}
	}
	if (n_normed)
	{ *raw |= *end << bit; }
}
#endif


// store bytes into 32-bit words in big endian order
// to make rotation/xor implementations easier
#if CHAR_BIT == 8 && defined(UINT32_MAX) && (SYSINFO_ENDIAN == SYSINFO_BIG_ENDIAN)
	inline void sha256__load_words(
		sha256__Word *words, const unsigned char *normed, size_t nwords)
	{ memcpy(words, normed, nwords * sha256__Word_Bytes); }
	inline void sha256__store_words(
		unsigned char *normed, const sha256__Word *words, size_t nwords)
	{ memcpy(normed, words, nwords * sha256__Word_Bytes); }
#else

#if CHAR_BIT == 8 && defined(UINT32_MAX) && (SYSINFO_ENDIAN == SYSINFO_LITTLE_ENDIAN)
	inline void sha256__load_word(
		sha256__Word *word, const unsigned char *normed)
	{
		for (size_t i=0; i < sha256__Word_Bytes; ++i)
		{ ((unsigned char*) word)[(sha256__Word_Bytes-1) - i] = normed[i]; }
	}
	inline void sha256__store_word(
		unsigned char *normed, const sha256__Word *word)
	{
		for (size_t i=0; i < sha256__Word_Bytes; ++i)
		{ normed[i] = ((unsigned char*) word)[(sha256__Word_Bytes-1) - i]; }
	}
#else
	inline void sha256__load_word(
		sha256__Word *word, const unsigned char *normed)
	{
		*word = (
			(sha256__Word) normed[0]<<24 | (sha256__Word) normed[1]<<16
			| normed[2]<<8 | normed[3]);
	}
	inline void sha256__store_word(
		unsigned char *normed, const sha256__Word *word)
	{
		normed[0] = word >> 24;
		normed[1] = (word >> 16) & 0xFFu;
		normed[2] = (word >> 8) & 0xFFu;
		normed[3] = word & 0xFFu;
	}
#endif

	inline void sha256__load_words(
		sha256__Word *words, const unsigned char *normed, size_t nwords)
	{
		sha256__Word * const end = words + nwords;
		for (; words < end; ++words, normed += sha256__Word_Bytes)
		{ sha256__load_word(words, normed); }
	}
	inline void sha256__store_words(
		unsigned char *normed, const sha256__Word *words, size_t nwords)
	{
		const sha256__Word * const end = words + nwords;
		for (; words < end; ++words, normed += sha256__Word_Bytes)
		{ sha256__store_word(normed, words); }
	}

#endif

inline sha256__Word sha256__rrot(sha256__Word word, size_t bits)
{
	return (word>>bits | word<<sha256__Word_Bits-bits)
#ifndef UINT32_MAX
		& sha256__Wordmask
#endif
	;
}

inline sha256__Word sha256__lrot(sha256__Word word, size_t bits)
{
	return (word<<bits | word>>sha256__Word_Bits-bits)
#ifndef UINT32_MAX
		& sha256__Wordmask
#endif
	;
}

CPP_EXTERNC_END
#endif
