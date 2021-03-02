namespace os
{
	//------------------------------
	//Path
	//------------------------------
	inline Path::Path(const std::string &s):
		ebuff(),
		p(os__Path__make(s.c_str(), ebuff))
	{
		if (!p)
		{ raise("creation error: "); }
	}
	inline Path::Path(const Path &o):
		ebuff(), p(os__Path__copy(o.p, ebuff))
	{
		if (!p)
		{ raise("copy error: "); }
	}

	inline Path Path::operator+(const std::string &s)
	{
		Path p(*this);
		p += s;
		return p;
	}
	inline Path& Path::operator+=(const std::string &s)
	{
		if (os__Path__add(p, s.c_str(), ebuff))
		{ raise("add error: "); }
		return *this;
	}

	inline bool Path::operator==(const Path &o) const noexcept
	{
		return std::strcmp(
			os__Path__c_str(p), os__Path__c_str(o.p)) == 0;
	}

	inline void Path::pop_ext()
	{ os__Path__pop_ext(p); }
	inline void Path::add_ext(const std::string &s)
	{ 
		if (os__Path__add_ext(p, s.c_str(), ebuff))
		{ raise("add ext error: "); }
	}
	inline void Path::dirname()
	{ os__Path__dirname(p); }

	//------------------------------
	//Dirlist
	//------------------------------
	inline Dirlist::Dirlist(const std::string &s):
		ebuff(),
		data(os__Dirlist__make(s.c_str(), ebuff))
	{	
		if (!data) { raise("open: "); }
	}

	inline std::string Dirlist::next()
	{
		ebuff.buf[0] = 0;
		const char *dat = os__Dirlist__next(data, ebuff);
		if (dat)
		{ return dat; }
		else
		{ raise("next: "); }
	}

	inline Filenode::Filenode(const std::string &path):
		ebuff{},
		info(os__Filenode__make(path.c_str(), ebuff))
	{
		if (info < 0)
		{ throw std::runtime_error("filenode error: " + ebuff.buf); }
	}

	inline void mkdir(const std::string &s)
	{
		ebuf ebuff;
		if (os__mkdir(s.c_str(), ebuff))
		{ throw std::runtime_error(ebuff.buf); }
	}

	inline void makedirs(const std::string &dname)
	{
		ebuf ebuff;
		if (os__makedirs(dname.c_str(), ebuff))
		{ throw std::runtime_error(ebuff.buf); }
	}

	inline void rename(const std::string &before, const std::string &after)
	{
		ebuf ebuff;
		if (os__rename(before.c_str(), after.c_str(), ebuff))
		{ throw std::runtime_error(ebuff.buf); }
	}

	inline void remove(const std::string &path)
	{
		ebuf ebuff;
		if (os__remove(path.c_str(), ebuff))
		{ throw std::runtime_error(ebuff.buf); }
	}

	inline std::string get_sysdir(const std::string &name)
	{ return os__get_sysdir(name.c_str()); }

	inline void hide_window() { os__hide_window(); }
	inline void show_window() { os__show_window(); }


	inline Term::Term(const std::string &mode):
		tptr(os__term_open(mode.c_str()))
	{
		if (!tptr)
		{ throw std::runtime_error("failed to open terminal"); }
	}
	inline Term::~Term(){ os__term_close(tptr); }

	inline std::string Term::readline(bool strip, int chunksize)
	{
		std::string buf(chunksize, '\0');
		std::size_t start = 0;
		char *s;
		while (s = ::fgets(&buf[start], static_cast<int>(buf.size() - start), tptr))
		{
			char check = buf[buf.size() - 2];
			if (check && check != '\n')
			{
				start = buf.size() - 1;
				buf.resize(buf.size() + chunksize);
			}
			else
			{
				buf.resize(start + std::strlen(s));
				if (strip)
				{
					while (buf.size() && buf.find_last_of("\r\n") == buf.size() - 1)
					{ buf.pop_back(); }
				}
				return buf;
			}
		}
		if (::feof(tptr))
		{ return ""; }
		else
		{ throw std::runtime_error("error reading from terminal"); }
	}
	inline void Term::hide_input()
	{
		ebuf ebuff;
		if (os__hide_input(tptr, ebuff))
		{ throw std::runtime_error(ebuff.buf); }
	}
	inline void Term::show_input()
	{
		ebuf ebuff;
		if (os__show_input(tptr, ebuff))
		{ throw std::runtime_error(ebuff.buf); }
	}

	inline void set_env(const std::string &val, const std::string &var)
	{
		ebuf ebuff;
		if (os__set_env(val.c_str(), var.c_str(), ebuff))
		{ throw std::runtime_error(ebuff.buf); }
	}

	inline std::string get_env(const std::string &var)
	{
		ebuf ebuff;
		const int defaultsize = 256;
		std::string ret(defaultsize, '\0');
		int res = os__get_env(&ret[0], var.c_str(), defaultsize, ebuff);
		if (res > defaultsize)
		{
			ret.resize(res);
			res = os__get_env(&ret[0], var.c_str(), ret.size(), ebuff);
		}
		if (res >= 0)
		{
			ret.resize(res);
			return ret;
		}
		else if (res == -1)
		{ throw NotFound(var + " not found"); }
		else
		{ throw std::runtime_error(ebuff.buf); }
	}

}
