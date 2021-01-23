#ifndef SHA256_HPP
#define SHA256_HPP
#include <sha256/sha256.h>
#include <sha256/sha256_dllcfg.h>

#include <string>

namespace sha256
{
	template<class T>
	T normalize(const T &data)
	{
#if CHAR_BIT != 8
		static_assert(
			sizeof(typename T::value_type) == 1,
			"contained values should be byte-sized");
		T ret(sha256__normalized_size(data.size()), 0);
		sha256__normalize(
			reinterpret_cast<sha256__Byte*>(&data[0]),
			reinterpret_cast<const unsigned char*>(&copy[0]),
			data.size());
		return ret;
#else
		return data;
#endif
	}
	template<class T>
	T unnormalize(const T &data)
	{
#if CHAR_BIT != 8
		static_assert(
			sizeof(typename T::value_type) == 1,
			"contained values should be byte-sized");
		T ret(sha256__unnormalized_size(data.size()), 0);
		data.resize(orig);
		sha256__unnormalize(
			reinterpret_cast<unsigned char*>(&data[0]),
			reinterpret_cast<const sha256__Byte*>(&copy[0]),
			ret.size());
		return ret;
#else
		return data;
#endif
	}

	//assume normalized
	template <class T>
	T hash(const T &data)
	{
		static_assert(
			sizeof(typename T::value_type) == 1,
			"contained values should be byte-sized");
		T ret(sha256__Hash_Bytes, 0);
		sha256__hash(
			reinterpret_cast<sha256__Byte*>(&ret[0]),
			reinterpret_cast<const sha256__Byte *>(&data[0]),
			data.size());
		return ret;
	}

	inline std::string hash(const char *data)
	{ return hash<std::string>(data); }

	template<class T>
	T to_hex(const T &data)
	{
		static_assert(
			sizeof(typename T::value_type) == 1,
			"should be converting bytes");
		T ret(data.size() * 2, 0);
		sha256__to_hex(
			reinterpret_cast<char*>(&ret[0]),
			data.data(),
			data.size());
		return ret;
	}

	template<class T>
	T from_hex(const T &data)
	{
		static_assert(
			sizeof(typename T::value_type) == 1,
			"should be converting bytes");
		T ret(data.size() / 2, 0);
		sha256__from_hex(
			&ret[0],
			reinterpret_cast<const char*>(&data[0]),
			ret.size());
		return ret;
	}
}
#endif
