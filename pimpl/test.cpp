#include <pimpl/pimpl.hpp>
#include <iostream>

namespace
{
	class myclass
	{
		myclass(int z);
		struct c;
		void gets() const;
		private:
			pimpl::ptr<c> impl;
	};

	struct myclass::c
	{
		int a
		c(int z): a(z) { std::cout << "pimpl made" << std::endl; }
		~c() { std::cout << "pimpl deleted" << std::endl; }
	}
	myclass::myclass(int z): impl(new c(z)){}
}

FINISH_PIMPL(myclass::c)

int main()
{
	myclass c1(1);
	myclass c2(2);

	c1.gets();
	c2.gets();
}
