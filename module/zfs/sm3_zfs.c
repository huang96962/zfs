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
 */
/*
 * Copyright (c) 2021, Beijing Asia Creation Technology Co.,Ltd
 * All rights reserved.
 */
#include <sys/zfs_context.h>
#include <sys/zio.h>
#include <sys/sm3.h>
#include <sys/zfs_context.h>	/* For CTASSERT() */
#include <sys/abd.h>


static int
sm3_iteration(void *buf, size_t size, void *arg)
{
	sm3_ctx_t *ctx = arg;
	sm3_update(ctx, buf, size);
	return (0);
}

/*
 * Native zio_checksum interface for the sm3 hash function.
 */
/*ARGSUSED*/
void
abd_checksum_sm3_native(abd_t *abd, uint64_t size,
    const void *ctx_template, zio_cksum_t *zcp)
{
	sm3_ctx_t	ctx;

	sm3_init(&ctx);
	(void) abd_iterate_func(abd, 0, size, sm3_iteration, &ctx);
	sm3_finish(&ctx, (uint8_t *)zcp);
}

/*
 * Byteswapped zio_checksum interface for the sm3 hash function.
 */
void
abd_checksum_sm3_byteswap(abd_t *abd, uint64_t size,
    const void *ctx_template, zio_cksum_t *zcp)
{
	zio_cksum_t	tmp;

	abd_checksum_sm3_native(abd, size, ctx_template, &tmp);
	zcp->zc_word[0] = BSWAP_64(zcp->zc_word[0]);
	zcp->zc_word[1] = BSWAP_64(zcp->zc_word[1]);
	zcp->zc_word[2] = BSWAP_64(zcp->zc_word[2]);
	zcp->zc_word[3] = BSWAP_64(zcp->zc_word[3]);
}
