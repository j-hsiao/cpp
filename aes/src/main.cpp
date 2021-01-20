#include <aes/aes.hpp>
#define ARGPARSE_MAIN
#include <argparse/argparse.hpp>

#include <stdio.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#endif

namespace
{
	std::string str2hex(const std::string &str)
	{
		static const std::map<char, char> hexmap = {
			{0, '0'}, {1, '1'}, {2, '2'}, {3, '3'}, {4, '4'},
			{5, '5'}, {6, '6'}, {7, '7'}, {8, '8'}, {9, '9'},
			{10, 'A'}, {11, 'B'}, {12, 'C'}, {13, 'D'}, {14, 'E'}, {15, 'F'},
		};
		std::string out(str.size() * 2, 0);
		for (std::size_t i = 0; i < str.size(); ++i)
		{
			unsigned char c = static_cast<unsigned char>(str[i]);
			out[i * 2] = hexmap.at(c >> 4);
			out[(i * 2) + 1] = hexmap.at(c & 0x0F);
		}
		return out;
	}

	std::string hex2str(const std::string &hex)
	{
		static const std::map<char, char> hexmap = {
			{'0', 0}, {'1', 1}, {'2', 2}, {'3', 3}, {'4', 4},
			{'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9},
			{'a', 10}, {'b', 11}, {'c', 12}, {'d', 13}, {'e', 14}, {'f', 15},
			{'A', 10}, {'B', 11}, {'C', 12}, {'D', 13}, {'E', 14}, {'F', 15},
		};
		if (hex.size() % 2)
		{
			throw std::runtime_error("hex should be even number of chars");
		}
		std::string result(hex.size() / 2, 0);
		for (std::size_t i = 0; i < hex.size(); i += 2)
		{
			unsigned char hi = hexmap.at(hex[i]);
			unsigned char lo = hexmap.at(hex[i + 1]);
			result[i / 2] = (hi << 4) | lo;
		}
		return result;
	}

#ifdef _WIN32
	const std::string PATHSEP = "/\\";
	class binstdin
	{
		public:
			binstdin()
			{
				int ifd = _fileno(stdin);
				if (ifd == -1)
				{
					throw std::runtime_error("failed to get stdin fileno");
				}
				if (_setmode(ifd, _O_BINARY) == -1)
				{
					throw std::runtime_error("failed to set stdin to binary");
				}
			}

			std::string get()
			{
				std::stringstream s;
				s << std::cin.rdbuf();
				if (!std::cin)
				{
					throw std::runtime_error("failed reading stdin");
				}
				return s.str();
			}
	};
	class binstdout
	{
		public:
			binstdout()
			{
				int ifd = _fileno(stdout);
				if (ifd == -1)
				{
					throw std::runtime_error("failed to get stdiout");
				}
				if (_setmode(ifd, _O_BINARY) == -1)
				{
					throw std::runtime_error("failed to set stdout to binary");
				}
			}
			void write(const std::string &data)
			{
				std::cout.write(data.data(), data.size());
				if (!std::cout)
				{
					throw std::runtime_error("failed writing to stdout");
				}
			}
	};
#else
	const std::string PATHSEP = "/";
	class binstdin
	{
		public:
			binstdin()
			{
				if (!std::freopen(nullptr, "rb", stdin))
				{
					throw std::runtime_error("failed to change stdin to binary");
				}
			}
			std::string get()
			{
				std::stringstream s;
				s << std::cin.rdbuf();
				if (!std::cin)
				{
					throw std::runtime_error("failed reading from stdin");
				}
				return s.str();
			}
	};

	class binstdout
	{
		public:
			binstdout()
			{
				if (!std::freopen(nullptr, "wb", stdout))
				{
					throw std::runtime_error("failed to change stdout to binary");
				}
			}
			void write(const std::string &data)
			{
				std::cout.write(data.data(), data.size());
				if (!std::cout)
				{
					throw std::runtime_error("failed writing to stdout");
				}
			}
	};
#endif

	std::string basename(const std::string &fname)
	{
		std::size_t end = fname.find_last_of(PATHSEP);
		if (end != std::string::npos)
		{
			return fname.substr(end + 1);
		}
		return fname;
	}

	std::pair<std::string, std::string> splitext(const std::string &fname)
	{
		std::size_t end = fname.find_last_of(".");
		if (end != std::string::npos)
		{
			return {fname.substr(0, end), fname.substr(end)};
		}
		return {fname, ""};
	}

	std::string readall(const std::string &inname)
	{
		if (inname.size())
		{
			std::ifstream in(inname, std::ios::binary | std::ios::in);
			if (in)
			{
				if (in.seekg(0, in.end))
				{
					std::size_t datalen = in.tellg();
					if (in.seekg(0, in.beg))
					{
						std::string s(datalen, 0);
						in.read(&s[0], s.size());
						return s;
					}
					else
					{
						throw std::runtime_error("failed to seek back to beginning");
					}
				}
				std::cerr << "failed to find file size" << std::endl;
				std::stringstream s;
				s << in.rdbuf();
				return s.str();
			}
			else
			{
				throw std::runtime_error("failed to open " + inname);
			}
		}
		else
		{
			std::cerr << "input from stdin" << std::endl;
			binstdin b;
			return b.get();
		}
	}

	void writeall(const std::string &data, const std::string &outname)
	{
		if (outname.size())
		{
			std::ifstream check(outname, std::ios::binary | std::ios::in );
			if (check)
			{
				throw std::runtime_error(
					"output file \"" + outname + "\" already exists");
			}
			check.close();
			std::ofstream out(outname, std::ios::binary | std::ios::out);
			out.write(data.data(), data.size());
		}
		else
		{
			std::cerr << "output to stdout" << std::endl;
			binstdout b;
			b.write(data);
		}
	}

	void process(
		const aes::AES &a,
		const std::string &inname,
		const std::string &outname,
		bool encrypt,
		bool cbc
	)
	{
		std::string s;
		try
		{
			s = readall(inname);
		}
		catch (std::exception &exc)
		{
			std::cerr << "failed to process \"" << inname << "\": " << exc.what() << std::endl;
			return;
		}
		try
		{
			if (encrypt)
			{
				if (cbc) { a.encrypt_cbc(s); }
				else { a.encrypt(s); }
			}
			else
			{
				if (cbc) { a.decrypt_cbc(s); }
				else { a.decrypt(s); }
			}
		}
		catch (std::exception &exc)
		{
			std::cerr << "failed to " 
				<< (encrypt ? "encrypt" : "decrypt") 
				<< " data: " << exc.what() << std::endl;
			return;
		}
		try
		{
			writeall(s, outname);
		}
		catch (std::exception &exc)
		{
			std::cerr << "error writing to \"" + outname + "\": " << exc.what() << std::endl;
			return;
		}
	}
}
int argmain(int argc, char *argv[])
{
	argparse::Parser p(
		{
			{{"inputs"}, "input files", 0, {""}},
			{{"-d", "--decrypt"}, "decrypt, if omitted, encrypt", 0},
			{{"-e", "--extension"}, "extension to use for output files without the .", 1, {"enc"}},
			{{"-o", "--output"}, "output file", 1, {""}},
			{{"-m", "--mode"}, "encryption mode: 128, 192, 256", 1, {"128"}},
			{{"-k", "--key"}, "encryption key (plain text)", 1, {""}},
			{{"-p", "--password"}, "encryption password", 1, {""}},
			{{"-x", "--hex"}, "encryption key (hex)", 1, {""}},
			{{"--cbc"}, "use cbc encryption mode", 0},
			{{"--passes"}, "passes for make_key when giving password", 1, {"1"}},
			{{"-s", "--show"}, "show key in hex and exit", 0}
		},
		"If no inputs, use stdin as single input.\n"
		"If there is only 1 input, then -o indicates the output file, otherwise it is ignored\n"
		"If multiple files, then -o is the output directory (or use original dir and append instead of change extension)\n"
		"key priority is -x, -k, -p\n"
		"input and output are opened separately so inplace is okay\n"
		"all data is read into memory before encryption/decryption, may have ram limitations"
	);
	auto args = p.parsemain(argc, argv);

	const std::string &mode = args.at("mode")[0];
	std::map <std::string, aes::AESVersion> versions = {
		{"128", aes::aes128},
		{"192", aes::aes192},
		{"256", aes::aes256},
	};
	if (!versions.count(mode))
	{ throw std::runtime_error("bad mode " + mode); }
	aes::AESVersion v = versions.at(mode);

	std::string key;
	if (args.at("key")[0].size())
	{
		key = args.at("key")[0];
	}
	else if (args.at("hex")[0].size())
	{
		key = hex2str(args.at("hex")[0]);
	}
	else if (args.at("password")[0].size())
	{
		key = aes::make_key(
			args.at("password")[0],
			v,
			std::stoul(args.at("passes")[0]));
	}
	if (key.size() != aes::NUM_KBYTES[v])
	{
		
		std::cerr << "bad key or password" << std::endl;
		std::cerr << "key should be " << aes::NUM_KBYTES[v]
			<< " bytes but was " << key.size() << std::endl;
		return 1;
	}
	
	if (args.at("show").as<bool>())
	{
		std::cout << str2hex(key) << std::endl;
		return 0;
	}

	bool encrypting = !(args.at("decrypt").as<bool>());
	bool usingCBC = args.at("cbc").as<bool>();

	aes::AES ec(key.data(), v);
	if (args.at("inputs").size() > 1)
	{
		std::cerr << "multi file" << std::endl;
		std::string outdir = args.at("output").values[0];
		for (const std::string &fname : args.at("inputs"))
		{
			std::cerr << "input file: " << fname << std::endl;
			std::string outfile;
			if (outdir.size())
			{
				auto split = splitext(basename(fname));
				outfile = outdir + "/" + split.first  + "." + args.at("extension")[0];
			}
			else
			{
				outfile = fname + "." + args.at("extension")[0];
			}
			std::cerr << "output file: " << outfile << std::endl;
			process(ec, fname, outfile, encrypting, usingCBC);
		}
	}
	else
	{
		std::cerr << "single file" << std::endl;
		std::string input = args.at("inputs")[0];
		std::string output = args.at("output")[0];
		std::cerr << "input file: \"" << input << '"' << std::endl;
		std::cerr << "output file: \"" << output << '"' << std::endl;
		process(ec, input, output, encrypting, usingCBC);
	}
	return 0;
}

//#include <cstring>
//#include <exception>
//#include <stdexcept>
//#include <cstdlib>
//#include <iostream>
//#include <istream>
//#include <ostream>
//#include <vector>
//#include <fstream>
//#include <ctime>
//#include <string>
//#include <memory>
//
//#include "aes.hpp"
//
//#ifndef _WIN32
//# include <termios.h>
//# include <cstdio>
//# include <unistd.h>
//
//  class SilentGetcher 
//  {
//    FILE *_term;
//    struct termios _oldSettings, _newSettings;
//    int _fd;
//    public:
//      SilentGetcher();
//      ~SilentGetcher();
//      char getch();
//  };
//
//  SilentGetcher::SilentGetcher(): _term(0)
//  {
//    _fd = fileno(stdin);
//    if (!isatty(_fd))
//    {
//      _term = fopen("/dev/tty", "rb");
//      if (!_term)
//      {
//        throw std::runtime_error("could not open terminal input");
//      }
//      else
//      {
//        _fd = fileno(_term);
//      }
//    }
//    else
//    {
//      _term = stdin;
//    }
//    tcgetattr(_fd, &_oldSettings);
//    _newSettings = _oldSettings;
//    _newSettings.c_lflag &= ~(ICANON | ECHO);
//    tcsetattr(_fd, TCSANOW, &_newSettings);
//  }
//
//  SilentGetcher::~SilentGetcher()
//  {
//    tcsetattr(_fd, TCSANOW, &_oldSettings);
//    if (_term && (_term != stdin))
//    {
//      fclose(_term);
//    }
//  }
//
//  char SilentGetcher::getch()
//  {
//    return fgetc(_term);
//  }
//
//#else
//
//#include <windows.h>
//    class SilentGetcher 
//    {
//      HANDLE _hIn;
//      DWORD _oldFlags, _newFlags;
//      public:
//        SilentGetcher();
//        ~SilentGetcher();
//        char getch();
//    };
//    SilentGetcher::SilentGetcher():
//      _hIn(CreateFile("CONIN$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))
//    {
//      if (_hIn == INVALID_HANDLE_VALUE)
//      {
//        throw std::runtime_error("unable to open terminal input for password");
//      }
//      GetConsoleMode(_hIn, &_oldFlags);
//      _newFlags = _oldFlags;
//      _newFlags &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
//      SetConsoleMode(_hIn, _newFlags);
//    }
//    SilentGetcher::~SilentGetcher()
//    {
//      SetConsoleMode(_hIn, _oldFlags);
//      CloseHandle(_hIn);
//    }
//
//    char SilentGetcher::getch()
//    {
//      char ch;
//      DWORD dwRead;
//      ReadConsole(_hIn, &ch, 1, &dwRead, NULL);
//      return dwRead ? ch : -1;
//    }
//
//#endif
//
//std::string get_password(bool do_print = 0)
//{
//  std::cerr << "password: " << std::flush;
//  std::string password;
//  SilentGetcher getcher;
//  char ch = getcher.getch();
//  while ((ch != '\r') && (ch != '\n'))
//  {
//    if ((ch == '\b') | (ch == 0x7f))
//    {
//      if (password.size())
//      {
//        password.resize(password.size() - 1);
//        if (do_print)
//        {
//          std::cerr << "\b \b" << std::flush;
//        }
//      }
//    }
//    else
//    {
//      //std::cerr << "got char " << ch << "(" << (int) ch << ")" << std::endl;
//      if (do_print)
//      {
//        std::cerr << "*" << std::flush;
//      }
//      password.push_back(ch);
//    }
//    ch = getcher.getch();
//  }
//  std::cerr << std::endl;
//  return password;
//}
//
//
//
//enum mode
//{
//  m_decrypt,
//  m_encrypt
//};
//
//int main(int argc, char *argv[])
//{
//  try
//  {
//    mode m = m_encrypt;
//    bool getPassword = 1;
//    std::string infile;
//    std::string outfile;
//    std::string password;
//    int aesVersion = AES::AES256;
//    int i = 1;
//    while (i < argc)
//    {
//      std::string tok = argv[i];
//      if (tok == "-d")
//      {
//        m = m_decrypt;
//        std::cerr << "set mode to decrypt" << std::endl;
//      }
//      else if (tok == "-e")
//      {
//        m = m_encrypt;
//        std::cerr << "set mode to encrypt" << std::endl;
//      }
//      else if (tok.substr(0,2) == "-p")
//      {
//        getPassword = 0;
//        //maybe check if anything after password?
//      }
//      else if (tok == "-128")
//      {
//        aesVersion = AES::AES128;
//      }
//      else if (tok == "-192")
//      {
//        aesVersion = AES::AES192;
//      }
//      else if (tok == "-256")
//      {
//        aesVersion = AES::AES256;
//      }
//      else if ((tok == "-h") || (tok == "--help"))
//      {
//        std::cerr
//          << "usage: crypt input output args\n"
//          << "  -d: decrypt\n"
//          << "  -e: encrypt\n"
//          << "  -p: default password\n"
//          << "  -128: use aes 128\n"
//          << "  -192: use aes 192\n"
//          << "  -256 (default): use aes 256"
//          << std::endl;
//        std::exit(0);
//      }
//      else if ((tok[0] != '-') || (tok == "-"))
//      {
//        if (!infile.size())
//        {
//          infile = argv[i];
//        }
//        else if (!outfile.size())
//        {
//          outfile = argv[i];
//        }
//        else
//        {
//          std::cerr << "error: extra filename \"" << argv[i] << "\" given, use -h for help" << std::endl;
//          std::exit(1);
//        }
//      }
//      else
//      {
//        std::cerr << "error: unrecognized argument " << argv[i] << std::endl;
//        std::cerr << "use -h for help" << std::endl;
//        std::exit(1);
//      }
//      i += 1;
//    }
//    if (infile == "-")
//    {
//      infile = "";
//    }
//    if (outfile == "-")
//    {
//      outfile = "";
//    }
//    std::cerr << (m == m_decrypt ? "decrypting " : "encrypting ")
//      << (infile.size() ? infile : "stdin") << " to " << (outfile.size() ? outfile : "stdout") << std::endl;
//
//    auto version = static_cast<AES::AES_KEYSIZE>(aesVersion);
//    if (getPassword)
//    {
//      password = get_password(0);
//    }
//    AES::create_key<std::string>(password, version, password);
//    password.resize(version, 0);
//    AES aes(password);
//
//    std::shared_ptr<std::istream> in;
//    std::shared_ptr<std::ostream> out;
//    //seems fstream with CONIN$ or CONOUT$ doesn't work
//    //on cygwin/windows
//    if (!infile.size())
//    {
//      in = std::make_shared<std::istream>(std::cin.rdbuf());
//    }
//    else
//    {
//      in = std::make_shared<std::ifstream>(infile, std::ifstream::binary);
//    }
//    if (!outfile.size())
//    {
//      out = std::make_shared<std::ostream>(std::cout.rdbuf());
//    }
//    else
//    {
//      out = std::make_shared<std::ofstream>(outfile, std::ofstream::binary);
//    }
//
//    double tdelta = std::clock();
//    if (m == m_encrypt)
//    {
//      aes.encrypt(*in, *out);
//    }
//    else
//    {
//      aes.decrypt(*in, *out);
//    }
//    tdelta = std::clock() - tdelta;
//    std::cerr
//      << ((m == m_encrypt) ? "Encryption" : "Decryption")
//      << " took " << tdelta << " clocks ("
//      << (tdelta / CLOCKS_PER_SEC) << " seconds)."
//      << std::endl;
//  }
//  catch (std::exception &exc)
//  {
//    std::cerr << "Error during execution: " << exc.what() << std::endl;
//  }
//}
