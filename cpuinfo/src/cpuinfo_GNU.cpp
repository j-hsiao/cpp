#include <cpuinfo/cpuinfo.h>
#include <string>

#ifdef CPUINFO_DEBUG
#include <iostream>
#endif


namespace
{
	template<class T>
	void logd(const T &thing, bool nl = 1)
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

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)  || defined(__i386__) || defined(__i386) || defined(i386)
#include <cpuid.h>
#include <cpuinfo/intel.hpp>
using namespace cpuinfo;
bool cpuinfo_has(const char *s)
{
	static bool initialized = 0;
	//                              0  1  2  3  4  5  6  7
	//                              a  b  c  d  a  b  c  d
	static unsigned int flags[8] = {0, 0, 0, 0, 7, 0, 0, 0};
	if (!initialized)
	{
		if (__get_cpuid(0, flags, flags + 1, flags + 2, flags + 3))
		{
			logd("initializing cpuinfo flags");
			unsigned int lvl = flags[0];
			logd("manufacturer ID: \"", 0);
			logd(std::string(reinterpret_cast<const char*>(flags + 1), sizeof(unsigned int)), 0);
			logd(std::string(reinterpret_cast<const char*>(flags + 3), sizeof(unsigned int)), 0);
			logd(std::string(reinterpret_cast<const char*>(flags + 2), sizeof(unsigned int)), 0);
			logd('"');
			logd("maximum level is ", 0);
			logd(lvl);
			flags[0] = 0;
			flags[1] = 0;
			flags[2] = 0;
			flags[3] = 0;

			if (!__get_cpuid(1, flags, flags + 1, flags + 2, flags + 3))
			{
				logd("CPUID(1) failed");
			}
			/*
			if (!__get_cpuid_count(7, 0, flags + 4, flags + 5, flags + 6, flags + 7))
			if (!__cpuidex(flags + 4, 7, 0))
			{
				logd("CPUID_COUNT(7, 0) failed");
			}
			*/
			if (lvl >= 7)
			{
				__cpuid_count(7, 0, flags[4], flags[5], flags[6], flags[7]);
			}
			else
			{
				logd("CPUID(7) not available: max is ");
				logd(lvl);
			}
			initialized = 1;
			for (int i = 0; i < 8; ++i)
			{
				logd(flags[i]);
			}
		}
		else
		{
			logd("intel CPUID(0) failed");
		}
	}
	return check(s, flags);
}

#else
bool cpuinfo_has(const char *s) { return 0; }
#endif
