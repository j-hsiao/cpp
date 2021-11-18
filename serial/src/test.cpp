#include "serial/serial.hpp"
#include "serial/serial.h"
#include "timeutil/timeutil.hpp"

#include <string>
#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <climits>
#include <limits>
#include <iostream>
#include <typeinfo>
#include <vector>
#include <map>
#include <memory>
#include <type_traits>


const unsigned char n16_bytes[][3] = {
	"\x00\x01", "\x00\x02", "\x00\x04", "\x00\x08",
	"\x00\x10", "\x00\x20", "\x00\x40", "\x00\x80",
	"\x01\x00", "\x02\x00", "\x04\x00", "\x08\x00",
	"\x10\x00", "\x20\x00", "\x40\x00", "\x80\x00"};

const unsigned char n32_bytes[][5] = {
	"\x00\x00\x00\x01", "\x00\x00\x00\x02", "\x00\x00\x00\x04", "\x00\x00\x00\x08",
	"\x00\x00\x00\x10", "\x00\x00\x00\x20", "\x00\x00\x00\x40", "\x00\x00\x00\x80",
	"\x00\x00\x01\x00", "\x00\x00\x02\x00", "\x00\x00\x04\x00", "\x00\x00\x08\x00",
	"\x00\x00\x10\x00", "\x00\x00\x20\x00", "\x00\x00\x40\x00", "\x00\x00\x80\x00",
	"\x00\x01\x00\x00", "\x00\x02\x00\x00", "\x00\x04\x00\x00", "\x00\x08\x00\x00",
	"\x00\x10\x00\x00", "\x00\x20\x00\x00", "\x00\x40\x00\x00", "\x00\x80\x00\x00",
	"\x01\x00\x00\x00", "\x02\x00\x00\x00", "\x04\x00\x00\x00", "\x08\x00\x00\x00",
	"\x10\x00\x00\x00", "\x20\x00\x00\x00", "\x40\x00\x00\x00", "\x80\x00\x00\x00"};

const unsigned char n64_bytes[][9] = {
	"\x00\x00\x00\x00\x00\x00\x00\x01", "\x00\x00\x00\x00\x00\x00\x00\x02", "\x00\x00\x00\x00\x00\x00\x00\x04", "\x00\x00\x00\x00\x00\x00\x00\x08",
	"\x00\x00\x00\x00\x00\x00\x00\x10", "\x00\x00\x00\x00\x00\x00\x00\x20", "\x00\x00\x00\x00\x00\x00\x00\x40", "\x00\x00\x00\x00\x00\x00\x00\x80",
	"\x00\x00\x00\x00\x00\x00\x01\x00", "\x00\x00\x00\x00\x00\x00\x02\x00", "\x00\x00\x00\x00\x00\x00\x04\x00", "\x00\x00\x00\x00\x00\x00\x08\x00",
	"\x00\x00\x00\x00\x00\x00\x10\x00", "\x00\x00\x00\x00\x00\x00\x20\x00", "\x00\x00\x00\x00\x00\x00\x40\x00", "\x00\x00\x00\x00\x00\x00\x80\x00",
	"\x00\x00\x00\x00\x00\x01\x00\x00", "\x00\x00\x00\x00\x00\x02\x00\x00", "\x00\x00\x00\x00\x00\x04\x00\x00", "\x00\x00\x00\x00\x00\x08\x00\x00",
	"\x00\x00\x00\x00\x00\x10\x00\x00", "\x00\x00\x00\x00\x00\x20\x00\x00", "\x00\x00\x00\x00\x00\x40\x00\x00", "\x00\x00\x00\x00\x00\x80\x00\x00",
	"\x00\x00\x00\x00\x01\x00\x00\x00", "\x00\x00\x00\x00\x02\x00\x00\x00", "\x00\x00\x00\x00\x04\x00\x00\x00", "\x00\x00\x00\x00\x08\x00\x00\x00",
	"\x00\x00\x00\x00\x10\x00\x00\x00", "\x00\x00\x00\x00\x20\x00\x00\x00", "\x00\x00\x00\x00\x40\x00\x00\x00", "\x00\x00\x00\x00\x80\x00\x00\x00",

	"\x00\x00\x00\x01\x00\x00\x00\x00", "\x00\x00\x00\x02\x00\x00\x00\x00", "\x00\x00\x00\x04\x00\x00\x00\x00", "\x00\x00\x00\x08\x00\x00\x00\x00",
	"\x00\x00\x00\x10\x00\x00\x00\x00", "\x00\x00\x00\x20\x00\x00\x00\x00", "\x00\x00\x00\x40\x00\x00\x00\x00", "\x00\x00\x00\x80\x00\x00\x00\x00",
	"\x00\x00\x01\x00\x00\x00\x00\x00", "\x00\x00\x02\x00\x00\x00\x00\x00", "\x00\x00\x04\x00\x00\x00\x00\x00", "\x00\x00\x08\x00\x00\x00\x00\x00",
	"\x00\x00\x10\x00\x00\x00\x00\x00", "\x00\x00\x20\x00\x00\x00\x00\x00", "\x00\x00\x40\x00\x00\x00\x00\x00", "\x00\x00\x80\x00\x00\x00\x00\x00",
	"\x00\x01\x00\x00\x00\x00\x00\x00", "\x00\x02\x00\x00\x00\x00\x00\x00", "\x00\x04\x00\x00\x00\x00\x00\x00", "\x00\x08\x00\x00\x00\x00\x00\x00",
	"\x00\x10\x00\x00\x00\x00\x00\x00", "\x00\x20\x00\x00\x00\x00\x00\x00", "\x00\x40\x00\x00\x00\x00\x00\x00", "\x00\x80\x00\x00\x00\x00\x00\x00",
	"\x01\x00\x00\x00\x00\x00\x00\x00", "\x02\x00\x00\x00\x00\x00\x00\x00", "\x04\x00\x00\x00\x00\x00\x00\x00", "\x08\x00\x00\x00\x00\x00\x00\x00",
	"\x10\x00\x00\x00\x00\x00\x00\x00", "\x20\x00\x00\x00\x00\x00\x00\x00", "\x40\x00\x00\x00\x00\x00\x00\x00", "\x80\x00\x00\x00\x00\x00\x00\x00"
};


