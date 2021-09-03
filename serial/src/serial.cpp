#include <serial/serial.h>

#include <cmath>
#include <cstring>
#include <climits>
#include <cstdint>
#include <limits>

#if SERIAL_DEBUG
#include <iostream>
#endif

namespace
{
#if SERIAL_DEBUG
	std::ostream& printhex(unsigned char thing)
	{
		const char *hex = "0123456789abcdef";
		std::cerr << hex[thing >> 4] << hex[thing & 0x0Fu];
		return std::cerr;
	}
#endif

	struct fp32info
	{
		typedef float fp_type;
		typedef uint_least32_t mant_type;

		static const int sexp = 0xFF;
		static const unsigned char qnan[];
		static const unsigned char inf[];
		static const float infv;
		static const float nanv;
		static const int nbytes = 4;
		static const int mant_dig = 24;
		static const int exp_dig = 8;
		static const int exp_min = -125;
#ifdef UINT32_MAX
		static const bool exact = 0;
#else
		static const bool exact = 0;
#endif

		static void merge(
			unsigned char *data, unsigned char sign,
			int exp, uint_least32_t mantissa);
		static void split(
			const unsigned char *data, unsigned char &sign,
			int &exp, uint_least32_t &mantissa);
		static void ustore(unsigned char *data, uint_least32_t v)
		{ serial__store_ui32(data, v); }
		static uint_least32_t uload(const unsigned char *data)
		{ return serial__load_ui32(data); }
	};

	struct fp64info
	{
		typedef double fp_type;
		typedef uint_least64_t mant_type;

		static const int sexp = 0x7FF;
		static const unsigned char qnan[];
		static const unsigned char inf[];
		static const double infv;
		static const double nanv;
		static const int nbytes = 8;
		static const int mant_dig = 53;
		static const int exp_dig = 11;
		static const int exp_min = -1021;
#ifdef UINT64_MAX
		static const bool exact = 0;
#else
		static const bool exact = 0;
#endif


		static void merge(
			unsigned char *data, unsigned char sign,
			int exp, uint_least64_t mantissa);
		static void split(
			const unsigned char *data, unsigned char &sign,
			int &exp, uint_least64_t &mantissa);
		static void ustore(unsigned char *data, uint_least64_t v)
		{ serial__store_ui64(data, v); }
		static uint_least64_t uload(const unsigned char *data)
		{ return serial__load_ui64(data); }
	};

	const float fp32info::infv = 1.0f / 0.0f;
	const float fp32info::nanv = 0.0f / 0.0f;
	const unsigned char fp32info::qnan[] = "\x7F\xC0\x00\x00";
	const unsigned char fp32info::inf[] = "\x7F\x80\x00\x00";
	void fp32info::merge(
		unsigned char *data, unsigned char sign,
		int exp, uint_least32_t mantissa)
	{
		mantissa &= 0x7FFFFFu;
		unsigned int uexp = static_cast<unsigned int>(exp);
		data[0] = sign | uexp >> 1;
		data[1] = uexp << 7 | static_cast<unsigned char>(mantissa >> 16);
		data[2] = static_cast<unsigned char>(mantissa >> 8);
		data[3] = static_cast<unsigned char>(mantissa);
	}
	void fp32info::split(
		const unsigned char *data, unsigned char &sign,
		int &exp, uint_least32_t &mantissa)
	{
		sign = data[0] & 0x80u;
		exp = static_cast<unsigned char>(data[0]<<1 | data[1]>>7);
		mantissa = (
			static_cast<uint_least32_t>(data[1] & 0x7Fu) << 16
			| static_cast<uint_least32_t>(data[2]) << 8
			| static_cast<uint_least32_t>(data[3]));
	}

