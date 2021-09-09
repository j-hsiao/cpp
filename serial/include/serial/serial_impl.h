#ifndef SERIAL_IMPL_H
#define SERIAL_IMPL_H

#include <sysinfo/sysinfo.h>

#include <serial/serial_dllcfg.h>

#include <string.h>
#include <limits.h>

#include <iostream>


CPP_EXTERNC_BEGIN
inline void serial__swap_2B(unsigned char *dst, const unsigned char *src)
{
	dst[0] = src[1];
	dst[1] = src[0];
}
inline void serial__swap_4B(unsigned char *dst, const unsigned char *src)
{
	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];
}
inline void serial__swap_8B(unsigned char *dst, const unsigned char *src)
{
	dst[0] = src[7];
	dst[1] = src[6];
	dst[2] = src[5];
	dst[3] = src[4];
	dst[4] = src[3];
	dst[5] = src[2];
	dst[6] = src[1];
	dst[7] = src[0];
}

#if SYSINFO_ENDIAN == SYSINFO_BIG_ENDIAN
inline void serial__store_2B(unsigned char *dst, const unsigned char *src)
{ memcpy(dst, src, 2); }
inline void serial__store_4B(unsigned char *dst, const unsigned char *src)
{ memcpy(dst, src, 4); }
inline void serial__store_8B(unsigned char *dst, const unsigned char *src)
{ memcpy(dst, src, 8); }
#else
inline void serial__store_2B(unsigned char *dst, const unsigned char *src)
{ serial__swap_2B(dst, src); }
inline void serial__store_4B(unsigned char *dst, const unsigned char *src)
{ serial__swap_4B(dst, src); }
inline void serial__store_8B(unsigned char *dst, const unsigned char *src)
{ serial__swap_8B(dst, src); }
#endif


#define SERIAL_OPT (CHAR_BIT==8 && SYSINFO_ENDIAN!=SYSINFO_UNKNOWN_ENDIAN)

#if defined(UINT16_MAX) && SERIAL_OPT
inline void serial__store_ui16(unsigned char *data, uint_least16_t value)
{ serial__store_2B(data, (unsigned char*) &value); }
inline uint_least16_t serial__load_ui16(const unsigned char *data)
{
	uint_least16_t value;
	serial__store_2B((unsigned char*) &value, data);
	return value;
}
#else
inline void serial__store_ui16(unsigned char *data, uint_least16_t value)
{
	data[0] = (value >> 8) & 0xFFu;
	data[1] = value & 0xFFu;
}
inline uint_least16_t serial__load_ui16(const unsigned char *data)
{
	return (uint_least16_t) data[0] << 8 | data[1];
}
#endif


#if defined(UINT32_MAX) && SERIAL_OPT
inline void serial__store_ui32(unsigned char *data, uint_least32_t value)
{ serial__store_4B(data, (unsigned char*) &value); }
inline uint_least32_t serial__load_ui32(const unsigned char *data)
{
	uint_least32_t value;
	serial__store_4B((unsigned char*) &value, data);
	return value;
}
#else
inline void serial__store_ui32(unsigned char *data, uint_least32_t value)
{
	data[0] = (value >> 24) & 0xFFu;
	data[1] = (value >> 16) & 0xFFu;
	data[2] = (value >> 8) & 0xFFu;
	data[3] = value & 0xFFu;
}
inline uint_least32_t serial__load_ui32(const unsigned char *data)
{
	return (
		(uint_least32_t) data[0] << 24 | (uint_least32_t) data[1] << 16
		| (uint_least32_t) data[2] << 8 | data[3]);
}
#endif


#if defined(UINT64_MAX) && SERIAL_OPT
inline void serial__store_ui64(unsigned char *data, uint_least64_t value)
{ serial__store_8B(data, (unsigned char*) &value); }
inline uint_least64_t serial__load_ui64(const unsigned char *data)
{
	uint_least64_t value;
	serial__store_8B((unsigned char*) &value, data);
	return value;
}
#else
inline void serial__store_ui64(unsigned char *data, uint_least64_t value)
{
	data[0] = (value >> 56) & 0xFFu;
	data[1] = (value >> 48) & 0xFFu;
	data[2] = (value >> 40) & 0xFFu;
	data[3] = (value >> 32) & 0xFFu;
	data[4] = (value >> 24) & 0xFFu;
	data[5] = (value >> 16) & 0xFFu;
	data[6] = (value >> 8) & 0xFFu;
	data[7] = value & 0xFFu;
}

