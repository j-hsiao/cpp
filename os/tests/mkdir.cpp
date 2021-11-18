#include "os/os.hpp"
#include <cstddef>
#include <iostream>

int main(int argc, char *argv[])
{
	bool use_compat = 0;
	for (int i = 1; i < argc; ++i)
	{
		if (argv[i] == std::string("-c"))
		{
			use_compat = !use_compat;
			continue;
		}
		try
		{
			if (use_compat)
			{ os::compat::makedirs(argv[i]); }
			else
			{
#if !(defined(_WIN32) && TESTING_DLL)
				os::makedirs(argv[i]);
#else
				std::cerr << "testing dll, use compat anyways" << std::endl;
				os::compat::makedirs(argv[i]);
#endif
			}
		}
		catch (std::exception &e)
		{ std::cerr << e.what() << std::endl; }
	}
	return 0;
}
