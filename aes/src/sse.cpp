#include "aes/sse.hpp"
#include "aes/raw.hpp"

namespace aes
{
	struct Impl_SSE::Impl
	{
		virtual std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const = 0;
		virtual std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const = 0;
		virtual std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const = 0;
		virtual std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const = 0;
	};
	namespace
	{
		struct fallback: public Impl_SSE::Impl
		{
			AES_FALLBACK f;
			fallback(const void *key, const Version &v): f(key, v) {}
			virtual std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const
			{ return f.encrypt_ecb(dst, src, sz); }
			virtual std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const
			{ return f.decrypt_ecb(dst, src, sz); }
			virtual std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const
			{ return f.encrypt_cbc(dst, src, sz); }
			virtual std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const
			{ return f.decrypt_cbc(dst, src, sz); }
		};
	}
}

#if defined(AES_USE_SSE) && AES_USE_SSE && defined(UINT32_MAX) && CHAR_BIT == 8 \
	&& ( \
		(defined(_WIN32) \
		&& (defined(_M_AMD64) || defined(_M_I86)  || defined(_M_IX86))) \
		|| (defined(__GNUC__) \
		&& (defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)  || defined(__i386__) || defined(__i386) || defined(i386))) \
	)

#include "cpuinfo/cpuinfo.hpp"

#ifdef _WIN32
//	_mm_storeu_si128, _mm_loadu_si128, _mm_xor_si128, _mm_set_epi32 (sse2)
//	_mm_shuffle_epi8 (ssse3)
#		include <intrin.h>
//	_mm_i32gather_epi32 (avx2)
#		include <immintrin.h>

#else

//	_mm_storeu_si128, _mm_loadu_si128, _mm_xor_si128, _mm_set_epi32 (sse2)
#		include <emmintrin.h>
#		include <immintrin.h> //_mm_i32gather_epi32, (avx2)
#		include <tmmintrin.h> //_mm_shuffle_epi8 (ssse3)

#endif

#include "aes/defs.hpp"
#include "aes/aesalg.hpp"

namespace aes
{
	namespace
	{
		static const __m128i RotatedCol0Shuffle = _mm_set_epi32(
			0xFFFFFF0C, 0xFFFFFF08, 0xFFFFFF04, 0xFFFFFF00);
		static const __m128i RotatedCol1Shuffle = _mm_set_epi32(
			0xFFFFFF01, 0xFFFFFF0D, 0xFFFFFF09, 0xFFFFFF05);
		static const __m128i RotatedCol2Shuffle = _mm_set_epi32(
			0xFFFFFF06, 0xFFFFFF02, 0xFFFFFF0E, 0xFFFFFF0A);
		static const __m128i RotatedCol3Shuffle = _mm_set_epi32(
			0xFFFFFF0B, 0xFFFFFF07, 0xFFFFFF03, 0xFFFFFF0F);

		static const __m128i IRotatedCol0Shuffle = _mm_set_epi32(
			0xFFFFFF0C, 0xFFFFFF08, 0xFFFFFF04, 0xFFFFFF00);
		static const __m128i IRotatedCol1Shuffle = _mm_set_epi32(
			0xFFFFFF09, 0xFFFFFF05, 0xFFFFFF01, 0xFFFFFF0D);
		static const __m128i IRotatedCol2Shuffle = _mm_set_epi32(
			0xFFFFFF06, 0xFFFFFF02, 0xFFFFFF0E, 0xFFFFFF0A);
		static const __m128i IRotatedCol3Shuffle = _mm_set_epi32(
			0xFFFFFF03, 0xFFFFFF0F, 0xFFFFFF0B, 0xFFFFFF07);

		static const __m128i Rotated_Shuffle = _mm_loadu_si128(
			reinterpret_cast<const __m128i*>(rot));
		static const __m128i IRotated_Shuffle = _mm_loadu_si128(
			reinterpret_cast<const __m128i*>(irot));

