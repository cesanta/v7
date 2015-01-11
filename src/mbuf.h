/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MBUF_H_INCLUDED
#define MBUF_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

#include <stdlib.h>

#ifndef MBUF_SIZE_MULTIPLIER
#define MBUF_SIZE_MULTIPLIER 1.5
#endif

struct mbuf {
  char *buf;
  size_t len;
  size_t size;
};

#ifdef V7_EXPOSE_PRIVATE
#define V7_PRIVATE
#define V7_EXTERN extern
#else
#define V7_PRIVATE static
#define V7_EXTERN static
#endif

V7_PRIVATE void mbuf_init(struct mbuf *, size_t);
V7_PRIVATE void mbuf_free(struct mbuf *);
V7_PRIVATE void mbuf_resize(struct mbuf *, size_t);
V7_PRIVATE void mbuf_trim(struct mbuf *);
V7_PRIVATE size_t mbuf_insert(struct mbuf *, size_t, const char *, size_t);
V7_PRIVATE size_t mbuf_append(struct mbuf *, const char *, size_t);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif  /* MBUF_H_INCLUDED */
