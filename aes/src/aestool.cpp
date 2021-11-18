#include "aes/aes.hpp"

#include "argparse/argparse.hpp"
#include "os/os.hpp"
#include "sha256/sha256.hpp"

#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>
namespace
{
	std::string fromhex(const std::string &src)
	{
		unsigned char lut[256];
		for (std::size_t i=0; i<256; ++i)
		{ lut[i] = 0x80u; }
		std::string hx("0123456789abcdef");
		for (std::size_t i=0; i<hx.size(); ++i)
		{ lut[hx[i]] = static_cast<unsigned char>(i); }
		hx = "ABCDEF";
		for (std::size_t i=0; i<hx.size(); ++i)
		{ lut[hx[i]] = static_cast<unsigned char>(i+10u); }
		if (src.size() % 2)
		{ throw std::runtime_error("hex requires even number of chars, got " + std::to_string(src.size())); }
		std::string out;
		for (std::size_t i=0; i<src.size(); ++i)
		{
			auto hi = lut[src[i]];
			auto lo = lut[src[++i]];
			if (hi & 0x80u || lo & 0x80u)
			{ throw std::runtime_error("invalid hex character '" + src.substr(i-1, 2) + "'"); }
			out.push_back(hi<<4u | lo);
		}
		return out;
	}
}

int argmain(int argc, char *argv[])
{
	argparse::Parser p("aestool", "encrypt/decrypt files");
	p(
		{"files"},
		"<input file(s) ...> <output>, <output> can be a dir."
		"  <output> MUST be a dir if multiple inputs are given."
		"  Streaming input is not supported so files should be small enough"
		" to fit into memory.",
		-2.0);
	p({"-d", "--decrypt", ""}, "decrypt instead of encrypt", 0);
	p({"--reps", ""}, "hash repetitions", 1, {"1"});
	p({"--hex", "-x", ""}, "key is in hex format", 0);

	auto args = p.parse(argc, argv);

	os::compat::Term t("r");

	auto dst = std::prev(args["files"].end());
	std::string out = *dst;
	bool dstdir = args["files"].size() > 2;
	{
		os::compat::Filenode check(out);
		if (check)
		{
			if (check.isdir())
			{ dstdir = 1; }
			else
			{
				if (dstdir)
				{
					throw std::runtime_error(
						"multiple inputs requires dir output, but " + out + " is not a dir");
				}
			}
		}
		else
		{
			if (dstdir) { os::compat::makedirs(out); }
		}
	}

	std::string key;
	std::cerr << "key: " << std::flush;
	key = t.show(0).readline();
	if (args["hex"].as<bool>())
	{ key = fromhex(key); }
	const std::size_t reps = args["reps"].as<std::size_t>();
	for (std::size_t i=0; i<reps; ++i)
	{ key = sha256::hash(key); }

	if (key.size() < aes::aes256.Key_Bytes)
	{
		std::cerr << "key too short, aes256 requires " << aes::aes256.Key_Bytes
			<< " bytes, got " << key.size() << std::endl;
		return 1;
	}
	aes::compat::Codec codec(key.c_str(), aes::compat::Version::aes256);

	std::string buf;
	for (auto beg = args["files"].begin(); beg != dst; ++beg)
	{
		std::string input = *beg;
		std::string oname;
		if (dstdir)
		{
			oname = (os::compat::Path(out) + os::compat::Path(input).basename()).c_str();
		}
		else
		{ oname = out;}
		if (os::compat::Filenode(oname))
		{
			std::cerr << out << " exists, overwrite? (y/n): " << std::flush;
			std::string line = t.readline();
			if (line.find_first_of("yY") != 0)
			{
				std::cerr << "skipped " << input << std::endl;
				continue;
			}
		}

		std::cerr << "processing " << input << std::endl;
		std::ifstream f;
		f.open(input, f.in|f.binary);
		f.seekg(0, f.end);
		buf.resize(f.tellg());
		f.seekg(0, f.beg);
		f.read(&buf[0], buf.size());
		if (!f)
		{ throw std::runtime_error("failed to read input " + input); }
		std::cerr << "\tbytes: " << buf.size() << std::endl;

		if (args["decrypt"].as<bool>())
		{
			buf.resize(codec.decrypt_cbc(&buf[0], &buf[0], buf.size()));
		}
		else
		{
			std::size_t datasize = buf.size();
			buf.resize(datasize + aes::State_Bytes);
			buf.resize(codec.encrypt_cbc(&buf[0], &buf[0], datasize));
		}
		std::ofstream o;
		o.open(oname, o.binary|o.out|o.trunc);
		o.write(buf.data(), buf.size());
		if (!o)
		{ throw std::runtime_error("error writing output for " + input); }
		o.close();
	}
	return 0;
}
