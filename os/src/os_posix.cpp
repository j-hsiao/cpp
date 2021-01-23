#include <os/os.h>
#include <.os/util.hpp>

#include "path_incl.cpp"

#include <cerrno>
#include <cstring>
#include <exception>
#include <map>
#include <string>
#include <system_error>
#include <stdlib.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>


//new process with piped in/out
//https://stackoverflow.com/questions/17716359/redirect-child-processs-stdin-and-stdout-to-pipes
namespace
{
	static const std::error_condition noent = std::make_error_condition(
		std::errc::no_such_file_or_directory);
	os__ebuf& operator<<(os__ebuf &buf, int code)
	{
		std::strncpy(
			buf.buf, std::generic_category().message(code).c_str(), buf.size);
		buf.buf[buf.size - 1] = 0;
		return buf;
	}
}

struct os__Dirlist {};
os__Dirlist* os__Dirlist__make(const char *path, os__ebuf *buf)
{
	DIR *ptr = ::opendir(path);
	if (!ptr && buf)
	{ *buf << errno; }
	return reinterpret_cast<os__Dirlist*>(ptr);
}
int os__Dirlist__free(os__Dirlist *ptr, os__ebuf *buf)
{
	int ret = ::closedir(reinterpret_cast<DIR*>(ptr));
	if (ret == -1 && buf)
	{ *buf << errno; }
	return ret;
}
//https://man7.org/linux/man-pages/man3/readdir.3.html
//readdir is threadsafe if the stream is different (on modern systems)
//don't bother with synchronization then
const char* os__Dirlist__next(os__Dirlist *ptr, os__ebuf *ebuf)
{
	while (1)
	{
		errno = 0;
		struct dirent *ent = ::readdir(reinterpret_cast<DIR*>(ptr));
		if (ent)
		{
			if (
				std::strcmp(ent->d_name, ".") && std::strcmp(ent->d_name, ".."))
			{ return ent->d_name; }
		}
		else
		{
			if (!errno) { return ""; }
			if (ebuf) { *ebuf << errno; }
			return nullptr;
		}
	}
}

//------------------------------
//file node
//------------------------------
os__Filenode os__Filenode__make(const char *path, os__ebuf *buf)
{
	struct stat info;
	if (::stat(path, &info) == -1)
	{
		int code = errno;
		if (std::error_code(code, std::generic_category()) != noent)
		{
			if (buf) { *buf << code; }
			return -1;
		}
		else
		{ return 0; }
	}
	else
	{ return info.st_mode & S_IFMT; }
}
bool os__Filenode__exists(os__Filenode node)
{ return node > 0; }
bool os__Filenode__is_dir_nc(os__Filenode node)
{ return node == S_IFDIR; }
bool os__Filenode__is_file_nc(os__Filenode node)
{ return node == S_IFREG; }

//------------------------------
//fs modification
//------------------------------
int os__mkdir(const char *path, os__ebuf *buf)
{
	int ret = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (ret == -1 && buf)
	{ *buf << errno; }
	return ret;
}

#include "normdst_incl.cpp"

//works for directories and files
int os__rename(const char *before, const char *after, os__ebuf *buf)
{
	try
	{
		std::string dst = normalize_dst(before, after, buf);
		if (!dst.size()) { return -1; }
		if (dst != after) {
			os__Filenode node = os__Filenode__make(dst.c_str(), buf);
			if (node < 0) { return -1; }
			if (os__Filenode__exists(node))
			{
				if (buf) { *buf << std::errc::file_exists; }
				return -1;
			}
		}
		if (::rename(before, dst.c_str()))
		{
			int code = errno;
			if (buf) { *buf << code; }
			return code;
		}
		return 0;
	}
	catch (std::exception &exc)
	{
		if (buf) { *buf << exc; }
		return -1;
	}
}
int os__remove(const char *path, os__ebuf *buf)
{
	try
	{
		os__Filenode node = os__Filenode__make(path, buf);
		if (node < 0) { return -1; }
		if (os__Filenode__exists(node))
		{
			int errored;
			if (os__Filenode__is_file_nc(node))
			{ errored = ::unlink(path); }
			else if (os__Filenode__is_dir_nc(node))
			{ errored = ::rmdir(path); }
			if (errored)
			{
				int code = errno;
				if (std::error_code(code, std::generic_category()) != noent)
				{ throw std::system_error(code, std::generic_category(), "remove"); }
			}
		}
		return 0;
	}
	catch (std::exception &exc)
	{
		if (buf) { *buf << exc; }
		return -1;
	}
}

const char* os__get_sysdir(const char *name)
{
	static std::map<std::string, std::string> dirs;
	if (!dirs.size())
	{
		dirs[""] = "";
		char *home = getenv("HOME");
		if (home)
		{
			std::string homedir(home);
			dirs["home"] = homedir;
			dirs["downloads"] = homedir + "/Downloads";
			dirs["pictures"] = homedir + "/Pictures";
			dirs["documents"] = homedir + "/Documents";
			dirs["videos"] = homedir + "/Videos";
			dirs["desktop"] = homedir + "/Desktop";
		}
		else
		{ Log() << "failed to get home directory" << std::endl; }
	}
	auto it = dirs.find(name);
	return (dirs.end() == it) ? "" : it->second.c_str();
}

OS_API void os__hide_window() {}
OS_API void os__show_window() {}

FILE* os__term_open(const char *mode)
{ return fopen("/dev/tty", mode); }

int os__hide_input(FILE *f, os__ebuf *buf)
{
	struct termios current;
	int fd = ::fileno(f);
	if (::tcgetattr(fd, &current) == -1)
	{
		if (buf) { *buf << errno; }
		return -1;
	}
	current.c_lflag &= ~ECHO;
	if (tcsetattr(fd, TCSAFLUSH, &current) == -1)
	{
		if (buf) { *buf << errno; }
		return -1;
	}
	return 0;
}
int os__show_input(FILE *f, os__ebuf *buf)
{
	struct termios current;
	int fd = ::fileno(f);
	if (::tcgetattr(fd, &current) == -1)
	{
		if (buf) { *buf << errno; }
		return -1;
	}
	current.c_lflag |= ECHO;
	if (::tcsetattr(fd, TCSAFLUSH, &current) == -1)
	{
		if (buf) { *buf << errno; }
		return -1;
	}
	return 0;
}

int os__set_env(const char *val, const char *var, os__ebuf *buf)
{
	if (!val)
	{
		if (::unsetenv(var))
		{
			if (buf) { *buf << errno; }
			return -1;
		}
	}
	else
	{
		if (::setenv(var, val, 1))
		{
			if (buf) { *buf << errno; }
			return -1;
		}
	}
	return 0;
}
int os__get_env(char *buf, const char *var, size_t bufsize, os__ebuf *ebuf)
{
	char *val = ::getenv(var);
	if (val)
	{
		size_t len = std::strlen(val);
		if (len <= bufsize)
		{ std::strncpy(buf, val, bufsize); }
		return static_cast<int>(len);
	}
	else
	{ return -1; }
}

#include "common_incl.cpp"
