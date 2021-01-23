#include <.aes/types.hpp>
#undef NDEBUG
#include <algorithm>
#include <cassert>
#include <iostream>
#include <cstddef>
#include <string>

int main()
{
	{
		aes__Byte checks[16] =
		{
			0x00, 0x01, 0x02, 0x03,
			0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b,
			0x0c, 0x0d, 0x0e, 0x0f
		};
		aes__Byte rrot[16] =
		{
			0x03, 0x00, 0x01, 0x02,
			0x07, 0x04, 0x05, 0x06,
			0x0b, 0x08, 0x09, 0x0a,
			0x0f, 0x0c, 0x0d, 0x0e
		};
		aes__Byte lrot[16] =
		{
			0x01, 0x02, 0x03, 0x00,
			0x05, 0x06, 0x07, 0x04,
			0x09, 0x0a, 0x0b, 0x08,
			0x0d, 0x0e, 0x0f, 0x0c
		};

		aes__Word buf[4];

		aes::load_words(buf, checks, 16);
		for (std::size_t i = 0; i < 16; ++i)
		{
			assert(aes::get_byte(buf[i / aes__Word_Bytes], i % aes__Word_Bytes) == checks[i]);
		}

		for (int i = 0; i < 4; ++i)
		{ buf[i] = aes::lrot(buf[i], 8); }

		for (int w = 0; w < 4; ++w)
		{
			for (int b = 0; b < 4; ++b)
			{
				assert(aes::get_byte(buf[w], b) == lrot[w * 4 + b]);
			}
		}

		std::string tmp(aes__State_Bytes, 0);
		aes::store_words(reinterpret_cast<aes__Byte*>(&tmp[0]), buf, aes__State_Bytes);
		assert(std::equal(lrot, lrot + aes__State_Bytes, reinterpret_cast<const aes__Byte*>(tmp.data())));

		for (int i = 0; i < 4; ++i)
		{ buf[i] = aes::rrot(buf[i], 16); }

		for (int w = 0; w < 4; ++w)
		{
			for (int b = 0; b < 4; ++b)
			{
				assert(aes::get_byte(buf[w], b) == rrot[w * 4 + b]);
			}
		}
		aes::store_words(reinterpret_cast<aes__Byte*>(&tmp[0]), buf, aes__State_Bytes);
		assert(std::equal(rrot, rrot + aes__State_Bytes, reinterpret_cast<const aes__Byte*>(tmp.data())));
	}


	{
		aes__Byte rawdat[aes__State_Bytes] = 
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

		aes__Byte expect[aes__State_Bytes] = 
		{ 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

		aes__Word state[aes__State_Words];
		aes::load_words(state, rawdat, aes__State_Bytes);
		for (aes__Byte i = 0; i < aes__State_Bytes; ++i)
		{
			assert(aes::get_state_byte(state, i) == i);
			aes::set_state_byte(state, aes__State_Bytes - (i + 1), i);
		}
		aes::store_words(rawdat, state, aes__State_Bytes);
		assert(std::equal(rawdat, rawdat + aes__State_Bytes, expect));
	}

	std::cout << "pass" << std::endl;
	return 0;
}
