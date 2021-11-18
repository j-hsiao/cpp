#ifndef SHA256_HPP
#define SHA256_HPP
#include <sha256/sha256.h>
#include <sha256/sha256_dllcfg.h>

#include <string>
#include <stdexcept>

namespace sha256
{
	//assume normalized
	template <class T>
	T hash(const T &data)
	{
		static_assert(
			sizeof(typename T::value_type) == 1,
			"contained values should be byte-sized");
		T ret(sha256_Hash_Bytes, 0);
		sha256_hash(
			reinterpret_cast<unsigned char*>(&ret[0]),
			reinterpret_cast<const unsigned char *>(&data[0]),
			data.size());
		return ret;
	}

	inline std::string hash(const char *data)
	{ return hash<std::string>(data); }

	template<class T>
	T to_hex(const T &data)
	{
		typedef typename T::value_type out;
		static_assert(sizeof(out) == 1, "should be converting bytes");
		T ret;
		ret.reserve(data.size() * 2);
		const unsigned char hx[] = "0123456789abcdef";
		for (out thing : data)
		{
			ret.push_back(static_cast<out>(hx[thing>>4 & 0xfu]));
			ret.push_back(static_cast<out>(hx[thing&0xfu]));
		}
		return ret;
	}

	template<class T>
	T from_hex(const T &data)
	{
		typedef typename T::value_type out;
		static_assert(sizeof(out) == 1, "should be converting bytes");
		T ret;
		ret.reserve(data.size() / 2);
		unsigned char hx[] = "0123456789abcdef";
		unsigned char HX[] = "0123456789ABCDEF";
		unsigned char re[256];
		for (std::size_t i=0; i<256; ++i)
		{ re[i] = 0xffu; }
		for (unsigned char i=0; i<16; ++i)
		{
			re[hx[i]] = i;
			re[HX[i]] = i;
		}
		for (std::size_t i=0; i<data.size(); i+=2)
		{
			unsigned char hi = re[static_cast<unsigned char>(data[i])];
			unsigned char lo = re[static_cast<unsigned char>(data[i+1])];
			if (hi == 0xffu || lo == 0xffu)
			{ throw std::runtime_error("bad hex character"); }
			ret.push_back(static_cast<out>(hi<<4 | lo));
		}
		return ret;
	}
}
#endif
