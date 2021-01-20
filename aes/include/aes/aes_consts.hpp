#ifndef AES_CONSTS_H
#define AES_CONSTS_H
#include <aes/aes_types.hpp>
#include <aes/aes.hpp>


namespace aes
{
	extern const std::size_t NUM_ROUNDS[];
	extern const std::size_t NUM_CONSTS[];
	extern const std::size_t NUM_KWORDS[];

	extern const ubyte SBOX[256];
	extern const ubyte IBOX[256];
	extern const ubyte shiftrows[16];
	extern const ubyte ishiftrows[16];

	extern const AES_wordvec MIX[4];
	extern const AES_wordvec IMIX[4];
}
#endif
