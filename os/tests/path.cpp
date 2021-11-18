#include "os/os.hpp"

#include <iostream>
namespace
{
	struct Testcase
	{
		std::string path;
		std::string normalized;
		std::string dirname;
		std::string basename;
		std::string ext;
	};
}
#ifdef _WIN32
#define x "\\"
#else
#define x "/"
#endif

std::string rootdir(os::compat::Path::sysdir(".").c_str(), 2);

std::vector<Testcase> general = {
#ifdef _WIN32
	{
		"/hello/world.ext",
		rootdir + "\\hello\\world.ext",
		rootdir + "\\hello",
		"world.ext",
		".ext"
	},
	{
		"/c/hello/world.ext.other",
		"C:\\hello\\world.ext.other",
		"C:\\hello",
		"world.ext.other",
		".other"
	},
	{
		"/c/hello/world.ext.other..\\\\..//////..\\..",
		"C:",
		"C:",
		"",
		""
	},
#else
	{
		"/home/nowhere/../../../../..",
		"/",
		"/",
		"",
		""
	},
#endif
	{
		"./a//../../../b///.",
		".." x ".." x "b",
		".." x "..",
		"b",
		""
	},
	{
		"./a/b/c",
		"a" x "b" x "c",
		"a" x "b",
		"c",
		""
	},
	{
		"./a//../../../b///..",
		".." x "..",
		"..",
		"..",
		""
	},
	{
		"somedir/..",
		".",
		"",
		".",
		""
	}
};
#undef x

int main(int argc, char *argv[])
{
#if !(defined(_WIN32) && TESTING_DLL)
	for(const auto &q : general)
	{
		std::cerr << "test path: " << q.path << std::endl;
		os::Path p(q.path);
		std::cerr << "\tnormalized: " << p.path << std::endl;
		if (p.path != q.normalized)
		{
			std::cerr << "path normalization failed: expected " << q.normalized
				<< " but got " << p.path << std::endl;
			return 1;
		}
		auto base = p.basename();
		std::cerr << "\tbase: " << base << std::endl;
		if (base != q.basename)
		{
			std::cerr << "basename was \"" << base << "\" but expected \"" << q.basename << '"' << std::endl;
			return 1;
		}
		auto ext = p.ext();
		std::cerr << "\text: " << ext << std::endl;
		if (ext != q.ext)
		{
			std::cerr << "ext was \"" << ext << "\" but expected \"" << q.ext << '"' << std::endl;
			return 1;
		}
	}
#endif
	std::cerr << "testing compat" << std::endl;
	for(const auto &q : general)
	{
		std::cerr << "test path: " << q.path << std::endl;
		os::compat::Path p(q.path);
		std::cerr << "\tnormalized: " << p.c_str() << std::endl;
		if (p.c_str() != q.normalized)
		{
			std::cerr << "path normalization failed: expected " << q.normalized
				<< " but got " << p.c_str() << std::endl;
			return 1;
		}
		auto base = p.basename();
		std::cerr << "\tbase: " << base << std::endl;
		if (base != q.basename)
		{
			std::cerr << "basename was \"" << base << "\" but expected \"" << q.basename << '"' << std::endl;
			return 1;
		}
		auto ext = p.ext();
		std::cerr << "\text: " << ext << std::endl;
		if (ext != q.ext)
		{
			std::cerr << "ext was \"" << ext << "\" but expected \"" << q.ext << '"' << std::endl;
			return 1;
		}
	}
	std::cerr << "passed" << std::endl;
	return 0;
}
