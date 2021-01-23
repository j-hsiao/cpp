// goals:
//	1. learn cmake/installing packages etc
//	2. learn SSE/SSE2 etc 
//		after reading up some more, it seems that
//		generally have to use:
//			gcc/g++	: __cpuid or __cpuid_count
//			windows	: ????
//			and then pick function pointers
//			each version is compiled within its own Translation Unit
//			and then pointers are set to use the corresponding functions
//
//			this was called CPU dispatching or something...
//
//	3. understand AES
//
// algorithm:
//			rounds  key words
// 128: 10      4
// 192: 12      6
// 256: 14      8
//
// roundkeys: need rounds + 1
//    constants = [rc_i 0 0 0] 4 bytes
//    rc_1 = 1
//    rc_i = (rc_i - 1) << 2 ^ (rc_i & 0x80 ? 0x1b : 0)
//
//    then:
//    N = words in key (
//    words_0 to words_(N - 1) = key
//    i == 0 mod N ? :      w_(i - N) ^ (sub(rotate(w_(i - 1)))) ^ rc_(i / N)
//    N > 6, i == 4 mod N:  w_(i - N) ^ sub(w_(i - 1))
//    else:                 w_(i - N) ^ w_(i - 1)
//
//  aes:
//    notes:
//      1. data is "column-major", so are round keys
//      2. state = 4 words (16 bytes)
//
//    1:  add roundkey  xor byte by byte)
//    2:  R - 1 rounds of:
//          SubBytes      run state through SBOX)
//          ShiftRows     shift row X left by X)
//          MixCols       matmul
//          add roundkey
//    3:  Rth round: (no mixcols)
//          SubBytes
//          ShiftRows
//          add roundkey
//
//    decryption:run in reverse
//    1: add roundkey
//
//    2: R - 1 rounds of:
//        IShiftRows
//        ISubBytes
//        add roundkey    -> cannot precompute like with forward
//        IMixCols        -> BUT if compute IMixCols on roundkey
//                        -> then can swap:
//                        I * (state ^ roundkey) = (I * state) ^ (I * roundkey)
//                        -> xor is commutative
//                        so pre-compute (I * roundkey)
//    3: Rth round
//        IShiftRows
//        ISubBytes
//        add roundkey
//
//
//  mixcol matrices:
//      forward
//    02  03  01  01
//    01  02  03  01
//    01  01  02  03
//    03  01  01  02
//      inverse
//    14  11  13  09
//    09  14  11  13
//    13  09  14  11
//    11  13  09  14
//
//
//  is mixcols(SubBytes(x) ^ a) ?= mixcols(SubBytes(x)) ^ mixcols(a)
//
//
//aes views data as column major data
//but C is usually interpreted in row-major order
//  ------------------------------
//  column major
//  ------------------------------
//    raw data
//  00  04  08  12
//  01  05  09  13
//  02  06  10  14
//  03  07  11  15
//
//    shift rows (left)		((i * 5) % 16)
//  00  04  08  12
//  05  09  13  01
//  10  14  02  06
//  15  03  07  11
//
//    shift rows(right) ((i * -3) % 16)
//  00  04  08  12
//  13  01  05  09
//  10  14  02  06
//  07  11  15  03
//
//  -------------------------------
//  transpose into row-major
//  ------------------------------
//    raw data
//  00  01  02  03
//  04  05  06  07
//  08  09  10  11
//  12  13  14  15
//
//    shift rows (left)
//  00  05  10  15
//  04  09  14  03
//  08  13  02  07
//  12  01  06  11
//
//    shift rows (right)
//  00  13  10  07
//  04  01  14  11
//  08  05  02  15
//  12  09  06  03
//
//
//  implementation possibilities:
//              transpose the data                  transpose operations
//  SubBytes    same                                same
//  shift rows  2 bit shifts and an |               12 copies?
//  mixcols     
//
//  wiki says 16 table lookups (sub + shift + mul mats)
//  12 32-bit xors (sum components of matmul)
//  4 32-bit xors (add keys)
//  
//
//
//
//    a b c d   00 04 08 12
//    d a b c   01 05 09 13
//    c d a b   02 06 10 14
//    b c d a   03 07 11 15
//
//
//    a0b1c3d3  a4b5c6d7
//    d0a1b2c3  d4a5b6c7
//    c0d1a2b3  c4d5a6b7
//    b0c1d2a3  b4c5d6a7
//
//    0*adcb ^ 1*badc ^ ...
//    4*adcb ^ 5*badc ^ ...
//
//    can just save the adcb or badc into LUT
//

//https://en.wikipedia.org/wiki/AES_key_schedule
//https://en.wikipedia.org/wiki/Advanced_Encryption_Standard
//
//
//
#include <aes/aes.h>
#include <.aes/consts.hpp>
#include <.aes/types.hpp>
#include <.aes/impl.hpp>

