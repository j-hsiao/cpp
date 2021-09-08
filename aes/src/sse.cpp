//using sse to implement aes
//usefuls:
//
//
//multiplication:
//	mul by 2 = left shift by 1 for 16 bits, then mask bytes
//
//SSE2:
//	_mm_xor_si128		xor state
//	_mm_and_si128		bitwise and
//	_mm_slli_si128	left shift lshift 1 and _mm_and_si128 with 0xFE everywhere to get lshift by 1 for 8-bit int
//SSSE3:
//	__m128i _mm_shuffle_epi8(__m128i a, __m128i b)
//
//	b = 16 x 8-byte indices
//	for each index, if index & 0x80, then set corresponding value to 0 in output
//	otherwise, copy a[index] to corresponding position in output
//
//
//
//avx2:
//	__m128i _mm_i32gather_epi32(int const *base, __m128i vindex, const int scale)
//	{
//		uint32_t result[4];
//		for (int i = 0; i < 4; ++i)
//		{
//			result[i] = base[vindex[i] * scale];
//		}
//	}
//
//	shift/mask to get each byte and then gather from mixcols
//
//	gather (load 32-bit int from memory)		sub bytes/shift rows?, use with lut?
//		(bytes need to be converted to 32 bit ints though)
//		(maybe can use shuffle to get selected vals?)

#include <aes/aes.h>
#include <.aes/impl.hpp>
#include <.aes/consts.hpp>
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
#include <cpuinfo/cpuinfo.hpp>

#ifdef _WIN32
// _mm_storeu_si128, _mm_loadu_si128, _mm_xor_si128, _mm_set_epi32 (sse2)
// _mm_shuffle_epi8 (ssse3)
#include <intrin.h>
// _mm_i32gather_epi32 (avx2)
#include <immintrin.h> 
#else
// _mm_storeu_si128, _mm_loadu_si128, _mm_xor_si128, _mm_set_epi32 (sse2)
#include <emmintrin.h>
#include <immintrin.h> //_mm_i32gather_epi32, (avx2)
#include <tmmintrin.h> //_mm_shuffle_epi8 (ssse3)
#endif

#endif

namespace aes
{
	namespace
	{
		static const __m128i Pick0 = _mm_set_epi32(
			0xFFFFFF0C, 0xFFFFFF08, 0xFFFFFF04, 0xFFFFFF00);
		static const __m128i Pick1 = _mm_set_epi32(
			0xFFFFFF01, 0xFFFFFF0D, 0xFFFFFF09, 0xFFFFFF05);
		static const __m128i Pick2 = _mm_set_epi32(
			0xFFFFFF06, 0xFFFFFF02, 0xFFFFFF0E, 0xFFFFFF0A);
		static const __m128i Pick3 = _mm_set_epi32(
			0xFFFFFF0B, 0xFFFFFF07, 0xFFFFFF03, 0xFFFFFF0F);

		static const __m128i IPick0 = _mm_set_epi32(
			0xFFFFFF0C, 0xFFFFFF08, 0xFFFFFF04, 0xFFFFFF00);
		static const __m128i IPick1 = _mm_set_epi32(
			0xFFFFFF09, 0xFFFFFF05, 0xFFFFFF01, 0xFFFFFF0D);
		static const __m128i IPick2 = _mm_set_epi32(
			0xFFFFFF06, 0xFFFFFF02, 0xFFFFFF0E, 0xFFFFFF0A);
		static const __m128i IPick3 = _mm_set_epi32(
			0xFFFFFF03, 0xFFFFFF0F, 0xFFFFFF0B, 0xFFFFFF07);

		static const __m128i Rowshift = _mm_loadu_si128(
			reinterpret_cast<const __m128i*>(aes::ShiftRows));
		static const __m128i IRowshift = _mm_loadu_si128(
			reinterpret_cast<const __m128i*>(aes::IShiftRows));

		__m128i lo8mask()
		{
			int lo[4] = {0xFF, 0xFF, 0xFF, 0xFF};
			static __m128i ret = _mm_loadu_si128(reinterpret_cast<__m128i*>(lo));
			return ret;
		}
		__m128i nthbytes(__m128i val, int n)
		{ return _mm_and_si128(_mm_srli_epi32(val, n * 8), lo8mask()); }