	const double fp64info::infv = 1.0 / 0.0;
	const double fp64info::nanv = 0.0 / 0.0;
	const unsigned char fp64info::qnan[] = "\x7F\xF8\x00\x00\x00\x00\x00\x00";
	const unsigned char fp64info::inf[] = "\x7F\xF0\x00\x00\x00\x00\x00\x00";
	void fp64info::merge(
		unsigned char *data, unsigned char sign,
		int exp, uint_least64_t mantissa)
	{
		mantissa &= 0xFFFFFFFFFFFFFull;
		unsigned int uexp = static_cast<unsigned int>(exp);
		data[0] = sign | uexp >> 4;
		data[1] = static_cast<unsigned char>(uexp<<4 | mantissa>>48);
		data[2] = static_cast<unsigned char>(mantissa >> 40);
		data[3] = static_cast<unsigned char>(mantissa >> 32);
		data[4] = static_cast<unsigned char>(mantissa >> 24);
		data[5] = static_cast<unsigned char>(mantissa >> 16);
		data[6] = static_cast<unsigned char>(mantissa >> 8);
		data[7] = static_cast<unsigned char>(mantissa);
	}
	void fp64info::split(
		const unsigned char *data, unsigned char &sign,
		int &exp, uint_least64_t &mantissa)
	{
		sign = data[0] & 0x80u;
		exp = (data[0]&0x7Fu)<<4 | data[1]>>4;
		mantissa = (
			static_cast<uint_least64_t>(data[1]&0x0Fu) << 48
			| static_cast<uint_least64_t>(data[2]) << 40
			| static_cast<uint_least64_t>(data[3]) << 32
			| static_cast<uint_least64_t>(data[4]) << 24
			| static_cast<uint_least64_t>(data[5]) << 16
			| static_cast<uint_least64_t>(data[6]) << 8
			| static_cast<uint_least64_t>(data[7]));
	}

	template<class info>
	void store_fp(unsigned char *data, typename info::fp_type value)
	{
		static_assert(
			info::nbytes==4 || info::nbytes==8, "Only 4 or 8 byte floats supported.");
#if SERIAL_DEBUG
		std::cerr << "frexp/ldexp decomposition" << std::endl;
#endif
		if (std::isnan(value))
		{ std::memcpy(data, info::qnan, info::nbytes); }
		else if (std::isinf(value))
		{
			std::memcpy(data, info::inf, info::nbytes);
			if (value < 0) { data[0] |=  0x80u; }
		}
		else if (value == 0)
		{ std::memset(data, 0, info::nbytes); }
		else
		{
			unsigned char sign;
			if (value < 0)
			{
				value *= -1;
				sign = 0x80u;
			}
			else
			{ sign = 0u; }
			int exp;
			typename info::fp_type mant = std::frexp(value, &exp);
			exp -= info::exp_min - 1;
			typename info::mant_type storem;
			if (exp <= 0)
			{
				storem = static_cast<typename info::mant_type>(
					std::ldexp(mant, info::mant_dig + exp - 1));
				exp = 0;
			}
			else
			{
				storem = static_cast<typename info::mant_type>(
					std::ldexp(mant, info::mant_dig));
			}
			info::merge(data, sign, exp, storem);
		}
	}

	template<class info>
	typename info::fp_type load_fp(const unsigned char * const data)
	{
		static_assert(
			info::nbytes==4 || info::nbytes==8, "Only 4 or 8 byte floats supported.");
#if SERIAL_DEBUG
		std::cerr << "frexp/ldexp recomposition" << std::endl;
		for (int bidx=0; bidx<info::nbytes; ++bidx)
		{
			printhex(data[bidx]) << " ";
		}
		std::cerr << std::endl;
#endif
		unsigned char sign;
		int exp;
		typename info::mant_type mantissa;
		info::split(data, sign, exp, mantissa);

		if (!exp && !mantissa)
		{ return sign ? -0.0f : 0.0f; }
		else if (exp == info::sexp)
		{
			return mantissa ? info::nanv : (sign ? -info::infv : info::infv);
		}
		else
		{
			if (exp)
			{
				exp -= 1;
				mantissa |= 1ull << info::mant_dig-1;
			}
			typename info::fp_type ret = std::ldexp(
				mantissa, exp + info::exp_min - info::mant_dig);
			if (sign) { ret *= -1; }
			return ret;
		}
	}
}


CPP_EXTERNC_BEGIN
#if !SERIAL_CPFP32
void serial__store_fp32(unsigned char *data, float value)
{ store_fp<fp32info>(data, value); }
float serial__load_fp32(const unsigned char *data)
{ return load_fp<fp32info>(data); }
#endif

#if !SERIAL_CPFP32
void serial__store_fp64(unsigned char *data, double value)
{ store_fp<fp64info>(data, value); }
double serial__load_fp64(const unsigned char *data)
{ return load_fp<fp64info>(data); }
#endif


CPP_EXTERNC_END
