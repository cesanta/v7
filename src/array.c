/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t Array_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  (void) v7;
  (void) this_obj;
  return args;
}

static val_t Array_push(struct v7 *v7, val_t this_obj, val_t args) {
  val_t v = V7_UNDEFINED;
  int i, len = v7_array_length(v7, args);
  for (i = 0; i < len; i++) {
    v = v7_array_at(v7, args, i);
    v7_array_append(v7, this_obj, v);
  }
  return v;
}

static val_t Array_get_length(struct v7 *v7, val_t this_obj, val_t args) {
  (void) args;
  assert(is_prototype_of(this_obj, v7->array_prototype));
  return v7_create_number(v7_array_length(v7, this_obj));
}

static val_t Array_set_length(struct v7 *v7, val_t this_obj, val_t args) {
  long new_len = arg_long(v7, args, 0, -1);

  if (!v7_is_object(this_obj)) {
    throw_exception(v7, "TypeError", "Array expected");
  } else if (new_len < 0) {
    throw_exception(v7, "RangeError", "Invalid array length");
  } else {
    struct v7_property **p, **next;
    long index, max_index = -1;

    /* Remove all items with an index higher then new_len */
    for (p = &v7_to_object(this_obj)->properties; *p != NULL; p = next) {
      next = &p[0]->next;
      index = strtol(p[0]->name, NULL, 10);
      if (index >= new_len) {
        v7_destroy_property(p);
        *p = *next;
        next = p;
      } else if (index > max_index) {
        max_index = index;
      }
    }

    /* If we have to expand, insert an item with appropriate index */
    if (new_len > 0 && max_index < new_len - 1) {
      char buf[40];
      snprintf(buf, sizeof(buf), "%ld", new_len - 1);
      v7_set_property(v7, this_obj, buf, strlen(buf), 0, V7_UNDEFINED);
    }
  }
  return v7_create_number(new_len);
}

static int a_cmp(const void *pa, const void *pb) {
  val_t a = * (val_t *) pa, b = * (val_t *) pb;
  /* TODO(lsm): support comparison for all types, not just numbers */
  return v7_to_double(b) - v7_to_double(a);
}

static val_t a_sort(struct v7 *v7, val_t this_obj, val_t args,
                    int (*sorting_func)(const void *, const void *)) {
  int i = 0, len = v7_array_length(v7, this_obj);
  val_t *arr = (val_t *) malloc(len * sizeof(arr[0]));
  val_t arg0 = v7_array_at(v7, args, 0);
  struct v7_property *p;

  assert(v7_is_object(this_obj));
  assert(this_obj != v7->global_object);

  /* TODO(lsm): respect first argument, a sorting function */
  (void) arg0;

  for (i = 0; i < len; i++) {
    arr[i] = v7_array_at(v7, this_obj, i);
  }
  if (sorting_func != NULL) {
    qsort(arr, len, sizeof(arr[0]), sorting_func);
  }

  i = 0;
  for (p = v7_to_object(this_obj)->properties; p != NULL; p = p->next) {
    p->value = arr[i++];
  }

  free(arr);

  return this_obj;
}

static val_t Array_sort(struct v7 *v7, val_t this_obj, val_t args) {
  return a_sort(v7, this_obj, args, a_cmp);
}

static val_t Array_reverse(struct v7 *v7, val_t this_obj, val_t args) {
  return a_sort(v7, this_obj, args, NULL);
}

static val_t Array_pop(struct v7 *v7, val_t this_obj, val_t args) {
  struct v7_property *p = v7_to_object(this_obj)->properties;
  val_t res = V7_UNDEFINED;

  (void) v7; (void) args;
  if (p != NULL) {
    res = p->value;
    v7_to_object(this_obj)->properties = p->next;
  }

  return res;
}

V7_PRIVATE void init_array(struct v7 *v7) {
  val_t ctor = v7_create_cfunction_object(v7, Array_ctor);
  val_t length = v7_create_array(v7);

  v7_set_property(v7, v7->global_object, "Array", 5, 0, ctor);
  v7_to_object(ctor)->prototype = v7_to_object(v7->array_prototype);

  set_cfunc_obj_prop(v7, v7->array_prototype, "push", Array_push);
  set_cfunc_obj_prop(v7, v7->array_prototype, "sort", Array_sort);
  set_cfunc_obj_prop(v7, v7->array_prototype, "reverse", Array_reverse);
  set_cfunc_obj_prop(v7, v7->array_prototype, "pop", Array_pop);

  v7_set(v7, length, "0", 1, v7_create_cfunction(Array_get_length));
  v7_set(v7, length, "1", 1, v7_create_cfunction(Array_set_length));
  v7_set_property(v7, v7->array_prototype, "length", 6,
                  V7_PROPERTY_GETTER | V7_PROPERTY_SETTER, length);
}
