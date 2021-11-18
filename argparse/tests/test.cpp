#include "argparse/argparse.hpp"

#include <string>
#include <iostream>
#include <stdexcept>
#include <exception>
#include <type_traits>
#include <climits>


template<
	class T,
	class V,
	bool tisnum = std::is_integral<T>::value || std::is_floating_point<T>::value,
	bool visnum = std::is_integral<V>::value || std::is_floating_point<V>::value,
	class = typename std::enable_if<tisnum && visnum>::type>
void asserteq(const T &thing1, const V &thing2)
{
	if (thing1 != thing2)
	{
		std::cerr << "comparison failed:" << thing1 << " vs " << thing2 << std::endl;
		throw std::logic_error("comparison failed");
	}
}

template<class T>
void asserteq(const T &thing1, const std::string &thing2)
{
	if (thing1 != thing2)
	{
		std::cerr << "comparison failed:" << thing1 << " vs " << thing2 << std::endl;
		throw std::logic_error("comparison failed");
	}
}

int argmain(int argc, char *argv[])
{
	argparse::Parser::Value v1{"", "69"};
	argparse::Parser::Value v2{"", "-69"};
	argparse::Parser::Value v3{"", "69.125"};
	argparse::Parser::Value v4{"", "true"};

	asserteq(v1.as<signed int>(), 69);
	asserteq(v1.as<signed short>(), 69);
	asserteq(v1.as<unsigned int>(), 69);
	asserteq(v1.as<unsigned short>(), 69);
	asserteq(v1.as<float>(), 69);
	asserteq(v1.as<double>(), 69);
	asserteq(v1.val, "69");

	asserteq(v2.as<signed int>(), -69);
	asserteq(v2.as<signed short>(), -69);
	asserteq(v2.as<signed char>(), '-');
	asserteq(v2.as<float>(), -69);
	asserteq(v2.as<double>(), -69);
	asserteq(v2.val, "-69");

	asserteq(v3.as<float>(), 69.125);
	asserteq(v3.as<double>(), 69.125);
	asserteq(v3.val, "69.125");

	asserteq(v4.as<bool>(), static_cast<bool>(1));

	std::cerr << "pass" << std::endl;

	argparse::Parser p("argtest", "test argparse", "-");
	p.add({"-x"}, "1 val", 1, {"hi"})
		.add({"-y"}, "bool", 0, {"0"})
		.add({"req"}, "pos arg", 2)
		.add({"pos"}, "pos arg", -0.0)
		.add({"xy"}, "xy pair", 2, {"69", "32"})
		.add({"1+"}, "1 or more", {1, ULLONG_MAX})
		.add({"somearg"}, "some very very long description that should get wrapped around 40aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa 60aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa         80aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

	auto args = p.parse(argc, argv);

	for (const auto &arg : args)
	{
		std::cerr << arg.name << std::endl;
		std::cerr << '\t';
		for (const auto &s : arg)
		{
			const char *sp = s;
			std::cerr << sp << ' ';
		}
		std::cerr << std::endl;
	}

	return 0;
}
