/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"
#include "gc.h"

#define MARK(p) (* (uintptr_t *) &(p) |= 1)

/* call only on already marked values */
#define UNMARK(p) (* (uintptr_t *) &(p)) &= ~1)

#define MARKED(p) ((uintptr_t) (p) & 1)

V7_PRIVATE struct v7_object *new_object(struct v7 *v7) {
  (void) v7;
  return (struct v7_object *) malloc(sizeof(struct v7_object));
}

V7_PRIVATE struct v7_property *new_property(struct v7 *v7) {
  (void) v7;
  return (struct v7_property *) malloc(sizeof(struct v7_property));
}

V7_PRIVATE struct v7_function *new_function(struct v7 *v7) {
  (void) v7;
  return (struct v7_function *) malloc(sizeof(struct v7_function));
}

V7_PRIVATE void gc_mark(val_t v) {
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
    gc_mark(prop->value);
    next = prop->next;
    MARK(prop->next);
  }

  /* function scope pointer is aliased to the object's prototype pointer */
  gc_mark(v7_object_to_value(obj->prototype));
}

/* Perform garbage collection */
void v7_gc(struct v7 *v7) {
  gc_mark(v7->global_object);
}