#include <algorithm>
#include <cstddef>

namespace
{
	struct plainstate
	{
		static void encrypt_state(
			aes__Word *dst, const aes__Word *src, const aes__Keys &keys)
		{
			aes__Word buf[aes__State_Words];
			std::copy(src, src + aes__State_Words, buf);
			aes::xor_state(buf, keys.encrypt[0]);
			auto lastkey = keys.encrypt + aes::NumRounds[keys.version];
			for (auto pkey = keys.encrypt + 1; pkey < lastkey; ++pkey)
			{
				dst[0] = aes::Mix[0][aes::get_state_byte(buf, 0)]
				 ^ aes::Mix[1][aes::get_state_byte(buf, 5)]
				 ^ aes::Mix[2][aes::get_state_byte(buf, 10)]
				 ^ aes::Mix[3][aes::get_state_byte(buf, 15)];
				dst[1] = aes::Mix[0][aes::get_state_byte(buf, 4)]
				 ^ aes::Mix[1][aes::get_state_byte(buf, 9)]
				 ^ aes::Mix[2][aes::get_state_byte(buf, 14)]
				 ^ aes::Mix[3][aes::get_state_byte(buf, 3)];
				dst[2] = aes::Mix[0][aes::get_state_byte(buf, 8)]
				 ^ aes::Mix[1][aes::get_state_byte(buf, 13)]
				 ^ aes::Mix[2][aes::get_state_byte(buf, 2)]
				 ^ aes::Mix[3][aes::get_state_byte(buf, 7)];
				dst[3] = aes::Mix[0][aes::get_state_byte(buf, 12)]
				 ^ aes::Mix[1][aes::get_state_byte(buf, 1)]
				 ^ aes::Mix[2][aes::get_state_byte(buf, 6)]
				 ^ aes::Mix[3][aes::get_state_byte(buf, 11)];
				aes::xor_state(buf, dst, *pkey);
			}
			for (std::size_t i = 0; i < aes__State_Bytes; ++i)
			{
				aes::set_state_byte(
					dst, 
					aes::SBox[aes::get_state_byte(buf, aes::ShiftRows[i])],
					i);
			}
			aes::xor_state(dst, *lastkey);
		}

		static void decrypt_state(
			aes__Word *dst, const aes__Word *src, const aes__Keys &keys)
		{
			aes__Word buf[aes__State_Words];
			std::copy(src, src + aes__State_Words, buf);
			aes::xor_state(buf, keys.decrypt[0]);
			auto lastkey = keys.decrypt + aes::NumRounds[keys.version];
			for (auto pkey = keys.decrypt + 1; pkey < lastkey; ++pkey)
			{
				dst[0] = aes::IMix[0][aes::get_state_byte(buf, 0)]
				 ^ aes::IMix[1][aes::get_state_byte(buf, 13)]
				 ^ aes::IMix[2][aes::get_state_byte(buf, 10)]
				 ^ aes::IMix[3][aes::get_state_byte(buf, 7)];
				dst[1] = aes::IMix[0][aes::get_state_byte(buf, 4)]
				 ^ aes::IMix[1][aes::get_state_byte(buf, 1)]
				 ^ aes::IMix[2][aes::get_state_byte(buf, 14)]
				 ^ aes::IMix[3][aes::get_state_byte(buf, 11)];
				dst[2] = aes::IMix[0][aes::get_state_byte(buf, 8)]
				 ^ aes::IMix[1][aes::get_state_byte(buf, 5)]
				 ^ aes::IMix[2][aes::get_state_byte(buf, 2)]
				 ^ aes::IMix[3][aes::get_state_byte(buf, 15)];
				dst[3] = aes::IMix[0][aes::get_state_byte(buf, 12)]
				 ^ aes::IMix[1][aes::get_state_byte(buf, 9)]
				 ^ aes::IMix[2][aes::get_state_byte(buf, 6)]
				 ^ aes::IMix[3][aes::get_state_byte(buf, 3)];
				aes::xor_state(buf, dst, *pkey);
			}
			for (std::size_t i = 0; i < aes__State_Bytes; ++i)
			{
				aes::set_state_byte(
					dst, 
					aes::IBox[aes::get_state_byte(buf, aes::IShiftRows[i])],
					i);
			}
			aes::xor_state(dst, *lastkey);
		}
	};
}

namespace aes
{
	std::size_t PlainImpl::encrypt_ecb(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::encrypt_ecb<plainstate>(dst, src, keys, datalen); }

	std::size_t PlainImpl::decrypt_ecb(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::decrypt_ecb<plainstate>(dst, src, keys, datalen); }

	std::size_t PlainImpl::encrypt_cbc(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::encrypt_cbc<plainstate>(dst, src, keys, datalen); }

	std::size_t PlainImpl::decrypt_cbc(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{ return aes::decrypt_cbc<plainstate>(dst, src, keys, datalen); }
}

