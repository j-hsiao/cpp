#include <string>

#define ARGPARSE_MAIN
#include <argparse/argparse.hpp>



int argmain(int argc, char *argv[])
{
	argparse::Parser p(
		{
			{{"posarg1"}, "help message for posarg1", 2},
			{{"posarg2"}, "help for posarg2", 1, {"hello"}},
			{{"-f1", "--flag1"}, "help for flag1", 0},
			{{"-f2", "--flag2"}, "help for flag2", 0, {"1"}},
			{{"posarg3"}, "help for posarg3", argparse::multi, {"default1", "default2"}},
			{{"-f3", "--flag3"}, "help for flag3", argparse::multi},
			{{"--detector", "-d"}, "detector arguments", argparse::multi}
		}
	);

	auto args = p.parsemain(argc, argv);

	for (const std::string &k : args.at("detector"))
	{
		std::cout << k << std::endl;
	}
	return 0;
}

/*
int main(int argc, char *argv[])
{
	return argparse::run(m, argc, argv);
}
*/
