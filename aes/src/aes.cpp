#include "aes/aes.h"
#include "aes/aes.hpp"

#include "aes/aes_dllcfg.h"
#include "aes/defs.hpp"

#include "aes/aesni.hpp"
#include "aes/ssl.hpp"
#include "aes/raw.hpp"
#include "aes/sslaes.hpp"
#include "aes/ui32.hpp"
#include "aes/uil32.hpp"
#include "aes/plain.hpp"
#include "aes/sse.hpp"

#include <cstddef>
#include <memory>

namespace aes
{
	namespace
	{
		template<class T>
		struct cdc: public Codec
		{
			T impl;
			cdc(const void *k, const Version &v): impl(k, v) {}
			virtual std::size_t encrypt_ecb(void *d, const void *s, std::size_t sz) const
			{ return impl.encrypt_ecb(d, s, sz); }
			virtual std::size_t decrypt_ecb(void *d, const void *s, std::size_t sz) const
			{ return impl.decrypt_ecb(d, s, sz); }
			virtual std::size_t encrypt_cbc(void *d, const void *s, std::size_t sz) const
			{ return impl.encrypt_cbc(d, s, sz); }
			virtual std::size_t decrypt_cbc(void *d, const void *s, std::size_t sz) const
			{ return impl.decrypt_cbc(d, s, sz); }
		};
	}

	std::shared_ptr<Codec> getcodec(const void *k, const Version &v)
	{
		if (Impl_AESNI::okay())
		{ return std::make_shared<cdc<Impl_AESNI>>(k, v); }
		else if (Impl_SSL::okay())
		{ return std::make_shared<cdc<Impl_SSL>>(k, v); }
		else
		{ return std::make_shared<cdc<AES_FALLBACK>>(k, v); }
	}
}
namespace
{
	template<class T>
	struct funcs
	{
		static std::size_t pencrypt_ecb(
			const aes_pimpl *p, void *dst, const void *src, size_t nbytes)
		{ return reinterpret_cast<const T*>(p)->encrypt_ecb(dst, src, nbytes); }
		static std::size_t pdecrypt_ecb(
			const aes_pimpl *p, void *dst, const void *src, size_t nbytes)
		{ return reinterpret_cast<const T*>(p)->decrypt_ecb(dst, src, nbytes); }
		static std::size_t pencrypt_cbc(
			const aes_pimpl *p, void *dst, const void *src, size_t nbytes)
		{ return reinterpret_cast<const T*>(p)->encrypt_cbc(dst, src, nbytes); }
		static std::size_t pdecrypt_cbc(
			const aes_pimpl *p, void *dst, const void *src, size_t nbytes)
		{ return reinterpret_cast<const T*>(p)->decrypt_cbc(dst, src, nbytes); }
		static void free(aes_pimpl *p)
		{ delete reinterpret_cast<T*>(p); }

		static aes_Codec make(const void *key, aes_version v)
		{
			T *ptr = nullptr;
			switch (v)
			{
				case ::aes128:
					ptr = new T(key, aes::aes128);
					break;
				case ::aes192:
					ptr = new T(key, aes::aes192);
					break;
				case ::aes256:
					ptr = new T(key, aes::aes256);
					break;
			}
			if (ptr)
			{
				return {
					reinterpret_cast<aes_pimpl*>(ptr),
					funcs<T>::pencrypt_ecb, funcs<T>::pdecrypt_ecb,
					funcs<T>::pencrypt_cbc, funcs<T>::pdecrypt_cbc,
					funcs<T>::free};
			}
			else
			{ return {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}; }
		}
	};
}
CPP_EXTERNC_BEGIN

aes_Codec aes_Codec_new(const void *key, aes_version vs, aes_impl impl)
{
	switch(impl)
	{
		case aes_ssl:
			return funcs<aes::Impl_SSL>::make(key, vs);
		case aes_sslaes:
			return funcs<aes::Impl_SSLAES>::make(key, vs);
		case aes_aesni:
			return funcs<aes::Impl_AESNI>::make(key, vs);
		case aes_sse:
			return funcs<aes::Impl_SSE>::make(key, vs);
		case aes_ui32:
			return funcs<aes::Impl_UI32>::make(key, vs);
		case aes_uil32:
			return funcs<aes::Impl_UIL32>::make(key, vs);
		case aes_plain:
			return funcs<aes::Impl_Plain>::make(key, vs);
		case aes_rawuil32:
			return funcs<aes::Impl_RawUIL32>::make(key, vs);
		case aes_ui32_t:
			return funcs<aes::Impl_UI32_t>::make(key, vs);
		case aes_uil32_t:
			return funcs<aes::Impl_UIL32_t>::make(key, vs);
		case aes_rawui32:
			return funcs<aes::Impl_RawUI32>::make(key, vs);
		case aes_auto:
		{
			if (aes::Impl_AESNI::okay())
			{ return funcs<aes::Impl_AESNI>::make(key, vs); }
			else if (aes::Impl_SSL::okay())
			{ return funcs<aes::Impl_SSL>::make(key, vs); }
		}
		default:
			return funcs<aes::AES_FALLBACK>::make(key, vs);
	}
}

bool aes_impl_okay(aes_impl impl)
{
	switch (impl)
	{
		case ::aes_aesni:
			return aes::Impl_AESNI::okay();
		case ::aes_ssl:
			return aes::Impl_SSL::okay();
		case ::aes_sslaes:
			return aes::Impl_SSLAES::okay();
		case ::aes_sse:
			return aes::Impl_SSE::okay();
		default:
			return 1;
	}
}

CPP_EXTERNC_END
