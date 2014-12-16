/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_APARSER_H_INCLUDED
#define V7_APARSER_H_INCLUDED

#include "internal.h"

#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

V7_PRIVATE enum v7_err aparse(struct ast *, const char*, int);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif  /* V7_APARSER_H_INCLUDED */
