#include <serial/serial.h>

#include <cstdint>
#include <climits>
#include <iostream>
#include <typeinfo>

template<class T, int bits>
int check_int(
	void (*store)(T, unsigned char *data),
	T (*load)(unsigned char *data))
{
	T x;
	std::cerr << "int test " << typeid(x).name() << "("
		<< bits << " bits)" << std::endl;
	unsigned char buf[sizeof(T)];

	for (int i=0; i<bits; ++i)
	{
		T thing = 1;
		thing <<= i;
		std::cerr << "testing " << i << ": " << thing << std::endl;
		store(thing, buf);
		if (load(buf) != thing)
		{
			std::cerr << "error, expected " << thing
				<< " but got " << load(buf) << std::endl;
			return 1;
		}
	}
	std::cerr << "passed" << std::endl;
	return 0;
}


int main(int argc, char *argv[])
{
	return (
		check_int<uint_least16_t, 16>(serial__store_ui16, serial__load_ui16)
		|| check_int<uint_least32_t, 32>(serial__store_ui32, serial__load_ui32)
		|| check_int<uint_least64_t, 64>(serial__store_ui64, serial__load_ui64)
		|| check_int<int_least16_t, 16>(serial__store_i16, serial__load_i16)
		|| check_int<int_least32_t, 32>(serial__store_i32, serial__load_i32)
		|| check_int<int_least64_t, 64>(serial__store_i64, serial__load_i64)
	);
}
