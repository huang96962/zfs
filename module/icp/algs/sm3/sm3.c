/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://opensource.org/licenses/CDDL-1.0.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 *
 * CDDL HEADER END
 *
 */
/*
 * Copyright (c) 2021, Beijing Asia Creation Technology Co.,Ltd
 * All rights reserved
 */

#include <sys/strings.h>
#include <sys/sysmacros.h>
#include <sys/debug.h>
#include <sys/byteorder.h>
#include <sys/sm3.h>

#define ROTATELEFT(X, n)  (((X)<<(n)) | ((X)>>(32-(n))))

#define P0(x) ((x) ^  ROTATELEFT((x), 9)  ^ ROTATELEFT((x), 17))
#define P1(x) ((x) ^  ROTATELEFT((x), 15) ^ ROTATELEFT((x), 23))

#define FF0(x,y,z) ( (x) ^ (y) ^ (z))
#define FF1(x,y,z) (((x) & (y)) | ( (x) & (z)) | ( (y) & (z)))

#define GG0(x,y,z) ( (x) ^ (y) ^ (z))
#define GG1(x,y,z) (((x) & (y)) | ( (~(x)) & (z)) )

void sm3_update_block(uint32_t *digest, const uint8_t *block);

void
sm3_update_block(uint32_t *digest, const uint8_t *block)
{
	int j;
	uint32_t W[68], W1[64];
	const uint32_t *pblock = (const uint32_t *)block;

	uint32_t A = digest[0];
	uint32_t B = digest[1];
	uint32_t C = digest[2];
	uint32_t D = digest[3];
	uint32_t E = digest[4];
	uint32_t F = digest[5];
	uint32_t G = digest[6];
	uint32_t H = digest[7];
	uint32_t SS1, SS2, TT1, TT2, T[64];

	for (j = 0; j < 16; j++) {
		W[j] = BE_32(pblock[j]);
	}
	for (j = 16; j < 68; j++) {
		W[j] = P1( W[j - 16] ^ W[j - 9] ^ ROTATELEFT(W[j - 3],15)) ^
		     ROTATELEFT(W[j - 13],7 ) ^ W[j - 6];;
	}
	for( j = 0; j < 64; j++) {
		W1[j] = W[j] ^ W[j + 4];
	}

	for(j = 0; j < 16; j++) {

		T[j] = 0x79CC4519;
		SS1 = ROTATELEFT((ROTATELEFT(A, 12) + E +
		     ROTATELEFT(T[j], j)), 7);
		SS2 = SS1 ^ ROTATELEFT(A,12);
		TT1 = FF0(A,B,C) + D + SS2 + W1[j];
		TT2 = GG0(E,F,G) + H + SS1 + W[j];
		D = C;
		C = ROTATELEFT(B,9);
		B = A;
		A = TT1;
		H = G;
		G = ROTATELEFT(F,19);
	}

	for(j = 16; j < 64; j++) {

		T[j] = 0x7A879D8A;
		SS1 = ROTATELEFT((ROTATELEFT(A, 12) + E +
		    ROTATELEFT(T[j], j % 32)), 7);
		SS2 = SS1 ^ ROTATELEFT(A, 12);
		TT1 = FF1(A,B,C) + D + SS2 + W1[j];
		TT2 = GG1(E,F,G) + H + SS1 + W[j];
		D = C;
		C = ROTATELEFT(B,9);
		B = A;
		A = TT1;
		H = G;
		G = ROTATELEFT(F,19);
		F = E;
		E = P0(TT2);
	}

	digest[0] ^= A;
	digest[1] ^= B;
	digest[2] ^= C;
	digest[3] ^= D;
	digest[4] ^= E;
	digest[5] ^= F;
	digest[6] ^= G;
	digest[7] ^= H;
}

void sm3_init(sm3_ctx_t *ctx);
void
sm3_init(sm3_ctx_t *ctx)
{
	ctx->digest[0] = 0x7380166F;
	ctx->digest[1] = 0x4914B2B9;
	ctx->digest[2] = 0x172442D7;
	ctx->digest[3] = 0xDA8A0600;
	ctx->digest[4] = 0xA96F30BC;
	ctx->digest[5] = 0x163138AA;
	ctx->digest[6] = 0xE38DEE4D;
	ctx->digest[7] = 0xB0FB0E4E;

	ctx->nblocks = 0;
	ctx->num = 0;
}

void
sm3_update(sm3_ctx_t *ctx, const uint8_t *data, size_t size)
{
	if (ctx->num) {
		unsigned int left = SM3_BLOCK_SIZE - ctx->num;
		if (size < left) {
			(void)memcpy(ctx->block + ctx->num, data, size);
			ctx->num += size;
			return;
		} else {
			(void)memcpy(ctx->block + ctx->num, data, left);
			sm3_update_block(ctx->digest, ctx->block);
			ctx->nblocks++;
			data += left;
			size -= left;
		}
	}
	while (size >= SM3_BLOCK_SIZE) {
		sm3_update_block(ctx->digest, data);
		ctx->nblocks++;
		data += SM3_BLOCK_SIZE;
		size -= SM3_BLOCK_SIZE;
	}
	ctx->num = size;
	if (size) {
		(void)memcpy(ctx->block, data, size);
	}
}

void
sm3_finish(sm3_ctx_t *ctx, uint8_t *digest)
{
	int i;
	uint32_t *pdigest = (uint32_t *)digest;
	uint32_t *count = (uint32_t *)(ctx->block + SM3_BLOCK_SIZE - 8);

	ctx->block[ctx->num] = 0x80;

	if (ctx->num + 9 <= SM3_BLOCK_SIZE) {
		(void)memset(ctx->block + ctx->num + 1, 0,
		    SM3_BLOCK_SIZE - ctx->num - 9);
	} else {
		(void)memset(ctx->block + ctx->num + 1, 0,
		    SM3_BLOCK_SIZE - ctx->num - 1);
		sm3_update_block(ctx->digest, ctx->block);
		(void)memset(ctx->block, 0, SM3_BLOCK_SIZE - 8);
	}

	count[0] = BE_32((ctx->nblocks) >> 23);
	count[1] = BE_32((ctx->nblocks << 9) + (ctx->num << 3));

	sm3_update_block(ctx->digest, ctx->block);
	for (i = 0; i < sizeof(ctx->digest)/sizeof(ctx->digest[0]); i++) {
		pdigest[i] = BE_32(ctx->digest[i]);
	}
}

#ifdef _KERNEL
EXPORT_SYMBOL(sm3_init);
EXPORT_SYMBOL(sm3_update);
EXPORT_SYMBOL(sm3_finish);
#endif
