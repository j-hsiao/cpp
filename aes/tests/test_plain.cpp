//example  (128)
//
//	key: "Thats my Kung Fu"
//	hex  5468617473206d79204b756e67204675
//
//	plain: "Two One Nine Two"
//
//	encrypted result:
//	hex  29c3505f571420f6402299b31a02d73a

#ifndef AES_DEBUG
#define AES_DEBUG
#endif

#include <aes/aes.hpp>
#include <aes/aes_util.hpp>
#include <aes/aes_plain.hpp>
#include <aes/aes_key.hpp>
#include <aes/aes_consts.hpp>
#include <aes/aes_types.hpp>

#include <cassert>
#include <numeric>
#include <string>

namespace aes128ex
{
	aes::ubyte key[] = {
		0x54, 0x68, 0x61, 0x74, 
		0x73, 0x20, 0x6d, 0x79, 
		0x20, 0x4b, 0x75, 0x6e, 
		0x67, 0x20, 0x46, 0x75
	};
	aes::ubyte text[] = {
		0x54, 0x77, 0x6f, 0x20, 
		0x4f, 0x6e, 0x65, 0x20, 
		0x4e, 0x69, 0x6e, 0x65, 
		0x20, 0x54, 0x77, 0x6f
	};
	aes::ubyte addkey0[] = {
		0x00, 0x1f, 0x0e, 0x54,
		0x3c, 0x4e, 0x08, 0x59,
		0x6e, 0x22, 0x1b, 0x0b,
		0x47, 0x74, 0x31, 0x1a
	};
	aes::ubyte sub0[] = {
		0x63, 0xc0, 0xab, 0x20,
		0xeb, 0x2f, 0x30, 0xcb,
		0x9f, 0x93, 0xaf, 0x2b,
		0xa0, 0x92, 0xc7, 0xa2
	};
	aes::ubyte shift0[] = {
		0x63, 0x2f, 0xaf, 0xa2,
		0xeb, 0x93, 0xc7, 0x20,
		0x9f, 0x92, 0xab, 0xcb,
		0xa0, 0xc0, 0x30, 0x2b
	};

	aes::ubyte mix0[] = {
		0xba, 0x75, 0xf4, 0x7a,
		0x84, 0xa4, 0x8d, 0x32,
		0xe8, 0x8d, 0x06, 0x0e,
		0x1b, 0x40, 0x7d, 0x5d
	};
	
	aes::ubyte addkey1[] = {
		0x58, 0x47, 0x08, 0x8b,
		0x15, 0xb6, 0x1c, 0xba,
		0x59, 0xd4, 0xe2, 0xe8,
		0xcd, 0x39, 0xdf, 0xce
	};
	aes::ubyte sub1[] = {
		0x6a, 0xa0, 0x30, 0x3d,
		0x59, 0x4e, 0x9c, 0xf4,
		0xcb, 0x48, 0x98, 0x9b,
		0xbd, 0x12, 0x9e, 0x8b
	};
	aes::ubyte shift1[] = {
		0x6a, 0x4e, 0x98, 0x8b,
		0x59, 0x48, 0x9e, 0x3d,
		0xcb, 0x12, 0x30, 0xf4,
		0xbd, 0xa0, 0x9c, 0x9b
	};
}

