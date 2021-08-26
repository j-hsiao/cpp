#include <serial/serial_dllcfg.h>
#include <iostream>

CPP_EXTERNC_BEGIN
// strict aliasing:
// signed* to unsigned* and dereferencing result is allowed
// However, typecasting signed to unsigned may be undefined
// as a result, because data is stored into unsigned char*
// all signed store/load will typealias and call unsigned
// versions
//
// Just a side note, maybe fix it if I ever encounter an issue but...
// 0xFF as bitmask assumes 2's complement right? What if
// not 2's complement? Does that mean I have to explicitly
// calculate all the bits in a 2's complement representation?
// (Divide by 2, mod 1?, store the bit via shifts if applicable?)
// The code here actually only addresses the endianness issue.

inline void serial__store_i16(int_least16_t value, unsigned char *data)
{
	serial__store_ui16(*((uint_least16_t*) &value), data);
}
inline void serial__store_i32(int_least32_t value, unsigned char *data)
{
	serial__store_ui32(*((uint_least32_t*) &value), data);
}
inline void serial__store_i64(int_least64_t value, unsigned char *data)
{
	serial__store_ui64(*((uint_least64_t*) &value), data);
}
inline void serial__store_ui16(uint_least16_t value, unsigned char *data)
{
	data[0] = (value >> 8) & 0xFFu;
	data[1] = value & 0xFFu;
}
inline void serial__store_ui32(uint_least32_t value, unsigned char *data)
{
	data[0] = (value >> 24) & 0xFFu;
	data[1] = (value >> 16) & 0xFFu;
	data[2] = (value >> 8) & 0xFFu;
	data[3] = value & 0xFFu;
}
inline void serial__store_ui64(uint_least64_t value, unsigned char *data)
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

inline int_least16_t serial__load_i16(unsigned char *data)
{
	uint_least16_t tmp = serial__load_ui16(data);
	return *((int_least16_t*) &tmp);
}
inline int_least32_t serial__load_i32(unsigned char *data)
{
	uint_least32_t tmp = serial__load_ui32(data);
	return *((int_least32_t*) &tmp);
}
inline int_least64_t serial__load_i64(unsigned char *data)
{
	uint_least64_t tmp = serial__load_ui64(data);
	return *((int_least64_t*) &tmp);
}

inline uint_least16_t serial__load_ui16(unsigned char *data)
{
	return data[0] << 8 | data[1];
}
inline uint_least32_t serial__load_ui32(unsigned char *data)
{
	return (
		(uint_least32_t) data[0] << 24 | (uint_least32_t) data[1] << 16
		| data[2] << 8 | data[3]);
}
inline uint_least64_t serial__load_ui64(unsigned char *data)
{
	return (
		(uint_least64_t) data[0] << 56
		| (uint_least64_t) data[1] << 48
		| (uint_least64_t) data[2] << 40
		| (uint_least64_t) data[3] << 32
		| (uint_least64_t) data[4] << 24
		| (uint_least64_t) data[5] << 16
		| data[6] << 8 | data[7]);
}

CPP_EXTERNC_END
