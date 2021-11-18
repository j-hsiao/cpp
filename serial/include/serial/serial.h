// Integers are stored in big endian order.
// Note, I've never encountered a system where CHAR_BIT > 8 so
// not sure how storing bytes would work on that system (would
// 0x0102 be 2 bytes exactly, or would it be less than 2 bytes?),
// but bytes are stored with values from 0 to 255 byte by byte.
// Signed integers are stored using 2s complement.
//
// floats are stored in IEEE 754 big endian format.
// Note that nans may not necessarily be preserved as is 
// (signalling vs non-signalling) and the actual mantissa bits
// may vary.
#ifndef SERIAL_H
#define SERIAL_H

#include "serial/serial_dllcfg.h"

#if defined(SERIAL_USE_CPOPT) && SERIAL_USE_CPOPT
#  include "sysinfo/sysinfo.h"
#else
#  ifndef SERIAL_USE_CPOPT
#    define SERIAL_USE_CPOPT 0
#  endif
#endif

#include <stddef.h>
#include <stdint.h>


CPP_EXTERNC_BEGIN
//expected bytes for each type
static const size_t serial_I8_Bytes = 1;
static const size_t serial_I16_Bytes = 2;
static const size_t serial_I32_Bytes = 4;
static const size_t serial_I64_Bytes = 8;
static const size_t serial_UI8_Bytes = 1;
static const size_t serial_UI16_Bytes = 2;
static const size_t serial_UI32_Bytes = 4;
static const size_t serial_UI64_Bytes = 8;
static const size_t serial_FP32_Bytes = 4;
static const size_t serial_FP64_Bytes = 8;

inline void serial_store_i16(void *data, int_least16_t value);
inline int_least16_t serial_load_i16(const void*data);

inline void serial_store_i32(void *data, int_least32_t value);
inline int_least32_t serial_load_i32(const void *data);

inline void serial_store_i64(void *data, int_least64_t value);
inline int_least64_t serial_load_i64(const void *data);

inline void serial_store_ui16(void *data, uint_least16_t value);
inline uint_least16_t serial_load_ui16(const void *data);

inline void serial_store_ui32(void *data, uint_least32_t value);
inline uint_least32_t serial_load_ui32(const void *data);

inline void serial_store_ui64(void *data, uint_least64_t value);
inline uint_least64_t serial_load_ui64(const void *data);

#if SERIAL_USE_CPOPT && SYSINFO_32_FPCP && CHAR_BIT == 8
inline void serial_store_fp32(void *data, float value);
inline float serial_load_fp32(const void *data);
#else
SERIAL_API void serial_store_fp32(void *data, float value);
SERIAL_API float serial_load_fp32(const void *data);
#endif

#if SERIAL_USE_CPOPT && SYSINFO_64_FPCP && CHAR_BIT == 8
inline void serial_store_fp64(void *data, double value);
inline double serial_load_fp64(const void *data);
#else
SERIAL_API void serial_store_fp64(void *data, double value);
SERIAL_API double serial_load_fp64(const void *data);
#endif

CPP_EXTERNC_END

#include <serial/serial_impl.h>
#endif
