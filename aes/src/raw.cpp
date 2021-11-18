#include "aes/raw.hpp"
#include "aes/defs.hpp"

#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <climits>
#include <string>

namespace aes
{
	namespace
	{
		std::uint_least32_t fixw(std::uint_least32_t w)
		{
			if (CHAR_BIT == 8 && sizeof(std::uint_least32_t) == 4)
			{ return w; }
			else { return w & 0xFFFFFFFFu; }
		}
		unsigned char fixb(unsigned char b)
		{
			if (CHAR_BIT == 8) { return b; }
			else { return b & 0xFFu; }
		}
		unsigned char nthbyte(std::uint_least32_t w, std::size_t n)
		{ return fixb(static_cast<unsigned char>(w >> Byte_Bits*(Word_Bytes-(n+1)))); }
		std::uint_least32_t loaduil(const unsigned char *d)
		{
			return static_cast<std::uint_least32_t>(fixb(d[0])) << 24
				^ static_cast<std::uint_least32_t>(fixb(d[1])) << 16
				^ static_cast<std::uint_least32_t>(fixb(d[2])) << 8
				^ static_cast<std::uint_least32_t>(fixb(d[3]));
		}
		void storeuil(unsigned char *dst, std::uint_least32_t w)
		{
			for (std::size_t i=0; i<Word_Bytes; ++i)
			{ dst[i] = nthbyte(w, i); }
		}


		std::uint_least32_t subuil(std::uint_least32_t w)
		{
			const unsigned char *b = SBox();
			unsigned char bytes[Word_Bytes] = {
				b[nthbyte(w, 0)], b[nthbyte(w, 1)], b[nthbyte(w, 2)], b[nthbyte(w, 3)] };
			return loaduil(bytes);
		}
		std::uint_least32_t rsubuil(std::uint_least32_t w)
		{
			const unsigned char *b = SBox();
			unsigned char bytes[Word_Bytes] = {
				b[nthbyte(w, 1)], b[nthbyte(w, 2)], b[nthbyte(w, 3)], b[nthbyte(w, 0)] };
			return loaduil(bytes);
		}
		std::uint_least32_t rconuil(std::size_t i)
		{
			unsigned char bytes[Word_Bytes] = {aes::rcon(i), 0, 0, 0};
			return loaduil(bytes);
		}

		template<class T>
		T& getkw(T (&arr)[Max_Rounds][State_Words], std::size_t i)
		{ return arr[i/State_Words][i%State_Words]; }

		struct colresuil
		{
			std::uint_least32_t data[256];
			colresuil(
				unsigned char col,
				aes::word (*func)(unsigned char, unsigned char),
				const unsigned char *box)
			{
				for (std::size_t i=0; i<0x100u; ++i)
				{
					auto w = func(box[i], col);
					data[i] = loaduil(w.data);
				}
			}
			operator const std::uint_least32_t*() const { return data; }
		};

		colresuil coluil[State_Words] = {
			{0, mixcol, SBox()}, {1, mixcol, SBox()}, {2, mixcol, SBox()}, {3, mixcol, SBox()} };
		colresuil icoluil[State_Words] = {
			{0, imixcol, IBox()}, {1, imixcol, IBox()}, {2, imixcol, IBox()}, {3, imixcol, IBox()} };


