//NOTE: no difference between different nans
//all nans converted into non-signalling nan
//-0 will be stored as 0
//fp notes:
//
//[sign bit][exp bits][mantissa]
//	sign bit: 1 = negative, 0 = positive
//	exp bits: 1 = exp_min, 0xFFFFFF... - 1 = exp_max
//	mantissa: leading (unstored) 1 if exp bits, else none
//
//	only mant_dig - 1 digits are stored (implicit 1 when exp > 0).
//
//
//exp: stored as 0: denormalized (no implied leading 1)
//               1 to 0xFF... - 1: normalized
//               0xFFFF... inf/nan
//
//
//	normal_mantissa is 0b1.xxxxx but frexp_mantissa is 0b0.xxxxx.
//	As a result, normal_exp = frexp_exp - 1.
//	normal_exp_min = 1 = normal_exp_max.
//
//	IEEE stores normal_exp and normal_mantissa.
//
//	frexp:
//		stored exp = frexp_exp - 1 + normal_exp_max
//
//	If given frexp exp,
//	the stored exp is exp + max - 1
//
//	NOTE: for denormalized, exponent is the same as normal_exp_min, just no leading 1
//	in other words, thinking of denormalized numbers:
//	exp = 1 => exponent is exp_min
//	and mantissa is 
//	exp+mantissa
//
//	ex: float32  exp_min       exp_max  stored_exp      mantissa     stored
//	normal:      -126          127      exp+127         1.1xxxxx     nmant * (2^23), round last dig
//	frexp:       -125          128      frexp+126       0.1xxxxx     nmant * (2^24), round last dig
//
//	examples:
//	denormalized:
//		0x00000001
//		decimal: 1.401298464324817e-45
//		
//
#include "serial/serial.h"
#include "serial/tlog.hpp"

#include <cmath>
#include <cstring>
#include <climits>
#include <cstdint>
#include <limits>



namespace
{
	static const std::size_t Byte_Bits = 8;
	//mant_dig: full digits, unnormalized
	//exp_min: smallest valid exponent
	//mantissa is viewed as .1xxx
	struct fp32info
	{
		typedef float fp_type;
		typedef uint_least32_t utype;
		static const unsigned int nbytes = 4;
		static const unsigned int mant_dig = 24;
		static const unsigned int exp_dig = 8;
		static const int frexp_min = -125;
		static const int frexp_max = 128;
		static const int frexp_offset = 126;
		static const int exp_min = -126;
		static const int exp_max = 127;

		static const unsigned char qnan[];
		static const unsigned char inf[];
		static const float infv;
		static const float nanv;
		static void storeu(unsigned char *data, utype v)
		{ serial_store_ui32(data, v); }
		static utype loadu(const unsigned char *data)
		{ return serial_load_ui32(data); }
	};
	const unsigned char fp32info::qnan[] = "\x7F\xC0\x00\x00";
	const unsigned char fp32info::inf[] = "\x7F\x80\x00\x00";
	const float fp32info::infv = std::numeric_limits<float>::infinity();
	const float fp32info::nanv = std::nanf("0");

	struct fp64info
	{
		typedef double fp_type;
		typedef uint_least64_t utype;
		static const unsigned int nbytes = 8;
		static const unsigned int mant_dig = 53;
		static const unsigned int exp_dig = 11;
		static const int frexp_max = 1024;
		static const int frexp_min = -1021;
		static const int frexp_offset = 1022;
		static const int exp_max = 1023;
		static const int exp_min = -1022;

		static const unsigned char qnan[];
		static const unsigned char inf[];
		static const double infv;
		static const double nanv;
		static void storeu(unsigned char *data, utype v)
		{ serial_store_ui64(data, v); }
		static utype loadu(const unsigned char *data)
		{ return serial_load_ui64(data); }
	};
	const unsigned char fp64info::qnan[] = "\x7F\xF8\x00\x00\x00\x00\x00\x00";
	const unsigned char fp64info::inf[] = "\x7F\xF0\x00\x00\x00\x00\x00\x00";
	const double fp64info::infv = std::numeric_limits<double>::infinity();
	const double fp64info::nanv = std::nan("0");


