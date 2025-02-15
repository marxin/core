/*
 * MD4 (RFC-1320) message digest.
 * Modified from MD5 code by Andrey Panin <pazke@donpac.ru>
 *
 * Written by Solar Designer <solar@openwall.com> in 2001, and placed in
 * the public domain.  There's absolutely no warranty.
 *
 * This differs from Colin Plumb's older public domain implementation in
 * that no 32-bit integer data type is required, there's no compile-time
 * endianness configuration, and the function prototypes match OpenSSL's.
 * The primary goals are portability and ease of use.
 *
 * This implementation is meant to be fast, but not as fast as possible.
 * Some known optimizations are not included to reduce source code size
 * and avoid compile-time configuration.
 */

#include "lib.h"
#include "safe-memset.h"
#include "md4.h"

/*
 * The basic MD4 functions.
 */
#define F(x, y, z)	((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)	(((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define H(x, y, z)	((x) ^ (y) ^ (z))

/*
 * The MD4 transformation for all four rounds.
 */
#define STEP(f, a, b, c, d, x, s) \
	(a) += f((b), (c), (d)) + (x);	 \
	(a) = ((a) << (s)) | ((a) >> (32 - (s)))


/*
 * SET reads 4 input bytes in little-endian byte order and stores them
 * in a properly aligned word in host byte order.
 *
 * The check for little-endian architectures which tolerate unaligned
 * memory accesses is just an optimization.  Nothing will break if it
 * doesn't work.
 */
#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
/* uint_fast32_t might be 64 bit, and thus may read 4 more bytes
 * beyond the end of the buffer. So only read precisely 32 bits
 */
#define SET(n)				\
	(*(const uint32_t *)&ptr[(n) * 4])
#define GET(n) \
	SET(n)
#else
#define SET(n) \
	(ctx->block[(n)] = \
	(uint_fast32_t)ptr[(n) * 4] | \
	((uint_fast32_t)ptr[(n) * 4 + 1] << 8) | \
	((uint_fast32_t)ptr[(n) * 4 + 2] << 16) | \
	((uint_fast32_t)ptr[(n) * 4 + 3] << 24))
#define GET(n) \
	(ctx->block[(n)])
#endif

/*
 * This processes one or more 64-byte data blocks, but does NOT update
 * the bit counters.  There're no alignment requirements.
 */
static const void * ATTR_NOWARN_UNUSED_RESULT ATTR_UNSIGNED_WRAPS
	ATTR_NO_SANITIZE_UNDEFINED ATTR_NO_SANITIZE_INTEGER
	ATTR_NO_SANITIZE_IMPLICIT_CONVERSION
body(struct md4_context *ctx, const void *data, size_t size)
{
	const unsigned char *ptr;
	uint32_t a, b, c, d;
	uint32_t saved_a, saved_b, saved_c, saved_d;

	ptr = data;

	a = ctx->a;
	b = ctx->b;
	c = ctx->c;
	d = ctx->d;

	do {
		saved_a = a;
		saved_b = b;
		saved_c = c;
		saved_d = d;

/* Round 1 */
		STEP(F, a, b, c, d, SET( 0),  3);
		STEP(F, d, a, b, c, SET( 1),  7);
		STEP(F, c, d, a, b, SET( 2), 11);
		STEP(F, b, c, d, a, SET( 3), 19);

		STEP(F, a, b, c, d, SET( 4),  3);
		STEP(F, d, a, b, c, SET( 5),  7);
		STEP(F, c, d, a, b, SET( 6), 11);
		STEP(F, b, c, d, a, SET( 7), 19);

		STEP(F, a, b, c, d, SET( 8),  3);
		STEP(F, d, a, b, c, SET( 9),  7);
		STEP(F, c, d, a, b, SET(10), 11);
		STEP(F, b, c, d, a, SET(11), 19);

		STEP(F, a, b, c, d, SET(12),  3);
		STEP(F, d, a, b, c, SET(13),  7);
		STEP(F, c, d, a, b, SET(14), 11);
		STEP(F, b, c, d, a, SET(15), 19);
/* Round 2 */
		STEP(G, a, b, c, d, GET( 0) + 0x5A827999,  3);
		STEP(G, d, a, b, c, GET( 4) + 0x5A827999,  5);
		STEP(G, c, d, a, b, GET( 8) + 0x5A827999,  9);
		STEP(G, b, c, d, a, GET(12) + 0x5A827999, 13);

		STEP(G, a, b, c, d, GET( 1) + 0x5A827999,  3);
		STEP(G, d, a, b, c, GET( 5) + 0x5A827999,  5);
		STEP(G, c, d, a, b, GET( 9) + 0x5A827999,  9);
		STEP(G, b, c, d, a, GET(13) + 0x5A827999, 13);

		STEP(G, a, b, c, d, GET( 2) + 0x5A827999,  3);
		STEP(G, d, a, b, c, GET( 6) + 0x5A827999,  5);
		STEP(G, c, d, a, b, GET(10) + 0x5A827999,  9);
		STEP(G, b, c, d, a, GET(14) + 0x5A827999, 13);

		STEP(G, a, b, c, d, GET( 3) + 0x5A827999,  3);
		STEP(G, d, a, b, c, GET( 7) + 0x5A827999,  5);
		STEP(G, c, d, a, b, GET(11) + 0x5A827999,  9);
		STEP(G, b, c, d, a, GET(15) + 0x5A827999, 13);
/* Round 3 */
		STEP(H, a, b, c, d, GET( 0) + 0x6ED9EBA1,  3);
		STEP(H, d, a, b, c, GET( 8) + 0x6ED9EBA1,  9);
		STEP(H, c, d, a, b, GET( 4) + 0x6ED9EBA1, 11);
		STEP(H, b, c, d, a, GET(12) + 0x6ED9EBA1, 15);

		STEP(H, a, b, c, d, GET( 2) + 0x6ED9EBA1,  3);
		STEP(H, d, a, b, c, GET(10) + 0x6ED9EBA1,  9);
		STEP(H, c, d, a, b, GET( 6) + 0x6ED9EBA1, 11);
		STEP(H, b, c, d, a, GET(14) + 0x6ED9EBA1, 15);

		STEP(H, a, b, c, d, GET( 1) + 0x6ED9EBA1,  3);
		STEP(H, d, a, b, c, GET( 9) + 0x6ED9EBA1,  9);
		STEP(H, c, d, a, b, GET( 5) + 0x6ED9EBA1, 11);
		STEP(H, b, c, d, a, GET(13) + 0x6ED9EBA1, 15);

		STEP(H, a, b, c, d, GET( 3) + 0x6ED9EBA1,  3);
		STEP(H, d, a, b, c, GET(11) + 0x6ED9EBA1,  9);
		STEP(H, c, d, a, b, GET( 7) + 0x6ED9EBA1, 11);
		STEP(H, b, c, d, a, GET(15) + 0x6ED9EBA1, 15);

		a += saved_a;
		b += saved_b;
		c += saved_c;
		d += saved_d;

		ptr += 64;
	} while ((size -= 64) != 0);

	ctx->a = a;
	ctx->b = b;
	ctx->c = c;
	ctx->d = d;

	return ptr;
}

