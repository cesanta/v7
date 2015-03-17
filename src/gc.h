/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef GC_H_INCLUDED
#define GC_H_INCLUDED

#include "internal.h"
#include "vm.h"

/* Disable GC on 32-bit platform for now */
#if ULONG_MAX == 4294967295
#define V7_DISABLE_GC
#endif

#define MARK(p) (((struct gc_cell *) (p))->word |= 1)
#define UNMARK(p) (((struct gc_cell *) (p))->word &= ~1)
#define MARKED(p) (((struct gc_cell *) (p))->word & 1)

struct gc_tmp_frame {
  struct v7 *v7;
  size_t pos;
};

struct gc_cell {
  uintptr_t word;
};

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

V7_PRIVATE struct v7_object *new_object(struct v7 *);
V7_PRIVATE struct v7_property *new_property(struct v7 *);
V7_PRIVATE struct v7_function *new_function(struct v7 *);

V7_PRIVATE void gc_mark(struct v7 *, val_t);

V7_PRIVATE void gc_arena_init(struct gc_arena *, size_t, size_t, const char *);
V7_PRIVATE void gc_arena_grow(struct gc_arena *, size_t);
V7_PRIVATE void gc_arena_destroy(struct gc_arena *a);
V7_PRIVATE void gc_sweep(struct gc_arena *, size_t);
V7_PRIVATE void *gc_alloc_cell(struct v7 *, struct gc_arena *);

V7_PRIVATE struct gc_tmp_frame new_tmp_frame(struct v7 *);
V7_PRIVATE void tmp_frame_cleanup(struct gc_tmp_frame *);
V7_PRIVATE void tmp_stack_push(struct gc_tmp_frame *, val_t *);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* GC_H_INCLUDED */