void check_aesmul()
{
	//1, 2, 3, 14, 11, 9, 13
	for (std::size_t i = 0; i < 256; ++i)
	{
		aes::ubyte v = aes::aesmul(static_cast<aes::ubyte>(i), 1);
		assert(v == i);
	}
	std::cout << "mul1 passed" << std::endl;
	for (std::size_t i = 0; i < 256; ++i)
	{
		aes::ubyte v = aes::aesmul(static_cast<aes::ubyte>(i), 2);
		aes::ubyte e = aes::aesmul2(static_cast<aes::ubyte>(i));
		assert(e == v);
	}
	std::cout << "mul2 passed" << std::endl;
	for (std::size_t i = 0; i < 256; ++i)
	{
		aes::ubyte v = aes::aesmul(static_cast<aes::ubyte>(i), 3);
		aes::ubyte e = static_cast<aes::ubyte>(aes::aesmul2(static_cast<aes::ubyte>(i)) ^ i);
		assert(e == v);
	}
	std::cout << "mul3 passed" << std::endl;
	for (std::size_t j = 0; j < 256; ++j)
	{
		aes::ubyte i = static_cast<aes::ubyte>(j);
		aes::ubyte v = aes::aesmul(i, 14);
		aes::ubyte e = (
			aes::aesmul2(aes::aesmul2(aes::aesmul2(i)))
			^ aes::aesmul2(aes::aesmul2(i))
			^ aes::aesmul2(i));
		assert(e == v);
	}
	std::cout << "mul14 passed" << std::endl;
	for (std::size_t j = 0; j < 256; ++j)
	{
		aes::ubyte i = static_cast<aes::ubyte>(j);
		aes::ubyte v = aes::aesmul(i, 11);
		aes::ubyte e = (
			aes::aesmul2(aes::aesmul2(aes::aesmul2(i)))
			^ aes::aesmul2(i)
			^ i);
		assert(e == v);
	}
	std::cout << "mul11 passed" << std::endl;
	for (std::size_t j = 0; j < 256; ++j)
	{
		aes::ubyte i = static_cast<aes::ubyte>(j);
		aes::ubyte v = aes::aesmul(i, 13);
		aes::ubyte e = (
			aes::aesmul2(aes::aesmul2(aes::aesmul2(i)))
			^ aes::aesmul2(aes::aesmul2(i))
			^ i);
		assert(e == v);
	}
	std::cout << "mul13 passed" << std::endl;
	for (std::size_t j = 0; j < 256; ++j)
	{
		aes::ubyte i = static_cast<aes::ubyte>(j);
		aes::ubyte v = aes::aesmul(i, 9);
		aes::ubyte e = (
			aes::aesmul2(aes::aesmul2(aes::aesmul2(i)))
			^ i);
		assert(e == v);
	}
	std::cout << "mul9 passed" << std::endl;
}
void gmix_column(unsigned char *r) {
    unsigned char a[4];
    unsigned char b[4];
    unsigned char c;
    unsigned char h;
    /* The array 'a' is simply a copy of the input array 'r'
     * The array 'b' is each element of the array 'a' multiplied by 2
     * in Rijndael's Galois field
     * a[n] ^ b[n] is element n multiplied by 3 in Rijndael's Galois field */ 
    for (c = 0; c < 4; c++) {
        a[c] = r[c];
        /* h is 0xff if the high bit of r[c] is set, 0 otherwise */
        h = (unsigned char)((signed char)r[c] >> 7); /* arithmetic right shift, thus shifting in either zeros or ones */
        b[c] = r[c] << 1; /* implicitly removes high bit because b[c] is an 8-bit char, so we xor by 0x1b and not 0x11b in the next line */
        b[c] ^= 0x1B & h; /* Rijndael's Galois field */
    }
    r[0] = b[0] ^ a[3] ^ a[2] ^ b[1] ^ a[1]; /* 2 * a0 + a3 + a2 + 3 * a1 */
    r[1] = b[1] ^ a[0] ^ a[3] ^ b[2] ^ a[2]; /* 2 * a1 + a0 + a3 + 3 * a2 */
    r[2] = b[2] ^ a[1] ^ a[0] ^ b[3] ^ a[3]; /* 2 * a2 + a1 + a0 + 3 * a3 */
    r[3] = b[3] ^ a[2] ^ a[1] ^ b[0] ^ a[0]; /* 2 * a3 + a2 + a1 + 3 * a0 */
}

