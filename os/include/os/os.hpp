#ifndef OS_HPP
#define OS_HPP
#include <os/os.h>
#include <os/os_dllcfg.h>

#include <iostream>

#include <cstring>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace os
{
	struct ebuf
	{
		ebuf(std::size_t s = 256): buf(s, '\0'), ebuf_{&buf[0], buf.size()} {}
		operator os__ebuf*() noexcept { return &ebuf_; }
		std::string buf;
		os__ebuf ebuf_;
	};
	inline std::ostream& operator<<(std::ostream& o, const ebuf &e) { return o << e.buf; }
	struct Path
	{
		Path(const std::string &s);
		Path(const Path &o);
		Path(Path &&o) noexcept: ebuff(), p(o.p) { o.p = nullptr; }

		~Path() { if (p) { os__Path__free(p); }}

		Path operator+(const std::string &s);
		Path& operator+=(const std::string &s);
		operator std::string() const { return os__Path__c_str(p); }
		const char * c_str() const { return os__Path__c_str(p); }

		void pop_ext();
		void add_ext(const std::string &);
		void dirname();
		const char* base() const noexcept { return os__Path__base(p); }
		const char* ext() const noexcept { return os__Path__ext(p); }

		std::size_t segments() const { return os__Path__segments(p); }
		std::size_t size() const { return os__Path__size(p); }

		bool operator==(const Path &o) const noexcept;

		private:
			ebuf ebuff;
			os__Path *p;

			void raise(const std::string &prefix)
			{ throw std::runtime_error("Path " + prefix + ebuff.buf); }
	};

	struct Dirlist
	{
		Dirlist(const std::string &s);
		std::string next();
		~Dirlist() { if (data) { os__Dirlist__free(data, ebuff); }}

		Dirlist(const Dirlist &o) = delete;
		Dirlist(Dirlist &&o) = delete;

		private:
			ebuf ebuff;
			os__Dirlist *data;

			void raise(const std::string &prefix)
			{ throw std::runtime_error("Dirlist " + prefix + ebuff.buf); }
	};

	struct Filenode
	{
		Filenode(const std::string &path);
		bool exists() const noexcept { return os__Filenode__exists(info); }
		bool is_dir() const noexcept { return os__Filenode__is_dir(info); }
		bool is_file() const noexcept { return os__Filenode__is_file(info); }
		bool is_dir_nc() const noexcept { return os__Filenode__is_dir_nc(info); }
		bool is_file_nc() const noexcept { return os__Filenode__is_file_nc(info); }

		ebuf ebuff;
		os__Filenode info;
	};

	inline void mkdir(const std::string &dname);
	inline void makedirs(const std::string &dname);
	inline void rename(const std::string &before, const std::string &after);
	inline void remove(const std::string &path);
	inline std::string get_sysdir(const std::string &name);

	//terminal
	inline void hide_window();
	inline void show_window();

	struct Term
	{
		Term(const std::string &mode);
		~Term();
		std::string readline(bool strip = 1, int chunksize = 256);
		void hide_input();
		void show_input();
		FILE *tptr;
	};


	//environment
	void set_env(const std::string &val, const std::string &var);
	//> bufsize: required length
	//0 <= ret < = bufsize : string size
	//-1: not found
	//-2: other
	struct NotFound: public std::runtime_error
	{ NotFound(const std::string &e) : std::runtime_error(e){} };
	std::string get_env(const std::string &var);
}
#include <os/os_impl.hpp>
#endif
