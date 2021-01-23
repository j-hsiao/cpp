//https://en.wikipedia.org/wiki/SHA-2
#include <sha256/sha256.h>

#include <algorithm>
#include <vector>

namespace
{
	static const size_t ChunkBits = 512;
	static const size_t ChunkBytes = ChunkBits / sha256__Byte_Bits;
	static const size_t ChunkWords = ChunkBits / sha256__Word_Bits;
	static const size_t HashWords = sha256__Hash_Bits / sha256__Word_Bits;
	static const size_t NumRoundConsts = 64;
	static const size_t SizeBits = 64;
	static const size_t SizeBytes = SizeBits / sha256__Byte_Bits;

	static const sha256__Word RoundConstants[NumRoundConsts] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};
	inline void fix_word(sha256__Word &w)
	{
#ifndef UINT32_MAX
		w &= SHA256_Word_Mask;
#endif
	}

	void process_chunk(sha256__Word *hash, sha256__Word *chunk)
	{
		sha256__Word h[sha256__Hash_Bytes];
		sha256__Word w[NumRoundConsts];
		std::copy(chunk, chunk + ChunkWords, w);
		std::copy(hash, hash + HashWords, h);
		for (size_t i = 16; i < NumRoundConsts; ++i)
		{
			auto w15 = w[i - 15];
			auto w2 = w[i - 2];
			sha256__Word s0 = sha256__rrot(w15, 7) ^ sha256__rrot(w15, 18) ^ (w15 >> 3);
			sha256__Word s1 = sha256__rrot(w2, 17) ^ sha256__rrot(w2, 19) ^ (w2 >> 10);
			w[i] = w[i - 16] + s0 + w[i - 7] + s1;
			fix_word(w[i]);
		}
		for (size_t i = 0; i < NumRoundConsts; ++i)
		{
			sha256__Word S1 = sha256__rrot(h[4], 6)
				^ sha256__rrot(h[4], 11) ^ sha256__rrot(h[4], 25);
			sha256__Word ch = (h[4] & h[5]) ^ ((~h[4]) & h[6]);
			fix_word(ch);
			sha256__Word temp1 = h[7] + S1 + ch + RoundConstants[i] + w[i];
			fix_word(temp1);
			sha256__Word S0 = sha256__rrot(h[0], 2)
				^ sha256__rrot(h[0], 13) ^ sha256__rrot(h[0], 22);
			sha256__Word maj = (h[0] & h[1]) ^ (h[0] & h[2]) ^ (h[1] & h[2]);
			sha256__Word temp2 = S0 + maj;
			fix_word(temp2);
			h[7] = h[6];
			h[6] = h[5];
			h[5] = h[4];
			h[4] = h[3] + temp1;
			fix_word(h[4]);
			h[3] = h[2];
			h[2] = h[1];
			h[1] = h[0];
			h[0] = temp1 + temp2;
			fix_word(h[0]);
		}
		for (size_t i = 0; i < HashWords; ++i)
		{
			hash[i] += h[i];
			fix_word(hash[i]);
		}
	}

	void store_size(sha256__Byte *dst, uint_least64_t size)
	{
		size_t shift = SizeBits;
		for (size_t i = 0; i < SizeBytes; ++i)
		{
			shift -= sha256__Byte_Bits;
			dst[i] = static_cast<sha256__Byte>(size >> shift);
		}
	}
}

void sha256__hash(
	sha256__Byte dst[sha256__Hash_Bytes], const sha256__Byte *src, size_t numBytes)
{
	sha256__Word hash[HashWords] = {
		0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
		0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
	};
	sha256__Word chunk[ChunkWords];

	size_t remain = numBytes % ChunkBytes;
	const sha256__Byte * const end = src + numBytes - remain;
	for (; src < end; src += ChunkBytes)
	{
		sha256__load_words(chunk, src, ChunkBytes);
		process_chunk(hash, chunk);
	}

	uint_least64_t numbits = static_cast<uint_least64_t>(numBytes) * sha256__Byte_Bits;
	sha256__Byte buf[ChunkBytes];
	if (remain + 1 + SizeBytes > ChunkBytes)
	{
		std::copy(src, src + remain, buf);
		buf[remain] = 0x80;
		std::fill(buf + remain + 1, buf + ChunkBytes, 0);
		sha256__load_words(chunk, buf, ChunkBytes);
		process_chunk(hash, chunk);
		std::fill(buf, buf + ChunkBytes - SizeBytes, 0);
		store_size(buf + ChunkBytes - SizeBytes, numbits);
	}
	else
	{
		std::copy(src, src + remain, buf);
		buf[remain] = 0x80;
		std::fill(buf + remain + 1, buf + ChunkBytes - SizeBytes, 0);
		store_size(buf + ChunkBytes - SizeBytes, numbits);
	}
	sha256__load_words(chunk, buf, ChunkBytes);
	process_chunk(hash, chunk);
	sha256__store_words(dst, hash, sha256__Hash_Bytes);
}
