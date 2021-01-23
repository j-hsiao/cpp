#ifndef ARGPARSE_IMPL_HPP
#define ARGPARES_IMPL_HPP

namespace argparse
{
	// ------------------------------
	// helper functors/functions
	// ------------------------------
	inline std::string ltrimmed(
		const std::string &val, const std::string &remove);

	struct IsFlag
	{
		bool operator()(const std::string &s) const noexcept
		{ return s.size() && s[0] == '-'; }
	};

	struct IsOp
	{
		bool operator()(const std::string &s) const noexcept
		{
			bool postnumeric = s.find_first_not_of("0123456789", 1) == s.npos;
			return IsFlag()(s) && (s == "--" || postnumeric);
		}
	};
	struct lencmp
	{
		bool operator()(const std::string &a, const std::string &b) const noexcept
		{ return a.size() < b.size(); }
	};

	template<class Func>
	inline int run(Func &&func, int argc, char *argv[])
	{
		try
		{ return func(argc, argv); }
		catch (StopParse&)
		{ return 0; }
		catch (std::exception &exc)
		{
			std::cerr << "terminate after error: \""
				<< exc.what() << '"' << std::endl;
		}
		catch (...)
		{ std::cerr << "terminate after unknown error" << std::endl; }
		return 1;
	}

	inline std::string ltrimmed(
		const std::string &val, const std::string &remove)
	{
		std::size_t start = val.find_first_not_of(remove);
		return start == val.npos ? "" : val.substr(start);
	}

	template<class T>
	inline std::string join(const T &strs, std::string delim)
	{
		std::string ret;
		for (const std::string &s : strs)
		{
			ret += s;
			ret += delim;
		}
		if (ret.size())
		{
			ret.resize(ret.size() - delim.size());
		}
		return ret;
	}

	typedef std::vector<std::string> strvec;

	inline strvec::const_iterator Value::begin() const
	{ return values.begin(); }
	inline strvec::const_iterator Value::end() const
	{ return values.end(); }

	inline Value Value::operator()(std::size_t i) const
	{
		has(i);
		return {name + "[" + std::to_string(i) + "]", {values[i]}};
	}

	inline const std::string& Value::operator[](std::size_t i) const
	{
		if (i < values.size()) { return values[i]; }
		throw std::runtime_error(
			name + " index " + std::to_string(i) + " out of range");
	}


	inline void Value::has(std::size_t i) const
	{
		if (i >= values.size())
		{
			throw std::runtime_error(
				"index " + std::to_string(i) + " out of range for " + name);
		}
	}

	template<class T>
	inline Value::operator T() const
	{
		has(0);
		return this->as<T>();
	}

	template<class T>
	inline Value::operator std::vector<T>() const
	{
		std::vector<T> ret;
		for (std::size_t i = 0; i < values.size(); ++i)
		{ ret.push_back(as<T>(i)); }
		return ret;
	}

	// ------------------------------
	// value
	// ------------------------------

	template<class T>
	inline T Value::as(std::size_t i) const
	{
		has(i);
		std::stringstream ss(values[i]);
		T ret;
		ss >> ret;
		if (!ss)
		{
			throw std::runtime_error(
				"failed to convert " + name + "[" + std::to_string(i) + "]");
		}
		return ret;
	}
	template<>
	inline std::string Value::as<std::string>(std::size_t i) const
	{ return values[i]; }
	template<>
	inline const char* Value::as<const char*>(std::size_t i) const
	{ return values[i].c_str(); }
	template<>
	inline char* Value::as<char*>(std::size_t i) const;

