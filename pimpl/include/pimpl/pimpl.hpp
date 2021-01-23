// pimpl helper
// like a light-weight smart pointer for c++
// use a pimpl::ptr as pointer to implementation
//
// example:
//    class Example
//    {
//      struct impl_t;
//      PIMPL_CREATE(impl_t) impl;
//    };
//    struct Example::impl_t
//    {
//      //define the class
//    };
//    PIMPL_FINALIZE(Example, impl_t);
//
//  PIMPL_CREATE(impl_t):
//    declares a class with an impl_t pointer named Pimpl
//  PIMPL_FINALIZE(Example, impl_t)
//    defines some methods of Example::Pimpl that require a impl_t to be complete
//
//  PIMPL_FINALIZE uses *p = *other.p for copy assignment
//  PIMPL_FINALIZE_NO_COPY_ASSIGN deletes p and then uses copy construction, making a new p
//    (for example, if impl_t cannot be copy assigned due to const members)
//
//  move assignment just swaps pointers
//
// makes use of incomplete types for pimpl easier
#ifndef PIMPL_H
#define PIMPL_H

#define PIMPL_CREATE(T) \
	class Pimpl\
	{\
		public:\
			Pimpl(T *pointer): p(pointer){}\
			Pimpl(const Pimpl &o): p(copy(o.p)){}\
			Pimpl(Pimpl &&o) noexcept: p(o.p){ o.p = nullptr; }\
			Pimpl& operator=(const Pimpl &o);\
			Pimpl& operator=(Pimpl &&o) noexcept\
			{ T *tmp = p; p = o.p; o.p = tmp; return *this; }\
			operator bool() const noexcept { return p; }\
			T& operator*() { return *p; }\
			const T& operator*() const { return *p; }\
			T* operator->() { return p; }\
			const T* operator->() const { return p; }\
			\
			~Pimpl() { if (p) { release(p); } }\
		private:\
			static T* copy(const T*);\
			static void release(T*);\
			T *p;\
	};\
	Pimpl \

//use copy assignment operator for the Pimpl struct
#define PIMPL_FINALIZE(NS, T)\
	NS::T* NS::Pimpl::copy(const NS::T* o) { return new NS::T(*o); }\
	void NS::Pimpl::release(NS::T* o) { delete o; }\
	NS::Pimpl& NS::Pimpl::operator=(const NS::Pimpl &o)\
	{ if (p && o) { *p = *o.p; return *this; } else if (p) { release(p); } p = copy(o.p); return *this; }\

//release and use copy constructor
#define PIMPL_FINALIZE_NO_COPY_ASSIGN(NS, T)\
	NS::T* NS::Pimpl::copy(const NS::T* o) { return new NS::T(*o); }\
	void NS::Pimpl::release(NS::T* o) { delete o; }\
	NS::Pimpl& NS::Pimpl::operator=(const NS::Pimpl &o)\
	{ if (p) { release(p); } p = copy(o.p); return *this; }\

#endif
