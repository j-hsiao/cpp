#include <aes/aes.hpp>
#include <sha256/sha256.hpp>
#include <timeutil/timeutil.hpp>

#undef NDEBUG
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

std::ostream& operator<<(std::ostream &o, aes::Version version)
{
	const char *vals[] = {
		"aes128",
		"aes192",
		"aes256"
	};
	return o << vals[version];
}

int main(int argc, char *argv[])
{
	std::vector<std::string> data = 
	{
		"hello world",
		"goodbye world",
		"Peter Piper picked a pickled plum or something"
	};

	timeutil::Timer timer;
	timeutil::Clocker clocker;
	std::string timetest(1024 * 1024 * 64, '\0');
	for (aes::Version version : {aes::Version::aes128, aes::Version::aes192, aes::Version::aes256})
	{
		std::cout << version << std::endl;
		for (const std::string &s : data)
		{
			std::cout << "plain: " << s << std::endl;
			std::string key = sha256::hash(s);
			aes::Codec codec(key, version);
			std::string encrypted = codec.encrypt_ecb(s);
			std::cout << "encrypted: " << sha256::to_hex(encrypted) << std::endl;
			assert(encrypted.size() == (s.size() + aes::padding(s.size())));
			assert(codec.decrypt_ecb(encrypted) == s);
			encrypted = codec.encrypt_cbc(s);
			assert(encrypted.size() == (s.size() + aes::padding(s.size())));
			assert(codec.decrypt_cbc(encrypted) == s);
		}
		aes::Codec codec(sha256::hash(data[0]), version);

		timer.tic(); clocker.tic();
		std::string dummy = codec.encrypt_ecb(timetest);
		double wall = timer.toc();
		double clock = clocker.toc();
		std::cout << "64 MB encrypt ecb: wall: " << wall << ", clock: " << clock << std::endl;

		timer.tic(); clocker.tic();
		codec.decrypt_ecb(dummy);
		wall = timer.toc(); clock = clocker.toc();
		std::cout << "64 MB decrypt ecb: wall: " << wall << ", clock: " << clock << std::endl;

		timer.tic(); clocker.tic();
		dummy = codec.encrypt_cbc(timetest);
		wall = timer.toc(); clock = clocker.toc();
		std::cout << "64 MB encrypt cbc: wall: " << wall << ", clock: " << clock << std::endl;

		timer.tic(); clocker.tic();
		codec.decrypt_cbc(dummy);
		wall = timer.toc(); clock = clocker.toc();
		std::cout << "64 MB decrypt cbc: wall: " << wall << ", clock: " << clock << std::endl;
	}
	std::cout << "pass" << std::endl;
}
