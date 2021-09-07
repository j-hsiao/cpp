#ifndef AES_TYPES_IMPL_H
#define AES_TYPES_IMPL_H
#include <sha256/types.h>

#include <sysinfo/sysinfo.h>
#include <algorithm>
#include <climits>
#include <stdexcept>

#include <ostream>
namespace aes
{
	inline aes__Byte mul2(aes__Byte b)
	{
	#if CHAR_BIT == 8
		return (b << 1) ^ ((((char)(b)) >> 7) & 0x1b);
	#else
		return ((b << 1) ^ ((((char)(b)) >> 7) & 0x1b)) & aes__Byte_Mask;
	#endif
	}
	inline aes__Byte mul(aes__Byte b, size_t v)
	{
		aes__Byte ret = 0;
		while (v)
		{
			if (v & 0x01)
			{ ret ^= b; }
			v >>= 1;
			b = mul2(b);
		}
		return ret;
	}

#ifndef AES_COMPATIBILITY_MODE
	inline void load_words(aes__Word *dst, const aes__Byte *src, size_t numBytes)
	{
		aes__Byte *d = reinterpret_cast<aes__Byte*>(dst);
		std::copy(src, src + numBytes, d);
	}
	inline void store_words(aes__Byte *dst, const aes__Word *src, size_t numBytes)
	{
		const aes__Byte *s = reinterpret_cast<const aes__Byte *>(src);
		std::copy(s, s + numBytes, dst);
	}
	inline aes__Byte get_byte(aes__Word w, size_t num)
	{ return reinterpret_cast<const aes__Byte*>(&w)[num]; }
	inline void set_byte(aes__Word &w, aes__Byte val, size_t num)
	{ reinterpret_cast<aes__Byte*>(&w)[num] = val; }

	inline aes__Byte get_state_byte(const aes__Word *w, size_t num)
	{ return reinterpret_cast<const aes__Byte*>(w)[num]; }
	inline void set_state_byte(aes__Word *w, aes__Byte val, size_t num)
	{ reinterpret_cast<aes__Byte*>(w)[num] = val; }

#if SYSINFO_ENDIAN == SYSINFO_BIG_ENDIAN
	inline aes__Word lrot(aes__Word w, size_t numbits)
	{ return (w << numbits) | (w >> (aes__Word_Bits - numbits)); }
	inline aes__Word rrot(aes__Word w, size_t numbits);
	{ return (w >> numbits) | (w << (aes__Word_Bits - numbits)); }
#elif SYSINFO_ENDIAN == SYSINFO_LITTLE_ENDIAN
	inline aes__Word lrot(aes__Word w, size_t numbits)
	{ return (w >> numbits) | (w << (aes__Word_Bits - numbits)); }
	inline aes__Word rrot(aes__Word w, size_t numbits)
	{ return (w << numbits) | (w >> (aes__Word_Bits - numbits)); }
#else
//mix endian
	inline aes__Word lrot(aes__Word w, size_t numbits)
	{
		if (numbits % aes_Byte_Bits)
		{
			throw std::runtime_error("aes lrot only defined for multiples of aes__Byte_Bit rotation in mixed endian mode");
		}
		aes__Word ret;
		size_t byteshift = numbits / aes__Byte_Bits;
		for (size_t i = 0; i < aes__Word_Bytes; ++i)
		{ set_byte(ret, get_byte(w, (i + byteshift) % aes__Word_Bytes), i); }
		return ret;
	}
	inline aes__Word rrot(aes__Word w, size_t numbits)
	{
		if (numbits % aes__Byte_Bits)
		{
			throw std::runtime_error("aes lrot only defined for multiples of aes__Byte_Bit rotation in mixed endian mode");
		}
		aes__Word ret;
		size_t byteshift = aes__Word_Bytes - (numbits / aes__Byte_Bits);
		for (size_t i = 0; i < aes__Word_Bytes; ++i)
		{ set_byte(ret, get_byte(w, (i + byteshift) % aes__Word_Bytes), i); }
		return ret;
	}
#endif

#else
	//compatibility, load by bitshifts, value is as written in big endian
	inline void load_words(aes__Word *dst, const aes__Byte *src, size_t numBytes)
	{
		sha256__load_words(dst, src, numBytes);
	}
	inline void store_words(aes__Byte *dst, const aes__Word *src, size_t numBytes)
	{
		sha256__store_words(dst, src, numBytes);
	}

	inline aes__Byte get_byte(aes__Word w, size_t num)
	{
		return sha256__get_byte(w, num);
	}
	inline void set_byte(aes__Word &w, aes__Byte val, size_t num)
	{
		sha256__set_byte(&w, val, num);
	}

	inline aes__Byte get_state_byte(const aes__Word *w, size_t num)
	{ return get_byte(w[num / aes__Word_Bytes], num % aes__Word_Bytes); }
	inline void set_state_byte(aes__Word *w, aes__Byte val, size_t num)
	{ set_byte(w[num / aes__Word_Bytes], val, num % aes__Word_Bytes); }

	inline aes__Word lrot(aes__Word w, size_t numbits)
	{ return sha256__lrot(w, numbits); }
	inline aes__Word rrot(aes__Word w, size_t numbits)
	{ return sha256__rrot(w, numbits); }

#endif

	inline void xor_state(aes__Word *state1, const aes__Word *state2)
	{
		state1[0] ^= state2[0];
		state1[1] ^= state2[1];
		state1[2] ^= state2[2];
		state1[3] ^= state2[3];
	}

	inline void xor_state(
		aes__Word *dst,
		const aes__Word *state1,
		const aes__Word *state2)
	{
		dst[0] = state1[0] ^ state2[0];
		dst[1] = state1[1] ^ state2[1];
		dst[2] = state1[2] ^ state2[2];
		dst[3] = state1[3] ^ state2[3];
	}
}
#endif
