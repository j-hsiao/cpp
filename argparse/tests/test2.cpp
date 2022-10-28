//test argparse2 interface
//


#include <iostream>
#include "argparse/argparse2.hpp"



int main(int argc, char *argv[])
{
	argparse::Parser p('-');
	const auto &intpair = p.add<int, 2>("doubleint", "a pair of ints");
	const auto &verbose = p.add<bool,0>("--verbose", "be verbose", "-v", {true});
	const auto &mynums = p.add<int, -1>("mynums", "multiple values");
	p.parse_main(argc, argv);

	std::cout << "intpair was" << intpair[0].as<int>() <<
		" and " << intpair[1].as<int>() << std::endl;
	std::cout << "verbose was " << verbose << std::endl;
	std::cout << "got " << mynums.size() << " extra nums." << std::endl;
	for (const auto &val : mynums)
	{
		std::cout << '\t' << val.as<int>() << std::endl;
	}
	return 0;
}
