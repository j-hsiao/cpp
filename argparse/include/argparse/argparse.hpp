// Simple header-only library for parsing args.
// focus = ease of use, not performance.
//
//	usage:
//		argparse::Parser parser(description, prefix)
//			description: program description.
//			prefix: prefix to distinguish flags. A flag is anything that
//				starts with prefix.
//
//		parser.add({"altnames"...}, "help str", nargs={lo, [hi]}, {"defaults"...});
//			or
//		parser(...) (forwards to add)
//			If even a single name begins with prefix, then the argument
//			is considered a flag.  Arguments that do not begin with prefix act as
//			an alias and can be used to access the values, but cannot be used
//			on the command line. Names must all be unique.
//			operator() or add() returns reference to self so calls
//			can be chained.  An empty name "" is treated specially and will add a
//			left-prefix-stripped version of each name as aliases.
//			For example: names: {"--flag", "-----", ""}, prefix="-"
//			used names = {"--flag", "-----", "flag"}.  (The left-prefix-stripped version of
//			"-----" is "", which is ignored.
//
//
//			Only flags can have a nargs of 0, which indicates that it is a boolean
//			flag. For boolean flags, they are by default, False.  If defaults are given,
//			they should be an int and only "0" is considered false.
//			else makes the flag default to True.  When boolean flags are encountered,
//			they're parsed value will be the opposite of their default.
//
//			nargs:
//				1 argument: a float: nonnegative values are taken as exact (lo = hi = val)
//					negative numbers will set hi to INT_MAX and lo to abs(int(val))
//					-0 => 0 or more, -1 => 1 or more
//				2 arguments: sets lo and hi directly.
//
//		results = parser.parse(argc, argv); //args from main(int argc, char *argv[])
//		results = parse.parse(begin_iterator, end_iterator); //iterators
//
//		int/float/const char* = results["name provided to add"][nth sub-arg]
//		int/float/const char* = results["name provided to add"] (default to [0])
//			The name is verbatim, so if provided --something, then you must use
//			--something as the name. If multiple names are given, then any of them
//			can be used.
//
//	num_args < 0 -> variable, collect args until stop (end, a flag, etc)
//	special flags:
//		<prefix>/: stop parsing the current argument
//		<prefix><prefix>: all remaining arguments are not flags.
//		<prefix>N: The next N arguments are NOT flags, even if they start with <prefix>
//
//	<prefix>h will yield short description
//	<prefix><prefix>help will yield full description
//
//	arguments can be converted to:
//		const char*
//		integral types
//		float types

#ifndef ARGPARSE_HPP
#define ARGPARSE_HPP

#include <cctype>
#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <sstream>
#include <utility>
#include <climits>
namespace argparse
{
	static const unsigned long long NOLIM = ULLONG_MAX;
	struct ss
	{
		template<class ... T>
		ss(T&&...args): s(std::forward<T>(args)...)
		{s.seekp(0, s.end);}
		std::stringstream s;
		std::string str() { return s.str(); }
		template<class T>
		ss& operator<<(T &&x)
		{
			s << std::forward<T>(x);
			return *this;
		}
		operator std::string() const { return s.str(); }
		ss& operator<<(std::ostream& (*func)(std::ostream&))
		{
			s << func;
			return *this;
		}
	};

	class StopParse: public std::runtime_error { using std::runtime_error::runtime_error; };
	class StopHelp: public std::runtime_error { using std::runtime_error::runtime_error; };
	template<class func>
	int tryrun(func &&f, int argc, char *argv[])
	{
		try
		{ return f(argc, argv); }
		catch (StopParse &exc)
		{ std::cerr << exc.what() << std::endl; }
		catch (StopHelp&) { return 0; }
		catch (std::exception &exc)
		{ std::cerr << "terminate after error: " << exc.what() << std::endl; }
		catch (...)
		{ std::cerr << "terminate, unknown" << std::endl; }
		return 1;
	}

