#include <os/os.hpp>
#include <string>
#include <iostream>

#undef NDEBUG
#include <cstring>
#include <cassert>

namespace
{
	typedef os::Path Path;

	std::ostream& operator<<(std::ostream &o, const Path &p)
	{ return o << '"' << static_cast<std::string>(p) << '"'; }
	bool operator==(const std::string &s, const Path &p)
	{ std::string o = p; return s == o; }
	bool operator==(const Path &p, const std::string &s)
	{ return s == p; }
}

void check(
	const Path &p,
	const std::string &value,
	const std::string &base,
	const std::string &ext,
	std::size_t segs)
{
	std::cout << p << std::endl
		<< "\tbase: \"" << base << "\" vs \"" << p.base() << '"' << std::endl
		<< "\text : \"" << ext << "\" vs \"" << p.ext() << '"' << std::endl
		<< "\tsize: " << p.size() << " vs " << static_cast<std::string>(p).size() << std::endl;
	assert(p.segments() == segs);
	assert(p.size() == static_cast<std::string>(p).size());
	assert(p == value);
	assert(p.base() == base);
	assert(p.ext() == ext);
}

int main(int argc, char *argv[])
{
	try
	{
#ifdef _WIN32
#define Sep "\\"
#define Root "C:"
#else
#define Sep "/"
#define Root ""
#endif

		Path p("some/path");
		check(p, "some" Sep "path", "path", "", 2);

		p += "seg1";
		check(p, "some" Sep "path" Sep "seg1", "seg1", "", 3);
		
		p += "..";
		check(p, "some" Sep "path", "path", "", 2);
		
		p += "..";
		check(p, "some", "some", "", 1);

		p += ".";
		check(p, "some", "some", "", 1);

		p += "..";
		check(p, ".", ".", "", 0);

		p += "hello////../goodbye";
		check(p, "goodbye", "goodbye", "", 1);

		p += "../..";
		check(p, "..", "..", "", 1);

		Path o("some/other/path");
		check(o, "some" Sep "other" Sep "path", "path", "", 3);

		o += p;
		check(o, "some" Sep "other", "other", "", 2);

		Path cp(o);
		assert(cp == o);
		cp += "hello";
		check(o, "some" Sep "other", "other", "", 2);
		check(cp, "some" Sep "other" Sep "hello", "hello", "", 3);
		
		cp += Root Sep "whatever" Sep;
		check(cp, Root Sep "whatever", "whatever", "", 1);

		cp += ".." Sep ".." Sep ".." Sep "blaha";
		check(cp, Root Sep "blaha", "blaha", "", 1);

		cp += ".whatever";
		check(cp, Root Sep "blaha" Sep ".whatever", ".whatever", "", 2);

		cp.add_ext(".txt");
		check(cp, Root Sep "blaha" Sep ".whatever.txt", ".whatever.txt", ".txt", 2);

		cp += "my.secret";
		check(cp, Root Sep "blaha" Sep ".whatever.txt" Sep "my.secret", "my.secret", ".secret", 3);

		cp.pop_ext();
		check(cp, Root Sep "blaha" Sep ".whatever.txt" Sep "my", "my", "", 3);
		cp.pop_ext();
		check(cp, Root Sep "blaha" Sep ".whatever.txt" Sep "my", "my", "", 3);

		cp.dirname();
		check(cp, Root Sep "blaha" Sep ".whatever.txt", ".whatever.txt", ".txt", 2);
		cp.pop_ext();
		check(cp, Root Sep "blaha" Sep ".whatever", ".whatever", "", 2);

		cp.dirname();
		check(cp, Root Sep "blaha", "blaha", "", 1);
		cp.pop_ext();
		check(cp, Root Sep "blaha", "blaha", "", 1);

		cp.dirname();
		check(cp, Root Sep, "", "", 0);
		cp.dirname();
		check(cp, Root Sep, "", "", 0);
		cp.pop_ext();
		check(cp, Root Sep, "", "", 0);






	}
	catch (std::exception &exc)
	{
		std::cerr << "error: " << exc.what() << std::endl;
		return -1;
	}
	return 0;
}
