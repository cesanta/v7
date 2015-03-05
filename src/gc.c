/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"
#include "gc.h"

#ifdef V7_ENABLE_COMPACTING_GC
void gc_mark_string(struct v7 *, val_t *);
#endif

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
V7_PRIVATE void gc_arena_init(struct gc_arena *a, size_t cell_size,
                              size_t size, const char *name) {
  assert(cell_size >= sizeof(uintptr_t));
  memset(a, 0, sizeof(*a));
  a->cell_size = cell_size;
  a->name = name;
  /* Avoid arena initialization cost when GC is disabled */
#ifndef V7_DISABLE_GC
  gc_arena_grow(a, size);
  assert(a->free != NULL);
#else
  (void) size;
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
#ifdef V7_DISABLE_GC
  (void) v7;
  return malloc(a->cell_size);
#else
  char **r;
  if (a->free == NULL) {
#if 0
    fprintf(stderr, "Exhausting arena %s, invoking GC.\n", a->name);
#endif
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

  UNMARK(r);

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
    if (MARKED(cur)) {
      UNMARK(cur);
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
  if (MARKED(obj)) return;

  for ((prop = obj->properties), MARK(obj);
       prop != NULL; prop = next) {

#ifdef V7_ENABLE_COMPACTING_GC
    gc_mark_string(v7, &prop->value);
    gc_mark_string(v7, &prop->name);
#endif
    gc_mark(v7, prop->value);

    next = prop->next;

    assert((char *) prop >= v7->property_arena.base &&
           (char *) prop < (v7->property_arena.base + v7->property_arena.size *
                            v7->property_arena.cell_size));
    MARK(prop);
  }

  /* function scope pointer is aliased to the object's prototype pointer */
  gc_mark(v7, v7_object_to_value(obj->prototype));
}

static void gc_dump_arena_stats(const char *msg, struct gc_arena *a) {
  if (a->verbose) {
    fprintf(stderr, "%s: total allocations %lu, max %lu, alive %lu\n", msg,
            a->allocations, a->size, a->alive);
  }
}

#ifdef V7_ENABLE_COMPACTING_GC

uint64_t gc_string_val_to_offset(val_t v) {
  return ((uint64_t) v7_to_pointer(v)) & ~V7_TAG_MASK;
}

val_t gc_string_val_from_offset(uint64_t s) {
  return s | V7_TAG_STRING_O;
}

/* Mark a string value */
void gc_mark_string(struct v7 *v7, val_t *v) {
  val_t h, tmp = 0;
  char *s;

  if (((*v & V7_TAG_MASK) != V7_TAG_STRING_O) &&
      (*v & V7_TAG_MASK) != V7_TAG_STRING_C) {
    return;
  }

  /*
   * If a value points to an unmarked string we shall:
   *  1. save the first 6 bytes of the string
   *     since we need to be able to distinguish real values from
   *     the saved first 6 bytes of the string, we need to tag the chunk
   *     as V7_TAG_STRING_C
   *  2. encode value's address (v) into the first bytes of the string.
   *     the first byte is set to 0 to serve as a mark.
   *     The remaining 6 bytes are taken from v's least significant bytes.
   *  3. put the saved 8 bytes (tag + chunk) back into the value.
   *
   * If a value points to an already marked string we shall:
   *     (0, <6 bytes of a pointer to a val_t>), hence we have to skip
   *     the first byte. We tag the value pointer as a V7_TAG_FOREIGN
   *     so that it won't be followed during recursive mark.
   *
   *  ... the rest is the same
   *
   *  Note: 64-bit pointers can be represented with 48-bits
   */

  s = v7->owned_strings.buf + gc_string_val_to_offset(*v);
  if (s[-1] == '\0') {
    memcpy(&tmp, s, sizeof(tmp) - 2);
    tmp |= V7_TAG_STRING_C;
  } else {
    memcpy(&tmp, s, sizeof(tmp) - 2);
    tmp |= V7_TAG_FOREIGN;
  }

  h = (val_t) v;
  s[-1] = 1;
  memcpy(s, &h, sizeof(h) - 2);
  memcpy(v, &tmp, sizeof(tmp));
}

void gc_compact_strings(struct v7 *v7) {
  char *p = v7->owned_strings.buf + 1;
  uint64_t h, next, head = 1;
  int len, llen;

  while (p < v7->owned_strings.buf + v7->owned_strings.len) {
    if (p[-1] == '\1') {
      /* relocate and update ptrs */
      h = 0;
      memcpy(&h, p, sizeof(h) - 2);

      /*
       * relocate pointers until we find the tail.
       * The tail is marked with V7_TAG_STRING_C,
       * while val_t link pointers are tagged with V7_TAG_FOREIGN
       */
      for (; (h & V7_TAG_MASK) != V7_TAG_STRING_C ; h = next) {
        h &= ~V7_TAG_MASK;
        memcpy(&next, (char *) h, sizeof(h));

        * (val_t *) h = gc_string_val_from_offset(head);
      }
      h &= ~V7_TAG_MASK;

      /*
       * the tail contains the first 6 bytes we stole from
       * the actual string.
       */
      len = decode_varint((unsigned char *) &h, &llen);
      len += llen + 1;

      /*
       * restore the saved 6 bytes
       * TODO(mkm): think about endianness
       */
      memcpy(p, &h, sizeof(h) - 2);

      /*
       * and relocate the string data by packing it to the left.
       */
      memmove(v7->owned_strings.buf + head, p, len);
      v7->owned_strings.buf[head - 1] = 0x0;
      p += len;
      head += len;
    } else {
      len = decode_varint((unsigned char *) p, &llen);
      len += llen + 1;

      p += len;
    }
  }

  v7->owned_strings.len = head;
}

void gc_dump_owned_strings(struct v7 *v7) {
  size_t i;
#if 0
  for (i = 0; i < v7->owned_strings.len; i++) {
    printf("%02x ", (uint8_t) v7->owned_strings.buf[i]);
  }
  printf("\n");
  for (i = 0; i < v7->owned_strings.len; i++) {
    if (isprint(v7->owned_strings.buf[i])) {
      printf(" %c ", v7->owned_strings.buf[i]);
    } else {
      printf(" . ");
    }
  }
#else
    for (i = 0; i < v7->owned_strings.len; i++) {
    if (isprint(v7->owned_strings.buf[i])) {
      printf("%c", v7->owned_strings.buf[i]);
    } else {
      printf(".");
    }
  }
#endif
  printf("\n");
}

#endif

/* Perform garbage collection */
void v7_gc(struct v7 *v7) {
  val_t **vp;

  gc_dump_arena_stats("Before GC objects", &v7->object_arena);
  gc_dump_arena_stats("Before GC functions", &v7->function_arena);
  gc_dump_arena_stats("Before GC properties", &v7->property_arena);

#if 0
#ifdef V7_ENABLE_COMPACTING_GC
  printf("DUMP BEFORE\n");
  gc_dump_owned_strings(v7);
#endif
#endif

  /* TODO(mkm): paranoia? */
  gc_mark(v7, v7->object_prototype);
  gc_mark(v7, v7->array_prototype);
  gc_mark(v7, v7->boolean_prototype);
  gc_mark(v7, v7->error_prototype);
  gc_mark(v7, v7->string_prototype);
  gc_mark(v7, v7->number_prototype);
  gc_mark(v7, v7->function_prototype); /* possibly not reachable */
  gc_mark(v7, v7->this_object);

  gc_mark(v7, v7->object_prototype);
  gc_mark(v7, v7->global_object);
  gc_mark(v7, v7->this_object);
  gc_mark(v7, v7->call_stack);

  for (vp = (val_t **) v7->tmp_stack.buf;
       (char *) vp < v7->tmp_stack.buf + v7->tmp_stack.len; vp++) {
    gc_mark(v7, **vp);
  }

#ifdef V7_ENABLE_COMPACTING_GC
#if 0
  printf("Owned string mbuf len was %lu\n", v7->owned_strings.len);
#endif
  gc_compact_strings(v7);
#if 0
  printf("DUMP AFTER\n");
  gc_dump_owned_strings(v7);
  printf("Owned string mbuf len is %lu\n", v7->owned_strings.len);
#endif
#endif

  gc_sweep(&v7->object_arena, 0);
  gc_sweep(&v7->function_arena, 0);
  gc_sweep(&v7->property_arena, 0);

  gc_dump_arena_stats("After GC objects", &v7->object_arena);
  gc_dump_arena_stats("After GC functions", &v7->function_arena);
  gc_dump_arena_stats("After GC properties", &v7->property_arena);
}
