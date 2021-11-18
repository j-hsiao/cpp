#include "os/os.hpp"

#include <cctype>
#include <iterator>
#include <string>
#include <vector>

#include "os/tlog.hpp"
#include "os/os_dllcfg.h"

#if defined(_WIN32)
#	include "./os_win.cpp"
#else
#	include "./os_posix.cpp"
#endif

namespace os
{
	namespace
	{
		bool issep(char c)
		{ return Path::Pathseps.find(c) != Path::Pathseps.npos; }

		std::string squeeze_seps(const std::string &path)
		{
			std::size_t pos = 0;
	#ifdef _WIN32
			if (!path.compare(0, 4, "\\\\?\\"))
			{
				pos = 4;
			}
	#endif
			std::string fixsep;
			while (pos < path.size())
			{
				std::size_t nxt = path.find_first_of(Path::Pathseps, pos);
				fixsep += path.substr(pos, nxt-pos);
				if (nxt != path.npos)
				{
					fixsep += Path::Pathseps[0];
					pos = path.find_first_not_of(Path::Pathseps, nxt);
				}
				else
				{ pos = nxt; }
			}
			return fixsep;
		}

		std::string norm_root(const std::string &path)
		{
			// /C/... -> C:\...
			// / -> sysdir(".")[:2]
			// /asdf -> sysdir(".")[:2]\asdf
	#		ifdef _WIN32
			if (path.size() && issep(path[0]))
			{
				if (
					path.size() > 1 && std::isalpha(path[1])
					&& (path.size() == 2 || issep(path[2])))
				{
					auto ret = path;
					ret[0] = std::toupper(path[1]);
					ret[1] = ':';
					return ret;
				}
				else
				{ return Path::sysdir(".").path.substr(0, 2) + path; }
			}
	#		endif
			return path;
		}
	}


/*
Path
*/
#	ifdef _WIN32
	const std::string Path::Pathseps = "\\/";
#	else
	const std::string Path::Pathseps = "/";
#	endif
	bool Path::isroot(const std::string &s)
	{
		return (s.size() && issep(s[0]))
#		ifdef _WIN32
			|| (s.size() > 1 && std::isalpha(s[0]) && s[1] == ':'
				&& (s.size() == 2 || issep(s[2])))
#		endif
		;
	}

	Path& Path::normalize()
	{
		path = norm_root(squeeze_seps(path));
		if (path.size() > 1 && issep(path.back()))
		{ path.pop_back(); }
		bool rooted = isroot(path);
		std::vector<std::string> segments;

		std::size_t pos = path.find_first_of(Pathseps, 0);
		auto seg = path.substr(0, pos);
		if (seg != ".") { segments.push_back(seg); }
		if (pos != path.npos) { ++pos; }
		while (pos < path.size())
		{
			std::size_t nxt = path.find_first_of(Pathseps, pos);
			seg = path.substr(pos, nxt-pos);
			if (seg == "..")
			{
				if (
					segments.size()>static_cast<std::size_t>(rooted)
					&& segments.back() != "..")
				{ segments.pop_back(); }
				else if (!rooted)
				{ segments.push_back(seg); }
			}
			else if (seg != ".")
			{ segments.push_back(seg); }
			pos = nxt==path.npos ? nxt : nxt+1;
		}
		if (segments.size())
		{
			path = segments[0];
			if (segments.size() > 1)
			{
				for (auto it=std::next(segments.begin()); it!=segments.end(); ++it)
				{ path.append(1, Pathseps[0]).append(*it); }
			}
			else
			{ if (!path.size()) { path = Pathseps[0]; } }
		}
		else
		{ path = "."; }
		return *this;
	}

	Path Path::operator+(const std::string &s) const
	{
		Path ret = *this;
		return ret += s;
	}
	Path& Path::operator+=(const std::string &s)
	{
		if (isroot(s)) { path = s; }
		else if (s.size())
		{
			path += Pathseps[0];
			path += s;
		}
		return *this;
	}

	void Path::pop_ext()
	{
		auto base = basename();
		if (base=="." || base=="..") { return; }
		auto lastdot = path.rfind('.');
		if (lastdot == path.npos) { return; }
		auto lastpathsep = path.find_last_of(Pathseps);
		if (lastpathsep == path.npos || lastdot > lastpathsep)
		{ path.resize(lastdot); }
	}
	void Path::pop_base()
	{
		auto idx = path.find_last_of(Pathseps);
		if (idx && idx != path.npos)
		{ path.resize(idx); }
		else if (!idx)
		{ path = Pathseps.substr(0, 1); }
		else if (!isroot())
		{ path = "."; }
	}

	std::string Path::basename() const
	{
		auto idx = path.find_last_of(Pathseps);
		if (idx != path.npos)
		{ return path.substr(idx+1); }
		if (isroot())
		{ return ""; }
		else { return path; }
	}
	std::string Path::ext() const
	{
		auto base = basename();
		if (base=="." || base=="..") { return ""; }
		auto lastdot = base.rfind('.');
		if (lastdot == path.npos) { return ""; }
		else
		{ return base.substr(lastdot); }
	}

