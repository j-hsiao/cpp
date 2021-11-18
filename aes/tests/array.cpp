#include "aes/array.hpp"
#include "aes/hexlog.hpp"

#include <iostream>
#include <string>
#include <ctime>

int bad(const std::string &s)
{
	std::cerr << s << std::endl;
	return 1;
}
template<template<class T, std::size_t s> class T>
int check(std::size_t repeats)
{
	const int a[4] = {1, 2, 3, 4};

	T<const int, 4> w1{a};
	if (&w1[0] != a) { bad("wrapper should wrap given data"); }
	if (!std::is_same<int, typename T<const int, 4>::owned_type::value_type>::value)
	{ bad("owned value_type should never be const"); }

	auto o1 = w1 ^ w1;
	auto o2 = o1 ^ o1;

	auto o3 = a ^ o1;


	if (!std::is_same<decltype(o1), decltype(o2)>::value)
	{ return bad("owned xor return type should be same"); }
	for (std::size_t i=0; i<o1.size; ++i)
	{
		if (&w1[i] != a+i)
		{ return bad("wrapper address is wrong"); }
		if (o1[i])
		{ return bad("a^a should be all 0"); }
	}

	std::size_t warmups = 10000;
	for (std::size_t i=0;  i<warmups; ++i)
	{ o1 ^= w1 ^ w1; }

	auto start = std::clock();
	for (std::size_t i=0;  i<repeats; ++i)
	{ o1 ^= w1 ^ w1 ^ a; }
	auto end = std::clock();
	o1 ^= a;
	std::cerr << aes::hex(o1) << std::endl;
	std::cerr << repeats << " xors: "
		<< (static_cast<double>(end-start) / CLOCKS_PER_SEC) << std::endl;
	std::cerr << "passed" << std::endl;
	return 0;
}


#ifdef arr_crtp
template<class T, std::size_t n>
using crtpwrapped = aes::crtp::Wrapped<T, n, aes::crtp::Xorable>;
#endif

int main(int argc, char *argv[])
{
	std::size_t repeats = 10000000;
	if (argc > 1)
	{
		try
		{ repeats = std::stoull(argv[1]); }
		catch (...)
		{}
	}
	return (
		0
#ifdef arr_normal
		|| (std::cerr << "testing normal" << std::endl,
		check<aes::normal::Wrapped>(repeats))
#endif
#ifdef arr_crtp
		|| (std::cerr << "testing crtp" << std::endl,
		check<crtpwrapped>(repeats))
#endif
#ifdef arr_adl
		|| (std::cerr << "testing adl" << std::endl,
		check<aes::adl::Wrapped>(repeats))
#endif
#ifdef arr_sfinae
		|| (std::cerr << "testing sfinae" << std::endl,
		check<aes::sfinae::Wrapped>(repeats))
#endif
	);
}
