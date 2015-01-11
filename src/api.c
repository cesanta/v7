/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

struct v7 *v7_create(void) {
  static int prototypes_initialized = 0;
  struct v7 *v7 = NULL;

  if (prototypes_initialized == 0) {
    prototypes_initialized++;
    init_stdlib();  /* One-time initialization */
  }

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) != NULL) {
    v7_set_class(&v7->root_scope, V7_CLASS_OBJECT);
    v7->root_scope.proto = &s_global;
    v7->root_scope.ref_count = 1;
    v7->ctx = &v7->root_scope;
    /*
     * Ensure the first call to v7_create_value will use a null proto:
     * {}.__proto__.__proto__ == null
     */
    v7->object_prototype = NULL;
    v7->object_prototype = val_to_object(
        v7_create_value(v7, V7_TYPE_GENERIC_OBJECT));
    v7->array_prototype = val_to_object(
        v7_create_value(v7, V7_TYPE_GENERIC_OBJECT));
    v7->global_object = v7_create_value(v7, V7_TYPE_GENERIC_OBJECT);
  }

  return v7;
}

struct v7_val *v7_global(struct v7 *v7) {
  return &v7->root_scope;
}

void v7_destroy(struct v7 **v7) {
  if (v7 == NULL || v7[0] == NULL) return;
  assert(v7[0]->sp >= 0);
  inc_stack(v7[0], -v7[0]->sp);
  v7[0]->root_scope.ref_count = 1;
  v7_freeval(v7[0], &v7[0]->root_scope);
  free_values(v7[0]);
  free_props(v7[0]);
  mbuf_free(&v7[0]->owned_strings);
  mbuf_free(&v7[0]->foreign_strings);
  free(v7[0]);
  v7[0] = NULL;
}

