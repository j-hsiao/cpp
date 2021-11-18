#include "aes/uil32.hpp"
#include "aes/raw.hpp"
#include "aes/defs.hpp"
#include "aes/aesalg.hpp"
#include "aes/keys.hpp"

#include <cstdint>
#include <climits>

namespace aes
{
	namespace uil32
	{
		namespace
		{
			void xorinto(uilword *dst, const uilword *a, const uilword *b)
			{
				for (std::size_t i=0; i<State_Words; ++i)
				{ dst[i] = a[i] ^ b[i]; }
			}

			uilword fixword(uilword w)
			{
				if (sizeof(uilword)*CHAR_BIT == 32)
				{ return w; }
				else
				{ return w & 0xffffffffu; }
			}
			unsigned char fixbyte(unsigned char b)
			{
				if (CHAR_BIT == 8) { return b; }
				else { return b & 0xffu; }
			}
			uilword load_uil(const unsigned char *data)
			{
				uilword v = 
					static_cast<uilword>(fixbyte(data[0])) << 24
					| static_cast<uilword>(fixbyte(data[1])) << 16
					| static_cast<uilword>(fixbyte(data[2])) << 8
					| static_cast<uilword>(fixbyte(data[3]));
				return fixword(v);
			}
			unsigned char nthbyte(uilword v, std::size_t n)
			{ return fixbyte(static_cast<unsigned char>(v >> (Word_Bytes-(n+1))*Byte_Bits)); }
			void store_uil(unsigned char *data, uilword v)
			{
				for(std::size_t i=0; i<Word_Bytes; ++i)
				{ data[i] = nthbyte(v, i); }
			}

			const std::uint_least32_t * const * col = submixuil();
			const std::uint_least32_t * const * icol = isubmixuil();

			//ulrot[m][n] = source for the mth byte of the nth post-rotation word
			//	and also the source for the nth byte of the mth post-rotation word
			static const std::size_t ulrot[][4] = {
				{0, 1, 2, 3}, {1, 2, 3, 0}, {2, 3, 0, 1}, {3, 0, 1, 2}};

#		if AES_UIL32_BYTEFIRST
			//ulirot[m][n] = source for the mth byte of the nth post-rotation word
			//(byte first), may use less cache because use the 256xsizeof(uilword)
			//lut one at a time
			static const std::size_t ulirot[][4] = {
				{0, 1, 2, 3}, {3, 0, 1, 2}, {2, 3, 0, 1}, {1, 2, 3, 0}};
#		endif
			//ulicrot[m][n] = source for the nth byte of the mth post-rotation word
			//(word first)
			//uses the 4 256xsizeof(uilword) luts all at once
			static const std::size_t ulicrot[][4] = {
				{0, 3, 2, 1}, {1, 0, 3, 2}, {2, 1, 0, 3}, {3, 2, 1, 0}};
		}

