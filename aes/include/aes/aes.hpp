#ifndef AES_HPP
#define AES_HPP
#include <aes/aes.h>

#include <cstddef>
#include <string>

#include <iostream>

namespace aes
{
	typedef aes__Version Version;

	inline std::size_t padding(std::size_t datalength);

	struct Codec
	{
		public:
			Codec(const std::string &key, Version v);
			std::string encrypt_ecb(const std::string &data) const;
			std::string decrypt_ecb(const std::string &data) const;
			std::string encrypt_cbc(const std::string &data) const;
			std::string decrypt_cbc(const std::string &data) const;

			//inplace
			void iencrypt_ecb(std::string &data) const;
			void idecrypt_ecb(std::string &data) const;
			void iencrypt_cbc(std::string &data) const;
			void idecrypt_cbc(std::string &data) const;
		private:
			aes__Keys keys;
	};
}

//impls
namespace aes
{
	inline std::size_t padding(std::size_t datalength)
	{ return aes__padding(datalength); }

	inline Codec::Codec(const std::string &key, Version version)
	{
		aes__init_roundkeys(
			&keys, reinterpret_cast<const aes__Byte*>(key.c_str()), version);
	}

	inline std::string Codec::encrypt_ecb(const std::string &data) const
	{
		std::string cp = data;
		iencrypt_ecb(cp);
		return cp;
	}
	inline std::string Codec::decrypt_ecb(const std::string &data) const
	{
		std::string cp = data;
		idecrypt_ecb(cp);
		return cp;
	}
	inline std::string Codec::encrypt_cbc(const std::string &data) const
	{
		std::string cp = data;
		iencrypt_cbc(cp);
		return cp;
	}
	inline std::string Codec::decrypt_cbc(const std::string &data) const
	{
		std::string cp = data;
		idecrypt_cbc(cp);
		return cp;
	}


	inline void Codec::iencrypt_ecb(std::string &data) const
	{
		std::size_t rawsize = data.size();
		data.resize(rawsize + padding(rawsize));
		std::size_t outsize = aes__encrypt_ecb(
			reinterpret_cast<aes__Byte*>(&data[0]),
			reinterpret_cast<const aes__Byte*>(data.c_str()),
			&keys, rawsize);
		if (outsize == aes__Invalid) { throw std::runtime_error("encrypt: invalid data"); }
		else if (outsize != data.size()) { throw std::runtime_error("encrypt: unexpected output size"); }
	}

	inline void Codec::idecrypt_ecb(std::string &data) const
	{
		std::size_t outsize = aes__decrypt_ecb(
			reinterpret_cast<aes__Byte*>(&data[0]),
			reinterpret_cast<const aes__Byte*>(data.c_str()),
			&keys, data.size());
		if (outsize == aes__Invalid) { throw std::runtime_error("decrypt: invalid data"); }
		data.resize(outsize);
	}

	inline void Codec::iencrypt_cbc(std::string &data) const
	{
		std::size_t rawsize = data.size();
		data.resize(rawsize + padding(rawsize));
		std::size_t outsize = aes__encrypt_cbc(
			reinterpret_cast<aes__Byte*>(&data[0]),
			reinterpret_cast<const aes__Byte*>(data.c_str()),
			&keys, rawsize);
		if (outsize == aes__Invalid) { throw std::runtime_error("encrypt: invalid data"); }
		else if (outsize != data.size()) { throw std::runtime_error("encrypt: unexpected output size"); }
	}

	inline void Codec::idecrypt_cbc(std::string &data) const
	{
		std::size_t outsize = aes__decrypt_cbc(
			reinterpret_cast<aes__Byte*>(&data[0]),
			reinterpret_cast<const aes__Byte*>(data.c_str()),
			&keys, data.size());
		if (outsize == aes__Invalid) { throw std::runtime_error("decrypt: invalid data"); }
		data.resize(outsize);
	}
}
#endif
