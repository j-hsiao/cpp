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
			{{"-t", "--token"}, "token to add", 1, {""}},
			{{"--show"}, "show the token from input", 0}
		},
		"github ASK_PASS to use password with git O_AUTH tokens. "
		"tokens should be encrypted and stored in %profile% (WINDOWS) or $HOME (linux). "
	);
	auto args = p.parsemain(argc, argv);

	std::string gittokDir;
	try
	{ gittokDir = os::get_env("GITTOK_DIR"); }
	catch (os::NotFound &e)
	{
		gittokDir = os::get_sysdir("home");
		if (!gittokDir.size())
		{ throw std::runtime_error("could not determine home directory");}
		gittokDir = os::Path(gittokDir) + ".gittok";
	}

	if (!os::Filenode(gittokDir).exists())
	{ os::makedirs(gittokDir); }
	os::Path tokenpath(gittokDir);
	os::Term term("rb");
	if (args["add"][0].size())
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
		std::string token = args["token"][0];
		if (!token.size())
		{
			if (!args["show"].as<bool>())
			{ term.hide_input(); }
			std::cerr << "token: " << std::flush;
			token = term.readline();
			if (!args["show"].as<bool>())
			{ std::cerr << std::endl; }
		}

		std::cerr << "password: " << std::flush;
		term.hide_input();
		std::string password = term.readline();
		std::cerr << std::endl;
		term.show_input();
		for (char c  : password)
		{
			Log() << '"' << c << "\"(" << static_cast<int>(c) << "), ";
		}
		Log() << std::endl;

		aes::Codec encrypter(sha256::hash(password), aes::Version::aes256);
		encrypter.iencrypt_cbc(token);
		std::ofstream f(tokenpath.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
		f.write(token.c_str(), token.size());
		if (!f)
		{
			throw std::runtime_error(
				std::string("failed to write data to ") + tokenpath.c_str());
		}
		return 0;
	}
	std::string prompt;
	auto argbegin = args["prompt"].begin();
	if (argbegin != args["prompt"].end())
	{
		prompt = *argbegin;
		++argbegin;
		while (argbegin != args["prompt"].end())
		{
			prompt += " ";
			prompt += *argbegin;
			++argbegin;
		}
	}
	if (
		prompt.find("password") != std::string::npos 
		|| prompt.find("Password") != std::string::npos)
	{
		std::cerr << "git token: " << std::flush;
		std::string name = term.readline();
		tokenpath += name;

		std::cerr << "token password: " << std::flush;
		term.hide_input();
		std::string password = term.readline();
		std::cerr << std::endl;
		term.show_input();
		for (char c : password)
		{ Log() << '"' << c << "\", "; }
		Log() << std::endl;

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
				std::cout << std::endl;
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
