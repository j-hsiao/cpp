#include <os/os.hpp>
#include <os/os.h>
#include <argparse/argparse.hpp>

#include <iostream>

int argmain(int argc, char *argv[])
{
	argparse::Parser p;
	p.add({{"dirs"}, "dirs to look into", -1});
	auto args = p.parsemain(argc, argv);

	for (const std::string &dname: args["dirs"])
	{
		std::cout << dname << std::endl;
		os::Dirlist dlist(dname);
		std::string entry;
		while ((entry = dlist.next()).size())
		{
			std::cout << '\t' << entry << std::endl;
		}
	}
	return 0;
}
