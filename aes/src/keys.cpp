//https://en.wikipedia.org/wiki/AES_key_schedule
#include <aes/aes.h>
#include <.aes/types.hpp>
#include <.aes/consts.hpp>

#include <algorithm>
#include <vector>

namespace
{
	sha256__Byte Round_Consts[] = {
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

	aes__Word subword(aes__Word w)
	{
		for (size_t i = 0; i < aes__Word_Bytes; ++i)
		{ aes::set_byte(w, aes::SBox[aes::get_byte(w, i)], i); }
		return w;
	}
}
void aes__init_roundkeys(aes__Keys *keys, const aes__Byte *key, aes__Version version)
{
	keys->version = version;
	aes__Word buf[aes__Max_Keys * aes__State_Words];
	const size_t nroundkeys = aes::NumRounds[version] + 1;
	const size_t numwords =  nroundkeys * aes__State_Words;
	const size_t N = aes__Key_Bytes[version] / sha256__Word_Bytes;

	std::copy(key, key + aes__Key_Bytes[version], keys->key);
	aes::load_words(buf, key, N * aes__Word_Bytes);
	for (size_t i = N; i < numwords; ++i)
	{
		if (i % N == 0)
		{
			buf[i] = (
				buf[i - N]
				^ subword(aes::lrot(buf[i - 1], aes__Byte_Bits))
				^ Round_Consts[(i / N) - 1]);
		}
		else if (N > 6 && (i % N) == 4)
		{
			buf[i] = buf[i - N] ^ subword(buf[i - 1]);
		}
		else
		{
			buf[i] = buf[i - N] ^ buf[i - 1];
		}
	}
	for (size_t i = 0; i < numwords; i += aes__State_Words)
	{
		std::copy(
			buf + i,
			buf + i + aes__State_Words, 
			keys->encrypt[i / aes__State_Words]);
	}
	//prep decryption keys
	std::copy(
		keys->encrypt[0],
		keys->encrypt[0] + aes__State_Words,
		keys->decrypt[nroundkeys - 1]);
	std::copy(
		keys->encrypt[nroundkeys - 1],
		keys->encrypt[nroundkeys - 1] + aes__State_Words,
		keys->decrypt[0]);
	for (size_t dst = 1; dst < nroundkeys - 1; ++dst)
	{
		size_t src = (nroundkeys - 1) - dst;
		for (size_t word = 0; word < aes__State_Words; ++word)
		{
			keys->decrypt[dst][word] = 
				aes::IMix[0][aes::SBox[aes::get_byte(keys->encrypt[src][word], 0)]]
				^ aes::IMix[1][aes::SBox[aes::get_byte(keys->encrypt[src][word], 1)]]
				^ aes::IMix[2][aes::SBox[aes::get_byte(keys->encrypt[src][word], 2)]]
				^ aes::IMix[3][aes::SBox[aes::get_byte(keys->encrypt[src][word], 3)]];
		}
	}
}
