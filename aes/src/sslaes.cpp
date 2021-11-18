#include "aes/sslaes.hpp"
#include <memory>

//using openssl aes header directly
//I saw no need to initialize libs or whatever...
//maybe it's fine?
//slower than my impl though, but provides a constant-time
//impl which may be "more secure"
#if AES_USE_SSL
#include "openssl/aes.h"
#include "aes/defs.hpp"
#include "aes/aesalg.hpp"

namespace aes
{
	namespace
	{
		struct sslaesimpl: public Impl_SSLAES::Impl
		{
			struct State
			{
				unsigned char *dst;
				const unsigned char *src;
				State() = default;
				void init(unsigned char *dst_, const unsigned char *src_)
				{
					dst = dst_;
					src = src_;
				}
				void operator>>(unsigned char *dst) {}

				const unsigned char *data() const { return src ? src : dst; }

				State& operator ^=(const State &other)
				{
					const unsigned char *d = data();
					src = nullptr;
					const unsigned char *od = other.data();
					for (std::size_t i=0; i<State_Bytes; ++i)
					{ dst[i] = d[i] ^ od[i]; }
					return *this;
				}
			};
			void encrypt(State &state) const
			{
				const unsigned char *data = state.data();
				state.src = nullptr;
				AES_encrypt(data, state.dst, &ekeys);
			}
			void decrypt(State &state) const
			{
				const unsigned char *data = state.data();
				state.src = nullptr;
				AES_decrypt(data, state.dst, &dkeys);
			}

			AES_KEY ekeys;
			AES_KEY dkeys;
			const Version &version;
			sslaesimpl(const void *k, const Version &v):
				version(v)
			{
				AES_set_encrypt_key(
					reinterpret_cast<const unsigned char*>(k),
					static_cast<int>(version.Key_Bytes * Byte_Bits), &ekeys);
				AES_set_decrypt_key(
					reinterpret_cast<const unsigned char*>(k),
					static_cast<int>(version.Key_Bytes * Byte_Bits), &dkeys);
			}

			virtual std::size_t encrypt_ecb(void *d, const void *s, std::size_t sz) const
			{ return aes::encrypt_ecb(d, s, sz, *this); }
			virtual std::size_t decrypt_ecb(void *d, const void *s, std::size_t sz) const
			{ return aes::decrypt_ecb(d, s, sz, *this); }
			virtual std::size_t encrypt_cbc(void *d, const void *s, std::size_t sz) const
			{ return aes::encrypt_cbc(d, s, sz, *this); }
			virtual std::size_t decrypt_cbc(void *d, const void *s, std::size_t sz) const
			{ return aes::decrypt_cbc(d, s, sz, *this); }
		};
	}
	Impl_SSLAES::Impl_SSLAES(const void *key, const Version &v):
		impl(std::make_shared<sslaesimpl>(key, v))
	{}
	bool Impl_SSLAES::okay() { return 1; }
}
#else
#include "aes/raw.hpp"
namespace aes
{
	namespace
	{
		struct fallback: public aes::Impl_SSLAES::Impl
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
	Impl_SSLAES::Impl_SSLAES(const void *key, const Version &v):
		impl(std::make_shared<fallback>(key, v))
	{}
	bool Impl_SSLAES::okay() { return 0; }
}
#endif
namespace aes
{ Impl_SSLAES::~Impl_SSLAES() {} }
