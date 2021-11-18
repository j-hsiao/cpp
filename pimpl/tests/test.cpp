#include <pimpl/pimpl.hpp>
#include <iostream>
#include <cassert>

namespace
{
	class MyClass
	{
		public:
			MyClass(int z);
			struct c;
			void gets() const;
			PIMPL_CREATE(c) impl;
	};

	struct MyClass::c
	{
		int a;
		c(int z): a(z) { std::cout << "pimpl made" << std::endl; }
		c(c&&o): a(o.a) { std::cout << "pimpl moved" << std::endl; }
		c(const c &o): a(o.a) { std::cout << "pimpl copied" << std::endl; }
		~c() { std::cout << "pimpl(" << a << ") deleted" << std::endl; }
		c& operator=(const c&o) = default;
	};

	PIMPL_FINALIZE(MyClass, c)

	MyClass::MyClass(int z): impl(new c(z)){}
	void MyClass::gets() const{ std::cout << impl->a << std::endl;}

}

int main()
{
	MyClass c1(1);
	MyClass c2(2);

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
	c2.impl = new MyClass::c(3);
	assert(c2.impl->a == 3);
	c2.gets();

	std::cout << "pass" << std::endl;
}
