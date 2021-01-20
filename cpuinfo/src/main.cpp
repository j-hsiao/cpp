#include <iostream>
#include <cpuinfo/cpuinfo.hpp>
#include <vector>
#include <string>

int main()
{
	std::vector<std::string> features = 
	{
		"mmx",
		"sse",
		"sse2",
		"aes",
		"avx",
		"sse3",
		"ssse3",
		"sse4.1",
		"sse4.2",
		"fma",
		"avx2",
		"avx512_f",
		"avx512_dq",
		"avx512_ifma",
		"avx512_pf",
		"avx512_er",
		"avx512_cd",
		"avx512_bw",
		"avx512_vl",
		"avx512_vbmi",
		"avx512_vbmi2",
		"vaes",
		"avx512_vnni",
		"avx512_bitalg",
		"avx512_vpopcntdq",
		"avx512_4vnniw",
		"avx512_4fmaps",
		"avx512_vp2intersect",
		"amx_bf16",
		"amx_tile",
		"amx_int8",
		"some unsupported string"
	};
	for (const auto &feature : features)
	{
		std::cout << feature << ": " << scpuinfo::has(feature) << std::endl;
	}
}
