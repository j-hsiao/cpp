#include "os/os.hpp"

#include <cstddef>
#include <iostream>

#ifdef _WIN32
#define FORCE_COMPAT TESTING_DLL
#else
#define FORCE_COMPAT 0
#endif
int main(int argc, char *argv[])
{
	bool compat = 0;
	for (int i = 1; i < argc; ++i)
	{
		if (argv[i] == std::string("-c"))
		{ compat = !compat; continue; }
		try
		{
			if (compat || FORCE_COMPAT)
			{ os::compat::remove(argv[i]); }
			else
			{ os::remove(argv[i]); }
		}
		catch (std::exception &e)
		{ std::cerr << e.what() << std::endl; }
	}
}