	// ------------------------------
	// Argument
	// ------------------------------
	inline Argument::Argument(
		const std::set<std::string> &names_,
		const std::string &help_,
		int nargs_,
		const std::vector<std::string> &defaults_
	):
		defaults(defaults_),
		names(names_),
		help(help_),
		isFlag(std::all_of(names.begin(), names.end(), IsFlag())),
		nargs(((!isFlag && nargs_ == 0) || nargs_ < 0) ? std::string::npos : static_cast<std::size_t>(nargs_))
	{
		if (defaults.size() && defaults.size() != nargs && nargs != std::string::npos && nargs != 0)
		{
			throw std::runtime_error("defaults should match nargs");
		}
		if (!names.size())
		{ throw std::runtime_error("arguments require at least 1 name"); }
		if (!isFlag && std::any_of(names.begin(), names.end(), IsFlag()))
		{
			throw std::runtime_error("arguments must be all flags or all positional");
		}
	}

	inline bool Argument::falsey(const std::string &s)
	{
		static const std::set<std::string> f{"", "0", "false"};
		return f.count(s) > 0;
	}

	inline Value Argument::operator()() const
	{
		if (nargs == 0)
		{
			if (!defaults.size() || falsey(defaults[0]))
			{ return {name(), {"0"}}; }
			else
			{ return {name(), {"1"}}; }
		}
		else
		{
			auto dummy = defaults.begin();
			return (*this)(dummy, defaults.end());
		}
	}
	template<class T>
	inline Value Argument::operator()(T &it, const T &end) const
	{
		std::size_t skip = 0;
		if (nargs == 0)
		{
			//boolean flag
			if (!defaults.size() || falsey(defaults[0]))
			{ return {name(), {"1"}}; }
			else
			{ return {name(), {"0"}}; }
		}
		else
		{
			std::vector<std::string> vals;
			while (vals.size() < nargs && it != end)
			{
				if (skip)
				{
					vals.push_back(*it);
					--skip;
				}
				else if (IsFlag()(*it))
				{
					if (IsOp()(*it))
					{
						if ((*it) == "--")
						{ skip = std::string::npos; }
						else
						{
							skip = stoull(it->substr(1));
							if (skip == 0)
							{
								++it;
								break;
							}
						}
					}
					else
					{ break; }
				}
				else
				{
					vals.push_back(*it); 
				}
				++it;
			}
			if (nargs != std::string::npos)
			{
				while (vals.size() < nargs && vals.size() < defaults.size())
				{ vals.push_back(defaults[vals.size()]); }
			}
			else if (vals.size() == 0)
			{ vals = defaults; }
			if (vals.size() < nargs && nargs != std::string::npos)
			{ throw std::runtime_error(name() + " is missing arguments"); }
			return {name(), vals};
		}
	}

	inline const std::string Argument::helpstr(bool verbose) const
	{
		if (verbose)
		{
			std::stringstream ss;
			ss << "[" << join(names, " ") << "]: ";
			if (nargs == std::string::npos)
			{ ss << "*"; }
			else if (nargs == 0)
			{ ss << "T/F"; }
			else
			{ ss << nargs; }
			if (nargs == 0)
			{
				bool boodefault = defaults.size() && !falsey(defaults[0]);
				ss << ", (" << boodefault << ")";
			}
			else if (defaults.size())
			{
				ss << ", (\""
					<< join(defaults, "\", \"") << "\")";
			}
			ss << std::endl << '\t' << help;
			return ss.str();
		}
		else
		{
			auto shortname = *std::min_element(names.begin(), names.end(), lencmp());
			std::string enclose;
			if (nargs == 0 || defaults.size() || nargs == std::string::npos)
			{ enclose = "[]"; }
			else
			{ enclose = "<>"; }
			std::string ret;
			if (nargs == std::string::npos)
			{ return enclose[0] + shortname + " ..." + enclose[1]; }
			else
			{
				std::string ret =  enclose[0] + shortname;
				if (nargs > 1)
				{ ret += " x" + std::to_string(nargs); }
				return ret + enclose[1];
			}
		}
	}

	inline const std::string& Argument::name() const
	{ return *std::max_element(names.begin(), names.end(), lencmp()); }

	// ------------------------------
	// Arg values
	// ------------------------------
	inline Args::Args(
		const std::map<std::string, std::size_t> &inds_,
		const std::vector<Value> &args_
	):
		inds(inds_),
		args(args_)
	{}

