//	simple header-only library for parsing arguments
//
//	add defintion NO_ARGPARSE_MAIN
//	if don't want to use argmain()
//
//	argparse::Parser p(...);
//	auto args = p.parsemain(int argc, char *argv[]);
//	if (!args.size()) return 0;
//	// -h or --help (or no arguments to parse... but then why would you use Parser?)
//	args.at("argument").values;
//	args.at("argument").as<type>(index = 0);
//
//	convenience method: argparse::run(callable)
//		call callable with 0 arguments, catch exceptions and print, then return int
//
//	argparse::Parser p(
//	{
//		{{<names>}, "<help>", <nargs>, {<defaults>}}
//	});
//
//	<names>   : names/aliases for this argument
//	<help>    : description
//	<nargs>   : number of arguments for this arg
//	            std::string::npos for variable number of arguments
//	<defaults>: default values for this arg, length should match nargs
//	            unless nargs is 0 or std::string::npos (-1)
//	            defaults will be replaced if variable number of arguments
//	            otherwise unspecified positions will be filled from defaults
//
//	argument types:
//		argparse::multi is size_t(-1)
//	  positional
//	    0 or -1 will be variable args
//	  flag (names begins with - or --)
//	    0 : boolean flag
//	   -1 : variable args
//
//	returns std::Map<std::string, argparse::Value>
//	  if help flag is encountered or no registered arguments, return empty map
//	argparse::Value usage:
//	  v.values: std::vector<std::string>: raw values
//	  v.as<type>(index): parse value at index as type
//	                     essentially:
//	                     type t;
//	                     std::stringstream(v.values[index]) >> t;
//	
//
//	raise exceptions if parsing error (missing args etc)
//
//	special arguments:
//	  --: ignore any flags, treat all as positional arguments
//	  -N: N > 0: treat next N arguments as positional arguments
//	  -0: end a multi-argument parsing
//	  -h/--help   : print help message
//


#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

namespace argparse
{
	static const size_t multi = -1;
	class StopParse: public std::exception {};

	//wrap function taking same args as main with try/catch and print errors
	template<class T>
	int run(T &&func, int argc, char *argv[])
	{
		try
		{
			return func(argc, argv);
		}
		catch (StopParse&)
		{
			return 0;
		}
		catch (std::exception &exc)
		{
			std::cerr << "terminate after error: \"" << exc.what() << '"' << std::endl;
			return 1;
		}
	}

	std::string trimleft(const std::string s, std::string item)
	{
		size_t i = 0;
		while (s.substr(i, item.size()) == item)
		{
			i += item.size();
		}
		return s.substr(i);
	}

	template<class T>
	std::string join(const T &vals, std::string delim)
	{
		auto beg = vals.begin();
		auto end = vals.end();
		if (beg != end)
		{
			std::string ret = *(beg++);
			while (beg != end)
			{
				ret += delim;
				ret += *(beg++);
			}
			return ret;
		}
		return "";
	}

	class Value
	{
		public:
			Value(const std::string &nm, const std::vector<std::string> &vals):
				name(nm),
				values(vals)
			{}

			const std::string name;
			const std::vector<std::string> values;

			decltype(values)::const_iterator begin() const
			{ return values.begin(); }
			decltype(values)::const_iterator end() const
			{ return values.end(); }
			std::size_t size() const
			{ return values.size(); }
			const std::string& operator[](std::size_t i) const
			{ return values[i]; }

			Value operator()(std::size_t i) const
			{
				if (values.size() <= i)
				{
					throw std::runtime_error(name + " index out of bounds");
				}
				return Value(name, {values[i]});
			}

			template<class T>
			operator T() const
			{
				if (values.size() == 0)
				{
					throw std::runtime_error("no values to convert");
				}
				return this->as<T>();
			}

			template<class T>
			//NOTE if you want a std::string, you can just use values[i]
			//this is mostly for converting to bool, ints, etc
			T as(size_t i = 0) const
			{
				if (i >= values.size())
				{
					throw std::out_of_range(
						"bad index " + std::to_string(i) + " for " + name);
				}
				T ret;
				std::stringstream s(values[i]);
				s >> ret;
				if (s)
				{
					return ret;
				}
				else
				{
					throw std::runtime_error(
						"failed to retrieve value " + name + ":" + std::to_string(i)
						+ ": " + values[i]);
				}
			}
	};


	//an argument
	//stores a group of strings that were passed
	//as arguments
	//interprets the strings as needed
	class Argument
	{
		public:
			Argument(
				const std::set<std::string> &names_,
				const std::string &help_ = "",
				size_t nargs_ = 1,
				const std::vector<std::string> &defaults_ = {}
			):
				helpstr(help_),
				rawnames(names_),
				names(process_names()),
				isflag(check_flag()),
				nargs(((nargs_ == 0) && !isflag) ? std::string::npos : nargs_),
				defaults(defaults_.size() ? defaults_ : ((isflag && nargs == 0) ? std::vector<std::string>{"0"} : std::vector<std::string>{})),
				shortname(shortest(names)),
				shortraw(shortest(rawnames))
			{
				if (!isflag && nargs == 0)
				{
					throw std::logic_error(
						"positional args with nargs == 0 makes no sense");
				}
				if (defaults.size() != nargs)
				{
					//flags have 1 default but 0 nargs
					//variable has npos nargs, but fewer defaults
					if (
						!(
							((rawnames.begin()->substr(0, 1) == "-") && (nargs == 0))
							|| (nargs == std::string::npos)
							|| (defaults.size() == 0)
						)
					)
					{
						throw std::logic_error(
							"defaults given, but do not match number of args for "
							+ shortname);
					}
				}
			}

