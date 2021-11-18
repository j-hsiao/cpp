#include <stdlib.h>
#include <stdio.h>

#include <map>
#include <string>
#include <system_error>
#include <utility>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
namespace {
	const std::error_condition noent = std::make_error_condition(
		std::errc::no_such_file_or_directory);
	const std::error_condition exists = std::make_error_condition(
		std::errc::file_exists);
}
namespace os
{
/*
Paths
*/
	Path Path::sysdir(const std::string &id)
	{
		if (id == ".")
		{ return Path(".").abs(); }
		static std::map<std::string, Path> dirs;
		if (!dirs.size())
		{
			dirs.insert({"", {""}});
			try
			{
				Path home(getenv("HOME"));
				dirs.insert({"home", home});
				dirs.insert({"downloads", home + "Downloads"});
				dirs.insert({"pictures", home + "Pictures"});
				dirs.insert({"documents", home + "Documents"});
				dirs.insert({"videos", home + "Videos"});
				dirs.insert({"desktop", home + "Desktop"});
			}
			catch (std::system_error&)
			{}
		}
		auto it = dirs.find(id);
		if (it == dirs.end())
		{ throw std::system_error(noent.value(), noent.category(), id); }
		return it->second;
	}
	Path& Path::abs()
	{
		char *pth = ::realpath(path.c_str(), nullptr);
		if (pth)
		{
			path = pth;
			::free(pth);
			return *this;
		}
		throw std::system_error(errno, std::generic_category(), path);
	}
/*
ENV
*/
	void setenv(const std::string &name, const char *val)
	{
		if (val)
		{
			if (::setenv(name.c_str(), val, 1))
			{ throw std::system_error(errno, std::generic_category(), name+"->"+val); }
		}
		else
		{
			if (::unsetenv(name.c_str()))
			{ throw std::system_error(errno, std::generic_category(), "deleting "+name); }
		}
	}
	std::string getenv(const std::string &name)
	{
		char *val = ::getenv(name.c_str());
		if (val)
		{ return val; }
		throw std::system_error(
			std::make_error_code(std::errc::no_such_file_or_directory));
	}
/*
Dirlist
*/
	struct Dirlist::iterator_type::info
	{
		DIR *d;
		std::string val;
		info(const std::string &s):
			d(::opendir(s.c_str())), val{}
		{
			if (!d) { throw std::system_error(errno, std::generic_category(), s); }
			next();
		}

		void next()
		{
			errno = 0;
			struct dirent *ent = ::readdir(d);
			if (ent)
			{
				val = ent->d_name;
				if (val=="." || val=="..")
				{ next(); }
			}
			else
			{
				if (!errno) { val = ""; }
				else { throw std::system_error(errno, std::generic_category()); }
			}
		}
		~info() { ::closedir(d); }
	};
	Dirlist::iterator_type::iterator_type(): data{} {}
	Dirlist::iterator_type::iterator_type(const std::string &dname):
		data{new info{dname}}
	{}
	Dirlist::iterator_type::iterator_type(iterator_type &&o)
	{ data = std::move(o.data); }
	Dirlist::iterator_type::~iterator_type()
	{ data.reset(nullptr); }

	std::string Dirlist::iterator_type::operator*() const
	{ return data? data->val : ""; }
	Dirlist::iterator_type& Dirlist::iterator_type::operator++()
	{
		data->next();
		if (data->val == "")
		{ data.reset(nullptr); }
		return *this;
	}
	bool Dirlist::iterator_type::operator!=(const iterator_type& o)
	{ return !data != !o.data; }
/*
Filenode
*/
	Filenode::Filenode(const std::string &name):
		flags(0)
	{
		struct stat info;
		if (::stat(name.c_str(), &info))
		{
			std::error_code code(errno, std::generic_category());
			if (code != noent) { throw std::system_error(code, name); }
		}
		else
		{ flags = info.st_mode & S_IFMT; }
	}
	Filenode::operator bool() const
	{ return flags; }
	bool Filenode::isdir() const
	{ return flags == S_IFDIR; }
	bool Filenode::isfile() const
	{ return flags == S_IFREG; }

	void mkdir(const std::string &path)
	{
		if (::mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
		{
			std::error_code code(errno, std::generic_category());
			if (code != exists)
			{ throw std::system_error(code, path); }
		}
	}

	void rename(const std::string &name, const std::string &to)
	{
		std::string out;
		Filenode dst(to);
		if (dst)
		{
			out = (Path(to) + Path(name).basename()).path;
			if (!dst.isdir() || Filenode(out))
			{ throw std::system_error(exists.value(), exists.category(), "rename "+name+"->"+to); }
		}
		else { out = to; }
		if (::rename(name.c_str(), out.c_str()))
		{ throw std::system_error(errno, std::generic_category(), "rename "+name+"->"+to); }
	}
	void remove(const std::string &path)
	{
		Filenode node(path);
		if (node)
		{
			int errored;
			if (node.isdir())
			{ errored = ::rmdir(path.c_str()); }
			else
			{ errored = ::unlink(path.c_str()); }
			if (errored)
			{
				std::error_code code(errno, std::generic_category());
				if (code != noent) { throw std::system_error(code, path); }
			}
		}
	}
	void hide_window() {}
	void show_window() {}

	Term::Term(const std::string &mode): tptr(::fopen("/dev/tty", mode.c_str()))
	{ if (!tptr) { throw std::system_error(errno, std::generic_category(), "/dev/tty"); } }
	Term::~Term() { if (tptr) { ::fclose(tptr); }}
	bool Term::show_input(bool show)
	{
		struct termios current;
		int fd = ::fileno(tptr);
		if (fd == -1) { throw std::system_error(errno, std::generic_category(), "get tty fd"); }
		if (::tcgetattr(fd, &current) == -1)
		{ throw std::system_error(errno, std::generic_category(), "get tty attrs"); }
		bool ret = current.c_lflag & ECHO;
		if (show)
		{ current.c_lflag |= ECHO; }
		else
		{ current.c_lflag &= ~ECHO; }
		if (tcsetattr(fd, TCSAFLUSH, &current) == -1)
		{ throw std::system_error(errno, std::generic_category(), "set tty attrs"); }
		return ret;
	}
}
