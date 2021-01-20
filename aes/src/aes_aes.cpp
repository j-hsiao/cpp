//requirements: SSE2 (load into __m128i)
//              AES  (aes intrinsics)

#include <aes/aes_aes.hpp>
#include <aes/aes_types.hpp>

#include <cstdint>
#ifdef _WIN32
//docs say immintrin for msvc
//https://docs.microsoft.com/en-us/cpp/intrinsics/x64-amd64-intrinsics-list?view=msvc-160
#include <immintrin.h>
#else
//intel intrinsics guide says wmmintrin
//https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_aes&expand=233
#include <wmmintrin.h>
#endif


namespace aes
{
	void AES_aes::xorstate(AESState *dst, AESState *state1, AESState *state2)
	{
		__m128i s = _mm_load_si128(reinterpret_cast<const __m128i*>(state1));
		__m128i o = _mm_load_si128(reinterpret_cast<const __m128i*>(state2));
		__m128i d = _mm_xor_si128(s, o);
		_mm_store_si128(reinterpret_cast<__m128i*>(dst), d);
	}

	void AES_aes::xorstate(AESState *dst, AESState *other)
	{
		__m128i d = _mm_load_si128(reinterpret_cast<const __m128i*>(dst));
		__m128i o = _mm_load_si128(reinterpret_cast<const __m128i*>(other));
		d = _mm_xor_si128(d, o);
		_mm_store_si128(reinterpret_cast<__m128i*>(dst), d);
	}

	void AES_aes::encrypt(AESState &data, const AES_statevec &roundkeys)
	{
		const AESState *k = roundkeys.data();
		const AESState * const end = k + roundkeys.size() - 1;
		__m128i state = _mm_load_si128(reinterpret_cast<const __m128i*>(&data));
		__m128i key = _mm_loadu_si128(reinterpret_cast<const __m128i*>(k));
		state = _mm_xor_si128(state, key);
		++k;
		for (; k < end; ++k)
		{
			key = _mm_load_si128(reinterpret_cast<const __m128i*>(k));
			state = _mm_aesenc_si128(state, key);
		}
		key = _mm_load_si128(reinterpret_cast<const __m128i*>(end));
		state = _mm_aesenclast_si128(state, key);
		_mm_store_si128(reinterpret_cast<__m128i*>(&data), state);
	}
	
	void AES_aes::decrypt(AESState &data, const AES_statevec &iroundkeys)
	{
		const AESState *k = iroundkeys.data();
		const AESState * const end = k + iroundkeys.size() - 1;
		__m128i state = _mm_load_si128(reinterpret_cast<const __m128i*>(&data));
		__m128i key = _mm_loadu_si128(reinterpret_cast<const __m128i*>(k));
		state = _mm_xor_si128(state, key);
		++k;
		for (; k < end; ++k)
		{
			key = _mm_load_si128(reinterpret_cast<const __m128i*>(k));
			state = _mm_aesdec_si128(state, key);
		}
		key = _mm_load_si128(reinterpret_cast<const __m128i*>(end));
		state = _mm_aesdeclast_si128(state, key);
		_mm_store_si128(reinterpret_cast<__m128i*>(&data), state);
	}
}
