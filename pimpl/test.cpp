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
			pimpl::ptr<c> impl;
	};

	struct myclass::c
	{
		int a;
		c(int z): a(z) { std::cout << "pimpl made" << std::endl; }
		c(c&&o): a(o.a) { std::cout << "pimpl moved" << std::endl; }
		c(const c &o): a(o.a) { std::cout << "pimpl copied" << std::endl; }
		~c() { std::cout << "pimpl(" << a << ") deleted" << std::endl; }
	};
	myclass::myclass(int z): impl(new c(z)){}
	void myclass::gets() const{ std::cout << impl->a << std::endl;}
}

FINISH_PIMPL(myclass::c)

int main()
{
	myclass c1(1);
	myclass c2(2);

	c1.gets();
	c2.gets();

	assert(c1.impl->a == 1);
	assert(c2.impl->a == 2);


	c2 = c1;
	assert(c1.impl->a == 1);
	assert(c2.impl->a == 1);
	c1.gets();
	c2.gets();

	c2.impl = nullptr;
	assert(!c2.impl);
	std::cout << "c2 deleted?" << std::endl;
	c2.impl = new myclass::c(3);
	assert(c2.impl->a == 3);
	c2.gets();

}
