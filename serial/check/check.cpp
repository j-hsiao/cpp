#include <iostream>
#include <cstdint>
#include <climits>
#include <limits>
#include <string>
#include <cstring>


namespace
{
	//Return a ull with bytes counting up from most significant
	//ie: if sizeof(ull) == 8, then 0x0102030405060708 (and CHAR_BIT == 8)
	unsigned long long endian_check_int()
	{
		unsigned long long val = 0;
		std::cerr << "char values: ";
		for (std::size_t i=0; i < sizeof(val); ++i)
		{
			unsigned char t = (i+1)&0xFFu;
			std::cerr << static_cast<int>(t) << ",";
			val |= static_cast<unsigned long long>(t) << (sizeof(val)-(i+1))*CHAR_BIT;
		}
		std::cerr << std::endl << "in memory: ";
		for (std::size_t i=0; i<sizeof(val); ++i)
		{
			std::cerr << static_cast<int>(
				reinterpret_cast<unsigned char*>(&val)[i]) << ",";
		}
		std::cerr << std::endl;
		return val;
	}

	bool little_endian(unsigned long long eci)
	{
		unsigned char expected[sizeof(eci)];
		std::cerr << "little bytes: ";
		for (int i=0; i<sizeof(eci); ++i)
		{
			unsigned char t = (sizeof(eci)-i)&0xFFu;
			std::cerr << static_cast<int>(t) << ",";
			expected[i] = t;
		}
		std::cerr << std::endl;
		return std::memcmp(&eci, expected, sizeof(eci)) == 0;
	}

	bool big_endian(unsigned long long eci)
	{
		unsigned char expected[sizeof(eci)];
		std::cerr << "big bytes: ";
		for (std::size_t i=0; i<sizeof(eci); ++i)
		{
			unsigned char t = (i+1) & 0xFFu;
			std::cerr << static_cast<int>(t) << ",";
			expected[i] = t;
		}
		std::cerr << std::endl;;
		return std::memcmp(&eci, expected, sizeof(eci)) == 0;
	}


	template<class tp>
	struct rawinfo;
	template<>
	struct rawinfo<float>
	{
		static const char little[];
		static const char big[];
		static const std::size_t nbytes = 4;
#ifdef UINT32_MAX
		static const bool exact = 1;
#else
		static const bool exact = 0;
#endif
		static const float testval;
	};
	const char rawinfo<float>::little[] = "\x04\x03\x02\x01";
	const char rawinfo<float>::big[] = "\x01\x02\x03\x04";
	const float rawinfo<float>::testval = 2.3879393e-38;

	template<>
	struct rawinfo<double>
	{
		static const char little[];
		static const char big[];
		static const std::size_t nbytes = 8;
#ifdef UINT64_MAX
		static const bool exact = 1;
#else
		static const bool exact = 0;
#endif
		static const double testval;
	};
	const char rawinfo<double>::little[] = "\x08\x07\x06\x05\x04\x03\x02\x01";
	const char rawinfo<double>::big[] = "\x01\x02\x03\x04\x05\x06\x07\x08";
	const double rawinfo<double>::testval = 8.20788039913184e-304;

	template<class T>
	struct fpchecker: public rawinfo<T>
	{
		typedef T value_type;
		int copyable() const
		{
			if (
				std::numeric_limits<T>::is_iec559
				&& sizeof(T) == this->nbytes
				&& this->exact
				&& CHAR_BIT == 8)
			{
				auto eic = endian_check_int();
				bool little = little_endian(eic);
				bool big = big_endian(eic);
				if (big && !little)
				{ return !std::memcmp(&(this->testval), this->big, this->nbytes); }
				else if (little && !big)
				{ return !std::memcmp(&(this->testval), this->little, this->nbytes); }
			}
			return 0;
		}
	};
}


int main(int argc, char *argv[])
{
	if (argc <= 1)
	{ 
		std::cerr << "missing argument: 'endian' or 'rep'" << std::endl;
		return -1;
	}

	std::string prog = argv[1];
	if (prog == "rep")
	{
		if (~0 == 0)
		{ std::cout << "ONE" << std::endl; }
		else if (~0 == -1)
		{ std::cout << "TWO" << std::endl; }
		else if (~0 == -INT_MAX)
		{ std::cout << "SMAG" << std::endl; }
		else
		{ std::cout << "UNKNOWN" << std::endl; }
	}
	else if (prog == "endian")
	{
		unsigned long long eci = endian_check_int();
		bool big = big_endian(eci);
		bool little = little_endian(eci);
		if (little && !big)
		{ std::cout << "LITTLE" << std::endl; }
		else if (big && !little)
		{ std::cout << "BIG" << std::endl; }
		else
		{ std::cout << "UNKNOWN" << std::endl; }
	}
	else if (prog == "fp32")
	{
		std::cout << fpchecker<float>().copyable() << std::endl;
	}
	else if (prog == "fp64")
	{
		std::cout << fpchecker<double>().copyable() << std::endl;
	}
	else
	{
		std::cerr << "unknown command: \"" << prog << '"' << std::endl;
		return -1;
	}
	return 0;
}