uint_least64_t uvalues[] = {
1ull, 2ull, 4ull, 8ull,
16ull, 32ull, 64ull, 128ull,
256ull, 512ull, 1024ull, 2048ull,
4096ull, 8192ull, 16384ull, 32768ull,
65536ull, 131072ull, 262144ull, 524288ull,
1048576ull, 2097152ull, 4194304ull, 8388608ull,
16777216ull, 33554432ull, 67108864ull, 134217728ull,
268435456ull, 536870912ull, 1073741824ull, 2147483648ull,
4294967296ull, 8589934592ull, 17179869184ull, 34359738368ull,
68719476736ull, 137438953472ull, 274877906944ull, 549755813888ull,
1099511627776ull, 2199023255552ull, 4398046511104ull, 8796093022208ull,
17592186044416ull, 35184372088832ull, 70368744177664ull, 140737488355328ull,
281474976710656ull, 562949953421312ull, 1125899906842624ull, 2251799813685248ull,
4503599627370496ull, 9007199254740992ull, 18014398509481984ull, 36028797018963968ull,
72057594037927936ull, 144115188075855872ull, 288230376151711744ull, 576460752303423488ull,
1152921504606846976ull, 2305843009213693952ull, 4611686018427387904ull, 9223372036854775808ull};


template<class T>
void printbytes(T thing, std::size_t count)
{
	std::cout << "[ ";
	for (std::size_t i=0; i<count; ++i)
	{
		std::cerr << static_cast<int>(*thing) << " ";
		++thing;
	}
	std::cout << "]" << std::endl;
}


