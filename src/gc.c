/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"
#include "gc.h"

#define MARK(p) (* (uintptr_t *) &(p) |= 1)

/* call only on already marked values */
#define UNMARK(p) (* (uintptr_t *) &(p) &= ~1)

#define MARKED(p) ((uintptr_t) (p) & 1)

V7_PRIVATE struct v7_object *new_object(struct v7 *v7) {
  return (struct v7_object *) gc_alloc_cell(v7, &v7->object_arena);
}

V7_PRIVATE struct v7_property *new_property(struct v7 *v7) {
  return (struct v7_property *) gc_alloc_cell(v7, &v7->property_arena);
}

V7_PRIVATE struct v7_function *new_function(struct v7 *v7) {
  return (struct v7_function *) gc_alloc_cell(v7, &v7->function_arena);
}

V7_PRIVATE struct gc_tmp_frame new_tmp_frame(struct v7 *v7) {
  struct gc_tmp_frame frame;
  frame.v7 = v7;
  frame.pos = v7->tmp_stack.len;
  return frame;
}

/*
 * TODO(mkm): make this work without GCC/CLANG extensions.
 * It's not hard to do it, but it requires to a big diff in the
 * interpreter which I'd like to postpone.
 */
V7_PRIVATE void tmp_frame_cleanup(struct gc_tmp_frame *tf) {
  tf->v7->tmp_stack.len = tf->pos;
}

/*
 * TODO(mkm): perhaps it's safer to keep val_t in the temporary
 * roots stack, instead of keeping val_t*, in order to be better
 * able to debug the relocating GC.
 */
V7_PRIVATE void tmp_stack_push(struct gc_tmp_frame *tf, val_t *vp) {
  mbuf_append(&tf->v7->tmp_stack, (char *) &vp, sizeof(val_t *));
}

/* Initializes a new arena. */
V7_PRIVATE void gc_arena_init(struct gc_arena *a, size_t cell_size, size_t size) {
  assert(cell_size >= sizeof(uintptr_t));
  memset(a, 0, sizeof(*a));
  a->cell_size = cell_size;
  a->size = size;
  /* Avoid arena initialization cost when GC is disabled */
#ifdef V7_ENABLE_GC
  gc_arena_grow(a, size);
#endif
}

V7_PRIVATE void gc_arena_destroy(struct gc_arena *a) {
  if (a->base != NULL) {
    free(a->base);
  }
}

/*
 * Grows the arena by reallocating.
 *
 * The caller is responsible of relocating all the pointers.
 *
 * TODO(mkm): An alternative is to use offsets instead of pointers or
 * instead of growing, maintain a chain of pools, which would also
 * have a smaller memory spike footprint, but it’s slightly more
 * complicated, and can be implemented in a second phase.
 */
V7_PRIVATE void gc_arena_grow(struct gc_arena *a, size_t new_size) {
  size_t free_adjust = a->free ? a->free - a->base : 0;
  size_t old_size = a->size;
  uint32_t old_alive = a->alive;
  a->size = new_size;
  a->base = (char *) realloc(a->base, a->size * a->cell_size);
  memset(a->base + old_size * a->cell_size, 0,
         (a->size - old_size) * a->cell_size);
  /* in case we grow preemptively */
  a->free += free_adjust;
  /* sweep will add the trailing zeroed memory to free list */
  gc_sweep(a, old_size);
  a->alive = old_alive; /* sweeping will decrement `alive` */
}

V7_PRIVATE void *gc_alloc_cell(struct v7 *v7, struct gc_arena *a) {
#ifndef V7_ENABLE_GC
  (void) v7;
  return malloc(a->cell_size);
#else
  char **r;
  if (a->free == NULL) {
    fprintf(stderr, "Exhausting arena %s, invoking GC.\n", a->name);
    v7_gc(v7);
    if (a->free == NULL) {
#if 1
      fprintf(stderr, "TODO arena grow\n");
      abort();
#else
      gc_arena_grow(a, a->size * 1.50);
      /* TODO(mkm): relocate */
#endif
    }
  }
  r = (char **) a->free;

  UNMARK(*r);

  a->free = * r;
  a->allocations++;
  a->alive++;

  return (void *) r;
#endif
}

/*
 * Scans the arena and add all unmarked cells to the free list.
 */
void gc_sweep(struct gc_arena *a, size_t start) {
  char *cur;
  a->alive = 0;
  a->free = NULL;
  for (cur = a->base + (start * a->cell_size);
       cur < (a->base + (a->size * a->cell_size));
       cur += a->cell_size) {
    uintptr_t it = (* (uintptr_t *) cur);
    if (it & 1) {
      UNMARK(*cur);
      a->alive++;
    } else {
      memset(cur, 0, a->cell_size);
      * (char **) cur = a->free;
      a->free = cur;
    }
  }
}

V7_PRIVATE void gc_mark(struct v7 *v7, val_t v) {
  struct v7_object *obj;
  struct v7_property *prop;
  struct v7_property *next;

  if (!v7_is_object(v)) {
    return;
  }
  obj = v7_to_object(v);
  if (MARKED(obj->properties)) return;

  for ((prop = obj->properties), MARK(obj->properties);
       prop != NULL; prop = next) {
    gc_mark(v7, prop->value);
    next = prop->next;

    assert((char *) prop >= v7->property_arena.base &&
           (char *) prop < (v7->property_arena.base + v7->property_arena.size *
                            v7->property_arena.cell_size));
    MARK(prop->next);
  }

  /* function scope pointer is aliased to the object's prototype pointer */
  gc_mark(v7, v7_object_to_value(obj->prototype));
}

static void gc_dump_arena_stats(const char *msg, struct gc_arena *a) {
  if (a->verbose) {
    fprintf(stderr, "%s: total allocations %llu, max %lu, alive %u\n", msg,
            a->allocations, a->size, a->alive);
  }
}

/* Perform garbage collection */
void v7_gc(struct v7 *v7) {
  val_t **vp;

  gc_dump_arena_stats("Before GC objects", &v7->object_arena);
  gc_dump_arena_stats("Before GC functions", &v7->function_arena);
  gc_dump_arena_stats("Before GC properties", &v7->property_arena);

  /* TODO(mkm): paranoia? */
  gc_mark(v7, v7->object_prototype);
  gc_mark(v7, v7->array_prototype);
  gc_mark(v7, v7->boolean_prototype);
  gc_mark(v7, v7->error_prototype);
  gc_mark(v7, v7->string_prototype);
  gc_mark(v7, v7->number_prototype);
  gc_mark(v7, v7->cfunction_prototype); /* possibly not reachable */
  gc_mark(v7, v7->this_object);

  gc_mark(v7, v7->object_prototype);
  gc_mark(v7, v7->global_object);
  gc_mark(v7, v7->this_object);
  gc_mark(v7, v7->call_stack);

  for (vp = (val_t **) v7->tmp_stack.buf;
       (char *) vp < v7->tmp_stack.buf + v7->tmp_stack.len; vp++) {
    gc_mark(v7, **vp);
  }

  gc_sweep(&v7->object_arena, 0);
  gc_sweep(&v7->function_arena, 0);
  gc_sweep(&v7->property_arena, 0);

  gc_dump_arena_stats("After GC objects", &v7->object_arena);
  gc_dump_arena_stats("After GC functions", &v7->function_arena);
  gc_dump_arena_stats("After GC properties", &v7->property_arena);
}
