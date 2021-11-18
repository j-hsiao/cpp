#ifndef OS_HPP
#define OS_HPP
#include "os/os.h"
#include "os/os_dllcfg.h"

#include <cstring>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>

namespace os
{
	// \\?\ etc paths not supported on windows.
	struct Path
	{
		static const std::string Pathseps;
		std::string path;
		Path(): path{"."} {}
		Path(const std::string &s): path{s} { normalize(); }
		Path(const char *s): path{s} { normalize(); }

		static bool isroot(const std::string &s);
		bool isroot() const { return isroot(path); }
		Path& normalize();
		Path& abs();

		Path operator+(const std::string &s) const;
		Path operator+(const char *s) const { return *this + std::string(s); }
		Path operator+(const Path &s) const { return *this + s.path; }
		//Does not automatically normalize after appending
		Path& operator+=(const char *s) { return *this += std::string(s); }
		Path& operator+=(const std::string &s);
		Path& operator+=(const Path &s) { return *this += s.path; }

		const char* c_str() const { return path.c_str(); }

		void pop_ext();
		void pop_base();

		Path dirname() const { Path ret{path}; ret.pop_base(); return ret; }
		std::string basename() const;
		std::string ext() const;

		std::size_t size() const { return path.size(); }
		
		static Path sysdir(const std::string &id);
	};

	struct Dirlist
	{
		struct iterator_type
		{
			iterator_type(const std::string &s);
			iterator_type();
			iterator_type(const iterator_type&) = delete;
			iterator_type(iterator_type&&);

			std::string operator*() const;
			iterator_type& operator++();
			bool operator!=(const iterator_type &o);
			struct info;
			std::unique_ptr<info> data;
			~iterator_type();
		};
		iterator_type begin() const { return {dirname}; }
		iterator_type end() const { return {}; }
		Dirlist(const std::string &s): dirname(s) {}
		std::string dirname;
	};

	struct Filenode
	{
		unsigned long long flags;

		Filenode(const std::string &name);
		Filenode(unsigned long long f): flags(f) {}
		operator bool() const; 
		bool isdir() const;
		bool isfile() const;
	};

	//throw if not exist
	std::string getenv(const std::string &name);
	//set to nullptr to remove env var;
	void setenv(const std::string &name, const char *val);


	void mkdir(const std::string &dname);
	void makedirs(const std::string &dname);
	void rename(const std::string &before, const std::string &after);
	void remove(const std::string &path);

	//terminal
	void hide_window();
	void show_window();


	struct Term
	{
		struct Ctx
		{
			Term *owner;
			bool orig;
			std::string readline(bool strip=1, int chunksize=256)
			{ return owner->readline(strip, chunksize); }
			~Ctx() { try { owner->show_input(orig); } catch(...) {} };
		};
		Term(const std::string &mode);
		~Term();
		std::string readline(bool strip=1, int chunksize=256);
		//original show/notshow
		bool show_input(bool show=1);
		Ctx show(bool s) { return {this, show_input(s)}; }
		FILE *tptr;
	};

	//call C apis
	namespace compat
	{
		#define CAPI_FORWARD(thing) if (thing) { throw std::runtime_error(buf); }
		struct ebuffed
		{
			static const std::size_t BUF_SIZE = 64;
			char buf[BUF_SIZE];
			os_ebuf ebuf;
			ebuffed(): ebuf{buf, BUF_SIZE} {}
		};
		struct Path: public ebuffed
		{
			os_Path *p;
			Path(): Path(".") {}
			Path(const std::string &s): Path(s.c_str()) {}
			Path(const char *s): ebuffed(), p(os_Path_make(s, &ebuf))
			{
				if (!p)
				{
					throw std::runtime_error(
						"failed to create compat path: " + std::string(buf));
				}
			}
			Path(const Path &o): Path(o.p ? o.c_str() : ".") {}
			Path(Path &&o): ebuffed(), p(o.p) { o.p = nullptr; }
			Path& operator=(Path &&o)
			{
				p = o.p;
				o.p = nullptr;
				return *this;
			}
			~Path() { if (p) { os_Path_free(p); } }

			Path& abs() { CAPI_FORWARD(os_Path_abs(p, &ebuf)) }
			Path& normalize() { CAPI_FORWARD(os_Path_normalize(p, &ebuf)) }
			Path& operator+=(const char *s)
			{
				CAPI_FORWARD(os_Path_add(p, s, &ebuf))
				return *this;
			}
			Path& operator+=(const std::string &s) { return *this += s.c_str(); }
			Path& operator+=(const Path &s) { return *this += s.c_str(); }

			Path operator+(const char *s) const
			{
				Path ret(c_str());
				ret += s;
				return ret;
			}
			Path operator+(const std::string &s) const { return *this + s.c_str(); }
			Path operator+(const Path &s) const { return *this + s.c_str(); }
			void pop_ext() { os_Path_pop_ext(p); }
			void pop_base() { os_Path_pop_base(p); }

			const char* c_str() const { return os_Path_c_str(p); }

			Path dirname() const { Path ret(c_str()); ret.pop_base(); return ret; }
			std::string basename() const { return os_Path_base(p); }
			std::string ext() const { return os_Path_ext(p); }
			std::size_t size() const { return os_Path_size(p); }
			static Path sysdir(const std::string &id)
			{
				ebuffed buf;
				const char* ret = os_sysdir(id.c_str(), &buf.ebuf);
				if (!ret)
				{ throw std::runtime_error(buf.buf); }
				return ret;
			}
		};

