#ifndef AES_TYPES_H
#define AES_TYPES_H

#include <aes/aes.h>

#if CHAR_BIT != 8 || !defined(UINT32_MAX)
#define AES_COMPATIBILITY_MODE
#endif

namespace aes
{
	static const aes__Byte ByteMask = 0xFF;
	static const aes__Word WordMask = 0xFFFFFFFF;

	inline aes__Byte mul2(aes__Byte b);
	inline aes__Byte mul(aes__Byte b, size_t v);

	//size of dst should be numBytes + aes__padding(numBytes)
	inline void load_words(aes__Word *dst, const aes__Byte *src, size_t numBytes);
	inline void store_words(aes__Byte *dst, const aes__Word *src, size_t numBytes);

	inline aes__Byte get_byte(aes__Word w, size_t num);
	inline void set_byte(aes__Word &w, aes__Byte val, size_t num);
	inline aes__Word lrot(aes__Word w, size_t numbits);
	inline aes__Word rrot(aes__Word w, size_t numbits);

	inline aes__Byte get_state_byte(const aes__Word *w, size_t num);
	inline void set_state_byte(aes__Word *w, aes__Byte val, size_t num);
	inline void xor_state(aes__Word *state1, const aes__Word *state2);
	inline void xor_state(
		aes__Word *dst,
		const aes__Word *state1, const aes__Word *state2);
}

#include <.aes/types_impl.hpp>
#endif
