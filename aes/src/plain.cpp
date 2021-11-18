#include "aes/plain.hpp"
#include "aes/defs.hpp"
#include "aes/aesalg.hpp"
#include "aes/keys.hpp"

#include <cstring>

namespace aes
{
	namespace plain
	{
		namespace
		{
			struct colres
			{
				aes::word data[256];
				colres(
					unsigned char col,
					word (*func)(unsigned char, unsigned char ),
					const unsigned char *box)
				{
					for (std::size_t i=0; i<256; ++i)
					{ data[i] = func(box[i], col); }
				}
				operator const aes::word*() const { return data; }
			};
			colres c0(0, mixcol, SBox());
			colres c1(1, mixcol, SBox());
			colres c2(2, mixcol, SBox());
			colres c3(3, mixcol, SBox());
			colres i0(0, imixcol, IBox());
			colres i1(1, imixcol, IBox());
			colres i2(2, imixcol, IBox());
			colres i3(3, imixcol, IBox());
		}
		oword operator^(const wword &a, const wword &b)
		{
			return {
				static_cast<unsigned char>(a[0]^b[0]),
				static_cast<unsigned char>(a[1]^b[1]),
				static_cast<unsigned char>(a[2]^b[2]),
				static_cast<unsigned char>(a[3]^b[3])
			};
		}

		void State::imix()
		{
			const unsigned char * const end = data + State_Bytes;
			for (unsigned char *s = data; s<end; s += Word_Bytes)
			{
				std::memcpy(
					s, (imixcol(s[0], 0) ^ imixcol(s[1], 1)
					^ imixcol(s[2], 2) ^ imixcol(s[3], 3)).data, Word_Bytes);
			}
		}
		State& State::operator^=(const unsigned char *d)
		{
			for (std::size_t i=0; i<State_Bytes; ++i)
			{ data[i] ^= d[i]; }
			return *this;
		}
		void State::init(unsigned char *dst, const unsigned char *src)
		{
			std::memcpy(dst, src, State_Bytes);
			data = dst;
		}


		void State::aes_enc(const unsigned char *key)
		{
			auto w1 = c0[data[0]] ^ c1[data[5]] ^ c2[data[10]] ^ c3[data[15]];
			auto w2 = c0[data[4]] ^ c1[data[9]] ^ c2[data[14]] ^ c3[data[3]];
			auto w3 = c0[data[8]] ^ c1[data[13]] ^ c2[data[2]] ^ c3[data[7]];
			auto w4 = c0[data[12]] ^ c1[data[1]] ^ c2[data[6]] ^ c3[data[11]];
			std::memcpy(data, w1.data, Word_Bytes);
			std::memcpy(data+4, w2.data, Word_Bytes);
			std::memcpy(data+8, w3.data, Word_Bytes);
			std::memcpy(data+12, w4.data, Word_Bytes);
			*this ^= key;
		}
		void State::aes_enclast(const unsigned char *key)
		{
			const unsigned char *b = SBox();
			data[0] = b[data[0]];
			data[4] = b[data[4]];
			data[8] = b[data[8]];
			data[12] = b[data[12]];
			unsigned char tmp = b[data[1]];
			data[1] = b[data[5]];
			data[5] = b[data[9]];
			data[9] = b[data[13]];
			data[13] = tmp;
			tmp = b[data[2]];
			data[2] = b[data[10]];
			data[10] = tmp;
			tmp = b[data[6]];
			data[6] = b[data[14]];
			data[14] = tmp;
			tmp = b[data[3]];
			data[3] = b[data[15]];
			data[15] = b[data[11]];
			data[11] = b[data[7]];
			data[7] = tmp;
			*this ^= key;
		}
		void State::aes_dec(const unsigned char *key)
		{
			auto w1 = i0[data[0]] ^ i1[data[13]] ^ i2[data[10]] ^ i3[data[7]];
			auto w2 = i0[data[4]] ^ i1[data[1]] ^ i2[data[14]] ^ i3[data[11]];
			auto w3 = i0[data[8]] ^ i1[data[5]] ^ i2[data[2]] ^ i3[data[15]];
			auto w4 = i0[data[12]] ^ i1[data[9]] ^ i2[data[6]] ^ i3[data[3]];
			std::memcpy(data, w1.data, Word_Bytes);
			std::memcpy(data+4, w2.data, Word_Bytes);
			std::memcpy(data+8, w3.data, Word_Bytes);
			std::memcpy(data+12, w4.data, Word_Bytes);
			*this ^= key;
		}
		void State::aes_declast(const unsigned char *key)
		{
			const unsigned char *b = IBox();
			data[0] = b[data[0]];
			data[4] = b[data[4]];
			data[8] = b[data[8]];
			data[12] = b[data[12]];
			unsigned char tmp = b[data[1]];
			data[1] = b[data[13]];
			data[13] = b[data[9]];
			data[9] = b[data[5]];
			data[5] = tmp;
			tmp = b[data[2]];
			data[2] = b[data[10]];
			data[10] = tmp;
			tmp = b[data[6]];
			data[6] = b[data[14]];
			data[14] = tmp;
			tmp = b[data[3]];
			data[3] = b[data[7]];
			data[7] = b[data[11]];
			data[11] = b[data[15]];
			data[15] = tmp;
			*this ^= key;
		}

		Keys::Keys(const Version &v): version(v) {}
		Keys::Keys(const void *key, const Version &v): version(v)
		{
			std::memcpy(data, key, version.Key_Bytes);
			aes::init_keys<Keys>(*this, version);
		}
		oword Keys::rotsub(std::size_t i) const
		{
			const unsigned char *w = data + i*Word_Bytes;
			const unsigned char *s = SBox();
			return {s[w[1]], s[w[2]], s[w[3]], s[w[0]]};
		}
		oword Keys::sub(std::size_t i) const
		{
			const unsigned char *w = data + i*Word_Bytes;
			const unsigned char *s = SBox();
			return {s[w[0]], s[w[1]], s[w[2]], s[w[3]]};
		}
		Keys Keys::reverse() const
		{
			Keys ret(version);
			aes::reverse_keys(ret, *this, version);
			return ret;
		}
	}
}
