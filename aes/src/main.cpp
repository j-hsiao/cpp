#include <aes/aes.hpp>
#include <os/os.hpp>
#include <sha256/sha256.hpp>
#include <argparse/argparse.hpp>
#include <timeutil/timeutil.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	std::string dirdst(
		const std::string &dst, const std::string &inp, const std::string &ext)
	{
		os::Path p(dst);
		p += os::Path(inp).base();
		p.add_ext(ext);
		return p;
	}

	std::string rawdst(
		const std::string &dst, const std::string &inp, const std::string &ext)
	{
		os::Path p(dst);
		p.add_ext(ext);
		return p;
	}



	std::string readfile(const std::string &name)
	{
		std::ifstream in(name, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, in.end);
			std::size_t datasize = in.tellg();
			std::string buf(datasize, '\0');
			in.seekg(0, in.beg);
			in.read(&buf[0], buf.size());
			if (in)
			{ return buf; }
		}
		throw std::runtime_error("failed to read file " + name);
	}

	bool verify_overwrite(const std::string &name)
	{
		std::cout << "overwrite " << name << "? (y/n): " << std::flush;
		std::string response;
		std::getline(std::cin, response);
		return response.size() && response[0] == 'y';
	}
}

int argmain(int argc, char *argv[])
{
	argparse::Parser p(
		{
			{{"sources"}, "input files, last is dst", -1},
			{{"--ext"}, "extension to add", 1, {""}},
			{{"--decrypt"}, "decrypt instead of encrypt", 0},
			{{"--mode"}, "mode: <cbc | ecb>", 1, {"cbc"}},
			{{"-b", "--bits"}, "aes version bits: <128 | 192 | 256>", 1, {"128"}},
			{{"-p", "--pass"}, "password", 1, {""}},
			{{"-x", "--hex"}, "hex key", 1, {""}},
			{{"-s", "--showkey"}, "print the key(hex) and exit", 0},
			{{"-y", "--yes"}, "answer yes to overwrite", 0},
			{{"-n", "--no"}, "answer no to overwrite", 0},
			{{"-v", "--verbose"}, "verbose", 0}
		},
		"aes encrpt/decrypt files"
	);

	auto args = p.parsemain(argc, argv);

	auto mProcessor = &aes::Codec::encrypt_cbc;
	if (args["mode"][0] == "cbc")
	{
		if (args["decrypt"].as<bool>())
		{ mProcessor = &aes::Codec::decrypt_cbc; }
		else
		{ mProcessor = &aes::Codec::encrypt_cbc; }
	}
	else if (args["mode"][0] == "ecb")
	{
		if (args["decrypt"].as<bool>())
		{ mProcessor = &aes::Codec::decrypt_ecb; }
		else
		{ mProcessor = &aes::Codec::encrypt_ecb; }
	}
	else
	{ throw std::runtime_error("invalid mode " + args["mode"][0]); }

	std::vector<std::string> fnames = args["sources"];
	std::string extension = args["ext"][0];
	aes::Version version;
	if (args["bits"][0] == "128") { version = aes::Version::aes128; }
	else if (args["bits"][0] == "192") { version = aes::Version::aes192; }
	else if (args["bits"][0] == "256") { version = aes::Version::aes256; }
	else { throw std::runtime_error("invalid aes version: must be 128, 192, or 256"); }

	std::string key;
	if (args["pass"][0].size())
	{ key = sha256::hash(args["pass"][0]); }
	else if (args["hex"][0].size())
	{ key = sha256::from_hex(args["hex"][0]); }
	else
	{
		os::Term t("rb");
		t.hide_input();
		std::cout << "password: " << std::flush;
		std::getline(std::cin, key);
		std::cout << std::endl;
		key = sha256::hash(key);
		t.show_input();
	}
	if (key.size() < aes__Key_Bytes[version])
	{
		throw std::runtime_error(
			"aes" + args["bits"][0] + " requires a key at least "
			+ std::to_string(aes__Key_Bytes[version]) + " bytes but got "
			+ std::to_string(key.size()));
	}
	if (args["showkey"].as<bool>())
	{
		std::cout << sha256::to_hex(key) << std::endl;
		return 0;
	}

	aes::Codec codec(key, version);
	if (fnames.size() < 2)
	{
		std::cerr << "requires at least 1 input and an output" << std::endl;
		return 1;
	}
	std::string dst = fnames.back();
	fnames.pop_back();

	os::Filenode node(dst);
	if (node.exists())
	{
		if (node.is_file_nc() && fnames.size() > 1)
		{ throw std::runtime_error("multiple inputs requires directory as output"); }
	}
	else if (fnames.size() > 1)
	{ os::makedirs(dst); }
	auto get_outname = fnames.size() > 1 || node.is_dir() ? dirdst : rawdst;

	timeutil::Timer timer;
	timeutil::Clocker clocker;
	for (const std::string &fname : fnames)
	{
		std::string outname = get_outname(dst, fname, extension);
		if (args["verbose"].as<bool>())
		{ std::cerr << fname << " --> " << outname << std::endl; }
		std::string data = readfile(fname);
		if (
			args["yes"].as<bool>() || !os::Filenode(outname).exists() ||
			(!args["no"].as<bool>() && verify_overwrite(outname)))
		{
			timer.tic();
			clocker.tic();
			std::string result = (codec.*mProcessor)(data);
			if (args["verbose"].as<bool>())
			{
				double wall = timer.toc();
				double clock = clocker.toc();
				std::cerr << "wall: " << wall << ", clock: " << clock << std::endl;
			}
			std::ofstream out(outname, std::ios::out | std::ios::binary | std::ios::trunc);
			if (!out) { throw std::runtime_error("failed to open output file " + outname); }
			out.write(result.c_str(), result.size());
			if (!out) { throw std::runtime_error("failed to output to file " + outname); }
		}
		else
		{
			if (args["verbose"].as<bool>()) { std::cerr << "skipped" << std::endl; }
		}
	}
	return 0;
}
