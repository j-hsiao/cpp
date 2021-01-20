#ifndef AES_CONSTS_H
#define AES_CONSTS_H
#include <aes/aes_types.hpp>
#include <aes/aes.hpp>


namespace aes
{
	extern const std::size_t NumRounds[];
	extern const std::size_t NumConsts[];
	extern const std::size_t NumKeywords[];

	extern const ubyte SBox[256];
	extern const ubyte IBox[256];
	extern const ubyte Shiftrows[16];
	extern const ubyte IShiftrows[16];

	extern const AES_wordvec Mix[4];
	extern const AES_wordvec IMix[4];
}
#endif
