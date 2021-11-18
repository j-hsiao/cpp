// OpenSSL crypto "requires intialization" (maybe)
// Also when threading, needs some locking or whatever. Too lazy to go
// look and docs don't say how to set, only what to set... so leave
// that all to the user of this library.
#include "aes/ssl.hpp"
#include "aes/raw.hpp"

#if AES_USE_SSL
#include "aes/defs.hpp"

#include "openssl/conf.h"
#include "openssl/crypto.h"
#include "openssl/evp.h"
#include "openssl/err.h"

#include <cstdio>
#include <stdexcept>
#include <climits>

namespace aes
{
	namespace
	{
		bool initssl()
		{
#			if OPENSSL_VERSION_NUMBER < 0x10100000L
			ERR_load_crypto_strings();
			OpenSSL_add_all_algorithms();
			OPENSSL_config(NULL);
#			endif
			//Don't bother with deinitializing because client/other library
			//may still be using openssl.
			//Also... I think inits will do nothing if already initialized?
			return 1;
		}
		struct sslimpl: public Impl_SSL::Impl
		{
			Version version;
			AES_FALLBACK bak;
			unsigned char key[Max_Key_Bytes];
			const EVP_CIPHER *cbc_cipher;
			const EVP_CIPHER *ecb_cipher;
			const unsigned char nulliv[State_Bytes];

			sslimpl(const void *key_, const Version &version_):
				version(version_),
				bak(key_, version_),
				nulliv{0}
			{
				static bool inited = initssl();
				std::memcpy(key, key_, version.Key_Bytes);
				const EVP_CIPHER* (*(CBCCiphers[]))() = {
					EVP_aes_128_cbc, EVP_aes_192_cbc, EVP_aes_256_cbc };
				const EVP_CIPHER* (*(ECBCiphers[]))() = {
					EVP_aes_128_ecb, EVP_aes_192_ecb, EVP_aes_256_ecb };
				std::size_t idx = (version.Key_Bytes / Byte_Bits)-2;
				cbc_cipher = CBCCiphers[idx]();
				ecb_cipher = ECBCiphers[idx]();
			}
		
			//0 if fail
			std::size_t encrypt(
				void *dst_, const void *src_, std::size_t nbytes,
				const EVP_CIPHER *cipher) const
			{
				auto ctx = EVP_CIPHER_CTX_new();
				if (!ctx) { return 0; }
				//EVP_CIPHER_CTX_set_padding(ctx, 1);
				if (1 != EVP_EncryptInit_ex(ctx, cipher, nullptr, key, nulliv))
				{
					EVP_CIPHER_CTX_free(ctx);
					return 0;
				}
				auto *src = reinterpret_cast<const unsigned char*>(src_);
				auto *dst = reinterpret_cast<unsigned char*>(dst_);

				std::size_t totlen = 0;
				int steplen = 0;
				std::size_t remain = nbytes;
				while (remain)
				{
					int chunksize = remain>INT_MAX ? INT_MAX : static_cast<int>(remain);
					if (1 != EVP_EncryptUpdate(ctx, dst, &steplen, src, chunksize))
					{
						EVP_CIPHER_CTX_free(ctx);
						return 0;
					}
					totlen += steplen;
					dst += steplen;
					src += chunksize;
					remain -= chunksize;
				}
				int result = EVP_EncryptFinal_ex(ctx, dst, &steplen);
				EVP_CIPHER_CTX_free(ctx);
				if (1 != result) { return 0; }
				totlen += steplen;
				remain = nbytes % State_Bytes;
				std::size_t target = nbytes + State_Bytes - remain;
				return totlen == target ? target : 0;
			}
			//nbytes if fail
			std::size_t decrypt(
				void *dst_, const void *src_, std::size_t nbytes,
				const EVP_CIPHER *cipher) const
			{
				auto ctx = EVP_CIPHER_CTX_new();
				if (!ctx) { return nbytes; }
				//EVP_CIPHER_CTX_set_padding(ctx, 1);
				if (1 != EVP_DecryptInit_ex(ctx, cipher, nullptr, key, nulliv))
				{
					EVP_CIPHER_CTX_free(ctx);
					return nbytes;
				}
				auto *src = reinterpret_cast<const unsigned char*>(src_);
				auto *dst = reinterpret_cast<unsigned char*>(dst_);

				std::size_t totlen = 0;
				int steplen = 0;
				std::size_t remain = nbytes;
				while (remain)
				{
					int chunksize = remain > INT_MAX ? INT_MAX : static_cast<int>(remain);
					if (1 != EVP_DecryptUpdate(ctx, dst, &steplen, src, chunksize))
					{
						EVP_CIPHER_CTX_free(ctx);
						return nbytes;
					}
					totlen += steplen;
					dst += steplen;
					src += chunksize;
					remain -= chunksize;
				}
				int result = EVP_DecryptFinal_ex(ctx, dst, &steplen);
				EVP_CIPHER_CTX_free(ctx);
				if (1 != result) { return nbytes; }
				totlen += steplen;
				return totlen;
			}

