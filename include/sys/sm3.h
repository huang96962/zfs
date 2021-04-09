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
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 *
 */
/*
 * Copyright (c) 2021, Beijing Asia Creation Technology Co.,Ltd
 * All rights reserved
 */

#ifndef        _SYS_SM3_H_
#define        _SYS_SM3_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  _KERNEL
#include <sys/types.h>
#else
#include <stdint.h> /* uint32_t... */
#include <stdlib.h> /* size_t ... */
#endif

#define SM3_DIGEST_LENGTH	32
#define SM3_BLOCK_SIZE		64

typedef struct {
        uint32_t digest[8];
        int nblocks;
        uint8_t block[64];
        int num;
} sm3_ctx_t;

void sm3_init(sm3_ctx_t *ctx);
void sm3_update(sm3_ctx_t *ctx, const uint8_t *data, size_t size);
void sm3_finish(sm3_ctx_t *ctx, uint8_t *digest);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SM3_H_ */
