#include <sha256/sha256.hpp>
#include <argparse/argparse.hpp>
#include <string>


int argmain(int argc, char *argv[])
{
	argparse::Parser p("sha256", "make sha256 hashes");
	p({"inputs"}, "values to hash", -0.0);
	auto args = p.parse(argc, argv);

	for (const char *s : args["inputs"])
	{ std::cout << sha256::to_hex(sha256::hash(s)) << std::endl; }
	return 0;
}