		//using these rotation tables seems to be ~same speed as just hardcoding them
		//for uint_least32_t BUT using rotation tables for uint32_t is much slower than hardcoding
		//ulrot[m][n] = source for the mth byte of the nth post-rotation word
		//	and also the source for the nth byte of the mth post-rotation word
		static const std::size_t ulrot[][4] = {
			{0, 1, 2, 3}, {1, 2, 3, 0}, {2, 3, 0, 1}, {3, 0, 1, 2}};
#	if AES_UIL32_BYTEFIRST
		//ulirot[m][n] = source for the mth byte of the nth post-rotation word
		//(byte first), may use less cache because use the 256xsizeof(uilword)
		//lut one at a time
		static const std::size_t ulirot[][4] = {
			{0, 1, 2, 3}, {3, 0, 1, 2}, {2, 3, 0, 1}, {1, 2, 3, 0}};
#	endif
		//ulicrot[m][n] = source for the nth byte of the mth post-rotation word
		//(word first)
		//uses the 4 256xsizeof(uilword) luts all at once
		static const std::size_t ulicrot[][4] = {
			{0, 3, 2, 1}, {1, 0, 3, 2}, {2, 1, 0, 3}, {3, 2, 1, 0}};
#if AES_UIL32_PASSTMP
#pragma message("uil32 passtemp")
		void encuil(
			std::uint_least32_t *data, const std::uint_least32_t *key,
			std::uint_least32_t *tmp)
		{
#		if AES_UIL32_BYTEFIRST
//			for (std::size_t w=0; w<State_Words; ++w)
//			{ tmp[w] = coluil[0][nthbyte(data[w], 0)]; }
//			for (std::size_t nth=1; nth<Word_Bytes; ++nth)
//			{
//				for (std::size_t i=0; i<State_Words; ++i)
//				{ tmp[i] ^= coluil[nth][nthbyte(data[ulrot[nth][i]], nth)]; }
//			}

			tmp[0] = coluil[0][nthbyte(data[0], 0)];
			tmp[1] = coluil[0][nthbyte(data[1], 0)];
			tmp[2] = coluil[0][nthbyte(data[2], 0)];
			tmp[3] = coluil[0][nthbyte(data[3], 0)];
			tmp[0] ^= coluil[1][nthbyte(data[1], 1)];
			tmp[1] ^= coluil[1][nthbyte(data[2], 1)];
			tmp[2] ^= coluil[1][nthbyte(data[3], 1)];
			tmp[3] ^= coluil[1][nthbyte(data[0], 1)];
			tmp[0] ^= coluil[2][nthbyte(data[2], 2)];
			tmp[1] ^= coluil[2][nthbyte(data[3], 2)];
			tmp[2] ^= coluil[2][nthbyte(data[0], 2)];
			tmp[3] ^= coluil[2][nthbyte(data[1], 2)];
			tmp[0] ^= coluil[3][nthbyte(data[3], 3)];
			tmp[1] ^= coluil[3][nthbyte(data[0], 3)];
			tmp[2] ^= coluil[3][nthbyte(data[1], 3)];
			tmp[3] ^= coluil[3][nthbyte(data[2], 3)];
#		else
// 			for (std::size_t w=0; w<State_Words; ++w)
// 			{
// 				tmp[w] = coluil[0][nthbyte(data[ulrot[w][0]], 0)]
// 					^ coluil[1][nthbyte(data[ulrot[w][1]], 1)]
// 					^ coluil[2][nthbyte(data[ulrot[w][2]], 2)]
// 					^ coluil[3][nthbyte(data[ulrot[w][3]], 3)];
// 			}
			tmp[0] = coluil[0][nthbyte(data[0], 0)]
				^ coluil[1][nthbyte(data[1], 1)]
				^ coluil[2][nthbyte(data[2], 2)]
				^ coluil[3][nthbyte(data[3], 3)];
			tmp[1] = coluil[0][nthbyte(data[1], 0)]
				^ coluil[1][nthbyte(data[2], 1)]
				^ coluil[2][nthbyte(data[3], 2)]
				^ coluil[3][nthbyte(data[0], 3)];
			tmp[2] = coluil[0][nthbyte(data[2], 0)]
				^ coluil[1][nthbyte(data[3], 1)]
				^ coluil[2][nthbyte(data[0], 2)]
				^ coluil[3][nthbyte(data[1], 3)];
			tmp[3] = coluil[0][nthbyte(data[3], 0)]
				^ coluil[1][nthbyte(data[0], 1)]
				^ coluil[2][nthbyte(data[1], 2)]
				^ coluil[3][nthbyte(data[2], 3)];
#		endif
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] = tmp[i] ^ key[i]; }
		}
		void encuillast(
			std::uint_least32_t *data, const std::uint_least32_t *key,
			std::uint_least32_t *tmp)
		{
			const unsigned char *b = SBox();
			for (std::size_t i=0; i<State_Words; ++i)
			{
				unsigned char w[Word_Bytes];
				for (std::size_t j=0; j<Word_Bytes; ++j)
				{ w[j] = b[nthbyte(data[ulrot[i][j]], j)]; }
				tmp[i] = loaduil(w);
			}
			for (std::size_t i=0; i<State_Words; ++i)
#		if AES_UIL32_INTO
			{ tmp[i] ^= key[i]; }
#		else
			{ data[i] = tmp[i] ^ key[i]; }
#		endif
		}
		void decuil(
			std::uint_least32_t *data, const std::uint_least32_t *key,
			std::uint_least32_t *tmp)
		{
#		if AES_UIL32_BYTEFIRST
//			for (std::size_t w=0; w<State_Words; ++w)
//			{ tmp[w] = icoluil[0][nthbyte(data[w], 0)]; }
//			for (std::size_t nth=1; nth<Word_Bytes; ++nth)
//			{
//				for (std::size_t i=0; i<State_Words; ++i)
//				{ tmp[i] ^= icoluil[nth][nthbyte(data[ulirot[nth][i]], nth)]; }
//			}

			tmp[0] = icoluil[0][nthbyte(data[0], 0)];
			tmp[1] = icoluil[0][nthbyte(data[1], 0)];
			tmp[2] = icoluil[0][nthbyte(data[2], 0)];
			tmp[3] = icoluil[0][nthbyte(data[3], 0)];
			tmp[0] ^= icoluil[1][nthbyte(data[3], 1)];
			tmp[1] ^= icoluil[1][nthbyte(data[0], 1)];
			tmp[2] ^= icoluil[1][nthbyte(data[1], 1)];
			tmp[3] ^= icoluil[1][nthbyte(data[2], 1)];
			tmp[0] ^= icoluil[2][nthbyte(data[2], 2)];
			tmp[1] ^= icoluil[2][nthbyte(data[3], 2)];
			tmp[2] ^= icoluil[2][nthbyte(data[0], 2)];
			tmp[3] ^= icoluil[2][nthbyte(data[1], 2)];
			tmp[0] ^= icoluil[3][nthbyte(data[1], 3)];
			tmp[1] ^= icoluil[3][nthbyte(data[2], 3)];
			tmp[2] ^= icoluil[3][nthbyte(data[3], 3)];
			tmp[3] ^= icoluil[3][nthbyte(data[0], 3)];
#		else
// 			for (std::size_t w=0; w<State_Words; ++w)
// 			{
// 				tmp[w] = icoluil[0][nthbyte(data[ulicrot[w][0]], 0)]
// 					^ icoluil[1][nthbyte(data[ulicrot[w][1]], 1)]
// 					^ icoluil[2][nthbyte(data[ulicrot[w][2]], 2)]
// 					^ icoluil[3][nthbyte(data[ulicrot[w][3]], 3)];
// 			}
			tmp[0] = icoluil[0][nthbyte(data[0], 0)]
				^ icoluil[1][nthbyte(data[3], 1)]
				^ icoluil[2][nthbyte(data[2], 2)]
				^ icoluil[3][nthbyte(data[1], 3)];
			tmp[1] = icoluil[0][nthbyte(data[1], 0)]
				^ icoluil[1][nthbyte(data[0], 1)]
				^ icoluil[2][nthbyte(data[3], 2)]
				^ icoluil[3][nthbyte(data[2], 3)];
			tmp[2] = icoluil[0][nthbyte(data[2], 0)]
				^ icoluil[1][nthbyte(data[1], 1)]
				^ icoluil[2][nthbyte(data[0], 2)]
				^ icoluil[3][nthbyte(data[3], 3)];
			tmp[3] = icoluil[0][nthbyte(data[3], 0)]
				^ icoluil[1][nthbyte(data[2], 1)]
				^ icoluil[2][nthbyte(data[1], 2)]
				^ icoluil[3][nthbyte(data[0], 3)];

#		endif
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] = tmp[i] ^ key[i]; }
		}
		void decuillast(
			std::uint_least32_t *data, const std::uint_least32_t *key,
			std::uint_least32_t *tmp)
		{
			const unsigned char *b = IBox();
			for (std::size_t i=0; i<State_Words; ++i)
			{
				unsigned char w[Word_Bytes];
				for (std::size_t j=0; j<Word_Bytes; ++j)
				{ w[j] = b[nthbyte(data[ulicrot[i][j]], j)]; }
				tmp[i] = loaduil(w);
			}
			for (std::size_t i=0; i<State_Words; ++i)
#		if AES_UIL32_INTO
			{ tmp[i] ^= key[i]; }
#		else
			{ data[i] = tmp[i] ^ key[i]; }
#		endif
		}
