/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

enum v7_type val_type(struct v7 *v7, val_t v) {
  if (v7_is_double(v)) {
    return V7_TYPE_NUMBER;
  }
  switch (v & V7_TAG_MASK) {
    case V7_TAG_FOREIGN:
      if (v == V7_NULL) {
        return V7_TYPE_NULL;
      }
      return V7_TYPE_FOREIGN;
    case V7_TAG_UNDEFINED:
      return V7_TYPE_UNDEFINED;
    case V7_TAG_OBJECT:
      if (val_to_object(v)->prototype == v7->array_prototype) {
        return V7_TYPE_ARRAY_OBJECT;
      } else {
        return V7_TYPE_GENERIC_OBJECT;
      }
    case V7_TAG_STRING_I:
    case V7_TAG_STRING_O:
    case V7_TAG_STRING_F:
      return V7_TYPE_STRING;
    case V7_TAG_BOOLEAN:
      return V7_TYPE_BOOLEAN;
    case V7_TAG_FUNCTION:
      return V7_TYPE_FUNCTION_OBJECT;
    default:
      /* TODO(mkm): or should we crash? */
      return V7_TYPE_UNDEFINED;
  }
}

int v7_is_double(val_t v) {
  return v == V7_TAG_NAN || !isnan(val_to_double(v));
}

int v7_is_object(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_OBJECT ||
      (v & V7_TAG_MASK) == V7_TAG_FUNCTION;
}

int v7_is_function(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_FUNCTION;
}

int v7_is_string(val_t v) {
  uint64_t t = v & V7_TAG_MASK;
  return t == V7_TAG_STRING_I || t == V7_TAG_STRING_F || t == V7_TAG_STRING_O;
}

int v7_is_boolean(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_BOOLEAN;
}

int v7_is_null(val_t v) {
  return v == V7_NULL;
}

int v7_is_undefined(val_t v) {
  return v == V7_UNDEFINED;
}

int v7_is_cfunction(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_CFUNCTION;
}

/* A convenience function to check exec result. TODO(lsm): implement it. */
int v7_is_error(val_t v) {
  return v7_is_object(v) && 0;
}

V7_PRIVATE val_t v7_pointer_to_value(void *p) {
  return ((uint64_t) p & ((1L << 48) -1));
}

V7_PRIVATE void *val_to_pointer(val_t v) {
  struct {
    uint64_t s:48;
  } h;
  return (void *) (uintptr_t) (h.s = v);
}

val_t v7_object_to_value(struct v7_object *o) {
  if (o == NULL) {
    return V7_NULL;
  }
  return v7_pointer_to_value(o) | V7_TAG_OBJECT;
}

struct v7_object *val_to_object(val_t v) {
  return (struct v7_object *) val_to_pointer(v);
}

val_t v7_function_to_value(struct v7_function *o) {
  return v7_pointer_to_value(o) | V7_TAG_FUNCTION;
}

struct v7_function *val_to_function(val_t v) {
  return (struct v7_function *) val_to_pointer(v);
}

v7_cfunction_t val_to_cfunction(val_t v) {
  return (v7_cfunction_t) val_to_pointer(v);
}

val_t v7_cfunction_to_value(v7_cfunction_t f) {
  union { void *p; v7_cfunction_t f; } u;
  u.f = f;
  return v7_pointer_to_value(u.p) | V7_TAG_CFUNCTION;
}

val_t v7_foreign_to_value(void *p) {
  return v7_pointer_to_value(p) | V7_TAG_FOREIGN;
}

void *val_to_foreign(val_t v) {
  return val_to_pointer(v);
}

val_t v7_boolean_to_value(int v) {
  return (!!v) | V7_TAG_BOOLEAN;
}

int val_to_boolean(val_t v) {
  return v & 1;
}

val_t v7_double_to_value(double v) {
  val_t res;
  /* not every NaN is a JS NaN */
  if (isnan(v)) {
    res = V7_TAG_NAN;
  } else {
    * (double *) &res = v;
  }
  return res;
}

double val_to_double(val_t v) {
  return * (double *) &v;
}

V7_PRIVATE val_t v_get_prototype(val_t obj) {
  return v7_object_to_value(val_to_object(obj)->prototype);
}

