/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

struct a_sort_data {
  struct v7 *v7;
  val_t sort_func;
};

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
  long len = 0;
  (void) args;
  if (is_prototype_of(this_obj, v7->array_prototype)) {
    len = v7_array_length(v7, this_obj);
  }
  return v7_create_number(len);
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

static int a_cmp(void *user_data, const void *pa, const void *pb) {
  struct a_sort_data *sort_data = (struct a_sort_data *) user_data;
  struct v7 *v7 = sort_data->v7;
  val_t a = * (val_t *) pa, b = * (val_t *) pb, func = sort_data->sort_func;

  if (v7_is_function(func)) {
    val_t res, args = v7_create_array(v7);
    v7_array_append(v7, args, a);
    v7_array_append(v7, args, b);
    res = v7_apply(v7, func, V7_UNDEFINED, args);
    return (int) - v7_to_double(res);
  } else if (v7_is_double(a) && v7_is_double(b)) {
    return v7_to_double(b) - v7_to_double(a);
  } else {
    char sa[100], sb[100];
    to_str(v7, a, sa, sizeof(sa), 0);
    to_str(v7, b, sb, sizeof(sb), 0);
    sa[sizeof(sa) - 1] = sb[sizeof(sb) - 1] = '\0';
    return strcmp(sb, sa);
  }
}

static int a_partition(val_t *a, int l, int r, void *user_data) {
  val_t t, pivot = a[l];
  int i = l, j = r + 1;

  for (;;) {
    do ++i; while (i <= r && a_cmp(user_data, &a[i], &pivot) <= 0);
    do --j; while (a_cmp(user_data, &a[j], &pivot) > 0);
    if (i >= j) break;
    t = a[i]; a[i] = a[j]; a[j] = t;
  }
  t = a[l]; a[l] = a[j]; a[j] = t;
  return j;
}

static void a_qsort(val_t *a, int l, int r, void *user_data) {
  if (l < r) {
    int j = a_partition(a, l, r, user_data);
    a_qsort(a, l, j - 1, user_data);
    a_qsort(a, j + 1, r, user_data);
  }
}

static val_t a_sort(struct v7 *v7, val_t obj, val_t args,
                    int (*sorting_func)(void *, const void *, const void *)) {
  int i = 0, len = v7_array_length(v7, obj);
  val_t *arr = (val_t *) malloc(len * sizeof(arr[0]));
  val_t arg0 = v7_array_at(v7, args, 0);
  struct v7_property *p;

  if (!v7_is_object(obj)) return obj;
  assert(obj != v7->global_object);

  for (i = 0; i < len; i++) {
    arr[i] = v7_array_at(v7, obj, i);
  }

  if (sorting_func != NULL) {
    struct a_sort_data sort_data;
    sort_data.v7 = v7;
    sort_data.sort_func = arg0;
    a_qsort(arr, 0, len - 1, &sort_data);
  }

  for (i = 0; i < len; i++) {
    char buf[40];
    snprintf(buf, sizeof(buf), "%d", i);
    if ((p = v7_get_own_property(obj, buf, strlen(buf))) != NULL) {
      p->value = arr[len - (i + 1)];
    }
  }

  free(arr);

  return obj;
}

static val_t Array_sort(struct v7 *v7, val_t this_obj, val_t args) {
  return a_sort(v7, this_obj, args, a_cmp);
}

static val_t Array_reverse(struct v7 *v7, val_t this_obj, val_t args) {
  return a_sort(v7, this_obj, args, NULL);
}

static val_t Array_pop(struct v7 *v7, val_t this_obj, val_t args) {
  struct v7_property *p;
  val_t res = V7_UNDEFINED;

  (void) v7; (void) args;

  if (is_prototype_of(this_obj, v7->array_prototype) &&
      (p = v7_to_object(this_obj)->properties) != NULL) {
    res = p->value;
    v7_to_object(this_obj)->properties = p->next;
  }

  return res;
}

static val_t Array_join(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  val_t res = V7_UNDEFINED;
  size_t sep_size = 0;
  const char *sep = NULL;

  /* Get pointer to the separator string */
  if (!v7_is_string(arg0)) {
    /* If no separator is provided, use comma */
    arg0 = v7_create_string(v7, ",", 1, 1);
  }
  sep = v7_to_string(v7, &arg0, &sep_size);

  /* Do the actual join */
  if (is_prototype_of(this_obj, v7->array_prototype)) {
    struct mbuf m;
    char buf[100], *p;
    long i, n, num_elems = v7_array_length(v7, this_obj);

    mbuf_init(&m, 0);

    for (i = 0; i < num_elems; i++) {
      /* Append separator */
      if (i > 0) {
        mbuf_append(&m, sep, sep_size);
      }

      /* Append next item from an array */
      p = buf;
      n = to_str(v7, v7_array_at(v7, this_obj, i), buf, sizeof(buf), 0);
      if (n > (long) sizeof(buf)) {
        p = (char *) malloc(n + 1);
        to_str(v7, v7_array_at(v7, this_obj, i), p, n, 0);
      }
      mbuf_append(&m, p, n);
      if (p != buf) {
        free(p);
      }
    }

    /* mbuf contains concatenated string now. Copy it to the result. */
    res = v7_create_string(v7, m.buf, m.len, 1);
    mbuf_free(&m);
  }

  return res;
}

static val_t Array_toString(struct v7 *v7, val_t this_obj, val_t args) {
  return Array_join(v7, this_obj, args);
}

V7_PRIVATE void init_array(struct v7 *v7) {
  val_t ctor = v7_create_cfunction_object(v7, Array_ctor, 1);
  val_t length = v7_create_array(v7);

  v7_set_property(v7, ctor, "prototype", 9, 0, v7->array_prototype);
  v7_set_property(v7, v7->global_object, "Array", 5, 0, ctor);

  set_cfunc_obj_prop(v7, v7->array_prototype, "push", Array_push, 1);
  set_cfunc_obj_prop(v7, v7->array_prototype, "sort", Array_sort, 1);
  set_cfunc_obj_prop(v7, v7->array_prototype, "reverse", Array_reverse, 0);
  set_cfunc_obj_prop(v7, v7->array_prototype, "pop", Array_pop, 0);
  set_cfunc_obj_prop(v7, v7->array_prototype, "join", Array_join, 1);
  set_cfunc_obj_prop(v7, v7->array_prototype, "toString", Array_toString, 0);

  v7_set(v7, length, "0", 1, v7_create_cfunction(Array_get_length));
  v7_set(v7, length, "1", 1, v7_create_cfunction(Array_set_length));
  v7_set_property(v7, v7->array_prototype, "length", 6,
                  V7_PROPERTY_GETTER | V7_PROPERTY_SETTER, length);
}
