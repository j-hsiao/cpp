#ifndef CPUINFO_H
#define CPUINFO_H

#include <cpuinfo/cpuinfo_dllcfg.h>

CPP_EXTERNC_BEGIN
	CPUINFO_API bool cpuinfo_has(const char *feature);
CPP_EXTERNC_END
#endif
