//plain base aes codec

#include <aes/aes_plain.hpp>
#include <aes/aes_types.hpp>
#include <aes/aes_consts.hpp>

#include <aes/aes_util.hpp>

#include <cstring>
namespace aes
{
//------------------------------
//  encrypt/decrypt
//------------------------------
	void AES_plain::encrypt(AESState &data, const AES_statevec &roundkeys)
	{
		AESState buff;
		AESWord *state = reinterpret_cast<AESWord*>(&buff);
		ubyte *src = reinterpret_cast<ubyte*>(&data);
		const AESState *keys = roundkeys.data();
		const AESState * const last = keys + roundkeys.size() - 1;

		//dereference and access through data() improves speed by nearly 2x
		auto mix0 = MIX[0].data();
		auto mix1 = MIX[1].data();
		auto mix2 = MIX[2].data();
		auto mix3 = MIX[3].data();

		data ^= *keys;
		++keys;
		for (; keys < last; ++keys)
		{
			state[0] = mix0[src[0]] ^ mix1[src[5]] ^ mix2[src[10]] ^ mix3[src[15]];
			state[1] = mix0[src[4]] ^ mix1[src[9]] ^ mix2[src[14]] ^ mix3[src[3]];
			state[2] = mix0[src[8]] ^ mix1[src[13]] ^ mix2[src[2]] ^ mix3[src[7]];
			state[3] = mix0[src[12]] ^ mix1[src[1]] ^ mix2[src[6]] ^ mix3[src[11]];
			data.xorinto(buff, *keys);
		}
		for (std::size_t i = 0; i < STATEBYTES; ++i)
		{
			reinterpret_cast<ubyte*>(&buff)[i] = SBOX[src[shiftrows[i]]];
		}
		data.xorinto(buff, *last);
	}

	void AES_plain::decrypt(AESState &data, const AES_statevec &iroundkeys)
	{
		AESState buff;
		AESWord *state = reinterpret_cast<AESWord*>(&buff);
		ubyte *src = reinterpret_cast<ubyte*>(&data);
		const AESState *keys = iroundkeys.data();
		const AESState * const last = keys + iroundkeys.size() - 1;

		//dereference and access through data() improves speed by nearly 2x
		auto imix0 = IMIX[0].data();
		auto imix1 = IMIX[1].data();
		auto imix2 = IMIX[2].data();
		auto imix3 = IMIX[3].data();

		data ^= *keys;
		++keys;
		for (; keys < last; ++keys)
		{
			state[0] = imix0[src[0]] ^ imix1[src[13]] ^ imix2[src[10]] ^ imix3[src[7]];
			state[1] = imix0[src[4]] ^ imix1[src[1]] ^ imix2[src[14]] ^ imix3[src[11]];
			state[2] = imix0[src[8]] ^ imix1[src[5]] ^ imix2[src[2]] ^ imix3[src[15]];
			state[3] = imix0[src[12]] ^ imix1[src[9]] ^ imix2[src[6]] ^ imix3[src[3]];
			data.xorinto(buff, *keys);
		}
		for (std::size_t i = 0; i < STATEBYTES; ++i)
		{
			reinterpret_cast<ubyte*>(&buff)[i] = IBOX[src[ishiftrows[i]]];
		}
		data.xorinto(buff, *last);
	}
};