	template<class T>
	void store_fp(unsigned char *data, typename T::fp_type value)
	{
		cerr << "src fp store: " << value << endl;
		if (std::isnan(value))
		{
			if (std::signbit(value)) { data[0] |= 0x80u; }
			std::memcpy(data, T::qnan, T::nbytes);
		}
		else if (std::isinf(value))
		{
			std::memcpy(data, T::inf, T::nbytes);
			if (value < 0) { data[0] |= 0x80u; }
		}
		else if (value == 0)
		{ std::memset(data, 0, T::nbytes); }
		else
		{
			typedef typename T::utype utype;
			utype sign = value < 0;
			sign <<= (Byte_Bits * T::nbytes) - 1;
			if (sign)
			{ value *= -1; }

			int exp;
			typename T::fp_type mant = std::frexp(value, &exp);
			if (exp > T::frexp_max)
			{
				std::memcpy(data, T::inf, T::nbytes);
				if (sign) { data[0] |= 0x80u; }
			}
			else
			{
				exp += T::frexp_offset;
				if (exp > 0)
				{
					//normalized number
					utype normed = static_cast<utype>(
						std::ldexp(mant, T::mant_dig))
						& ((static_cast<utype>(1u) << (T::mant_dig-1))-1);
					T::storeu(
						data, sign | normed | (static_cast<utype>(exp) << (T::mant_dig-1)));
				}
				else
				{
					//denormalized number
					utype mantissa = static_cast<utype>(
						std::ldexp(mant, T::mant_dig-1 + exp));
					T::storeu(data, sign | mantissa);
				}
			}
		}
	};
	template<class T>
	typename T::fp_type load_fp(const unsigned char *data)
	{
		cerr << "src fp load" << endl;
		typedef typename T::utype utype;
		const utype full_exp = (static_cast<utype>(1)<<T::exp_dig)-1;
		const utype mant_mask = (static_cast<utype>(1)<<(T::mant_dig-1))-1;

		utype bits = T::loadu(data);
		utype exp = (bits >> (T::mant_dig-1)) & full_exp;
		utype mant = bits & mant_mask;
		if (exp == full_exp)
		{
			if (mant)
			{ return T::nanv; }
			else
			{ return (data[0] >> 7) ? -T::infv : T::infv; }
		}
		else
		{
			int fr_exp;
			if (exp)
			{
				mant |= static_cast<utype>(1) << (T::mant_dig-1);
				fr_exp = static_cast<int>(exp) - T::frexp_offset;
			}
			else
			{ fr_exp = T::frexp_min; }
			auto ret = ldexp(
				static_cast<typename T::fp_type>(mant), fr_exp - T::mant_dig);
			return (data[0] >> 7) ? -ret : ret;
		}
	}
}


CPP_EXTERNC_BEGIN
#if !(SERIAL_USE_CPOPT && CHAR_BIT==8 && SYSINFO_32_FPCP)
void serial_store_fp32(void *data, float value)
{ store_fp<fp32info>(reinterpret_cast<unsigned char*>(data), value); }
float serial_load_fp32(const void *data)
{ return load_fp<fp32info>(reinterpret_cast<const unsigned char*>(data)); }
#endif

#if !(SERIAL_USE_CPOPT && CHAR_BIT==8 && SYSINFO_64_FPCP)
void serial_store_fp64(void *data, double value)
{ store_fp<fp64info>(reinterpret_cast<unsigned char*>(data), value); }
double serial_load_fp64(const void *data)
{ return load_fp<fp64info>(reinterpret_cast<const unsigned char*>(data)); }
#endif


CPP_EXTERNC_END