template<class T, int bsize, bool issigned>
int check_int(
	void (*store)(void *data, T),
	T (*load)(const void *data),
	const unsigned char (*bins)[bsize])
{
	const int nbytes = bsize-1;
	const int nbits = nbytes*8;
	std::cerr << "testing " << nbits << " bit "
		<< (issigned ? "": "un") << "signed integer" << std::endl;

	{
		std::vector<unsigned char> buf(nbits / 8);
		for (int i=0; i < nbits-issigned; ++i)
		{
			T value = static_cast<T>(uvalues[i]);
			store(&buf[0], value);

			if (std::memcmp(bins[i], &buf[0], buf.size()))
			{
				std::cerr << "store failed: " << value << std::endl;
				return 1;
			}
			if (load(bins[i]) != value)
			{
				std::cerr << "load failed: " << value << std::endl;
				return 1;
			}
			if (issigned)
			{
				for (unsigned char &c : buf)
				{ c = ~c; }
				if (load(&buf[0]) + value != -1)
				{
					std::cerr << "load bitflipped binary failed" << std::endl;
					std::cerr << "value: " << value << std::endl;
					std::cerr << "expected bitflipped: " << (-1 - value) << std::endl;
					std::cerr << "loaded result: " << load(&buf[0]) << std::endl;
					return 1;
				}
			}
		}
	}
	//cpp test
	{
		const serial::tp ssign = (
			issigned ? serial::tp::Signed : serial::tp::Unsigned);
		std::vector<unsigned char> buf(
			serial::Loadcfg<nbits>::nbytes(nbits-issigned));
		std::vector<unsigned char> expected(
			serial::Loadcfg<nbits>::nbytes(nbits-issigned));
		for (int i=0; i<nbits-issigned; ++i)
		{ std::memcpy(&expected[i*nbytes], bins[i], nbytes); }
		serial::store<ssign, nbits>(&buf[0], uvalues, uvalues + (nbits-issigned));
		if (std::memcmp(&buf[0], &expected[0], buf.size()))
		{
			std::cerr << "serial::store failed" << std::endl;
			printbytes(buf.begin(), buf.size());
			printbytes(expected.begin(), expected.size());
			return 1;
		}
		std::vector<T> sloaded(nbits-issigned);
		serial::load<ssign, nbits>(&buf[0], sloaded.begin(), sloaded.end());
		for (std::size_t i = 0; i < sloaded.size(); ++i)
		{
			if (sloaded[i] != uvalues[i])
			{
				std::cerr << "failed restoring item " << i << ": " << uvalues[i]
					<< ", got " << sloaded[i] << std::endl;
				return 1;
			}
		}
	}

	std::cerr << "passed" << std::endl;
	return 0;
}

int check_fp32()
{
	std::cerr << "testing fp32" << std::endl;
	std::vector<float> fvalues{
		std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity(),
		0,
		0.15625,
		ldexpf(1, -126 -23),
		std::nanf("0"),
		ldexpf(0xffffff, 127 - 23),
		ldexpf(0x800000, 127 - 23),
	};
	const unsigned char fbytes[][5] = {
		"\x7F\x80\x00\x00",
		"\xFF\x80\x00\x00",
		"\x00\x00\x00\x00",
		"\x3E\x20\x00\x00",
		"\x00\x00\x00\x01",
		"\x7F\xC0\x00\x00",
		"\x7F\x7F\xFF\xFF",
		"\x7F\x00\x00\x00"
	};
	unsigned char buf[4];
	for (int i=0; i<fvalues.size(); ++i)
	{
		if (std::isinf(fvalues[i]))
		{
			float result = serial_load_fp32(fbytes[i]);
			if (!(std::isinf(result) && (result * fvalues[i])>0))
			{
				std::cerr << "decoding " << fvalues[i] << " failed" << std::endl;
				return 1;
			}
			std::memset(buf, 0, 4);
			serial_store_fp32(buf, fvalues[i]);
			if (std::memcmp(fbytes[i], buf, 4))
			{
				std::cerr << "encoding " << fvalues[i] << " failed" << std::endl;
				return 1;
			}
		}
		else if (std::isnan(fvalues[i]))
		{
			if (!std::isnan(serial_load_fp32(fbytes[i])))
			{
				std::cerr << "decoding " << fvalues[i] << " failed" << std::endl;
				return 1;
			}
			std::memset(buf, 0, 4);
			serial_store_fp32(buf, fvalues[i]);
			if (
				((buf[0] & 0x7Fu) != 0x7Fu)
				|| !(buf[1] & 0x80u)
				|| !(buf[1] & 0x7Fu || buf[2] || buf[3])
			)
			{
				std::cerr << "encoding " << fvalues[i] << " failed" << std::endl;
				return 1;
			}
		}
		else
		{
			if (serial_load_fp32(fbytes[i]) != fvalues[i])
			{
				std::cerr << "decoding " << fvalues[i] << " failed" << std::endl;
				std::cerr << "got " << serial_load_fp32(fbytes[i]) << std::endl;
				return 1;
			}
			std::memset(buf, 0, 4);
			serial_store_fp32(buf, fvalues[i]);
			if (std::memcmp(buf, fbytes[i], 4))
			{
				std::cerr << "encoding " << fvalues[i] << " failed" << std::endl;
				return 1;
			}
		}
	}
	std::cerr << "passed" << std::endl;
	return 0;
}

