#include <sha256/sha256.hpp>
#include <argparse/argparse.hpp>
#include <string>

int argmain(int argc, char *argv[])
{
	argparse::Parser p(
		{
			{{"inputs"}, "values to hash", 0}
		},
		"make sha256 hashes"
	);
	auto args = p.parsemain(argc, argv);

	for (const std::string &s : args.at("inputs"))
	{
		std::cout << sha256::to_hex(sha256::hash(sha256::normalize(s)))
			<< std::endl;
	}
	return 0;
}
