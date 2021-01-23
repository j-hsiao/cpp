#include <aes/aes.hpp>
#include <argparse/argparse.hpp>
#include <os/os.hpp>
#include <sha256/sha256.hpp>

#include <tlog/tlog.hpp>

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <string>

#ifndef GITTOK_DEBUG
#define GITTOK_DEBUG 0
#endif

int argmain(int argc, char *argv[])
{
	typedef tlog::Log<GITTOK_DEBUG> Log;
	argparse::Parser p(
		{
			{{"prompt"}, "the prompt", -1},
			{{"-a", "--add"}, "name of token to add", 1, {""}},
			{{"-t", "--token"}, "token to add", 1, {""}}
		},
		"github ASK_PASS to use password with git O_AUTH tokens. "
		"tokens should be encrypted and stored in %profile% (WINDOWS) or $HOME (linux). "
	);
	auto args = p.parsemain(argc, argv);
	std::stringstream ss;
	std::string homedir = os::get_sysdir("home");
	os::Term term("rb");
	if (!homedir.size())
	{ throw std::runtime_error("could not determine home directory"); }
	os::Path tokenpath(homedir);
	tokenpath += ".gittok";
	os::Filenode tokdir(tokenpath);
	if (!tokdir.exists())
	{ os::makedirs(tokenpath); }
	if (args["add"][0].size() && args["token"][0].size())
	{
		tokenpath += args["add"][0];
		os::Filenode target(tokenpath);
		if (target.exists())
		{
			std::cerr << "file " << tokenpath.c_str() << " exists, overwrite? (y/n): " << std::flush;
			std::string answer = term.readline();
			if (answer != "y")
			{
				std::cerr << "canceled" << std::endl;
				return 0;
			}
		}
		std::cerr << "password: " << std::flush;
		term.hide_input();
		std::string password = term.readline();
		term.show_input();
		for (char c  : password)
		{
			Log() << '"' << c << "\", ";
		}
		Log() << std::endl;

		aes::Codec encrypter(sha256::hash(password), aes::Version::aes256);
		std::string token = encrypter.encrypt_cbc(args["token"][0]);
		std::ofstream f(tokenpath.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
		f.write(token.c_str(), token.size());
		if (!f)
		{
			throw std::runtime_error(std::string("failed to write data to ") + tokenpath.c_str());
		}
		return 0;
	}
	auto argbegin = args["prompt"].begin();
	if (argbegin != args["prompt"].end())
	{
		ss << *argbegin; 
		++argbegin;
		while (argbegin != args["prompt"].end())
		{
			ss << " " << *argbegin;
			++argbegin;
		}
	}
	std::string prompt = ss.str();
	if (
		prompt.find("password") != std::string::npos 
		|| prompt.find("Password") != std::string::npos)
	{
		std::cerr << "git token: " << std::flush;
		std::string name = term.readline();
		std::cerr << "token password: " << std::flush;
		term.hide_input();
		std::cerr << std::endl;
		std::string password = term.readline();
		term.show_input();
		for (char c : password)
		{ Log() << '"' << c << "\", "; }
		Log() << std::endl;
		tokenpath += name;
		std::ifstream f(tokenpath.c_str(), std::ios::binary | std::ios::in);
		if (f)
		{
			f.seekg(0, f.end);
			std::string token(f.tellg(), '\0');
			f.seekg(0, f.beg);
			f.read(&token[0], token.size());
			if (f)
			{
				aes::Codec decrypter(sha256::hash(password), aes::Version::aes256);
				decrypter.idecrypt_cbc(token);
				std::cout.write(token.c_str(), token.size());
			}
			else
			{ throw std::runtime_error("failed to read file " + name); }
		}
		else
		{ throw std::runtime_error("failed to open file " + name); }
	}
	else
	{
		std::cerr << prompt << std::flush;
		std::string response = term.readline();
		std::cout << response << std::flush;
	}
	return 0;
}
