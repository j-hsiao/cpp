//AES codecs
#ifndef AES_HPP
#define AES_HPP
#include "aes/aes_dllcfg.h"
#include "aes/aes.h"
#include "aes/defs.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
namespace aes
{
	template<class Imp>
	struct ECB: public Imp
	{
		using Imp::Imp;
		std::size_t encrypt(void *dst, const void *src, std::size_t sz) const
		{ return this->encrypt_ecb(dst, src, sz); }
		std::size_t decrypt(void *dst, const void *src, std::size_t sz) const
		{ return this->decrypt_ecb(dst, src, sz); }
	};
	template<class Imp>
	struct CBC: public Imp
	{
		using Imp::Imp;
		std::size_t encrypt(void *dst, const void *src, std::size_t nbytes) const
		{ return this->encrypt_cbc(dst, src, nbytes); }
		std::size_t decrypt(void *dst, const void *src, std::size_t nbytes) const
		{ return this->decrypt_cbc(dst, src, nbytes); }
	};

	struct Codec
	{
		virtual std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const = 0;
		virtual std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const = 0;
		virtual std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const = 0;
		virtual std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const = 0;
		virtual ~Codec(){}
	};

	std::shared_ptr<Codec> getcodec(const void *key, const Version &v);
	namespace compat
	{
		struct Version
		{
			static const ::aes_version aes128 = ::aes128;
			static const ::aes_version aes192 = ::aes192;
			static const ::aes_version aes256 = ::aes256;
		};
		struct Impl
		{
			static const ::aes_impl ssl = ::aes_ssl;
			static const ::aes_impl sslaes = ::aes_sslaes;
			static const ::aes_impl aesni = ::aes_aesni;
			static const ::aes_impl sse = ::aes_sse;
			static const ::aes_impl ui32 = ::aes_ui32;
			static const ::aes_impl uil32 = ::aes_uil32;
			static const ::aes_impl plain = ::aes_plain;
			static const ::aes_impl automatic = ::aes_auto;
			static const ::aes_impl rawui32 = ::aes_rawui32;
			static const ::aes_impl rawuil32 = ::aes_rawuil32;
			static const ::aes_impl ui32_t = ::aes_ui32_t;
			static const ::aes_impl uil32_t = ::aes_uil32_t;
		};

		struct Codec
		{
			::aes_Codec codec;
			Codec(const void *key, ::aes_version v, ::aes_impl impl=Impl::automatic):
				codec{::aes_Codec_new(key, v, impl)}
			{
				if (!codec.pimpl)
				{ throw std::runtime_error("failed to create impl"); }
			}
			~Codec() { if (codec.pimpl) { codec.free(codec.pimpl); } }
			Codec(const Codec&) = delete;
			Codec(Codec&&) = default;

			std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const
			{ return codec.encrypt_ecb(codec.pimpl, dst, src, sz); }
			std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const
			{ return codec.decrypt_ecb(codec.pimpl, dst, src, sz); }
			std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const
			{ return codec.encrypt_cbc(codec.pimpl, dst, src, sz); }
			std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const
			{ return codec.decrypt_cbc(codec.pimpl, dst, src, sz); }
		};
	}
}
// include the implementation(s) you want to use:
// #include "aes/plain.hpp"
// #include "aes/ui32.hpp"
// #include "aes/uil32.hpp"
// #include "aes/aesni.hpp"
// #include "aes/sse.hpp"
// #include "aes/ssl.hpp"
// #include "aes/sslaes.hpp"
#endif