void md4_init(struct md4_context *ctx)
{
	ctx->a = 0x67452301;
	ctx->b = 0xefcdab89;
	ctx->c = 0x98badcfe;
	ctx->d = 0x10325476;

	ctx->lo = 0;
	ctx->hi = 0;
}

void md4_update(struct md4_context *ctx, const void *data, size_t size)
{
	/* @UNSAFE */
	uint_fast32_t saved_lo;
	unsigned long used, free;

	if (size == 0)
		return;

	saved_lo = ctx->lo;
	if ((ctx->lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
		ctx->hi++;
	ctx->hi += size >> 29;

	used = saved_lo & 0x3f;

	if (used != 0) {
		free = 64 - used;

		if (size < free) {
			memcpy(&ctx->buffer[used], data, size);
			return;
		}

		memcpy(&ctx->buffer[used], data, free);
		data = (const unsigned char *) data + free;
		size -= free;
		body(ctx, ctx->buffer, 64);
	}

	if (size >= 64) {
		data = body(ctx, data, size & ~0x3fUL);
		size &= 0x3f;
	}

	memcpy(ctx->buffer, data, size);
}

void ATTR_NO_SANITIZE_UNDEFINED ATTR_NO_SANITIZE_INTEGER
	ATTR_NO_SANITIZE_IMPLICIT_CONVERSION
md4_final(struct md4_context *ctx, unsigned char result[STATIC_ARRAY MD4_RESULTLEN])
{
	/* @UNSAFE */
	unsigned long used, free;

	used = ctx->lo & 0x3f;

	ctx->buffer[used++] = 0x80;

	free = 64 - used;

	if (free < 8) {
		memset(&ctx->buffer[used], 0, free);
		body(ctx, ctx->buffer, 64);
		used = 0;
		free = 64;
	}

	memset(&ctx->buffer[used], 0, free - 8);

	ctx->lo <<= 3;
	ctx->buffer[56] = ctx->lo;
	ctx->buffer[57] = ctx->lo >> 8;
	ctx->buffer[58] = ctx->lo >> 16;
	ctx->buffer[59] = ctx->lo >> 24;
	ctx->buffer[60] = ctx->hi;
	ctx->buffer[61] = ctx->hi >> 8;
	ctx->buffer[62] = ctx->hi >> 16;
	ctx->buffer[63] = ctx->hi >> 24;

	body(ctx, ctx->buffer, 64);

	result[0] = ctx->a;
	result[1] = ctx->a >> 8;
	result[2] = ctx->a >> 16;
	result[3] = ctx->a >> 24;
	result[4] = ctx->b;
	result[5] = ctx->b >> 8;
	result[6] = ctx->b >> 16;
	result[7] = ctx->b >> 24;
	result[8] = ctx->c;
	result[9] = ctx->c >> 8;
	result[10] = ctx->c >> 16;
	result[11] = ctx->c >> 24;
	result[12] = ctx->d;
	result[13] = ctx->d >> 8;
	result[14] = ctx->d >> 16;
	result[15] = ctx->d >> 24;

	i_zero_safe(ctx);
}

void md4_get_digest(const void *data, size_t size,
		    unsigned char result[STATIC_ARRAY MD4_RESULTLEN])
{
	struct md4_context ctx;

	md4_init(&ctx);
	md4_update(&ctx, data, size);
	md4_final(&ctx, result);
}

static void hash_method_init_md4(void *context)
{
	md4_init(context);
}
static void hash_method_loop_md4(void *context, const void *data, size_t size)
{
	md4_update(context, data, size);
}

static void hash_method_result_md4(void *context, unsigned char *result_r)
{
	md4_final(context, result_r);
}

const struct hash_method hash_method_md4 = {
	.name = "md4",
	.block_size = 64, /* block size is 512 bits */
	.context_size = sizeof(struct md4_context),
	.digest_size = MD4_RESULTLEN,

	.init = hash_method_init_md4,
	.loop = hash_method_loop_md4,
	.result = hash_method_result_md4,
};
