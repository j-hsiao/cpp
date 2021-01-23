#include <sha256/sha256.hpp>
#include <sha256/sha256.h>

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#undef NDEBUG
#include <cassert>

int main(int argc, char *argv[])
{
	{
		char test[] = {"abc"};
		auto *udat = reinterpret_cast<unsigned char*>(test);
		size_t nchars = 3;
		size_t nbytes = sha256__normalized_size(nchars);
		std::vector<sha256__Byte> data(nbytes, 0);
		sha256__Byte *normed = sha256__normalize(&data[0], udat, nchars);
		if (!normed) { normed = udat; }
		std::vector<sha256__Byte> hash(sha256__Hash_Bytes, 0);
		sha256__hash(&hash[0], normed, nbytes);
		sha256__Byte expected[sha256__Hash_Bytes] = 
		{
			0xBA, 0x78, 0x16, 0xBF,
			0x8F, 0x01, 0xCF, 0xEA,
			0x41, 0x41, 0x40, 0xDE,
			0x5D, 0xAE, 0x22, 0x23,
			0xB0, 0x03, 0x61, 0xA3,
			0x96, 0x17, 0x7A, 0x9C,
			0xB4, 0x10, 0xFF, 0x61,
			0xF2, 0x00, 0x15, 0xAD
		};
		auto out = sha256::hash(std::string(test));
		assert(out == std::string(reinterpret_cast<char*>(expected), sha256__Hash_Bytes));
		assert(std::equal(expected, expected + sha256__Hash_Bytes, &hash[0]));
		std::cout << "single block passed" << std::endl;
	}
	{
		char test[] = {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"};
		auto *udat = reinterpret_cast<unsigned char*>(test);
		size_t nchars = std::string(test).size();
		size_t nbytes = sha256__normalized_size(nchars);
		std::vector<sha256__Byte> data(nbytes, 0);
		sha256__Byte *normed = sha256__normalize(&data[0], udat, nchars);
		if (!normed) { normed = udat; }
		std::vector<sha256__Byte> hash(sha256__Hash_Bytes, 0);
		sha256__hash(&hash[0], normed, nbytes);
		sha256__Byte expected[sha256__Hash_Bytes] = 
		{
			0x24, 0x8D, 0x6A, 0x61,
			0xD2, 0x06, 0x38, 0xB8,
			0xE5, 0xC0, 0x26, 0x93,
			0x0C, 0x3E, 0x60, 0x39,
			0xA3, 0x3C, 0xE4, 0x59,
			0x64, 0xFF, 0x21, 0x67,
			0xF6, 0xEC, 0xED, 0xD4,
			0x19, 0xDB, 0x06, 0xC1
		};
		auto out = sha256::hash(std::string(test));
		assert(out == std::string(reinterpret_cast<char*>(expected), sha256__Hash_Bytes));
		assert(std::equal(expected, expected + sha256__Hash_Bytes, &hash[0]));
		std::cout << "multiblock passed" << std::endl;
	}
	return 0;
}
