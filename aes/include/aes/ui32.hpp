// Implementation using uint32_t/uint_least32_t to perform operations
#ifndef AES_IMPL_IMPLS_HPP
#define AES_IMPL_IMPLS_HPP


#include <cstdint>
#include <climits>
#define AES_USE_UI32 (defined(UINT32_MAX) && CHAR_BIT==8)

#if AES_USE_UI32
#include "aes/aes_dllcfg.h"
#include "aes/aesalg.hpp"
#include "aes/defs.hpp"

#include <cstring>
namespace aes
{
	static_assert(sizeof(std::uint32_t)==4, "uint32_t wrong size");
	namespace ui32
	{
		struct State
		{
			std::uint32_t data[State_Words];
			State& operator^=(const std::uint32_t *s);
			State& operator^=(const State &o) { return *this ^= o.data; }

			void init(unsigned char *dst, const unsigned char *src);
			void operator>>(unsigned char *dst) const;

			void aes_enc(const std::uint32_t *key);
			void aes_enclast(const std::uint32_t *key);
			void aes_dec(const std::uint32_t *key);
			void aes_declast(const std::uint32_t *key);
		};
		struct WState
		{
			std::uint32_t *data;
			WState(State &&o): data(o.data) {}
			WState(std::uint32_t *o): data(o) {}
			WState& operator=(const std::uint32_t *o)
			{ std::memcpy(data, o, State_Bytes); return *this;}
			void imix();
		};

		struct Keys
		{
			std::uint32_t data[State_Words * Max_Rounds];
			const Version &version;

			Keys(const Version &v): version(v) {}
			Keys(const void *key, const Version &v);
			Keys reverse() const;

			void store(void *dst) const;

			std::uint32_t rcon(std::size_t n);
			std::uint32_t& word(std::size_t n) { return data[n]; }
			std::uint32_t rotsub(std::size_t n) const;
			std::uint32_t sub(std::size_t n) const;

			WState state(std::size_t n) { return data + n*State_Words; }
			const std::uint32_t* state(std::size_t n) const
			{ return data + n*State_Words; }

		};
	}
	typedef Impl<ui32::Keys, ui32::State> Impl_UI32_t;

	struct Impl_UI32
	{
		typedef ui32::State State ;
		ui32::Keys ekeys;
		ui32::Keys dkeys;

		Impl_UI32(const void *key, const Version &v);

		void encrypt(State &s) const;
		void decrypt(State &s) const;
		std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const;
		std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const;
		std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const;
		std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const;
	};
}
#else
#include "aes/uil32.hpp"
namespace aes {
	typedef Impl_UIL32 Impl_UI32;
	typedef Impl_UIL32 Impl_UI32_t;
}
#endif

#endif
