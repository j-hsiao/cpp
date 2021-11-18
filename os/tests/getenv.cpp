#include <os/os.hpp>

#include <cstddef>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		std::cerr << argv[i] << ": ";
#if !(defined(_WIN32) && TESTING_DLL)
		try
		{
			std::cerr << os::getenv(argv[i]) << std::endl;

			std::cerr << "orig clearing...";
			os::setenv(argv[i], nullptr);
			std::cerr << "cleared" << std::endl;
			try
			{
				std::cerr << "getting unset...";
				os::getenv(argv[i]);
				std::cerr << "should not get..." << std::endl;
				return 1;
			}
			catch (std::exception &)
			{ std::cerr << "successfully unset" << std::endl; }
		}
		catch (std::exception &e)
		{
			std::cerr << "error: " << e.what() << std::endl;
			try
			{
				os::setenv(argv[i], nullptr);
				std::cerr << "deleted " << argv[i] << std::endl;
			}
			catch (std::exception &e)
			{
				std::cerr << "failed to delete non-existing env var" << std::endl;
				std::cerr << e.what() << std::endl;
			}
			try
			{
				os::setenv(argv[i], "some new value");
				std::cerr << "set " << argv[i] << " to " << os::getenv(argv[i]) << std::endl;
			}
			catch (std::exception &e)
			{ std::cerr << "failed to create env var: " << e.what() << std::endl; }
		}
# endif
		std::cerr << "testing compat..." << std::endl;
		try
		{
			std::cerr << os::compat::getenv(argv[i]) << std::endl;

			os::compat::setenv(argv[i], nullptr);
			try
			{
				os::compat::getenv(argv[i]);
				std::cerr << "compat failed to unset" << std::endl;
				return 1;
			}
			catch (std::exception &)
			{ std::cerr << "compat successfully unset" << std::endl; }
		}
		catch (std::exception &e)
		{
			std::cerr << "compat error: " << e.what() << std::endl;
			try
			{
				os::compat::setenv(argv[i], nullptr);
				std::cerr << "compat deleted " << argv[i] << std::endl;
			}
			catch (std::exception &e)
			{
				std::cerr << "compat failed to delete non-existing env var" << std::endl;
				std::cerr << e.what() << std::endl;
			}
			try
			{
				os::compat::setenv(argv[i], "compat some new value");
				std::cerr << "compat set " << argv[i] << " to " << os::compat::getenv(argv[i]) << std::endl;
			}
			catch (std::exception &e)
			{ std::cerr << "compat failed to create env var: " << e.what() << std::endl; }
		}


	}
}