void check_round()
{
	auto roundkeys = aes::expand_keys(aes128ex::key, aes::aes128);
	auto iroundkeys = aes::iexpand_keys(roundkeys);

	aes::AESState state1;
	aes::AESState buff;

	state1 << aes128ex::text;
	aes::ubyte *src = reinterpret_cast<aes::ubyte*>(&state1);
	aes::AESWord *state = reinterpret_cast<aes::AESWord*>(&buff);

	assert(std::memcmp(&state1, aes128ex::text, 16) == 0);
	state1 ^= roundkeys[0];
	assert(std::memcmp(&state1, aes128ex::addkey0, 16) == 0);

	state[0] = aes::k_mix[0][src[0]] ^ aes::k_mix[1][src[5]] ^ aes::k_mix[2][src[10]] ^ aes::k_mix[3][src[15]];
	state[1] = aes::k_mix[0][src[4]] ^ aes::k_mix[1][src[9]] ^ aes::k_mix[2][src[14]] ^ aes::k_mix[3][src[3]];
	state[2] = aes::k_mix[0][src[8]] ^ aes::k_mix[1][src[13]] ^ aes::k_mix[2][src[2]] ^ aes::k_mix[3][src[7]];
	state[3] = aes::k_mix[0][src[12]] ^ aes::k_mix[1][src[1]] ^ aes::k_mix[2][src[6]] ^ aes::k_mix[3][src[11]];

	assert(std::memcmp(&buff, aes128ex::mix0, 16) == 0);
	state1 = buff ^ roundkeys[1];
	assert(std::memcmp(&state1, aes128ex::addkey1, 16) == 0);

	for (std::size_t i = 0; i < aes::STATEBYTES; ++i)
	{
		reinterpret_cast<aes::ubyte*>(&buff)[i] = aes::k_sbox[src[aes::k_shiftrows[i]]];
	}

	assert(std::memcmp(&buff, aes128ex::shift1, 16) == 0);

//begin test backwards
	state1 = buff;
	state[0] = aes::k_imix[0][src[0]] ^ aes::k_imix[1][src[13]] ^ aes::k_imix[2][src[10]] ^ aes::k_imix[3][src[7]];
	state[1] = aes::k_imix[0][src[4]] ^ aes::k_imix[1][src[1]] ^ aes::k_imix[2][src[14]] ^ aes::k_imix[3][src[11]];
	state[2] = aes::k_imix[0][src[8]] ^ aes::k_imix[1][src[5]] ^ aes::k_imix[2][src[2]] ^ aes::k_imix[3][src[15]];
	state[3] = aes::k_imix[0][src[12]] ^ aes::k_imix[1][src[9]] ^ aes::k_imix[2][src[6]] ^ aes::k_imix[3][src[3]];
	buff ^= iroundkeys[iroundkeys.size() - 2];
	assert(std::memcmp(&buff, aes128ex::shift0, 16) == 0);




	std::cout << "round passed" << std::endl;


}

void check_mix()
{
	for (std::size_t i = 0; i < 256; ++i)
	{
		assert(i == aes::k_ibox[aes::k_sbox[i]]);
		assert(i == aes::k_sbox[aes::k_ibox[i]]);
	}
	aes::AESWord w1;
	aes::AESWord w2;

	aes::ubyte *bw1 = reinterpret_cast<aes::ubyte*>(&w1);
	aes::ubyte *bw2 = reinterpret_cast<aes::ubyte*>(&w2);

	//is the 0xfa 0x0a 0x22 0x5c test vector on wikipedia incorrect?
	aes::ubyte src[][4] = {
		{0xdb, 0x13, 0x53, 0x45},
		{0x01, 0x01, 0x01, 0x01},
		{0xc6, 0xc6, 0xc6, 0xc6},
		{0xd4, 0xd4, 0xd4, 0xd5},
		{0x2d, 0x26, 0x31, 0x4c},
		{0xfa, 0x0a, 0x22, 0x5c}
	};
	aes::ubyte mixed[][4] = {
		{0x8e, 0x4d, 0xa1, 0xbc},
		{0x01, 0x01, 0x01, 0x01},
		{0xc6, 0xc6, 0xc6, 0xc6},
		{0xd5, 0xd5, 0xd7, 0xd6},
		{0x4d, 0x7e, 0xbd, 0xf8},
		{0x9f, 0xdc, 0x58, 0x9d}
	};
	for (std::size_t i = 0; i < 6; ++i)
	{
		aes::ubyte *s = src[i];

		unsigned char check[4];
		std::memcpy(check, s, 4);
		gmix_column(check);
		w1 = aes::k_mix[0][aes::k_ibox[s[0]]] ^ aes::k_mix[1][aes::k_ibox[s[1]]] ^ aes::k_mix[2][aes::k_ibox[s[2]]] ^ aes::k_mix[3][aes::k_ibox[s[3]]];
		assert(memcmp(&w1, check, aes::k_wordbytes) == 0);

		w2 = aes::k_imix[0][aes::k_sbox[bw1[0]]] ^ aes::k_imix[1][aes::k_sbox[bw1[1]]] ^ aes::k_imix[2][aes::k_sbox[bw1[2]]] ^ aes::k_imix[3][aes::k_sbox[bw1[3]]];
		assert(memcmp(&w2, s, aes::k_wordbytes) == 0);
	}
	std::cout << "mixcols passed" << std::endl;
}
void check_sboxes()
{
	for (int i = 0; i < 256; ++i)
	{
		assert(aes::k_sbox[aes::k_ibox[i]] == i);
		assert(aes::k_ibox[aes::k_sbox[i]] == i);
	}
	std::cout << "sbox/ibox pass" << std::endl;
}
void check128()
{
	aes::ubyte key[] = "Thats my Kung Fu";
	aes::ubyte text[] = "Two One Nine Two";
	aes::ubyte expected[] = {
		0x29,0xc3,0x50,0x5f,
		0x57,0x14,0x20,0xf6,
		0x40,0x22,0x99,0xb3,
		0x1a,0x02,0xd7,0x3a,
	};
	aes::AESState state;

	auto roundkeys = aes::expand_keys(key, aes::aes128);
	auto iroundkeys = aes::iexpand_keys(roundkeys);
	state << text;
	aes::AES_plain::encrypt(state, roundkeys);
	assert(
		std::string(reinterpret_cast<const char*>(&state), 16)
		== std::string(reinterpret_cast<const char*>(expected), 16));
	std::cout << "aes 128 encrypt state passed" << std::endl;


	aes::AES_plain::decrypt(state, iroundkeys);
	assert(
		std::string(reinterpret_cast<const char*>(&state), 16)
		== "Two One Nine Two");


	std::cout << "aes 128 decrypt state passed" << std::endl;
}

