#include <cpuinfo/cpuinfo.hpp>
#include <tlog/tlog.hpp>

#include <string>

#if defined(_M_AMD64) || defined(_M_I86)  || defined(_M_IX86)
#include <.cpuinfo/intel.hpp>
#include <intrin.h>
using namespace cpuinfo;
bool cpuinfo__has(const char *s)
{
	typedef tlog::Log<CPUINFO_DEBUG> logger;

	static bool initialized = 0;
	static unsigned int flags[8] = {0, 0, 0, 0, 7, 0, 0, 0};
	if (!initialized)
	{
		__cpuid(reinterpret_cast<int*>(flags), 0);
		unsigned int lvl = flags[0];
		logger() << "manufacturer ID: \""
			<< std::string(reinterpret_cast<const char*>(flags + 1), sizeof(unsigned int))
			<< std::string(reinterpret_cast<const char*>(flags + 3), sizeof(unsigned int))
			<< std::string(reinterpret_cast<const char*>(flags + 2), sizeof(unsigned int))
			<< '"' << std::endl << "maximum level is " << lvl << std::endl;
		flags[0] = 0;
		flags[1] = 0;
		flags[2] = 0;
		flags[3] = 0;

		if (lvl >= 1)
		{ __cpuid(reinterpret_cast<int*>(flags), 1); }
		if (lvl >= 7)
		{ __cpuidex(reinterpret_cast<int*>(flags + 4), 7, 0); }
		initialized = 1;
	}
	return check(s, flags);
}
#else
bool cpuinfo__has(const char *s) { return 0; }
#endif
