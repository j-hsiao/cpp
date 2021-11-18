#ifndef AES_AES_H
#	define AES_AES_H
#include <aes/aes_dllcfg.h>

#include <stddef.h>

CPP_EXTERNC_BEGIN

typedef enum
{ aes128=0, aes192=1, aes256=2 } aes_version;

typedef enum
{
	aes_ssl, aes_sslaes, aes_aesni, aes_sse, aes_ui32, aes_uil32, aes_plain, aes_auto,
	aes_rawui32, aes_rawuil32, aes_ui32_t, aes_uil32_t
} aes_impl;

typedef struct aes_pimpl aes_pimpl;
typedef struct aes_Codec
{
	aes_pimpl *pimpl;
	size_t (*encrypt_ecb)(
		const aes_pimpl*, void *dst, const void *src, size_t nbytes);
	size_t (*decrypt_ecb)(
		const aes_pimpl*, void *dst, const void *src, size_t nbytes);
	size_t (*encrypt_cbc)(
		const aes_pimpl*, void *dst, const void *src, size_t nbytes);
	size_t (*decrypt_cbc)(
		const aes_pimpl*, void *dst, const void *src, size_t nbytes);
	void (*free)(aes_pimpl*);
} aes_Codec;
AES_API aes_Codec aes_Codec_new(const void *key, aes_version, aes_impl);
AES_API bool aes_impl_okay(aes_impl impl);


CPP_EXTERNC_END
#endif
