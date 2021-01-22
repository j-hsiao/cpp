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

#define PIMPL_CREATE(T) \
	class pimpl\
	{\
		public:\
			pimpl(T *pointer): p(pointer){}\
			pimpl(const pimpl &o): p(copy(o.p)){}\
			pimpl(pimpl &&o) noexcept: p(o.p){ o.p = nullptr; }\
			pimpl& operator=(const pimpl &o);\
			pimpl& operator=(pimpl &&o) noexcept\
			{ T *tmp = p; p = o.p; o.p = tmp; return *this; }\
			operator bool() const noexcept { return p; }\
			T& operator*() { return *p; }\
			const T& operator*() const { return *p; }\
			T* operator->() { return p; }\
			const T* operator->() const { return p; }\
			\
			~pimpl() { if (p) { release(p); } }\
		private:\
			static T* copy(const T*);\
			static void release(T*);\
			T *p;\
	};\

#define PIMPL_FINALIZE(NS, T)\
	NS::T* NS::pimpl::copy(const NS::T* o) { return new NS::T(*o); }\
	void NS::pimpl::release(NS::T* o) { delete o; }\
	NS::pimpl& NS::pimpl::operator=(const NS::pimpl &o)\
	{ if (p && o) { *p = *o.p; return *this; } else if (p) { release(p); } p = copy(o.p); return *this; }\

#define PIMPL_FINALIZE_NOCOPY(NS, T)\
	NS::T* NS::pimpl::copy(const NS::T* o) { return new NS::T(*o); }\
	void NS::pimpl::release(NS::T* o) { delete o; }\
	NS::pimpl& NS::pimpl::operator=(const NS::pimpl &o)\
	{ if (p) { release(p); } p = copy(o.p); return *this; }\

#endif
