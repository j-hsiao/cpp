#ifndef AES_SSL_H
#define AES_SSL_H

#include <aes/aes_types.hpp>

namespace aes
{
	void encrypt_ssl_ecb(
		void *data, std::size_t length, const AES_statevec &keys);

	std::size_t decrypt_ssl_ecb(
		void *data,
		std::size_t length,
		const AES_statevec &keys,
		const AES_statevec &ikeys);

	void encrypt_ssl_cbc(
		void *data, std::size_t length, const AES_statevec &keys);

	std::size_t decrypt_ssl_cbc(
		void *data,
		std::size_t length,
		const AES_statevec &keys,
		const AES_statevec &ikeys);
}
#endif