	inline const Value& Args::operator[](const std::string &name) const
	{
		auto it = inds.find(name);
		if (it != inds.end())
		{ return args[it->second]; }
		else
		{ throw std::runtime_error("no argument named " + name); }
	}

	// ------------------------------
	// parser
	// ------------------------------
	inline Parser::Parser(
		const std::vector<Argument> &args,
		const std::string &help_,
		const std::string progName
	):
		allNames{"help", "h"},
		positionals{},
		flags{},
		help(help_),
		programName(progName)
	{
		for (const Argument &arg : args)
		{ add(arg); }
	}

	inline void Parser::add(const Argument &arg)
	{
		if (arg.isFlag)
		{
			for (const std::string &name : arg.names)
			{
				std::string base = ltrimmed(name, "-");
				if (allNames.count(base))
				{
					throw std::runtime_error("name " + name + " already used");
				}
				allNames.insert(base);
				flagmap.insert({name, flags.size()});
			}
			flags.push_back(arg);
		}
		else
		{ positionals.push_back(arg); }
	}

	inline Args Parser::dohelp(
		bool verbose, bool exit, const std::string &progname) const
	{
		std::cout << (progname.size() ? progname : programName);
		for (auto &flag : flags)
		{
			std::cout << ' ' << flag.helpstr(0);
		}
		for (auto &pos : positionals)
		{
			std::cout << ' ' << pos.helpstr(0);
		}
		std::cout << std::endl;
		if (verbose)
		{
			if (help.size())
			{ std::cout << std::endl << help << std::endl; }
			for (auto &flag : flags)
			{
				std::cout << std::endl << flag.helpstr(1) << std::endl;
			}
			for (auto &pos : positionals)
			{
				std::cout << std::endl << pos.helpstr(1) << std::endl;
			}
		}
		if (exit)
		{ throw StopParse(); }
		else
		{ return {{}, {}}; }
	}

	template<class T>
	inline Args Parser::parse(
		T begin,
		const T &end,
		bool exit,
		const std::string &progname) const
	{
		std::size_t pos = 0;
		std::set<std::string> flagq;
		for (const Argument &flag : flags)
		{ flagq.insert(flag.name()); }
		std::vector<Value> vals;
		std::map<std::string, std::size_t> inds;

		while (begin != end)
		{
			const Argument * pick = nullptr;
			if (IsFlag()(*begin) && !IsOp()(*begin))
			{
				if (ltrimmed(*begin, "-") == "h" || ltrimmed(*begin, "-") == "help")
				{ return dohelp(ltrimmed(*begin, "-") == "help", exit, progname); }
				auto it = flagmap.find(*begin);
				if (it == flagmap.end())
				{ throw std::runtime_error("unrecognized flag " + (*begin)); }
				++begin;
				pick = &flags[it->second];
				flagq.erase(pick->name());
			}
			else
			{
				if (pos < positionals.size())
				{
					pick = &positionals[pos];
					++pos;
				}
				else
				{ throw std::runtime_error("extra argument: " + *begin); }
			}
			for (const std::string &name : pick->names)
			{ inds.insert({ltrimmed(name, "-"), vals.size()}); }
			vals.push_back((*pick)(begin, end));
		}
		for (; pos < positionals.size(); ++pos)
		{
			for (const auto &name : positionals[pos].names)
			{ inds.insert({ltrimmed(name, "-"), vals.size()}); }
			vals.push_back(positionals[pos]());
		}
		for (const auto &s : flagq)
		{
			const auto &arg = flags[flagmap.at(s)];
			for (const auto &name : arg.names)
			{ inds.insert({ltrimmed(name, "-"), vals.size()}); }
			vals.push_back(arg());
		}
		return {inds, vals};
	}

	inline Args Parser::parsemain(int argc, char *argv[]) const
	{
		std::vector<std::string> strs(argv,  argv + argc);
		return parse(strs.begin() + 1, strs.end(), 1, strs[0]);
	}
}
#endif
