#include <iostream>
#include <cstdint>
#include <climits>
#include <limits>
#include <string>
#include <cstring>
#include <fstream>

#include <vector>


namespace
{
	const std::vector<std::string> endians = {
		"SYSINFO_UNKNOWN_ENDIAN",
		"SYSINFO_LITTLE_ENDIAN",
		"SYSINFO_BIG_ENDIAN"
	};
	const std::vector<std::string> ireps = {
		"SYSINFO_UNKNOWN_IREP",
		"SYSINFO_ONE_IREP",
		"SYSINFO_TWO_IREP",
		"SYSINFO_SMAG_IREP"
	};
	const std::vector<std::string> fpcps = {
		"SYSINFO_UNKNOWN_FPCP",
		"SYSINFO_COPY_FPCP",
		"SYSINFO_REVERSE_FPCP"
	};


	uint_least32_t endiancheck()
	{
		//each byte in big-endian order is 1 2 3 4 5... 
		uint_least32_t val = 0;
		for (std::size_t i=0; i<sizeof(val); ++i)
		{
			unsigned char t = (i+1) & 0xFFu;
			auto bitshift = (sizeof(val)-(i+1))*CHAR_BIT;
			val |= static_cast<uint_least32_t>(t) << bitshift;
		}
		return val;
	}

	bool islittle()
	{
		uint_least32_t x = endiancheck();
		auto asbytes = reinterpret_cast<unsigned char*>(&x);
		for (std::size_t i=0; i<sizeof(x); ++i)
		{
			if (asbytes[i] != sizeof(x)-i)
			{ return 0; }
		}
		return 1;
	}

	bool isbig()
	{
		uint_least32_t x = endiancheck();
		auto asbytes = reinterpret_cast<unsigned char*>(&x);
		for (std::size_t i=0; i<sizeof(x); ++i)
		{
			if (asbytes[i] != i+1)
			{ return 0; }
		}
		return 1;
	}

	std::string endian_type()
	{
		if (islittle() == isbig())
		{ return endians[0]; }
		else if (islittle())
		{ return endians[1]; }
		else
		{ return endians[2]; }
	}

	std::string irep()
	{
		if (~0 == 0)
		{ return ireps[1]; }
		else if (~0 == -1)
		{ return ireps[2]; }
		else if (~0 == -INT_MAX)
		{ return ireps[3]; }
		else
		{ return ireps[0]; }
	}


	template<class T>
	struct info
	{
		typedef T value_type;
		const char *reversed;
		const char *copied;
		T value;
		std::size_t nbytes;
	};

	static const info<float> floatinfo{
		"\x04\x03\x02\x01",
		"\x01\x02\x03\x04",
		2.3879393e-38f,
		4};

	static const info<double> doubleinfo{
		"\x08\x07\x06\x05\x04\x03\x02\x01",
		"\x01\x02\x03\x04\x05\x06\x07\x08",
		8.20788039913184e-304,
		8};
	
	template<class T>
	std::string checkfloat(const T &data)
	{
		if (
			std::numeric_limits<typename T::value_type>::is_iec559
			&& sizeof(data.value) == data.nbytes
			&& CHAR_BIT == 8)
		{
			auto *asbytes = reinterpret_cast<const unsigned char*>(&data.value);
			if (!std::memcmp(data.copied, asbytes, sizeof(data.value)))
			{ return fpcps[1]; }
			else if (!std::memcmp(data.reversed, asbytes, sizeof(data.value)))
			{ return fpcps[2]; }
		}
		return fpcps[0];
	}

}


int main(int argc, char *argv[])
{
	std::string fname;
	if (argc > 1)
	{ fname = argv[1]; }
	else
	{
		std::cerr << "need a filename" << std::endl;
		return -1;
	}
	std::ofstream out;
	out.open(fname, out.out | out.trunc);
	out << "#ifndef SYSINFO_H" << std::endl
		<< "#define SYSINFO_H" << std::endl;

	for (const auto &strlist : {endians, ireps, fpcps})
	{
		for (std::size_t i=0; i<strlist.size(); ++i)
		{ out << "#define " << strlist[i] << " " << i << std::endl; }
	}

	out
		<< "#define SYSINFO_32_FPCP " << checkfloat(floatinfo) << std::endl
		<< "#define SYSINFO_64_FPCP " << checkfloat(doubleinfo) << std::endl
		<< "#define SYSINFO_ENDIAN " << endian_type() << std::endl
		<< "#define SYSINFO_IREP " << irep() << std::endl
		<< "#endif" << std::endl;
	return !out;
}
