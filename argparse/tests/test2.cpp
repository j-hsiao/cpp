//test argparse2 interface
//


#include "argparse/argparse2.hpp"

#include <iostream>



int main(int argc, char *argv[])
{
	argparse::Parser p('-');
	const auto &intpair = p.add<int, 2>("doubleint", "a pair of ints");
	const auto &verbose = p.add<bool,0>("--verbose", "be verbose", "-v", {true});
	const auto &mynums = p.add<float, -1>("mynums", "multiple values", "", {69, 70, 69.5});
	const auto &strflag = p.add<char*, -1>("--strflag", "multiple strings", "-s");
	if (p.parse_main(argc, argv)) { return 0; }

	std::cout << "intpair was " << intpair[0].as<int>() <<
		" and " << intpair[1].as<int>() << std::endl;
	std::cout << "verbose was " << verbose << std::endl;
	std::cout << "got " << mynums.size() << " extra nums." << std::endl;
	for (const auto &val : mynums)
	{
		std::cout << '\t' << val.as<float>() << std::endl;
	}
	std::cout << "extra strs" << std::endl;
	for (const auto &str : strflag)
	{
		std::cout << '\t' << str << std::endl;
	}
	return 0;
}