	struct Parser
	{
		struct Value
		{
			const std::string name;
			const std::string val;

// 			std::string badconvert(const char *tp) const
// 			{
// 				return (
// 					ss("error converting ") << name
// 					<< "(\"" << val << "\") to " << tp).str();
// 			}
// 			template<
// 				class T,
// 				bool isint = std::is_integral<T>::value,
// 				bool issigned = std::is_signed<T>::value,
// 				typename std::enable_if<isint && issigned, T>::type=1>
// 			operator T() const
// 			{
// 				try
// 				{ return static_cast<T>(std::stoll(val)); }
// 				catch (std::exception &e)
// 				{ throw std::runtime_error(badconvert("int")); }
// 			}
// 			template<
// 				class T,
// 				bool isint = std::is_integral<T>::value,
// 				bool issigned = std::is_unsigned<T>::value,
// 				typename std::enable_if<isint && issigned, bool>::type=1>
// 			operator T() const
// 			{
// 				try
// 				{ return static_cast<T>(std::stoull(val)); }
// 				catch (std::exception &e)
// 				{ throw std::runtime_error(badconvert("uint")); }
// 			}
// 			template<
// 				class T,
// 				bool isfloat = std::is_floating_point<T>::value,
// 				typename std::enable_if<isfloat, bool>::type=1>
// 			operator T() const
// 			{
// 				try
// 				{ return static_cast<T>(std::stod(val)); }
// 				catch (std::exception &e)
// 				{ throw std::runtime_error(badconvert("float")); }
// 			}
			//guard against operator char*
			template<
				class T, typename std::enable_if<!std::is_pointer<T>::value, bool>::type=1>
			operator T() const
			{
				std::stringstream ss(val);
				T item;
				ss >> item;
				if (!ss && std::is_same<T, bool>::value)
				{
					//std::stringstream reacts differently to boolalpha
					//std::stringstream boolalpha = ONLY true/false
					//std::stringstream noboolalpha = ONLY 0/1
					//but for example, std::cin boolalpha = 0/1/true/false etc all fine
					ss.seekg(0).clear();
					if (ss)
					{ ss >> std::boolalpha >> item; }
				}
				if (!ss)
				{ throw std::runtime_error("failure converting " + name); }
				return item;
			}
			operator const char*() const { return val.c_str(); }
			operator std::string() const { return val; }
			template<
				class T, typename std::enable_if<!std::is_same<char*, T>::value, bool>::type = 1>
			T as() const { return *this; }
			template<
				class T, typename std::enable_if<std::is_same<char*, T>::value, bool>::type = 1>
			const char* as() const { return val.c_str(); }
			std::size_t size() const { return val.size(); }
			const char* c_str() const { return val.c_str(); }
		};
		struct Values
		{
			std::vector<Value> data;
			std::string name;

			const Value& operator[](std::size_t i) const
			{
				try { return data[i]; }
				catch (std::exception&)
				{
					throw std::runtime_error(
						name + "[" + std::to_string(i) + "] not found");
				}
			}
			template<class T>
			operator T() const { return data[0]; }
			template<class T, typename std::enable_if<!std::is_same<T, char*>::value, bool>::type=1>
			T as() const { return data[0]; }
			template<class T, typename std::enable_if<std::is_same<T, char*>::value, bool>::type=1>
			const char* as() const { return data[0]; }
			const char* c_str() const { return data[0].c_str(); }

			std::size_t size() const { return data.size(); }
			std::vector<Value>::const_iterator begin() const { return data.begin(); }
			std::vector<Value>::const_iterator end() const { return data.end(); }

			Values(const std::string &name_, const std::vector<std::string> &vals):
				data{},
				name(name_)
			{
				for (std::size_t i=0; i<vals.size(); ++i)
				{ data.push_back({(ss(name) << '[' << i << ']').str() , vals[i]}); }
			}
		};
		struct Nargs
		{
			typedef unsigned long long sz;
			sz lo, hi;
			Nargs(double f):
				lo(static_cast<sz>(std::fabs(f))),
				hi(std::signbit(f) ? NOLIM : static_cast<sz>(f))
			{}
			Nargs(sz l, sz h): lo(l), hi(h)
		 {
			if (hi < lo)
			{ throw std::logic_error((ss("bad nargs ") << lo << ", " << hi).str()); }
		}
		};
		struct ArgInfo
		{
			std::set<std::string> names;
			std::set<std::string> argnames;
			std::string help;
			Nargs nargs;
			std::vector<std::string> defaults;
			const Parser* owner;
			ArgInfo(
				const std::set<std::string> &names_,
				const std::string &help_,
				const Nargs &nargs_,
				const std::vector<std::string> &defaults_,
				const Parser* owner_
			):
				names(names_), argnames(names_), help(help_), nargs(nargs_), defaults(defaults_), owner(owner_)
			{
				if (names.count(""))
				{
					names.erase("");
					argnames.erase("");
					const std::string &prefix = owner->prefix;
					for (const auto &name: names_)
					{
						if (!name.size()) { continue; }
						for (std::size_t start=0; start<name.size(); start+=prefix.size())
						{
							if (name.compare(start, prefix.size(), prefix))
							{
								names.insert(name.substr(start));
								break;
							}
						}
					}
				}
			}

