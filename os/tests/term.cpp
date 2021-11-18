#include "os/os.hpp"
#include "argparse/argparse.hpp"

#ifdef _WIN32
#define FORCE_COMPAT TESTING_DLL
#else
#define FORCE_COMPAT 0
#endif
int argmain(int argc, char *argv[])
{
	argparse::Parser p;
	p({"-c", "--chunksize", ""}, "chunk size", 1, {"256"});
	p({"-s", "--strip", ""}, "strip?", 0);
	p({"--compat"}, "use compat?", 0);

	auto args = p.parse(argc, argv);
	if (args["--compat"] || FORCE_COMPAT)
	{
		bool strip = args["strip"].as<bool>();
		auto chunksize = args["chunksize"].as<int>();
		os::compat::Term t("rb");
		std::string s = t.readline(strip);
		while (1)
		{
			if ( !s.size() || s == "\n" || s == "\r\n")
			{ return 0; }
			else if (s.substr(0, 5) == " hide")
			{ t.show_input(0); }
			else if (s.substr(0, 5) == " show")
			{ t.show_input(1); }
			else if (s.substr(0, 4) == "help")
			{ std::cerr << "\" hide\": hide input" << std::endl << "\" show\": show input" << std::endl; }
			std::cerr << '"' <<  s << '"' << std::endl;
			for (char c: s)
			{ std::cerr << "'" << static_cast<int>(c) << "', "; }
			std::cerr << std::endl;
			s = t.readline(strip);
		}
	}
	else
	{
		bool strip = args["strip"].as<bool>();
		auto chunksize = args["chunksize"].as<int>();
		os::Term t("rb");
		std::string s = t.readline(strip, chunksize);
		while (1)
		{
			if ( !s.size() || s == "\n" || s == "\r\n")
			{ return 0; }
			else if (s.substr(0, 5) == " hide")
			{ t.show_input(0); }
			else if (s.substr(0, 5) == " show")
			{ t.show_input(1); }
			else if (s.substr(0, 4) == "help")
			{ std::cerr << "\" hide\": hide input" << std::endl << "\" show\": show input" << std::endl; }
			std::cerr << '"' <<  s << '"' << std::endl;
			for (char c: s)
			{ std::cerr << "'" << static_cast<int>(c) << "', "; }
			std::cerr << std::endl;
			s = t.readline(strip, chunksize);
		}
	}
}