inline uint_least64_t serial__load_ui64(const unsigned char *data)
{
	return (
		(uint_least64_t) data[0] << 56
		| (uint_least64_t) data[1] << 48
		| (uint_least64_t) data[2] << 40
		| (uint_least64_t) data[3] << 32
		| (uint_least64_t) data[4] << 24
		| (uint_least64_t) data[5] << 16
		| (uint_least64_t) data[6] << 8
		| data[7]);
}
#endif

#if SYSINFO_IREP == SYSINFO_TWO_IREP
//already two's complement

inline void serial__store_i16(unsigned char *data, int_least16_t value)
{ serial__store_ui16(data, *((uint_least16_t*) &value)); }
inline void serial__store_i32(unsigned char *data, int_least32_t value)
{ serial__store_ui32(data, *((uint_least32_t*) &value)); }
inline void serial__store_i64(unsigned char *data, int_least64_t value)
{ serial__store_ui64(data, *((uint_least64_t*) &value)); }
inline int_least16_t serial__load_i16(const unsigned char *data)
{
	uint_least16_t v = serial__load_ui16(data);
	return *((int_least16_t*) &v);
}
inline int_least32_t serial__load_i32(const unsigned char *data)
{
	uint_least32_t v = serial__load_ui32(data);
	return *((int_least32_t*) &v);
}
inline int_least64_t serial__load_i64(const unsigned char *data)
{
	uint_least64_t v = serial__load_ui64(data);
	return *((int_least64_t*) &v);
}

#else
//positive = same as twos complement
inline void serial__store_i16(unsigned char *data, int_least16_t value)
{
	if (value > 0)
	{ serial__store_ui16(data, *((uint_least16_t*) &value)); }
	else
	{ serial__store_ui16(data, ~(uint_least16_t)(-1 - value)); }
}
inline void serial__store_i32(unsigned char *data, int_least32_t value)
{
	if (value > 0)
	{ serial__store_ui32(data, *((uint_least32_t*) &value)); }
	else
	{ serial__store_ui32(data, ~(uint_least32_t)(-1 - value)); }
}
inline void serial__store_i64(unsigned char *data, int_least64_t value)
{
	if (value > 0)
	{ serial__store_ui64(data, *((uint_least64_t*) &value)); }
	else
	{ serial__store_ui64(data, ~(uint_least64_t)(-1 - value)); }
}

inline int_least16_t serial__load_i16(const unsigned char *data)
{
	uint_least16_t v = serial__load_ui16(data);
	if (v >> 15)
	{ return -1 - (int_least16_t) ~v; }
	return v;
}
inline int_least32_t serial__load_i32(const unsigned char *data)
{
	uint_least32_t v = serial__load_ui32(data);
	if (v >> 31)
	{ return -1 - (int_least32_t) ~v; }
	return v;
}
inline int_least64_t serial__load_i64(const unsigned char *data)
{
	uint_least64_t v = serial__load_ui64(data);
	if (v >> 63)
	{ return -1 - (int_least64_t) ~v; }
	return v;
}
#endif

#if SYSINFO_CPFP32
inline void serial__store_fp32(unsigned char *data, float value)
{ serial__store_4B(data, (unsigned char*) &value); }
inline float serial__load_fp32(const unsigned char *data)
{
	float value;
	serial__store_4B((unsigned char*) &value, data);
	return value;
}
#endif

#if SYSINFO_CPFP64
inline void serial__store_fp64(unsigned char *data, double value)
{ serial__store_8B(data, (unsigned char*) &value); }
inline double serial__load_fp64(const unsigned char *data)
{
	double value;
	serial__store_8B((unsigned char*) &value, data);
	return value;
}
#endif

CPP_EXTERNC_END

#endif