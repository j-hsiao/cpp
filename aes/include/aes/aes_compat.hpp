#ifndef AES_AES_HPP
#define AES_AES_HPP
#include "aes/aes.h"

#include <cstddef>
#include <stdexcept>
#include <string>

namespace aes { namespace compat
{
	static const std::size_t Padding = 16;

	enum class Version:int 
	{ v128=aes_128, v192=aes_192, v256=aes_256 };

	enum class Implementation: int
	{
		ssl=aes_ssl,
		sslaes=aes_sslaes,
		aesni=aes_aesni,
		sse=aes_sse,
		ui32=aes_ui32,
		uil32=aes_uil32,
		plain=aes_plain
	};

	struct Codec
	{
		public:
			template<class T>
			Codec(const T &key, Version v, Implementation impl=Implementation::ssl):
				base(
					aes_Codec_new(
						&key[0], static_cast<aes_version>(v),
						static_cast<aes_impl>(impl)))
			{}
			~Codec(){ base.release(base.pimpl); }

			//new destination
			template<class T>
			T encrypt_ecb(const T &data) const;
			template<class T>
			T decrypt_ecb(const T &data) const;
			template<class T>
			T encrypt_cbc(const T &data) const;
			template<class T>
			T decrypt_cbc(const T &data) const;

			//inplace
			template<class T>
			T& encrypt_ecb(T &dst, const T &src) const;
			template<class T>
			T& decrypt_ecb(T &dst, const T &src) const;
			template<class T>
			T& encrypt_cbc(T &dst, const T &src) const;
			template<class T>
			T& decrypt_cbc(T &dst, const T &src) const;
		private:
			aes_Codec base;
	};
}}
//impls
namespace aes
{
	template<class T>
	T Codec::encrypt_ecb(const T &data) const
	{
		T dst;
		return encrypt_ecb(dst, data);
	}
	template<class T>
	T Codec::decrypt_ecb(const T &data) const
	{
		T dst;
		return decrypt_ecb(dst, data);
	}
	template<class T>
	T Codec::encrypt_cbc(const T &data) const
	{
		T dst;
		return encrypt_cbc(dst, data);
	}
	template<class T>
	T Codec::decrypt_cbc(const T &data) const
	{
		T dst;
		return decrypt_cbc(dst, data);
	}

	template<class T>
	T& Codec::encrypt_ecb(T &dst, const T &data) const
	{
		dst.resize(data.size() + Padding);
		dst.resize(base.encrypt_ecb(base.pimpl, &dst[0], &data[0], data.size()));
		if (!dst.size()) { throw std::runtime_error("encryption failed"); }
		return dst;
	}
	template<class T>
	T& Codec::decrypt_ecb(T &dst, const T &data) const
	{
		dst.resize(data.size());
		dst.resize(base.decrypt_ecb(base.pimpl, &dst[0], &data[0], data.size()));
		if (dst.size() == data.size()) { throw std::runtime_error("decryption failed"); }
		return dst;
	}
	template<class T>
	T& Codec::encrypt_cbc(T &dst, const T &data) const
	{
		dst.resize(data.size() + Padding);
		dst.resize(base.encrypt_cbc(base.pimpl, &dst[0], &data[0], data.size()));
		if (!dst.size()) { throw std::runtime_error("encryption failed"); }
		return dst;
	}
	template<class T>
	T& Codec::decrypt_cbc(T &dst, const T &data) const
	{
		dst.resize(data.size());
		dst.resize(base.decrypt_cbc(base.pimpl, &dst[0], &data[0], data.size()));
		if (dst.size() == data.size()) { throw std::runtime_error("decryption failed"); }
		return dst;
	}
}
#endif
