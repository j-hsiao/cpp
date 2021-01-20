#ifndef AES_ALIGN_H
#define AES_ALIGN_H
#include <cstddef>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace align
{
	template<class T>
	class AlignedAllocator
	{
			static const std::size_t MINALIGN = alignof(std::max_align_t) > alignof(T) ? alignof(std::max_align_t) : alignof(T);
		public:
			typedef T* pointer;
			typedef const T* const_pointer;
			typedef void* void_pointer;
			typedef const void* const_void_pointer;
			typedef T value_type;
			typedef std::size_t size_type;

			pointer allocate(std::size_t n)
			{
				std::size_t required = n * sizeof(T);
				std::size_t total = MINALIGN + required;
				void *dat = std::malloc(sizeof(void*) + total);
				if (!dat) { throw std::bad_alloc(); }
				void *aligned = reinterpret_cast<unsigned char*>(dat) + sizeof(void*);
				if (!std::align(MINALIGN, required, aligned, total))
				{
					std::free(dat);
					throw std::bad_alloc();
				}
				auto aubyte = reinterpret_cast<unsigned char*>(aligned);
				std::memcpy(aubyte - sizeof(void*), &dat, sizeof(void*));
				return reinterpret_cast<pointer>(aligned);
			}

			void deallocate(pointer p, std::size_t n)
			{
				void *base;
				auto *aubyte = reinterpret_cast<unsigned char*>(p);
				std::memcpy(&base, aubyte - sizeof(void*), sizeof(void*));
				std::free(base);
			}
	};
}
#endif
