#include <cpuinfo/cpuinfo.hpp>
#include <string>

#ifdef CPUINFO_DEBUG
#include <iostream>
#endif

namespace
{
	template<class T>
	void dprint(const T &thing, bool nl = 1)
	{
#ifdef CPUINFO_DEBUG
		std::cerr << thing;
		if (nl)
		{
			std::cerr << std::endl;
		}
#endif
	}
}

#if defined(_M_AMD64) || defined(_M_I86)  || defined(_M_IX86)
#include <cpuinfo/intel.hpp>
#include <intrin.h>
namespace cpuinfo
{
	bool has(const char *s)
	{
		static bool initialized = 0;
		static unsigned int flags[8] = {0, 0, 0, 0, 7, 0, 0, 0};
		if (!initialized)
		{
			__cpuid(reinterpret_cast<int*>(flags), 0);
			unsigned int lvl = flags[0];
			dprint("manufacturer ID: \"", 0);
			dprint(std::string(reinterpret_cast<const char*>(flags + 1), sizeof(unsigned int)), 0);
			dprint(std::string(reinterpret_cast<const char*>(flags + 3), sizeof(unsigned int)), 0);
			dprint(std::string(reinterpret_cast<const char*>(flags + 2), sizeof(unsigned int)), 0);
			dprint('"');
			dprint("maximum level is ", 0);
			dprint(lvl);
			flags[0] = 0;
			flags[1] = 0;
			flags[2] = 0;
			flags[3] = 0;

			if (lvl >= 1)
			{
				__cpuid(reinterpret_cast<int*>(flags), 1);
			}
			if (lvl >= 7)
			{
				__cpuidex(reinterpret_cast<int*>(flags + 4), 7, 0);
			}
			initialized = 1;
		}
		return check(s, flags);
	}
}
#else
namespace cpuinfo
{
	bool has(const char *s) { return 0; }
}
#endif
