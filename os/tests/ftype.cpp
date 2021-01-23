#include <os/os.h>
#include <os/os.hpp>

#include <iostream>
#include <cstddef>
#undef NDEBUG
#include <cassert>

int main(int argc, char *argv[])
{
	const std::size_t bufsize = 256;
	char buf[bufsize];
	os__ebuf ebuf{buf, bufsize};

	for (int i = 1; i < argc; ++i)
	{
		auto node = os__Filenode__make(argv[i], &ebuf);
		if (node >= 0)
		{
			os::Filenode onode(argv[i]);
			std::cout << argv[i] << ": ";
			if (os__Filenode__exists(node))
			{
				assert(onode.exists());
				if (os__Filenode__is_dir_nc(node))
				{
					std::cout << "dir" << std::endl;
					assert(onode.is_dir() && onode.is_dir_nc());
				}
				else if (os__Filenode__is_file_nc(node))
				{
					std::cout << "file" << std::endl;
					assert(onode.is_file() && onode.is_file_nc());
				}
				else
				{
					std::cout << "other" << std::endl;
					assert(!onode.is_dir() && !onode.is_file());
				}
			}
			else
			{
				assert(!onode.exists());
				std::cout << "nonexistent" << std::endl;
			}
		}
		else
		{
			std::cerr << "stat error: " << buf << std::endl;
			try
			{
				os::Filenode onode(argv[i]);
				assert(0);
			}
			catch (std::exception &exc)
			{
				std::cerr << "os::Filenode error: " << exc.what() << std::endl;
			}
		}
	}


}