	void makedirs(const std::string &path)
	{
		Path p(path);
		std::vector<std::string> stack;
		while (!Filenode(p.path))
		{
			stack.push_back(p.basename());
			p.pop_base();
		}
		while (stack.size())
		{
			p += stack.back();
			mkdir(p.path);
			stack.pop_back();
		}
	}

	std::string Term::readline(bool strip, int chunksize)
	{
		if (chunksize <= 0) { chunksize = 256; }
		std::string buf(chunksize, '\n');
		std::size_t pos = 0;
		while (::fgets(&buf[pos], static_cast<int>(buf.size() - pos), tptr))
		{
			//fgets ALWAYS adds nullptr 
			std::size_t ed = buf.find_last_of('\0');
			if (ed == buf.npos || ed <= pos)
			{ throw std::logic_error("\\0 should always be added"); }
			else if (::feof(tptr))
			{
				::clearerr(tptr);
				buf.resize(ed);
				return buf;
			}
			else
			{
				std::size_t nl = buf.find_first_of('\n', pos);
				if (nl == buf.npos)
				{
					if (ed != buf.size()-1)
					{ throw std::logic_error("not eof and no newline implies full buffer"); }
					else
					{
						pos = buf.size()-1;
						buf.resize(buf.size()+chunksize, '\n');
					}
				}
				else
				{
					buf.resize(nl + !strip);
					if (strip && buf.back() == '\r') { buf.pop_back(); }
					return buf;
				}
			}
		}
		if (::feof(tptr))
		{
			::clearerr(tptr);
			buf.resize(pos);
			return buf;
		}
		else { throw std::runtime_error("terminal read error"); }
	}
}

#include <cstring>
namespace
{
	void load_error(os_ebuf *buf, const char *data)
	{
		if (buf)
		{
			std::strncpy(buf->buf, data, buf->size);
			if (buf->size) { buf->buf[buf->size - 1] = 0; }
		}
	}
}

#define ebegin try {
#define eend } catch (std::exception &e) \
	{\
		load_error(ebuf, e.what()); \
	} catch (...) { \
		load_error(ebuf, "unknown"); \
	}
CPP_EXTERNC_BEGIN

os_Path* os_Path_make(const char *path, os_ebuf *ebuf)
{
	ebegin
	return reinterpret_cast<os_Path*>(new os::Path(path));
	eend
	return nullptr;
}
void os_Path_free(os_Path *p)
{ delete reinterpret_cast<os::Path*>(p); }

int os_Path_abs(os_Path *p, os_ebuf *ebuf)
{
	ebegin
	reinterpret_cast<os::Path*>(p)->abs();
	return 0;
	eend
	return 1;
}
int os_Path_normalize(os_Path *p, os_ebuf *ebuf)
{
	ebegin
	reinterpret_cast<os::Path*>(p)->normalize();
	return 0;
	eend
	return 1;
}
int os_Path_add(os_Path *p, const char *extra, os_ebuf *ebuf)
{
	ebegin
	*reinterpret_cast<os::Path*>(p) += extra;
	return 0;
	eend
	return 1;
}
int os_Path_add_ext(os_Path *p, const char *ext, os_ebuf *ebuf)
{
	ebegin
	reinterpret_cast<os::Path*>(p)->path += ext;
	return 0;
	eend
	return 1;
}
void os_Path_pop_ext(os_Path *p)
{ reinterpret_cast<os::Path*>(p)->pop_ext(); }
void os_Path_pop_base(os_Path *p)
{ reinterpret_cast<os::Path*>(p)->pop_base(); }

bool os_Path_isroot(const os_Path *p)
{ return reinterpret_cast<const os::Path*>(p)->isroot(); }
const char* os_Path_c_str(const os_Path *p)
{ return reinterpret_cast<const os::Path*>(p)->path.c_str(); }
const char* os_Path_base(const os_Path *p)
{
	const os::Path &self = *reinterpret_cast<const os::Path*>(p);
	auto idx = self.path.find_last_of(self.Pathseps);
	if (idx != self.path.npos)
	{ return self.path.c_str() + idx + 1; }
	if (self.isroot())
	{ return ""; }
	else { return self.path.c_str(); }
}
const char* os_Path_ext(const os_Path *p)
{
	const os::Path &self = *reinterpret_cast<const os::Path*>(p);
	const char *bs = os_Path_base(p);
	if (!std::strcmp(bs, "..") || !std::strcmp(bs, "."))
	{ return ""; }
	auto lastdot = self.path.rfind('.');
	auto lastsep = self.path.find_last_of(self.Pathseps);
	if (lastsep == self.path.npos) { lastsep = 0; }
	if (lastdot == self.path.npos || lastdot < lastsep) { return ""; }
	else
	{ return self.path.c_str() + lastdot; }
}
size_t os_Path_size(const os_Path *p)
{ return reinterpret_cast<const os::Path*>(p)->size(); }