		struct SSEState
		{
			static void encrypt_state(
				aes__Word *dst, const aes__Word *src, const aes__Keys &keys)
			{
				__m128i b0, b1, b2, state, key;
				static_assert(
					sizeof(aes__Word) == 4 && CHAR_BIT == 8,
					"aes__Word or CHAR_BIT has wrong size");
				state = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
				key = _mm_loadu_si128(
					reinterpret_cast<const __m128i*>(keys.encrypt[0]));
				state = _mm_xor_si128(state, key);
				auto lastkey = keys.encrypt + aes::NumRounds[keys.version];

				const int * const m0 = reinterpret_cast<const int*>(aes::Mix[0]);
				const int * const m1 = reinterpret_cast<const int*>(aes::Mix[1]);
				const int * const m2 = reinterpret_cast<const int*>(aes::Mix[2]);
				const int * const m3 = reinterpret_cast<const int*>(aes::Mix[3]);
				for (auto pkey = keys.encrypt + 1; pkey < lastkey; ++pkey)
				{
					//using sizeof as scale fails to compile for gather
					//"latency/throughput = 17/5, 14/7
					static_assert(sizeof(aes__Word) == 4, "aes__Word should have size 4 which is scale to use for gather");
					b0 = _mm_i32gather_epi32(m0, _mm_shuffle_epi8(state, Pick0), 4);
					key = _mm_loadu_si128(reinterpret_cast<const __m128i*>(*pkey));
					b1 = _mm_i32gather_epi32(m1, _mm_shuffle_epi8(state, Pick1), 4);
					b0 = _mm_xor_si128(b0, key);
					b2 = _mm_i32gather_epi32(m2, _mm_shuffle_epi8(state, Pick2), 4);
					b0 = _mm_xor_si128(b0, b1);
					b1 = _mm_i32gather_epi32(m3, _mm_shuffle_epi8(state, Pick3), 4);
					b0 = _mm_xor_si128(b0, b2);
					state = _mm_xor_si128(b0, b1);
				}
				state = _mm_shuffle_epi8(state, Rowshift);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst), state);
				for (std::size_t i = 0; i < aes__State_Bytes; ++i)
				{ aes::set_state_byte(dst, aes::SBox[aes::get_state_byte(dst, i)], i); }
				aes::xor_state(dst, *lastkey);
			}

