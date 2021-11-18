#ifndef SERIAL_HPP
#define SERIAL_HPP

#include "serial/serial.h"

#include <cstddef>
#include <climits>
#include <cstdint>
namespace serial
{
	enum class tp { Signed, Unsigned, Float };

	template<int nbits>
	struct Loadcfg
	{
		static std::size_t nbytes(std::size_t num_elems=1)
		{ return num_elems * (nbits / 8); }
	};


	template<tp, int nbits>
	struct Loader;

	template<>
	struct Loader<tp::Signed, 16>: Loadcfg<16>
	{
		typedef std::int_least16_t value_type;
		static void store(void *buf, value_type value)
		{ serial_store_i16(buf, value); }
		static value_type load(const void *buf)
		{ return serial_load_i16(buf); }
	};
	template<>
	struct Loader<tp::Signed, 32>: Loadcfg<32>
	{
		typedef std::int_least32_t value_type;
		static void store(void *buf, value_type value)
		{ serial_store_i32(buf, value); }
		static value_type load(const void *buf)
		{ return serial_load_i32(buf); }
	};
	template<>
	struct Loader<tp::Signed, 64>:Loadcfg<64>
	{
		typedef std::int_least64_t value_type;
		static void store(void *buf, value_type value)
		{ serial_store_i64(buf, value); }
		static value_type load(const void *buf)
		{ return serial_load_i64(buf); }
	};

	template<>
	struct Loader<tp::Unsigned, 16>:Loadcfg<16>
	{
		typedef std::uint_least16_t value_type;
		static void store(void *buf, value_type value)
		{ serial_store_ui16(buf, value); }
		static value_type load(const void *buf)
		{ return serial_load_ui16(buf); }
	};
	template<>
	struct Loader<tp::Unsigned, 32>:Loadcfg<32>
	{
		typedef std::uint_least32_t value_type;
		static void store(void *buf, value_type value)
		{ serial_store_ui32(buf, value); }
		static value_type load(const void *buf)
		{ return serial_load_ui32(buf); }
	};
	template<>
	struct Loader<tp::Unsigned, 64>:Loadcfg<64>
	{
		typedef std::uint_least64_t value_type;
		static void store(void *buf, value_type value)
		{ serial_store_ui64(buf, value); }
		static value_type load(const void *buf)
		{ return serial_load_ui64(buf); }
	};

	template<>
	struct Loader<tp::Float, 32>:Loadcfg<32>
	{
		typedef float value_type;
		static void store(void *buf, value_type value)
		{ serial_store_fp32(buf, value); }
		static value_type load(const void *buf)
		{ return serial_load_fp32(buf); }
	};
	template<>
	struct Loader<tp::Float, 64>:Loadcfg<64>
	{
		typedef double value_type;
		static void store(void *buf, value_type value)
		{ serial_store_fp64(buf, value); }
		static value_type load(const void *buf)
		{ return serial_load_fp64(buf); }
	};

	//range-based loading/storing
	template<tp type, int nbits, class T>
	void store(void *b, T begin, T end)
	{
		auto buf = reinterpret_cast<unsigned char*>(b);
		for (; begin != end; ++begin)
		{
			Loader<type, nbits>::store(
				buf, static_cast<typename Loader<type, nbits>::value_type>(*begin));
			buf += Loader<type, nbits>::nbytes();
		}
	}
	template<tp type, int nbits, class T>
	void load(const void *b, T begin, T end)
	{
		auto buf = reinterpret_cast<const unsigned char*>(b);
		for (; begin != end; ++begin)
		{
			*begin = Loader<type, nbits>::load(buf);
			buf += Loader<type, nbits>::nbytes();
		}
	}


}
#endif
