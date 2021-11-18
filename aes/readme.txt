implementations:
Impl_Plain (slow... turn on AES_USE_PLAIN to use this, otherwise aliases to Impl_RawUI32)

Impl_RawUI32	raw implementation of aes using uint_least32_t or uint32_t in C-style for the most part
Impl_RawUIL32	If the system doesn't define uint32_t, then alias RawUI32 to RawUIL32

Impl_UI32	Using c++ and classes to represent aes state
Impl_UIL32	Sometimes these end up slower or faster than the raw impls

Impl_UI32_t	template versions of above, for some reason, they are slower
Impl_UIL32_t	even though the template implementation is identical to the explicit
		implementation of the explicit classes

Impl_AESNI	forward to RawUI32 if not available (aes intrinsics not supported or built with
		AES_USE_AESNI=OFF)

Impl_SSE	Same as Impl_AESNI but use SSE(2,3...)/AVX2

Impl_SSL	Link to openssl and use their EVP interface. Otherwise, forward to RawUI32

Impl_SSLAES	Use openssl's aes header which may include constant-time impl
