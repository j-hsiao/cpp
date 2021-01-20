// pimpl helper
// use a pimpl::ptr as pointer to implementation
// automatically provides copy/move constructors/assignments
//     copy creates a new pointer copied from obj at previous pointer
//     rather than copying the value of the pointer itself
// to support incomplete types, pimpl::ptr<T>
// requires users to define copy and release methods
// default implementations can be made by using the FINISH_PIMPL macro
// (just calls new/delete)
#ifndef PIMPL_H
#define PIMPL_H

#include <utility>
#include <memory>

#define FINISH_PIMPL(a) namespace pimpl{ template<> void ptr<a>::release(a* x){delete x;} template<> a* ptr<a>::copy(const a *x){ return x ? new a(*x) : nullptr; } }

namespace pimpl
{
	template<class T>
	class ptr
	{
		public:
			~ptr() { if (p) { release(p); } }

			ptr(T* pointer): p(pointer){}
			ptr(const ptr &o): p(copy(o.p)){}
			ptr(ptr &&o): p(o.p) { o.p = nullptr; }
			ptr& operator=(const ptr &o)
			{
				if (p)
				{
					release(p);
				}
				p = copy(o.p);
				return *this;
			}
			ptr& operator=(ptr &&o)
			{
				T *tmp = p;
				p = o.p;
				o.p = tmp;
				return *this;
			}

			operator bool() const { return p; }

			T& operator*() { return *p; }
			const T& operator*() const { return *p; }

			T* operator->() { return p; }
			const T* operator->() const { return p; }

		private:
			static T* copy(const T*);
			static void release(T*);

			T* p;
	};
}
#endif
