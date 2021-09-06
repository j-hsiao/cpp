// Integers are stored in big endian order.
// Note, I've never encountered a system where CHAR_BIT > 8 so
// not sure how storing bytes would work on that system,
// but bytes are stored with values from 0 to 255 byte by byte.
// Signed integers are stored using 2s complement.
//
// floats are stored in IEEE 754 big endian format.
// Note that nans may not necessarily be preserved as is 
// (signalling vs non-signalling) and the actual mantissa bits
// may vary.
#ifndef SERIAL_H
#define SERIAL_H

#include <serial/serial_dllcfg.h>
#include <sysinfo/sysinfo.h>

#include <stddef.h>
#include <stdint.h>


CPP_EXTERNC_BEGIN
// For 8-bit numbers just use char/unsigned char, already serialized.
// The provided data unsigned char *buffer must point to
// a buffer of size at least serial__IXX_Bytes
static const size_t serial__I16_Bytes = 2;
static const size_t serial__I32_Bytes = 4;
static const size_t serial__I64_Bytes = 8;
static const size_t serial__UI16_Bytes = 2;
static const size_t serial__UI32_Bytes = 4;
static const size_t serial__UI64_Bytes = 8;
static const size_t serial__FP32_Bytes = 4;
static const size_t serial__FP64_Bytes = 8;

inline void serial__store_i16(unsigned char *data, int_least16_t value);
inline void serial__store_i32(unsigned char *data, int_least32_t value);
inline void serial__store_i64(unsigned char *data, int_least64_t value);
inline int_least16_t serial__load_i16(const unsigned char *data);
inline int_least32_t serial__load_i32(const unsigned char *data);
inline int_least64_t serial__load_i64(const unsigned char *data);

inline void serial__store_ui16(unsigned char *data, uint_least16_t value);
inline void serial__store_ui32(unsigned char *data, uint_least32_t value);
inline void serial__store_ui64(unsigned char *data, uint_least64_t value);
inline uint_least16_t serial__load_ui16(const unsigned char *data);
inline uint_least32_t serial__load_ui32(const unsigned char *data);
inline uint_least64_t serial__load_ui64(const unsigned char *data);

#if SYSINFO_CPFP32
inline void serial__store_fp32(unsigned char *data, float value);
inline float serial__load_fp32(const unsigned char *data);
#else
void serial__store_fp32(unsigned char *data, float value);
float serial__load_fp32(const unsigned char *data);
#endif

#if SYSINFO_CPFP64
inline void serial__store_fp64(unsigned char *data, double value);
inline double serial__load_fp64(const unsigned char *data);
#else
void serial__store_fp64(unsigned char *data, double value);
double serial__load_fp64(const unsigned char *data);
#endif

CPP_EXTERNC_END

#include <serial/serial_impl.h>
#endif