			static void decrypt_state(
				aes__Word *dst, const aes__Word *src, const aes__Keys &keys)
			{
				__m128i state, key, b0, b1, b2;
				static_assert(
					sizeof(aes__Word) == 4 && CHAR_BIT == 8,
					"aes__Word or CHAR_BIT has wrong size");
				state = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
				key = _mm_loadu_si128(
					reinterpret_cast<const __m128i*>(keys.decrypt[0]));
				state = _mm_xor_si128(state, key);
				auto lastkey = keys.decrypt + aes::NumRounds[keys.version];
				const int * const im0 = reinterpret_cast<const int*>(aes::IMix[0]);
				const int * const im1 = reinterpret_cast<const int*>(aes::IMix[1]);
				const int * const im2 = reinterpret_cast<const int*>(aes::IMix[2]);
				const int * const im3 = reinterpret_cast<const int*>(aes::IMix[3]);
				for (auto pkey = keys.decrypt + 1; pkey < lastkey; ++pkey)
				{
					key = _mm_loadu_si128(reinterpret_cast<const __m128i*>(*pkey));
					//using sizeof as scale fails to compile for gather
					static_assert(sizeof(aes__Word) == 4, "aes__Word should have size 4 which is scale to use for gather");
					b0 = _mm_i32gather_epi32(im0, _mm_shuffle_epi8(state, IPick0), 4);
					key = _mm_loadu_si128(reinterpret_cast<const __m128i*>(*pkey));
					b1 = _mm_i32gather_epi32(im1, _mm_shuffle_epi8(state, IPick1), 4);
					b0 = _mm_xor_si128(b0, key);
					b2 = _mm_i32gather_epi32(im2, _mm_shuffle_epi8(state, IPick2), 4);
					b0 = _mm_xor_si128(b0, b1);
					b1 = _mm_i32gather_epi32(im3, _mm_shuffle_epi8(state, IPick3), 4);
					b0 = _mm_xor_si128(b0, b2);
					state = _mm_xor_si128(b0, b1);
				}
				state = _mm_shuffle_epi8(state, IRowshift);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst), state);
				for (std::size_t i = 0; i < aes__State_Bytes; ++i)
				{ aes::set_state_byte(dst, aes::IBox[aes::get_state_byte(dst, i)], i); }

				aes::xor_state(dst, *lastkey);
			}
		};

		//SSE optimization??
		//convert aes__keys to __m128i onetime per each range encryption
		struct SSEkeys
		{
			SSEKeys(const aes__Keys &keys):
				version(keys.version)
			{
				std::memcpy(key, keys.key, aes__Max_Key_Bytes);
				for (std::size_t i=0; i<aes__Max_Keys; ++i)
				{
					encrypt[i] = _mm_loadu_si128(
						reinterpret_cast<const __m128i*>(keys.encrypt[i]));
					decrypt[i] = _mm_loadu_si128(
						reinterpret_cast<const __m128i*>(keys.decrypt[i]));
				}
			}
			__m128i encrypt[aes__Max_Keys];
			__m128i decrypt[aes__Max_Keys];
			int version;
			aes__Byte key[aes__Max_Key_Bytes];
		};
		template<class T, std::size_t alignment>
		class AlignedBuffer
		{
			private:
				std::vector<unsigned char> buf;
			public:
				typedef T value_type;
				Aligned_Data(std::size_t length):
					buf(sizeof(T)*length + alignment),
					data(&buf[0]),
					size(buf.size())
				{
					void *ptr = data_;
					buf.resize((sizeof(T) * length) + alignment);
					size = buf.size();
					ptr = &buf[0];
					if (!std::align(alignment, length * sizeof(T), ptr, size))
					{ throw std::runtime_error("alignment failed"); }
					data = reinterpret_cast<T*>(ptr);
					for (std::size_t i=0; i<length; ++i)
					{ new(data+i) T; }
				}
				T& operator[](std::size_t idx){ return data[idx]; }
				const std::size_t size;
				T *data;
		};

		struct AlignedSSEState
		{
			static void encrypt_state(
				aes__Byte *dst, const aes__Byte *src, const SSEkeys &keys,
				const aes__Keys &rkeys
			{
				static_assert(
					sizeof(aes__Word) == 4 && CHAR_BIT == 8,
					"aes__Word or CHAR_BIT has wrong size");
				__m128i b0, b1, b2, state;
				state = _mm_load_si128(reinterpret_cast<const __m128i*>(src));
				state = _mm_xor_si128(state, keys.encrypt[0]);

				auto *lastkey = keys.encrypt + aes::NumRounds[keys.version];

				const int * const m0 = reinterpret_cast<const int*>(aes::Mix[0]);
				const int * const m1 = reinterpret_cast<const int*>(aes::Mix[1]);
				const int * const m2 = reinterpret_cast<const int*>(aes::Mix[2]);
				const int * const m3 = reinterpret_cast<const int*>(aes::Mix[3]);
				for (auto pkey = keys.encrypt + 1; pkey < lastkey; ++pkey)
				{
					//using sizeof as scale fails to compile for gather
					//"latency/throughput = 17/5, 14/7
					b0 = _mm_i32gather_epi32(m0, _mm_shuffle_epi8(state, Pick0), 4);
					b1 = _mm_i32gather_epi32(m1, _mm_shuffle_epi8(state, Pick1), 4);
					b0 = _mm_xor_si128(b0, *pkey);
					b2 = _mm_i32gather_epi32(m2, _mm_shuffle_epi8(state, Pick2), 4);
					b0 = _mm_xor_si128(b0, b1);
					b1 = _mm_i32gather_epi32(m3, _mm_shuffle_epi8(state, Pick3), 4);
					b0 = _mm_xor_si128(b0, b2);
					state = _mm_xor_si128(b0, b1);
				}
				state = _mm_shuffle_epi8(state, Rowshift);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst), state);
				for (std::size_t i = 0; i < aes__State_Bytes; ++i)
				{ aes::set_state_byte(dst, aes::SBox[aes::get_state_byte(dst, i)], i); }
				aes::xor_state(dst, rkeys.encrypt[aes::NumRounds[keys.version]]);
			}

			static void decrypt_state(
				aes__Byte *dst, const aes__Byte *src, const SSEkeys &keys,
				const aes__Keys &rkeys)
			{
				static_assert(
					sizeof(aes__Word) == 4 && CHAR_BIT == 8,
					"aes__Word or CHAR_BIT has wrong size");

				__m128i state, b0, b1, b2;
				state = _mm_load_si128(reinterpret_cast<const __m128i*>(src));
				state = _mm_xor_si128(state, keys.decrypt[0]);
				auto lastkey = keys.decrypt + aes::NumRounds[keys.version];
				const int * const im0 = reinterpret_cast<const int*>(aes::IMix[0]);
				const int * const im1 = reinterpret_cast<const int*>(aes::IMix[1]);
				const int * const im2 = reinterpret_cast<const int*>(aes::IMix[2]);
				const int * const im3 = reinterpret_cast<const int*>(aes::IMix[3]);
				for (auto pkey = keys.decrypt + 1; pkey < lastkey; ++pkey)
				{
					//using sizeof as scale fails to compile for gather
					b0 = _mm_i32gather_epi32(im0, _mm_shuffle_epi8(state, IPick0), 4);
					b1 = _mm_i32gather_epi32(im1, _mm_shuffle_epi8(state, IPick1), 4);
					b0 = _mm_xor_si128(b0, *pkey);
					b2 = _mm_i32gather_epi32(im2, _mm_shuffle_epi8(state, IPick2), 4);
					b0 = _mm_xor_si128(b0, b1);
					b1 = _mm_i32gather_epi32(im3, _mm_shuffle_epi8(state, IPick3), 4);
					b0 = _mm_xor_si128(b0, b2);
					state = _mm_xor_si128(b0, b1);
				}
				state = _mm_shuffle_epi8(state, IRowshift);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst), state);
				for (std::size_t i = 0; i < aes__State_Bytes; ++i)
				{ aes::set_state_byte(dst, aes::IBox[aes::get_state_byte(dst, i)], i); }
				aes::xor_state(dst, rkeys.decrypt[aes::NumRounds[keys.version]]);
			}
		};
	}
