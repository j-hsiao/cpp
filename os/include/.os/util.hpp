#ifndef OS_UTIL_HPP
#define OS_UTIL_HPP
#include <os/os.h>
#include <tlog/tlog.hpp>

#include <cstring>
#include <string>
namespace
{
	inline void operator<<(os__ebuf &buf, const std::exception &exc)
	{
		std::strncpy(buf.buf, exc.what(), buf.size);
		buf.buf[buf.size - 1] = 0;
	}

	inline void operator<<(os__ebuf &buf, std::errc ec)
	{
		int c = static_cast<int>(ec);
		std::strncpy(
			buf.buf, std::generic_category().message(c).c_str(), buf.size);
		buf.buf[buf.size - 1] = 0;
	}
}
#endif