static val_t v7_create_object(struct v7 *v7, struct v7_object *proto) {
  /* TODO(mkm): use GC heap */
  struct v7_object *o = (struct v7_object *) malloc(sizeof(struct v7_object));
  if (o == NULL) {
    return V7_NULL;
  }
  (void) v7;
  o->properties = NULL;
  o->prototype = proto;
  return v7_object_to_value(o);
}

val_t v7_create_value(struct v7 *v7, enum v7_type type, ...) {
  va_list ap;
  va_start(ap, type);
  return v7_va_create_value(v7, type, ap);
}

val_t v7_va_create_value(struct v7 *v7, enum v7_type type,
                            va_list ap) {
  val_t v;

  /* Initialize value based on type */
  switch (type) {
    case V7_TYPE_NULL:
      return V7_NULL;
    case V7_TYPE_UNDEFINED:
      return V7_UNDEFINED;
    case V7_TYPE_NUMBER:
      return v7_double_to_value(va_arg(ap, double));
    case V7_TYPE_BOOLEAN:
      return v7_boolean_to_value(va_arg(ap, int));
    case V7_TYPE_STRING:
      {
        char *p = va_arg(ap, char *);
        size_t len = va_arg(ap, size_t);
        int own = va_arg(ap, int);
        return v7_string_to_value(v7, p, len, own);
      }
    case V7_TYPE_GENERIC_OBJECT:
      return v7_create_object(v7, v7->object_prototype);
    case V7_TYPE_ARRAY_OBJECT:
      return v7_create_object(v7, v7->array_prototype);
    case V7_TYPE_FUNCTION_OBJECT:
      {
        /* TODO(mkm): use GC heap */
        struct v7_function *f =
            (struct v7_function *) malloc(sizeof(struct v7_function));
        val_t fval = v7_function_to_value(f);
        if (f == NULL) {
          return V7_NULL;
        }
        f->properties = NULL;
        f->scope = NULL;
        /* TODO(mkm): lazily create these properties on first access */
        v7_set_property_value(v7, fval, "prototype", -1, 0,
                              v7_create_object(v7, v7->object_prototype));
        return fval;
      }
    case V7_TYPE_CFUNCTION_OBJECT:
      return v7_cfunction_to_value(va_arg(ap, v7_cfunction_t));
    case V7_TYPE_BOOLEAN_OBJECT:
    case V7_TYPE_STRING_OBJECT:
    case V7_TYPE_NUMBER_OBJECT:
    case V7_TYPE_REGEXP_OBJECT:
    case V7_TYPE_DATE_OBJECT:
    case V7_TYPE_ERROR_OBJECT:
    default:
      printf("NOT IMPLEMENTED YET create %d\n", type); /* LCOV_EXCL_LINE */
      abort();
  }

  return v;
}

