#include <aes/aes_consts.hpp>
#include <aes/aes_key.hpp>
#include <aes/aes.hpp>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>


bool cmp(const void *a, const void *b, std::size_t size)
{
	const aes::ubyte *b1 = reinterpret_cast<const aes::ubyte*>(a);
	const aes::ubyte *b2 = reinterpret_cast<const aes::ubyte*>(b);
	if (std::memcmp(b1, b2, size))
	{
		for (std::size_t i = 0; i < size; i += aes::WORDBYTES)
		{
			for (std::size_t k = 0; k < aes::WORDBYTES; ++k)
			{
				std::cout << std::hex;
				if (b1[i + k] != b2[i + k])
				{
					std::cout << "difference in word " << std::dec << (i / aes::WORDBYTES)
						<< " byte " << k << ':' << std::hex
						<< static_cast<unsigned int>(b1[i + k]) << " vs "
						<< static_cast<unsigned int>(b2[i + k]) << std::endl;
				}
			}
		}
		return false;
	}
	return true;
}

void check_expand_keys256()
{
	aes::ubyte key[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	};
	auto v = aes::expand_keys(key, aes::AES256);
	aes::ubyte expected[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
		0x62, 0x63, 0x7c, 0x63, 0x62, 0x63, 0x7c, 0x63, 0x62, 0x63, 0x7c, 0x63, 0x62, 0x63, 0x7c, 0x63,
		0xaa, 0xfb, 0x10, 0xfb, 0xaa, 0xfb, 0x10, 0xfb, 0xaa, 0xfb, 0x10, 0xfb, 0xaa, 0xfb, 0x10, 0xfa,
		0x6f, 0xa9, 0x51, 0xcf, 0x0d, 0xca, 0x2d, 0xac, 0x6f, 0xa9, 0x51, 0xcf, 0x0d, 0xca, 0x2d, 0xac,
		0x7d, 0x8f, 0xc8, 0x6a, 0xd7, 0x74, 0xd8, 0x91, 0x7d, 0x8f, 0xc8, 0x6a, 0xd7, 0x74, 0xd8, 0x90,
		0xf9, 0xc8, 0x31, 0xc1, 0xf4, 0x02, 0x1c, 0x6d, 0x9b, 0xab, 0x4d, 0xa2, 0x96, 0x61, 0x60, 0x0e,
		0xed, 0x60, 0x18, 0xc1, 0x3a, 0x14, 0xc0, 0x50, 0x47, 0x9b, 0x08, 0x3a, 0x90, 0xef, 0xd0, 0xaa,
		0x2e, 0xb8, 0x9d, 0xa1, 0xda, 0xba, 0x81, 0xcc, 0x41, 0x11, 0xcc, 0x6e, 0xd7, 0x70, 0xac, 0x60,
		0xe3, 0x31, 0x89, 0x11, 0xd9, 0x25, 0x49, 0x41, 0x9e, 0xbe, 0x41, 0x7b, 0x0e, 0x51, 0x91, 0xd1,
		0xef, 0x39, 0xa3, 0x0a, 0x35, 0x83, 0x22, 0xc6, 0x74, 0x92, 0xee, 0xa8, 0xa3, 0xe2, 0x42, 0xc8,
		0xe9, 0xa9, 0xa5, 0xf9, 0x30, 0x8c, 0xec, 0xb8, 0xae, 0x32, 0xad, 0xc3, 0xa0, 0x63, 0x3c, 0x12,
		0x34, 0xd2, 0x6a, 0xea, 0x01, 0x51, 0x48, 0x2c, 0x75, 0xc3, 0xa6, 0x84, 0xd6, 0x21, 0xe4, 0x4c,
		0x1f, 0x54, 0xcc, 0xd0, 0x2f, 0xd8, 0x20, 0x68, 0x81, 0xea, 0x8d, 0xab, 0x21, 0x89, 0xb1, 0xb9,
		0xd3, 0x1a, 0x3c, 0x17, 0xd2, 0x4b, 0x74, 0x3b, 0xa7, 0x88, 0xd2, 0xbf, 0x71, 0xa9, 0x36, 0xf3
	};
	std::size_t numrounds = aes::NUM_ROUNDS[aes::AES256];
	assert(cmp(v.data(), expected, numrounds * aes::STATEBYTES));
	std::cout << "aes256 key expansion passed" << std::endl;
}

