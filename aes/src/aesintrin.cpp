#include <aes/aes.h>
#include <.aes/impl.hpp>
#include <.aes/consts.hpp>

#include <cstdint>
#include <stdexcept>



#ifndef AES_USE_AES_INTRINSICS

#if defined(UINT32_MAX)

#if defined(_WIN32) && (defined(_M_AMD64) || defined(_M_I86)  || defined(_M_IX86))
#define AES_USE_AES_INTRINSICS 1
#elif defined(__GNUC__) && (defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)  || defined(__i386__) || defined(__i386) || defined(i386))
#define AES_USE_AES_INTRINSICS 1
#endif

#endif

#ifndef AES_USE_AES_INTRINSICS
#define AES_USE_AES_INTRINSICS 0
#endif

#endif


#if AES_USE_AES_INTRINSICS

#ifdef _WIN32
#include <intrin.h>    // _mm_storeu_si128, _mm_loadu_si128, _mm_xor_si128
#include <immintrin.h> // _mm_aesenc_si128, _mm_aesdec_si128...
#else
#include <emmintrin.h> //_mm_storeu_si128, _mm_loadu_si128, _mm_xor_si128
#include <wmmintrin.h> //_mm_aesenc_si128, _mm_aesdec_si128...
#endif

#endif

namespace aes
{
	namespace
	{
		struct IntrinState
		{
#if AES_USE_AES_INTRINSICS
			static void encrypt_state(
				aes__Word *dst, const aes__Word *src, const aes__Keys &keys)
			{
				static_assert(
					sizeof(aes__Word) == 4 && CHAR_BIT == 8,
					"aes__Word or CHAR_BIT has wrong size");
				__m128i state = _mm_loadu_si128(
					reinterpret_cast<const __m128i*>(src));
				__m128i key = _mm_loadu_si128(
					reinterpret_cast<const __m128i*>(keys.encrypt[0]));
				state = _mm_xor_si128(state, key);
				auto lastkey = keys.encrypt + aes::NumRounds[keys.version];
				for (auto pkey = keys.encrypt + 1; pkey < lastkey; ++pkey)
				{
					key = _mm_loadu_si128(reinterpret_cast<const __m128i*>(*pkey));
					state = _mm_aesenc_si128(state, key);
				}
				key = _mm_loadu_si128(reinterpret_cast<const __m128i*>(*lastkey));
				state = _mm_aesenclast_si128(state, key);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst), state);
			}
			static void decrypt_state(
				aes__Word *dst, const aes__Word *src, const aes__Keys &keys)
			{
				static_assert(
					sizeof(aes__Word) == 4 && CHAR_BIT == 8,
					"aes__Word or CHAR_BIT has wrong size");
				__m128i state = _mm_loadu_si128(
					reinterpret_cast<const __m128i*>(src));
				__m128i key = _mm_loadu_si128(
					reinterpret_cast<const __m128i*>(keys.decrypt[0]));
				state = _mm_xor_si128(state, key);
				auto lastkey = keys.decrypt + aes::NumRounds[keys.version];
				for (auto pkey = keys.decrypt + 1; pkey < lastkey; ++pkey)
				{
					key = _mm_loadu_si128(reinterpret_cast<const __m128i*>(*pkey));
					state = _mm_aesdec_si128(state, key);
				}
				key = _mm_loadu_si128(reinterpret_cast<const __m128i*>(*lastkey));
				state = _mm_aesdeclast_si128(state, key);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst), state);
			}
#else
			static void encrypt_state(
				aes__Word *dst, const aes__Word *src, const aes__Keys &keys)
			{ throw std::runtime_error("should not be dispatched here"); }
			static void decrypt_state(
				aes__Word *dst, const aes__Word *src, const aes__Keys &keys)
			{ throw std::runtime_error("should not be dispatched here"); }
#endif
		};
	}

	std::size_t AESIntrinImpl::encrypt_ecb(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::encrypt_ecb<IntrinState>(dst, src, keys, datalen); }

	std::size_t AESIntrinImpl::decrypt_ecb(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::decrypt_ecb<IntrinState>(dst, src, keys, datalen); }
	std::size_t AESIntrinImpl::encrypt_cbc(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::encrypt_cbc<IntrinState>(dst, src, keys, datalen); }
	std::size_t AESIntrinImpl::decrypt_cbc(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::decrypt_cbc<IntrinState>(dst, src, keys, datalen); }
}
