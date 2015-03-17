/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

/* Initializes mbuf. */
V7_PRIVATE void mbuf_init(struct mbuf *mbuf, size_t initial_size) {
  mbuf->len = mbuf->size = 0;
  mbuf->buf = NULL;
  mbuf_resize(mbuf, initial_size);
}

/* Frees the space allocated for the iobuffer and resets the iobuf structure. */
V7_PRIVATE void mbuf_free(struct mbuf *mbuf) {
  if (mbuf->buf != NULL) {
    free(mbuf->buf);
    mbuf_init(mbuf, 0);
  }
}

/*
 * Resize mbuf.
 *
 * If `new_size` is smaller than buffer's `len`, the
 * resize is not performed.
 */
V7_PRIVATE void mbuf_resize(struct mbuf *a, size_t new_size) {
  char *p;
  if ((new_size > a->size || (new_size < a->size && new_size >= a->len)) &&
      (p = (char *) realloc(a->buf, new_size)) != NULL) {
    a->size = new_size;
    a->buf = p;
  }
}

/* Shrinks mbuf size to just fit it's length. */
V7_PRIVATE void mbuf_trim(struct mbuf *mbuf) {
  mbuf_resize(mbuf, mbuf->len);
}

/*
 * Appends data to the mbuf.
 *
 * It returns the amount of bytes appended.
 */
V7_PRIVATE size_t mbuf_append(struct mbuf *a, const char *buf, size_t len) {
  return mbuf_insert(a, a->len, buf, len);
}

/*
 * Inserts data at a specified offset in the mbuf.
 *
 * Existing data will be shifted forwards and the buffer will
 * be grown if necessary.
 * It returns the amount of bytes inserted.
 */
V7_PRIVATE size_t
mbuf_insert(struct mbuf *a, size_t off, const char *buf, size_t len) {
  char *p = NULL;

  assert(a != NULL);
  assert(a->len <= a->size);
  assert(off <= a->len);

  /* check overflow */
  if (~(size_t) 0 - (size_t) a->buf < len) return 0;

  if (a->len + len <= a->size) {
    memmove(a->buf + off + len, a->buf + off, a->len - off);
    if (buf != NULL) {
      memcpy(a->buf + off, buf, len);
    }
    a->len += len;
  } else if ((p = (char *) realloc(
                  a->buf, (a->len + len) * MBUF_SIZE_MULTIPLIER)) != NULL) {
    a->buf = p;
    memmove(a->buf + off + len, a->buf + off, a->len - off);
    if (buf != NULL) {
      memcpy(a->buf + off, buf, len);
    }
    a->len += len;
    a->size = a->len * MBUF_SIZE_MULTIPLIER;
  } else {
    len = 0;
  }

  return len;
}
