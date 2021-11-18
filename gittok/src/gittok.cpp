#include "aes/aes.hpp"
#include "argparse/argparse.hpp"
#include "os/os.hpp"
#include "sha256/sha256.hpp"

#include "gittok/tlog.hpp"

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>

using namespace gittok;

int argmain(int argc, char *argv[])
{
	argparse::Parser p("gittok", "manage git oauth tokens");
	p({"prompt"}, "the prompt to show (provided by git), none if adding tokens", {0, 1});
	p({"-a", "--add", ""}, "the name of the token to add", {0, 1});
	p({"-s", "--show", ""}, "show token when typing it in", 0);
	p({"--dir", ""}, "print the git token directory path", 0);

	auto args = p.parse(argc, argv);

	os::compat::Path tokdir;
	try { tokdir = os::compat::getenv("GITTOK_DIR"); }
	catch (std::exception&)
	{ tokdir = os::compat::Path::sysdir("home") + ".gittok"; }

	if (args["dir"])
	{
		std::cout << "git token directory: \"" << tokdir.c_str() << '"' << std::endl;
		for (std::string name : os::compat::Dirlist(tokdir.c_str()))
		{ std::cerr << '\t' << name << std::endl; }
	}
	else if (args["add"].size())
	{
		os::compat::Term term("r");
		std::string tokname = args["add"];
		std::string outname = (tokdir + tokname).c_str();
		os::compat::Filenode check(outname);
		if (check)
		{
			if (check.isdir())
			{ 
				std::cerr << tokname << " is a directory." << std::endl;
				return 1;
			}
			std::cerr << "token \"" << tokname << "\" exists, overwrite? (y/n): "
				<< std::flush;
			auto response = term.readline();
			if (response.find_first_of("yY"))
			{
				std::cerr << "canceled" << std::endl;
				return 0;
			}
		}
		std::cerr << "adding token: \"" << tokname << '"' << std::endl
			<< "token: " << std::flush;
		std::string token = term.show(args["show"]).readline();
		if (!args["show"]) { std::cerr << std::endl; }

		std::cerr << "password: " << std::flush;
		std::string password = term.show(0).readline();
		std::cerr << std::endl;

		password = sha256::hash(password);
		std::size_t size = token.size();
		token.resize(size + aes::State_Bytes);
		std::size_t outsize = aes::compat::Codec(password.c_str(), aes::compat::Version::aes256)
			.encrypt_cbc(&token[0], token.c_str(), size);
		token.resize(outsize);
		if (!os::compat::Filenode(tokdir.c_str()))
		{ os::compat::makedirs(tokdir.c_str()); }
		std::ofstream out;
		out.open(outname, out.binary|out.out|out.trunc);
		out << token;
		if (out)
		{ out.close(); return 0; }
		else
		{
			std::cerr << "error writing to " << outname;
			return 1;
		}
	}
	else if (args["prompt"].size())
	{
		std::string prompt = args["prompt"];
		os::compat::Term term("r");

		if (prompt.find("password") == prompt.npos
			&& prompt.find("Password") == prompt.npos)
		{
			std::cerr << args["prompt"].as<char*>() << std::flush;
			std::string response = term.readline();
			std::cout << response << std::endl;
		}
		else
		{
			std::cerr << "oauth token id: " << std::flush;
			std::string id = term.readline();
			if (id.size())
			{
				std::stringstream buf;
				std::ifstream tok;
				tok.open((tokdir + id).c_str(), tok.in | tok.binary);
				buf << tok.rdbuf();
				if (buf && tok)
				{
					std::string token = buf.str();
					std::cerr << "password: " << std::flush;
					std::string password = term.show(0).readline();
					std::cerr << std::endl;
					password = sha256::hash(password);
					token.resize(
						aes::compat::Codec(password.c_str(), aes::compat::Version::aes256)
						.decrypt_cbc(&token[0], token.c_str(), token.size()));
					std::cout << token << std::endl;
				}
				else
				{ std::cerr << "error reading token " << id << std::endl; }
			}
			else
			{
				std::cerr << "empty token id, using password" << std::endl;
				std::cerr << "password: " << std::flush;
				auto password = term.show(0).readline();
				std::cerr << std::endl;
				std::cout << password << std::endl;
			}
		}
	}
	else
	{ std::cerr << "use -h/--help for help" << std::endl; }
	return 0;
}
