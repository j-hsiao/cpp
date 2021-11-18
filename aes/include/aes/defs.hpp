#ifndef AES_IMPL_CONSTS_HPP
#define AES_IMPL_CONSTS_HPP

#include "aes/aes_dllcfg.h"
#include <cstddef>
#include <type_traits>
#include <cstring>
#include <utility>

namespace aes
{
	static const std::size_t Byte_Bits = 8;
	static const std::size_t Word_Bytes = 4;
	static const std::size_t Word_Bits = Word_Bytes * Byte_Bits;
	static const std::size_t State_Bytes = 16;
	static const std::size_t State_Words = State_Bytes / Word_Bytes;
	
	struct Version
	{
		const std::size_t Round_Keys;
		const std::size_t Key_Bytes;
	};
	static const Version aes128 {11, 16};
	static const Version aes192 {13, 24};
	static const Version aes256 {15, 32};
	static const std::size_t Max_Rounds = 15;
	static const std::size_t Max_Key_Bytes = 32;

	static const unsigned char rot[16] = {
		0, 5, 10, 15,
		4, 9, 14, 3,
		8, 13, 2, 7,
		12, 1, 6, 11};
	static const unsigned char irot[16] = {
		0, 13, 10, 7,
		4, 1, 14, 11,
		8, 5, 2, 15,
		12, 9, 6, 3};

	const unsigned char* SBox();
	const unsigned char* IBox();


	//aes multiplication mod 0x11b in the galois field
	unsigned char mul(unsigned char a, unsigned int val);
	unsigned char rcon(std::size_t i);

	struct word { unsigned char data[Word_Bytes]; };
	inline word operator^(word a, const word &b)
	{
		a.data[0] ^= b.data[0];
		a.data[1] ^= b.data[1];
		a.data[2] ^= b.data[2];
		a.data[3] ^= b.data[3];
		return a;
	}
	word mixcol(unsigned char c, unsigned char column);
	word imixcol(unsigned char c, unsigned char column);
}
#endif
