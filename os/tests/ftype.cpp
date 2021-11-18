#include <os/os.hpp>

#include <iostream>

int main(int argc, char *argv[])
{
	for (int i=1; i<argc; ++i)
	{
#if !(defined(_WIN32) && TESTING_DLL)
			try
			{
				os::Filenode n(argv[i]);
				std::cerr << argv[i];
				if (!n)
				{ std::cerr << " does not exist" << std::endl; }
				else if (n.isdir())
				{ std::cerr << " is dir" << std::endl; }
				else if (n.isfile())
				{ std::cerr << " is file" << std::endl; }
				else
				{ std::cerr << " is other" << std::endl; }
			}
			catch (std::exception &e)
			{ std::cerr << " error: " << e.what() << std::endl; }
		std::cerr << "testing compat" << std::endl;
#endif
		try
		{
			os::compat::Filenode n(argv[i]);
			std::cerr << argv[i];
			if (!n)
			{ std::cerr << " does not exist" << std::endl; }
			else if (n.isdir())
			{ std::cerr << " is dir" << std::endl; }
			else if (n.isfile())
			{ std::cerr << " is file" << std::endl; }
			else
			{ std::cerr << " is other" << std::endl; }
		}
		catch (std::exception &e)
		{ std::cerr << " error: " << e.what() << std::endl; }
	}
}
