#include <sysinfo/sysinfo.h>
#include <iostream>

int main(int argc, char *argv[])
{
#if SYSINFO_ENDIAN == SYSINFO_LITTLE_ENDIAN
	std::cerr << "little endian" << std::endl;
#elif SYSINFO_ENDIAN == SYSINFO_BIG_ENDIAN
	std::cerr << "big endian" << std::endl;
#else
	std::cerr << "unknown endian" << std::endl;
#endif

#if SYSINFO_IREP == SYSINFO_TWO_IREP
	std::cerr << "ints are two's complement" << std::endl;
#elif SYSINFO_IREP == SYSINFO_SMAG_IREP
	std::cerr << "ints are two's sign/magnitude" << std::endl;
#elif SYSINFO_IREP == SYSINFO_ONE_IREP
	std::cerr << "ints are two's one's complement" << std::endl;
#else
	std::cerr << "ints are two's something freaky" << std::endl;
#endif
	
	if (SYSINFO_32_FPCP)
	{
		std::cerr << "can copy float "
			<< (SYSINFO_32_FPCP == SYSINFO_COPY_FPCP ? "directly" : "reversed") << std::endl;
	}

	if (SYSINFO_64_FPCP)
	{
		std::cerr << "can copy double "
			<< (SYSINFO_64_FPCP == SYSINFO_COPY_FPCP ? "directly" : "reversed") << std::endl;
	}
}
