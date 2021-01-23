#include <os/os.h>
#include <cstddef>
#include <iostream>

int main(int argc, char *argv[])
{
	const std::size_t s = 256;
	char b[s];
	os__ebuf ebuf{b, s};
	for (int i = 1; i < argc; ++i)
	{
		if (os__remove(argv[i], &ebuf))
		{ std::cerr << "error removing " << argv[i] << ": " << b << std::endl; }
	}
}
