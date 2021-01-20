#ifndef AES_CONSTS_H
#define AES_CONSTS_H
#include <aes/aes_types.hpp>
#include <aes/aes.hpp>


namespace aes
{
	extern const std::size_t k_nrounds[];
	extern const std::size_t k_nconsts[];
	extern const std::size_t k_nkeywords[];

	extern const ubyte k_sbox[256];
	extern const ubyte k_ibox[256];
	extern const ubyte k_shiftrows[16];
	extern const ubyte k_ishiftrows[16];

	extern const AES_wordvec k_mix[4];
	extern const AES_wordvec k_imix[4];
}
#endif