#if 0
	//Try to optimize, in this case the data is being loaded into
	//words, encoded, then stored
	std::size_t SSEImpl::encrypt_ecb(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, std::size_t nbytes)
	{
		std::size_t remain = nbytes % aes__State_Bytes;
		aes__Byte padbytes = static_cast<aes__Byte>(aes__State_Bytes - remain);
		std::size_t total = nbytes + padbytes;
		const aes__Byte * const end = src + total - aes__State_Bytes;

		Aligned_Data<aes__Word, sizeof(__m128i)> buf(nullptr, aes__State_Words);
		aes__Word *state = buf.data
		SSEkeys skeys(keys);
		for (; src < end; src+=aes__State_Bytes, dst+=aes__State_Bytes)
		{
			load_words(state, src, aes__State_Bytes);
			AlignedSSEState::encrypt_state(state, state, skeys, keys);
			store_words(dst, state, aes__State_Bytes);
		}
		load_words(state, src, aes__State_Bytes);
		for (std::size_t i = remain; i < aes__State_Bytes; ++i)
		{ set_state_byte(state, padbytes, i); }
		AlignedSSEState::encrypt_state(state, state, skeys, keys);
		store_words(dst, state, aes__State_Bytes);
		return total;
	}
#else
	std::size_t SSEImpl::encrypt_ecb(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::encrypt_ecb<SSEState>(dst, src, keys, datalen); }
	std::size_t SSEImpl::decrypt_ecb(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::decrypt_ecb<SSEState>(dst, src, keys, datalen); }
	std::size_t SSEImpl::encrypt_cbc(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::encrypt_cbc<SSEState>(dst, src, keys, datalen); }
	std::size_t SSEImpl::decrypt_cbc(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::decrypt_cbc<SSEState>(dst, src, keys, datalen); }
#endif
}