int v7_to_json(struct v7 *v7, val_t v, char *buf, size_t size) {
  /* TODO(mkm): call the toString method instead of custom C code. */
  switch (val_type(v7, v)) {
    case V7_TYPE_NULL:
      return stpncpy(buf, "null", size) - buf;
    case V7_TYPE_UNDEFINED:
      return stpncpy(buf, "undefined", size) - buf;
    case V7_TYPE_BOOLEAN:
      return stpncpy(buf,
                     val_to_boolean(v) ? "true" : "false", size) - buf;
    case V7_TYPE_NUMBER:
      if (v == V7_TAG_NAN) {
        return snprintf(buf, size, "NaN");
      }
      return snprintf(buf, size, "%lg", val_to_double(v));
    case V7_TYPE_STRING:
      {
        size_t n;
        const char *str = val_to_string(v7, &v, &n);
        return snprintf(buf, size, "\"%.*s\"", (int) n, str);
      }
    case V7_TYPE_CFUNCTION_OBJECT:
      return snprintf(buf, size, "cfunc_%p", val_to_pointer(v));
    case V7_TYPE_GENERIC_OBJECT:
    case V7_TYPE_BOOLEAN_OBJECT:
    case V7_TYPE_STRING_OBJECT:
    case V7_TYPE_NUMBER_OBJECT:
    case V7_TYPE_REGEXP_OBJECT:
    case V7_TYPE_DATE_OBJECT:
    case V7_TYPE_ERROR_OBJECT:
      {
        char *b = buf;
        struct v7_property *p;
        b += snprintf(b, size - (b - buf), "{");
        for (p = val_to_object(v)->properties;
             p && (size - (b - buf)); p = p->next) {
          b += snprintf(b, size - (b - buf), "\"%s\":", p->name);
          b += v7_to_json(v7, p->value, b, size - (b - buf));
          if (p->next) {
            b += snprintf(b, size - (b - buf), ",");
          }
        }
        b += snprintf(b, size - (b - buf), "}");
        return b - buf;
      }
    case V7_TYPE_ARRAY_OBJECT:
      {
        struct v7_property *p;
        char *b = buf;
        char key[512];
        size_t i, len = v7_array_length(v7, v);
        b += snprintf(b, size - (b - buf), "[");
        for (i = 0; i < len; i++) {
          /* TODO */
          snprintf(key, sizeof(key), "%lu", i);
          if ((p = v7_get_property(v, key, -1)) != NULL) {
            b += v7_to_json(v7, p->value, b, size - (b - buf));
          }
          if (i != len - 1) {
            b += snprintf(b, size - (b - buf), ",");
          }
        }
        b += snprintf(b, size - (b - buf), "]");
        return b - buf;
      }
    case V7_TYPE_FUNCTION_OBJECT:
      {
        char *name;
        size_t name_len;
        char *b = buf;
        struct v7_function *func = val_to_function(v);
        ast_off_t end, body, var, var_end, start, pos = func->ast_off;
        struct ast *a = func->ast;

        b += snprintf(b, size - (b - buf), "[function");

        assert(ast_fetch_tag(a, &pos) == AST_FUNC);
        start = pos - 1;
        end = ast_get_skip(a, pos, AST_END_SKIP);
        body = ast_get_skip(a, pos, AST_FUNC_BODY_SKIP);
        /* TODO(mkm) cleanup this - 1*/
        var = ast_get_skip(a, pos, AST_FUNC_FIRST_VAR_SKIP) - 1;

        ast_move_to_children(a, &pos);
        if (ast_fetch_tag(a, &pos) == AST_IDENT) {
          name = ast_get_inlined_data(a, pos, &name_len);
          ast_move_to_children(a, &pos);
          b += snprintf(b, size - (b - buf), " %.*s", (int) name_len, name);
        }
        b += snprintf(b, size - (b - buf), "(");
        while (pos < body) {
          assert(ast_fetch_tag(a, &pos) == AST_IDENT);
          name = ast_get_inlined_data(a, pos, &name_len);
          ast_move_to_children(a, &pos);
          b += snprintf(b, size - (b - buf), "%.*s", (int) name_len, name);
          if (pos < body) {
            b += snprintf(b, size - (b - buf), ",");
          }
        }
        b += snprintf(b, size - (b - buf), ")");
        if (var != start) {
          ast_off_t next;
          b += snprintf(b, size - (b - buf), "{var ");

          do {
            assert(ast_fetch_tag(a, &var) == AST_VAR);
            next = ast_get_skip(a, var, AST_VAR_NEXT_SKIP);
            if (next == var) {
              next = 0;
            }

            var_end = ast_get_skip(a, var, AST_END_SKIP);
            ast_move_to_children(a, &var);
            while (var < var_end) {
              assert(ast_fetch_tag(a, &var) == AST_VAR_DECL);
              name = ast_get_inlined_data(a, var, &name_len);
              ast_move_to_children(a, &var);
              ast_skip_tree(a, &var);

              b += snprintf(b, size - (b - buf), "%.*s", (int) name_len, name);
              if (var < var_end || next) {
                b += snprintf(b, size - (b - buf), ",");
              }
            }
            var = next - 1; /* TODO(mkm): cleanup */
          } while (next != 0);
          b += snprintf(b, size - (b - buf), "}");
        }
        b += snprintf(b, size - (b - buf), "]");
        return b - buf;
      }
    default:
      printf("NOT IMPLEMENTED YET\n");  /* LCOV_EXCL_LINE */
      abort();
  }
}

char *debug_json(struct v7 *v7, val_t v) {
  char buf[1024];
  char *res;
  v7_to_json(v7, v, buf, sizeof(buf) - 1);
  res = (char *) malloc(strlen(buf) + 1);
  memset(res, 0, strlen(buf) + 1);
  memcpy(res, buf, strlen(buf));
  return res;
}


