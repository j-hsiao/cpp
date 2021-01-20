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



#include <aes/aes.hpp>
#include <aes/aes_key.hpp>
#include <aes/aes_util.hpp>
#include <aes/aes_plain.hpp>
#include <aes/aes_types.hpp>

#if !defined(AES_FORCE_B) && !defined(AES_FORCE_MB)

#ifndef AES_FORCE_SSL
#include <cpuinfo/cpuinfo.hpp>
#include <aes/aes_aes.hpp>
#endif

#ifdef AES_WITH_SSL
#include <aes/aes_ssl.hpp>
#endif

#endif


#include <cstdint>
#include <stdexcept>
#include <vector>

namespace aes
{
	//check runtime against compiled?
	//is this necessary? doesn't hurt probably...
	const int AES::ENDIAN = []()
	{
#if AES_ENDIAN
		uint32_t asint = 0x01020304;
		ubyte *check = reinterpret_cast<ubyte*>(&asint);
		logd("checking endian\n");
		if (
			!(
				check[0] == 0x04 && check[1] == 0x03
				&& check[2] == 0x02 && check[3] == 0x01
				&& AES_ENDIAN == 2)
			&& !(
				check[0] == 0x01 && check[1] == 0x02
				&& check[2] == 0x03 && check[3] == 0x04
				&& AES_ENDIAN == 1))
		{
			throw std::runtime_error("compiled and runtime endian do not match");
		}
#endif
		return AES_ENDIAN;
	}();


	struct dispatch
	{
		void (*encrypt)(
			void *state, std::size_t length, const AES_statevec &key);

		std::size_t (*decrypt)(
			void *state,
			std::size_t length,
			const AES_statevec &key,
			const AES_statevec &ikey
		);

		void (*encryptcbc)(
			void *state, std::size_t length, const AES_statevec &key);

		std::size_t (*decryptcbc)(
			void *state,
			std::size_t length,
			const AES_statevec &key,
			const AES_statevec &ikey);

		dispatch(bool v = 0)
		{
#if !defined(AES_FORCE_B) && !defined(AES_FORCE_MB)
#ifndef AES_FORCE_SSL
			logd("checking for aes\n");
			bool has_sse2 = cpuinfo::has("sse2");
			bool has_aes = cpuinfo::has("aes");
			logd("sse2: ") << has_sse2 << '\n';
			logd("aes: ") << has_aes << '\n';
			if (has_sse2 && has_aes)
			{
				logd("dispatching to aes intrinsics\n");
				encrypt = &encrypt_ecb<AES_aes>;
				decrypt = &decrypt_ecb<AES_aes>;
				encryptcbc = &encrypt_cbc<AES_aes>;
				decryptcbc = &decrypt_cbc<AES_aes>;
				return;
			}
#endif
#ifdef AES_WITH_SSL
			if (sizeof(ubyte) == sizeof(unsigned char))
			{
				logd("dispatching to openssl\n");
				encrypt = &encrypt_ssl_ecb;
				decrypt = &decrypt_ssl_ecb;
				encryptcbc = &encrypt_ssl_cbc;
				decryptcbc = &decrypt_ssl_cbc;
				return;
			}
#endif
#endif
			logd("dispatching to plain\n");
			encrypt = &encrypt_ecb<AES_plain>;
			decrypt = &decrypt_ecb<AES_plain>;
			encryptcbc = &encrypt_cbc<AES_plain>;
			decryptcbc = &decrypt_cbc<AES_plain>;
		}
	};

	struct AES::implcls
	{
		static const dispatch d;
		const AES_statevec roundkeys;
		const AES_statevec iroundkeys;
		implcls(const void *key, AESVersion v):
			roundkeys(expand_keys(key, v)),
			iroundkeys(iexpand_keys(roundkeys))
		{}
	};
	const dispatch AES::implcls::d;


	AES::AES(const void *key, AESVersion v):
		impl(std::make_shared<implcls>(key, v))
	{}

	void AES::encrypt(void *data, std::size_t length) const
	{
		(*implcls::d.encrypt)(data, length, impl->roundkeys);
	}
	std::size_t AES::decrypt(void *data, std::size_t length) const
	{
		return (*implcls::d.decrypt)(data, length, impl->roundkeys, impl->iroundkeys);
	}
	void AES::encrypt_cbc(void *data, std::size_t length) const
	{
		(*implcls::d.encryptcbc)(data, length, impl->roundkeys);
	}
	std::size_t AES::decrypt_cbc(void *data, std::size_t length) const
	{
		logd("calling dispatched decrypting cbc\n");
		return (*implcls::d.decryptcbc)(data, length, impl->roundkeys, impl->iroundkeys);
	}
}
