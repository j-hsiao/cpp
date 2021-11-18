#include "aes/ui32.hpp"
#if AES_USE_UI32

#include "aes/raw.hpp"

#include <iostream>
#include "aes/defs.hpp"
#include "aes/keys.hpp"

#include <cstdint>
#include <cstring>

namespace aes
{
	namespace ui32
	{
		namespace
		{
			const std::uint32_t* const * col = submixui();
			const std::uint32_t* const * icol = isubmixui();
		}

		State& State::operator^=(const std::uint32_t *s)
		{
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] ^= s[i]; }
			return *this;
		}

		void State::init(unsigned char *dst, const unsigned char *src)
		{ std::memcpy(data, src, State_Bytes); }
		void State::operator>>(unsigned char *dst) const
		{ std::memcpy(dst, data, State_Bytes); }


		void State::aes_enc(const std::uint32_t *key)
		{
			auto *bytes = reinterpret_cast<const unsigned char*>(data);
			*this = {
				col[0][bytes[0]] ^ col[1][bytes[5]] ^ col[2][bytes[10]] ^ col[3][bytes[15]],
				col[0][bytes[4]] ^ col[1][bytes[9]] ^ col[2][bytes[14]] ^ col[3][bytes[3]],
				col[0][bytes[8]] ^ col[1][bytes[13]] ^ col[2][bytes[2]] ^ col[3][bytes[7]],
				col[0][bytes[12]] ^ col[1][bytes[1]] ^ col[2][bytes[6]] ^ col[3][bytes[11]]
			};
			*this ^= key;
		}
		void State::aes_enclast(const std::uint32_t *key)
		{
			const auto *b = SBox();
			auto d = reinterpret_cast<const unsigned char*>(data);
			unsigned char s[State_Bytes];
			for (std::size_t i=0; i<State_Bytes; ++i)
			{ s[i] = b[d[rot[i]]]; }
			std::memcpy(data, s, State_Bytes);
			*this ^= key;
		}
		void State::aes_dec(const std::uint32_t *key)
		{
			auto *bytes = reinterpret_cast<const unsigned char*>(data);
			*this = {
				icol[0][bytes[0]] ^ icol[1][bytes[13]] ^ icol[2][bytes[10]] ^ icol[3][bytes[7]],
				icol[0][bytes[4]] ^ icol[1][bytes[1]] ^ icol[2][bytes[14]] ^ icol[3][bytes[11]],
				icol[0][bytes[8]] ^ icol[1][bytes[5]] ^ icol[2][bytes[2]] ^ icol[3][bytes[15]],
				icol[0][bytes[12]] ^ icol[1][bytes[9]] ^ icol[2][bytes[6]] ^ icol[3][bytes[3]]
			};
			*this ^= key;
		}
		void State::aes_declast(const std::uint32_t *key)
		{
			const auto *b = IBox();
			auto d = reinterpret_cast<const unsigned char*>(data);
			unsigned char s[State_Bytes];
			for (std::size_t i=0; i<State_Bytes; ++i)
			{ s[i] = b[d[irot[i]]]; }
			std::memcpy(data, s, State_Bytes);
			*this ^= key;
		}

		void WState::imix()
		{
			const auto *b = SBox();
			for (std::size_t i=0; i<State_Words; ++i)
			{
				auto d = reinterpret_cast<unsigned char*>(data+i);
				data[i] = icol[0][b[d[0]]] ^ icol[1][b[d[1]]]
								^ icol[2][b[d[2]]] ^ icol[3][b[d[3]]];
			}
		}

		Keys::Keys(const void *key, const Version &v): version(v)
		{
			std::memcpy(data, key, version.Key_Bytes);
			aes::init_keys(*this, version);
		}
		Keys Keys::reverse() const
		{
			Keys ret(version);
			aes::reverse_keys(ret, *this, version);
			return ret;
		}
		void Keys::store(void *dst) const
		{ std::memcpy(dst, data, State_Bytes * version.Round_Keys); }
		std::uint32_t Keys::rcon(std::size_t i)
		{
			std::uint32_t ret = 0;
			reinterpret_cast<unsigned char*>(&ret)[0] = aes::rcon(i);
			return ret;
		}
		std::uint32_t Keys::rotsub(std::size_t n) const
		{
			std::uint32_t ret;
			const auto *d = reinterpret_cast<const unsigned char*>(data + n);
			auto *r = reinterpret_cast<unsigned char*>(&ret);
			const auto *lut = SBox();
			r[0] = lut[d[1]];
			r[1] = lut[d[2]];
			r[2] = lut[d[3]];
			r[3] = lut[d[0]];
			return ret;
		}
		std::uint32_t Keys::sub(std::size_t n) const
		{
			std::uint32_t ret;
			const auto *d = reinterpret_cast<const unsigned char*>(data + n);
			auto *r = reinterpret_cast<unsigned char*>(&ret);
			const auto *lut = SBox();
			r[0] = lut[d[0]];
			r[1] = lut[d[1]];
			r[2] = lut[d[2]];
			r[3] = lut[d[3]];
			return ret;
		}
	}

	Impl_UI32::Impl_UI32(const void *key, const Version &v):
		ekeys(key, v),
		dkeys(ekeys.reverse())
	{}
	void Impl_UI32::encrypt(State &s) const
	{ aes::encrypt_state(s, ekeys, ekeys.version); }
	void Impl_UI32::decrypt(State &s) const
	{ aes::decrypt_state(s, dkeys, dkeys.version); }
	std::size_t Impl_UI32::encrypt_ecb(void *dst, const void *src, std::size_t sz) const
	{ return aes::encrypt_ecb(dst, src, sz, *this); }
	std::size_t Impl_UI32::decrypt_ecb(void *dst, const void *src, std::size_t sz) const
	{ return aes::decrypt_ecb(dst, src, sz, *this); }
	std::size_t Impl_UI32::encrypt_cbc(void *dst, const void *src, std::size_t sz) const
	{ return aes::encrypt_cbc(dst, src, sz, *this); }
	std::size_t Impl_UI32::decrypt_cbc(void *dst, const void *src, std::size_t sz) const
	{ return aes::decrypt_cbc(dst, src, sz, *this); }
}
#endif
