#include <os/os.h>
#include <cstddef>
#include <iostream>

int main(int argc, char *argv[])
{
	const std::size_t bufsize = 256;
	char b[bufsize];
	os__ebuf buf{b, bufsize};
	for (int i = 1; i < argc; ++i)
	{
		if (os__makedirs(argv[i], &buf))
		{
			std::cerr << "error making dir " << argv[i]
				<< ": " << buf.buf << std::endl;
		}
	}
	return 0;
}
