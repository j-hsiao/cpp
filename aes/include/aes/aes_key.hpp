#ifndef AES_KEY_H
#define AES_KEY_H

#include <aes/aes_types.hpp>
#include <aes/aes.hpp>

namespace aes
{
	AES_statevec expand_keys(const void *key, AESVersion v);
	AES_statevec iexpand_keys(const AES_statevec &roundkeys);
}
#endif
