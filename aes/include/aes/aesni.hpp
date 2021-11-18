#ifndef AES_IMPL_AESNI_HPP
#define AES_IMPL_AESNI_HPP

#include "aes/aes_dllcfg.h"
#include "aes/defs.hpp"
#include <cstddef>
#include <memory>

namespace aes
{
	struct Impl_AESNI
	{
		struct Impl;
		std::shared_ptr<Impl> impl;
		Impl_AESNI(const void *key, const Version &version_);
		~Impl_AESNI();

		std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const;
		std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const;
		std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const;
		std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const;
		static bool okay();
	};
}

#endif