		const auto *submix0 = submixui()[0];
		const auto *submix1 = submixui()[1];
		const auto *submix2 = submixui()[2];
		const auto *submix3 = submixui()[3];

		const auto *isubmix0 = isubmixui()[0];
		const auto *isubmix1 = isubmixui()[1];
		const auto *isubmix2 = isubmixui()[2];
		const auto *isubmix3 = isubmixui()[3];

		struct sseimpl: public Impl_SSE::Impl
		{
			struct State
			{
				__m128i data;
				State() = default;

				void init(unsigned char *dst, const unsigned char *src)
				{ data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src)); }
				void operator>>(void *dst) const
				{ _mm_storeu_si128(reinterpret_cast<__m128i*>(dst), data); }
				State& operator ^=(__m128i o)
				{ data = _mm_xor_si128(data, o); return *this; }
				State& operator ^=(const State &o) { return *this ^= o.data; }

				void aes_enc(const __m128i key)
				{
					__m128i cola = _mm_i32gather_epi32(
						reinterpret_cast<const int*>(submix0),
						_mm_shuffle_epi8(data, RotatedCol0Shuffle), 4);
					__m128i colb = _mm_i32gather_epi32(
						reinterpret_cast<const int*>(submix1),
						_mm_shuffle_epi8(data, RotatedCol1Shuffle), 4);
					__m128i result = _mm_xor_si128(cola, key);
					cola = _mm_i32gather_epi32(
						reinterpret_cast<const int*>(submix2),
						_mm_shuffle_epi8(data, RotatedCol2Shuffle), 4);
					result = _mm_xor_si128(colb, result);
					colb = _mm_i32gather_epi32(
						reinterpret_cast<const int*>(submix3),
						_mm_shuffle_epi8(data, RotatedCol3Shuffle), 4);
					result = _mm_xor_si128(cola, result);
					data = _mm_xor_si128(colb, result);
				}
				void aes_enclast(__m128i key)
				{
					__m128i rotated = _mm_shuffle_epi8(data, Rotated_Shuffle);
					auto ptr = reinterpret_cast<unsigned char*>(&rotated);
					unsigned char subbed[State_Bytes];
					const auto *b = SBox();
					for (std::size_t i=0; i<State_Bytes; ++i)
					{ subbed[i] = b[ptr[i]]; }
					data = _mm_xor_si128(
						_mm_loadu_si128(reinterpret_cast<__m128i*>(subbed)), key);
				}
				void aes_dec(const __m128i &key)
				{
					__m128i cola = _mm_i32gather_epi32(
						reinterpret_cast<const int*>(isubmix0),
						_mm_shuffle_epi8(data, IRotatedCol0Shuffle), 4);
					__m128i colb = _mm_i32gather_epi32(
						reinterpret_cast<const int*>(isubmix1),
						_mm_shuffle_epi8(data, IRotatedCol1Shuffle), 4);
					__m128i result = _mm_xor_si128(cola, key);
					cola = _mm_i32gather_epi32(
						reinterpret_cast<const int*>(isubmix2),
						_mm_shuffle_epi8(data, IRotatedCol2Shuffle), 4);
					result = _mm_xor_si128(colb, result);
					colb = _mm_i32gather_epi32(
						reinterpret_cast<const int*>(isubmix3),
						_mm_shuffle_epi8(data, IRotatedCol3Shuffle), 4);
					result = _mm_xor_si128(cola, result);
					data = _mm_xor_si128(colb, result);
				}
				void aes_declast(const __m128i &key)
				{
					__m128i rotated = _mm_shuffle_epi8(data, IRotated_Shuffle);
					auto ptr = reinterpret_cast<unsigned char*>(&rotated);
					unsigned char subbed[State_Bytes];
					const auto *b = IBox();
					for (std::size_t i=0; i<State_Bytes; ++i)
					{ subbed[i] = b[ptr[i]]; }
					data = _mm_xor_si128(
						_mm_loadu_si128(reinterpret_cast<__m128i*>(subbed)), key);
				}
			};

