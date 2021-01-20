#ifndef AES_H
#define AES_H

#ifndef AES_API

#ifdef _WIN32

#ifdef aes_EXPORTS
#define AES_API __declspec(dllexport)
#else
#define AES_API __declspec(dllimport)
#endif

#else
#define AES_API
#endif

#endif

#include <cstddef>
#include <memory>

namespace aes
{

	//------------------------------
	// constants/enums
	//------------------------------
	enum AESVersion : int { aes128 = 0, aes192 = 1, aes256 = 2 };
	enum AESEndian : int { mix_endian = 0, big_endian = 1, little_endian = 2 };
	static const std::size_t k_bytebits = 8;
	static const std::size_t k_wordbytes = 4;
	static const std::size_t STATEWORDS = 4;
	static const std::size_t STATEBYTES = k_wordbytes * STATEWORDS;
	static const std::size_t NUM_KBYTES[] = {16, 24, 32};

	//------------------------------
	// classes
	//------------------------------
	class AES_API AES
	{
			struct implcls;
			std::shared_ptr<implcls> impl;

		public:
			static const int ENDIAN;
			static std::size_t padding(std::size_t size) { return STATEBYTES - (size % STATEBYTES); }

			AES(const void *key, AESVersion version);

			//encrypt a buffer, should be approprivately sized
			void encrypt(void *data, std::size_t length) const;
			void encrypt_cbc(void *data, std::size_t length) const;
			//decrypt a buffer, returns the amount of padding at end
			std::size_t decrypt(void *data, std::size_t length) const;
			std::size_t decrypt_cbc(void *data, std::size_t length) const;

			//encrypt/decrypt for containers
			template<class T, void (AES::*f)(void *data, std::size_t length) const = &AES::encrypt>
			void encrypt(T &container) const
			{
				static_assert(sizeof(typename T::value_type) == 1, "container should have value_type of 1 byte");
				std::size_t origlen = container.size();
				std::size_t p = padding(origlen);
				container.resize(container.size() + p);
				(this->*f)(&container[0], origlen);
			}
			template<class T, std::size_t (AES::*f)(void *data, std::size_t length) const = &AES::decrypt>
			void decrypt(T &container) const
			{
				static_assert(sizeof(typename T::value_type) == 1, "container should have value_type of 1 byte");
				container.resize(
					container.size() - 
					(this->*f)(&container[0], container.size()));
			}

			template<class T>
			void encrypt_cbc(T &container) const
			{
				encrypt<T, &AES::encrypt_cbc>(container);
			}
			template<class T>
			void decrypt_cbc(T &container) const
			{
				decrypt<T, &AES::decrypt_cbc>(container);
			}
	};

	//------------------------------
	// functions
	//------------------------------
	AES_API void make_key(
		void *dst, const void *src, std::size_t srcsize,
		AESVersion version = aes128, int passes = 5);
	template<class T> T make_key(
		const T &password, AESVersion version = aes128, int passes = 5);
	template<class T> AES aes(
		const T &password, AESVersion version = aes128);



	//------------------------------
	//template/inline implementations
	//------------------------------
	template<class T>
	inline T make_key(const T &password, AESVersion version, int passes)
	{
		static_assert(sizeof(typename T::value_type) == 1, "container should have value_type of 1 byte");
		T ret(NUM_KBYTES[version], 0);
		make_key(&ret[0], &password[0], password.size(), version, passes);
		return ret;
	}

	template<class T>
	inline AES aes(const T &password, AESVersion version)
	{
		auto k = make_key(password, version);
		return {&k[0], version};
	}
}
#endif
