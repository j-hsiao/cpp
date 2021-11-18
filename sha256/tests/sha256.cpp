#include <sha256/sha256.h>

#include <iostream>
#include <cstring>
#undef NDEBUG
#include <cassert>

int main(int argc, char *argv[])
{
	{
		unsigned char test[] = "abc";
		unsigned char hash[sha256_Hash_Bytes];
		sha256_hash(hash, test, 3);
		unsigned char expected[] = 
		{
			0xBAu, 0x78u, 0x16u, 0xBFu,
			0x8Fu, 0x01u, 0xCFu, 0xEAu,
			0x41u, 0x41u, 0x40u, 0xDEu,
			0x5Du, 0xAEu, 0x22u, 0x23u,
			0xB0u, 0x03u, 0x61u, 0xA3u,
			0x96u, 0x17u, 0x7Au, 0x9Cu,
			0xB4u, 0x10u, 0xFFu, 0x61u,
			0xF2u, 0x00u, 0x15u, 0xADu
		};
		if (std::memcmp(expected, hash, sha256_Hash_Bytes))
		{
			std::cerr << "failed abc" << std::endl;
			return 1;
		}
		std::cerr << "passed abc" << std::endl;
	}
	{
		unsigned char test[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
		unsigned char hash[sha256_Hash_Bytes];
		sha256_hash(hash, test, std::strlen(reinterpret_cast<char*>(test)));
		unsigned char expected[] = 
		{
			0x24u, 0x8Du, 0x6Au, 0x61u,
			0xD2u, 0x06u, 0x38u, 0xB8u,
			0xE5u, 0xC0u, 0x26u, 0x93u,
			0x0Cu, 0x3Eu, 0x60u, 0x39u,
			0xA3u, 0x3Cu, 0xE4u, 0x59u,
			0x64u, 0xFFu, 0x21u, 0x67u,
			0xF6u, 0xECu, 0xEDu, 0xD4u,
			0x19u, 0xDBu, 0x06u, 0xC1u
		};
		if (std::memcmp(expected, hash, sha256_Hash_Bytes))
		{
			std::cerr << "failed " << test << std::endl;
			return 1;
		}
		std::cerr << "passed " << test << std::endl;
	}
	return 0;
}