			__m128i ekey[Max_Rounds];
			__m128i dkey[Max_Rounds];
			const Version &version;
			sseimpl(const void *key, const Version &v): version(v)
			{
				Impl_RawUI32 tmp(key, v);
				unsigned char k[Max_Rounds][State_Bytes];
				tmp.storekey(k, tmp.ekey);
				for (std::size_t i=0; i<v.Round_Keys; ++i)
				{ ekey[i] = _mm_loadu_si128(reinterpret_cast<const __m128i*>(k[i])); }
				tmp.storekey(k, tmp.dkey);
				for (std::size_t i=0; i<v.Round_Keys; ++i)
				{ dkey[i] = _mm_loadu_si128(reinterpret_cast<const __m128i*>(k[i])); }
			}

			void encrypt(State &state) const
			{
				state ^= ekey[0];
				for (std::size_t i=1; i<version.Round_Keys-1; ++i)
				{ state.aes_enc(ekey[i]); }
				state.aes_enclast(ekey[version.Round_Keys-1]);
			}
			void decrypt(State &state) const
			{
				state ^= dkey[0];
				for (std::size_t i=1; i<version.Round_Keys-1; ++i)
				{ state.aes_dec(dkey[i]); }
				state.aes_declast(dkey[version.Round_Keys-1]);
			}

			virtual std::size_t encrypt_ecb(
				void *dst, const void *src, std::size_t nbytes) const
			{ return aes::encrypt_ecb(dst, src, nbytes, *this); }
			virtual std::size_t decrypt_ecb(
				void *dst, const void *src, std::size_t nbytes) const
			{ return aes::decrypt_ecb(dst, src, nbytes, *this); }
			virtual std::size_t encrypt_cbc(
				void *dst, const void *src, std::size_t nbytes) const
			{ return aes::encrypt_cbc(dst, src, nbytes, *this); }
			virtual std::size_t decrypt_cbc(
				void *dst, const void *src, std::size_t nbytes) const
			{ return aes::decrypt_cbc(dst, src, nbytes, *this); }
		};

		std::shared_ptr<Impl_SSE::Impl> getimpl(const void *k, const Version &v)
		{
			if (Impl_SSE::okay())
			{ return std::make_shared<sseimpl>(k, v); }
			else
			{ return std::make_shared<fallback>(k, v); }
		}
	}
	Impl_SSE::Impl_SSE(const void *k, const Version &v): impl(getimpl(k, v))
	{}
	bool Impl_SSE::okay()
	{
		static const bool SSE_Okay = (
			cpuinfo::has("sse2") && cpuinfo::has("ssse3") && cpuinfo::has("avx2"));
		return SSE_Okay;
	}
}
#else
namespace aes
{
	bool Impl_SSE::okay() { return 0; }
	Impl_SSE::Impl_SSE(const void *k, const Version &v):
		impl(std::make_shared<fallback>(k, v))
	{}
}
#endif
namespace aes
{
	Impl_SSE::~Impl_SSE() {}
	std::size_t Impl_SSE::encrypt_ecb(void *dst, const void *src, std::size_t sz) const
	{ return impl->encrypt_ecb(dst, src, sz); }
	std::size_t Impl_SSE::decrypt_ecb(void *dst, const void *src, std::size_t sz) const
	{ return impl->decrypt_ecb(dst, src, sz); }
	std::size_t Impl_SSE::encrypt_cbc(void *dst, const void *src, std::size_t sz) const
	{ return impl->encrypt_cbc(dst, src, sz); }
	std::size_t Impl_SSE::decrypt_cbc(void *dst, const void *src, std::size_t sz) const
	{ return impl->decrypt_cbc(dst, src, sz); }
}
