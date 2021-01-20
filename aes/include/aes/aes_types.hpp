#ifndef AES_TYPES_H
#define AES_TYPES_H

#include <aes/align.hpp>
#include <aes/aes.hpp>

#include <cstring>
#include <cstdint>
#include <vector>

//handle endianness
#if defined(UINT32_MAX) && !defined(AES_FORCE_B)

#ifdef LITTLE_ENDIAN
#define AES_ENDIAN 2
#elif defined(BIG_ENDIAN)
#define AES_ENDIAN 1
#else

#ifdef _WIN32
#pragma message("uint32 detected but endianness not set, using byte-wise implementations")
#else
#warning "uint32 detected but endianness not set, using byte-wise implementations"
#endif

#define AES_ENDIAN 0
#endif

#else
#define AES_ENDIAN 0
#endif

namespace aes
{

#if defined(INT8_MAX) && defined(UINT8_MAX)
	typedef uint8_t ubyte;
	typedef int8_t byte;
#elif CHAR_BIT == 8
	typedef unsigned char ubyte;
	typedef char byte;
#else
#error "AES requires 8-bit type but none detected"
#endif
	inline ubyte aesmul2(ubyte ub)
	{
		return (ub << 1) ^ ((reinterpret_cast<const byte&>(ub) >> 7) & 0x1b);
	}

	inline ubyte aesmul(ubyte ub, ubyte v)
	{
		ubyte ret = 0;
		ubyte tmp = ub;
		while (v)
		{
			if (v & 0x01)
			{
				ret ^= tmp;
			}
			v >>= 1;
			tmp = aesmul2(tmp);
		}
		return ret;
	}

	static const std::size_t kWordbits = k_wordbytes * k_bytebits;

//performance differences seem mostly because
//didn't turn on optimization... CMAKE_BUILD_TYPE default was not Release?
#if AES_ENDIAN
	typedef uint32_t AESWord;
	//macro still slightly faster than inline function but small difference
	//but prefer avoiding macros
#define xorwords(a, b, c, d, e) (a = b ^ c ^ d ^ e)
//	inline void xorwords(
//		AESWord &dst,
//		const AESWord &w1, const AESWord &w2,
//		const AESWord &w3, const AESWord &w4)
//	{
//		dst = w1 ^ w2 ^ w3 ^ w4;
//	}

#if AES_ENDIAN == 2
	inline AESWord lshift(AESWord v, std::size_t amt)
	{
		return v >> amt;
	}
	inline AESWord rshift (AESWord v, std::size_t amt)
	{
		return v << amt;
	}
#elif AES_ENDIAN == 1
	inline AESWord lshift(AESWord v, std::size_t amt)
	{
		return v << amt;
	}
	inline AESWord rshift (AESWord v, std::size_t amt)
	{
		return v >> amt;
	}
#endif

	inline AESWord rotate_right(AESWord v, std::size_t amt)
	{
		amt %= kWordbits;
		return rshift(v, amt) | lshift(v, (kWordbits - amt));
	}

#else
	struct alignas(k_wordbytes) AESWord
	{
		ubyte bytes[k_wordbytes];
	};
	inline void xorwords(
		AESWord &dst,
		const AESWord &w1, const AESWord &w2,
		const AESWord &w3, const AESWord &w4)
	{
		dst.bytes[0] = w1.bytes[0] ^ w2.bytes[0] ^ w3.bytes[0] ^ w4.bytes[0];
		dst.bytes[1] = w1.bytes[1] ^ w2.bytes[1] ^ w3.bytes[1] ^ w4.bytes[1];
		dst.bytes[2] = w1.bytes[2] ^ w2.bytes[2] ^ w3.bytes[2] ^ w4.bytes[2];
		dst.bytes[3] = w1.bytes[3] ^ w2.bytes[3] ^ w3.bytes[3] ^ w4.bytes[3];
	}

	inline AESWord rotate_right(const AESWord &v, std::size_t amt)
	{
		amt %= kWordbits;
		AESWord buff[2];
		buff[0] = v;
		buff[1] = v;
		const ubyte *p = reinterpret_cast<ubyte*>(buff) + k_wordbytes - (amt / k_bytebits);
		std::size_t r = amt % k_bytebits;
		std::size_t l = k_bytebits - r;
		AESWord ret;
		for (int i = 0; i < k_wordbytes; ++i)
		{
			ret.bytes[i] = (p[i] >> r) | (p[i - 1] << l);
		}
		return ret;
	}

	inline AESWord operator^(const AESWord &a, const AESWord &b)
	{
		AESWord w;
		w.bytes[0] = a.bytes[0] ^ b.bytes[0];
		w.bytes[1] = a.bytes[1] ^ b.bytes[1];
		w.bytes[2] = a.bytes[2] ^ b.bytes[2];
		w.bytes[3] = a.bytes[3] ^ b.bytes[3];
		return w;
	}
	inline AESWord& operator^= (AESWord &a, const AESWord &b)
	{
		a.bytes[0] ^= b.bytes[0];
		a.bytes[1] ^= b.bytes[1];
		a.bytes[2] ^= b.bytes[2];
		a.bytes[3] ^= b.bytes[3];
		return a;
	}

#endif

	union alignas(STATEBYTES) AESState
	{
		AESWord words[STATEWORDS];
#ifdef UINT64_MAX
		uint64_t uints[2];
		AESState& operator^=(const AESState &o)
		{
			uints[0] ^= o.uints[0];
			uints[1] ^= o.uints[1];
			return *this;
		}
		void xorinto(const AESState &a, const AESState &b)
		{
			uints[0] = a.uints[0] ^ b.uints[0];
			uints[1] = a.uints[1] ^ b.uints[1];
		}
#else
		AESState& operator^=(const AESState &o)
		{
			words[0] ^= o.words[0];
			words[1] ^= o.words[1];
			words[2] ^= o.words[2];
			words[3] ^= o.words[3];
			return *this;
		}
		void xorinto(const AESState &a, const AESState &b)
		{
#if AES_ENDIAN
			words[0] = a.words[0] ^ b.words[0];
			words[1] = a.words[1] ^ b.words[1];
			words[2] = a.words[2] ^ b.words[2];
			words[3] = a.words[3] ^ b.words[3];
#else
			*this = a;
			*this ^= b;
#endif
		}
#endif

		AESState operator^ (const AESState &o) const
		{
			AESState res(*this);
			res ^= o;
			return res;
		}

		void operator<<(const void *ptr)
		{
			std::memcpy(this, ptr, sizeof(*this));
		}

		void operator>>(void *ptr) const
		{
			std::memcpy(ptr, this, sizeof(*this));
		}
	};
	static_assert(sizeof(AESWord) == k_wordbytes, "AESWord size is wrong");
	static_assert(sizeof(AESState) == STATEBYTES, "AESState size is wrong");

	typedef std::vector<AESWord, align::AlignedAllocator<AESWord>> AES_wordvec;
	typedef std::vector<AESState, align::AlignedAllocator<AESState>> AES_statevec;
}
#endif
