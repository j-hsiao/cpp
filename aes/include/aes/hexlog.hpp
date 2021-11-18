#ifndef AES_PARRAY_HPP
#define AES_PARRAY_HPP
#include <iostream>
#include <string>
#include <type_traits>
#include <climits>

namespace aes
{
	template<class T>
	std::string tohex(T val)
	{
		const char *lut = "0123456789abcdef";
		std::string v;
		for (int i=sizeof(val)*CHAR_BIT - 4; i>=0; i -= 4)
		{ v.push_back(lut[(val >> i) & 0x0fu]); }
		return v;
	}

	template<class T>
	struct HexArray_
	{
		const T &data;
		std::size_t n;
	};

	template<class T>
	struct const_size
	{
		static const bool value = std::is_convertible<
			typename std::remove_pointer<decltype(&T::size)>::type,
			std::size_t>::value;
	};

	template<
		class T,
		typename std::enable_if<const_size<T>::value, bool>::type=1>
	HexArray_<T> hex(const T &item)
	{ return HexArray_<T>{item, T::size}; }

	template<
		class T,
		typename std::enable_if<!const_size<T>::value, bool>::type=1>
	HexArray_<T> hex(const T &item)
	{ return HexArray_<T>{item, item.size()}; }

	template<
		class T,
		bool okay=std::is_pointer<T>::value || std::is_array<T>::value,
		typename std::enable_if<okay, bool>::type=1>
	HexArray_<T> hex(const T &item, std::size_t n)
	{ return HexArray_<T>{item, n}; }

	template<class T>
	std::ostream& operator<<(std::ostream &o, const HexArray_<T> &item)
	{
		o << "[";
		for (std::size_t i=0; i<item.n; ++i)
		{ o << " " << tohex(item.data[i]); }
		return o << " ]";
	}
}
#endif
