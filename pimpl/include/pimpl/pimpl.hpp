// pimpl helper
// use a pimpl::ptr for implementations
// in impl:
//   define pimpl::details::operator()
//   pimpl::details::cp(const T*)
#ifndef PIMPL_H
#define PIMPL_H

#include <utility>
#include <memory>

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
				return *this = copy(o.p);
			}
			ptr& operator=(ptr &&o)
			{
				T *tmp = p;
				p = o.p;
				o.p = tmp;
				return *this;
			}
			ptr& operator=(T* pointer)
			{
				if (p)
				{
					release(p);
				}
				p = pointer;
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
#define FINISH_PIMPL(a) namespace pimpl{ template<> void ptr<a>::release(a* x){delete x;} template<> a* ptr<a>::copy(const a *x){ return x ? a(*x) : x; } }

#endif
