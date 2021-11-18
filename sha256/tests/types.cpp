#include <sha256/types.h>
#undef NDEBUG
#include <algorithm>
#include <cassert>
#include <iostream>
#include <cstddef>
#include <vector>

int main(int argc, char *argv[])
{
	unsigned char data[16] = 
	{
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f,
	};

	const std::size_t bufsize = sha256__normalized_size(16);
	std::vector<sha256__Byte> buf(bufsize, 0);
	std::vector<sha256__Word> words(bufsize / 4 + (bufsize % 4 > 0), 0);

	sha256__Byte *normed = sha256__normalize(&buf[0], data, 16);
	if (!normed) { normed = data; }

	sha256__Byte normdat[16] = 
	{
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f,
	};

	sha256__load_words(&words[0], normdat, bufsize);

	sha256__Word expected[4] =
	{
		0x00010203,
		0x04050607,
		0x08090a0b,
		0x0c0d0e0f
	};

	assert(
		(sha256__get_byte(words[0], 0) == 0)
		&& (sha256__get_byte(words[0], 1) == 1)
		&& (sha256__get_byte(words[0], 2) == 2)
		&& (sha256__get_byte(words[0], 3) == 3)
		&& (sha256__get_byte(words[1], 0) == 4)
		&& (sha256__get_byte(words[1], 1) == 5)
		&& (sha256__get_byte(words[1], 2) == 6)
		&& (sha256__get_byte(words[1], 3) == 7)
		&& (sha256__get_byte(words[2], 0) == 8)
		&& (sha256__get_byte(words[2], 1) == 9)
		&& (sha256__get_byte(words[2], 2) == 10)
		&& (sha256__get_byte(words[2], 3) == 11)
		&& (sha256__get_byte(words[3], 0) == 12)
		&& (sha256__get_byte(words[3], 1) == 13)
		&& (sha256__get_byte(words[3], 2) == 14)
		&& (sha256__get_byte(words[3], 3) == 15)
	);

	sha256__Word x = 0;
	sha256__set_byte(&x, 0xFF, 0);
	assert(x == 0xFF000000);
	sha256__set_byte(&x, 0xFF, 1);
	assert(x == 0xFFFF0000);
	sha256__set_byte(&x, 0xFF, 2);
	assert(x == 0xFFFFFF00);
	sha256__set_byte(&x, 0xFF, 3);
	assert(x == 0xFFFFFFFF);

	assert(sha256__rrot(0x01020304, 8) == 0x04010203);
	assert(sha256__lrot(0x01020304, 8) == 0x02030401);

	assert(std::equal(words.begin(), words.end(), expected));

	std::cout << "pass" << std::endl;
	return 0;
}
