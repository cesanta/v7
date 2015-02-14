/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_VARINT_H_INCLUDED
#define V7_VARINT_H_INCLUDED

#include "internal.h"

#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

V7_PRIVATE int encode_varint(size_t len, unsigned char *p);
V7_PRIVATE size_t decode_varint(const unsigned char *p, int *llen);
V7_PRIVATE int calc_llen(size_t len);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif  /* V7_VARINT_H_INCLUDED */