int check_fp64()
{
	std::cerr << "testing fp64" << std::endl;
	std::vector<double> fvalues{
		0,
		std::numeric_limits<double>::infinity(),
		-std::numeric_limits<double>::infinity(),
		std::nan("0"),
		0.15625,
		5e-324,
		1.7976931348623157e+308
	};
	const unsigned char fbytes[][9] = {
		"\x00\x00\x00\x00\x00\x00\x00\x00",
		"\x7F\xF0\x00\x00\x00\x00\x00\x00",
		"\xFF\xF0\x00\x00\x00\x00\x00\x00",
		"\x7F\xF8\x00\x00\x00\x00\x00\x00",
		"\x3F\xC4\x00\x00\x00\x00\x00\x00",
		"\x00\x00\x00\x00\x00\x00\x00\x01",
		"\x7F\xEF\xFF\xFF\xFF\xFF\xFF\xFF"
	};
	unsigned char buf[8];
	for (int i=0; i<fvalues.size(); ++i)
	{
		if (std::isinf(fvalues[i]))
		{
			double result = serial_load_fp64(fbytes[i]);
			if (!(std::isinf(result) && (result * fvalues[i])>0))
			{
				std::cerr << "decoding " << fvalues[i] << " failed" << std::endl;
				std::cerr << "got " << result << std::endl;
				return 1;
			}
			std::memset(buf, 0, 8);
			serial_store_fp64(buf, fvalues[i]);
			if (std::memcmp(fbytes[i], buf, 8))
			{
				std::cerr << "encoding " << fvalues[i] << " failed" << std::endl;
				return 1;
			}
		}
		else if (std::isnan(fvalues[i]))
		{
			if (!std::isnan(serial_load_fp64(fbytes[i])))
			{
				std::cerr << "decoding " << fvalues[i] << " failed" << std::endl;
				return 1;
			}
			std::memset(buf, 0, 8);
			serial_store_fp64(buf, fvalues[i]);
			if (
				((buf[0] & 0x7Fu) != 0x7Fu)
				|| !(buf[1] & 0xF0u)
				|| !(buf[1] & 0x0Fu || buf[2] || buf[3] || buf[4] || buf[5] || buf[6] || buf[7])
			)
			{
				std::cerr << "encoding " << fvalues[i] << " failed" << std::endl;
				return 1;
			}
		}
		else
		{
			if (serial_load_fp64(fbytes[i]) != fvalues[i])
			{
				std::cerr << "decoding " << fvalues[i] << " failed" << std::endl;
				std::cerr << "got " << serial_load_fp64(fbytes[i]) << std::endl;
				return 1;
			}
			std::memset(buf, 0, 8);
			serial_store_fp64(buf, fvalues[i]);
			if (std::memcmp(buf, fbytes[i], 8))
			{
				std::cerr << "encoding " << fvalues[i] << " failed" << std::endl;
				return 1;
			}
		}
	}
	std::cerr << "passed" << std::endl;
	return 0;
}

template<
	class T, int nbytes,
	void store(void *data, T value),
	T load(const void *data)