const char* os_sysdir(const char *id, os_ebuf *ebuf)
{
	static std::map<std::string, std::string> mp;
	ebegin
	auto it = mp.find(id);
	if (it == mp.end())
	{
		std::string ret = os::Path::sysdir(id).path;
		return mp.insert({id, ret}).first->second.c_str();
	}
	else
	{ return it->second.c_str(); }
	eend
	return nullptr;
}

struct os_Dirlist
{
	os::Dirlist::iterator_type start;
	os::Dirlist::iterator_type stop;
	std::string cur;
};
os_Dirlist* os_Dirlist_make(const char *path, os_ebuf *ebuf)
{
	ebegin
	os::Dirlist d(path);
	return reinterpret_cast<os_Dirlist*>(
		new os_Dirlist{d.begin(), d.end(), {}});
	eend
	return nullptr;
}
void os_Dirlist_free(os_Dirlist *d)
{ delete reinterpret_cast<os::Dirlist::iterator_type*>(d); }
const char* os_Dirlist_next(os_Dirlist *d, os_ebuf *ebuf)
{
	ebegin
	if (d->start != d->stop)
	{
		d->cur = *d->start;
		++(d->start);
		return d->cur.c_str();
	}
	else
	{
		if (ebuf && ebuf->size) { ebuf->buf[0] = 0; }
		return nullptr;
	}
	eend
	return nullptr;
}

os_Filenode os_Filenode_make(const char *path, os_ebuf *ebuf)
{
	ebegin
	return os::Filenode(path).flags;
	eend
	return 0;
}
bool os_Filenode_exists(os_Filenode node)
{ return os::Filenode(node); }
bool os_Filenode_is_dir(os_Filenode node)
{ return os::Filenode(node).isdir(); }
bool os_Filenode_is_file(os_Filenode node)
{ return os::Filenode(node).isfile(); }


int os_mkdir(const char *path, os_ebuf *ebuf)
{
	ebegin
	os::mkdir(path);
	return 0;
	eend
	return 1;
}
//recursive makedirs
int os_makedirs(const char *path, os_ebuf *ebuf)
{
	ebegin
	os::makedirs(path);
	return 0;
	eend
	return 1;
}
//works for directories and files
int os_rename(const char *before, const char *after, os_ebuf *ebuf)
{
	ebegin
	os::rename(before, after);
	return 0;
	eend
	return 1;
}
int os_remove(const char *path, os_ebuf *ebuf)
{
	ebegin
	os::remove(path);
	return 0;
	eend
	return 1;
}

void os_hide_window()
{ try{ os::hide_window(); } catch(...){} }
void os_show_window()
{ try{ os::show_window(); } catch(...){} }

os_Term* os_Term_open(const char *mode)
{
	try
	{ return reinterpret_cast<os_Term*>(new os::Term(mode)); }
	catch (...)
	{ return nullptr; }
}
void os_Term_free(os_Term *term)
{
	try
	{ delete reinterpret_cast<os::Term*>(term); }
	catch (...)
	{}
}
int os_hide_input(os_Term *t, os_ebuf *ebuf)
{
	ebegin
	return reinterpret_cast<os::Term*>(t)->show_input(0);
	eend
	return 0x02;
}
int os_show_input(os_Term *t, os_ebuf *ebuf)
{
	ebegin
	return reinterpret_cast<os::Term*>(t)->show_input(1);
	eend
	return 0x02;
}
char* os_Term_readline(os_Term *term, size_t *sz, bool strip)
{
	try
	{
		auto *t = reinterpret_cast<os::Term*>(term);
		std::string ret = t->readline(strip);
		char *r = new char[ret.size()];
		std::memcpy(r, ret.c_str(), ret.size());
		if (sz) { *sz = ret.size(); }
		return r;
	}
	catch (...)
	{ return nullptr; }
}
void os_Term_linefree(char *p)
{ if (p) { delete[] p; } }


int os_setenv(const char *name, const char *value, os_ebuf *ebuf)
{
	ebegin
	os::setenv(name, value);
	return 0;
	eend
	return 1;
}
int os_getenv(const char *name, char *buf, size_t bufsize, os_ebuf *ebuf)
{
	try
	{
		std::string ret = os::getenv(name);
		if (bufsize >= ret.size())
		{ std::memcpy(buf, ret.c_str(), ret.size()); }
		return static_cast<int>(ret.size());
	}
	catch (std::system_error &e)
	{
		if (
			e.code().default_error_condition()
			== std::make_error_condition(std::errc::no_such_file_or_directory))
		{ return -1; }
		else
		{
			load_error(ebuf, e.what());
			return -2;
		}
	}
	catch (std::exception &e)
	{
		load_error(ebuf, e.what());
		return -2;
	}
	catch (...)
	{
		load_error(ebuf, "unknown");
		return -2;
	}
}
CPP_EXTERNC_END
