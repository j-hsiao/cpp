#ifndef SERIAL_IMPL_H
#define SERIAL_IMPL_H

#include "serial/serial.h"
#include "serial/serial_dllcfg.h"

#include <limits.h>
#include <string.h>

CPP_EXTERNC_BEGIN
#if SERIAL_USE_CPOPT && CHAR_BIT == 8
# if (SYSINFO_ENDIAN == SYSINFO_LITTLE_ENDIAN \
	|| SYSINFO_32_FPCP == SYSINFO_REVERSE_FPCP \
	|| SYSINFO_64_FPCP == SYSINFO_REVERSE_FPCP)

	inline void serial_swap(void *dst_, const void *src_, const size_t count)
	{
		unsigned char *dst = (unsigned char *) dst_;
		const unsigned char *src = (unsigned char *) src_;
		for (size_t i=0; i<count; ++i)
		{ dst[i] = src[count-(i+1)]; }
	}

# endif

# if SYSINFO_ENDIAN
#  if SYSINFO_ENDIAN == SYSINFO_LITTLE_ENDIAN
#   define SERIAL_STOREU(a, b, c) serial_swap((a), (b), (c))
#  elif SYSINFO_ENDIAN
#   define SERIAL_STOREU(a, b, c) memcpy((a), (b), (c))
#  endif
#  define SERIAL_LOADU(tp, nbytes) tp ret; SERIAL_STOREU(&ret, data, nbytes); return ret

#  ifdef UINT16_MAX
		inline void serial_store_ui16(void *data, uint_least16_t value)
		{ SERIAL_STOREU(data, &value, 2); }
		inline uint_least16_t serial_load_ui16(const void *data)
		{ SERIAL_LOADU(uint_least16_t, 2); }
#  endif
#  ifdef UINT32_MAX
		inline void serial_store_ui32(void *data, uint_least32_t value)
		{ SERIAL_STOREU(data, &value, 4); }
		inline uint_least32_t serial_load_ui32(const void *data)
		{ SERIAL_LOADU(uint_least32_t, 4); }
#  endif
#  ifdef UINT64_MAX
		inline void serial_store_ui64(void *data, uint_least64_t value)
		{ SERIAL_STOREU(data, &value, 8); }
		inline uint_least64_t serial_load_ui64(const void *data)
		{ SERIAL_LOADU(uint_least64_t, 8); }
#  endif

#  if SYSINFO_IREP == SYSINFO_TWO_IREP
#   ifdef INT16_MAX
			inline void serial_store_i16(void *data, int_least16_t value)
			{ SERIAL_STOREU(data, &value, 2); }
			inline int_least16_t serial_load_i16(const void *data)
			{ SERIAL_LOADU(int_least16_t, 2); }
#   endif

#   ifdef INT32_MAX
			inline void serial_store_i32(void *data, int_least32_t value)
			{ SERIAL_STOREU(data, &value, 4); }
			inline int_least32_t serial_load_i32(const void *data)
			{ SERIAL_LOADU(int_least32_t, 4); }
#   endif

#   ifdef INT64_MAX
			inline void serial_store_i64(void *data, int_least64_t value)
			{ SERIAL_STOREU(data, &value, 8); }
			inline int_least64_t serial_load_i64(const void *data)
			{ SERIAL_LOADU(int_least64_t, 8); }
#   endif
#  endif

#  undef SERIAL_STOREU
#  undef SERIAL_LOADU
# endif

# if SYSINFO_32_FPCP
#  if SYSINFO_32_FPCP == SYSINFO_REVERSE_FPCP
#   define SERIAL_STOREFP32(a, b) serial_swap((a), (b), 4)
#  elif SYSINFO_32_FPCP
#   define SERIAL_STOREFP32(a, b) memcpy((a), (b), 4)
#  endif
	inline void serial_store_fp32(void *data, float value)
	{ SERIAL_STOREFP32(data, &value); }
	inline float serial_load_fp32(const void *data)
	{
		float ret;
		SERIAL_STOREFP32(&ret, data);
		return ret;
	}
#  undef SERIAL_STOREFP32
# endif

# if SYSINFO_64_FPCP
#  if SYSINFO_64_FPCP == SYSINFO_REVERSE_FPCP
#   define SERIAL_STOREFP64(a, b) serial_swap((a), (b), 8)
#  elif SYSINFO_64_FPCP
#   define SERIAL_STOREFP64(a, b) memcpy((a), (b), 8)
#  endif
	inline void serial_store_fp64(void *data, double value)
	{ SERIAL_STOREFP64(data, &value); }
	inline double serial_load_fp64(const void *data)
	{
		double ret;
		SERIAL_STOREFP64(&ret, data);
		return ret;
	}
#  undef SERIAL_STOREFP64
# endif

#endif