int v7_stringify_value(struct v7 *v7, val_t v, char *buf,
                       size_t size) {
  if (v7_is_string(v)) {
    size_t n;
    const char *str = val_to_string(v7, &v, &n);
    if (n >= size) {
      n = size - 1;
    }
    stpncpy(buf, str, n);
    buf[n] = '\0';
    return n;
  } else {
    return v7_to_json(v7, v, buf, size);
  }
}

V7_PRIVATE struct v7_property *v7_create_property(struct v7 *v7) {
  /* TODO(mkm): allocate from GC pool */
  (void) v7;
  return (struct v7_property *) calloc(1, sizeof(struct v7_property));
}

struct v7_property *v_find_property(val_t obj, const char *name, size_t len) {
  struct v7_property *prop;
  if (len == (size_t) ~0) {
    len = strlen(name);
  }
  for (prop = val_to_object(obj)->properties; prop != NULL;
       prop = prop->next) {
    if (len == strlen(prop->name) && strncmp(prop->name, name, len) == 0) {
      return prop;
    }
  }
  return NULL;
}

struct v7_property *v7_get_property(val_t obj, const char *name,
                                    size_t len) {
  if (!v7_is_object(obj)) {
    return NULL;
  }
  for (; obj != V7_NULL; obj = v_get_prototype(obj)) {
    struct v7_property *prop;
    if ((prop = v_find_property(obj, name, len)) != NULL) {
      return prop;
    }
  }
  return NULL;
}

static void v7_destroy_property(struct v7_property **p) {
  free((*p)->name);
  free(*p);
  *p = NULL;
}

int v7_set_property_value(struct v7 *v7, val_t obj,
                          const char *name, size_t len,
                          unsigned int attributes, val_t val) {
  struct v7_property *prop;

  if (!v7_is_object(obj)) {
    return -1;
  }

  prop = v_find_property(obj, name, len);
  if (prop == NULL) {
    if ((prop = v7_create_property(v7)) == NULL) {
      return -1;
    }
    prop->next = val_to_object(obj)->properties;
    val_to_object(obj)->properties = prop;
  }

  prop->attributes = attributes;
  if (len == (size_t) ~0) {
    len = strlen(name);
  }
  prop->name = malloc(len + 1);
  strncpy(prop->name, name, len);
  prop->name[len] = '\0';

  prop->value = val;
  return 0;
}

int v7_set_property(struct v7 *v7, val_t obj, const char *name,
                    size_t len, unsigned int attributes,
                    enum v7_type type, ...) {
  val_t val;
  va_list ap;
  int res;

  va_start(ap, type);
  val = v7_va_create_value(v7, type, ap);
  res = v7_set_property_value(v7, obj, name, len, attributes, val);
  return res;
}

int v7_del_property(val_t obj, const char *name, size_t len) {
  struct v7_property *prop, *prev;

  if (!v7_is_object(obj)) {
    return -1;
  }
  if (len == (size_t) ~0) {
    len = strlen(name);
  }
  for (prev = NULL, prop = val_to_object(obj)->properties; prop != NULL;
       prev = prop, prop = prop->next) {
    if (len == strlen(prop->name) && strncmp(prop->name, name, len) == 0) {
      if (prev) {
        prev->next = prop->next;
      } else {
        val_to_object(obj)->properties = prop->next;
      }
      v7_destroy_property(&prop);
      return 0;
    }
  }
  return -1;
}

V7_PRIVATE val_t v7_property_value(struct v7_property *p) {
  if (p == NULL) {
    return V7_UNDEFINED;
  }
  return p->value;
}

V7_PRIVATE long v7_array_length(struct v7 *v7, val_t v) {
  struct v7_property *p;
  long max = -1, k;
  char *end;

  if (val_type(v7, v) != V7_TYPE_ARRAY_OBJECT) {
    return -1;
  }
  for (p = val_to_object(v)->properties; p != NULL; p = p->next) {
    k = strtol(p->name, &end, 10);
    if (end != p->name && k > max) {
      max = k;
    }
  }
  return max + 1;
}

void v7_array_append(struct v7 *v7, v7_val_t arr, v7_val_t v) {
  if (val_type(v7, arr) == V7_TYPE_ARRAY_OBJECT) {
    char buf[20];
    int n = snprintf(buf, sizeof(buf), "%ld", v7_array_length(v7, arr));
    v7_set_property_value(v7, arr, buf, n, 0, v);
  }
}

