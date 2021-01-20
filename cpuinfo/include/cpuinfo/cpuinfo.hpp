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

namespace cpuinfo
{
	CPUINFO_API bool has(const char*);
	template<class T>
	bool has(const T& s) { return has(s.c_str()); }
}
#endif