			template<bool PositionalsAre0>
			bool flagcmp(const std::string &s1, const std::string &s2) const
			{
				bool b1 = owner->ispos(s1);
				bool b2 = owner->ispos(s2);
				if (b1 == b2)
				{ return s1.size() < s2.size(); }
				else
				{ return PositionalsAre0 == b1; }
			}
			std::string longest() const
			{
				return *std::max_element(
					names.begin(), names.end(),
					[this](const std::string &s1, const std::string &s2)
					{ return this->flagcmp<1>(s1, s2); });
			}
			std::string shortest() const
			{
				return *std::min_element(
					names.begin(), names.end(),
					[this](const std::string &s1, const std::string &s2)
					{ return this->flagcmp<0>(s1, s2); });
			}
		};
		struct Parsed
		{
			std::map<std::string, Values*> mapping;
			std::list<Values> vals;

			std::list<Values>::const_iterator begin() const { return vals.begin(); }
			std::list<Values>::const_iterator end() const { return vals.end(); }
			const Values& operator[](const std::string &s) const
			{
				try
				{ return *mapping.at(s); }
				catch (std::exception&)
				{ throw std::runtime_error("argument " + s + " was not found"); }
			}
			void add(const ArgInfo &info, const std::vector<std::string> &vs)
			{
				auto it = mapping.find(info.longest());
				if (it == mapping.end())
				{
					vals.push_back({info.longest(), vs});
					for(const std::string &name : info.names)
					{ mapping[name] = &vals.back(); }
				}
				else
				{ throw StopParse("repeated arg " + info.longest()); }
			}
		};

		Parser(
			const std::string &progname="",
			const std::string &desc="",
			const std::string &flagprefix="-"
		):
			program_name(progname.size() ? progname : "program"),
			description(desc),
			prefix(flagprefix),
			args{},
			posstart(args.end()),
			flags{},
			positionals{},
			posnames{}
		{}

		std::string program_name;
		std::string description;
		std::string prefix;
		std::list<ArgInfo> args;
		std::list<ArgInfo>::const_iterator posstart;
		std::map<std::string, ArgInfo*> flags;
		std::vector<ArgInfo*> positionals;
		std::set<std::string> posnames;

		//using templates to forward
		//cannot forward initializer lists...
		Parser& operator()(
			const std::set<std::string> &names,
			const std::string &help="",
			const Nargs &nargs=1,
			const std::vector<std::string> &defaults={})
		{ return add(names, help, nargs, defaults); }

		Parser& add(
			const std::set<std::string> &names,
			const std::string &help="",
			const Nargs &nargs=1,
			const std::vector<std::string> &defaults={})
		{
			if (!names.size())
			{ throw std::logic_error("missing arg name"); }
			else if (
				defaults.size()
				&& (defaults.size() < nargs.lo || (nargs.hi && defaults.size() > nargs.hi)))
			{ throw std::logic_error(*names.begin()+" default count is invalid"); }

			ArgInfo info(names, help, nargs, defaults, this);
			if (ispos(info.longest()))
			{
				if (!info.nargs.hi)
				{ throw std::runtime_error("positional args max must be > 0"); }
				for (const auto &name : info.names)
				{
					if (!posnames.insert(name).second)
					{ throw std::runtime_error("Argument name conflict: " + name); }
				}
				auto it = args.insert(args.end(), info);
				if (posstart == args.end()) { posstart = it; }
				positionals.push_back(&*it);
			}
			else
			{
				auto it = args.insert(posstart, info);
				for (const auto &name : info.names)
				{
					if (ispos(name))
					{
						if (!posnames.insert(name).second)
						{
							throw std::runtime_error("Flag alias conflicted: " + name);
						}
					}
					else
					{
						if (!flags.insert({name, &*it}).second)
						{ throw std::runtime_error("Flag name conflicted: " + name); }
					}
				}
			}
			return *this;
		}

