#define AES_DEBUG 1
#include "aes/aes.hpp"
#include "aes/defs.hpp"
#include "aes/plain.hpp"
#include "aes/uil32.hpp"
#include "aes/ui32.hpp"
#include "aes/raw.hpp"
#include "aes/aesni.hpp"
#include "aes/sse.hpp"
#include "aes/sslaes.hpp"
#include "aes/ssl.hpp"
#include "aes/hexlog.hpp"

#include <chrono>
#include <ctime>

#include <string>
#include <cstring>
#include <iostream>
#include <vector>

struct TestCase
{
	const aes::Version version;
	const std::string key;
	const std::string text;
	const std::string encrypted;
};

const std::string key(
	"Thats my Kung Fu"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
	32);
const std::string text1("Two One Nine Two");
const std::string text2("");
const std::string text3("hello world");
const std::string text4("goodbye world");
const std::string text5("Peter Piper picked a pickled plum or something");

static const std::vector<TestCase> ecb_cases = {
	{aes::aes128, key, text1,
		{"\x29\xc3\x50\x5f\x57\x14\x20\xf6\x40\x22\x99\xb3\x1a\x02\xd7\x3a"
		"\xb3\xe4\x6f\x11\xba\x8d\x2b\x97\xc1\x87\x69\x44\x9a\x89\xe8\x68", 32}},
	{aes::aes192, key, text1,
		{"\x36\x54\x13\xa3\x43\x69\x1c\xe6\xe2\x3c\x7a\x67\x2b\xd1\xa6\x95"
		"\x94\xd0\x65\xae\x4c\xa2\xcc\x68\x26\xb1\xff\xa2\xeb\xe7\xb1\x15", 32}},
	{aes::aes256, key, text1,
		{"\x52\x10\x75\x19\x27\x02\x73\x64\xcf\x67\x8e\x1c\xd8\x5e\xe3\x6e"
		"\x28\x5e\x59\x18\x68\xd1\xb7\x1b\x39\x94\xbd\xc6\xf4\x27\x8c\xff", 32}},
	{aes::aes128, key, text2,
		{"\xb3\xe4\x6f\x11\xba\x8d\x2b\x97\xc1\x87\x69\x44\x9a\x89\xe8\x68", 16}},
	{aes::aes192, key, text2,
		{"\x94\xd0\x65\xae\x4c\xa2\xcc\x68\x26\xb1\xff\xa2\xeb\xe7\xb1\x15", 16}},
	{aes::aes256, key, text2,
		{"\x28\x5e\x59\x18\x68\xd1\xb7\x1b\x39\x94\xbd\xc6\xf4\x27\x8c\xff", 16}},
	{aes::aes128, key, text3,
		{"\x8f\x6a\x98\x5e\x1e\xb9\xe7\xd8\xec\xb5\xa4\x06\x86\x50\xb0\xeb", 16}},
	{aes::aes192, key, text3,
		{"\x19\xbe\x35\xd1\x6c\xf3\x04\x50\x49\x80\x07\xea\x15\x72\x78\x95", 16}},
	{aes::aes256, key, text3,
		{"\xda\x3a\xd0\xeb\x27\x87\x22\x18\xed\x2b\xb0\xd2\xed\xec\x54\xe3", 16}},
	{aes::aes128, key, text4,
		{"\x4c\x40\x63\x3f\xff\x12\xf6\xc9\x17\x5c\xc3\x23\x2d\x4a\xcc\xd6", 16}},
	{aes::aes192, key, text4,
		{"\xc0\x49\xfd\x83\x7a\xb8\x46\xbf\xd6\x84\x52\x58\xca\xe4\xbb\xbe", 16}},
	{aes::aes256, key, text4,
		{"\xe9\x3d\x70\x64\x3c\x1c\xc2\xa8\x30\x87\x90\x4e\xa3\x61\x72\x52", 16}},
	{aes::aes128, key, text5,
		{"\x4d\x74\x9f\xcd\x5c\x3b\x4a\xc4\xeb\xb3\x0c\x8c\xbb\x61\xe7\xb5"
		"\x29\xb0\x21\xf0\x55\x99\x9e\x55\xd9\x7d\xb0\x74\x80\x51\x5f\xa1"
		"\xe1\x7e\xeb\x48\xa0\x7f\x71\x89\x73\xd4\x59\xd7\x97\xe2\xb8\xa8", 48}},
	{aes::aes192, key, text5,
		{"\xb3\x7e\xce\x96\x49\x50\x0c\x4d\x2c\xba\x81\x48\x0b\x9e\x16\x4b"
		"\xee\x7d\x0e\x1c\xcb\xc4\x82\x48\x85\x49\xcc\x9b\x65\x41\x34\x56"
		"\x7e\x73\xb1\x21\x5f\x38\x73\xc7\xce\x41\xdf\x06\x3f\x95\xba\x81", 48}},
	{aes::aes256, key, text5,
		{"\x7a\x9e\x53\x3c\x11\x08\x20\x71\x4f\x0a\xa2\x61\xc7\xd2\xbc\x33"
		"\xc7\xd6\x9f\x40\x54\xe2\x49\x53\x72\x85\xdc\x80\xb0\x99\x69\xd8"
		"\x79\xf6\xbf\x16\x1a\x94\xa3\x26\xcd\x0f\xc1\x2a\xab\x7c\x71\x61", 48}}
};
static const std::vector<TestCase> cbc_cases = {
	{aes::aes128, key, text1,
		{"\x29\xc3\x50\x5f\x57\x14\x20\xf6\x40\x22\x99\xb3\x1a\x02\xd7\x3a"
		"\x5f\x59\x17\xec\x37\x6a\x3a\x26\x9e\xfa\xdb\x6b\x2d\x61\xe4\xe3", 32}},
	{aes::aes192, key, text1,
		{"\x36\x54\x13\xa3\x43\x69\x1c\xe6\xe2\x3c\x7a\x67\x2b\xd1\xa6\x95"
		"\x46\x67\xe2\x55\xb6\x9f\x32\x5b\x51\x39\xd3\x6f\x25\xed\xdf\x07", 32}},
	{aes::aes256, key, text1,
		{"\x52\x10\x75\x19\x27\x02\x73\x64\xcf\x67\x8e\x1c\xd8\x5e\xe3\x6e"
		"\xa7\x4c\x1d\x7e\xc4\x5e\x53\xcc\x11\x13\x6a\x72\x97\xa3\x98\x19", 32}},
	{aes::aes128, key, text2,
		{"\xb3\xe4\x6f\x11\xba\x8d\x2b\x97\xc1\x87\x69\x44\x9a\x89\xe8\x68", 16}},
	{aes::aes192, key, text2,
		{"\x94\xd0\x65\xae\x4c\xa2\xcc\x68\x26\xb1\xff\xa2\xeb\xe7\xb1\x15", 16}},
	{aes::aes256, key, text2,
		{"\x28\x5e\x59\x18\x68\xd1\xb7\x1b\x39\x94\xbd\xc6\xf4\x27\x8c\xff", 16}},
	{aes::aes128, key, text3,
		{"\x8f\x6a\x98\x5e\x1e\xb9\xe7\xd8\xec\xb5\xa4\x06\x86\x50\xb0\xeb", 16}},
	{aes::aes192, key, text3,
		{"\x19\xbe\x35\xd1\x6c\xf3\x04\x50\x49\x80\x07\xea\x15\x72\x78\x95", 16}},
	{aes::aes256, key, text3,
		{"\xda\x3a\xd0\xeb\x27\x87\x22\x18\xed\x2b\xb0\xd2\xed\xec\x54\xe3", 16}},
	{aes::aes128, key, text4,
		{"\x4c\x40\x63\x3f\xff\x12\xf6\xc9\x17\x5c\xc3\x23\x2d\x4a\xcc\xd6", 16}},
	{aes::aes192, key, text4,
		{"\xc0\x49\xfd\x83\x7a\xb8\x46\xbf\xd6\x84\x52\x58\xca\xe4\xbb\xbe", 16}},
	{aes::aes256, key, text4,
		{"\xe9\x3d\x70\x64\x3c\x1c\xc2\xa8\x30\x87\x90\x4e\xa3\x61\x72\x52", 16}},
	{aes::aes128, key, text5,
		{"\x4d\x74\x9f\xcd\x5c\x3b\x4a\xc4\xeb\xb3\x0c\x8c\xbb\x61\xe7\xb5"
		"\x21\xec\xed\xe4\xd7\x66\x88\xb2\xed\xd0\x1d\xab\x6a\x56\x09\x48"
		"\x9c\xcf\x48\xa9\x2e\xfe\x3c\x4c\xe4\x88\x40\x5c\xfd\xf0\x84\x3b", 48}},
	{aes::aes192, key, text5,
		{"\xb3\x7e\xce\x96\x49\x50\x0c\x4d\x2c\xba\x81\x48\x0b\x9e\x16\x4b"
		"\x7f\x68\x36\x08\x37\x41\x46\x90\x2c\xb9\x02\x36\xc6\xf2\xfa\xea"
		"\x11\x2a\xc1\x15\x4d\xa7\xca\x1a\xb8\x57\xee\x45\x16\x8c\x50\x88", 48}},
	{aes::aes256, key, text5,
		{"\x7a\x9e\x53\x3c\x11\x08\x20\x71\x4f\x0a\xa2\x61\xc7\xd2\xbc\x33"
		"\xb1\x67\xed\x5f\xc3\x64\x23\x0c\x26\x1e\x69\x98\x79\x29\x6c\x59"
		"\xe3\xbf\xa0\x68\xf6\x5d\xf7\xc5\xb2\xd2\x55\xd7\xbd\xd0\x09\xe6", 48}}
};