void check_iexpand_keys256()
{
	aes::ubyte key[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	};
	auto v = aes::expand_keys(key, aes::AES256);
	auto r = aes::iexpand_keys(v);
	aes::ubyte expected[] = {
		0xd3, 0x1a, 0x3c, 0x17, 0xd2, 0x4b, 0x74, 0x3b, 0xa7, 0x88, 0xd2, 0xbf, 0x71, 0xa9, 0x36, 0xf3,
		0x31, 0x80, 0x4c, 0xaa, 0xd6, 0xf5, 0xbc, 0x20, 0x74, 0x28, 0xe2, 0xf3, 0xcb, 0xee, 0x00, 0x85,
		0xfb, 0x22, 0x69, 0xd6, 0x11, 0x03, 0x30, 0x16, 0x3b, 0x56, 0xdb, 0x22, 0x7a, 0x8e, 0xf6, 0x5d,
		0xbe, 0x16, 0xca, 0x7e, 0xe7, 0x75, 0xf0, 0x8a, 0xa2, 0xdd, 0x5e, 0xd3, 0xbf, 0xc6, 0xe2, 0x76,
		0xd9, 0x70, 0x6b, 0xbd, 0xea, 0x21, 0x59, 0xc0, 0x2a, 0x55, 0xeb, 0x34, 0x41, 0xd8, 0x2d, 0x7f,
		0xc3, 0xf6, 0x3d, 0x42, 0x59, 0x63, 0x3a, 0xf4, 0x45, 0xa8, 0xae, 0x59, 0x1d, 0x1b, 0xbc, 0xa5,
		0x6e, 0x1b, 0x92, 0x4d, 0x33, 0x51, 0x32, 0x7d, 0xc0, 0x74, 0xb2, 0xf4, 0x6b, 0x8d, 0xc6, 0x4b,
		0xd7, 0xe0, 0x8a, 0xe9, 0x9a, 0x95, 0x07, 0xb6, 0x1c, 0xcb, 0x94, 0xad, 0x58, 0xb3, 0x12, 0xfc,
		0x05, 0x96, 0x54, 0x06, 0x5d, 0x4a, 0xa0, 0x30, 0xf3, 0x25, 0x80, 0x89, 0xab, 0xf9, 0x74, 0xbf,
		0x86, 0x5e, 0x93, 0x1b, 0x4d, 0x75, 0x8d, 0x5f, 0x86, 0x5e, 0x93, 0x1b, 0x44, 0x78, 0x86, 0x51,
		0xae, 0x6f, 0x20, 0xb9, 0x58, 0xdc, 0xf4, 0x36, 0xae, 0x6f, 0x20, 0xb9, 0x58, 0xdc, 0xf4, 0x36,
		0xcb, 0x2b, 0x1e, 0x44, 0xcb, 0x2b, 0x1e, 0x44, 0xcb, 0x2b, 0x1e, 0x44, 0xc2, 0x26, 0x15, 0x4a,
		0xf6, 0xb3, 0xd4, 0x8f, 0xf6, 0xb3, 0xd4, 0x8f, 0xf6, 0xb3, 0xd4, 0x8f, 0xf6, 0xb3, 0xd4, 0x8f,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0d, 0x0b, 0x0e,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};
	assert(cmp(r.data(), expected, r.size() * sizeof(r[0])));
}

void check_expand_keys128()
{
	aes::ubyte key[] = "Thats my Kung Fu";
	std::size_t roundkeyBytes = aes::NUM_ROUNDS[aes::AES128] * aes::STATEBYTES;

	auto v = aes::expand_keys(key, aes::AES128);
	aes::ubyte expected[] = {
		0x54, 0x68, 0x61, 0x74, 0x73, 0x20, 0x6d, 0x79, 0x20, 0x4b, 0x75, 0x6e, 0x67, 0x20, 0x46, 0x75,
		0xe2, 0x32, 0xfc, 0xf1, 0x91, 0x12, 0x91, 0x88, 0xb1, 0x59, 0xe4, 0xe6, 0xd6, 0x79, 0xa2, 0x93,
		0x56, 0x08, 0x20, 0x07, 0xc7, 0x1a, 0xb1, 0x8f, 0x76, 0x43, 0x55, 0x69, 0xa0, 0x3a, 0xf7, 0xfa,
		0xd2, 0x60, 0x0d, 0xe7, 0x15, 0x7a, 0xbc, 0x68, 0x63, 0x39, 0xe9, 0x01, 0xc3, 0x03, 0x1e, 0xfb,
		0xa1, 0x12, 0x02, 0xc9, 0xb4, 0x68, 0xbe, 0xa1, 0xd7, 0x51, 0x57, 0xa0, 0x14, 0x52, 0x49, 0x5b,
		0xb1, 0x29, 0x3b, 0x33, 0x05, 0x41, 0x85, 0x92, 0xd2, 0x10, 0xd2, 0x32, 0xc6, 0x42, 0x9b, 0x69,
		0xbd, 0x3d, 0xc2, 0x87, 0xb8, 0x7c, 0x47, 0x15, 0x6a, 0x6c, 0x95, 0x27, 0xac, 0x2e, 0x0e, 0x4e,
		0xcc, 0x96, 0xed, 0x16, 0x74, 0xea, 0xaa, 0x03, 0x1e, 0x86, 0x3f, 0x24, 0xb2, 0xa8, 0x31, 0x6a,
		0x8e, 0x51, 0xef, 0x21, 0xfa, 0xbb, 0x45, 0x22, 0xe4, 0x3d, 0x7a, 0x06, 0x56, 0x95, 0x4b, 0x6c,
		0xbf, 0xe2, 0xbf, 0x90, 0x45, 0x59, 0xfa, 0xb2, 0xa1, 0x64, 0x80, 0xb4, 0xf7, 0xf1, 0xcb, 0xd8,
		0x28, 0xfd, 0xde, 0xf8, 0x6d, 0xa4, 0x24, 0x4a, 0xcc, 0xc0, 0xa4, 0xfe, 0x3b, 0x31, 0x6f, 0x26
	};
	assert(cmp(v.data(), expected, roundkeyBytes));
	std::cout << "aes128 key expansion passed" << std::endl;
}

