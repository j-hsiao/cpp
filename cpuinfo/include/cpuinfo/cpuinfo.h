#ifndef CPUINFO_H
#define CPUINFO_H

#include <cpuinfo/cpuinfo_dllcfg.h>

extern "C"
{
	cpuinfo_API bool cpuinfo_has(const char *feature);
}
#endif
