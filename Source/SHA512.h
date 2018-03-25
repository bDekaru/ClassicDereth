
#ifndef SHA512_H
#define SHA512_H

#include <stddef.h>
#include <stdint.h>

#if !defined(SHA512_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

	/**
	* \brief          SHA-512 context structure
	*/
	typedef struct
	{
		uint64_t total[2];          /*!< number of bytes processed  */
		uint64_t state[8];          /*!< intermediate digest state  */
		unsigned char buffer[128];  /*!< data block being processed */
		int is384;                  /*!< 0 => SHA-512, else SHA-384 */
	}
	sha512_context;

	/**
	* \brief          Initialize SHA-512 context
	*
	* \param ctx      SHA-512 context to be initialized
	*/
	void sha512_init(sha512_context *ctx);

	/**
	* \brief          Clear SHA-512 context
	*
	* \param ctx      SHA-512 context to be cleared
	*/
	void sha512_free(sha512_context *ctx);

	/**
	* \brief          Clone (the state of) a SHA-512 context
	*
	* \param dst      The destination context
	* \param src      The context to be cloned
	*/
	void sha512_clone(sha512_context *dst,
		const sha512_context *src);

	/**
	* \brief          SHA-512 context setup
	*
	* \param ctx      context to be initialized
	* \param is384    0 = use SHA512, 1 = use SHA384
	*/
	void sha512_starts(sha512_context *ctx, int is384);

	/**
	* \brief          SHA-512 process buffer
	*
	* \param ctx      SHA-512 context
	* \param input    buffer holding the  data
	* \param ilen     length of the input data
	*/
	void sha512_update(sha512_context *ctx, const unsigned char *input,
		size_t ilen);

	/**
	* \brief          SHA-512 final digest
	*
	* \param ctx      SHA-512 context
	* \param output   SHA-384/512 checksum result
	*/
	void sha512_finish(sha512_context *ctx, unsigned char output[64]);

#ifdef __cplusplus
}
#endif

#else  /* SHA512_ALT */
#include "sha512_alt.h"
#endif /* SHA512_ALT */

#ifdef __cplusplus
extern "C" {
#endif

	/**
	* \brief          Output = SHA-512( input buffer )
	*
	* \param input    buffer holding the  data
	* \param ilen     length of the input data
	* \param output   SHA-384/512 checksum result
	* \param is384    0 = use SHA512, 1 = use SHA384
	*/
	void SHA512(const unsigned char *input, size_t ilen, unsigned char output[64]);

	/* Internal use */
	void sha512_process(sha512_context *ctx, const unsigned char data[128]);

#ifdef __cplusplus
}
#endif

std::string SHA512(const void *input, size_t ilen);

#endif /* sha512.h */

