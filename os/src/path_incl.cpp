#include <os/os.h>
#include <.os/util.hpp>

#include <cstring>
#include <exception>
#include <cctype>
#include <cstddef>
#include <string>
#include <vector>

#include <tlog/tlog.hpp>

typedef os__ebuf ebuf;

namespace
{
	typedef tlog::Log<OS_DEBUG> Log;
#ifdef _WIN32
	static const char Sep = '\\';
	static const std::string Seps = "/\\";
	bool rooted(const std::string &s)
	{
		return (
			s.size() > 1
			&& std::isalpha(s[0])
			&& s[1] == ':'
			&& (
				(s.size() == 2)
				|| Seps.find(s[2]) != Seps.npos));
	}
#else
	static const char Sep = '/';
	static const std::string Seps = "/";
	bool rooted(const std::string &s)
	{ return s.size() && s[0] == Sep; }
#endif

#if OS_DEBUG
	static std::size_t newed = 0;
	static std::size_t deled = 0;
#endif
}

struct os__Path
{
	os__Path(const std::string &dat);

	const char* c_str() const noexcept;
	operator const char*() const noexcept { return c_str(); }
	os__Path& operator+=(const std::string&);
	void add(const std::string&);

	void add_ext(const std::string&);
	void dirname() noexcept;
	void pop_ext() noexcept;

	const char* base() const noexcept;
	const char* ext() const noexcept;

	std::string data;
	std::vector<std::size_t> inds;
};

os__Path::os__Path(const std::string &dat):
	data{}, inds{}
{
	std::size_t pos = 0;
	if (rooted(dat))
	{
		pos = dat.find_first_of(Seps);
		data = dat.substr(0, pos);
		pos = pos < dat.size() ? pos + 1 : dat.size();
	}
	else
	{ data = "."; }
	data += Sep;
	add(dat.substr(pos));
}

const char* os__Path::c_str() const noexcept
{
	if (data[0] == '.')
	{
		if (data[0] == '.' && inds.size() == 0)
		{ return "."; }
		return data.c_str() + inds[0];
	}
	else
	{ return data.c_str(); }
}

os__Path& os__Path::operator+=(const std::string &s)
{
	if (rooted(s))
	{ *this = os__Path(s); }
	else
	{ add(s); }
	return *this;
}

void os__Path::add(const std::string &dat)
{
	std::size_t pos = 0;
	while (pos < dat.size())
	{
		std::size_t end = dat.find_first_of(Seps, pos);
		auto tok = dat.substr(pos, end - pos);
		if (tok == "..")
		{
			bool asIs = !inds.size() || data.substr(inds.back()) == "..";
			if (asIs)
			{
				if (data[0] == '.')
				{
					if (inds.size()) { data += Sep; }
					inds.push_back(data.size());
					data += tok;
				}
			}
			else
			{
				data.resize(inds.back() - (inds.size() > 1));
				inds.pop_back();
			}
		}
		else if (tok.size() && tok != ".")
		{
			if (inds.size()) { data += Sep; }
			inds.push_back(data.size());
			data += tok;
		}
		pos = end + (end != dat.npos);
	}
}

void os__Path::add_ext(const std::string &e)
{
	if (e.size() && e[0] != '.')
	{ data.append(1, '.').append(e); }
	else
	{ data += e; }
}
void os__Path::dirname() noexcept
{
	if (inds.size())
	{
		data.resize(inds.back() - (inds.size() > 1));
		inds.pop_back();
	}
}
void os__Path::pop_ext() noexcept
{
	if (inds.size())
	{
		data.resize(ext() - data.c_str());
	}
}

const char* os__Path::base() const noexcept
{
	if (inds.size())
	{ return data.c_str() + inds.back(); }
	else
	{ return data[0] == '.' ? "." : ""; }
}
const char* os__Path::ext() const noexcept
{
	if (inds.size())
	{
		auto pos = data.find_last_of('.');
		if (
			pos >= data.size() || pos <= inds.back()
			|| std::strcmp(base(), "..") == 0
		)
		{ return data.c_str() + data.size(); }
		else
		{ return data.c_str() + pos; }
	}
	return data.c_str() + data.size();
}

os__Path* os__Path__make(const char *path, ebuf* buf)
{
#if OS_DEBUG
	++newed;
#endif
	try
	{ return new os__Path{path}; }
	catch (std::exception &exc)
	{
		if (buf) { *buf << exc; };
		return nullptr;
	}
}

os__Path* os__Path__copy(const os__Path *p, ebuf *buf)
{
#if OS_DEBUG
	++newed;
#endif
	try
	{ return new os__Path{*p}; }
	catch (std::exception &exc)
	{
		if (buf) { *buf << exc; }
		return nullptr;
	}
}
void os__Path__free(os__Path *p)
{
#if OS_DEBUG
	++deled;
	Log() << "new/del: " << newed << " / " << deled << std::endl;
#endif
	delete p;
}

int os__Path__add(os__Path *p, const char *extra, ebuf *buf)
{
	try
	{
		*p += extra;
		return 0;
	}
	catch (std::exception &exc)
	{
		if (buf) { *buf << exc; }
		return -1;
	}
}
void os__Path__pop_ext(os__Path *p)
{ p->pop_ext(); }
int os__Path__add_ext(os__Path *p, const char *ext, ebuf *buf)
{
	try
	{ p->add_ext(ext); return 0; }
	catch (std::exception &exc)
	{ if (buf) { *buf << exc; }; return -1; }
}
void os__Path__dirname(os__Path *p)
{ p->dirname(); }

const char* os__Path__c_str(const os__Path *p)
{ return p->c_str(); }
const char* os__Path__base(const os__Path *p)
{ return p->base(); }
const char* os__Path__ext(const os__Path *p)
{ return p->ext(); }
size_t os__Path__segments(const os__Path *p)
{ return p->inds.size(); }
size_t os__Path__size(const os__Path *p)
{
	if (p->data[0] == '.')
	{
		if (p->inds.size() == 0) { return 1; }
		return p->data.size() - p->inds[0];
	}
	else
	{
		return p->data.size();
	}
}
