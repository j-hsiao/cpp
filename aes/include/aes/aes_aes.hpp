#ifndef AES_AES_H
#define AES_AES_H
#include <aes/aes_types.hpp>

namespace aes
{
	class AES_aes
	{
		public:
			static void encrypt(
				AESState &state, const AES_statevec &roundkeys);
			static void decrypt(
				AESState &state, const AES_statevec &iroundkeys);
			static void xorstate(
				AESState *dst, AESState *state1, AESState *state2);
			static void xorstate(
				AESState *dst, AESState *other);
	};
}
#endif