		//assume argc/argv are from main(), so ignore argv[0]
		Parsed parse(int argc, char *argv[]) const { return parse(argv+1, argv+argc); }
		template<class T>
		Parsed parse(T begin, T end) const
		{
			std::string ignoreflags(prefix+prefix);
			std::string shorthelp(prefix + "h");
			std::string fullhelp(prefix + prefix + "help");
			std::string stop(prefix + "/");

			Parsed result;
			std::map<std::string, const ArgInfo*> unparsed;
			for (const ArgInfo &info : args)
			{ unparsed[info.longest()] = &info; }
			std::vector<ArgInfo*>::const_iterator  pit = positionals.begin();
			unsigned long long skip = 0;
			while (begin != end)
			{
				const ArgInfo *infop;
				bool skipflag = isskip(*begin);
				bool notflag = ispos(*begin);
				if (skip || stop==*begin || ignoreflags==*begin || skipflag || notflag)
				{
					if (pit == positionals.end())
					{ throw std::runtime_error(std::string("unexpected argument ") + *begin); }
					infop = *(pit++);
					if (ignoreflags == *begin) { skip = NOLIM; ++begin; }
					else if (skipflag) { skip = std::stoull(&(*begin++)[prefix.size()]); }
				}
				else if (shorthelp == *begin) { doshorthelp(); }
				else if (!fullhelp.compare(0, fullhelp.size(), *begin, 0, fullhelp.size()))
				{
					std::size_t cols;
					try
					{
						cols = std::stoull(std::string(*begin).substr(fullhelp.size()));
					} catch (...) { dofullhelp(); }
					dofullhelp(cols);
				}
				else
				{
					auto it = flags.find(*(begin++));
					if (it == flags.end())
					{ throw std::runtime_error(std::string("unrecognized flag ") + *(--begin)); }
					infop = it->second;
				}
				if (infop->nargs.hi == 0) { add_boolflag(result, *infop); }
				else
				{
					unsigned long long target = infop->nargs.hi;
					std::vector<std::string> curargs;
					for (; begin!=end && curargs.size()<target; ++begin)
					{
						if (skip) { curargs.push_back(*begin); --skip; }
						else if (ispos(*begin)) { curargs.push_back(*begin); }
						else if (ignoreflags==*begin) { skip = NOLIM; }
						else if (isskip(*begin))
						{ skip = std::stoull(&(*begin)[prefix.size()]); }
						else if (stop==*begin) { ++begin; break; }
						else if (shorthelp == *begin) { doshorthelp(); }
						else if (!fullhelp.compare(0, fullhelp.size(), *begin, 0, fullhelp.size()))
						{
							std::size_t cols;
							try
							{
								cols = std::stoull(std::string(*begin).substr(fullhelp.size()));
							} catch (...) { dofullhelp(); }
							dofullhelp(cols);
						}
						else { break; }
					}
					if (curargs.size() < infop->nargs.lo)
					{ throw std::runtime_error("inadequate args for " + infop->longest()); }
					result.add(*infop, curargs);
				}
				unparsed.erase(infop->longest());
			}
			for (auto it : unparsed)
			{
				if (it.second->nargs.hi == 0) { add_boolflag(result, *it.second, 0); }
				else if (it.second->defaults.size() || it.second->nargs.lo == 0)
				{
					auto &info = *it.second;
					result.add(info, info.defaults);
				}
				else
				{ throw std::runtime_error("missing argument " + it.second->longest()); }
			}
			return result;
		}
		private:
			template<class T>
			bool ispos(const T &thing) const
			{ return prefix.compare(0, prefix.size(), thing, 0, prefix.size()); }
			bool isskip(const std::string &s) const
			{
				return (
					!ispos(s) && s.size() > prefix.size()
					&& s.find_first_not_of("0123456789", prefix.size()) == s.npos);
			}

			void add_boolflag(Parsed &result, const ArgInfo &info, bool found=1) const
			{
				if (info.defaults.size())
				{
					std::stringstream s(info.defaults[0]);
					bool defaultb;
					s >> defaultb;
					if (!s)
					{
						s.seekg(0, s.beg);
						s >> std::boolalpha >> defaultb;
						if (!s)
						{
							throw std::runtime_error(
								"bad default value for flag: " + info.defaults[0]);
						}
					}
					found = found != defaultb;
				}
				result.add(info, {found ? "1" : "0"});
			}

			std::string shortstr(const ArgInfo &info) const
			{
				auto argname = info.shortest();
				ss out(" ");
				bool optional = info.nargs.lo == 0 || info.defaults.size();
				if (optional) { out << "["; }
				out << info.shortest();
				if (info.nargs.hi == NOLIM)
				{ out << "("<<info.nargs.lo<<"-...)"; }
				else if (info.nargs.hi != info.nargs.lo)
				{ out << "(" << info.nargs.lo << "-" << info.nargs.hi << ")"; }
				else if (info.nargs.hi)
				{ out << "(" << info.nargs.hi << ")"; }
				if (optional) { out << "]"; }
				return out.str();
			}

