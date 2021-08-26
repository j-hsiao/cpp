#include <serial/serial.h>

#include <cmath>
#include <cstring>
#include <climits>
#include <cstdint>
#include <limits>

namespace
{
	const int ieee_flt_mant_dig = 24;
	const int ieee_flt_exp_dig = 8;
	const int ieee_flt_exp_min = -125;

	const int ieee_dbl_mant_dig = 53;
	const int ieee_dbl_exp_dig = 11;
	const int ieee_dbl_exp_min = -1021;

	const unsigned char fqnan[] = "\x7F\xC0\x00\x00";
	const unsigned char finf[] = "\x7F\x80\x00\x00";

	const unsigned char dqnan[] = "\x7F\xF8\x00\x00\x00\x00\x00\x00";
	const unsigned char dinf[] = "\x7F\xF0\x00\x00\x00\x00\x00\x00";


	template<
		class F, class M, int tbytes,
		int mant_dig, int exp_dig, int min_exp,
		M mantmask, void ustore(unsigned char *, M)
	>
	void store_fp(
		unsigned char *data,
		F value,
		const unsigned char *nanbytes,
		const unsigned char *infbytes
	)
	{
		if (std::isnan(value))
		{ std::memcpy(data, nanbytes, tbytes); }
		else if (std::isinf(value))
		{
			std::memcpy(data, infbytes, tbytes);
			if (value < 0) { data[0] |=  0x80; }
		}
		else if (value == 0)
		{ std::memset(data, 0, tbytes); }
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
			F mant = std::frexp(value, &exp);
			exp -= min_exp;
			M storem;
			if (exp < 0)
			{
				storem = static_cast<M>(
					std::ldexp(mant, mant_dig + exp));
				exp = 0;
			}
			else
			{
				exp += 1;
				storem = static_cast<M>(std::ldexp(mant, mant_dig)) & mantmask;
			}
			data[0] = sign 
			int b_idx = 1;
			expremain
			while (b_idx < tbytes * 8)
			{
				data[b_idx / 8] |= 
			}
		}
	}
}


CPP_EXTERNC_BEGIN
void serial__store_fp32(unsigned char *data, float value)
{
#ifdef UINT32_MAX
	if (
		std::numeric_limits<float>::is_iec559()
		and sizeof(float) == 4
		and CHAR_BIT == 8
	)
	{
		// Side note... do I need to bother with endianness?
		// From what I've read with endianness, float endianness
		// does not necessarily match integer endianness or something
		// but not really sure... I guess add that handling if
		// the tests fail or something.
		uint_least32_t asint;
		std::memcpy(asint, value, sizeof(float));
		serial__store_ui32(data, asint);
	}
	else
	{
#endif
		if (std::isnan(value))
		{ std::memcpy(data, fqnan, 4); }
		else if (std::isinf(value))
		{
			std::memcpy(data, finf, 4);
			if (value < 0) { data[0] |=  0x80; }
		}
		else if (value == 0)
		{ std::memset(data, 0, tbytes); }
#ifdef UINT32_MAX
	}
#endif
}
float serial__load_fp32(const unsigned char *data)
{
	if (std::numeric_limits<float>::is_iec559())
	{
	}
	else
	{
		//calculate via frexp/ldexp
	}
}

void serial__store_fp64(unsigned char *data, double value)
{
	if (std::numeric_limits<double>::is_iec559())
	{
	}
	else
	{
		//calculate via frexp/ldexp
	}
}
double serial__load_fp64(const unsigned char *data)
{
	if (std::numeric_limits<double>::is_iec559())
	{
	}
	else
	{
		//calculate via frexp/ldexp
	}
}
CPP_EXTERNC_END
