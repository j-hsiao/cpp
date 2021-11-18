#include <os/os.hpp>
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
			{ std::cerr << argv[i] << ": " << os::compat::Path::sysdir(argv[i]).c_str() << std::endl; }
			else
			{ std::cerr << argv[i] << ": " << os::Path::sysdir(argv[i]).c_str() << std::endl; }
		}
		catch (std::exception &e)
		{ std::cerr << "error: " << e.what() << std::endl; }
	}
}
