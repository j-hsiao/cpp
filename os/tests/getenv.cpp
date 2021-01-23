#include <os/os.hpp>

#include <cstddef>
#include <iostream>
#include <string>

void penv(char *buf, char *var, std::size_t bufsize)
{
	std::cout << var << ": ";
	os::ebuf ebuf;
	int ret = os__get_env(buf, var, bufsize, ebuf);
	if (ret < 0)
	{
		if (ret == -1)
		{ std::cerr << "not set" << std::endl; }
		else
		{ std::cerr << "error: " << ebuf << std::endl; }
	}
	else if (ret > bufsize)
	{
		std::cout << "buffersize " << bufsize << " too small, need " << ret << std::endl;
		if (bufsize == 5)
		{
			std::string nbuf(ret, '\0');
			penv(&nbuf[0], var, nbuf.size());
		}
	}
	else
	{ std::cout << std::string(buf, buf + ret) << std::endl; }
}

int main(int argc, char *argv[])
{
	char buf[5];
	for (int i = 1; i < argc; ++i)
	{
		penv(buf, argv[i], 5);
		try
		{ std::cout << argv[i] << ": \"" << os::get_env(argv[i]) << '"' << std::endl; }
		catch (os::NotFound &exc)
		{ std::cerr << "os::get_env: not found" << std::endl; }
		catch (std::exception &exc)
		{ std::cerr << "os::get_env error: " <<  exc.what() << std::endl; }
	}
}
