#ifndef AES_IMPL_AES_HPP
#define AES_IMPL_AES_HPP
#include <aes/aes.h>
namespace aes
{
	/*	next impl method to test:
	 *		each implementation specifies types, methods, etc
	 *		templates will use those types and impls to perform aes
	 *	
	 *	impl requirements:
	 *	types:
	 *		State
	 *		StateIter:
	 *			methods:
	 *				StateIter(const unsigned char*, std::size_t);
	 *				bool next(State&) // store next state into input
	 *			members:
	 *				std::size_t total // total bytes
		*		Info
	 *	methods:
	 *		encrypt_state(State out, State in, Info);
	 *		decrypt_state(State out, State in, Info);
	 *
	 *		wrot(word)
	 *		wxor(w1, w2)
	 *
	 *		sxor(state1, state2)
	 *		srot(state)
	 *		sirot(state)
	 *		mixcol(state)
	 *
	 *		store_state(unsigned char *dst, const State&)
	*/

	template<class T>
	std::size_t encrypt_ecb(
		unsigned char *dst, const unsigned char *src,
		const typename T::Info &info, std::size_t nbytes)
	{
		typename T::StateIter state_iter(src, nbytes);
		typename T::State state;
		while (state_iter.next(state))
		{
			T::encrypt_state(state, state, info);
			T::store_state(dst, state);
		}
		return state_iter.total;
	}
	template<class T>
	std::size_t decrypt_ecb(
		unsigned char *dst, const unsigned char *src,
		const typename T::Info &info, std::size_t nbytes)
	{
		if (nbytes % aes__State_Bytes)
		{ return aes__Invalid; }
		typename T::StateIter state_iter(src, nbytes);
		typename T::State state;
		while (state_iter.next(state))
		{
			T::decrypt_state(state, state, info);
			T::store_state(dst, state);
		}
		return nbytes - dst[nbytes - 1];
	}

}
#endif
