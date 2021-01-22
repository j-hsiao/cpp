#include <pimpl/pimpl.hpp>
#include <iostream>
#include <cassert>

namespace
{
	class myclass
	{
		public:
			myclass(int z);
			struct c;
			void gets() const;
			PIMPL_CREATE(c)
			pimpl impl;
	};

	struct myclass::c
	{
		int a;
		c(int z): a(z) { std::cout << "pimpl made" << std::endl; }
		c(c&&o): a(o.a) { std::cout << "pimpl moved" << std::endl; }
		c(const c &o): a(o.a) { std::cout << "pimpl copied" << std::endl; }
		~c() { std::cout << "pimpl(" << a << ") deleted" << std::endl; }
		c& operator=(const c&o) = default;
	};

	PIMPL_FINALIZE_NOCOPY(myclass, c)

	myclass::myclass(int z): impl(new c(z)){}
	void myclass::gets() const{ std::cout << impl->a << std::endl;}

}

int main()
{
	myclass c1(1);
	myclass c2(2);

	c1.gets();
	c2.gets();

	assert(c1.impl->a == 1);
	assert(c2.impl->a == 2);

	std::cout << "c2 copyassign c1" << std::endl;
	c2 = c1;
	assert(c1.impl->a == 1);
	assert(c2.impl->a == 1);
	c1.gets();
	c2.gets();

	std::cout << "deleting c2(1)" << std::endl;
	c2.impl = nullptr;
	assert(!c2.impl);

	std::cout << "set c2 to 3" << std::endl;
	c2.impl = new myclass::c(3);
	assert(c2.impl->a == 3);
	c2.gets();

	std::cout << "pass" << std::endl;
}
