#ifndef AES_H
#define AES_H
#include <aes/aes_dllcfg.h>
#include <sha256/types.h>

#include <stddef.h>

CPP_EXTERNC_BEGIN

typedef sha256__Byte aes__Byte;
typedef sha256__Word aes__Word;

static const size_t aes__Invalid = -1;
static const size_t aes__Byte_Bits = 8;
static const size_t aes__Word_Bits = 32;
static const size_t aes__Word_Bytes = aes__Word_Bits / aes__Byte_Bits;
static const size_t aes__State_Bits = 128;
static const size_t aes__State_Bytes = aes__State_Bits / aes__Byte_Bits;
static const size_t aes__State_Words = aes__State_Bits / aes__Word_Bits;
static const size_t aes__Max_Keys = 15;
static const size_t aes__Key_Bytes[] = { 16, 24, 32 };
static const size_t aes__Max_Key_Bytes = 32;

typedef enum aes__Version
{
	aes128 = 0,
	aes192 = 1,
	aes256 = 2
} aes__Version;

//padding used (num of padding characters and the
//character used for padding)
inline AES_API aes__Byte aes__padding(size_t datalength)
{ return (aes__Byte)(aes__State_Bytes - (datalength % aes__State_Bytes));}

typedef struct aes__Keys
{
	aes__Word encrypt[aes__Max_Keys][aes__State_Words];
	aes__Word decrypt[aes__Max_Keys][aes__State_Words];
	int version;
	aes__Byte key[aes__Max_Key_Bytes];
} aes__Keys;

AES_API void aes__init_roundkeys(
	aes__Keys *keys, const aes__Byte *key, aes__Version version);

//return aes__Invalid if result not valid
AES_API size_t aes__encrypt_ecb(
	aes__Byte *dst, const aes__Byte *src, const aes__Keys *keys, size_t datalen);
AES_API size_t aes__decrypt_ecb(
	aes__Byte *dst, const aes__Byte *src, const aes__Keys *keys, size_t datalen);
AES_API size_t aes__encrypt_cbc(
	aes__Byte *dst, const aes__Byte *src, const aes__Keys *keys, size_t datalen);
AES_API size_t aes__decrypt_cbc(
	aes__Byte *dst, const aes__Byte *src, const aes__Keys *keys, size_t datalen);

CPP_EXTERNC_END
#endif
