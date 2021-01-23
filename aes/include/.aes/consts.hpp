#ifndef AES_CONSTS_HPP
#define AES_CONSTS_HPP

#include <aes/aes.h>

#include <cstddef>

namespace aes
{
	static const std::size_t NumRounds[] = {10, 12, 14};
	extern const aes__Byte SBox[256];
	extern const aes__Byte IBox[256];
	extern const aes__Word (* const Mix)[256];
	extern const aes__Word (* const IMix)[256];

	static const aes__Byte ShiftRows[] = 
	{0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11};
	static const aes__Byte IShiftRows[] = 
	{0, 13, 10, 7, 4, 1, 14, 11, 8, 5, 2, 15, 12, 9, 6, 3};
}
#endif
