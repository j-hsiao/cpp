#include "os/os.hpp"
#include "argparse/argparse.hpp"

#include <set>
#include <string>
#include <iostream>

int argmain(int argc, char *argv[])
{
	argparse::Parser p;
	p.add({"dirs"}, "dirs to look into", -0.0, {"."});
	auto args = p.parse(argc, argv);

	for (const char *dname: args["dirs"])
	{
		if (args["dirs"].size() > 1)
		{ std::cout << dname << std::endl; }
		std::set<std::string> orig, compat;
#if !(defined(_WIN32) && TESTING_DLL)
		try
		{
			for (const std::string &fname : os::Dirlist(dname))
			{
				orig.insert(fname);
				if (args["dirs"].size() > 1)
				{ std::cout << '\t'; }
				std::cout << fname << std::endl;
			}
		}
		catch (std::exception &e)
		{ std::cerr << e.what() << std::endl; }
#endif
		std::cout << "------------------------------" << std::endl
			<< "using compat interface..." << std::endl;
		try
		{
			for (const std::string &fname : os::compat::Dirlist(dname))
			{
				compat.insert(fname);
				if (args["dirs"].size() > 1)
				{ std::cout << '\t'; }
				std::cout << fname << std::endl;
			}
		}
		catch (std::exception &e)
		{ std::cerr << e.what() << std::endl; }
#if !(defined(_WIN32) && TESTING_DLL)
		if (orig != compat)
		{
			std::cerr << "compat and raw impls do not match: " << std::endl;
			auto i1 = orig.begin();
			auto i2 = compat.begin();
			while (i1 != orig.end() || i2 != compat.end())
			{
				if (i1 != orig.end()) { std::cerr << *i1; ++i1; }
				std::cerr << '\t';
				if (i2 != compat.end()) { std::cerr << *i2; ++i2; }
			}
		}
		else
		{ std::cerr << "\ncompat and orig match!" << std::endl; }
#endif
	}
	return 0;
}
