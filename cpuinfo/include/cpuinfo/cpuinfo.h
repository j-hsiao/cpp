#ifndef CPUINFO_H
#define CPUINFO_H

#ifndef CPUINFO_API

#ifdef _WIN32

#ifdef cpuinfo_EXPORTS
#define CPUINFO_API __declspec(dllexport)
#else
#define CPUINFO_API __declspec(dllimport)
#endif

#else
#define CPUINFO_API
#endif

#endif

extern "C"
{
	CPUINFO_API bool cpuinfo_has(const char *feature);
}
#endif
