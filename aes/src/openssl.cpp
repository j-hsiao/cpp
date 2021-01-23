#include <aes/aes.h>
#include <.aes/impl.hpp>
#include <tlog/tlog.hpp>

#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <climits>

#ifndef AES_DEBUG
#define AES_DEBUG 0
#endif
namespace aes
{
	namespace
	{
		typedef tlog::Log<AES_DEBUG> Log;
		//https://wiki.openssl.org/index.php/Library_Initialization
		//https://wiki.openssl.org/index.php/Libcrypto_API
		struct SSLCryptoInit
		{
			SSLCryptoInit()
			{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
				Log() << "initializing ssl libcrypto" << std::endl;
				ERR_load_crypto_strings();
				OpenSSL_add_all_algorithms();
				OPENSSL_config(NULL);
#endif
			}
			~SSLCryptoInit()
			{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
				Log() << "uninitializing ssl libcrypto" << std::endl;
				EVP_cleanup();
				CRYPTO_cleanup_all_ex_data();
				ERR_free_strings();
#endif
			}
		};

		struct SSLContext
		{
			static const unsigned char Null_IV[aes__State_Bytes];
			static const int Chunk_Size;
			EVP_CIPHER_CTX *ctx;

			SSLContext(
				const aes__Keys &keys, bool cbc, bool encrypting
			):
				ctx(EVP_CIPHER_CTX_new())
			{
				static const SSLCryptoInit libinit;
				const EVP_CIPHER* (*(EVPCiphers[2][3]))() = 
				{
					{ EVP_aes_128_ecb, EVP_aes_192_ecb, EVP_aes_256_ecb },
					{ EVP_aes_128_cbc, EVP_aes_192_cbc, EVP_aes_256_cbc }
				};
				if (ctx)
				{
					const EVP_CIPHER *cipher = EVPCiphers[cbc][keys.version]();
					auto *key = reinterpret_cast<const unsigned char*>(keys.key);
					int result = 0;
					if (encrypting)
					{ result = EVP_EncryptInit_ex(ctx, cipher, nullptr, key, Null_IV); }
					else
					{ result = EVP_DecryptInit_ex(ctx, cipher, nullptr, key, Null_IV); }
					if (result != 1)
					{
						EVP_CIPHER_CTX_free(ctx);
						ctx = nullptr;
					}
				}
			}

			~SSLContext() { if (ctx) { EVP_CIPHER_CTX_free(ctx); } }

			std::size_t encrypt(void *dest, const void *data, std::size_t length)
			{
				auto *dat = reinterpret_cast<const unsigned char*>(data);
				auto *dst = reinterpret_cast<unsigned char*>(dest);
				int steplen = 0;
				std::size_t totlen = 0;
				std::size_t target = length + aes__padding(length);

				while (length)
				{
					int chunklen = length < Chunk_Size ? length : Chunk_Size;
					if (1 != EVP_EncryptUpdate(ctx, dst, &steplen, dat, chunklen))
					{ throw std::runtime_error("failed to encrypt data"); }
					length -= chunklen;
					dat += chunklen;
					dst += steplen;
					totlen += steplen;
				}
				if (1 != EVP_EncryptFinal_ex(ctx, dst, &steplen))
				{ throw std::runtime_error("failed to finalize encyprt data"); }
				totlen += steplen;
				if (totlen != target)
				{
					Log() << "expected " << target << " but got " << totlen << std::endl;
					throw std::runtime_error("unexpected encryption length");
				}
				return totlen;
			}

			std::size_t decrypt(void *dest, const void *data, std::size_t length)
			{
				auto *dat = reinterpret_cast<const unsigned char*>(data);
				auto *dst = reinterpret_cast<unsigned char*>(dest);
				std::size_t upper = length;
				std::size_t lower = upper - aes__State_Bytes;
				int steplen = 0;
				std::size_t totlen = 0;
				while (length)
				{
					int chunklen = length < Chunk_Size ? length : Chunk_Size;
					if (1 != EVP_DecryptUpdate(ctx, dst, &steplen, dat, chunklen))
					{ throw std::runtime_error("failed to decrypt data"); }
					length -= chunklen;
					dat += chunklen;
					dst += steplen;
					totlen += steplen;
				}
				if (1 != EVP_DecryptFinal_ex(ctx, dst, &steplen))
				{ throw std::runtime_error("failed to finalize decrypt data"); }
				totlen += steplen;
				if (totlen >= upper || totlen < lower)
				{
					Log() << "expected result to be in range [" << lower << ", " << upper
						<< ") but got " << totlen << std::endl;
					throw std::runtime_error("unexpected decryption length");
				}
				return totlen;
			}
		};
		const int SSLContext::Chunk_Size = INT_MAX;
		const unsigned char SSLContext::Null_IV[aes__State_Bytes] = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};
	}

	std::size_t SSLImpl::encrypt_ecb(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{
		SSLContext ctx(keys, 0, 1);
		if (ctx.ctx)
		{ return ctx.encrypt(dst, src, datalen); }
		else
		{
			Log() << "failed to initialize ssl context, fallback to plain" << std::endl;
			return PlainImpl::encrypt_ecb(dst, src, keys, datalen);
		}
	}
	std::size_t SSLImpl::decrypt_ecb(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{
		SSLContext ctx(keys, 0, 0);
		if (ctx.ctx)
		{ return ctx.decrypt(dst, src, datalen); }
		else
		{
			Log() << "failed to initialize ssl context, fallback to plain" << std::endl;
			return PlainImpl::decrypt_ecb(dst, src, keys, datalen);
		}
	}
	std::size_t SSLImpl::encrypt_cbc(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{
		SSLContext ctx(keys, 1, 1);
		if (ctx.ctx)
		{ return ctx.encrypt(dst, src, datalen); }
		else
		{
			Log() << "failed to initialize ssl context, fallback to plain" << std::endl;
			return PlainImpl::encrypt_cbc(dst, src, keys, datalen);
		}
	}
	std::size_t SSLImpl::decrypt_cbc(
		aes__Byte *dst, const aes__Byte *src,
		const aes__Keys &keys, size_t datalen)
	{
		SSLContext ctx(keys, 1, 0);
		if (ctx.ctx)
		{ return ctx.decrypt(dst, src, datalen); }
		else
		{
			Log() << "failed to initialize ssl context, fallback to plain" << std::endl;
			return PlainImpl::decrypt_cbc(dst, src, keys, datalen);
		}
	}
}
