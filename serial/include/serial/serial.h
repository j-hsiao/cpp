#ifndef SERIAL_H
#define SERIAL_H

#include <serial/serial_dllcfg.h>
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

void serial__store_fp32(unsigned char *data, float value);
void serial__store_fp64(unsigned char *data, double value);
float serial__load_fp32(const unsigned char *data);
double serial__load_fp64(const unsigned char *data);

CPP_EXTERNC_END

#include <serial/serial_impl.h>
#endif
