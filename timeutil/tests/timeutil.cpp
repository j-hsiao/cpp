#include <timeutil/timeutil.hpp>
#include <iostream>
#undef NDEBUG
#include <cassert>

int main()
{
	timeutil::Timer t;
	double t1 = timeutil::time();
	t.tic();
	timeutil::sleep(2);
	double t2 = timeutil::time();
	double t3 = t.toc();

	assert((t2 - t1) >= 2);
	assert(t3 < 1);
	std::cout << "slept for " << (t2 - t1) << "seconds" << std::endl;
	std::cout << "processor time was " << t3 << std::endl;
}
