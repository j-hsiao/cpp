//raw C-like no template etc implementation

#ifndef AES_RAW_HPP
#define AES_RAW_HPP
#include "aes/aes_dllcfg.h"
#include "aes/defs.hpp"

#include <cstddef>
#include <cstdint>
#include <climits>
namespace aes
{
	const std::uint_least32_t* const * submixuil();
	const std::uint_least32_t* const * isubmixuil();

	struct Impl_RawUIL32
	{
		typedef std::uint_least32_t word;
		const Version &version;
		word ekey[Max_Rounds][State_Words];
		word dkey[Max_Rounds][State_Words];

		Impl_RawUIL32(const void *key, const Version &v);
		std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const;
		std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const;
		std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const;
		std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const;

		void storekey(void *dst, word (&arr)[Max_Rounds][State_Words]) const;
	};

#if defined(UINT32_MAX) && CHAR_BIT==8

	const std::uint32_t* const * submixui();
	const std::uint32_t* const * isubmixui();

	struct Impl_RawUI32
	{
		typedef std::uint32_t word;
		const Version &version;
		word ekey[Max_Rounds][State_Words];
		word dkey[Max_Rounds][State_Words];

		Impl_RawUI32(const void *key, const Version &v);
		void storekey(void *dst, const word (&key)[Max_Rounds][State_Words]) const;
		std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const;
		std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const;
		std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const;
		std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const;
	};
#else
	typedef Impl_RawUIL32 Impl_RawUI32;
#endif
}
#endif
