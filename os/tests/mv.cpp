#include "os/os.hpp"
#include "argparse/argparse.hpp"

#include <iostream>
#include <iterator>

#ifdef _WIN32
#define FORCE_COMPAT TESTING_DLL
#else
#define FORCE_COMPAT 0
#endif
int argmain(int argc, char *argv[])
{
	argparse::Parser p;
	p({"files"}, "files and dst", -2.0);
	p({"-c", "--compat"}, "use compat", 0);

	auto args = p.parse(argc, argv);
	auto dst = std::prev(args["files"].end());

	if (args["files"].size() > 2 && !os::compat::Filenode(dst->as<const char*>()).isdir())
	{
		throw std::runtime_error(
			">2 args requires directory destination");
	}
	bool compat = args["--compat"];
	if (compat)
	{ std::cerr << "compat mode" << std::endl; }
	for (auto it=args["files"].begin(); it!=dst; ++it)
	{
		std::string arg = *it;
		try
		{
			if (compat || FORCE_COMPAT)
			{ os::compat::rename(it->as<const char*>(), dst->as<const char*>()); }
			else
			{ os::rename(it->as<const char*>(), dst->as<const char*>()); }
		}
		catch (std::exception &e)
		{ std::cerr << e.what() << std::endl; }
	}
	return 0;
}