		State& State::operator^=(const uilword *d)
		{
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] ^= d[i]; }
			return *this;
		}
		State& State::operator=(const uilword *d)
		{
			std::memcpy(data, d, sizeof(uilword)*State_Words);
			return *this;
		}
		void State::imix()
		{
			const unsigned char *b = SBox();
			for (int i=0; i<State_Words; ++i)
			{
				auto word = data[i];
				data[i] = icol[0][b[nthbyte(word, 0)]]
					^ icol[1][b[nthbyte(word, 1)]]
					^ icol[2][b[nthbyte(word, 2)]]
					^ icol[3][b[nthbyte(word, 3)]];
			}
		}

		void OState::init(unsigned char *dst, const unsigned char *src)
		{
			for (std::size_t i=0; i<State_Words; ++i, src += Word_Bytes)
			{ data[i] = load_uil(src); }
		}
		void OState::operator>>(unsigned char *dst)
		{
			for (std::size_t i=0; i<State_Words; ++i, dst += Word_Bytes)
			{ store_uil(dst, data[i]); }
		}
		OState& OState::operator^=(const uilword *o)
		{
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] ^= o[i]; }
			return *this;
		}

		//loop on col lut has better cache behavior so faster?
		//even if it results in nested loop
		void OState::aes_enc(const uilword *key)
		{
			OState tmp;
#		if AES_UIL32_BYTEFIRST
			for (std::size_t w=0; w<State_Words; ++w)
			{ tmp.data[w] = col[0][nthbyte(data[w], 0)]; }
			for (std::size_t nth=1; nth<Word_Bytes; ++nth)
			{
				for (std::size_t i=0; i<State_Words; ++i)
				{ tmp.data[i] ^= col[nth][nthbyte(data[ulrot[nth][i]], nth)]; }
			}
#		else
			for (std::size_t w=0; w<State_Words; ++w)
			{
				tmp.data[w] = col[0][nthbyte(data[ulrot[w][0]], 0)]
					^ col[1][nthbyte(data[ulrot[w][1]], 1)]
					^ col[2][nthbyte(data[ulrot[w][2]], 2)]
					^ col[3][nthbyte(data[ulrot[w][3]], 3)];
			}
#		endif
			xorinto(data, tmp.data, key);
			//*this = tmp ^ key;
		}
		void OState::aes_enclast(const uilword *key)
		{
			const unsigned char *b = SBox();
			OState tmp;
			for (std::size_t i=0; i<State_Words; ++i)
			{
				unsigned char w[Word_Bytes];
				for (std::size_t j=0; j<Word_Bytes; ++j)
				{ w[j] = b[nthbyte(data[ulrot[i][j]], j)]; }
				tmp.data[i] = load_uil(w);
			}
			xorinto(data, tmp.data, key);
			//*this = tmp ^ key;
		}
		void OState::aes_dec(const uilword *key)
		{
			OState tmp;
#		if AES_UIL32_BYTEFIRST
			for (std::size_t w=0; w<State_Words; ++w)
			{ tmp.data[w] = icol[0][nthbyte(data[w], 0)]; }
			for (std::size_t nth=1; nth<Word_Bytes; ++nth)
			{
				for (std::size_t i=0; i<State_Words; ++i)
				{ tmp.data[i] ^= icol[nth][nthbyte(data[ulirot[nth][i]], nth)]; }
			}
#		else
			for (std::size_t w=0; w<State_Words; ++w)
			{
				tmp.data[w] = icol[0][nthbyte(data[ulicrot[w][0]], 0)]
					^ icol[1][nthbyte(data[ulicrot[w][1]], 1)]
					^ icol[2][nthbyte(data[ulicrot[w][2]], 2)]
					^ icol[3][nthbyte(data[ulicrot[w][3]], 3)];
			}
#		endif
			//wtf? using *this = tmp ^ key results in twice as slow
			//for cbc, but ecb is unaffected (tested on 21x).
			//However, using tmp^key vs forloop doesn't affect encryption ecb nor cbc
			//*this = tmp ^ key;
			xorinto(data, tmp.data, key);
		}
		void OState::aes_declast(const uilword *key)
		{
			const unsigned char *b = IBox();
			OState tmp;
			for (std::size_t i=0; i<State_Words; ++i)
			{
				unsigned char w[Word_Bytes];
				for (std::size_t j=0; j<Word_Bytes; ++j)
				{ w[j] = b[nthbyte(data[ulicrot[i][j]], j)]; }
				tmp.data[i] = load_uil(w);
			}
			xorinto(data, tmp.data, key);
			//*this = tmp ^ key;
		}

		OState operator^(const State &a, const State &b)
		{
			return {
				a.data[0] ^ b.data[0],
				a.data[1] ^ b.data[1],
				a.data[2] ^ b.data[2],
				a.data[3] ^ b.data[3]
			};
		}


		Keys::Keys(const Version &v): version(v) {}
		Keys::Keys(const void *k, const Version &v):
			version(v)
		{
			auto key = reinterpret_cast<const unsigned char*>(k);
			for (std::size_t i=0; i<v.Key_Bytes/Word_Bytes; ++i)
			{ data[i] = load_uil(key + i*Word_Bytes); }
			aes::init_keys(*this, version);
		}
		uilword Keys::rcon(std::size_t i)
		{
			unsigned char d[Word_Bytes] = {aes::rcon(i), 0, 0, 0};
			return load_uil(d);
		}
		uilword Keys::rotsub(std::size_t n)
		{
			const unsigned char *b = SBox();
			unsigned char d[4] = {
				b[nthbyte(data[n], 1)],
				b[nthbyte(data[n], 2)],
				b[nthbyte(data[n], 3)],
				b[nthbyte(data[n], 0)]};
			return load_uil(d);
		}
		uilword Keys::sub(std::size_t n)
		{
			const unsigned char *b = SBox();
			unsigned char d[4] = {
				b[nthbyte(data[n], 0)],
				b[nthbyte(data[n], 1)],
				b[nthbyte(data[n], 2)],
				b[nthbyte(data[n], 3)]};
			return load_uil(d);
		}
		void Keys::store(void *dst)
		{
			auto *d = reinterpret_cast<unsigned char*>(dst);
			for (std::size_t i=0; i<version.Round_Keys*State_Words; ++i, d+=Word_Bytes)
			{ store_uil(d, data[i]); }
		}

		Keys Keys::reverse() const
		{
			Keys ret(version);
			aes::reverse_keys(ret, *this, version);
			return ret;
		}
	}

	Impl_UIL32::Impl_UIL32(const void *key, const Version &v):
		ekeys(key, v),
		dkeys(ekeys.reverse())
	{}
	void Impl_UIL32::encrypt(State &s) const
	{ aes::encrypt_state(s, ekeys, ekeys.version); }
	void Impl_UIL32::decrypt(State &s) const
	{ aes::decrypt_state(s, dkeys, dkeys.version); }
	std::size_t Impl_UIL32::encrypt_ecb(void *dst, const void *src, std::size_t sz) const
	{ return aes::encrypt_ecb(dst, src, sz, *this); }
	std::size_t Impl_UIL32::decrypt_ecb(void *dst, const void *src, std::size_t sz) const
	{ return aes::decrypt_ecb(dst, src, sz, *this); }
	std::size_t Impl_UIL32::encrypt_cbc(void *dst, const void *src, std::size_t sz) const
	{ return aes::encrypt_cbc(dst, src, sz, *this); }
	std::size_t Impl_UIL32::decrypt_cbc(void *dst, const void *src, std::size_t sz) const
	{ return aes::decrypt_cbc(dst, src, sz, *this); }
}
