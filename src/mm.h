/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MM_H_INCLUDED
#define MM_H_INCLUDED

#include "internal.h"

struct gc_arena {
  char *base;
  size_t size;
  char *free; /* head of free list */
  size_t cell_size;

  unsigned long allocations; /* cumulative counter of allocations */
  unsigned long alive;       /* number of living cells */

  int verbose;
  const char *name; /* for debugging purposes */
};

#endif /* GC_H_INCLUDED */
