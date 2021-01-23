#include <os/os.hpp>
#include <iostream>
#include <timeutil/timeutil.hpp>


int main()
{
	double sleeptime;
	while (std::cout << "sleep time: " && std::cin >> sleeptime)
	{
		os__hide_window();
		timeutil::sleep(sleeptime);
		os__show_window();
	};
}
