#ifndef AES_IMPL_SSL_HPP
#define AES_IMPL_SSL_HPP

#include "aes/aes_dllcfg.h"
#include "aes/defs.hpp"

#include <cstddef>
#include <memory>
namespace aes
{
	struct Impl_SSL
	{
		struct Impl
		{
			virtual std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const = 0;
			virtual std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const = 0;
			virtual std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const = 0;
			virtual std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const = 0;
			virtual ~Impl() {}
		};
		std::shared_ptr<Impl> impl;

		Impl_SSL(const void *key, const Version &version_);
		~Impl_SSL();

		std::size_t encrypt_ecb(void *d, const void *s, std::size_t sz) const
		{ return impl->encrypt_ecb(d, s, sz); }
		std::size_t decrypt_ecb(void *d, const void *s, std::size_t sz) const
		{ return impl->decrypt_ecb(d, s, sz); }
		std::size_t encrypt_cbc(void *d, const void *s, std::size_t sz) const
		{ return impl->encrypt_cbc(d, s, sz); }
		std::size_t decrypt_cbc(void *d, const void *s, std::size_t sz) const
		{ return impl->decrypt_cbc(d, s, sz); }
		static bool okay();
	};
}

#endif
