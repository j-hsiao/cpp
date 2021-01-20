#include <aes/aes_key.hpp>
#include <aes/aes_consts.hpp>
#include <aes/aes_types.hpp>
#include <aes/aes.hpp>
#include <aes/aes_util.hpp>

#include <climits>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace aes
{
#if CHAR_BIT <= 32
// 512 bits = 64 bytes = 16 words
//
//orig (L bits)
//add 1 bit
//add k * '0' bits 
//s.t. (L + k + 1  64) % 512 == 0
//
//for each 512-bit chunk:
//make a 64-word buffer
//copy 512 into first 16 words
//for (i = 16; i <= 64; ++i)
//{
//	s0 = (rrot(w[i - 15], 7) ^ rrot(w[i - 15], 18) ^ (w[i - 15] >> 3);
//	s1 = (rrot(w[i - 2], 17) ^ rrot(w[i - 2], 19) ^ (w[i - 2] >> 10);
//	w[i] = w[i - 16] + s0 + w[i - 7] + s1
//}
//for (i = 0; i < 64; ++i)
//{
//	
//}
//
//
//SHA256("")
//0x e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
	namespace sha256
	{
		static const std::size_t HASHBITS = 256;
		static const std::size_t HASHBYTES = HASHBITS / CHAR_BIT;
		static const std::size_t WORDBITS = 32;
		static const std::size_t WORDBYTES = WORDBITS / CHAR_BIT;
		static const std::size_t CHUNKBITS = 512;
		static const std::size_t CHUNKBYTES = CHUNKBITS / CHAR_BIT;
		static const std::size_t CHUNKWORDS = CHUNKBITS / WORDBITS;
		static const std::size_t HASHWORDS = HASHBITS / WORDBITS;
		static const std::size_t CHUNKLENBITS = 64;
		static const std::size_t CHUNKLENBYTES = CHUNKLENBITS / CHAR_BIT;
		static const std::size_t CHUNKBUFF = 64;

		static const std::uint_fast32_t WORDMASK = 0xFFFFFFFFu;
		static const std::uint_fast32_t BYTEMASK = UCHAR_MAX;

		static const std::uint_fast32_t ROUNDKEYS[64] = {
			0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
			0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
			0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
			0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
			0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
			0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
			0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
			0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
		};

		void loadwords(uint_fast32_t *dst, const void *src, std::size_t nwords)
		{
			auto *b = reinterpret_cast<const unsigned char*>(src);
			for (std::size_t i = 0; i < nwords; ++i)
			{
				uint_fast32_t &d = dst[i];
				d = 0;
				for (std::size_t bi = 0; bi < WORDBYTES; ++bi)
				{
					std::size_t shift = CHAR_BIT * (WORDBYTES - (bi + 1));
					d |= static_cast<uint_fast32_t>(*b) << shift;
					++b;
				}
			}
		}

		void storewords(void *dst, const uint_fast32_t *src, std::size_t nwords)
		{
			auto *b = reinterpret_cast<unsigned char*>(dst);
			for (std::size_t i = 0; i < nwords; ++i)
			{
				uint_fast32_t s = src[i];
				for (std::size_t bi = 0; bi < WORDBYTES; ++bi)
				{
					std::size_t shift = CHAR_BIT * (WORDBYTES - (bi + 1));
					*b = static_cast<unsigned char>(((s >> shift) & BYTEMASK));
					++b;
				}
			}
		}

		void storelen(void *dst, uint_fast64_t src)
		{
			auto *b = reinterpret_cast<unsigned char*>(dst);
			for (int i = 0; i < CHUNKLENBYTES; ++i)
			{
				std::size_t shift = (CHUNKLENBYTES - (i + 1)) * CHAR_BIT;
				b[i] = static_cast<ubyte>((src >> shift) & BYTEMASK);
			}
		}

		uint_fast32_t rrot(uint_fast32_t val, std::size_t amt)
		{
			return val >> amt | val << (WORDBITS - amt);
		}

		void process_chunk(uint_fast32_t chunk[CHUNKWORDS], uint_fast32_t hash[HASHWORDS])
		{
			uint_fast32_t w[CHUNKBUFF];
			std::memcpy(w, chunk, CHUNKWORDS * sizeof(uint_fast32_t));
			for (int i = CHUNKWORDS; i < CHUNKBYTES; ++i)
			{
				uint_fast32_t w15 = w[i - 15];
				uint_fast32_t w2 = w[i - 2];

				uint_fast32_t s0 = rrot(w15, 7) ^ rrot(w15, 18) ^ (w15 >> 3);
				uint_fast32_t s1 = rrot(w2, 17) ^ rrot(w2, 19) ^ (w2 >> 10);
				w[i] = (w[i - 16] + s0 + w[i - 7] + s1) & WORDMASK;
			}
			for (int i = 0; i < CHUNKBYTES; ++i)
			{
				
			}
		}

		//https://en.wikipedia.org/wiki/SHA-2
		std::string sha256(const void *data, std::size_t size)
		{
			auto raw = reinterpret_cast<const unsigned char*>(data);
			std::uint_fast32_t hash[8] = {
				0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
				0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
			std::size_t targetsize = size + CHUNKLENBYTES + 1;
			unsigned char rawbuf[CHUNKBYTES];
			uint_fast32_t chunkbuf[CHUNKWORDS];
			while (size > CHUNKBYTES)
			{
				loadwords(chunkbuf, raw, CHUNKWORDS);
				//process etc
				raw += CHUNKBYTES;
			}
		}
	}
#endif

	std::vector<ubyte> roundconsts(AESVERSION v)
	{
		std::vector<ubyte> b(NUM_CONSTS[v]);
		b[0] = 0x01;
		for (std::size_t i = 1; i < b.size(); ++i)
		{
			b[i] = aesmul2(b[i - 1]);
		}
		return b;
	}

	namespace
	{
#if AES_ENDIAN
		const std::size_t WORDBITS = WORDBYTES * BYTEBITS;
		AESWord rotate_left_byte(AESWord w)
		{
			return lshift(w, BYTEBITS) | rshift(w, WORDBITS - BYTEBITS);
		}

#else
		AESWord rotate_left_byte(const AESWord &w)
		{
			AESWord ret;
			ret.bytes[0] = w.bytes[1];
			ret.bytes[1] = w.bytes[2];
			ret.bytes[2] = w.bytes[3];
			ret.bytes[3] = w.bytes[0];
			return ret;
		}
#endif
		AESWord sub_w(const AESWord &w)
		{
			AESWord ret;
			const ubyte *src = reinterpret_cast<const ubyte*>(&w);
			ubyte *dst = reinterpret_cast<ubyte*>(&ret);
			dst[0] = SBOX[src[0]];
			dst[1] = SBOX[src[1]];
			dst[2] = SBOX[src[2]];
			dst[3] = SBOX[src[3]];
			return ret;
		}
	}

	AES_statevec expand_keys(const void *key, AESVERSION v)
	{
		std::vector<ubyte> rc_i = roundconsts(v);
		const std::size_t N = NUM_KWORDS[v];
		const std::size_t R = NUM_ROUNDS[v] + 1;
		AES_statevec roundkeys(R);
		AESWord *words = reinterpret_cast<AESWord*>(&roundkeys[0]);
		std::memcpy(words, key, N * WORDBYTES);


		
		const std::size_t end = R * STATEWORDS;
		for (std::size_t i = N; i < end; ++i)
		{
			AESWord *w = words + i;
			if (i % N == 0)
			{
				std::size_t remain = end - i;
				std::memcpy(w, w - N, (N < remain ? N : remain) * WORDBYTES);
				*w ^= sub_w(rotate_left_byte(w[-1]));
				*reinterpret_cast<ubyte*>(w) ^= rc_i[(i / N) - 1];
			}
			else if (N > 6 && (i % N == 4))
			{
				*w ^= sub_w(w[-1]);
			}
			else
			{
				*w ^= w[-1];
			}
		}
		return roundkeys;
	}

	AES_statevec iexpand_keys(const AES_statevec &roundkeys)
	{
		AES_statevec iroundkeys(roundkeys.size());
		std::size_t rk = roundkeys.size() - 1;
		std::size_t end = rk;
		iroundkeys[0] = roundkeys[rk];
		rk -= 1;
		for (std::size_t s = 1; s < end; ++s, --rk)
		{
			auto *src = reinterpret_cast<const ubyte*>(&roundkeys[rk]);
			AESState &state = iroundkeys[s];
			for (std::size_t w = 0; w < STATEWORDS; ++w)
			{
				std::size_t byteoff = w * WORDBYTES;
				state.words[w] = IMIX[0][SBOX[src[byteoff]]];
				for (std::size_t b = 1; b < WORDBYTES; ++b)
				{
					state.words[w] ^= IMIX[b][SBOX[src[byteoff + b]]];
				}
			}
		}
		iroundkeys[end] = roundkeys[0];
		return iroundkeys;
	}

	void make_key(
		void *dst, const void *data, std::size_t passbytes,
		AESVERSION version, int passes)
	{
		std::size_t keybytes = NUM_KBYTES[version];
		ubyte *key = reinterpret_cast<ubyte*>(dst);
		const ubyte *pass;
		if (passbytes)
		{
			pass = reinterpret_cast<const ubyte*>(data);
		}
		else
		{
			passbytes = 1;
			pass = reinterpret_cast<const ubyte*>("\0");
		}
		uint_fast32_t state = 0;
		std::size_t i = 0;
		std::size_t end = passbytes > keybytes ? passbytes : keybytes;
		for (; i < end; ++i)
		{
			std::size_t pshift = (i / passbytes) % BYTEBITS;
			ubyte p = pass[i % passbytes];
			p = p >> pshift ^ p << (BYTEBITS - pshift);
			state ^= p ^ 0x1b;
			key[i % keybytes] ^= static_cast<ubyte>(state & 0xffu);
			state ^= (state << 7) ^ (state >> 3);
			state &= 0xffffffffu;
		}
		if (passes > 0)
		{
			make_key(dst, dst, keybytes, version, passes - 1);
		}
	}
}


//#if AES_ENDIAN
//	namespace {
//		uint32_t rotate_left_w(uint32_t w, std::size_t amt)
//		{
//			return (lshift(w, amt)
//				| rshift(w, (WORDBYTES * BYTEBITS) - amt));
//		}
//
//		uint32_t sub_w(uint32_t w)
//		{
//			uint32_t ret;
//			ubyte *src = reinterpret_cast<ubyte*>(&w);
//			ubyte *dst = reinterpret_cast<ubyte*>(&ret);
//			dst[0] = SBOX[src[0]];
//			dst[1] = SBOX[src[1]];
//			dst[2] = SBOX[src[2]];
//			dst[3] = SBOX[src[3]];
//			return ret;
//		}
//	}
//
//	StateVec expand_keys(const void *key, AESVERSION v)
//	{
//		logd("multi-byte plain expand keys");
//		std::vector<ubyte> rc_i = roundconsts(v);
//		const std::size_t N = NUM_KWORDS[v];
//		const std::size_t R = NUM_ROUNDS[v] + 1;
//		StateVec roundkeys(R);
//		AESWord *words = reinterpret_cast<AESWord*>(&roundkeys[0]);
//		std::memcpy(words, key, N * WORDBYTES);
//		const std::size_t end = R * STATEWORDS;
//		for (std::size_t i = N; i < end; ++i)
//		{
//			uint32_t pw[2];
//			if (i % N == 0)
//			{
//				std::size_t remain = end - i;
//				std::memcpy(w, w - N, (N < remain ? N : remain) * WORDBYTES);
//				*w ^= sub_w(rotate_left_w(w[-1], BYTEBITS));
//				*reinterpret_cast<ubyte*>(w) ^= rc_i[(i / N) - 1];
//			}
//			else if (N > 6 && (i % N == 4))
//			{
//				*w ^= sub_w(w[-1]);
//			}
//			else
//			{
//				*w ^= w[-1];
//			}
//		}
//		return roundkeys;
//	}
//
//	StateVec iexpand_keys(const StateVec &roundkeysv)
//	{
//		StateVec iroundkeys(roundkeysv.size());
//
//		auto *rk = reinterpret_cast<const uint32_t*>(
//			&roundkeysv[roundkeysv.size() - 1]);
//		auto *end = reinterpret_cast<const uint32_t*>(&roundkeysv[0]);
//		uint32_t *state = reinterpret_cast<uint32_t*>(&iroundkeys[0]);
//
//		std::memcpy(state, rk, STATEBYTES);
//		rk -= STATEWORDS;
//		state += STATEWORDS;
//		for(; rk > end; rk -= STATEWORDS, state += STATEWORDS)
//		{
//			auto *src = reinterpret_cast<const ubyte*>(rk);
//			state[0] = IMIX1[SBOX[src[0]]] ^ IMIX2[SBOX[src[1]]] ^ IMIX3[SBOX[src[2]]] ^ IMIX4[SBOX[src[3]]];
//			state[1] = IMIX1[SBOX[src[4]]] ^ IMIX2[SBOX[src[5]]] ^ IMIX3[SBOX[src[6]]] ^ IMIX4[SBOX[src[7]]];
//			state[2] = IMIX1[SBOX[src[8]]] ^ IMIX2[SBOX[src[9]]] ^ IMIX3[SBOX[src[10]]] ^ IMIX4[SBOX[src[11]]];
//			state[3] = IMIX1[SBOX[src[12]]] ^ IMIX2[SBOX[src[13]]] ^ IMIX3[SBOX[src[14]]] ^ IMIX4[SBOX[src[15]]];
//		}
//		std::memcpy(state, rk, STATEBYTES);
//		return iroundkeys;
//	}
//
//#else
//	StateVec expand_keys(const void *key, AESVERSION VERSION)
//	{
//		logd("single-byte plain expand keys");
//		std::vector<ubyte> rc_i = roundconsts(VERSION);
//		const std::size_t N = NUM_KWORDS[VERSION];
//		const std::size_t R = NUM_ROUNDS[VERSION] + 1;
//		StateVec roundkeys(R);
//		AESWord *words = reinterpret_cast<AESWord*>(&roundkeys[0]);
//		std::memcpy(words, key, N * WORDBYTES);
//		const std::size_t end = R * STATEWORDS;
//		for (std::size_t i = N; i < end; ++i)
//		{
//			AESWord *word = words + i;
//			AESWord *prev = words - 1;
//			if (i % N == 0)
//			{
//				std::size_t remain = end - i;
//				std::memcpy(word, word - N, (N < remain ? N : remain) * WORDBYTES);
//				word->bytes[0] ^= SBOX[prev->bytes[1]] ^ rc_i[(i / N) - 1];
//				word->bytes[1] ^= SBOX[prev->bytes[2]];
//				word->bytes[2] ^= SBOX[prev->bytes[3]];
//				word->bytes[3] ^= SBOX[prev->bytes[0]];
//			}
//			else if (N > 6 && (i % N == 4))
//			{
//				word->bytes[0] ^= SBOX[prev->bytes[0]];
//				word->bytes[1] ^= SBOX[prev->bytes[1]];
//				word->bytes[2] ^= SBOX[prev->bytes[2]];
//				word->bytes[3] ^= SBOX[prev->bytes[3]];
//			}
//			else
//			{
//				word->bytes[0] ^= prev->bytes[0];
//				word->bytes[1] ^= prev->bytes[1];
//				word->bytes[2] ^= prev->bytes[2];
//				word->bytes[3] ^= prev->bytes[3];
//			}
//		}
//		return roundkeys;
//	}
//	std::vector<ubyte> iexpand_keys(const std::vector<ubyte> &roundkeysv)
//	{
//		std::vector<ubyte> iroundkeys(roundkeysv.size());
//		auto *imix1 = reinterpret_cast<const ubyte*>(&IMIX1[0]);
//		auto *imix2 = reinterpret_cast<const ubyte*>(&IMIX2[0]);
//		auto *imix3 = reinterpret_cast<const ubyte*>(&IMIX3[0]);
//		auto *imix4 = reinterpret_cast<const ubyte*>(&IMIX4[0]);
//
//		auto *rk = reinterpret_cast<const ubyte*>(
//			&roundkeysv[roundkeysv.size() - STATEBYTES]);
//		auto *end = reinterpret_cast<const ubyte*>(&roundkeysv[0]);
//		auto *state = reinterpret_cast<ubyte*>(&iroundkeys[0]);
//		for (int i = 0; i < STATEBYTES; ++i)
//		{
//			state[i] = rk[i];
//		}
//		rk -= STATEBYTES;
//		state += STATEBYTES;
//		for (; rk > end; rk -= STATEBYTES, state += STATEBYTES)
//		{
//			for (int wordoffset = 0; wordoffset < STATEBYTES; wordoffset += WORDBYTES)
//			{
//				auto src = rk + wordoffset;
//				auto stateword = state + wordoffset;
//
//				auto b1 = static_cast<unsigned int>(SBOX[src[0]]) * WORDBYTES;
//				auto b2 = static_cast<unsigned int>(SBOX[src[1]]) * WORDBYTES;
//				auto b3 = static_cast<unsigned int>(SBOX[src[2]]) * WORDBYTES;
//				auto b4 = static_cast<unsigned int>(SBOX[src[3]]) * WORDBYTES;
//
//				stateword[0] = imix1[b1] ^ imix2[b2] ^ imix3[b3] ^ imix4[b4];
//				stateword[1] = imix1[b1 + 1] ^ imix2[b2 + 1] ^ imix3[b3 + 1] ^ imix4[b4 + 1];
//				stateword[2] = imix1[b1 + 2] ^ imix2[b2 + 2] ^ imix3[b3 + 2] ^ imix4[b4 + 2];
//				stateword[3] = imix1[b1 + 3] ^ imix2[b2 + 3] ^ imix3[b3 + 3] ^ imix4[b4 + 3];
//			}
//		}
//		for (int i = 0; i < STATEBYTES; ++i)
//		{
//			state[i] = rk[i];
//		}
//		return iroundkeys;
//	}
//#endif
//
//	std::string make_key(
//		const void *data, std::size_t passbytes , AESVERSION version, int passes)
//	{
//		std::size_t keybytes = NUM_KWORDS[version] * WORDBYTES;
//		std::string ret;
//		ret.resize(keybytes);
//		ubyte *key = reinterpret_cast<ubyte*>(&ret[0]);
//		const ubyte *pass;
//		if (passbytes)
//		{
//			pass = reinterpret_cast<const ubyte*>(data);
//		}
//		else
//		{
//			passbytes = 1;
//			pass = reinterpret_cast<const ubyte*>("\0");
//		}
//		uint_least32_t state = 0;
//		std::size_t i = 0;
//		std::size_t end = passbytes > keybytes ? passbytes : keybytes;
//		for (; i < end; ++i)
//		{
//			std::size_t pshift = (i / passbytes) % BYTEBITS;
//			ubyte p = pass[i % passbytes];
//			p = p >> pshift ^ p << (BYTEBITS - pshift);
//			state ^= p ^ 0x1b;
//			key[i % keybytes] ^= static_cast<ubyte>(state & 0xffu);
//			state ^= (state << 7) ^ (state >> 3);
//			state &= 0xffffffffu;
//		}
//		if (passes > 0)
//		{
//			return make_key(&ret[0], ret.size(), version, passes - 1);
//		}
//		else
//		{
//			return ret;
//		}
//	}
//}