		struct Dirlist
		{
			struct iterator_type: public ebuffed
			{
				os_Dirlist *p;
				std::string item;

				iterator_type(const std::string &s): iterator_type(s.c_str()) {}
				iterator_type(const char *s):
					ebuffed(), p(os_Dirlist_make(s, &ebuf))
				{
					if (!p) { throw std::runtime_error(buf); }
					++(*this);
				}
				iterator_type(const iterator_type&) = delete;
				iterator_type(iterator_type &&o): ebuffed(), p(o.p) { o.p = nullptr; }
				iterator_type(): ebuffed(), p{nullptr} {}
				~iterator_type() { if (p) { os_Dirlist_free(p); } }

				std::string operator*() const { return item; }
				iterator_type& operator++()
				{
					if (p)
					{
						const char *v = os_Dirlist_next(p, &ebuf);
						if (v) { item = v; }
						else
						{
							os_Dirlist_free(p);
							p = nullptr;
							if (buf[0]) { throw std::runtime_error(buf); }
							item.resize(0);
						}
					}
					return *this;
				}
				bool operator!=(const iterator_type &o)
				{ return (p == nullptr) != (o.p == nullptr); }
			};

			std::string dirname;
			iterator_type begin() const { return {dirname}; }
			iterator_type end() const { return {}; }
			Dirlist(const std::string &s): dirname(s) {}
		};

		struct Filenode: public ebuffed
		{
			os_Filenode node;

			Filenode(const std::string &name):
				ebuffed(), node(os_Filenode_make(name.c_str(), &ebuf))
			{ if (!node) { throw std::runtime_error(buf); } }
			Filenode(os_Filenode f): ebuffed(), node(f) {}
			operator bool() const { return os_Filenode_exists(node); }
			bool isdir() const { return os_Filenode_is_dir(node); }
			bool isfile() const { return os_Filenode_is_file(node); }
		};

		inline std::string getenv(const std::string &name)
		{
			ebuffed buf;
			std::string ret;
			int sz = os_getenv(name.c_str(), nullptr, 0, &buf.ebuf);
			if (!sz) { return ""; }
			if (sz > 0)
			{
				ret.resize(sz);
				sz = os_getenv(name.c_str(), &ret[0], ret.size(), &buf.ebuf);
			}
			if (sz > 0) { return ret; }
			else if (sz == -1)
			{ throw std::runtime_error("environment variable \"" + name + "\" not found"); }
			else
			{ throw std::runtime_error(buf.buf); }
		}
		//set to nullptr to remove env var;
		inline void setenv(const std::string &name, const char *val)
		{
			ebuffed ebuf;
			const char *buf = ebuf.buf;
			CAPI_FORWARD(os_setenv(name.c_str(), val, &ebuf.ebuf))
		}

		inline void mkdir(const std::string &dname)
		{
			ebuffed ebuf;
			auto *buf = ebuf.buf;
			CAPI_FORWARD(os_mkdir(dname.c_str(), &ebuf.ebuf))
		}
		inline void makedirs(const std::string &dname)
		{
			ebuffed ebuf;
			auto *buf = ebuf.buf;
			CAPI_FORWARD(os_makedirs(dname.c_str(), &ebuf.ebuf))
		}
		inline void rename(const std::string &before, const std::string &after)
		{
			ebuffed ebuf;
			auto *buf = ebuf.buf;
			CAPI_FORWARD(os_rename(before.c_str(), after.c_str(), &ebuf.ebuf))
		}
		inline void remove(const std::string &path)
		{
			ebuffed ebuf;
			auto *buf = ebuf.buf;
			CAPI_FORWARD(os_remove(path.c_str(), &ebuf.ebuf))
		}

		inline void hide_window() { os_hide_window(); }
		inline void show_window() { os_show_window(); }

		struct Term: public ebuffed
		{
			struct Ctx
			{
				Term *owner;
				bool orig;
				std::string readline(bool strip=1)
				{ return owner->readline(strip); }
				~Ctx() { try { owner->show_input(orig); } catch(...) {} };
			};

			os_Term *t;
			Term(const std::string &mode): ebuffed(), t(os_Term_open(mode.c_str()))
			{ if (!t) { throw std::runtime_error("failed to open term"); } }
			~Term() { if (t) { os_Term_free(t); } }

			std::string readline(bool strip=1)
			{
				std::size_t sz;
				char *ln = os_Term_readline(t, &sz, strip);
				if (ln)
				{
					std::string ret(ln, sz);
					os_Term_linefree(ln);
					return ret;
				}
				else
				{ throw std::runtime_error("failed to readline"); }
			}
			//original show/notshow
			bool show_input(bool show=1)
			{
				int code;
				if (show)
				{ code = os_show_input(t, &ebuf); }
				else
				{ code = os_hide_input(t, &ebuf); }
				if (code & 0x02)
				{ throw std::runtime_error(buf); }
				else
				{ return code & 0x01; }
			}
			Ctx show(bool s) { return {this, show_input(s)}; }
		};

		#undef CAPI_FORWARD
	}
}
#endif
