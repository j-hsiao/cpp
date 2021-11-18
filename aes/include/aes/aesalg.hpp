#ifndef AES_ALG_HPP
#define AES_ALG_HPP

#include "aes/defs.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
namespace aes
{
	//	//state encryption
	//		Keys::CState Keys::state(i) const
	//		Codec::State
	//			operator^=(Keys::CState&&)
	//			aes_enc(Keys::CState&&)
	//			aes_enclast(Keys::CState&&)
	//			aes_dec(Keys::CState&&)
	//			aes_declast(Keys::CState&&)
	//
	//	//algorithm encryption
	//	//encrypt/decrypt src into dst via state
	//		Codec::State();
	//		Codec::State::init(uchar *dst, const uchar *src)
	//		Codec::State::operator>>(unsigned char *dst) const;
	//		Codec::State::operator^=(Codec::State&)
	//		Codec::encrypt(Codec::State&)
	//		Codec::decrypt(Codec::State&)
	//
	template<class State, class KeyType>
	void encrypt_state(
		State &state, const KeyType &keys, const Version &version)
	{
		state ^= keys.state(0);
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ state.aes_enc(keys.state(i)); }
		state.aes_enclast(keys.state(version.Round_Keys-1));
	}
	template<class State, class KeyType>
	void decrypt_state(
		State &state, const KeyType &ikeys, const Version &version)
	{
		state ^= ikeys.state(0);
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ state.aes_dec(ikeys.state(i)); }
		state.aes_declast(ikeys.state(version.Round_Keys-1));
	}

	template<class T>
	std::size_t encrypt_ecb(
		void *dst_, const void *src_,
		std::size_t nbytes, const T &codec)
	{
		auto dst = reinterpret_cast<unsigned char*>(dst_);
		auto src = reinterpret_cast<const unsigned char*>(src_);
		std::size_t remain = nbytes % State_Bytes;
		nbytes -= remain;
		unsigned char padding = static_cast<unsigned char>(State_Bytes - remain);

		const unsigned char * const end = src + nbytes;
		typename T::State state;
		for (; src<end; src += State_Bytes, dst += State_Bytes)
		{
			state.init(dst, src);
			codec.encrypt(state);
			state >> dst;
		}
		unsigned char padded[State_Bytes];
		std::memcpy(padded, src, remain);
		std::memset(padded + remain, padding, padding);
		state.init(dst, padded);
		codec.encrypt(state);
		state >> dst;
		return nbytes + State_Bytes;
	};

	template<class T>
	std::size_t decrypt_ecb(
		void *dst_, const void *src_,
		std::size_t nbytes, const T &codec)
	{
		auto dst = reinterpret_cast<unsigned char*>(dst_);
		auto src = reinterpret_cast<const unsigned char*>(src_);
		const unsigned char * const end = src + nbytes;
		typename T::State state;
		for (; src<end; src += State_Bytes, dst += State_Bytes)
		{
			state.init(dst, src);
			codec.decrypt(state);
			state >> dst;
		}
		unsigned char pad = *(dst-1);
		if (pad == 0 || pad > State_Bytes)
		{
			throw std::runtime_error(
				"invalid padding " + std::to_string(static_cast<int>(pad)));
		}
		return nbytes - pad;
	};

	template<class T>
	std::size_t encrypt_cbc(
		void *dst_, const void *src_,
		std::size_t nbytes, const T &codec)
	{
		if (nbytes < State_Bytes)
		{ return encrypt_ecb<T>(dst_, src_, nbytes, codec); }
		std::size_t remain = nbytes % State_Bytes;
		nbytes -= remain;
		unsigned char padding = static_cast<unsigned char>(State_Bytes - remain);
		auto dst = reinterpret_cast<unsigned char*>(dst_);
		auto src = reinterpret_cast<const unsigned char*>(src_);
		typename T::State states[2];
		typename T::State *pre = states;
		typename T::State *post = states+1;
		const unsigned char * const end = src + nbytes;
		pre->init(dst, src);
		codec.encrypt(*pre);
		*pre >> dst;
		src += State_Bytes;
		dst += State_Bytes;
		for (; src < end; src += State_Bytes, dst += State_Bytes)
		{
			post->init(dst, src);
			*post ^= *pre;
			codec.encrypt(*post);
			*post >> dst;
			auto tmp = pre;
			pre = post;
			post = tmp;
		}
		unsigned char padded[State_Bytes];
		std::memcpy(padded, src, remain);
		std::memset(padded + remain, padding, padding);
		post->init(dst, padded);
		*post ^= *pre;
		codec.encrypt(*post);
		*post >> dst;
		return nbytes + State_Bytes;
	};

	template<class T>
	std::size_t decrypt_cbc(
		void *dst_, const void *src_,
		std::size_t nbytes, const T &codec)
	{
		if (nbytes <= State_Bytes)
		{ return aes::decrypt_ecb<T>(dst_, src_, nbytes, codec); }
		auto dst = reinterpret_cast<unsigned char*>(dst_);
		auto src = reinterpret_cast<const unsigned char*>(src_);
		auto const end = dst;
		src += nbytes - State_Bytes;
		dst += nbytes - State_Bytes;
		typename T::State states[2];
		typename T::State *pre = states;
		typename T::State *post = states+1;

		post->init(dst, src);
		codec.decrypt(*post);
		src -= State_Bytes;
		for (; dst > end; src -= State_Bytes)
		{
			unsigned char *nxt = dst - State_Bytes;
			pre->init(nxt, src);
			*post ^= *pre;
			*post >> dst;
			dst = nxt;
			codec.decrypt(*pre);
			auto tmp = pre;
			pre = post;
			post = tmp;
		}
		*post >> dst;
		unsigned char pad = *(end+nbytes-1);
		if (pad == 0 || pad > State_Bytes)
		{
			throw std::runtime_error(
				"invalid padding " + std::to_string(static_cast<int>(pad)));
		}
		return nbytes - pad;
	};

	template<class Keytype, class Statetype>
	struct Impl
	{
		typedef Statetype State;
		Keytype ekeys;
		Keytype dkeys;

		Impl(const void *key, const Version &version):
			ekeys(key, version),
			dkeys(ekeys.reverse())
		{}

		void encrypt(State &s) const
		{ encrypt_state(s, ekeys, ekeys.version); }
		void decrypt(State &s) const
		{ decrypt_state(s, dkeys, dkeys.version); }

		std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const
		{ return aes::encrypt_ecb(dst, src, sz, *this); }
		std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const
		{ return aes::decrypt_ecb(dst, src, sz, *this); }
		std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const
		{ return aes::encrypt_cbc(dst, src, sz, *this); }
		std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const
		{ return aes::decrypt_cbc(dst, src, sz, *this); }
	};
}
#endif
