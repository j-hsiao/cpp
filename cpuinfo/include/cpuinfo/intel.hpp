//https://en.wikipedia.org/wiki/CPUID

#ifndef CPUINFO_INTEL_H
#define CPUINFO_INTEL_H
#include <stdexcept>
#include <map>
#include <string>

namespace cpuinfo
{
	//flags: 8: 0-3 = cpuid(0), 4-7 = cpuid(1)
	inline bool check(const std::string &s, unsigned int *flags)
	{
		//put this inside because
		//it was not being initialized before this function was called
		static const std::map<std::string, unsigned int> offset = {
			{"mmx", 3},
			{"sse", 3},
			{"sse2", 3},

			{"aes", 2},
			{"avx", 2},
			{"sse3", 2},
			{"ssse3", 2},
			{"sse4.1", 2},
			{"sse4.2", 2},
			{"fma", 2},

			{"avx2", 5},
			{"avx512_f", 5},
			{"avx512_dq", 5},
			{"avx512_ifma", 5},
			{"avx512_pf", 5},
			{"avx512_er", 5},
			{"avx512_cd", 5},
			{"avx512_bw", 5},
			{"avx512_vl", 5},

			{"avx512_vbmi", 6},
			{"avx512_vbmi2", 6},
			{"vaes", 6},
			{"avx512_vnni", 6},
			{"avx512_bitalg", 6},
			{"avx512_vpopcntdq", 6},

			{"avx512_4vnniw", 7},
			{"avx512_4fmaps", 7},
			{"avx512_vp2intersect", 7},
			{"amx_bf16", 7},
			{"amx_tile", 7},
			{"amx_int8", 7},
		};
		static const std::map<std::string, unsigned int> shift = {
			{"mmx", 23},
			{"sse", 25},
			{"sse2", 26},

			{"sse3", 0},
			{"ssse3", 9},
			{"fma", 12},
			{"sse4.1", 19},
			{"sse4.2", 20},
			{"aes", 25},
			{"avx", 28},

			{"avx2", 5},
			{"avx512_f", 16},
			{"avx512_dq", 17},
			{"avx512_ifma", 21},
			{"avx512_pf", 26},
			{"avx512_er", 27},
			{"avx512_cd", 28},
			{"avx512_bw", 30},
			{"avx512_vl", 31},

			{"avx512_vbmi", 1},
			{"avx512_vbmi2", 6},
			{"vaes", 9},
			{"avx512_vnni", 11},
			{"avx512_bitalg", 12},
			{"avx512_vpopcntdq", 14},

			{"avx512_4vnniw", 2},
			{"avx512_4fmaps", 3},
			{"avx512_vp2intersect", 8},
			{"amx_bf16", 22},
			{"amx_tile", 24},
			{"amx_int8", 25},
		};

		try
		{
			return flags[offset.at(s)] & (1 << shift.at(s));
		}
		catch (std::out_of_range&)
		{}
		return 0;
	}
}
#endif