void check192()
{
	aes::ubyte key[] = "0123456789abcdef01234567";
	//hex: 303132333435363738396162636465663031323334353637
	aes::ubyte text[] = "Two One Nine Two";
	aes::ubyte expected[] = {
		0x3e, 0xa0, 0x74, 0x24,
		0xa7, 0x07, 0x1d, 0xaa,
		0x66, 0x20, 0x5c, 0x60,
		0x70, 0xc4, 0xa1, 0x8f
	};
	aes::AESState state;

	auto roundkeys = aes::expand_keys(key, aes::aes192);
	auto iroundkeys = aes::iexpand_keys(roundkeys);
	state << text;
	aes::AES_plain::encrypt(state, roundkeys);

	assert(
		std::string(reinterpret_cast<const char*>(&state), 16)
		== std::string(reinterpret_cast<const char*>(expected), 16));
	std::cout << "aes 192 encrypt pased" << std::endl;
	aes::AES_plain::decrypt(state, iroundkeys);
	assert(
		std::string(reinterpret_cast<const char*>(&state), 16)
		== "Two One Nine Two");
	std::cout << "aes 192 decrypt pased" << std::endl;
}

void check256()
{
	aes::ubyte key[] = "0123456789abcdef0123456789abcdef";
	//hex: 3031323334353637383961626364656630313233343536373839616263646566
	aes::ubyte text[] = "Two One Nine Two";
	aes::ubyte expected[] = {
		0xf2, 0x84, 0x66, 0x7e,
		0x56, 0x94, 0xc6, 0x5c,
		0x0c, 0xc4, 0xb4, 0x96,
		0x31, 0x9f, 0xbe, 0xea
	};
	aes::AESState state;
	auto roundkeys = aes::expand_keys(key, aes::aes256);
	auto iroundkeys = aes::iexpand_keys(roundkeys);
	state << text;
	aes::AES_plain::encrypt(state, roundkeys);

	assert(
		std::string(reinterpret_cast<const char*>(&state), 16)
		== std::string(reinterpret_cast<const char*>(expected), 16));
	std::cout << "aes 256 encrypt pased" << std::endl;
	aes::AES_plain::decrypt(state, iroundkeys);
	assert(
		std::string(reinterpret_cast<const char*>(&state), 16)
		== "Two One Nine Two");
	std::cout << "aes 256 decrypt pased" << std::endl;
}

int main(int argc, char *argv[])
{
	check_aesmul();
	check_mix();
	check_sboxes();
	check_round();
	check128();
	check192();
	check256();
}