			void doshorthelp(bool end=1) const
			{
				std::cerr << program_name;
				for (const auto &info : args)
				{ std::cerr << shortstr(info); }
				std::cerr << std::endl;
				if (end) { throw StopHelp("short help"); }
			}
			static std::string escape(const std::string &in)
			{
				std::string out;
				out.reserve(in.size());
				for (char c : in)
				{
					switch (c)
					{
						case '\\':
							out += "\\\\"; break;
						case '\n':
							out += "\\n"; break;
						case '\r':
							out += "\\r"; break;
						case '"':
							out += "\\\""; break;
						case '\t':
							out += "\\t"; break;
						default:
						{
							if (c != ' ' && !std::isprint(c))
							{
								const char *lut = "0123456789abcdef";
								out += "\\x";
								out += lut[(c >> 4) & 0xfu];
								out += lut[c & 0xfu];
							}
							else
							{ out += c; }
						}
					}
				}
				return out;
			}
			template<class T>
			static std::size_t wrap_print(
				const std::string &s, const std::string &indent, std::size_t curcol,
				std::size_t cols, T &out)
			{
				if (s.size() + curcol > cols)
				{
					out << std::endl << indent;
					curcol = indent.size();
				}
				auto toadd = s.substr(0, cols-curcol);
				curcol += toadd.size();
				out << toadd;
				std::size_t i = toadd.size();
				while (i < s.size())
				{
					toadd = s.substr(i, cols-indent.size());
					out << std::endl << indent << toadd;
					curcol = indent.size() + toadd.size();
					i += toadd.size();
				}
				return curcol;
			}

			void dofullhelp(std::size_t columns=80) const
			{
				if (columns <= 20)
				{ columns = 21; }
				int helpstart = 20;
				std::size_t helpcols = columns-helpstart;

				doshorthelp(0);
				std::cerr << std::endl << description << std::endl;
				for (const auto &info : args)
				{
					auto it = info.argnames.begin();
					ss line(*it++);
					for (; it!=info.argnames.end(); ++it)
					{ line << ", " << *it; }
					line << " (" << info.nargs.lo;
					if (info.nargs.hi == NOLIM)
					{ line << "-...)"; }
					else if (info.nargs.hi != info.nargs.lo)
					{ line << "-" << info.nargs.hi << ")"; }
					else { line << ")"; }
					std::cerr << line.str();
					if (line.s.tellp() >= helpstart)
					{ std::cerr << std::endl << std::string(helpstart, ' '); }
					else { std::cerr << std::string(helpstart-line.s.tellp(), ' '); }

					std::string helpstr = info.help;
					if (info.defaults.size())
					{
						line.s.str("");
						auto it = info.defaults.begin();
						line << "defaults: (\"" << escape(*it++);
						for (; it != info.defaults.end(); ++it)
						{ line << "\", \"" << escape(*it); }
						line << "\")";
						helpstr = line.str() + "\n" + info.help;
					}

					std::string indent(helpstart, ' ');
					std::size_t col = helpstart;
					std::size_t idx = 0;
					ss cerr;
					while (idx < helpstr.size())
					{
						std::size_t nxtidx = helpstr.find_first_of("\n\r \t", idx);
						if (nxtidx > helpstr.size()) { nxtidx = helpstr.size(); }
						if (nxtidx == idx)
						{
							char c = helpstr[idx++];
							if (c=='\n' || c=='\r')
							{ cerr << c << indent; col=helpstart; }
							else if (c=='\t')
							{
								col += 4; 
								if (col < columns) { cerr << "    "; }
							}
							else { if (col < columns) { cerr << ' '; ++col; } }
						}
						else
						{
							col = wrap_print(
								helpstr.substr(idx, nxtidx-idx), indent, col, columns, cerr);
							idx = nxtidx;
						}
					}
					std::cerr << cerr.str() << std::endl;
				}
				throw StopHelp("full help");
			}
	};
};

#ifndef ARGPARSE_NO_ARGMAIN
int argmain(int argc, char *argv[]);
int main(int argc, char *argv[])
{ return argparse::tryrun(argmain, argc, argv); }
#endif
#endif