			std::string help(bool verbose) const
			{
				char pre = defaults.size() ? '[' : '<';
				char post = defaults.size() ? ']' : '>';
				if (verbose)
				{
					std::stringstream s;
					s << '"' << join(rawnames, "\", \"") << "\" (args: ";
					if (nargs == std::string::npos)
					{ s << "*)"; }
					else
					{ s << nargs << ")"; }
					if (defaults.size())
					{
						s << std::endl << "    default: \"" << join(defaults, "\" \"") << '"';
					}
					if (helpstr.size())
					{
						s << std::endl << "    description: " << helpstr;
					}
					return s.str();
				}
				else
				{
					std::string ret;
					if (nargs == std::string::npos)
					{
						ret = pre + shortname + "..." + post;
					}
					else
					{
						auto v = std::vector<std::string>(nargs, pre + shortname + post);
						ret = join(
							std::vector<std::string>(nargs, pre + shortname + post), " ");
					}
					if (isflag)
					{
						if (ret.size())
						{
							ret = shortraw + ' ' + ret;
						}
						else
						{
							ret = shortraw;
						}
						ret = "[" + ret + "]";
					}
					return ret;
				}
			}

			Value parse(const std::vector<std::string> &args) const
			{
				if (args.size() > nargs)
				{
					throw std::runtime_error("too many arguments for " + shortname);
				}
				if (nargs)
				{
					if (nargs == std::string::npos)
					{
						if (args.size())
						{
							return Value(shortname, args);
						}
						else
						{
							return Value(shortname, defaults);
						}
					}
					else
					{
						std::vector<std::string> v(args);
						while (v.size() < nargs)
						{
							try
							{
								v.push_back(defaults.at(v.size()));
							}
							catch (std::out_of_range)
							{
								throw std::runtime_error(shortname + " is missing arguments");
							}
						}
						return Value(shortname, v);
					}
				}
				else
				{
					if (defaults.size())
					{
						Value v(shortname, defaults);
						return Value(shortname, {v.as<bool>() ? "0" : "1"});
					}
					else
					{
						return Value(shortname, {"1"});
					}
				}
			}

			const std::string helpstr;
			const std::set<std::string> rawnames;
			const std::set<std::string> names;
			const bool isflag;
			const size_t nargs;
			const std::vector<std::string> defaults;
			const std::string shortname;
			const std::string shortraw;

			private:
				std::set<std::string> process_names()
				{
					if (!rawnames.size())
					{
						throw std::runtime_error("arguments must have at least 1 name");
					}
					std::set<std::string> ret;
					for (std::string n : rawnames)
					{
						ret.insert(trimleft(n, "-"));
					}
					return ret;
				}

				std::string shortest(const std::set<std::string> &s)
				{
					std::string ret = *s.begin();
					for (const std::string &cand : s)
					{
						if (cand.size() < ret.size())
						{
							ret = cand;
						}
					}
					return ret;
				}

				bool check_flag()
				{
					if (rawnames != names)
					{
						for (const std::string &n : rawnames)
						{
							if (names.count(n))
							{
								throw std::logic_error(
									"names should only be all args or all flags: "
									+ join(rawnames, ", "));
							}
						}
						return 1;
					}
					return 0;
				}
	};

	typedef std::map<std::string, Value> Args;
	class Parser
	{
		public:
			Parser(const std::vector<Argument> &args, const std::string &desc = "", const std::string &prog = ""):
				description(desc),
				program(prog)
			{
				std::set<std::string> collisioncheck;
				for (auto &a : args)
				{
					for (const std::string &s : a.names)
					{
						auto p = collisioncheck.insert(s);
						if (!(p.second))
						{
							throw std::logic_error(
								"multiple arguments with same name: " + s);
						}
					}
					if (a.isflag)
					{
						flags.push_back(a);
					}
					else
					{
						positionals.push_back(a);
					}
				}
				for (auto &a : flags)
				{
					for (const std::string &n : a.rawnames)
					{
						flagmap.insert({n, &a});
					}
				}
			}

			Args parsemain(int argc, char *argv[], bool exit = 1)
			{
				if (!program.size())
				{
					program = argv[0];
				}
				return parse(argv + 1, argv + argc, exit);
			}

