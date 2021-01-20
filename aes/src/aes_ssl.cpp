#include <aes/aes_ssl.hpp>
#include <aes/aes.hpp>
#include <aes/aes_util.hpp>
#include <aes/aes_plain.hpp>
#include <aes/aes_types.hpp>

#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <stdexcept>
#include <map>

namespace aes
{
	namespace
	{
		//docs not very clear
		//is this really needed?
		//i compiled and tested some funcs
		//without doing this and it seemed
		//to work fine...
		//automatically initialize library
		//and deinitialize afterwards
		class CryptoInitializer
		{
			private:
				static bool initialized;
				CryptoInitializer()
				{
					if (initialized)
					{
						return;
					}
					logd("initializing crypto");
					ERR_load_crypto_strings();
					OpenSSL_add_all_algorithms();
					OPENSSL_config(nullptr);
					initialized = 1;
				}
				~CryptoInitializer()
				{
					logd("uninitializing crypto");
					EVP_cleanup();
					CRYPTO_cleanup_all_ex_data();
					ERR_free_strings();
				}
		};
		bool CryptoInitializer::initialized = 0;
		const std::map<std::size_t, const EVP_CIPHER* (*)(void)> ECB
		{
			{11, &EVP_aes_128_ecb},
			{13, &EVP_aes_192_ecb},
			{15, &EVP_aes_256_ecb}
		};
		const std::map<std::size_t, const EVP_CIPHER* (*)(void)> CBC
		{
			{11, &EVP_aes_128_cbc},
			{13, &EVP_aes_192_cbc},
			{15, &EVP_aes_256_cbc}
		};

		struct SSLCTX
		{
			static const CryptoInitializer init_ssl_crypto;
			static const unsigned char IV[STATEBYTES];
			EVP_CIPHER_CTX *ctx;
			SSLCTX(
				const AES_statevec &keys,
				bool cbc,
				bool encrypting
			):
				ctx(EVP_CIPHER_CTX_new())
			{
				if (ctx)
				{
					const EVP_CIPHER *cipher;
					if (cbc)
					{
						cipher = (*CBC.at(keys.size()))();
					}
					else
					{
						cipher = (*ECB.at(keys.size()))();
					}
					auto *key = reinterpret_cast<const unsigned char*>(keys.data());
					int result = 0;
					if (encrypting)
					{ result = EVP_EncryptInit_ex(ctx, cipher, nullptr, key, IV); }
					else
					{ result = EVP_DecryptInit_ex(ctx, cipher, nullptr, key, IV); }
					if (result != 1)
					{
						EVP_CIPHER_CTX_free(ctx);
						ctx = nullptr;
					}
				}
			}

			~SSLCTX()
			{
				if (ctx)
				{
					EVP_CIPHER_CTX_free(ctx);
				}
			}

			void encrypt(void *data, std::size_t length)
			{
				int len = static_cast<int>(length);
				if (len != length)
				{
					throw std::runtime_error(
						"data length cast to int for openssl interface size does not match");
				}
				auto *dat = reinterpret_cast<unsigned char *>(data);
				int steplen = 0;
				int totlen = 0;
				if (1 != EVP_EncryptUpdate(ctx, dat, &totlen, dat, len))
				{
					throw std::runtime_error("failed to encrypt data, indeterminate state");
				}
				if (1 != EVP_EncryptFinal_ex(ctx, dat + totlen, &steplen))
				{
					throw std::runtime_error("failed to encrypt data, indeterminate state");
				}
				totlen += steplen;
				if (totlen != length + AES::padding(length))
				{
					throw std::runtime_error("bad encryption length");
				}
			}

			std::size_t decrypt(void *data, std::size_t length)
			{
				int len = static_cast<int>(length);
				if (len != length)
				{
					throw std::runtime_error(
						"data length cast to int for openssl interface size does not match");
				}
				auto *dat = reinterpret_cast<unsigned char *>(data);
				int steplen = 0;
				int totlen = 0;
				if (1 != EVP_DecryptUpdate(ctx, dat, &totlen, dat, len))
				{
					throw std::runtime_error("failed to decrypt data, indeterminate state");
				}
				if (1 != EVP_DecryptFinal_ex(ctx, dat + totlen, &steplen))
				{
					throw std::runtime_error("failed to decrypt data, indeterminate state");
				}
				totlen += steplen;
				if ((totlen < length) && ((totlen + STATEBYTES) >= length))
				{
					return length - totlen;
				}
				throw std::runtime_error("invalid decryption length");
			}

		};
		const unsigned char SSLCTX::IV[STATEBYTES] = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};
	}

	void encrypt_ssl_ecb(
		void *data, std::size_t length, const AES_statevec &keys)
	{
		SSLCTX ctx(keys, 0, 1);
		if (ctx.ctx)
		{
			ctx.encrypt(data, length);
		}
		else
		{
			logd("failed to create ssl context, falling back to plain impl");
			encrypt_ecb<AES_plain>(data, length, keys);
		}
	}
	std::size_t decrypt_ssl_ecb(
		void *data,
		std::size_t length,
		const AES_statevec &keys,
		const AES_statevec &ikeys)
	{
		SSLCTX ctx(keys, 0, 0);
		if (ctx.ctx)
		{
			return ctx.decrypt(data, length);
		}
		else
		{
			logd("failed to create ssl context, falling back to plain impl");
			return decrypt_ecb<AES_plain>(data, length, keys, ikeys);
		}
	}

	void encrypt_ssl_cbc(
		void *data,
		std::size_t length,
		const AES_statevec &keys)
	{
		SSLCTX ctx(keys, 1, 1);
		if (ctx.ctx)
		{
			ctx.encrypt(data, length);
		}
		else
		{
			logd("failed to create ssl context, falling back to plain impl");
			encrypt_cbc<AES_plain>(data, length, keys);
		}
	}
	std::size_t decrypt_ssl_cbc(
		void *data,
		std::size_t length,
		const AES_statevec &keys,
		const AES_statevec &ikeys)
	{
		SSLCTX ctx(keys, 1, 0);
		if (ctx.ctx)
		{
			return ctx.decrypt(data, length);
		}
		else
		{
			logd("failed to create ssl context, falling back to plain impl");
			return decrypt_cbc<AES_plain>(data, length, keys, ikeys);
		}
	}


}
