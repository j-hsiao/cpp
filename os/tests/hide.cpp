#include "os/os.hpp"
#include <iostream>
#include "timeutil/timeutil.hpp"


int main()
{
	double sleeptime;
	while (std::cout << "sleep time: " && std::cin >> sleeptime)
	{
#if !(defined(_WIN32) && TESTING_DLL)
		os::hide_window();
		timeutil::sleep(sleeptime);
		os::show_window();
#endif
		if (std::cout << "compat sleep time: " && std::cin >> sleeptime)
		{
			os::compat::hide_window();
			timeutil::sleep(sleeptime);
			os::compat::show_window();
		}
		else
		{ return 0; }
	};
	return 0;
}
