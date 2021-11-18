#include <iostream>
#include <locale>
#include <cstdint>
#include <vector>
#include <cstring>
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include <string>
#undef WIN32_LEAN_AND_MEAN
namespace
{
	std::wstring tows(const std::string &s)
	{
		if (s.size() > INT_MAX)
		{
			throw std::runtime_error(
				"strings > INT_MAX(" + std::to_string(INT_MAX) + ") not supported");
		}
		if (!s.size()) { return {}; }
		int sz = MultiByteToWideChar(
			CP_UTF8, 0, &s[0], static_cast<int>(s.size()), nullptr, 0);
		if (sz == 0)
		{
			throw std::system_error(
				GetLastError(), std::system_category(), "mbs->wcs size");
		}
		std::wstring out(sz, 0);
		sz = MultiByteToWideChar(
			CP_UTF8, 0, &s[0], static_cast<int>(s.size()), &out[0], sz);
		if (sz == 0)
		{
			throw std::system_error(
				GetLastError(), std::system_category(), "mbs->wcs conversion");
		}
		return out;
	}
	std::string tos(const std::wstring &ws)
	{
		if (ws.size() > INT_MAX)
		{
			throw std::runtime_error(
				"strings > INT_MAX(" + std::to_string(INT_MAX) + ") not supported");
		}
		int sz = WideCharToMultiByte(
			CP_UTF8, 0, &ws[0], static_cast<int>(ws.size()),
			nullptr, 0, NULL, nullptr);
		if (sz == 0)
		{
			throw std::system_error(
				GetLastError(), std::system_category(), "wcs->mbs size");
		}
		std::string out(sz, 0);
		sz = WideCharToMultiByte(
			CP_UTF8, 0, &ws[0], static_cast<int>(ws.size()),
			&out[0], sz, NULL, nullptr);
		if (sz == 0)
		{
			throw std::system_error(
				GetLastError(), std::system_category(), "wcs->mbs conversion");
		}
		return out;
	}
}
#endif

namespace
{
	std::locale getlocale()
	{
		try
		{
			return std::locale("en.US_UTF-8");
		}
		catch (std::exception&)
		{
			std::cerr << "fallback, en.US_UTF-8 failed" << std::endl;
			return std::locale("C");
		}
	};
}

template <class chartype>
void check(const std::string &s)
{
	auto loc = getlocale();
	if (std::has_facet<std::codecvt<chartype, char, std::mbstate_t>>(loc))
	{
		auto &facet = std::use_facet<std::codecvt<chartype, char, std::mbstate_t>>(loc);
		std::vector<chartype> buf(s.size(), 0);
		const char *src;
		chartype *dst;
		std::mbstate_t state;
		auto result = facet.in(
			state, &s[0], &s[0]+s.size(), src,
			&buf[0], &buf[0] + s.size(), dst);
		switch (result)
		{
			case facet.ok:
			{
				std::cerr << "\tokay!: " << std::endl;
				std::wcerr << L"\t\"" << &buf[0] << L'"' << std::endl;
				if (sizeof(chartype) == sizeof(wchar_t))
				{
					std::wstring str(dst - &buf[0], 0);
					std::cerr << str.size() << std::endl;
					std::memcpy(&str[0], &buf[0], str.size() * sizeof(wchar_t));
					std::wcerr << L"\t\"" << str << L"\"" << std::endl;
					std::cerr << "wtf?" << std::endl;
					for (auto c : str)
					{ std::cerr << static_cast<int>(c) << ", "; }
					std::cerr << std::endl;
				}
				break;
			}
			case facet.error:
			{
				std::cerr << "\tfailed" << std::endl;
				break;
			}
			case facet.partial:
			{
				std::cerr << "\tpartial" << std::endl;
				break;
			}
			case facet.noconv:
			{
				std::cerr << "\tno conversion" << std::endl;
				break;
			}
			default:
			std::cerr << "\tother" << std::endl;
		}
	}
	else
	{ std::cerr << "\tnope" << std::endl; }
}

template<class chartype>
void checknihao()
{
	const char *nihao_utf8 = "\xe4\xbd\xa0\xe5\xa5\xbd";
	const chartype nihao_utf16[] = {20320, 22909, 0};

	auto loc = getlocale();
	std::cerr << "locale name is " << loc.name() << std::endl;
	if (!std::has_facet<std::codecvt<chartype, char, std::mbstate_t>>(loc))
	{
		std::cerr << "\tno facet" << std::endl;
		return;
	}
	auto &facet = std::use_facet<std::codecvt<chartype, char, std::mbstate_t>>(loc);
	chartype dbuf[6];
	chartype *dptr;
	const char *eptr;
	std::mbstate_t state;
	auto result = facet.in(
		state, nihao_utf8, nihao_utf8+6, eptr,
		dbuf, dbuf + 6, dptr);
	if (result == facet.ok)
	{
		std::cerr << "\tresulting size: " << (dptr - dbuf) << std::endl
			<< '\t';
		for (std::size_t i=0; i < static_cast<std::size_t>(dptr-dbuf); ++i)
		{ std::cerr << static_cast<int>(dbuf[i]) << ", "; }
		std::cerr << std::endl;
	}
	else
	{
		std::cerr << "\tresult: "<< result << std::endl;
		std::cerr << "\tok: " << facet.ok << std::endl;
		std::cerr << "\terror: " << facet.error << std::endl;
		std::cerr << "\tpartial: " << facet.partial << std::endl;
		std::cerr << "\tnoconv: " << facet.noconv << std::endl;
	}
}


int main(int argc, char *argv[])
{
	{
		std::locale loc = getlocale();
		std::cerr << loc.name() << std::endl;
		for (int i=0; i<argc; ++i)
		{
			std::cerr << "input: " << argv[i] << std::endl;
			std::cerr << "wchar" << std::endl;
			check<wchar_t>(argv[i]);
			std::cerr << "char16_t" << std::endl;
			check<char16_t>(argv[i]);
			std::cerr << "int16_t" << std::endl;
			check<int16_t>(argv[i]);
			std::cerr << "uint16_t" << std::endl;
			check<uint16_t>(argv[i]);
		}
	}
	{
		std::cerr << "------------------------------" << std::endl;
		std::cerr << "nihao check" << std::endl;

		std::cerr << "utf8 is " << 0xe4 << ", " << 0xbd << ", " << 0xa0 << ", "
			<< 0xe5 << ", " << 0xa5 << ", " << 0xbd << std::endl;
		std::cerr << "expected utf16: 20320, 22909" << std::endl;

		std::cerr << "wchar_t" << std::endl;
		checknihao<wchar_t>();

		std::cerr << "char16_t" << std::endl;
		checknihao<char16_t>();

		std::cerr << "uint16_t" << std::endl;
		checknihao<std::uint16_t>();

		std::cerr << "int16_t" << std::endl;
		checknihao<std::int16_t>();
	}
#ifdef _WIN32
	{
		const std::string nihao_utf8("\xe4\xbd\xa0\xe5\xa5\xbd", 6);
		const wchar_t nihao_utf16[] = {20320, 22909, 0};
		auto asws = tows(nihao_utf8);
		std::cerr << nihao_utf8.size() << "->" << asws.size() << std::endl;
		for (auto c : asws)
		{ std::cerr << static_cast<int>(c) << std::endl; }
	}
#endif
	return 0;
}
