//Template key expansion
#ifndef AES_IMPL_KEYS_HPP
#define AES_IMPL_KEYS_HPP

#include "aes/defs.hpp"

#include <cstddef>

namespace aes
{
	//Keys
	//	Keys(const unsigned char*, const Version&)
	//	Word rcon(n);
	//	Word word(N);
	//	Word rotsub(N);
	//	Word sub(N);
	//	State state(N);
	//	CState state(N) const;
	//	Keys reverse() const;
	//Word:
	//	operator^
	//	operator=(<word xor result>)&&
	//		copy data
	//State:
	//	imix();
	//	operator=(CState&&)&&
	//		copy data

	//https://en.wikipedia.org/wiki/AES_key_schedule

	template<class T>
	void init_keys(T &k, const Version &version)
	{
		const std::size_t N = version.Key_Bytes / Word_Bytes;
		const std::size_t R = version.Round_Keys;
		for (std::size_t i=N; i<R*State_Words; ++i)
		{
			if (!(i%N))
			{ k.word(i) = k.word(i-N) ^ k.rcon(i/N) ^ k.rotsub(i-1); }
			else if (N>6 && i%N == 4)
			{ k.word(i) = k.sub(i-1) ^ k.word(i-N); }
			else
			{ k.word(i) = k.word(i-1) ^ k.word(i-N); }
		}
	}

	template<class T>
	void reverse_keys(T &k, const T &encryption_keys, const Version &version)
	{
		std::size_t last = version.Round_Keys - 1;
		for (std::size_t i=0; i<=last; ++i)
		{ k.state(i) = encryption_keys.state(last-i); }
		for (std::size_t i=1; i<last; ++i)
		{ k.state(i).imix(); }
	}
}
#endif
