#ifndef AES_IMPL_UILEAST32_HPP
#define AES_IMPL_UILEAST32_HPP
#include "aes/aes_dllcfg.h"
#include "aes/defs.hpp"
#include "aes/aesalg.hpp"

#include <cstdint>
#include <climits>



namespace aes
{
	namespace uil32
	{
		typedef std::uint_least32_t uilword;

		struct OState
		{
			uilword data[State_Words];
			void init(unsigned char *dst, const unsigned char *src);
			void operator>>(unsigned char *dst);
			OState& operator^=(const uilword *o);
			OState& operator^=(const OState &o) { return *this ^= o.data; }
			void aes_enc(const uilword *d);
			void aes_enclast(const uilword *d);
			void aes_dec(const uilword *d);
			void aes_declast(const uilword *d);
		};
		inline OState operator^(OState a, const uilword *o)
		{ a ^= o; return a; }
		inline OState operator^(OState a, const OState &o)
		{ a ^= o.data; return a; }

		struct State
		{
			uilword *data;
			State(uilword *d): data(d) {}
			State(OState &&o): data(o.data) {}
			State& operator^=(const uilword *d);
			State& operator=(const uilword *d);
			void imix();
		};
		OState operator^(const State &a, const State &b);

		struct Keys
		{
			uilword data[Max_Rounds * State_Words];
			const Version &version;
			Keys(const Version &v);
			Keys(const void *k, const Version &v);

			uilword rcon(std::size_t i);
			uilword& word(std::size_t n) { return data[n]; }
			uilword rotsub(std::size_t n);
			uilword sub(std::size_t n);

			State state(std::size_t n) { return data + n*State_Words; }
			const uilword* state(std::size_t n) const
			{ return data + n*State_Words; }
			void store(void *data);

			Keys reverse() const;
		};
	}
	typedef Impl<uil32::Keys, uil32::OState> Impl_UIL32_t;

	struct Impl_UIL32
	{
		typedef uil32::OState State ;
		uil32::Keys ekeys;
		uil32::Keys dkeys;

		Impl_UIL32(const void *key, const Version &v);

		void encrypt(State &s) const;
		void decrypt(State &s) const;
		std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const;
		std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const;
		std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const;
		std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const;
	};
}
#endif
