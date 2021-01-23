#include <timeutil/timeutil.hpp>
#include <iostream>
#undef NDEBUG
#include <cassert>

#include <limits>

int main()
{
	timeutil::Timer t;
	timeutil::Clocker c;
	double t1 = timeutil::time();
	t.tic();
	c.tic();
	timeutil::sleep(2);
	double t2 = timeutil::time();
	double t3 = t.toc();
	double t4 = c.toc();

	double thresh = 1.9;

	std::cout << "slept for " << (t2 - t1) << "seconds" << std::endl;
	std::cout << "wall time was " << t3 << std::endl;
	std::cout << "processor time was " << t4 << std::endl;

	assert((t2 - t1) > thresh);
	assert(t3 > thresh);
	assert(t4 < 1);
	return 0;
}
