/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE val_t Array_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  (void) v7;
  (void) this_obj;
  return args;
}

V7_PRIVATE val_t Array_push(struct v7 *v7, val_t this_obj, val_t args) {
  val_t v = V7_UNDEFINED;
  int i, len = v7_array_length(v7, args);
  for (i = 0; i < len; i++) {
    v = v7_array_at(v7, args, i);
    v7_array_append(v7, this_obj, v);
  }
  return v;
}

static val_t Arr_length(struct v7 *v7, val_t this_obj, val_t args) {
  (void) args;
  assert(val_type(v7, this_obj) == V7_TYPE_ARRAY_OBJECT);
  return v7_create_number(v7_array_length(v7, this_obj));
}

#if 0
V7_PRIVATE int cmp_prop(const void *pa, const void *pb) {
  const struct v7_prop *p1 = *(struct v7_prop **) pa;
  const struct v7_prop *p2 = *(struct v7_prop **) pb;
  return cmp(p2->val, p1->val);
}

V7_PRIVATE enum v7_err Arr_sort(struct v7_c_func_arg *cfa) {
  int i = 0, length = 0;
  struct v7_val *v = cfa->this_obj;
  struct v7_prop *p, **arr;

  /* TODO(lsm): do proper error checking */
  for (p = v->v.array; p != NULL; p = p->next) {
    length++;
  }
  arr = (struct v7_prop **) malloc(length * sizeof(p));
  for (i = 0, p = v->v.array; p != NULL; p = p->next) {
    arr[i++] = p;
  }
  qsort(arr, length, sizeof(p), cmp_prop);
  v->v.array = NULL;
  for (i = 0; i < length; i++) {
    arr[i]->next = v->v.array;
    v->v.array = arr[i];
  }
  free(arr);
  return V7_OK;
}
#endif

V7_PRIVATE void init_array(struct v7 *v7) {
  v7_set_property(v7, v7->global_object, "Array", 5, 0,
                  v7_create_cfunction(Array_ctor));
  v7_set_property(v7, v7->array_prototype, "push", 4, 0,
                  v7_create_cfunction(Array_push));
  v7_set_property(v7, v7->array_prototype, "length", 6, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Arr_length));
}