			std::size_t encrypt_ecb(
				void *dst, const void *src, std::size_t nbytes) const
			{
				auto ret = encrypt(dst, src, nbytes, ecb_cipher);
				if (ret) { return ret; }
				else if (src != dst) { return bak.encrypt_ecb(dst, src, nbytes); }
				else { throw std::runtime_error("ssl encrypt ecb inplace failed"); }
			}
			std::size_t decrypt_ecb(
				void *dst, const void *src, std::size_t nbytes) const
			{
				auto ret = decrypt(dst, src, nbytes, ecb_cipher);
				if (ret != nbytes) { return ret; }
				else if (src != dst) { return bak.decrypt_ecb(dst, src, nbytes); }
				else { throw std::runtime_error("ssl decrypt ecb inplace failed"); }
			}
			std::size_t encrypt_cbc(
				void *dst, const void *src, std::size_t nbytes) const
			{
				auto ret = encrypt(dst, src, nbytes, cbc_cipher);
				if (ret) { return ret; }
				else if (src != dst) { return bak.encrypt_cbc(dst, src, nbytes); }
				else { throw std::runtime_error("ssl encrypt cbc inplace failed"); }
			}
			std::size_t decrypt_cbc(
				void *dst, const void *src, std::size_t nbytes) const
			{
				auto ret = decrypt(dst, src, nbytes, cbc_cipher);
				if (ret != nbytes) { return ret; }
				else if (src != dst) { return bak.decrypt_cbc(dst, src, nbytes); }
				else { throw std::runtime_error("ssl decrypt cbc inplace failed"); }
			}
		};
	}
	Impl_SSL::Impl_SSL(const void *key, const Version &v):
		impl(std::make_shared<sslimpl>(key, v))
	{}
	bool Impl_SSL::okay() { return 1; }
}
#else
namespace aes
{
	namespace
	{
		struct fallback: public aes::Impl_SSL::Impl
		{
			aes::AES_FALLBACK f;

			fallback(const void *key, const Version &v): f(key, v) {}
			virtual std::size_t encrypt_ecb(void *dst, const void *src, std::size_t sz) const
			{ return f.encrypt_ecb(dst, src, sz); }
			virtual std::size_t decrypt_ecb(void *dst, const void *src, std::size_t sz) const
			{ return f.decrypt_ecb(dst, src, sz); }
			virtual std::size_t encrypt_cbc(void *dst, const void *src, std::size_t sz) const
			{ return f.encrypt_cbc(dst, src, sz); }
			virtual std::size_t decrypt_cbc(void *dst, const void *src, std::size_t sz) const
			{ return f.decrypt_cbc(dst, src, sz); }
		};
	}
	Impl_SSL::Impl_SSL(const void *key, const Version &v):
		impl(std::make_shared<fallback>(key, v))
	{}
	bool Impl_SSL::okay() { return 0; }
}
#endif


namespace aes
{ Impl_SSL::~Impl_SSL() {} }