template<class Impl>
int check(const TestCase &info)
{
	switch(info.version.Key_Bytes)
	{
		case 16:
			std::cerr << "  aes128";
			break;
		case 24:
			std::cerr << "  aes192";
			break;
		case 32:
			std::cerr << "  aes256";
			break;
	}
	std::cerr << " \"" << info.text << '"' << std::endl;

	std::string buffer1(info.encrypted.size(), 0);
	std::string buffer2(info.encrypted.size(), 0);

	Impl impl(info.key.c_str(), info.version);

	std::size_t encrypted_size = impl.encrypt(
		&buffer1[0], info.text.c_str(), info.text.size());
	if (encrypted_size != info.encrypted.size())
	{
		std::cerr << "encrypted size does not match" << std::endl
			<< encrypted_size << " vs " << info.encrypted.size() << std::endl
			<< "got      " << aes::hex(buffer1) << std::endl
			<< "expected " << aes::hex(info.encrypted) << std::endl;
		return 1;
	}
	if (buffer1 != info.encrypted)
	{
		std::cerr << "encryption failed" << std::endl
			<< "got      " << aes::hex(buffer1) << std::endl
			<< "expected " << aes::hex(info.encrypted) << std::endl;
		return 1;
	}
	std::size_t decrypted_size = impl.decrypt(
		&buffer2[0], buffer1.c_str(), encrypted_size);
	if (decrypted_size != info.text.size())
	{
		std::cerr << "decrypted size does not match: "
			<< decrypted_size << " vs " << info.text.size() << std::endl
			<< aes::hex(buffer2) << std::endl
			<< aes::hex(info.text) << std::endl;
		return 1;
	}
	buffer2.resize(decrypted_size);
	if (buffer2 != info.text)
	{
		std::cerr << "decryption failed" << std::endl
			<< "got      " << aes::hex(buffer2) << std::endl
			<< "expected " << aes::hex(info.text) << std::endl;
		return 1;
	}
	return 0;
}
template<class Impl>
int check_impl(bool dotiming=1)
{
	std::cerr << "testing ecb" << std::endl;
	for (const auto &testcase : ecb_cases)
	{
		if (check<aes::ECB<Impl>>(testcase))
		{ return 1; }
	}

	std::cerr << "testing cbc" << std::endl;
	for (const auto &testcase : cbc_cases)
	{
		if (check<aes::CBC<Impl>>(testcase))
		{ return 1; }
	}

	if (!dotiming)
	{
		std::cerr << "skip timing" << std::endl;
		return 0;
	}
	const char *names[] = {"aes128", "aes192", "aes256"};
	for (const aes::Version &v : {aes::aes128, aes::aes192, aes::aes256})
	{
		std::cerr << "timing " << names[(v.Key_Bytes - 16)/8] << std::endl;
		Impl codec(key.c_str(), v);
		std::string timetest(1024 * 1024 * 64, '\0');
		auto tstart = std::chrono::high_resolution_clock::now();
		auto cstart = std::clock();
		std::string se(timetest.size() + aes::State_Bytes, '\0');
		std::size_t esize = codec.encrypt_ecb(&se[0], timetest.c_str(), timetest.size());
		auto tend = std::chrono::high_resolution_clock::now();
		auto cend = std::clock();
		double clock = static_cast<double>(cend - cstart) / CLOCKS_PER_SEC;
		double wall = std::chrono::duration<double>(tend - tstart).count();
		std::cerr << "64 MB encrypt ecb: wall: " << wall << ", clock: " << clock << std::endl;

		tstart = std::chrono::high_resolution_clock::now();
		cstart = std::clock();
		std::string sd(timetest.size(), '\0');
		std::size_t dsize = codec.decrypt_ecb(&sd[0], se.c_str(), esize);
		tend = std::chrono::high_resolution_clock::now();
		cend = std::clock();
		clock = static_cast<double>(cend - cstart) / CLOCKS_PER_SEC;
		wall = std::chrono::duration<double>(tend - tstart).count();
		std::cerr << "64 MB decrypt ecb: wall: " << wall << ", clock: " << clock << std::endl;

		tstart = std::chrono::high_resolution_clock::now();
		cstart = std::clock();
		std::string sec(timetest.size() + aes::State_Bytes, '\0');
		std::size_t ecsize = codec.encrypt_cbc(&sec[0], timetest.c_str(), esize);
		tend = std::chrono::high_resolution_clock::now();
		cend = std::clock();
		clock = static_cast<double>(cend - cstart) / CLOCKS_PER_SEC;
		wall = std::chrono::duration<double>(tend - tstart).count();
		std::cerr << "64 MB encrypt cbc: wall: " << wall << ", clock: " << clock << std::endl;

		tstart = std::chrono::high_resolution_clock::now();
		cstart = std::clock();
		std::string sdc(timetest.size() + aes::State_Bytes, '\0');
		std::size_t dcsize = codec.decrypt_cbc(&sdc[0], sec.c_str(), ecsize);
		tend = std::chrono::high_resolution_clock::now();
		cend = std::clock();
		clock = static_cast<double>(cend - cstart) / CLOCKS_PER_SEC;
		wall = std::chrono::duration<double>(tend - tstart).count();
		std::cerr << "64 MB decrypt cbc: wall: " << wall << ", clock: " << clock << std::endl;
	}
	return 0;
}