			template<class T>
			Args parse(T begin, const T &end, bool exit = 1)
			{
				try
				{
					if (!program.size())
					{
						program = "[program]";
					}
					std::list<Argument> argq(positionals.begin(), positionals.end());
					std::map<std::string, const Argument*> flagq;
					for (const auto &arg : flags)
					{
						flagq.insert({arg.shortname, &arg});
					}
					Args vals;
					size_t ignore = 0;
					while (begin != end)
					{
						if (argq.size())
						{
							std::vector<std::string> argvals = nxt(
								argq.front().nargs, begin, end, ignore);
							if (argvals.size() == 0)
							{
								if (begin != end)
								{
									std::string flag = *begin++;
									if (add_flag(begin, end, flag, vals, flagq, ignore))
									{
										if (exit)
										{
											throw StopParse();
										}
										else
										{
											return {};
										}
									}
								}
							}
							else
							{
								add_value(vals, argq.front().parse(argvals), argq.front());
								argq.pop_front();
							}
						}
						else
						{
							if (flagq.size())
							{
								std::string flag = *(begin++);
								if (add_flag(begin, end, flag, vals, flagq, ignore))
								{
									if (exit)
									{
										throw StopParse();
									}
									else
									{
										return {};
									}
								}
							}
							else
							{
								throw std::runtime_error(
									"extra argument " + std::string(*begin));
							}
						}
					}
					for (const Argument &a : argq)
					{
						if (a.nargs)
						{
							add_value(vals, a.parse({}), a);
						}
						else
						{
							add_value(vals, Value(a.shortname, a.defaults), a);
						}
					}
					for (auto it : flagq)
					{
						const Argument &a = *(it.second);
						add_value(
							vals, Value(a.shortname, a.defaults), a);
					}
					return vals;
				}
				catch (StopParse&)
				{
					throw;
				}
				catch (std::exception &exc)
				{
					std::cerr << "argparse: failed to parse: " << exc.what() << std::endl << std::endl;
					shorthelp();
					throw;
				}
			}

		private:
			std::vector<Argument> positionals;
			std::vector<Argument> flags;
			std::map<std::string, Argument*> flagmap;
			std::string description;
			std::string program;

			void shorthelp()
			{
				std::cout << "usage: " << program << ' ' << "[-h | --help] ";
				for (const auto &a : flags)
				{
					std::cout << a.help(0) << ' ';
				}
				for (const auto &a : positionals)
				{
					std::cout << a.help(0) << ' ';
				}
				std::cout << std::endl;
			}

			void longhelp()
			{
				if (positionals.size())
				{
					std::cout << std::endl << "positionals" << std::endl;
					for (const auto &a : positionals)
					{ std::cout << std::endl <<  a.help(1) << std::endl; }
				}
				if (flags.size())
				{
					std::cout << std::endl << "flags" << std::endl;
					for (const auto &a : flags)
					{ std::cout << std::endl << a.help(1) << std::endl; }
				}
			}

			//handle a flag
			//return True if help flag (abort parsing)
			template<class T>
			bool add_flag(
				T &begin,
				const T &end,
				const std::string &flag,
				Args &vals,
				std::map<std::string, const Argument*> &flagq,
				size_t &ignore)
			{
				if (flag.substr(0, 1) != "-")
				{
					throw std::runtime_error("extra argument: " + flag);
				}
				if (flag == "-h" || flag == "--help")
				{
					shorthelp();
					if (flag == "--help")
					{
						std::cout << std::endl << description << std::endl;
						longhelp();
					}
					return 1;
				}
				if (!flagmap.count(flag))
				{
					throw std::runtime_error("unrecognized flag " + flag);
				}
				Argument &a = *flagmap.at(flag);
				flagq.erase(a.shortname);
				if (a.nargs)
				{
					Value v = a.parse(nxt(a.nargs, begin, end, ignore));
					add_value(vals, v, a);
				}
				else
				{
					Value v = a.parse({});
					add_value(vals, v, a);
				}
				return 0;
			}

			//add a value to value map
			//under all alternative names of the argument
			//for now, just make copies
			void add_value(Args &vals, const Value &val, const Argument &arg)
			{
				for (const std::string &nm : arg.names)
				{
					vals.insert({nm, val});
				}
			}

			//get next count arguments or until a flag (does not consume flag)
			template<class T>
			std::vector<std::string> nxt(size_t count, T &begin, const T &end, size_t &ignore)
			{
				std::vector<std::string> ret;
				while (begin != end && ret.size() < count)
				{
					std::string tok = *begin;
					if (ignore)
					{
						--ignore;
						++begin;
						ret.push_back(tok);
					}
					else if (tok.substr(0, 1) == "-")
					{
						if (tok == "--")
						{
							++begin;
							ignore = std::string::npos;
						}
						else if (tok.find_first_not_of("0123456789", 1) == std::string::npos)
						{
							++begin;
							ignore = std::stoul(tok.substr(1));
							if (ignore == 0)
							{
								return ret;
							}
						}
						else
						{
							//reached a flag, premature end
							return ret;
						}
					}
					else
					{
						++begin;
						ret.push_back(tok);
					}
				}
				return ret;
			}
	};
}

#ifndef NO_ARGPARSE_MAIN
int argmain(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	return argparse::run(argmain, argc, argv);
}
#endif

#endif//ARGPARSE_H
