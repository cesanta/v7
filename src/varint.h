/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

V7_PRIVATE int encode_varint(size_t len, unsigned char *p);
V7_PRIVATE size_t decode_varint(const unsigned char *p, int *llen);
V7_PRIVATE int calc_llen(size_t len);