void check_iexpand_keys128()
{
	auto key = "Thats my Kung Fu";
	auto v = aes::expand_keys(key, aes::AES128);
	auto r = aes::iexpand_keys(v);
	aes::ubyte expected[] = {
		0x28, 0xfd, 0xde, 0xf8, 0x6d, 0xa4, 0x24, 0x4a, 0xcc, 0xc0, 0xa4, 0xfe, 0x3b, 0x31, 0x6f, 0x26,
		0xca, 0x44, 0x2e, 0xd2, 0x75, 0x47, 0x62, 0x04, 0x8c, 0xe2, 0x54, 0xcb, 0xb6, 0x9c, 0xa0, 0x9f,
		0x2d, 0x74, 0x6e, 0x26, 0xbf, 0x03, 0x4c, 0xd6, 0xf9, 0xa5, 0x36, 0xcf, 0x3a, 0x7e, 0xf4, 0x54,
		0x22, 0x88, 0x5a, 0x51, 0x92, 0x77, 0x22, 0xf0, 0x46, 0xa6, 0x7a, 0x19, 0xc3, 0xdb, 0xc2, 0x9b,
		0xf6, 0xb1, 0x18, 0x9a, 0xb0, 0xff, 0x78, 0xa1, 0xd4, 0xd1, 0x58, 0xe9, 0x85, 0x7d, 0xb8, 0x82,
		0xf8, 0x19, 0x49, 0x38, 0x46, 0x4e, 0x60, 0x3b, 0x64, 0x2e, 0x20, 0x48, 0x51, 0xac, 0xe0, 0x6b,
		0xf3, 0xe6, 0xa0, 0xcd, 0xbe, 0x57, 0x29, 0x03, 0x22, 0x60, 0x40, 0x73, 0x35, 0x82, 0xc0, 0x23,
		0x52, 0x3e, 0x3d, 0x09, 0x4d, 0xb1, 0x89, 0xce, 0x9c, 0x37, 0x69, 0x70, 0x17, 0xe2, 0x80, 0x50,
		0xb5, 0xf8, 0x31, 0x05, 0x1f, 0x8f, 0xb4, 0xc7, 0xd1, 0x86, 0xe0, 0xbe, 0x8b, 0xd5, 0xe9, 0x20,
		0x4e, 0xfb, 0xf1, 0x99, 0xaa, 0x77, 0x85, 0xc2, 0xce, 0x09, 0x54, 0x79, 0x5a, 0x53, 0x09, 0x9e,
		0x54, 0x68, 0x61, 0x74, 0x73, 0x20, 0x6d, 0x79, 0x20, 0x4b, 0x75, 0x6e, 0x67, 0x20, 0x46, 0x75,
	};
	assert(cmp(r.data(), expected, r.size() * sizeof(r[0])));
	std::cout << "aes128 inverse key expansion passed" << std::endl;
}

void check_make_key()
{
	assert(aes::make_key("", 0, aes::AES128).size() == 16);
	assert(aes::make_key("", 0, aes::AES192).size() == 24);
	assert(aes::make_key("", 0, aes::AES256).size() == 32);

	assert(aes::make_key("0123456789012345678901234567890123456789", 40, aes::AES128).size() == 16);
	assert(aes::make_key("0123456789012345678901234567890123456789", 40, aes::AES192).size() == 24);
	assert(aes::make_key("0123456789012345678901234567890123456789", 40, aes::AES256).size() == 32);

	assert(aes::make_key("0123456789", 10, aes::AES128).size() == 16);
	assert(aes::make_key("0123456789", 10, aes::AES192).size() == 24);
	assert(aes::make_key("0123456789", 10, aes::AES256).size() == 32);

	std::cout << "make keys passed" << std::endl;
}

int main()
{
	check_expand_keys128();
	check_iexpand_keys128();
	check_expand_keys256();
	check_iexpand_keys256();
	check_make_key();
	std::cout << "pass" << std::endl;
}