>
void ttest(
	std::size_t dsize,
	std::vector<unsigned char> &buf,
	std::vector<unsigned char> &vsrc,
	std::vector<unsigned char> &vdst)
{
	std::cerr << "type: " << typeid(T).name()
		<< "\tnbytes: " << nbytes << std::endl;
	void *psrc = &vsrc[0];
	void *pdst = &vdst[0];
	std::size_t ssrc = vsrc.size();
	std::size_t sdst = vdst.size();
	T *asrc = reinterpret_cast<T*>(std::align(
		std::alignment_of<T>::value, sizeof(T) * dsize, psrc, ssrc));
	T *adst = reinterpret_cast<T*>(std::align(
		std::alignment_of<T>::value, sizeof(T) * dsize, pdst, sdst));
	if (!asrc || !adst)
	{
		std::cerr << "alignment failed" << std::endl;
		return;
	}
	T *tsrc = new(asrc) T[dsize];
	T *tdst = new(adst) T[dsize];
	if (tsrc != asrc || adst != tdst)
	{
		std::cerr << "placement new did not return same address "
			"as aligned, skipping test" << std::endl;
		return;
	}
	timeutil::Clocker timer;
	timer.tic();
	for (std::size_t i=0; i < dsize; ++i)
	{ store(&buf[i*nbytes], tsrc[i]); }
	double tdelta = timer.toc();
	std::cerr << "storing: " << tdelta << std::endl;

	timer.tic();
	for (std::size_t i=0; i < dsize; ++i)
	{ tdst[i] = load(&buf[i*nbytes]); }
	tdelta = timer.toc();
	std::cerr << "loading: " << tdelta << std::endl;
}

int timetest(int argc, char *argv[])
{
	if (argc <= 1) { return 0; }
	std::size_t dsize = 1;
	{
		std::map<char, std::size_t> sizes = {
			{'k', 1024},
			{'m', 1024*1024},
			{'g', 1024*1024*1024}
		};
		dsize = 1;
		for (int i=1; i < argc; ++i)
		{
			std::string amt = argv[1];
			std::size_t multiplier = 1;
			if (!std::isdigit(amt.back()))
			{
				char spec = std::tolower(amt.back());
				multiplier = sizes[spec] ? sizes[spec] : 1;
				amt.pop_back();
			}
			try
			{
				dsize = std::stoull(amt) * multiplier;
			} catch (...) {}
		}
	}
	std::size_t dblalign = std::alignment_of<double>::value;
	std::size_t ui64align = std::alignment_of<uint_least64_t>::value;
	std::size_t align = dblalign > ui64align ? dblalign : ui64align;

	std::cerr << "timing test, num items: " << dsize << std::endl;
	std::vector<unsigned char> buf(dsize * 8);
	std::vector<unsigned char> src((dsize+1) * align);
	std::vector<unsigned char> dst((dsize+1) * align);

	ttest<int_least16_t, 2, serial_store_i16, serial_load_i16>(dsize, buf, src, dst);
	ttest<int_least32_t, 4, serial_store_i32, serial_load_i32>(dsize, buf, src, dst);
	ttest<int_least64_t, 8, serial_store_i64, serial_load_i64>(dsize, buf, src, dst);
	ttest<uint_least16_t, 2, serial_store_ui16, serial_load_ui16>(dsize, buf, src, dst);
	ttest<uint_least32_t, 4, serial_store_ui32, serial_load_ui32>(dsize, buf, src, dst);
	ttest<uint_least64_t, 8, serial_store_ui64, serial_load_ui64>(dsize, buf, src, dst);
	ttest<float, 4, serial_store_fp32, serial_load_fp32>(dsize, buf, src, dst);
	ttest<double, 8, serial_store_fp64, serial_load_fp64>(dsize, buf, src, dst);

	return 0;
}

int main(int argc, char *argv[])
{
	return (
		check_int<uint_least16_t, 3, 0>(serial_store_ui16, serial_load_ui16, n16_bytes)
		|| check_int<uint_least32_t, 5, 0>(serial_store_ui32, serial_load_ui32, n32_bytes)
		|| check_int<uint_least64_t, 9, 0>(serial_store_ui64, serial_load_ui64, n64_bytes)
		|| check_int<int_least16_t, 3, 1>(serial_store_i16, serial_load_i16, n16_bytes)
		|| check_int<int_least32_t, 5, 1>(serial_store_i32, serial_load_i32, n32_bytes)
		|| check_int<int_least64_t, 9, 1>(serial_store_i64, serial_load_i64, n64_bytes)
		|| check_fp32()
		|| check_fp64()
		|| timetest(argc, argv)
	);
}