#else
#pragma message("uil32 no pass tmp")
		void encuilt(std::uint_least32_t *data, const std::uint_least32_t *key)
		{
			std::uint_least32_t tmp[State_Words];
#		if AES_UIL32_BYTEFIRST
//			for (std::size_t w=0; w<State_Words; ++w)
//			{ tmp[w] = coluil[0][nthbyte(data[w], 0)]; }
//			for (std::size_t nth=1; nth<Word_Bytes; ++nth)
//			{
//				for (std::size_t i=0; i<State_Words; ++i)
//				{ tmp[i] ^= coluil[nth][nthbyte(data[ulrot[nth][i]], nth)]; }
//			}

			tmp[0] = coluil[0][nthbyte(data[0], 0)];
			tmp[1] = coluil[0][nthbyte(data[1], 0)];
			tmp[2] = coluil[0][nthbyte(data[2], 0)];
			tmp[3] = coluil[0][nthbyte(data[3], 0)];
			tmp[0] ^= coluil[1][nthbyte(data[1], 1)];
			tmp[1] ^= coluil[1][nthbyte(data[2], 1)];
			tmp[2] ^= coluil[1][nthbyte(data[3], 1)];
			tmp[3] ^= coluil[1][nthbyte(data[0], 1)];
			tmp[0] ^= coluil[2][nthbyte(data[2], 2)];
			tmp[1] ^= coluil[2][nthbyte(data[3], 2)];
			tmp[2] ^= coluil[2][nthbyte(data[0], 2)];
			tmp[3] ^= coluil[2][nthbyte(data[1], 2)];
			tmp[0] ^= coluil[3][nthbyte(data[3], 3)];
			tmp[1] ^= coluil[3][nthbyte(data[0], 3)];
			tmp[2] ^= coluil[3][nthbyte(data[1], 3)];
			tmp[3] ^= coluil[3][nthbyte(data[2], 3)];
#		else
// 			for (std::size_t w=0; w<State_Words; ++w)
// 			{
// 				tmp[w] = coluil[0][nthbyte(data[ulrot[w][0]], 0)]
// 					^ coluil[1][nthbyte(data[ulrot[w][1]], 1)]
// 					^ coluil[2][nthbyte(data[ulrot[w][2]], 2)]
// 					^ coluil[3][nthbyte(data[ulrot[w][3]], 3)];
// 			}
			tmp[0] = coluil[0][nthbyte(data[0], 0)]
				^ coluil[1][nthbyte(data[1], 1)]
				^ coluil[2][nthbyte(data[2], 2)]
				^ coluil[3][nthbyte(data[3], 3)];
			tmp[1] = coluil[0][nthbyte(data[1], 0)]
				^ coluil[1][nthbyte(data[2], 1)]
				^ coluil[2][nthbyte(data[3], 2)]
				^ coluil[3][nthbyte(data[0], 3)];
			tmp[2] = coluil[0][nthbyte(data[2], 0)]
				^ coluil[1][nthbyte(data[3], 1)]
				^ coluil[2][nthbyte(data[0], 2)]
				^ coluil[3][nthbyte(data[1], 3)];
			tmp[3] = coluil[0][nthbyte(data[3], 0)]
				^ coluil[1][nthbyte(data[0], 1)]
				^ coluil[2][nthbyte(data[1], 2)]
				^ coluil[3][nthbyte(data[2], 3)];
#		endif
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] = tmp[i] ^ key[i]; }
		}
		void decuilt(std::uint_least32_t *data, const std::uint_least32_t *key)
		{
			std::uint_least32_t tmp[State_Words];
#		if AES_UIL32_BYTEFIRST
//			for (std::size_t w=0; w<State_Words; ++w)
//			{ tmp[w] = icoluil[0][nthbyte(data[w], 0)]; }
//			for (std::size_t nth=1; nth<Word_Bytes; ++nth)
//			{
//				for (std::size_t i=0; i<State_Words; ++i)
//				{ tmp[i] ^= icoluil[nth][nthbyte(data[ulirot[nth][i]], nth)]; }
//			}

			tmp[0] = icoluil[0][nthbyte(data[0], 0)];
			tmp[1] = icoluil[0][nthbyte(data[1], 0)];
			tmp[2] = icoluil[0][nthbyte(data[2], 0)];
			tmp[3] = icoluil[0][nthbyte(data[3], 0)];
			tmp[0] ^= icoluil[1][nthbyte(data[3], 1)];
			tmp[1] ^= icoluil[1][nthbyte(data[0], 1)];
			tmp[2] ^= icoluil[1][nthbyte(data[1], 1)];
			tmp[3] ^= icoluil[1][nthbyte(data[2], 1)];
			tmp[0] ^= icoluil[2][nthbyte(data[2], 2)];
			tmp[1] ^= icoluil[2][nthbyte(data[3], 2)];
			tmp[2] ^= icoluil[2][nthbyte(data[0], 2)];
			tmp[3] ^= icoluil[2][nthbyte(data[1], 2)];
			tmp[0] ^= icoluil[3][nthbyte(data[1], 3)];
			tmp[1] ^= icoluil[3][nthbyte(data[2], 3)];
			tmp[2] ^= icoluil[3][nthbyte(data[3], 3)];
			tmp[3] ^= icoluil[3][nthbyte(data[0], 3)];
#		else
// 			for (std::size_t w=0; w<State_Words; ++w)
// 			{
// 				tmp[w] = icoluil[0][nthbyte(data[ulicrot[w][0]], 0)]
// 					^ icoluil[1][nthbyte(data[ulicrot[w][1]], 1)]
// 					^ icoluil[2][nthbyte(data[ulicrot[w][2]], 2)]
// 					^ icoluil[3][nthbyte(data[ulicrot[w][3]], 3)];
// 			}
			tmp[0] = icoluil[0][nthbyte(data[0], 0)]
				^ icoluil[1][nthbyte(data[3], 1)]
				^ icoluil[2][nthbyte(data[2], 2)]
				^ icoluil[3][nthbyte(data[1], 3)];
			tmp[1] = icoluil[0][nthbyte(data[1], 0)]
				^ icoluil[1][nthbyte(data[0], 1)]
				^ icoluil[2][nthbyte(data[3], 2)]
				^ icoluil[3][nthbyte(data[2], 3)];
			tmp[2] = icoluil[0][nthbyte(data[2], 0)]
				^ icoluil[1][nthbyte(data[1], 1)]
				^ icoluil[2][nthbyte(data[0], 2)]
				^ icoluil[3][nthbyte(data[3], 3)];
			tmp[3] = icoluil[0][nthbyte(data[3], 0)]
				^ icoluil[1][nthbyte(data[2], 1)]
				^ icoluil[2][nthbyte(data[1], 2)]
				^ icoluil[3][nthbyte(data[0], 3)];

#		endif
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] = tmp[i] ^ key[i]; }
		}

#	define encuil(a,b,c) encuilt((a),(b))
#	define decuil(a,b,c) decuilt((a),(b))

#	if AES_UIL32_INTO
		void encuillast(std::uint_least32_t *data, const std::uint_least32_t *key, std::uint_least32_t *tmp)
		{
			const unsigned char *b = SBox();
			for (std::size_t i=0; i<State_Words; ++i)
			{
				unsigned char w[Word_Bytes];
				for (std::size_t j=0; j<Word_Bytes; ++j)
				{ w[j] = b[nthbyte(data[ulrot[i][j]], j)]; }
				tmp[i] = loaduil(w);
			}
			for (std::size_t i=0; i<State_Words; ++i)
			{ tmp[i] ^= key[i]; }
		}
		void decuillast(std::uint_least32_t *data, const std::uint_least32_t *key, std::uint_least32_t *tmp)
		{
			const unsigned char *b = IBox();
			for (std::size_t i=0; i<State_Words; ++i)
			{
				unsigned char w[Word_Bytes];
				for (std::size_t j=0; j<Word_Bytes; ++j)
				{ w[j] = b[nthbyte(data[ulicrot[i][j]], j)]; }
				tmp[i] = loaduil(w);
			}
			for (std::size_t i=0; i<State_Words; ++i)
			{ tmp[i] ^= key[i]; }
		}
#	else
		void encuillastt(std::uint_least32_t *data, const std::uint_least32_t *key)
		{
			const unsigned char *b = SBox();
			std::uint_least32_t tmp[State_Words];
			for (std::size_t i=0; i<State_Words; ++i)
			{
				unsigned char w[Word_Bytes];
				for (std::size_t j=0; j<Word_Bytes; ++j)
				{ w[j] = b[nthbyte(data[ulrot[i][j]], j)]; }
				tmp[i] = loaduil(w);
			}
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] = tmp[i] ^ key[i]; }
		}
		void decuillastt(std::uint_least32_t *data, const std::uint_least32_t *key)
		{
			const unsigned char *b = IBox();
			std::uint_least32_t tmp[State_Words];
			for (std::size_t i=0; i<State_Words; ++i)
			{
				unsigned char w[Word_Bytes];
				for (std::size_t j=0; j<Word_Bytes; ++j)
				{ w[j] = b[nthbyte(data[ulicrot[i][j]], j)]; }
				tmp[i] = loaduil(w);
			}
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] = tmp[i] ^ key[i]; }
		}