struct v7_val *v7_push_number(struct v7 *v7, double num) {
  return push_number(v7, num) == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_push_bool(struct v7 *v7, int is_true) {
  return push_bool(v7, is_true) == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_push_string(struct v7 *v7, const char *str, unsigned long n,
                              int own) {
  return push_string(v7, str, n, own) == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_push_func(struct v7 *v7, v7_func_t func) {
  return push_func(v7, func) == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_push_new_object(struct v7 *v7) {
  return push_new_object(v7) == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_push_val(struct v7 *v7, struct v7_val *v) {
  return v7_push(v7, v) == V7_OK ? v : NULL;
}

enum v7_type v7_type(const struct v7_val *v) { return v->type; }

double v7_number(const struct v7_val *v) { return v->v.num; }

const char *v7_string(const struct v7_val *v, unsigned long *plen) {
  if (plen != NULL) *plen = v->v.str.len;
  return v->v.str.buf;
}

struct v7_val *v7_get(struct v7_val *obj, const char *key) {
  struct v7_val k = v7_str_to_val(key);
  return get2(obj, &k);
}

int v7_is_true(const struct v7_val *v) {
  return (v->type == V7_TYPE_BOOL && v->v.num != 0.0) ||
         (v->type == V7_TYPE_NUM && v->v.num != 0.0 && !isnan(v->v.num)) ||
         (v->type == V7_TYPE_STR && v->v.str.len > 0) ||
         (v->type == V7_TYPE_OBJ);
}

enum v7_err v7_append(struct v7 *v7, struct v7_val *arr, struct v7_val *val) {
  struct v7_prop **head, *prop;
  CHECK(v7_is_class(arr, V7_CLASS_ARRAY), V7_INTERNAL_ERROR);
  /* Append to the end of the list, to make indexing work */
  for (head = &arr->v.array; *head != NULL; head = &head[0]->next) {
  }
  prop = mkprop(v7);
  CHECK(prop != NULL, V7_OUT_OF_MEMORY);
  prop->next = *head;
  *head = prop;
  prop->key = NULL;
  prop->val = val;
  INC_REF_COUNT(val);
  return V7_OK;
}

void v7_copy(struct v7 *v7, struct v7_val *orig, struct v7_val *v) {
  struct v7_prop *p;

  switch (v->type) {
    case V7_TYPE_OBJ:
      for (p = orig->props; p != NULL; p = p->next) {
        v7_set2(v7, v, p->key, p->val);
      }
      break;
      /* TODO(lsm): add the rest of types */
    default:
      abort();
      break;
  }
}

const char *v7_get_error_string(const struct v7 *v7) {
  return v7->error_message;
}

struct v7_val *v7_call(struct v7 *v7, struct v7_val *this_obj, int num_args) {
  v7_call2(v7, this_obj, num_args, 0);
  return v7_top_val(v7);
}

enum v7_err v7_set(struct v7 *v7, struct v7_val *obj, const char *key,
                   struct v7_val *val) {
  return v7_setv(v7, obj, V7_TYPE_STR, V7_TYPE_OBJ, key, strlen(key), 1, val);
}

enum v7_err v7_del(struct v7 *v7, struct v7_val *obj, const char *key) {
  return v7_del2(v7, obj, key, strlen(key));
}

static void arr_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m, *head = v->v.array;
  int n = snprintf(buf, bsiz, "%s", "[");

  for (m = head; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != head) n += snprintf(buf + n, bsiz - n, "%s", ", ");
    v7_stringify(m->val, buf + n, bsiz - n);
    n = (int)strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "]");
}

static void obj_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m, *head = v->props;
  int n = snprintf(buf, bsiz, "%s", "{");

  for (m = head; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != head) n += snprintf(buf + n, bsiz - n, "%s", ", ");
    v7_stringify(m->key, buf + n, bsiz - n);
    n = (int)strlen(buf);
    n += snprintf(buf + n, bsiz - n, "%s", ": ");
    v7_stringify(m->val, buf + n, bsiz - n);
    n = (int)strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "}");
}

char *v7_stringify(const struct v7_val *v, char *buf, int bsiz) {
  if (v->type == V7_TYPE_UNDEF) {
    snprintf(buf, bsiz, "%s", "undefined");
  } else if (v->type == V7_TYPE_NULL) {
    snprintf(buf, bsiz, "%s", "null");
  } else if (is_bool(v)) {
    snprintf(buf, bsiz, "%s", v->v.num ? "true" : "false");
  } else if (is_num(v)) {
    /* TODO: check this on 32-bit arch */
    if (INFINITY == v->v.num)
      snprintf(buf, bsiz, "Infinity");
    else if (-INFINITY == v->v.num)
      snprintf(buf, bsiz, "-Infinity");
    else if (isnan(v->v.num))
      snprintf(buf, bsiz, "NaN");
    else if (v->v.num > ((uint64_t)1 << 52) || ceil(v->v.num) != v->v.num)
      snprintf(buf, bsiz, "%lg", v->v.num);
    else
      snprintf(buf, bsiz, "%ld", (unsigned long)v->v.num);
  } else if (is_string(v)) {
    snprintf(buf, bsiz, "%.*s", (int)v->v.str.len, v->v.str.buf);
  } else if (v7_is_class(v, V7_CLASS_ARRAY)) {
    arr_to_string(v, buf, bsiz);
  } else if (v7_is_class(v, V7_CLASS_FUNCTION)) {
    if (v->fl.fl.js_func) {
      snprintf(buf, bsiz, "'function%s'", v->v.func.source_code);
    } else {
      snprintf(buf, bsiz, "'c_func_%p'", v->v.c_func);
    }
  } else if (v7_is_class(v, V7_CLASS_REGEXP)) {
    int sz = snprintf(buf, bsiz, "/%s/", v->v.str.buf);
    if (v->fl.fl.re_flags & SLRE_FLAG_G) sz += snprintf(buf + sz, bsiz, "g");
    if (v->fl.fl.re_flags & SLRE_FLAG_I) sz += snprintf(buf + sz, bsiz, "i");
    if (v->fl.fl.re_flags & SLRE_FLAG_M) snprintf(buf + sz, bsiz, "m");
  } else if (v->type == V7_TYPE_OBJ) {
    obj_to_string(v, buf, bsiz);
  } else {
    snprintf(buf, bsiz, "??");
  }

  buf[bsiz - 1] = '\0';
  return buf;
}

struct v7_val *v7_exec(struct v7 *v7, const char *source_code) {
  enum v7_err er = do_exec(v7, "<exec>", source_code, 0);
  return v7->sp > 0 && er == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_exec_file(struct v7 *v7, const char *path) {
  FILE *fp;
  char *p;
  long file_size;
  int old_sp = v7->sp;
  enum v7_err status = V7_INTERNAL_ERROR;

  if ((fp = fopen(path, "r")) == NULL) {
  } else if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0) {
    fclose(fp);
  } else if ((p = (char *) calloc(1, (size_t) file_size + 1)) == NULL) {
    fclose(fp);
  } else {
    rewind(fp);
    fread(p, 1, (size_t) file_size, fp);
    fclose(fp);
    status = do_exec(v7, path, p, v7->sp);
    free(p);
  }

  return v7->sp > old_sp && status == V7_OK ? v7_top_val(v7) : NULL;
  /* return status; */
}
