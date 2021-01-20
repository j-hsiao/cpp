#include <aes/aes.hpp>

#ifndef AES_DEBUG
#define AES_DEBUG
#endif
#include <aes/aes_util.hpp>

#include <timeutil/timeutil.hpp>

#include <cassert>
#include <iostream>
#include <string>

void check128()
{
	std::string s("Two One Nine Two");
	std::string expected(
		"\x29\xc3\x50\x5f\x57\x14\x20\xf6\x40\x22\x99\xb3\x1a\x02\xd7\x3a",
		16);
	aes::AES a("Thats my Kung Fu", aes::aes128);
	a.encrypt(s);
	assert(s.substr(0, 16) == expected);
	assert(s.size() == 32);
	a.decrypt(s);
	assert(s.size() == 16);
	assert(s == "Two One Nine Two");
	std::cout << "aes128 pass" << std::endl;
}

void check()
{
	std::string s("Two One Nine Two");
	aes::AES a192("0123456789abcdef01234567", aes::aes192);
	aes::AES a256("0123456789abcdef0124356789abcdef", aes::aes256);

	a192.encrypt(s);
	assert(s.size() == 32);
	assert(s.substr(0, 16) != "Two One Nine Two");
	a192.decrypt(s);
	assert(s.size() == 16);
	assert(s == "Two One Nine Two");
	std::cout << "pass aes 192" << std::endl;


	a256.encrypt(s);
	assert(s.size() == 32);
	assert(s.substr(0, 16) != "Two One Nine Two");
	a256.decrypt(s);
	assert(s.size() == 16);
	assert(s == "Two One Nine Two");
	std::cout << "pass aes 256" << std::endl;
}

void speed()
{
	std::string s("Two One Nine Two");
	std::string key("0123456789abcdef0123456789abcdef");
	aes::AES a128(key.data(), aes::aes128);
	aes::AES a192(key.data(), aes::aes192);
	aes::AES a256(key.data(), aes::aes256);

	timeutil::Timer t;

	std::size_t REPEATS = 1000000;

	std::cout << "aes 128... " << std::flush;
	t.tic();
	for (std::size_t i = 0; i < REPEATS; ++i)
	{
		a128.encrypt(s);
		a128.decrypt(s);
	}
	std::cout << t.toc() << std::endl;

	std::cout << "aes 192... " << std::flush;
	t.tic();
	for (std::size_t i = 0; i < REPEATS; ++i)
	{
		a192.encrypt(s);
		a192.decrypt(s);
	}
	std::cout << t.toc() << std::endl;

	std::cout << "aes 256... " << std::flush;
	t.tic();
	for (std::size_t i = 0; i < REPEATS; ++i)
	{
		a256.encrypt(s);
		a256.decrypt(s);
	}
	std::cout << t.toc() << std::endl;
}
int main()
{
	check128();
	check();
	speed();
}