#	define encuillast(a,b,c) encuillastt((a),(b))
#	define decuillast(a,b,c) decuillastt((a),(b))
#	endif
#endif
	}
	const std::uint_least32_t* const * submixuil()
	{
		static std::uint_least32_t* parray[State_Words] = {
			coluil[0].data, coluil[1].data, coluil[2].data, coluil[3].data};
		return parray;
	}
	const std::uint_least32_t* const * isubmixuil()
	{
		static std::uint_least32_t* parray[State_Words] = {
			icoluil[0].data, icoluil[1].data, icoluil[2].data, icoluil[3].data};
		return parray;
	}

	Impl_RawUIL32::Impl_RawUIL32(const void *k, const Version &v):
		version(v)
	{
		const auto *key = reinterpret_cast<const unsigned char*>(k);
		for (std::size_t i=0; i<version.Key_Bytes/Word_Bytes; ++i)
		{ ekey[i/State_Words][i%State_Words] = loaduil(key + i*Word_Bytes); }

		const std::size_t N = version.Key_Bytes / Word_Bytes;
		const std::size_t R = version.Round_Keys;
		for (std::size_t i=N; i<R*State_Words; ++i)
		{
			if (!(i%N))
			{ getkw(ekey, i) = getkw(ekey, i-N) ^ rconuil(i/N) ^ rsubuil(getkw(ekey, i-1)); }
			else if (N>6 && i%N == 4)
			{ getkw(ekey, i) = subuil(getkw(ekey, i-1)) ^ getkw(ekey, i-N); }
			else
			{ getkw(ekey, i) = getkw(ekey, i-1) ^ getkw(ekey, i-N); }
		}

		for (std::size_t i=0; i<version.Round_Keys; ++i)
		{ std::memcpy(dkey[version.Round_Keys-(i+1)], ekey[i], sizeof(ekey[0])); }
		//imix
		const auto *b = SBox();
		for (std::size_t sidx=1; sidx<version.Round_Keys-1; ++sidx)
		{
			std::uint_least32_t (&state)[State_Words] = dkey[sidx];
			for (std::size_t widx=0; widx<State_Words; ++widx)
			{
				state[widx] = icoluil[0][b[nthbyte(state[widx], 0)]]
					^ icoluil[1][b[nthbyte(state[widx], 1)]]
					^ icoluil[2][b[nthbyte(state[widx], 2)]]
					^ icoluil[3][b[nthbyte(state[widx], 3)]];
			}
		}
	}
	void Impl_RawUIL32::storekey(void *d, word (&arr)[Max_Rounds][State_Words]) const
	{
		unsigned char *dst = reinterpret_cast<unsigned char*>(d);
		for (std::size_t sidx=0; sidx<version.Round_Keys; ++sidx)
		{
			for (std::size_t widx=0; widx<State_Words; ++widx, dst+=Word_Bytes)
			{ storeuil(dst, arr[sidx][widx]); }
		}
	}

	std::size_t Impl_RawUIL32::encrypt_ecb(void *d, const void *s, std::size_t sz) const
	{
		auto *dst = reinterpret_cast<unsigned char*>(d);
		const auto *src = reinterpret_cast<const unsigned char*>(s);

		std::size_t remain = sz % State_Bytes;
		sz-=remain;
		unsigned char padding = static_cast<unsigned char>(State_Bytes - remain);
		const auto * const end = src + sz;
		std::uint_least32_t state[State_Words];
#	if AES_UIL32_PASSTMP || AES_UIL32_INTO
		std::uint_least32_t tmp[State_Words];
#	endif
		while (src < end)
		{
			for (std::size_t i=0; i<State_Words; ++i, src += Word_Bytes)
			{ state[i] = loaduil(src) ^ ekey[0][i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ encuil(state, ekey[i], tmp); }
			encuillast(state, ekey[version.Round_Keys-1], tmp);
			for (std::size_t i=0; i<State_Words; ++i, dst+=Word_Bytes)
#		if AES_UIL32_INTO
			{ storeuil(dst, tmp[i]); }
#		else
			{ storeuil(dst, state[i]); }
#		endif
		}
		unsigned char padded[State_Bytes];
		std::memcpy(padded, src, remain);
		std::memset(padded+remain, padding, padding);
		src = padded;
		for (std::size_t i=0; i<State_Words; ++i, src += Word_Bytes)
		{ state[i] = loaduil(src) ^ ekey[0][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ encuil(state, ekey[i], tmp); }
		encuillast(state, ekey[version.Round_Keys-1], tmp);
		for (std::size_t i=0; i<State_Words; ++i, dst+=Word_Bytes)
#	if AES_UIL32_INTO
		{ storeuil(dst, tmp[i]); }
#	else
		{ storeuil(dst, state[i]); }
#	endif
		return sz + State_Bytes;
	}
	std::size_t Impl_RawUIL32::decrypt_ecb(void *d, const void *s, std::size_t sz) const
	{
		auto *dst = reinterpret_cast<unsigned char*>(d);
		const auto *src = reinterpret_cast<const unsigned char*>(s);
		const auto * const end = dst + sz;
		std::uint_least32_t state[State_Words];
#if AES_UIL32_PASSTMP || AES_UIL32_INTO
		std::uint_least32_t tmp[State_Words];
#endif
		while (dst < end)
		{
			for (std::size_t i=0; i<State_Words; ++i, src += Word_Bytes)
			{ state[i] = loaduil(src) ^ dkey[0][i]; }

			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ decuil(state, dkey[i], tmp); }
			decuillast(state, dkey[version.Round_Keys-1], tmp);
			for (std::size_t i=0; i<State_Words; ++i, dst+=Word_Bytes)
#		if AES_UIL32_INTO
			{ storeuil(dst, tmp[i]); }
#		else
			{ storeuil(dst, state[i]); }
#		endif
		}
		unsigned char pad = *(end-1);
		if (pad == 0 || pad > State_Bytes)
		{
			throw std::runtime_error(
				"invalid padding " + std::to_string(static_cast<int>(pad)));
		}
		return sz - pad;
	}
	std::size_t Impl_RawUIL32::encrypt_cbc(void *d, const void *s, std::size_t sz) const
	{
		if (sz < State_Bytes)
		{ return encrypt_ecb(d, s, sz); }

		auto *dst = reinterpret_cast<unsigned char*>(d);
		const auto *src = reinterpret_cast<const unsigned char*>(s);
		std::size_t remain = sz % State_Bytes;
		sz -= remain;
		unsigned char padding = static_cast<unsigned char>(State_Bytes - remain);
		const auto * const end = src + sz;

#	if AES_UIL32_INTO
		std::uint_least32_t pre[State_Words];
		std::uint_least32_t post[State_Words];

		for (std::size_t i=0; i<State_Words; ++i, src += Word_Bytes)
		{ post[i] = loaduil(src) ^ ekey[0][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ encuil(post, ekey[i], pre); }
		encuillast(post, ekey[version.Round_Keys-1], pre);
		for (std::size_t i=0; i<State_Words; ++i, dst+=Word_Bytes)
		{ storeuil(dst, pre[i]); }
		while (src < end)
		{
			for (std::size_t i=0; i<State_Words; ++i, src += Word_Bytes)
			{ post[i] = loaduil(src) ^ ekey[0][i] ^ pre[i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ encuil(post, ekey[i], pre); }
			encuillast(post, ekey[version.Round_Keys-1], pre);
			for (std::size_t i=0; i<State_Words; ++i, dst+=Word_Bytes)
			{ storeuil(dst, pre[i]); }
		};
		unsigned char padded[State_Bytes];
		std::memcpy(padded, src, remain);
		std::memset(padded+remain, padding, padding);
		src = padded;
		for (std::size_t i=0; i<State_Words; ++i, src += Word_Bytes)
		{ post[i] = loaduil(src) ^ ekey[0][i] ^ pre[i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ encuil(post, ekey[i], pre); }
		encuillast(post, ekey[version.Round_Keys-1], pre);
		for (std::size_t i=0; i<State_Words; ++i, dst+=Word_Bytes)
		{ storeuil(dst, pre[i]); }
#	else
		bool pre = 0;
		std::uint_least32_t state[2][State_Words];

		for (std::size_t i=0; i<State_Words; ++i, src += Word_Bytes)
		{ state[pre][i] = loaduil(src) ^ ekey[0][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ encuil(state[pre], ekey[i], state[!pre]); }
		encuillast(state[pre], ekey[version.Round_Keys-1], state[!pre]);
		for (std::size_t i=0; i<State_Words; ++i, dst+=Word_Bytes)
		{ storeuil(dst, state[pre][i]); }
		while (src < end)
		{
			pre = !pre;
			for (std::size_t i=0; i<State_Words; ++i, src += Word_Bytes)
			{ state[pre][i] = loaduil(src) ^ ekey[0][i] ^ state[!pre][i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ encuil(state[pre], ekey[i], state[!pre]); }
			encuillast(state[pre], ekey[version.Round_Keys-1], state[!pre]);
			for (std::size_t i=0; i<State_Words; ++i, dst+=Word_Bytes)
			{ storeuil(dst, state[pre][i]); }
		};
		unsigned char padded[State_Bytes];
		std::memcpy(padded, src, remain);
		std::memset(padded+remain, padding, padding);
		src = padded;
		pre = !pre;
		for (std::size_t i=0; i<State_Words; ++i, src += Word_Bytes)
		{ state[pre][i] = loaduil(src) ^ ekey[0][i] ^ state[!pre][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ encuil(state[pre], ekey[i], state[!pre]); }
		encuillast(state[pre], ekey[version.Round_Keys-1], state[!pre]);
		for (std::size_t i=0; i<State_Words; ++i, dst+=Word_Bytes)
		{ storeuil(dst, state[pre][i]); }
#	endif
		return sz + State_Bytes;
	}
	std::size_t Impl_RawUIL32::decrypt_cbc(void *d, const void *s, std::size_t sz) const
	{
		if (sz <= State_Bytes)
		{ return decrypt_ecb(d, s, sz); }

		const auto *src = reinterpret_cast<const unsigned char*>(s);
		const auto * const end = src;
		src += sz-Word_Bytes;
		auto *dst = reinterpret_cast<unsigned char*>(d) + sz - Word_Bytes;

#	if AES_UIL32_INTO
		std::uint_least32_t pre[State_Words];
		std::uint_least32_t post[State_Words];
		int isw = static_cast<int>(State_Words)-1;

		for (int i=isw; i>=0; --i, src-=Word_Bytes)
		{ pre[i] = loaduil(src) ^ dkey[0][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ decuil(pre, dkey[i], post); }
		decuillast(pre, dkey[version.Round_Keys-1], post);
		while (src > end)
		{
			for (int i=isw; i>=0; --i, src-=Word_Bytes)
			{ pre[i] = loaduil(src); }

			for (int i=isw; i>=0; --i, dst-=Word_Bytes)
			{ storeuil(dst, pre[i] ^ post[i]); }

			for (std::size_t i=0; i<State_Words; ++i)
			{ pre[i] ^= dkey[0][i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ decuil(pre, dkey[i], post); }
			decuillast(pre, dkey[version.Round_Keys-1], post);
		}
		for (int i=isw; i>=0; --i, dst-=Word_Bytes)
		{ storeuil(dst, post[i]); }
#	else
		bool post = 0;
		std::uint_least32_t state[2][State_Words];
		int isw = static_cast<int>(State_Words)-1;

		for (int i=isw; i>=0; --i, src-=Word_Bytes)
		{ state[post][i] = loaduil(src) ^ dkey[0][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ decuil(state[post], dkey[i], state[!post]); }
		decuillast(state[post], dkey[version.Round_Keys-1], state[!post]);
		while (src > end)
		{
			post = !post;
			for (int i=isw; i>=0; --i, src-=Word_Bytes)
			{ state[post][i] = loaduil(src); }

			for (int i=isw; i>=0; --i, dst-=Word_Bytes)
			{ storeuil(dst, state[!post][i] ^ state[post][i]); }

			for (std::size_t i=0; i<State_Words; ++i)
			{ state[post][i] ^= dkey[0][i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ decuil(state[post], dkey[i], state[!post]); }
			decuillast(state[post], dkey[version.Round_Keys-1], state[!post]);
		}
		for (int i=isw; i>=0; --i, dst-=Word_Bytes)
		{ storeuil(dst, state[post][i]); }
#	endif
		unsigned char pad = *(dst + Word_Bytes + sz - 1);
		if (pad == 0 || pad > State_Bytes)
		{
			throw std::runtime_error(
				"invalid padding " + std::to_string(static_cast<int>(pad)));
		}
		return sz - pad;
	}

#if defined(UINT32_MAX) && CHAR_BIT==8

	namespace
	{
		static_assert(sizeof(std::uint32_t) == 4, "uint32_t not 4 bytes");
		struct colmix
		{
			std::uint32_t data[256];
			colmix(
				unsigned char col, 
				aes::word (*func)(unsigned char, unsigned char),
				const unsigned char *box)
			{
				for (std::size_t i=0; i<256; ++i)
				{
					auto w = func(box[i], col);
					std::memcpy(data+i, w.data, Word_Bytes);
				}
			}
			operator const std::uint32_t*() const { return data; }
		};
		colmix colui[State_Words] = {
			{0, mixcol, SBox()}, {1, mixcol, SBox()}, {2, mixcol, SBox()}, {3, mixcol, SBox()} };
		colmix icolui[State_Words] = {
			{0, imixcol, IBox()}, {1, imixcol, IBox()}, {2, imixcol, IBox()}, {3, imixcol, IBox()} };

		std::uint32_t rconui(std::size_t i)
		{
			std::uint32_t ret = 0;
			reinterpret_cast<unsigned char*>(&ret)[0] = aes::rcon(i);
			return ret;
		}
		std::uint32_t subui(std::uint32_t w)
		{
			const unsigned char *b = SBox();
			auto *bw = reinterpret_cast<unsigned char*>(&w);
			for (std::size_t i=0; i<Word_Bytes; ++i)
			{ bw[i] = b[bw[i]]; }
			return w;
		}
		std::uint32_t rsubui(std::uint32_t w)
		{
			const unsigned char *b = SBox();
			auto *bw = reinterpret_cast<unsigned char*>(&w);
			unsigned char tmp = b[bw[0]];
			for (std::size_t i=1; i<Word_Bytes; ++i)
			{ bw[i-1] = b[bw[i]]; }
			bw[3] = tmp;
			return w;
		}
#if AES_UI32_PASSTMP
		void encui(std::uint32_t *data, const std::uint32_t *key, std::uint32_t *tmp)
		{
			unsigned char *b = reinterpret_cast<unsigned char*>(data);
#		if AES_UI32_BYTEFIRST
			tmp[0] = colui[0][b[0]];
			tmp[1] = colui[0][b[4]];
			tmp[2] = colui[0][b[8]];
			tmp[3] = colui[0][b[12]];
			tmp[0] ^= colui[1][b[5]];
			tmp[1] ^= colui[1][b[9]];
			tmp[2] ^= colui[1][b[13]];
			tmp[3] ^= colui[1][b[1]];
			tmp[0] ^= colui[2][b[10]];
			tmp[1] ^= colui[2][b[14]];
			tmp[2] ^= colui[2][b[2]];
			tmp[3] ^= colui[2][b[6]];
			tmp[0] ^= colui[3][b[15]];
			tmp[1] ^= colui[3][b[3]];
			tmp[2] ^= colui[3][b[7]];
			tmp[3] ^= colui[3][b[11]];
#else
			tmp[0] = colui[0][b[0]] ^ colui[1][b[5]] ^ colui[2][b[10]] ^ colui[3][b[15]];
			tmp[1] = colui[0][b[4]] ^ colui[1][b[9]] ^ colui[2][b[14]] ^ colui[3][b[3]];
			tmp[2] = colui[0][b[8]] ^ colui[1][b[13]] ^ colui[2][b[2]] ^ colui[3][b[7]];
			tmp[3] = colui[0][b[12]] ^ colui[1][b[1]] ^ colui[2][b[6]] ^ colui[3][b[11]];
#endif
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] = tmp[i] ^ key[i]; }
		}
		void encuilast(std::uint32_t *data, const std::uint32_t *key, std::uint32_t *tmp)
		{
			const auto *b = SBox();
			auto *bd = reinterpret_cast<unsigned char*>(data);
			auto *tp = reinterpret_cast<unsigned char*>(tmp);
			tp[0] = b[bd[0]];
			tp[1] = b[bd[5]];
			tp[2] = b[bd[10]];
			tp[3] = b[bd[15]];
			tp[4] = b[bd[4]];
			tp[5] = b[bd[9]];
			tp[6] = b[bd[14]];
			tp[7] = b[bd[3]];
			tp[8] = b[bd[8]];
			tp[9] = b[bd[13]];
			tp[10] = b[bd[2]];
			tp[11] = b[bd[7]];
			tp[12] = b[bd[12]];
			tp[13] = b[bd[1]];;
			tp[14] = b[bd[6]];
			tp[15] = b[bd[11]];
			for (std::size_t i=0; i<State_Words; ++i)
#		if AES_UI32_INTO
			{ tmp[i] ^= key[i]; }
#		else
			{ data[i] = tmp[i] ^ key[i]; }
#		endif
		}
		void decui(std::uint32_t *data, const std::uint32_t *key, std::uint32_t *tmp)
		{
			unsigned char *b = reinterpret_cast<unsigned char*>(data);
#		if AES_UI32_BYTEFIRST
			tmp[0] = icolui[0][b[0]];
			tmp[1] = icolui[0][b[4]];
			tmp[2] = icolui[0][b[8]];
			tmp[3] = icolui[0][b[12]];
			tmp[0] ^= icolui[1][b[13]];
			tmp[1] ^= icolui[1][b[1]];
			tmp[2] ^= icolui[1][b[5]];
			tmp[3] ^= icolui[1][b[9]];
			tmp[0] ^= icolui[2][b[10]];
			tmp[1] ^= icolui[2][b[14]];
			tmp[2] ^= icolui[2][b[2]];
			tmp[3] ^= icolui[2][b[6]];
			tmp[0] ^= icolui[3][b[7]];
			tmp[1] ^= icolui[3][b[11]];
			tmp[2] ^= icolui[3][b[15]];
			tmp[3] ^= icolui[3][b[3]];
#else
			tmp[0] = icolui[0][b[0]] ^ icolui[1][b[13]] ^ icolui[2][b[10]] ^ icolui[3][b[7]];
			tmp[1] = icolui[0][b[4]] ^ icolui[1][b[1]] ^ icolui[2][b[14]] ^ icolui[3][b[11]];
			tmp[2] = icolui[0][b[8]] ^ icolui[1][b[5]] ^ icolui[2][b[2]] ^ icolui[3][b[15]];
			tmp[3] = icolui[0][b[12]] ^ icolui[1][b[9]] ^ icolui[2][b[6]] ^ icolui[3][b[3]];
#endif
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] = tmp[i] ^ key[i]; }
		}
		void decuilast(std::uint32_t *data, const std::uint32_t *key, std::uint32_t *tmp)
		{
			const auto *b = IBox();
			auto *bd = reinterpret_cast<unsigned char*>(data);
			auto *tp = reinterpret_cast<unsigned char*>(tmp);
			tp[0] = b[bd[0]];
			tp[1] = b[bd[13]];
			tp[2] = b[bd[10]];
			tp[3] = b[bd[7]];
			tp[4] = b[bd[4]];
			tp[5] = b[bd[1]];
			tp[6] = b[bd[14]];
			tp[7] = b[bd[11]];
			tp[8] = b[bd[8]];
			tp[9] = b[bd[5]];
			tp[10] = b[bd[2]];
			tp[11] = b[bd[15]];
			tp[12] = b[bd[12]];
			tp[13] = b[bd[9]];
			tp[14] = b[bd[6]];
			tp[15] = b[bd[3]];
			for (std::size_t i=0; i<State_Words; ++i)
#		if AES_UI32_INTO
			{ tmp[i] ^= key[i]; }
#		else
			{ data[i] = tmp[i] ^ key[i]; }
#		endif
		}
#else
		void encuit(std::uint32_t *data, const std::uint32_t *key)
		{
			std::uint32_t tmp[State_Words];
			unsigned char *b = reinterpret_cast<unsigned char*>(data);
#		if AES_UI32_BYTEFIRST
			tmp[0] = colui[0][b[0]];
			tmp[1] = colui[0][b[4]];
			tmp[2] = colui[0][b[8]];
			tmp[3] = colui[0][b[12]];
			tmp[0] ^= colui[1][b[5]];
			tmp[1] ^= colui[1][b[9]];
			tmp[2] ^= colui[1][b[13]];
			tmp[3] ^= colui[1][b[1]];
			tmp[0] ^= colui[2][b[10]];
			tmp[1] ^= colui[2][b[14]];
			tmp[2] ^= colui[2][b[2]];
			tmp[3] ^= colui[2][b[6]];
			tmp[0] ^= colui[3][b[15]];
			tmp[1] ^= colui[3][b[3]];
			tmp[2] ^= colui[3][b[7]];
			tmp[3] ^= colui[3][b[11]];
#else
			tmp[0] = colui[0][b[0]] ^ colui[1][b[5]] ^ colui[2][b[10]] ^ colui[3][b[15]];
			tmp[1] = colui[0][b[4]] ^ colui[1][b[9]] ^ colui[2][b[14]] ^ colui[3][b[3]];
			tmp[2] = colui[0][b[8]] ^ colui[1][b[13]] ^ colui[2][b[2]] ^ colui[3][b[7]];
			tmp[3] = colui[0][b[12]] ^ colui[1][b[1]] ^ colui[2][b[6]] ^ colui[3][b[11]];
#endif
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] = tmp[i] ^ key[i]; }
		}
		void decuit(std::uint32_t *data, const std::uint32_t *key)
		{
			std::uint32_t tmp[State_Words];
			unsigned char *b = reinterpret_cast<unsigned char*>(data);
#		if AES_UI32_BYTEFIRST
			tmp[0] = icolui[0][b[0]];
			tmp[1] = icolui[0][b[4]];
			tmp[2] = icolui[0][b[8]];
			tmp[3] = icolui[0][b[12]];
			tmp[0] ^= icolui[1][b[13]];
			tmp[1] ^= icolui[1][b[1]];
			tmp[2] ^= icolui[1][b[5]];
			tmp[3] ^= icolui[1][b[9]];
			tmp[0] ^= icolui[2][b[10]];
			tmp[1] ^= icolui[2][b[14]];
			tmp[2] ^= icolui[2][b[2]];
			tmp[3] ^= icolui[2][b[6]];
			tmp[0] ^= icolui[3][b[7]];
			tmp[1] ^= icolui[3][b[11]];
			tmp[2] ^= icolui[3][b[15]];
			tmp[3] ^= icolui[3][b[3]];
#else
			tmp[0] = icolui[0][b[0]] ^ icolui[1][b[13]] ^ icolui[2][b[10]] ^ icolui[3][b[7]];
			tmp[1] = icolui[0][b[4]] ^ icolui[1][b[1]] ^ icolui[2][b[14]] ^ icolui[3][b[11]];
			tmp[2] = icolui[0][b[8]] ^ icolui[1][b[5]] ^ icolui[2][b[2]] ^ icolui[3][b[15]];
			tmp[3] = icolui[0][b[12]] ^ icolui[1][b[9]] ^ icolui[2][b[6]] ^ icolui[3][b[3]];
#endif
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] = tmp[i] ^ key[i]; }
		}
#	define encui(a,b,c) encuit((a),(b))
#	define decui(a,b,c) decuit((a),(b))
#	if AES_UI32_INTO
		void encuilast(std::uint32_t *data, const std::uint32_t *key, std::uint32_t *tmp)
		{
			const auto *b = SBox();
			auto *bd = reinterpret_cast<unsigned char*>(data);
			auto *tp = reinterpret_cast<unsigned char*>(tmp);
			tp[0] = b[bd[0]];
			tp[1] = b[bd[5]];
			tp[2] = b[bd[10]];
			tp[3] = b[bd[15]];
			tp[4] = b[bd[4]];
			tp[5] = b[bd[9]];
			tp[6] = b[bd[14]];
			tp[7] = b[bd[3]];
			tp[8] = b[bd[8]];
			tp[9] = b[bd[13]];
			tp[10] = b[bd[2]];
			tp[11] = b[bd[7]];
			tp[12] = b[bd[12]];
			tp[13] = b[bd[1]];;
			tp[14] = b[bd[6]];
			tp[15] = b[bd[11]];
			for (std::size_t i=0; i<State_Words; ++i)
			{ tmp[i] ^= key[i]; }
		}
		void decuilast(std::uint32_t *data, const std::uint32_t *key, std::uint32_t *tmp)
		{
			const auto *b = IBox();
			auto *bd = reinterpret_cast<unsigned char*>(data);
			auto *tp = reinterpret_cast<unsigned char*>(tmp);
			tp[0] = b[bd[0]];
			tp[1] = b[bd[13]];
			tp[2] = b[bd[10]];
			tp[3] = b[bd[7]];
			tp[4] = b[bd[4]];
			tp[5] = b[bd[1]];
			tp[6] = b[bd[14]];
			tp[7] = b[bd[11]];
			tp[8] = b[bd[8]];
			tp[9] = b[bd[5]];
			tp[10] = b[bd[2]];
			tp[11] = b[bd[15]];
			tp[12] = b[bd[12]];
			tp[13] = b[bd[9]];
			tp[14] = b[bd[6]];
			tp[15] = b[bd[3]];
			for (std::size_t i=0; i<State_Words; ++i)
			{ tmp[i] ^= key[i]; }
		}
#	else
		void encuilastt(std::uint32_t *data, const std::uint32_t *key)
		{
			const auto *b = SBox();
			auto *bd = reinterpret_cast<unsigned char*>(data);
			bd[0] = b[bd[0]];
			bd[4] = b[bd[4]];
			bd[8] = b[bd[8]];
			bd[12] = b[bd[12]];
			unsigned char tmp = b[bd[1]];
			bd[1] = b[bd[5]];
			bd[5] = b[bd[9]];
			bd[9] = b[bd[13]];
			bd[13] = tmp;
			tmp = b[bd[2]];
			bd[2] = b[bd[10]];
			bd[10] = tmp;
			tmp = b[bd[6]];
			bd[6] = b[bd[14]];
			bd[14] = tmp;
			tmp = b[bd[3]];
			bd[3] = b[bd[15]];
			bd[15] = b[bd[11]];
			bd[11] = b[bd[7]];
			bd[7] = tmp;
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] ^= key[i]; }
		}
		void decuilastt(std::uint32_t *data, const std::uint32_t *key)
		{
			const auto *b = IBox();
			auto *bd = reinterpret_cast<unsigned char*>(data);
			bd[0] = b[bd[0]];
			bd[4] = b[bd[4]];
			bd[8] = b[bd[8]];
			bd[12] = b[bd[12]];
			unsigned char tmp = b[bd[1]];
			bd[1] = b[bd[13]];
			bd[13] = b[bd[9]];
			bd[9] = b[bd[5]];
			bd[5] = tmp;
			tmp = b[bd[2]];
			bd[2] = b[bd[10]];
			bd[10] = tmp;
			tmp = b[bd[6]];
			bd[6] = b[bd[14]];
			bd[14] = tmp;
			tmp = b[bd[3]];
			bd[3] = b[bd[7]];
			bd[7] = b[bd[11]];
			bd[11] = b[bd[15]];
			bd[15] = tmp;
			for (std::size_t i=0; i<State_Words; ++i)
			{ data[i] ^= key[i]; }
		}
#	define encuilast(a,b,c) encuilastt((a),(b))
#	define decuilast(a,b,c) decuilastt((a),(b))
#	endif
#endif
	}
	const std::uint32_t* const * submixui()
	{
		static std::uint32_t* parray[State_Words] = {
			colui[0].data, colui[1].data, colui[2].data, colui[3].data};
		return parray;
	}
	const std::uint32_t* const * isubmixui()
	{
		static std::uint32_t* parray[State_Words] = {
			icolui[0].data, icolui[1].data, icolui[2].data, icolui[3].data};
		return parray;
	}


	Impl_RawUI32::Impl_RawUI32(const void *k, const Version &v):
		version(v)
	{
		std::memcpy(ekey, k, version.Key_Bytes);
		const std::size_t N = version.Key_Bytes / Word_Bytes;
		const std::size_t R = version.Round_Keys;
		for (std::size_t i=N; i<R*State_Words; ++i)
		{
			if (!(i%N))
			{ getkw(ekey, i) = getkw(ekey, i-N) ^ rconui(i/N) ^ rsubui(getkw(ekey, i-1)); }
			else if (N>6 && i%N == 4)
			{ getkw(ekey, i) = subui(getkw(ekey, i-1)) ^ getkw(ekey, i-N); }
			else
			{ getkw(ekey, i) = getkw(ekey, i-1) ^ getkw(ekey, i-N); }
		}
		for (std::size_t i=0; i<version.Round_Keys; ++i)
		{ std::memcpy(dkey[version.Round_Keys-(i+1)], ekey[i], sizeof(ekey[0])); }
		const auto *b = SBox();
		for (std::size_t sidx=1; sidx<version.Round_Keys-1; ++sidx)
		{
			std::uint_least32_t (&state)[State_Words] = dkey[sidx];
			for (std::size_t widx=0; widx<State_Words; ++widx)
			{
				auto *wb = reinterpret_cast<unsigned char*>(state+widx);
				state[widx] = icolui[0][b[wb[0]]] ^ icolui[1][b[wb[1]]]
					^ icolui[2][b[wb[2]]] ^ icolui[3][b[wb[3]]];
			}
		}
	}
	void Impl_RawUI32::storekey(void *d, const word (&key)[Max_Rounds][State_Words]) const
	{
		auto *dst = reinterpret_cast<unsigned char*>(d);
		for (std::size_t i=0; i<version.Round_Keys; ++i, dst += State_Bytes)
		{ std::memcpy(dst, key[i], State_Bytes); }
	}
	std::size_t Impl_RawUI32::encrypt_ecb(void *d, const void *s, std::size_t sz) const
	{
		auto *dst = reinterpret_cast<unsigned char*>(d);
		const auto *src = reinterpret_cast<const unsigned char*>(s);

		std::size_t remain = sz % State_Bytes;
		sz-=remain;
		unsigned char padding = static_cast<unsigned char>(State_Bytes - remain);
		const auto * const end = src + sz;
		std::uint32_t state[State_Words];
#if AES_UI32_PASSTMP || AES_UI32_INTO
		std::uint32_t tmp[State_Words];
#endif
		for (; src < end; src += State_Bytes, dst += State_Bytes)
		{
			std::memcpy(state, src, sizeof(state));
			for (std::size_t i=0; i<State_Words; ++i)
			{ state[i] ^= ekey[0][i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ encui(state, ekey[i], tmp); }
			encuilast(state, ekey[version.Round_Keys-1], tmp);
			encuilast(state, ekey[version.Round_Keys-1], tmp);
#		if AES_UI32_INTO
			std::memcpy(dst, tmp, sizeof(state));
#		else
			std::memcpy(dst, state, sizeof(state));
#		endif
		}
		std::memcpy(state, src, remain);
		std::memset(reinterpret_cast<char*>(state)+remain, padding, padding);
		for (std::size_t i=0; i<State_Words; ++i)
		{ state[i] ^= ekey[0][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ encui(state, ekey[i], tmp); }
		encuilast(state, ekey[version.Round_Keys-1], tmp);
#	if AES_UI32_INTO
		std::memcpy(dst, tmp, sizeof(state));
#	else
		std::memcpy(dst, state, sizeof(state));
#	endif
		return sz + State_Bytes;
	}
	std::size_t Impl_RawUI32::decrypt_ecb(void *d, const void *s, std::size_t sz) const
	{
		auto *dst = reinterpret_cast<unsigned char*>(d);
		const auto *src = reinterpret_cast<const unsigned char*>(s);
		const auto * const end = dst + sz;
		std::uint32_t state[State_Words];
#if AES_UI32_PASSTMP || AES_UI32_INTO
		std::uint32_t tmp[State_Words];
#endif
		for (; dst < end; src += State_Bytes, dst += State_Bytes)
		{
			std::memcpy(state, src, sizeof(state));
			for (std::size_t i=0; i<State_Words; ++i)
			{ state[i] ^= dkey[0][i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ decui(state, dkey[i], tmp); }
			decuilast(state, dkey[version.Round_Keys-1], tmp);
#		if AES_UI32_INTO
			std::memcpy(dst, tmp, sizeof(state));
#		else
			std::memcpy(dst, state, sizeof(state));
#		endif
		}
		unsigned char pad = *(end-1);
		if (pad == 0 || pad > State_Bytes)
		{
			throw std::runtime_error(
				"invalid padding " + std::to_string(static_cast<int>(pad)));
		}
		return sz - pad;
	}
	std::size_t Impl_RawUI32::encrypt_cbc(void *d, const void *s, std::size_t sz) const
	{
		if (sz < State_Bytes)
		{ return encrypt_ecb(d, s, sz); }

		auto *dst = reinterpret_cast<unsigned char*>(d);
		const auto *src = reinterpret_cast<const unsigned char*>(s);
		std::size_t remain = sz % State_Bytes;
		sz -= remain;
		unsigned char padding = static_cast<unsigned char>(State_Bytes - remain);
		const auto * const end = src + sz;

#	if AES_UI32_INTO
		std::uint32_t post[State_Words];
		std::uint32_t pre[State_Words];

		std::memcpy(post, src, sizeof(post));
		for (std::size_t i=0; i<State_Words; ++i)
		{ post[i] ^= ekey[0][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ encui(post, ekey[i], pre); }
		encuilast(post, ekey[version.Round_Keys-1], pre);
		std::memcpy(dst, pre, sizeof(pre));
		src += State_Bytes;
		dst += State_Bytes;
		for(; src < end; src += State_Bytes, dst += State_Bytes)
		{
			std::memcpy(post, src, sizeof(post));
			for (std::size_t i=0; i<State_Words; ++i)
			{ post[i] ^= ekey[0][i] ^ pre[i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ encui(post, ekey[i], pre); }
			encuilast(post, ekey[version.Round_Keys-1], pre);
			std::memcpy(dst, pre, sizeof(pre));
		};
		std::memcpy(post, src, remain);
		std::memset(reinterpret_cast<char*>(post)+remain, padding, padding);
		for (std::size_t i=0; i<State_Words; ++i)
		{ post[i] ^= ekey[0][i] ^ pre[i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ encui(post, ekey[i], pre); }
		encuilast(post, ekey[version.Round_Keys-1], pre);
		std::memcpy(dst, pre, sizeof(pre));
#else
		bool pre = 0;
		std::uint32_t state[2][State_Words];

		std::memcpy(state[pre], src, sizeof(state[pre]));
		for (std::size_t i=0; i<State_Words; ++i)
		{ state[pre][i] ^= ekey[0][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ encui(state[pre], ekey[i], state[!pre]); }
		encuilast(state[pre], ekey[version.Round_Keys-1], state[!pre]);
		std::memcpy(dst, state[pre], sizeof(state[pre]));
		src += State_Bytes;
		dst += State_Bytes;
		for(; src < end; src += State_Bytes, dst += State_Bytes)
		{
			pre = !pre;
			std::memcpy(state[pre], src, sizeof(state[pre]));
			for (std::size_t i=0; i<State_Words; ++i)
			{ state[pre][i] ^= ekey[0][i] ^ state[!pre][i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ encui(state[pre], ekey[i], state[!pre]); }
			encuilast(state[pre], ekey[version.Round_Keys-1], state[!pre]);
			std::memcpy(dst, state[pre], sizeof(state[pre]));
		};
		pre = !pre;
		std::memcpy(state[pre], src, remain);
		std::memset(reinterpret_cast<char*>(state[pre])+remain, padding, padding);
		for (std::size_t i=0; i<State_Words; ++i)
		{ state[pre][i] ^= ekey[0][i] ^ state[!pre][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ encui(state[pre], ekey[i], state[!pre]); }
		encuilast(state[pre], ekey[version.Round_Keys-1], state[!pre]);
		std::memcpy(dst, state[pre], sizeof(state[pre]));
#endif
		return sz + State_Bytes;
	}
	std::size_t Impl_RawUI32::decrypt_cbc(void *d, const void *s, std::size_t sz) const
	{
		if (sz <= State_Bytes)
		{ return decrypt_ecb(d, s, sz); }

		const auto *src = reinterpret_cast<const unsigned char*>(s);
		const auto * const end = src;
		src += sz-State_Bytes;
		auto *dst = reinterpret_cast<unsigned char*>(d) + sz - State_Bytes;

#	if AES_UI32_INTO
		std::uint32_t pre[State_Words];
		std::uint32_t post[State_Words];

		std::memcpy(pre, src, sizeof(pre));
		for (std::size_t i=0; i<State_Words; ++i)
		{ pre[i] ^= dkey[0][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ decui(pre, dkey[i], post); }
		decuilast(pre, dkey[version.Round_Keys-1], post);
		while (src > end)
		{
			src -= State_Bytes;
			std::memcpy(pre, src, sizeof(pre));
			for (std::size_t i=0; i<State_Words; ++i)
			{ post[i] ^= pre[i]; }
			std::memcpy(dst, post, sizeof(post));
			dst -= State_Bytes;

			for (std::size_t i=0; i<State_Words; ++i)
			{ pre[i] ^= dkey[0][i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ decui(pre, dkey[i], post); }
			decuilast(pre, dkey[version.Round_Keys-1], post);
		}
		std::memcpy(dst, post, sizeof(post));
#	else
		bool post = 0;
		std::uint32_t state[2][State_Words];

		std::memcpy(state[post], src, sizeof(state[post]));
		for (std::size_t i=0; i<State_Words; ++i)
		{ state[post][i] ^= dkey[0][i]; }
		for (std::size_t i=1; i<version.Round_Keys-1; ++i)
		{ decui(state[post], dkey[i], state[!post]); }
		decuilast(state[post], dkey[version.Round_Keys-1], state[!post]);
		while (src > end)
		{
			src -= State_Bytes;
			post = !post;
			std::memcpy(state[post], src, sizeof(state[post]));
			for (std::size_t i=0; i<State_Words; ++i)
			{ state[!post][i] ^= state[post][i]; }
			std::memcpy(dst, state[!post], sizeof(state[!post]));
			dst -= State_Bytes;

			for (std::size_t i=0; i<State_Words; ++i)
			{ state[post][i] ^= dkey[0][i]; }
			for (std::size_t i=1; i<version.Round_Keys-1; ++i)
			{ decui(state[post], dkey[i], state[!post]); }
			decuilast(state[post], dkey[version.Round_Keys-1], state[!post]);
		}
		std::memcpy(dst, state[post], sizeof(state[post]));
#	endif

		unsigned char pad = *(dst + sz - 1);
		if (pad == 0 || pad > State_Bytes)
		{
			throw std::runtime_error(
				"invalid padding " + std::to_string(static_cast<int>(pad)));
		}
		return sz - pad;
	}
#endif
}
