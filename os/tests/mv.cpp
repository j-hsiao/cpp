#include <os/os.h>
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		std::cerr << "need src and dst" << std::endl;
	}
	char b[256];
	os__ebuf ebuf{b, 256};
	const char *dst = argv[argc - 1];
	for (int i = 1; i < argc - 1; ++i)
	{
		if (os__rename(argv[i], dst, &ebuf))
		{
			std::cerr << "error renaming " << argv[i] << ": " << b << std::endl;
			return -1;
		}
	}
	return 0;
}
