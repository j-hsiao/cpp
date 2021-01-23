//	simple header-only library for parsing arguments
//
//	Argument: ordered collection of commandline arguments
//	flag: anything that begins with 1 or more -
//
//	example:
//		position is given as x y
//		Argument: "position"
//			args: x and y
//			nargs: 2
//
//	special flags:
//		--	stop parsing a multi-arg Argument
//		-N	treat next N args as non-flags
//
//	#define NO_ARGPARSE_MAIN
//	if you want to define main instead of using argmain
//
//	argparse::Argument(
//		std::set<std::string> names,
//		std::string help,
//		int nargs,
//		std::vector<std::string> defaults)
//
//	nargs: < 0 = multiargs
//	nargs == 0 : flag -> boolean flag, positional -> multiarg
//
//	argparse::Parser(
//		std::vector<Argument> args, std::string description, std::string program);
//	parser.add(const Argument&)
//
//	parser.parse(begin, end, exit = 1)
//	parser.parsemain(int argc, char *argv[]) (0 = program name)
//
//	-h: short help
//	--help: long help
//
//	each Argument has a certain number of args or variable
//	parsing ends when reaching:
//		specified number of arguments
//		a new flag
//		-0
//
//	treat flags as arguments:
//		--: parse any remaining flags as normal args for the current argument
//		-N: interpret N arguments as non-flags
//
//
//	convenience method: argparse::run(callable)
//		call callable with 0 arguments, catch exceptions and print, then return int
//
//	returns a map of values
//		keys are argument names with leading '-'s removed if any
//

#ifndef ARGPARSE_HPP
#define ARGPARSE_HPP

#include <algorithm>
#include <exception>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace argparse
{
	class StopParse: public std::exception {};

	template<class Func>
	int run(Func &&func, int argc, char *argv[]);

	//------------------------------
	// parsed values
	//------------------------------
	class Value
	{
		public:
			Value(const std::string &nm, const std::vector<std::string> &vals):
				name(nm), values(vals)
			{}

			std::vector<std::string>::const_iterator begin() const;
			std::vector<std::string>::const_iterator end() const;

			std::size_t size() const { return values.size(); }
			Value operator()(std::size_t i) const;
			const std::string& operator[](std::size_t i) const;

			template<class T>
			operator T() const;

			template<class T>
			operator std::vector<T>() const;

			template<class T>
			T as(std::size_t i = 0) const;

		private:
			const std::string name;
			const std::vector<std::string> values;

			void has(std::size_t i) const;
	};

	//------------------------------
	// argument description/parser
	//------------------------------
	class Argument
	{
		static bool falsey(const std::string &s);
		public:
			Argument(
				const std::set<std::string> &names_,
				const std::string &help_ = "",
				int nargs_ = 1,
				const std::vector<std::string> &defaults_ = {}
			);

			template<class T>
			Value operator()(T &it, const T &end) const;
			Value operator()() const;

			const std::string& name() const;

			const std::string helpstr(bool verbose) const;

			const std::vector<std::string> defaults;
			const std::set<std::string> names;
			const std::string help;
			const bool isFlag;
			const std::size_t nargs;
	};

	//------------------------------
	// args
	//------------------------------
	class Args
	{
		public:
			Args(
				const std::map<std::string, std::size_t> &inds_,
				const std::vector<Value> &args_);

			const Value& operator[](const std::string &name) const;
			const Value& at(const std::string &name) const { return (*this)[name]; }
			std::size_t size() const { return args.size(); }
		private:
			const std::map<std::string, std::size_t> inds;
			const std::vector<Value> args;
	};

	//------------------------------
	// parser
	//------------------------------
	class Parser
	{
		public:
			Parser(
				const std::vector<Argument> &args = {},
				const std::string &help_ = "",
				const std::string progName = "program");

			void add(const Argument &arg);

			Args dohelp(bool verbose, bool exit, const std::string &progname) const;

			template<class T>
			Args parse(
				T begin,
				const T &end,
				bool exit = 1,
				const std::string &progname = "") const;
			Args parsemain(int argc, char *argv[]) const;

		private:
			std::set<std::string> allNames;
			std::vector<Argument> positionals;
			std::vector<Argument> flags;
			std::map<std::string, std::size_t> flagmap;
			std::string help;
			std::string programName;
	};
}

#include <argparse/argparse_impl.hpp>

#ifndef NO_ARGPARSE_MAIN
	int argmain(int argc, char *argv[]);
	int main(int argc, char *argv[])
	{ argparse::run(argmain, argc, argv); }
#endif

#endif
