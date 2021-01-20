#ifndef AES_UTIL_H
#define AES_UTIL_H

#include <aes/aes_types.hpp>
#include <aes/aes.hpp>

#include <cstring>
#include <cstdint>
#include <stdexcept>

#ifdef AES_DEBUG
#include <iostream>
#endif

namespace aes
{
	inline void print_word(const void *s)
	{
#ifdef AES_DEBUG
		auto *word = reinterpret_cast<const aes::ubyte*>(s);
		std::cout << std::hex;
		for (int j = 0; j < WORDBYTES; ++j)
		{
			std::cout << '\t' << static_cast<unsigned int>(word[j]);
		}
		std::cout << std::endl;
		std::cout << std::dec;
#endif
	}

	inline void print_state(const void *s)
	{
#ifdef AES_DEBUG
		auto state = reinterpret_cast<const aes::ubyte*>(s);
		for (int i = 0; i < STATEBYTES; i += WORDBYTES)
		{
			print_word(state + i);
		}
		std::cout << std::dec;
#endif
	}


	class AESLogger
	{
		public:
			template<class T>
			const AESLogger& operator<<(const T &thing) const
			{
#ifdef AES_DEBUG
				std::cerr << thing;
#endif
				return *this;
			}

			const AESLogger& operator<<(std::ostream& (*func)(std::ostream&)) const
			{
#ifdef AES_DEBUG
				std::cerr << func;
#endif
				return *this;
			}
	};
	template<class T>
	inline AESLogger logd(const T &thing)
	{
		AESLogger l;
#ifdef AES_DEBUG
		l << thing;
#endif
		return l;
	}

//helper implementations using state-only encryption/decryption
	template<class codec>
	void encrypt_ecb(
		void *data,
		std::size_t length,
		const AES_statevec &roundkeys)
	{
		ubyte padding = static_cast<ubyte>(AES::padding(length));
		auto *d = reinterpret_cast<ubyte*>(data);
		auto *end = d + length;
		std::memset(end, padding, padding);
		end += padding;
		AESState tmp;
		for (; d < end; d += STATEBYTES)
		{
			tmp << d;
			codec::encrypt(tmp, roundkeys);
			tmp >> d;
		}
	}

	template<class codec>
	std::size_t decrypt_ecb(
		void *data,
		std::size_t length,
		const AES_statevec &roundkeys,
		const AES_statevec &iroundkeys)
	{
		if (length % STATEBYTES)
		{ throw std::runtime_error("invalid encrypted data length"); }
		auto *d = reinterpret_cast<ubyte*>(data);
		auto *end = d + length;
		AESState tmp;
		for (; d < end; d += STATEBYTES)
		{
			tmp << d;
			codec::decrypt(tmp, iroundkeys);
			tmp >> d;
		}
		return *(d - 1);
	}

	template<class codec>
	void encrypt_cbc(
		void *data,
		std::size_t length,
		const AES_statevec &roundkeys)
	{
		ubyte padding = static_cast<ubyte>(AES::padding(length));
		std::size_t outlen = length + padding;
		AES_statevec aligned(outlen / STATEBYTES);
		std::memcpy(&aligned[0], data, length);
		std::memset(
			reinterpret_cast<ubyte*>(&aligned[0]) + length,
			padding,
			padding);

		AESState *begin = &aligned[0];
		auto *end = begin + aligned.size();
		codec::encrypt(*begin, roundkeys);
		++begin;
		for (; begin != end; ++begin)
		{
			codec::xorstate(begin, (begin - 1));
			codec::encrypt(*begin, roundkeys);
		}
		std::memcpy(data, &aligned[0], outlen);
	}

	template<class codec>
	std::size_t decrypt_cbc(
		void *data,
		std::size_t length,
		const AES_statevec &roundkeys,
		const AES_statevec &iroundkeys)
	{
		if (length % STATEBYTES || length == 0)
		{ throw std::runtime_error("invalid encrypted data length"); }
		AES_statevec tmp(length / STATEBYTES);
		std::memcpy(&tmp[0], data, length);

		AESState *end = &tmp[0];
		AESState *begin = end + tmp.size() - 1;
		for (; begin != end; --begin)
		{
			codec::decrypt(*begin, iroundkeys);
			codec::xorstate(begin, begin - 1);
		}
		codec::decrypt(*end, iroundkeys);
		std::memcpy(data, &tmp[0], length);
		return reinterpret_cast<ubyte*>(data)[length - 1];
	}

}
#endif
