#include <aes/aes.h>
#include <.aes/types.hpp>

#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
namespace aes
{
	struct Impl
	{
		std::size_t (*encrypt_ecb)(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		std::size_t (*decrypt_ecb)(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		std::size_t (*encrypt_cbc)(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		std::size_t (*decrypt_cbc)(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
	};
	template<class T>
	Impl make_impl()
	{
		return {
			T::encrypt_ecb, T::decrypt_ecb, T::encrypt_cbc, T::decrypt_cbc };
	}


	//prepadded
	struct PlainImpl
	{
		static std::size_t encrypt_ecb(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t decrypt_ecb(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t encrypt_cbc(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t decrypt_cbc(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
	};

	struct AESIntrinImpl
	{
		static std::size_t encrypt_ecb(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t decrypt_ecb(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t encrypt_cbc(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t decrypt_cbc(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
	};

	struct SSLImpl
	{
		static std::size_t encrypt_ecb(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t decrypt_ecb(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t encrypt_cbc(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t decrypt_cbc(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
	};
	struct SSEImpl
	{
		static std::size_t encrypt_ecb(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t decrypt_ecb(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t encrypt_cbc(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
		static std::size_t decrypt_cbc(
			aes__Byte *dst, const aes__Byte *src,
			const aes__Keys &keys, size_t datalen);
	};

	//helper template functions
	//T should be a class that encrypts/decrypts a single state:
	//	encrypt_state(dst, src, keys)
	//	decrypt_state(dst, src, keys)
	//ecb mode
	template<class T>
	std::size_t encrypt_ecb(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, std::size_t nbytes)
	{
		std::size_t remain = nbytes % aes__State_Bytes;
		aes__Byte padbytes = static_cast<aes__Byte>(aes__State_Bytes - remain);
		std::size_t total = nbytes + padbytes;
		const aes__Byte * const end = src + total - aes__State_Bytes;

		aes__Word state[aes__State_Words];
		for (; src < end; src += aes__State_Bytes, dst += aes__State_Bytes)
		{
			load_words(state, src, aes__State_Bytes);
			T::encrypt_state(state, state, keys);
			store_words(dst, state, aes__State_Bytes);
		}
		load_words(state, src, remain);
		for (std::size_t i = remain; i < aes__State_Bytes; ++i)
		{ set_state_byte(state, padbytes, i); }
		T::encrypt_state(state, state, keys);
		store_words(dst, state, aes__State_Bytes);
		return total;
	}
	template<class T>
	std::size_t decrypt_ecb(
		aes__Byte *dst, const aes__Byte *src, const aes__Keys &keys, std::size_t nbytes)
	{
		if (nbytes % aes__State_Bytes)
		{ return aes__Invalid; }

		aes__Word state[aes__State_Bytes];
		const aes__Byte * const end = src + nbytes;
		for (; src < end; src += aes__State_Bytes, dst += aes__State_Bytes)
		{
			load_words(state, src, aes__State_Bytes);
			T::decrypt_state(state, state, keys);
			store_words(dst, state, aes__State_Bytes);
		}
		return nbytes - dst[-1];
	}
	//cbc mode
	template<class T>
	std::size_t encrypt_cbc(
		aes__Byte *dst, const aes__Byte *src, const aes__Keys &keys, std::size_t nbytes)
	{
		std::size_t remain = nbytes % aes__State_Bytes;
		aes__Byte padbytes = static_cast<aes__Byte>(aes__State_Bytes - remain);
		std::size_t total = nbytes + padbytes;
		const aes__Byte * const end = src + total - aes__State_Bytes;

		aes__Word istate[aes__State_Words];
		aes__Word ostate[aes__State_Words];
		std::fill(ostate, ostate + aes__State_Words, 0);
		for (; src < end; src += aes__State_Bytes, dst += aes__State_Bytes)
		{
			load_words(istate, src, aes__State_Bytes);
			aes::xor_state(istate, ostate);
			T::encrypt_state(ostate, istate, keys);
			store_words(dst, ostate, aes__State_Bytes);
		}
		load_words(istate, src, remain);
		for (std::size_t i = remain; i < aes__State_Bytes; ++i)
		{ set_state_byte(istate, padbytes, i); }
		aes::xor_state(istate, ostate);
		T::encrypt_state(ostate, istate, keys);
		store_words(dst, ostate, aes__State_Bytes);
		return total;
	}
	template<class T>
	std::size_t decrypt_cbc(
		aes__Byte *dst, const aes__Byte *src, const aes__Keys &keys, std::size_t nbytes)
	{
		if (nbytes % aes__State_Bytes)
		{ return aes__Invalid; }

		aes__Word istates[2][aes__State_Bytes];
		bool cur = 0;
		std::fill(istates[!cur], istates[!cur] + aes__State_Words, 0);
		aes__Word ostate[aes__State_Bytes];

		const aes__Byte * const end = src + nbytes;
		for (; src < end; src += aes__State_Bytes, dst += aes__State_Bytes)
		{
			load_words(istates[cur], src, aes__State_Bytes);
			T::decrypt_state(ostate, istates[cur], keys);
			cur = !cur;
			aes::xor_state(ostate, istates[cur]);
			store_words(dst, ostate, aes__State_Bytes);
		}
		return nbytes - dst[-1];
	}
}
