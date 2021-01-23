#include <aes/aes.h>
#include <.aes/impl.hpp>
#include <tlog/tlog.hpp>
#include <os/os.hpp>


#ifndef AES_DEBUG
#define AES_DEBUG 0
#endif

#ifndef AES_USE_SSL
#define AES_USE_SSL 0
#endif

#ifndef AES_USE_AESNI
#define AES_USE_AESNI 0
#endif

#ifndef AES_USE_SSE
#define AES_USE_SSE 0
#endif

#if AES_USE_AESNI || AES_USE_SSE
#include <cpuinfo/cpuinfo.hpp>
#endif


#include <set>
#include <string>
namespace
{
	typedef tlog::Log<AES_DEBUG> Log;
	//return runtime-determined impl
	const aes::Impl impl()
	{
		static aes::Impl chosen = []()
		{
			std::string AESForce;
			try
			{
				AESForce = os::get_env("AES_FORCE");
			}
			catch (os::NotFound){}
			catch (...) { Log() << "error looking for environment var AES_FORCE" << std::endl; }

			std::set<std::string> checks = {""};
//on my machine
//from testing, it seems ssl seems to be fastest, even faster than my code using
//the aes intrinsics directly, maybe they use the 256/512 versions if available or something?
#if AES_USE_SSL
			checks.insert("SSL");
			checks.insert("ssl");
			if (checks.count(AESForce))
			{
				Log() << "dispatch to OpenSSL impl" << std::endl;
				return aes::make_impl<aes::SSLImpl>();
			}
#endif
			
#if AES_USE_AESNI
			checks.insert("aes");
			checks.insert("AES");
			if (cpuinfo::has("aes") && cpuinfo::has("sse") && checks.count(AESForce))
			{
				Log() << "dispatch to aes intrinsics" << std::endl;
				return aes::make_impl<aes::AESIntrinImpl>();
			}
#endif

#if AES_USE_SSE
			//from testing, sse is slower than plain C code
			//maybe i'm not using the intrinsics correctly?
			checks = {"sse", "SSE"};
			if (
				cpuinfo::has("sse2")
				&& cpuinfo::has("ssse3")
				&& cpuinfo::has("avx2")
				&& checks.count(AESForce))
			{
				Log() << "dispatch to sse2/ssse3/avx2" << std::endl;
				return aes::make_impl<aes::SSEImpl>();
			}
#endif

			Log() << "dispatch to plain impl" << std::endl;
			return aes::make_impl<aes::PlainImpl>();
		}();
		return chosen;
	}
}

size_t aes__encrypt_ecb(
	aes__Byte *dst, const aes__Byte *src, const aes__Keys *keys, size_t datalen)
{
	try { return impl().encrypt_ecb(dst, src, *keys, datalen); }
	catch (std::exception &exc)
	{ Log() << "ecb encrypt error: " << exc.what() << std::endl; }
	catch (...) {}
	return aes__Invalid;
}
size_t aes__decrypt_ecb(
	aes__Byte *dst, const aes__Byte *src, const aes__Keys *keys, size_t datalen)
{
	try { return impl().decrypt_ecb(dst, src, *keys, datalen); }
	catch (std::exception &exc)
	{ Log() << "ecb decrypt error: " << exc.what() << std::endl; }
	catch (...) {}
	return aes__Invalid;
}
size_t aes__encrypt_cbc(
	aes__Byte *dst, const aes__Byte *src, const aes__Keys *keys, size_t datalen)
{
	try { return impl().encrypt_cbc(dst, src, *keys, datalen); }
	catch (std::exception &exc)
	{ Log() << "cbc encrypt error: " << exc.what() << std::endl; }
	catch (...) {}
	return aes__Invalid;
}
size_t aes__decrypt_cbc(
	aes__Byte *dst, const aes__Byte *src, const aes__Keys *keys, size_t datalen)
{ 
	try { return impl().decrypt_cbc(dst, src, *keys, datalen); }
	catch (std::exception &exc)
	{ Log() << "cbc decrypt error: " << exc.what() << std::endl; }
	catch (...) {}
	return aes__Invalid;
}
