//source: https://en.wikipedia.org/wiki/SHA-2
#include <sha256/sha256.h>
#include <sha256/sha256_dllcfg.h>
#include <serial/serial.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

#ifndef UINT32_MAX
#define fix_word(thing) ((thing) & 0xfffffffful)
#else
#define fix_word(thing) (thing)
#endif
namespace
{
	static const size_t Num_Keys = 64;
	static std::uint_least32_t keys[] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
	
	std::uint_least32_t rrot(std::uint_least32_t v, std::size_t bits)
	{ return fix_word((v >> bits) | (v << (sha256_Word_Bits-bits))); }

	inline void hashchunk(
		std::uint_least32_t hash[sha256_Hash_Words], const unsigned char *chunk)
	{
		//create 64-entry message schedule
		std::uint_least32_t w[Num_Keys];
		//copy chunk into first 16 words
		for (std::size_t i=0; i<16; ++i)
		{ w[i] = serial_load_ui32(chunk + i*sha256_Word_Bytes); }

		//extend 16 words into remaining 48
		for (std::size_t i=16; i<Num_Keys; ++i)
		{
			auto wi15 = w[i-15];
			auto wi2 = w[i-2];
			auto s0 = rrot(wi15, 7) ^ rrot(wi15, 18) ^ wi15>>3;
			auto s1 = rrot(wi2, 17) ^ rrot(wi2, 19) ^ wi2>>10;
			w[i] = fix_word(w[i-16] + s0 + w[i-7] + s1);
		}

		//initialize working variables
		auto a = hash[0];
		auto b = hash[1];
		auto c = hash[2];
		auto d = hash[3];
		auto e = hash[4];
		auto f = hash[5];
		auto g = hash[6];
		auto h = hash[7];

		//compress
		for (size_t i=0; i<Num_Keys; ++i)
		{
			auto s1 = rrot(e, 6) ^ rrot(e, 11) ^ rrot(e, 25);
			auto ch = e&f ^ ~e&g;
			auto temp1 = h + s1 + ch + keys[i] + w[i];
			auto s0 = rrot(a, 2) ^ rrot(a, 13) ^ rrot(a, 22);
			auto maj = a&b ^ a&c ^ b&c;
			auto temp2 = fix_word(s0 + maj + temp1);
			h = g;
			g = f;
			f = e;
			e = fix_word(d+temp1);
			d = c;
			c = b;
			b = a;
			a = temp2;
		}
		//add compressed chunk to current hash values;
		hash[0] += a;
		hash[1] += b;
		hash[2] += c;
		hash[3] += d;
		hash[4] += e;
		hash[5] += f;
		hash[6] += g;
		hash[7] += h;
		for (int i=0; i<sha256_Hash_Words; ++i)
		{ hash[i] = fix_word(hash[i]); }
	}
}

CPP_EXTERNC_BEGIN
void sha256_hash(void *dst_, const void *src_, size_t numBytes)
{
	auto dst = reinterpret_cast<unsigned char*>(dst_);
	auto src = reinterpret_cast<const unsigned char*>(src_);

	std::uint_least32_t h[sha256_Hash_Words] = {
		0x6a09e667ul, 0xbb67ae85ul, 0x3c6ef372ul, 0xa54ff53aul,
		0x510e527ful, 0x9b05688cul, 0x1f83d9abul, 0x5be0cd19ul};
	const size_t Chunk_Bits = 512;
	const size_t Chunk_Bytes = Chunk_Bits / sha256_Byte_Bits;
	size_t remain = numBytes % Chunk_Bytes;

	std::size_t remainder = numBytes % Chunk_Bytes;
	const auto* const end = src + numBytes - remainder;
	for (; src<end; src += Chunk_Bytes)
	{ hashchunk(h, src); }
	unsigned char chunks[Chunk_Bytes*2];
	std::memcpy(chunks, src, remainder);
	chunks[remainder] = 0x80u;
	unsigned char *chunkend;
	if (remainder + 1 + 8 > Chunk_Bytes)
	{ chunkend = chunks + 2*Chunk_Bytes; }
	else
	{ chunkend = chunks + Chunk_Bytes; }
	std::fill(chunks+remainder+1, chunkend, 0u);
	serial_store_ui64(chunkend - 8, numBytes*sha256_Byte_Bits);
	for(src = chunks; src < chunkend; src += Chunk_Bytes)
	{ hashchunk(h, src); }
	for (std::size_t i=0; i<sha256_Hash_Words; ++i)
	{ serial_store_ui32(dst + i*sha256_Word_Bytes, h[i]); }
}
CPP_EXTERNC_END
