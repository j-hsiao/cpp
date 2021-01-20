#ifndef AES_PLAIN_H
#define AES_PLAIN_H

#include <aes/aes_types.hpp>
#include <aes/aes_util.hpp>

#include <cstring>

namespace aes
{
	class AES_plain
	{
		public:
			static void encrypt(
				AESState &state, const AES_statevec &roundkeys);
			static void decrypt(
				AESState &state, const AES_statevec &iroundkeys);
			static void xorstate(
				AESState *dst, AESState *state1, AESState *state2)
			{ *dst = *state1 ^ *state2; }
			static void xorstate(
				AESState *dst, AESState *other)
			{ *dst ^= *other; }
	};
}
#endif