#define SERIAL_OPTIMIZED SERIAL_USE_CPOPT && SYSINFO_ENDIAN && CHAR_BIT == 8
#if !(SERIAL_OPTIMIZED && defined(UINT16_MAX))
	inline void serial_store_ui16(void *d, uint_least16_t value)
	{
		unsigned char *data = (unsigned char*)(d);
		data[0] = (unsigned char) (value>>8 & 0xFFu);
		data[1] = (unsigned char) (value & 0xFFu);
	}
	inline uint_least16_t serial_load_ui16(const void *d)
	{
		unsigned char *data = (unsigned char*)(d);
		return ((uint_least16_t)data[0] << 8) | data[1];
	}
#endif

#if !(SERIAL_OPTIMIZED && defined(UINT32_MAX))
	inline void serial_store_ui32(void *d, uint_least32_t value)
	{
		unsigned char *data = (unsigned char*)(d);
		data[0] = (unsigned char) (value>>24 & 0xFFu);
		data[1] = (unsigned char) (value>>16 & 0xFFu);
		data[2] = (unsigned char) (value>>8 & 0xFFu);
		data[3] = (unsigned char) (value & 0xFFu);
	}
	inline uint_least32_t serial_load_ui32(const void *d)
	{
		unsigned char *data = (unsigned char*)(d);
		return (uint_least32_t)data[0] << 24
			| (uint_least32_t)data[1] << 16
			| (uint_least32_t)data[2] << 8
			| (uint_least32_t)data[3];
	}
#endif

#if !(SERIAL_OPTIMIZED && defined(UINT64_MAX))
	inline void serial_store_ui64(void *d, uint_least64_t value)
	{
		unsigned char *data = (unsigned char*)(d);
		data[0] = (unsigned char) (value>>56 & 0xFFu);
		data[1] = (unsigned char) (value>>48 & 0xFFu);
		data[2] = (unsigned char) (value>>40 & 0xFFu);
		data[3] = (unsigned char) (value>>32 & 0xFFu);
		data[4] = (unsigned char) (value>>24 & 0xFFu);
		data[5] = (unsigned char) (value>>16 & 0xFFu);
		data[6] = (unsigned char) (value>>8 & 0xFFu);
		data[7] = (unsigned char) (value & 0xFFu);
	}
	inline uint_least64_t serial_load_ui64(const void *d)
	{
		unsigned char *data = (unsigned char*)(d);
		return (uint_least64_t)data[0] << 56
			| (uint_least64_t)data[1] << 48
			| (uint_least64_t)data[2] << 40
			| (uint_least64_t)data[3] << 32
			| (uint_least64_t)data[4] << 24
			| (uint_least64_t)data[5] << 16
			| (uint_least64_t)data[6] << 8
			| (uint_least64_t)data[7];
	}
#endif
#undef SERIAL_OPTIMIZED

#define SERIAL_OPTIMIZED SERIAL_USE_CPOPT && SYSINFO_ENDIAN \
	&& CHAR_BIT == 8 && SYSINFO_IREP == SYSINFO_TWO_IREP
#if !(SERIAL_OPTIMIZED && defined(INT16_MAX))
	inline void serial_store_i16(void *data, int_least16_t value)
	{
		if (value >= 0)
		{ serial_store_ui16(data, (uint_least16_t) value); }
		else
		{ serial_store_ui16(data, ~(uint_least16_t)(-1 - value)); }
	}
	inline int_least16_t serial_load_i16(const void *d)
	{
		unsigned char *data = (unsigned char*)(d);
		if (data[0] & 0x80)
		{ return (int_least16_t) serial_load_ui16(data); }
		else
		{ return -1 - ((int_least16_t) ~serial_load_ui16(data)); }
	}
#endif
#if !(SERIAL_OPTIMIZED && defined(INT32_MAX))
	inline void serial_store_i32(void *data, int_least32_t value)
	{
		if (value >= 0)
		{ serial_store_ui32(data, (uint_least32_t) value); }
		else
		{ serial_store_ui32(data, ~(uint_least32_t)(-1 - value)); }
	}
	inline int_least32_t serial_load_i32(const void *d)
	{
		unsigned char *data = (unsigned char*)(d);
		if (data[0] & 0x80)
		{ return (int_least32_t) serial_load_ui32(data); }
		else
		{ return -1 - ((int_least32_t) ~serial_load_ui32(data)); }
	}
#endif
#if !(SERIAL_OPTIMIZED && defined(INT64_MAX))
	inline void serial_store_i64(void *data, int_least64_t value)
	{
		if (value >= 0)
		{ serial_store_ui64(data, (uint_least64_t) value); }
		else
		{ serial_store_ui64(data, ~(uint_least64_t)(-1 - value)); }
	}
	inline int_least64_t serial_load_i64(const void *d)
	{
		unsigned char *data = (unsigned char*)(d);
		if (data[0] & 0x80)
		{ return (int_least64_t) serial_load_ui64(data); }
		else
		{ return -1 - ((int_least64_t) ~serial_load_ui64(data)); }
	}
#endif
#undef SERIAL_OPTIMIZED

CPP_EXTERNC_END
#endif
