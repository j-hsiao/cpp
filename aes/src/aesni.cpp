#include "aes/aesni.hpp"
#include "aes/raw.hpp"

#include <memory>

namespace aes
{
	struct Impl_AESNI::Impl
	{
		virtual std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const = 0;
		virtual std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const = 0;
		virtual std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const = 0;
		virtual std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const = 0;
		virtual ~Impl() {}
	};
	namespace
	{
		struct fallback: public aes::Impl_AESNI::Impl
		{
			aes::AES_FALLBACK f;

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

#include <cstdint>
#if (defined(AES_USE_AESNI) && AES_USE_AESNI \
	&& ((defined(_WIN32) && (defined(_M_AMD64) || defined(_M_I86)  || defined(_M_IX86))) \
	|| (\
		defined(__GNUC__) \
		&& (defined(__amd64__) || defined(__amd64) || defined(__x86_64__) \
			|| defined(__x86_64)  || defined(__i386__) || defined(__i386) \
			|| defined(i386)))))


#include "aes/defs.hpp"
#include "cpuinfo/cpuinfo.hpp"
#include "aes/aesalg.hpp"

#ifdef _WIN32
#include <intrin.h>    // _mm_storeu_si128, _mm_loadu_si128, _mm_xor_si128
#include <immintrin.h> // _mm_aesenc_si128, _mm_aesdec_si128...
#else
#include <emmintrin.h> //_mm_storeu_si128, _mm_loadu_si128, _mm_xor_si128
#include <wmmintrin.h> //_mm_aesenc_si128, _mm_aesdec_si128...
#endif


namespace aes
{
	namespace
	{
		struct aesimpl: public aes::Impl_AESNI::Impl
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
			};
			void encrypt(State &s) const
			{
				s ^= ekey[0];
				for (std::size_t i=1; i<version.Round_Keys-1; ++i)
				{ s.data = _mm_aesenc_si128(s.data, ekey[i]); }
				s.data = _mm_aesenclast_si128(s.data, ekey[version.Round_Keys-1]);
			}
			void decrypt(State &s) const
			{
				s ^= dkey[0];
				for (std::size_t i=1; i<version.Round_Keys-1; ++i)
				{ s.data = _mm_aesdec_si128(s.data, dkey[i]); }
				s.data = _mm_aesdeclast_si128(s.data, dkey[version.Round_Keys-1]);
			}

			__m128i ekey[Max_Rounds];
			__m128i dkey[Max_Rounds];
			const Version &version;
			aesimpl(const void *key, const Version &v): version(v)
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

			virtual std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const
			{ return aes::encrypt_ecb(dst, src, sz, *this); }
			virtual std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const
			{ return aes::decrypt_ecb(dst, src, sz, *this); }
			virtual std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const
			{ return aes::encrypt_cbc(dst, src, sz, *this); }
			virtual std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const
			{ return aes::decrypt_cbc(dst, src, sz, *this); }
		};
		std::shared_ptr<Impl_AESNI::Impl> getimpl(const void *k, const Version &v)
		{
			if (Impl_AESNI::okay())
			{ return std::make_shared<aesimpl>(k, v); }
			else
			{ return std::make_shared<fallback>(k, v); }
		}
	}
	Impl_AESNI::Impl_AESNI(const void *k, const Version &v): impl(getimpl(k, v))
	{}

	bool Impl_AESNI::okay()
	{
		static const bool AESNI_Okay = cpuinfo::has("sse2") && cpuinfo::has("aes");
		return AESNI_Okay;
	}
}
#else
namespace aes
{
	Impl_AESNI::Impl_AESNI(const void *k, const Version &v):
		impl(std::make_shared<fallback>(k, v))
	{}

	bool Impl_AESNI::okay() { return 0; }
}
#endif

namespace aes
{
	Impl_AESNI::~Impl_AESNI() {}
	std::size_t Impl_AESNI::encrypt_ecb(void *dst, const void *src, std::size_t sz) const
	{ return impl->encrypt_ecb(dst, src, sz); }
	std::size_t Impl_AESNI::decrypt_ecb(void *dst, const void *src, std::size_t sz) const
	{ return impl->decrypt_ecb(dst, src, sz); }
	std::size_t Impl_AESNI::encrypt_cbc(void *dst, const void *src, std::size_t sz) const
	{ return impl->encrypt_cbc(dst, src, sz); }
	std::size_t Impl_AESNI::decrypt_cbc(void *dst, const void *src, std::size_t sz) const
	{ return impl->decrypt_cbc(dst, src, sz); }
}
