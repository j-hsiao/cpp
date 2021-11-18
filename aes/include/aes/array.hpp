#ifndef AES_ARRAY_HPP
#define AES_ARRAY_HPP

// Notes
// on various testmachines, performance varies, maybe gcc version?
// pred21: normal is 8x slower, the other 3 are about the same
// on pred21 VM, normal is 5x faster than on raw machine...
//	but the other 3 are a bit slower on the vm, implies gcc version is the issue?
//	(pred21 = gcc5.5, VM = gcc 8.3
//
// home machine(cygwin): normal is fastest
// work laptop (cygwin): adl or crtp is fastest (11.2), sfinae is slowest
//
// home machine(mingw):
// work laptop (mingw): crtp is fastest (11.2), sfinae is slowest
//
// CRTP + mixins seems to be quite convenient and fast,
// but constantly cast to derived type is a bit annoying, a ?minor? annoyance.
// Implement a feature once, and add the mixin as a subclass
// however, this has the largest executable size
//
// ADL is quick too, but subclasses need to have
// using parent::parent;
// using parent::operator=;
// however, adl doesn't work if I want to use a primitive
// as a type
// for constructor, operator=, etc, a bit annoying
// smallest executable size
//
// sfinae very quick as well, most succinct, second smallest
// executable size
//
// C style approach:
// class with static functions passed as template parameter.
// Data passed along with args.
//
// standalone operators are useful for the 1st type != my class situation
// maybe combine crtp with adl?
//
// CRTP makes return types the derived types for base-class methods
// though adl template operators can also achieve the same thing
//
//
// final implementation: merge concepts?

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace aes
{
	namespace normal
	{
		template<
			class T,
			typename std::enable_if<!std::is_const<T>::value, std::size_t>::type count>
		struct Owned;

		template<class T, std::size_t count>
		struct Wrapped
		{
			typedef T value_type;
			typedef typename std::remove_const<T>::type vtype;

			typedef Owned<vtype, count> owned_type;
			typedef Wrapped<vtype, count> wrapped_type;
			typedef Wrapped<const vtype, count> cwrapped_type;

			static const std::size_t size = count;

			T *data;
			Wrapped(T *p): data(p) {}

			operator T*() { return data; }
			operator const T*() const { return data; }

			Wrapped& operator=(const void* o) &&
			{
				std::memcpy(data, o, sizeof(T)*count);
				return *this;
			}
			Wrapped& operator=(T *ptr)
			{
				data = ptr;
				return *this;
			}
		};

		template<
			class T,
			typename std::enable_if<!std::is_const<T>::value, std::size_t>::type count>
		struct Owned: public Wrapped<T, count>
		{
			typedef T value_type;
			typedef T vtype;

			typedef Owned owned_type;
			typedef Wrapped<T, count> wrapped_type;
			typedef Wrapped<const T, count> cwrapped_type;

			static const std::size_t size=count;

			T buffer[count];

			Owned(): wrapped_type(buffer){}
			Owned(const void *dat): wrapped_type(buffer) { *this = dat; }
			template<
				class First, class ... Args,
				bool ok = std::is_convertible<First, vtype>::value,
				typename std::enable_if<ok, bool>::type = 1>
			Owned(First &&o, Args&&...args):
				wrapped_type(buffer),
				buffer{ static_cast<vtype>(o), static_cast<vtype>(args)...}
			{}
			Owned(const Owned &other): Owned(other.buffer) {}

			Owned& operator=(const void *src)
			{
				std::memcpy(buffer, src, sizeof(buffer));
				return *this;
			}
			Owned& operator=(const Owned&) = delete;
			Owned& operator=(Owned&&) = delete;
		};



		template<class T>
		T& operator^=(T &t, const typename T::vtype *o)
		{
			for (std::size_t i=0; i<T::size; ++i)
			{ t[i] ^= o[i]; }
			return t;
		}

		template<class T>
		typename T::owned_type operator^(const T &t, const typename T::vtype *o)
		{
			typename T::owned_type ot(t);
			ot ^= o;
			return ot;
		}
		template<
			class V, class T,
			typename std::enable_if<std::is_same<V, typename T::vtype>::value, bool>::type=1>
		typename T::owned_type operator^(const V *o, const T &t)
		{
			typename T::owned_type ot(t);
			ot ^= o;
			return ot;
		}
	}

	namespace crtp
	{
		template<class T, std::size_t count>
		struct ArrayTraits
		{
			typedef T value_type;
			typedef typename std::remove_const<T>::type vtype;
			static const std::size_t size = count;
		};

		template<
			class T, std::size_t count,
			template<class d, class trt>
			class ... Mixins>
		struct Wrapped;

		template<
			class T,
			typename std::enable_if<!std::is_const<T>::value, std::size_t>::type count,
			template<class d, class trt>
			class ... Mixins>
		struct Owned:
			ArrayTraits<T, count>,
			Mixins<Owned<T, count, Mixins...>, ArrayTraits<T, count>>...
		{
			//typedef typename std::remove_const<T>::type vtype;
			typedef ArrayTraits<T, count> traits;

			typedef Owned owned_type;
			typedef Wrapped<T, count, Mixins...> wrapped_type;
			typedef Wrapped<const T, count, Mixins...> cwrapped_type;
			T data[count];

			Owned() = default;
			Owned(const void *p) { *this = p; }
			template<
				class First, class ... Remain,
				bool convertible = std::is_convertible<First, T>::value,
				bool nargsok = sizeof...(Remain) <= count-1,
				typename std::enable_if<convertible && nargsok, bool>::type = 1>
			Owned(First&& f, Remain&&...remain):
				data{static_cast<T>(f), static_cast<T>(remain)...}
			{}
			operator T*() { return data; }
			operator const T*() const { return data; }

			Owned& operator=(const void *ptr)
			{
				std::memcpy(data, ptr, sizeof(data));
				return *this;
			}
		};

		template<
			class T, std::size_t count,
			template<class d, class trt>
			class ... Mixins>
		struct Wrapped:
			ArrayTraits<T, count>,
			Mixins<Wrapped<T, count, Mixins...>, ArrayTraits<T, count>>...
		{
			typedef ArrayTraits<T, count> traits;

			typedef Owned<typename Wrapped::vtype, count, Mixins...> owned_type;
			typedef Wrapped<typename Wrapped::vtype, count, Mixins...> wrapped_type;
			typedef Wrapped<const typename Wrapped::vtype, count, Mixins...> cwrapped_type;

			T *data;
			Wrapped(T *p): data{p} {}
			Wrapped() = default;

			operator T*() { return data; }
			operator const T*() const { return data; }

			Wrapped& operator=(T *ptr) &
			{
				data = ptr;
				return *this;
			}
			Wrapped& operator=(const void *p) &&
			{
				std::memcpy(data, p, sizeof(T) * count);
				return *this;
			}
		};


		template<class d, class traits>
		struct Xorable
		{
			d& operator^=(const typename traits::vtype *data)
			{
				d &me = static_cast<d&>(*this);
				for (std::size_t i=0; i<d::size; ++i)
				{ me.data[i] ^= data[i]; }
				return me;
			}
		};
		template<
			class d,
			bool is_xorable = std::is_base_of<Xorable<d, typename d::traits>, d>::value,
			typename std::enable_if<is_xorable, bool>::type=1>
		typename d::owned_type operator^(const d &item, const typename d::vtype *data)
		{
			typename d::owned_type ret(item);
			ret ^= data;
			return ret;
		}
		template<
			class d, class V,
			bool is_xorable = std::is_base_of<Xorable<d, typename d::traits>, d>::value,
			bool tmatch = std::is_same<V, typename d::vtype>::value,
			typename std::enable_if<is_xorable && tmatch, bool>::type=1>
		typename d::owned_type operator^(const V *data, const d &item)
		{
			typename d::owned_type ret(item);
			ret ^= data;
			return ret;
		}
	}

	namespace adl
	{
		template<class T, std::size_t count>
		struct Wrapped;

		template<class T, std::size_t count>
		struct Owned
		{
			typedef T value_type;
			typedef typename std::remove_const<T>::type vtype;

			typedef Owned owned_type;
			typedef Wrapped<vtype, count> wrapped_type;
			typedef Wrapped<const vtype, count> cwrapped_type;

			static const std::size_t size = count;

			vtype data[count];
			operator T*(){ return data; }
			operator const T*() const { return data; }

			template<
				class First, class ... Remain,
				bool convertible = std::is_convertible<First, vtype>::value,
				bool nargsok = sizeof...(Remain) <= count-1,
				typename std::enable_if<convertible && nargsok, bool>::type = 1>
			Owned(First&& f, Remain&&...remains):
				data{static_cast<vtype>(f), static_cast<vtype>(remains)...}
			{}
			Owned(const Owned&) = default;
			Owned(Owned&&) = default;
			Owned(const void *d) { *this = d; }

			Owned& operator=(const Owned&) = default;
			Owned& operator=(Owned&&) = default;
			Owned& operator=(const void *d)
			{
				std::memcpy(data, d, sizeof(data));
				return *this;
			}
		};

		template<class T, std::size_t count>
		struct Wrapped
		{
			typedef T value_type;
			typedef typename std::remove_const<T>::type vtype;

			typedef Owned<vtype, count> owned_type;
			typedef Wrapped<vtype, count> wrapped_type;
			typedef Wrapped<const vtype, count> cwrapped_type;

			static const std::size_t size = count;

			T *data;
			operator T*(){ return data; }
			operator const T*() const { return data; }

			Wrapped(T *d): data{d} {}
			Wrapped& operator=(const void *d)&&
			{
				std::memcpy(data, d, sizeof(T)*count);
				return *this;
			}
			Wrapped& operator=(T *d)
			{
				data = d;
				return *this;
			}
		};

		template<class T>
		T& operator^=(T& o1, const typename T::vtype *o2)
		{
			for (std::size_t i=0; i<T::size; ++i)
			{ o1[i] ^= o2[i]; }
			return o1;
		}

		template<class T>
		typename T::owned_type operator^(const T &o1, const typename T::vtype *o2)
		{
			typename T::owned_type ret(o1);
			ret ^= o2;
			return ret;
		}
		template<
			class V, class T,
			typename std::enable_if<std::is_same<typename T::vtype, V>::value, bool>::type = 1>
		typename T::owned_type operator^(const V *o2, const T &o1)
		{
			typename T::owned_type ret(o1);
			ret ^= o2;
			return ret;
		}
	}

	namespace sfinae
	{
		template<class T, std::size_t count, bool owned>
		struct Array
		{
			typedef T value_type;
			typedef typename std::remove_const<T>::type vtype;
			typedef Array<vtype, count, 1> owned_type;
			static const std::size_t size=count;

			typename std::conditional<owned, vtype[count], T*>::type data;
			operator T*(){ return data; }
			operator const T*() const { return data; }

			template<
				class First, class ... Remain,
				bool convertible = std::is_convertible<First, vtype>::value,
				bool nargsok = sizeof...(Remain) <= count-1,
				typename std::enable_if<convertible && nargsok && owned, bool>::type = 1>
			Array(First&& f, Remain&&...remains):
				data{static_cast<vtype>(f), static_cast<vtype>(remains)...}
			{}
			template<bool o=owned, typename std::enable_if<!o, bool>::type = 1>
			Array(T *d): data{d} {}
			template<bool o=owned, typename std::enable_if<o, bool>::type = 1>
			Array(const void *d) { *this = d; }

			Array() = default;
			Array(const Array&) = default;
			Array(Array&&) = default;

			template<class V, bool o=owned, typename std::enable_if<!o, bool>::type=1>
			Array& operator=(const V &d)&&
			{
				std::memcpy(data, d, sizeof(T)*count);
				return *this;
			}
			template<class V, bool o=owned, typename std::enable_if<o, bool>::type=1>
			Array& operator=(const V &d)
			{
				std::memcpy(data, d, sizeof(T)*count);
				return *this;
			}

			Array& operator=(const Array&) = default;
			Array& operator=(Array&&) = default;
			Array& operator=(const void *d)
			{
				std::memcpy(data, d, sizeof(data));
				return *this;
			}

			Array& operator^=(const vtype *d)
			{
				for (std::size_t i=0; i<count; ++i)
				{ data[i] ^= d[i]; }
				return *this;
			}
		};
		template<class T>
		typename T::owned_type operator^(const T &item, const typename T::vtype *data)
		{
			typename T::owned_type ret(item);
			ret ^= data;
			return ret;
		}
		template<class V, class T>
		typename T::owned_type operator^(const V *data, const T &item)
		{ return item ^ data; }

		template<class T, std::size_t count>
		using Owned = Array<typename std::remove_const<T>::type, count, 1>;
		template<class T, std::size_t count>
		using Wrapped = Array<T, count, 0>;
	}
}
#endif
