#include <argparse/argparse.hpp>

#undef NDEBUG
#include <cassert>
#include <vector>
#include <string>


int argmain(int argc, char *argv[])
{
	{
		argparse::Value val("testval", {"v1", "v2", "3", "1"});
		std::string v1 = val;
		std::string v2 = val(1);
		assert(v1 == "v1");
		assert(v2 == "v2");
		int i = val(2);
		assert(i == 3);
		auto b = val.as<bool>(3);
		assert(b);

		const char *thing = val;

		std::set<std::string> strs = {"hi", "bye"};
		std::cout << strs.size() << std::endl;
		std::cout << "value passed" << std::endl;
	}
	{
		argparse::Argument arg(
			{"pos1", "alt"}, "some arg", -1);
		try
		{
			argparse::Argument arg(
				{"pos1", "-alt"}, "some arg", -1);
			assert(0);
		}
		catch (std::runtime_error&)
		{
			std::cerr << "caught" << std::endl;
		}

		std::vector<std::string> args = {"-1", "--hello", "world", "1", "69", "-newarg"};

		auto it = args.begin();
		auto val = arg(it, args.end());
		assert(*it == "-newarg");
		assert(val.size() == 4);
		assert(val[0] == "--hello");
		assert(val[1] == "world");
		assert(val[2] == "1");
		assert(val[3] == "69");

		std::cout << "argument: passed" << std::endl;
	}
	{
		argparse::Parser p(
			{
				{{"-v", "--verbose"}, "verbose", 0},
				{{"-t"}, "default true", 0, {"1"}},
				{{"-m", "--multi"}, "sample multi-arg arg", -1},
			},
			"testing parser"
		);
		p.add({{"pos1"}, "first positional argument", 1, {"default"}});
		p.add({{"pos2"}, "multi positional", -1, {"default1", "default2"}});

		std::vector<std::string> args = {};
		auto arg1 = p.parse(args.begin(), args.end());
		assert(arg1.size());
		assert(arg1["pos1"][0] == "default");


		p.add({{"pos3"}, "after multi", 2});
		args = {"-v", "-t", "-0", "-m", "-0", "hello", "-0", "-2", "-0", "-1" };
		auto arg2 = p.parse(args.begin(), args.end());
		assert(arg2.size());
		assert(arg2["pos1"].size() == 1);
		assert(arg2["pos2"].size() == 1);
		assert(arg2["pos3"].size() == 2);
		assert(arg2["pos1"][0] == "default");
		assert(arg2["pos2"][0] == "hello");
		assert(arg2["pos3"][0] == "-0");
		assert(arg2["pos3"][1] == "-1");
		assert(arg2["v"].as<bool>());
		assert(!arg2["t"].as<bool>());
		assert(arg2["m"].size() == 0);


		std::cout << "parse: passed" << std::endl;
	}
	return 0;
}