val_t v7_array_at(struct v7 *v7, val_t arr, long index) {
  if (val_type(v7, arr) == V7_TYPE_ARRAY_OBJECT) {
    char buf[20];
    int n = snprintf(buf, sizeof(buf), "%ld", index);
    struct v7_property *prop = v7_get_property(arr, buf, n);
    return prop == NULL ? V7_UNDEFINED : prop->value;
  } else {
    return V7_UNDEFINED;
  }
}

/* Insert a string into mbuf at specified offset */
V7_PRIVATE void embed_string(struct mbuf *m, size_t offset, const char *p,
                             size_t n) {
  int k = calc_llen(n);           /* Calculate how many bytes length takes */
  mbuf_insert(m, offset, NULL, k);   /* Allocate  buffer for length */
  encode_varint(n, (unsigned char *) m->buf + offset);  /* Write length */
  mbuf_insert(m, offset + k, p, n);  /* Copy the string itself */
}

/* Create a string */
val_t v7_string_to_value(struct v7 *v7, const char *p, size_t len, int own) {
  struct mbuf *m = own ? &v7->owned_strings : &v7->foreign_strings;
  val_t offset = m->len, tag = V7_TAG_STRING_F;

  if (len <= 5) {
    char *s = GET_VAL_NAN_PAYLOAD(offset) + 1;
    offset = 0;
    memcpy(s, p, len);
    s[-1] = len;
    tag = V7_TAG_STRING_I;
  } else if (own) {
    embed_string(m, m->len, p, len);
    tag = V7_TAG_STRING_O;
  } else {
    /* TODO(mkm): this doesn't set correctly the foreign string length */
    embed_string(m, m->len, (char *) &p, sizeof(p));
  }

  return v7_pointer_to_value((void *) offset) | tag;
}

/* Get a pointer to string and string length */
const char *val_to_string(struct v7 *v7, val_t *v, size_t *sizep) {
  uint64_t tag = v[0] & V7_TAG_MASK;
  char *p;
  int llen;

  if (tag == V7_TAG_STRING_I) {
    p = GET_VAL_NAN_PAYLOAD(*v) + 1;
    *sizep = p[-1];
  } else {
    struct mbuf *m = (tag == V7_TAG_STRING_O) ?
      &v7->owned_strings : &v7->foreign_strings;
    size_t offset = (size_t) val_to_pointer(*v);
    char *s = m->buf + offset;

    *sizep = decode_varint((uint8_t *) s, &llen);
    p = (tag == V7_TAG_STRING_O) ? s + llen : * (char **) (s + llen);
  }

  return p;
}

V7_PRIVATE int s_cmp(struct v7 *v7, val_t a, val_t b) {
  size_t a_len, b_len;
  const char *a_ptr, *b_ptr;

  a_ptr = val_to_string(v7, &a, &a_len);
  b_ptr = val_to_string(v7, &b, &b_len);

  /* TODO(lsm): fix this */
  return memcmp(a_ptr, b_ptr, a_len < b_len ? a_len : b_len);
}

V7_PRIVATE val_t s_concat(struct v7 *v7, val_t a, val_t b) {
  size_t a_len, b_len, offset = v7->owned_strings.len;
  const char *a_ptr, *b_ptr;
  char *s = v7->owned_strings.buf + offset;
  uint64_t tag = V7_TAG_STRING_F;

  a_ptr = val_to_string(v7, &a, &a_len);
  b_ptr = val_to_string(v7, &b, &b_len);

  /* Create a new string which is a concatenation a + b */
  if (a_len + b_len <= 5) {
    offset = 0;
    s = ((char *) &offset) + 3;
    tag = V7_TAG_STRING_I;
  } else {
    mbuf_append(&v7->owned_strings, NULL, a_len + b_len);
    tag = V7_TAG_STRING_O;
  }
  memcpy(s, a_ptr, a_len);
  memcpy(s + a_len, b_ptr, b_len);

  return v7_pointer_to_value((void *) offset) | tag;
}

V7_PRIVATE val_t s_substr(struct v7 *v7, val_t s, size_t start, size_t len) {
  size_t n;
  const char *p = val_to_string(v7, &s, &n);
  if (len > n) len = n;   /* boundary check */
  if (start > n) start = n;
  /* TODO(lsm): if the substring len <= 5 bytes, inline into val_t */
  return v7_string_to_value(v7, p + start, len, 1);
}
