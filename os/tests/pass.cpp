#include <os/os.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
	std::string line;
	std::cout << ">>> " << std::flush;
	os::ebuf ebuf;
	os::Term term("rb");
	bool showing = 1;
	while (1)
	{
		line = term.readline();
		if (!showing) { std::cout << std::endl; }
		if (line == "quit" || line == "exit")
		{ return 0; }
		std::cout << line << std::endl << ">>> " << std::flush;
		if (line == "h")
		{
			try { term.hide_input(); showing = 0; }
			catch (std::exception &exc)
			{ std::cerr << "error hiding input: " << exc.what() << std::endl; }
		}
		else if (line == "s")
		{
			try { term.show_input(); showing = 1; }
			catch (std::exception &exc)
			{ std::cerr << "error showing input: " << exc.what() << std::endl; }
		}
	}
	return 0;
}
