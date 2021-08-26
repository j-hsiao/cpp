#include <iostream>
#include <cstdint>
#include <climits>
#include <string>

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{ 
		std::cerr << "missing argument: 'endian' or 'rep'" << std::endl;
		return -1;
	}

	std::string prog = argv[1];
	if (prog == "rep")
	{
		if (~0 == 0)
		{ std::cout << "ONE" << std::endl; }
		else if (~0 == -1)
		{ std::cout << "TWO" << std::endl; }
		else if (~0 == -INT_MAX)
		{ std::cout << "SMAG" << std::endl; }
		else
		{ std::cout << "UNKNOWN" << std::endl; }
	}
	else if (prog == "endian")
	{
		unsigned int value = 0;
		for (int i=0; i < sizeof(unsigned int); ++i)
		{ value |= (i+1) << (CHAR_BIT * i); }
		auto *bytes = reinterpret_cast<unsigned char*>(&value);
		bool little = 1;
		bool big = 1;
		for (int i=0; i < sizeof(unsigned int); ++i)
		{ 
			big = big && bytes[i] == sizeof(unsigned int) - i;
			little = little && bytes[i] == i+1;
		}
		if (little && !big)
		{ std::cout << "LITTLE" << std::endl; }
		else if (big && !little)
		{ std::cout << "BIG" << std::endl; }
		else
		{ std::cout << "UNKNOWN" << std::endl; }
	}
	else
	{
		std::cerr << "unknown command: \"" << prog << '"' << std::endl;
		return -1;
	}
	return 0;
}
