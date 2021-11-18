#include <iostream>
#include <cstdint>
#include <climits>

int main()
{
	std::cerr << "uint_least32_t has size " << sizeof(uint_least32_t) << std::endl;
	std::cerr << "CHAR_BIT is " << CHAR_BIT << std::endl;

	std::uint_least32_t check = 0;
	for (uint_least32_t i = 0; i < sizeof(uint_least32_t); ++i)
	{
		check |= i << (i * CHAR_BIT);
	}
	unsigned char *repr = reinterpret_cast<unsigned char*>(&check);
	bool islittle = 1;
	for (unsigned char i = 0; i < sizeof(uint_least32_t); ++i)
	{
		islittle = islittle && (repr[i] == i);
	}
	if (islittle)
	{
		std::cout << "LITTLE" << std::flush;
		return 0;
	}
	bool isbig = 1;
	for (unsigned char i = 0; i < sizeof(uint_least32_t); ++i)
	{
		isbig = isbig && repr[i] == sizeof(uint_least32_t) - (i + 1);
	}
	if (isbig)
	{
		std::cout << "BIG" << std::flush;
		return 0;
	}
	else
	{
		std::cout << "MIXED" << std::flush;
		return 0;
	}
}
