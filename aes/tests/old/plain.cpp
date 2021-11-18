#include <aes/aes.h>
#include <.aes/impl.hpp>
#include <.aes/types.hpp>

#include <timeutil/timeutil.hpp>

#include <algorithm>
#undef NDEBUG
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

namespace
{
	template<class T>
	std::string vecstr(const std::vector<T> &vec)
	{
		std::stringstream ss;
		ss << "[";
		if (vec.size())
		{
			ss << +vec[0];
			for (auto it = vec.begin() + 1; it != vec.end(); ++it)
			{
				ss << ", " << +*it;
			}
		}
		ss << "]";
		return ss.str();
	}
}

int main()
{
	aes::Impl impl = aes::make_impl<aes::PlainImpl>();
	{
		aes__Keys keys;
		aes__Byte key[] = {
			'T', 'h', 'a', 't', 's', ' ', 'm', 'y', ' ', 'K', 'u', 'n', 'g', ' ', 'F', 'u',
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		aes__Byte text[] = {
			'T', 'w', 'o', ' ', 'O', 'n', 'e', ' ', 'N', 'i', 'n', 'e', ' ', 'T', 'w', 'o',
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		aes__Byte expected[] = {
			0x29,0xc3,0x50,0x5f,
			0x57,0x14,0x20,0xf6,
			0x40,0x22,0x99,0xb3,
			0x1a,0x02,0xd7,0x3a,
		};

		aes__init_roundkeys(&keys, key, aes__Version::aes128);
		std::size_t encrypted_length = 
			impl.encrypt_ecb(text, text, keys, aes__State_Bytes);
		assert(encrypted_length == aes__State_Bytes * 2);

		std::vector<aes__Byte> padcrypt_ecb(text + aes__State_Bytes, text + (aes__State_Bytes * 2));

		assert(std::equal(text, text + aes__State_Bytes, expected));

		std::size_t decrypted_length =
			impl.decrypt_ecb(text, text, keys, encrypted_length);

		assert(decrypted_length == aes__State_Bytes);
		assert(std::equal(text, text + aes__State_Bytes, "Two One Nine Two"));

		encrypted_length =
			impl.encrypt_cbc(text, text, keys, aes__State_Bytes);
		assert(encrypted_length == aes__State_Bytes * 2);

		std::vector<aes__Byte> padcrypt_cbc(text + aes__State_Bytes, text + (aes__State_Bytes * 2));

		assert(padcrypt_cbc != padcrypt_ecb);
		std::cout << "cbc: " << vecstr(padcrypt_cbc) << std::endl
			<< "ecb" << vecstr(padcrypt_ecb) << std::endl;

		assert(std::equal(text, text + aes__State_Bytes, expected));

		decrypted_length =
			impl.decrypt_cbc(text, text, keys, encrypted_length);

		assert(decrypted_length == aes__State_Bytes);
		assert(std::equal(text, text + aes__State_Bytes, "Two One Nine Two"));
	}
	{
		int REPEAT = 1024 * 1024 * 16;
		aes__Keys keys;
		aes__Byte key[] = {
			'T', 'h', 'a', 't', 's', ' ', 'm', 'y', ' ', 'K', 'u', 'n', 'g', ' ', 'F', 'u',
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
		aes__Byte text[] = {
			'T', 'w', 'o', ' ', 'O', 'n', 'e', ' ', 'N', 'i', 'n', 'e', ' ', 'T', 'w', 'o',
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		aes__init_roundkeys(&keys, key, aes__Version::aes128);

		std::size_t datasize = 1024 * 1024 * 256;
		std::size_t padding = aes__padding(datasize);
		std::string data(datasize + padding, '\0');
		aes__Byte *ptr = reinterpret_cast<aes__Byte*>(&data[0]);
		timeutil::Timer t;
		timeutil::Clocker c;
		t.tic();
		c.tic();
		for (int i = 0; i < REPEAT; ++i)
		{
			impl.encrypt_ecb(text, text, keys, aes__State_Bytes);
		}
		double walltime = t.toc();
		double clocktime = c.toc();
		std::cout << "wall: " << walltime << ", clock: " << clocktime << std::endl;
	}
}
