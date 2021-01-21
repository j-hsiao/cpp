#ifndef CPUINFO_HPP
#define CPUINFO_HPP
#include <cpuinfo/cpuinfo.h>
#include <string>

namespace cpuinfo
{
	inline CPUINFO_API bool has(const std::string &feature)
	{
		return ::cpuinfo_has(feature.c_str());
	}
}
#endif