template<::aes_impl pick>
struct CompatCodec: aes::compat::Codec
{
	CompatCodec(const void *key, const aes::Version &v):
		aes::compat::Codec(key, getv(v), pick)
	{}
	private:
		static ::aes_version getv(const aes::Version &v)
		{
			if (v.Round_Keys == aes::aes128.Round_Keys)
			{ return aes::compat::Version::aes128; }
			else if (v.Round_Keys == aes::aes192.Round_Keys)
			{ return aes::compat::Version::aes192; }
			else if (v.Round_Keys == aes::aes256.Round_Keys)
			{ return aes::compat::Version::aes256; }
			throw std::runtime_error("bad version: " + std::to_string(v.Round_Keys));
		}
};

int main(int argc, char *argv[])
{
	bool timeall = argc > 1;
	return (
		0
#if !(defined(_WIN32) && TESTING_DLL)
		|| (std::cerr << "----------testing plain" << std::endl
			&& check_impl<aes::Impl_Plain>(timeall))
		|| (std::cerr << "----------testing rawuil32" << std::endl
			&& check_impl<aes::Impl_RawUIL32>())
		|| (std::cerr << "----------testing rawui32" << std::endl
			&& check_impl<aes::Impl_RawUI32>())
		|| (std::cerr << "----------testing ssl" << std::endl
			&& (aes::Impl_SSL::okay() || ((std::cerr << "skip fallback" << std::endl) && 0))
			&& check_impl<aes::Impl_SSL>())
		|| (std::cerr << "----------testing ssl aes" << std::endl
			&& (aes::Impl_SSLAES::okay() || ((std::cerr << "skip fallback" << std::endl) && 0))
			&& check_impl<aes::Impl_SSLAES>(timeall))
		|| (std::cerr << "----------testing aesni" << std::endl
			&& (aes::Impl_AESNI::okay() || ((std::cerr << "skip fallback" << std::endl) && 0))
			&& check_impl<aes::Impl_AESNI>())
		|| (std::cerr << "----------testing sse" << std::endl
			&& (aes::Impl_SSE::okay() || ((std::cerr << "skip fallback" << std::endl) && 0))
			&& check_impl<aes::Impl_SSE>(timeall))
		|| (std::cerr << "----------testing ui32" << std::endl
			&& check_impl<aes::Impl_UI32>(timeall))
		|| (std::cerr << "----------testing ui32 template" << std::endl
			&& check_impl<aes::Impl_UI32_t>(timeall))
		|| (std::cerr << "----------testing uileast32" << std::endl
			&& check_impl<aes::Impl_UIL32>(timeall))
		|| (std::cerr << "----------testing uileast32 template" << std::endl
			&& check_impl<aes::Impl_UIL32_t>(timeall))
#endif
		|| (std::cerr << "-----------testing compat" << std::endl, 0)
		|| (std::cerr << "----------testing compat plain" << std::endl
			&& check_impl<CompatCodec<::aes_plain>>(timeall))
		|| (std::cerr << "----------testing compat rawuil32" << std::endl
			&& check_impl<CompatCodec<::aes_rawuil32>>())
		|| (std::cerr << "----------testing compat rawui32" << std::endl
			&& check_impl<CompatCodec<::aes_rawui32>>())
		|| (std::cerr << "----------testing compat ssl" << std::endl
			&& (::aes_impl_okay(::aes_ssl) || ((std::cerr << "skip fallback" << std::endl) && 0))
			&& check_impl<CompatCodec<::aes_ssl>>())
		|| (std::cerr << "----------testing compat ssl aes" << std::endl
			&& (::aes_impl_okay(::aes_sslaes) || ((std::cerr << "skip fallback" << std::endl) && 0))
			&& check_impl<CompatCodec<::aes_sslaes>>(timeall))
		|| (std::cerr << "----------testing compat aesni" << std::endl
			&& (::aes_impl_okay(::aes_aesni) || ((std::cerr << "skip fallback" << std::endl) && 0))
			&& check_impl<CompatCodec<::aes_aesni>>())
		|| (std::cerr << "----------testing compat sse" << std::endl
			&& (::aes_impl_okay(::aes_sse) || ((std::cerr << "skip fallback" << std::endl) && 0))
			&& check_impl<CompatCodec<::aes_sse>>(timeall))
		|| (std::cerr << "----------testing compat ui32" << std::endl
			&& check_impl<CompatCodec<::aes_ui32>>(timeall))
		|| (std::cerr << "----------testing compat ui32 template" << std::endl
			&& check_impl<CompatCodec<::aes_ui32_t>>(timeall))
		|| (std::cerr << "----------testing compat uileast32" << std::endl
			&& check_impl<CompatCodec<::aes_uil32>>(timeall))
		|| (std::cerr << "----------testing compat uileast32 template" << std::endl
			&& check_impl<CompatCodec<::aes_uil32_t>>(timeall))
		|| (std::cerr << "----------testing compat auto" << std::endl
			&& check_impl<CompatCodec<::aes_auto>>())
	);
}
