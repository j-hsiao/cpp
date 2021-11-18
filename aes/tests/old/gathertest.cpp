#include <algorithm>
#include <wmmintrin.h>
#include <immintrin.h>
#include <iostream>
#undef NDEBUG
#include <cassert>


__m128i rowshift()
{
	static unsigned char ShiftRows[] =
	{0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11};
	static __m128i shifts = _mm_loadu_si128(
		reinterpret_cast<const __m128i*>(ShiftRows));
	return shifts;
}
__m128i i_rowshift()
{
	static unsigned char IShiftRows[] = 
	{0, 13, 10, 7, 4, 1, 14, 11, 8, 5, 2, 15, 12, 9, 6, 3};
	static __m128i shifts = _mm_loadu_si128(
		reinterpret_cast<const __m128i*>(IShiftRows));
	return shifts;
}

void pblock(const unsigned char *dat)
{
	const unsigned char *end = dat + 16;
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 0; i < 4; ++i)
		{
			std::cout << static_cast<int>(dat[(i * 4) + j]) << ", ";
		}
		std::cout << std::endl;
	}
}

void pblock(const int *dat)
{
	for (int j = 0; j < 4; ++j)
	{
		std::cout << static_cast<int>(dat[j]) << ", ";
	}
	std::cout << std::endl;
}

__m128i lo8mask()
{
	int lo[4] = {0xF, 0xF, 0xF, 0xF};
	static __m128i ret = _mm_loadu_si128(reinterpret_cast<__m128i*>(lo));
	return ret;
}
__m128i nthbytes(__m128i val, int n)
{ return _mm_and_si128(_mm_srli_epi32(val, n * 8), lo8mask()); }

int main(int argc, char *argv[])
{
	const unsigned char data[16] = {
		0, 1, 2, 3,
		4, 5, 6, 7,
		8, 9, 10, 11,
		12, 13, 14, 15
	};
	{
		int idata[4];
		int expected[4][4] =
		{
			{ 0, 4, 8, 12 },
			{ 1, 5, 9, 13 },
			{ 2, 6, 10, 14},
			{ 3, 7, 11, 15}
		};

		int mapping[] = {
			100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115
		};

		int expectmap[4][4] =
		{
			{ 100, 104, 108, 112 },
			{ 101, 105, 109, 113 },
			{ 102, 106, 110, 114},
			{ 103, 107, 111, 115}
		};

		pblock(data);

		__m128i buf = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));

		for (int i = 0; i < 4; ++i)
		{
			__m128i pick = nthbytes(buf, i);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(idata), pick);
			assert(std::equal(idata, idata + 4, expected[i]));
			std::cout << "pick " << i << ": ";
			pblock(idata);

			static_assert(sizeof(int) == 4, "size of int is not 4!");
			__m128i gathered = _mm_i32gather_epi32(mapping, pick, 4);
			_mm_storeu_si128(reinterpret_cast<__m128i*>(idata), gathered);
			std::cout << "mapped " << i << ": ";
			pblock(idata);
			assert(std::equal(idata, idata + 4, expectmap[i]));
		}
	}
	{
		unsigned char buf[16];
		const unsigned char shifted[16] = {
			0, 5, 10, 15,
			4, 9, 14, 3,
			8, 13, 2, 7,
			12, 1, 6, 11};

		__m128i shuffled = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data));
		shuffled = _mm_shuffle_epi8(shuffled, rowshift());
		_mm_storeu_si128(reinterpret_cast<__m128i*>(buf), shuffled);
		pblock(buf);
		assert(std::equal(shifted, shifted + 16, buf));
		shuffled = _mm_shuffle_epi8(shuffled, i_rowshift());
		_mm_storeu_si128(reinterpret_cast<__m128i*>(buf), shuffled);
		pblock(buf);
		assert(std::equal(data, data + 16, buf));





	}
	std::cout << "pass" << std::endl;
}
