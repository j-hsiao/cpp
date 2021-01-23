//error code testing
#include <argparse/argparse.hpp>

#include <iostream>
#include <vector>

int argmain(int argc, char *argv[])
{
	argparse::Parser p;
	p.add({{"--lo"}, "low end of search", 1, {"0"}});
	p.add({{"--hi"}, "high end of search", 1, {"256"}});
	p.add({{"--system", "-s"}, "targets are system, not generic", 0});
	p.add({{"query"}, "value to search for", 1});

	auto args = p.parsemain(argc, argv);

	std::cout << std::system_category().message(static_cast<int>(args["query"])) << std::endl;

	std::error_code sys = std::error_code(args["query"], std::system_category());
	std::error_code gen = std::error_code(args["query"], std::generic_category());

	std::cerr << "syscode: " << sys.value() << ": " << sys.message() << std::endl;
	std::cerr << "gencode: " << gen.value() << ": " << gen.message() << std::endl;

	std::error_condition cond =
		static_cast<bool>(args["system"]) ? sys.default_error_condition() : gen.default_error_condition();

	for (int i = args["lo"]; i < int(args["hi"]); ++i)
	{
		auto code = std::error_code(i, std::generic_category());
		if (code == cond)
		{
			std::cerr << "matched: " << i << ": " << code.message() << std::endl;
		}
	}
	return 0;
}
