#ifndef TLOG_HPP
#define TLOG_HPP

#include <iostream>

namespace
{
	// Compile-time addition/removal of logging statements
	namespace tlog
	{
		template<bool verbose>
		struct Log
		{
			template<class T>
			const Log& operator<<(const T &thing) const
			{
				if (verbose) { std::cerr << thing; }
				return *this;
			}
			const Log& operator<<(std::ostream& (*func)(std::ostream &)) const
			{
				if (verbose) { std::cerr << func; }
				return *this;
			}
		};
	}
}
#endif
