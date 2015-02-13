/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"
#include "gc.h"

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
      if (v7_to_object(v)->prototype == v7_to_object(v7->array_prototype)) {
        return V7_TYPE_ARRAY_OBJECT;
      } else if (v7_to_object(v)->prototype ==
                 v7_to_object(v7->boolean_prototype)) {
        return V7_TYPE_BOOLEAN_OBJECT;
      } else if (v7_to_object(v)->prototype ==
                 v7_to_object(v7->string_prototype)) {
        return V7_TYPE_STRING_OBJECT;
      } else if (v7_to_object(v)->prototype ==
                 v7_to_object(v7->number_prototype)) {
        return V7_TYPE_NUMBER_OBJECT;
      } else if (v7_to_object(v)->prototype ==
                 v7_to_object(v7->cfunction_prototype)) {
        return V7_TYPE_CFUNCTION_OBJECT;
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
    case V7_TAG_CFUNCTION:
      return V7_TYPE_CFUNCTION;
    case V7_TAG_REGEXP:
      return V7_TYPE_REGEXP_OBJECT;
    default:
      /* TODO(mkm): or should we crash? */
      return V7_TYPE_UNDEFINED;
  }
}

int v7_is_double(val_t v) {
  return v == V7_TAG_NAN || !isnan(v7_to_double(v));
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

int v7_is_regexp(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_REGEXP;
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

/* A convenience function to check exec result */
int v7_is_error(struct v7 *v7, val_t v) {
  return is_prototype_of(v, v7->error_prototype);
}

V7_PRIVATE val_t v7_pointer_to_value(void *p) {
  return ((uint64_t) p & ((1L << 48) -1));
}

V7_PRIVATE void *v7_to_pointer(val_t v) {
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

struct v7_object *v7_to_object(val_t v) {
  return (struct v7_object *) v7_to_pointer(v);
}

val_t v7_function_to_value(struct v7_function *o) {
  return v7_pointer_to_value(o) | V7_TAG_FUNCTION;
}

struct v7_function *v7_to_function(val_t v) {
  return (struct v7_function *) v7_to_pointer(v);
}

v7_cfunction_t v7_to_cfunction(val_t v) {
  return (v7_cfunction_t) v7_to_pointer(v);
}

v7_val_t v7_create_cfunction(v7_cfunction_t f) {
  union {
    void *p;
    v7_cfunction_t f;
  } u;
  u.f = f;
  return v7_pointer_to_value(u.p) | V7_TAG_CFUNCTION;
}

void *v7_to_foreign(val_t v) {
  return v7_to_pointer(v);
}

v7_val_t v7_create_boolean(int v) {
  return (!!v) | V7_TAG_BOOLEAN;
}

int v7_to_boolean(val_t v) {
  return v & 1;
}

v7_val_t v7_create_number(double v) {
  val_t res;
  /* not every NaN is a JS NaN */
  if (isnan(v)) {
    res = V7_TAG_NAN;
  } else {
    * (double *) &res = v;
  }
  return res;
}

double v7_to_double(val_t v) {
  return * (double *) &v;
}

V7_PRIVATE val_t v_get_prototype(val_t obj) {
  return v7_object_to_value(v7_to_object(obj)->prototype);
}

V7_PRIVATE val_t create_object(struct v7 *v7, val_t prototype) {
  /* TODO(mkm): use GC heap */
  struct v7_object *o = new_object(v7);
  if (o == NULL) {
    return V7_NULL;
  }
  (void) v7;
  o->properties = NULL;
  o->prototype = v7_to_object(prototype);
  return v7_object_to_value(o);
}

v7_val_t v7_create_object(struct v7 *v7) {
  return create_object(v7, v7->object_prototype);
}

v7_val_t v7_create_null(void) {
  return V7_NULL;
}

v7_val_t v7_create_undefined(void) {
  return V7_UNDEFINED;
}

v7_val_t v7_create_array(struct v7 *v7) {
  return create_object(v7, v7->array_prototype);
}

v7_val_t v7_create_regexp(struct v7 *v7, const char *re, size_t re_len,
                          const char *flags, size_t flags_len) {
  struct slre_prog *p = NULL;
  struct v7_regexp *rp;

  if (slre_compile(re, re_len, flags, flags_len, &p) != SLRE_OK || p == NULL) {
    throw_exception(v7, "Error", "Invalid regex");
    return V7_UNDEFINED;
  } else {
    rp = (struct v7_regexp *) malloc(sizeof(*rp));
    rp->regexp_string = v7_create_string(v7, re, re_len, 1);
    rp->flags_string = v7_create_string(v7, flags, flags_len, 1);
    rp->compiled_regexp = p;

    return v7_pointer_to_value(rp) | V7_TAG_REGEXP;
  }
}

v7_val_t v7_create_function(struct v7 *v7) {
  /* TODO(mkm): use GC heap */
  struct v7_function *f = new_function(v7);
  val_t proto, fval = v7_function_to_value(f);
  if (f == NULL) {
    return V7_NULL;
  }
  f->properties = NULL;
  f->scope = NULL;
  f->attributes = 0;
  /* TODO(mkm): lazily create these properties on first access */
  proto = v7_create_object(v7);
  v7_set_property(v7, proto, "constructor", 11, V7_PROPERTY_DONT_ENUM, fval);
  v7_set_property(v7, fval, "prototype", 9, V7_PROPERTY_DONT_ENUM |
                  V7_PROPERTY_DONT_DELETE, proto);
  return fval;
}

/* like snprintf but returns `size` if write is truncated */
static int v_sprintf_s(char *buf, size_t size, const char *fmt, ...) {
  size_t n;
  va_list ap;
  va_start(ap, fmt);
  n = vsnprintf(buf, size, fmt, ap);
  if (n > size) {
    return size;
  }
  return n;
}

V7_PRIVATE int to_str(struct v7 *v7, val_t v, char *buf, size_t size,
                      int as_json) {
  char *vp;
  double num;
  for (vp = v7->json_visited_stack.buf;
       vp < v7->json_visited_stack.buf+ v7->json_visited_stack.len;
       vp += sizeof(val_t)) {
    if (* (val_t *) vp == v) {
      return stpncpy(buf, "[Circular]", size) - buf;
    }
  }

  switch (val_type(v7, v)) {
    case V7_TYPE_NULL:
      return stpncpy(buf, "null", size) - buf;
    case V7_TYPE_UNDEFINED:
      return stpncpy(buf, "undefined", size) - buf;
    case V7_TYPE_BOOLEAN:
      return stpncpy(buf,
                     v7_to_boolean(v) ? "true" : "false", size) - buf;
    case V7_TYPE_NUMBER:
      if (v == V7_TAG_NAN) {
        return v_sprintf_s(buf, size, "NaN");
      }
      num = v7_to_double(v);
      if (isinf(num)) {
        return v_sprintf_s(buf, size, "%sInfinity", num < 0.0 ? "-" : "");
      }
      return v_sprintf_s(buf, size, "%lg", num);
    case V7_TYPE_STRING:
      {
        size_t n;
        const char *str = v7_to_string(v7, &v, &n);
        if (as_json) {
          return v_sprintf_s(buf, size, "\"%.*s\"", (int) n, str);
        } else {
          return v_sprintf_s(buf, size, "%.*s", (int) n, str);
        }
      }
    case V7_TYPE_REGEXP_OBJECT:
      {
        size_t n1, n2;
        struct v7_regexp *rp = (struct v7_regexp *) v7_to_pointer(v);
        const char *s1 = v7_to_string(v7, &rp->regexp_string, &n1);
        const char *s2 = v7_to_string(v7, &rp->flags_string, &n2);
        return v_sprintf_s(buf, size, "/%.*s/%.*s", (int) n1, s1, (int) n2, s2);
      }
    case V7_TYPE_CFUNCTION:
      return v_sprintf_s(buf, size, "cfunc_%p", v7_to_pointer(v));
    case V7_TYPE_CFUNCTION_OBJECT:
      v = i_value_of(v7, v);
      return v_sprintf_s(buf, size, "Function cfunc_%p", v7_to_pointer(v));
    case V7_TYPE_GENERIC_OBJECT:
    case V7_TYPE_BOOLEAN_OBJECT:
    case V7_TYPE_STRING_OBJECT:
    case V7_TYPE_NUMBER_OBJECT:
    case V7_TYPE_DATE_OBJECT:
    case V7_TYPE_ERROR_OBJECT:
      {
        char *b = buf;
        struct v7_property *p;
        mbuf_append(&v7->json_visited_stack, (char *) &v, sizeof(v));
        b += v_sprintf_s(b, size - (b - buf), "{");
        for (p = v7_to_object(v)->properties;
             p && (size - (b - buf)); p = p->next) {
          if (p->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)) {
            continue;
          }
          b += v_sprintf_s(b, size - (b - buf), "\"%s\":", p->name);
          b += to_str(v7, p->value, b, size - (b - buf), 1);
          if (p->next) {
            b += v_sprintf_s(b, size - (b - buf), ",");
          }
        }
        b += v_sprintf_s(b, size - (b - buf), "}");
        v7->json_visited_stack.len -= sizeof(v);
        return b - buf;
      }
    case V7_TYPE_ARRAY_OBJECT:
      {
        struct v7_property *p;
        char *b = buf;
        char key[512];
        size_t i, len = v7_array_length(v7, v);
        mbuf_append(&v7->json_visited_stack, (char *) &v, sizeof(v));
        if (as_json) {
          b += v_sprintf_s(b, size - (b - buf), "[");
        }
        for (i = 0; i < len; i++) {
          /* TODO */
          v_sprintf_s(key, sizeof(key), "%lu", i);
          if ((p = v7_get_property(v, key, -1)) != NULL) {
            b += to_str(v7, p->value, b, size - (b - buf), 1);
          }
          if (i != len - 1) {
            b += v_sprintf_s(b, size - (b - buf), ",");
          }
        }
        if (as_json) {
          b += v_sprintf_s(b, size - (b - buf), "]");
        }
        v7->json_visited_stack.len -= sizeof(v);
        return b - buf;
      }
    case V7_TYPE_FUNCTION_OBJECT:
      {
        char *name;
        size_t name_len;
        char *b = buf;
        struct v7_function *func = v7_to_function(v);
        ast_off_t body, var, var_end, start, pos = func->ast_off;
        struct ast *a = func->ast;

        b += v_sprintf_s(b, size - (b - buf), "[function");

        V7_CHECK(v7, ast_fetch_tag(a, &pos) == AST_FUNC);
        start = pos - 1;
        body = ast_get_skip(a, pos, AST_FUNC_BODY_SKIP);
        /* TODO(mkm) cleanup this - 1*/
        var = ast_get_skip(a, pos, AST_FUNC_FIRST_VAR_SKIP) - 1;

        ast_move_to_children(a, &pos);
        if (ast_fetch_tag(a, &pos) == AST_IDENT) {
          name = ast_get_inlined_data(a, pos, &name_len);
          ast_move_to_children(a, &pos);
          b += v_sprintf_s(b, size - (b - buf), " %.*s", (int) name_len, name);
        }
        b += v_sprintf_s(b, size - (b - buf), "(");
        while (pos < body) {
          V7_CHECK(v7, ast_fetch_tag(a, &pos) == AST_IDENT);
          name = ast_get_inlined_data(a, pos, &name_len);
          ast_move_to_children(a, &pos);
          b += v_sprintf_s(b, size - (b - buf), "%.*s", (int) name_len, name);
          if (pos < body) {
            b += v_sprintf_s(b, size - (b - buf), ",");
          }
        }
        b += v_sprintf_s(b, size - (b - buf), ")");
        if (var != start) {
          ast_off_t next;
          b += v_sprintf_s(b, size - (b - buf), "{var ");

          do {
            V7_CHECK(v7, ast_fetch_tag(a, &var) == AST_VAR);
            next = ast_get_skip(a, var, AST_VAR_NEXT_SKIP);
            if (next == var) {
              next = 0;
            }

            var_end = ast_get_skip(a, var, AST_END_SKIP);
            ast_move_to_children(a, &var);
            while (var < var_end) {
              V7_CHECK(v7, ast_fetch_tag(a, &var) == AST_VAR_DECL);
              name = ast_get_inlined_data(a, var, &name_len);
              ast_move_to_children(a, &var);
              ast_skip_tree(a, &var);

              b += v_sprintf_s(b, size - (b - buf), "%.*s", (int) name_len,
                               name);
              if (var < var_end || next) {
                b += v_sprintf_s(b, size - (b - buf), ",");
              }
            }
            var = next - 1; /* TODO(mkm): cleanup */
          } while (next != 0);
          b += v_sprintf_s(b, size - (b - buf), "}");
        }
        b += v_sprintf_s(b, size - (b - buf), "]");
        return b - buf;
      }
    default:
      printf("NOT IMPLEMENTED YET\n");  /* LCOV_EXCL_LINE */
      abort();
  }
}

char *v7_to_json(struct v7 *v7, val_t v, char *buf, size_t size) {
  int len = to_str(v7, v, buf, size, 1);

  if (len > (int) size) {
    /* Buffer is not large enough. Allocate a bigger one */
    char *p = (char *) malloc(len + 1);
    to_str(v7, v, p, len + 1, 1);
    p[len] = '\0';
    return p;
  } else {
    return buf;
  }
}

int v7_stringify_value(struct v7 *v7, val_t v, char *buf,
                       size_t size) {
  if (v7_is_string(v)) {
    size_t n;
    const char *str = v7_to_string(v7, &v, &n);
    if (n >= size) {
      n = size - 1;
    }
    strncpy(buf, str, n);
    buf[n] = '\0';
    return n;
  } else {
    return to_str(v7, v, buf, size, 1);
  }
}

V7_PRIVATE struct v7_property *v7_create_property(struct v7 *v7) {
  struct v7_property *p = new_property(v7);
  p->next = NULL;
  p->name = NULL;
  p->value = V7_UNDEFINED;
  p->attributes = 0;
  return p;
}

V7_PRIVATE struct v7_property *v7_get_own_property2(val_t obj, const char *name,
                                                    size_t len,
                                                    unsigned int attrs) {
  struct v7_property *prop;
  if (len == (size_t) ~0) {
    len = strlen(name);
  }
  if (!v7_is_object(obj)) {
    return NULL;
  }

  for (prop = v7_to_object(obj)->properties; prop != NULL;
       prop = prop->next) {
    if (len == strlen(prop->name) && strncmp(prop->name, name, len) == 0 &&
        (attrs == 0 || (prop->attributes & attrs))) {
      return prop;
    }
  }
  return NULL;
}

V7_PRIVATE struct v7_property *v7_get_own_property(val_t obj, const char *name,
                                                   size_t len) {
  return v7_get_own_property2(obj, name, len, 0);
}

struct v7_property *v7_get_property(val_t obj, const char *name, size_t len) {
  if (!v7_is_object(obj)) {
    return NULL;
  }
  for (; obj != V7_NULL; obj = v_get_prototype(obj)) {
    struct v7_property *prop;
    if ((prop = v7_get_own_property(obj, name, len)) != NULL) {
      return prop;
    }
  }
  return NULL;
}

v7_val_t v7_get(struct v7 *v7, val_t obj, const char *name, size_t name_len) {
  val_t v = obj;
  if (v7_is_string(obj)) {
    v = v7->string_prototype;
  } else if (v7_is_double(obj)) {
    v = v7->number_prototype;
  } else if (v7_is_boolean(obj)) {
    v = v7->boolean_prototype;
  } else if (v7_is_undefined(obj)) {
    throw_exception(v7, "TypeError",
                    "cannot read property '%.*s' of undefined",
                    (int) name_len, name);
  } else if (v7_is_cfunction(obj)) {
    /*
     * TODO(mkm): until cfunctions can have properties
     * let's treat special constructors specially.
     * Slow path acceptable here.
     */
    if (obj == v7_get(v7, v7->global_object, "Boolean", 7) &&
        name_len == 9 && strncmp(name, "prototype", name_len) == 0) {
      return v7->boolean_prototype;
    } else if (obj == v7_get(v7, v7->global_object, "String", 7) &&
        name_len == 9 && strncmp(name, "prototype", name_len) == 0) {
      return v7->string_prototype;
    } else if (obj == v7_get(v7, v7->global_object, "Number", 7) &&
        name_len == 9 && strncmp(name, "prototype", name_len) == 0) {
      return v7->number_prototype;
    }
    return V7_UNDEFINED;
  }
  return v7_property_value(v7, obj, v7_get_property(v, name, name_len));
}

V7_PRIVATE void v7_destroy_property(struct v7_property **p) {
  *p = NULL;
}

int v7_set(struct v7 *v7, val_t obj, const char *name, size_t len, val_t val) {
  struct v7_property *p = v7_get_own_property(obj, name, len);
  if (p == NULL || !(p->attributes & V7_PROPERTY_READ_ONLY)) {
    return v7_set_property(v7, obj, name, len, p == NULL ? 0 : p->attributes, val);
  }
  return -1;
}

V7_PRIVATE void v7_invoke_setter(struct v7 *v7, struct v7_property *prop,
                                 val_t obj, val_t val) {
  val_t setter = prop->value, args = v7_create_array(v7);
  if (prop->attributes & V7_PROPERTY_GETTER) {
    setter = v7_array_at(v7, prop->value, 1);
  }
  v7_set(v7, args, "0", 1, val);
  v7_apply(v7, setter, obj, args);
}

int v7_set_property(struct v7 *v7, val_t obj, const char *name, size_t len,
                    unsigned int attributes, v7_val_t val) {
  struct v7_property *prop;

  if (!v7_is_object(obj)) {
    return -1;
  }

  prop = v7_get_own_property(obj, name, len);
  if (prop == NULL) {
    if ((prop = v7_create_property(v7)) == NULL) {
      return -1;  /* LCOV_EXCL_LINE */
    }
    prop->next = v7_to_object(obj)->properties;
    v7_to_object(obj)->properties = prop;
  }

  if (len == (size_t) ~0) {
    len = strlen(name);
  }
  if (prop->name == NULL) {
    prop->name = (char *) malloc(len + 1);
    strncpy(prop->name, name, len);
    prop->name[len] = '\0';
  }
  if (prop->attributes & V7_PROPERTY_SETTER) {
    v7_invoke_setter(v7, prop, obj, val);
    return 0;
  }
  prop->value = val;
  prop->attributes = attributes;
  return 0;
}

int v7_del_property(val_t obj, const char *name, size_t len) {
  struct v7_property *prop, *prev;

  if (!v7_is_object(obj)) {
    return -1;
  }
  if (len == (size_t) ~0) {
    len = strlen(name);
  }
  for (prev = NULL, prop = v7_to_object(obj)->properties; prop != NULL;
       prev = prop, prop = prop->next) {
    if (len == strlen(prop->name) && strncmp(prop->name, name, len) == 0) {
      if (prev) {
        prev->next = prop->next;
      } else {
        v7_to_object(obj)->properties = prop->next;
      }
      v7_destroy_property(&prop);
      return 0;
    }
  }
  return -1;
}

V7_PRIVATE v7_val_t v7_create_cfunction_object(struct v7 *v7,
                                               v7_cfunction_t f) {
  val_t obj = create_object(v7, v7->cfunction_prototype);
  v7_set_property(v7, obj, "", 0, V7_PROPERTY_HIDDEN, v7_create_cfunction(f));
  return obj;
}

V7_PRIVATE int set_cfunc_obj_prop(struct v7 *v7, val_t o, const char *name,
                                  v7_cfunction_t f) {
  return v7_set_property(v7, o, name, strlen(name), 0,
                         v7_create_cfunction_object(v7, f));
}

V7_PRIVATE int set_cfunc_prop(struct v7 *v7, val_t o, const char *name,
                              v7_cfunction_t f) {
  return v7_set_property(v7, o, name, strlen(name), 0, v7_create_cfunction(f));
}

V7_PRIVATE val_t v7_property_value(struct v7 *v7, val_t obj, struct v7_property *p) {
  if (p == NULL) {
    return V7_UNDEFINED;
  }
  if (p->attributes & V7_PROPERTY_GETTER) {
    val_t getter = p->value;
    if (p->attributes & V7_PROPERTY_SETTER) {
      getter = v7_array_at(v7, p->value, 0);
    }
    return v7_apply(v7, getter, obj, V7_UNDEFINED);
  }
  return p->value;
}

V7_PRIVATE long v7_array_length(struct v7 *v7, val_t v) {
  struct v7_property *p;
  long max = -1, k;
  char *end;

  (void) v7;
  if (!v7_is_object(v)) {
    return -1;
  }

  for (p = v7_to_object(v)->properties; p != NULL; p = p->next) {
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
    int n = v_sprintf_s(buf, sizeof(buf), "%ld", v7_array_length(v7, arr));
    v7_set_property(v7, arr, buf, n, 0, v);
  }
}

val_t v7_array_at(struct v7 *v7, val_t arr, long index) {
  if (v7_is_object(arr)) {
    char buf[20];
    int n = v_sprintf_s(buf, sizeof(buf), "%ld", index);
    return v7_get(v7, arr, buf, n);
  } else {
    return V7_UNDEFINED;
  }
}

int nextesc(const char **p);  /* from SLRE */
V7_PRIVATE size_t unescape(const char *s, size_t len, char *to) {
  const char *end = s + len;
  size_t n = 0;
  char tmp[4];
  Rune r;

  while (s < end) {
    s += chartorune(&r, s);
    if (r == '\\' && s < end) {
      switch (*s) {
        case '"':
          s++, r = '"';
          break;
        case '\'':
          s++, r = '\'';
          break;
        case '\n':
          s++, r = '\n';
          break;
        default:
          r = nextesc(&s);
      }
    }
    n += runetochar(to == NULL ? tmp : to + n, &r);
  }

  return n;
}

/* Insert a string into mbuf at specified offset */
V7_PRIVATE void embed_string(struct mbuf *m, size_t offset, const char *p,
                             size_t len) {
  size_t n = unescape(p, len, NULL);
  int k = calc_llen(n);           /* Calculate how many bytes length takes */
  mbuf_insert(m, offset, NULL, k + n);   /* Allocate  buffer */
  encode_varint(n, (unsigned char *) m->buf + offset);  /* Write length */
  unescape(p, len, m->buf + offset + k);  /* Write string */
}

/* Create a string */
v7_val_t v7_create_string(struct v7 *v7, const char *p, size_t len, int own) {
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
const char *v7_to_string(struct v7 *v7, val_t *v, size_t *sizep) {
  uint64_t tag = v[0] & V7_TAG_MASK;
  char *p;
  int llen;

  if (tag == V7_TAG_STRING_I) {
    p = GET_VAL_NAN_PAYLOAD(*v) + 1;
    *sizep = p[-1];
  } else {
    struct mbuf *m = (tag == V7_TAG_STRING_O) ?
      &v7->owned_strings : &v7->foreign_strings;
    size_t offset = (size_t) v7_to_pointer(*v);
    char *s = m->buf + offset;

    *sizep = decode_varint((uint8_t *) s, &llen);
    p = (tag == V7_TAG_STRING_O) ? s + llen : * (char **) (s + llen);
  }

  return p;
}

V7_PRIVATE int s_cmp(struct v7 *v7, val_t a, val_t b) {
  size_t a_len, b_len;
  const char *a_ptr, *b_ptr;

  a_ptr = v7_to_string(v7, &a, &a_len);
  b_ptr = v7_to_string(v7, &b, &b_len);

  if (a_len == b_len) {
    return memcmp(a_ptr, b_ptr, a_len);
  }
  return a_len - b_len;
}

V7_PRIVATE val_t s_concat(struct v7 *v7, val_t a, val_t b) {
  size_t a_len, b_len, offset = v7->owned_strings.len;
  const char *a_ptr, *b_ptr;
  char *s = NULL;
  uint64_t tag = V7_TAG_STRING_F;

  a_ptr = v7_to_string(v7, &a, &a_len);
  b_ptr = v7_to_string(v7, &b, &b_len);

  /* Create a new string which is a concatenation a + b */
  if (a_len + b_len <= 5) {
    offset = 0;
    /* TODO(mkm): make it work on big endian too */
    s = ((char *) &offset) + 1;
    s[-1] = a_len + b_len;
    tag = V7_TAG_STRING_I;
  } else {
    int llen = calc_llen(a_len + b_len);
    mbuf_append(&v7->owned_strings, NULL, a_len + b_len + llen);
    /* all pointers might have been relocated */
    s = v7->owned_strings.buf + offset;
    encode_varint(a_len + b_len, (unsigned char *) s);  /* Write length */
    s += llen;
    a_ptr = v7_to_string(v7, &a, &a_len);
    b_ptr = v7_to_string(v7, &b, &b_len);
    tag = V7_TAG_STRING_O;
  }
  memcpy(s, a_ptr, a_len);
  memcpy(s + a_len, b_ptr, b_len);

  return v7_pointer_to_value((void *) offset) | tag;
}

V7_PRIVATE val_t s_substr(struct v7 *v7, val_t s, long start, long len) {
  size_t n;
  const char *p = v7_to_string(v7, &s, &n);
  if (!v7_is_string(s)) return V7_UNDEFINED;
  if (start < 0) start = n + start;
  if (start < 0) start = 0;
  if (start > (long) n) start = n;
  if (len < 0) len = 0;
  if (len > (long) n - start) len = n - start;
  return v7_create_string(v7, p + start, len, 1);
}

/* TODO(lsm): remove this when init_stdlib() is upgraded */
V7_PRIVATE v7_val_t Std_print_2(struct v7 *v7, val_t this_obj, val_t args) {
  char *p, buf[1024];
  int i, num_args = v7_array_length(v7, args);

  (void) this_obj;
  for (i = 0; i < num_args; i++) {
    p = v7_to_json(v7, v7_array_at(v7, args, i), buf, sizeof(buf));
    printf("%s", p);
    if (p != buf) {
      free(p);
    }
  }
  putchar('\n');

  return v7_create_null();
}

V7_PRIVATE int is_prototype_of(val_t o, val_t p) {
  struct v7_object *obj, *proto;
  if (!v7_is_object(o) || !v7_is_object(p)) {
    return 0;
  }
  proto = v7_to_object(p);
  for (obj = v7_to_object(o); obj; obj = obj->prototype) {
    if (obj->prototype == proto) {
      return 1;
    }
  }
  return 0;
}

V7_PRIVATE val_t Std_eval(struct v7 *v7, val_t t, val_t args) {
  val_t res = V7_UNDEFINED, arg = v7_array_at(v7, args, 0);
  (void) t;
  if (arg != V7_UNDEFINED) {
    char buf[100], *p;
    p = v7_to_json(v7, arg, buf, sizeof(buf));
    if (p[0] == '"') {
      p[0] = p[strlen(p) - 1] = ' ';
    }
    if (v7_exec(v7, &res, p) != V7_OK) {
      throw_value(v7, res);
    }
    if (p != buf) {
      free(p);
    }
  }
  return res;
}

int v7_is_true(struct v7 *v7, val_t v) {
  size_t len;
  return ((v7_is_boolean(v) && v7_to_boolean(v)) ||
         (v7_is_double(v) && v7_to_double(v) != 0.0 ) ||
         (v7_is_string(v) && v7_to_string(v7, &v, &len) && len > 0) ||
         (v7_is_object(v))) && v != V7_TAG_NAN;
}

struct v7 *v7_create(void) {
  static int prototypes_initialized = 0;
  struct v7 *v7 = NULL;

  if (prototypes_initialized == 0) {
    prototypes_initialized++;
    #if 0
    init_stdlib();  /* One-time initialization */
    #endif
  }

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) != NULL) {
    /*
     * Ensure the first call to v7_create_value will use a null proto:
     * {}.__proto__.__proto__ == null
     */
    v7->object_prototype = create_object(v7, V7_NULL);
    v7->array_prototype = v7_create_object(v7);
    v7->boolean_prototype = v7_create_object(v7);
    v7->string_prototype = v7_create_object(v7);
    v7->number_prototype = v7_create_object(v7);
    v7->cfunction_prototype = v7_create_object(v7);
    v7->global_object = v7_create_object(v7);
    v7->this_object = v7->global_object;

    /* TODO(lsm): remove this when init_stdlib() is upgraded */
    v7_set_property(v7, v7->global_object, "print", 5, 0,
                    v7_create_cfunction(Std_print_2));
    v7_set_property(v7, v7->global_object, "eval", 4, 0,
                    v7_create_cfunction(Std_eval));
    v7_set_property(v7, v7->global_object, "Infinity", 8, 0,
                    v7_create_number(INFINITY));
    v7_set_property(v7, v7->global_object, "global", 6, 0,
                    v7->global_object);

    init_object(v7);
    init_array(v7);
    init_error(v7);
    init_boolean(v7);
    init_math(v7);
    init_string(v7);
    init_number(v7);
    init_json(v7);

    v7->thrown_error = V7_UNDEFINED;
  }

  return v7;
}

val_t v7_get_global_object(struct v7 *v7) {
  return v7->global_object;
}

void v7_destroy(struct v7 *v7) {
  if (v7 != NULL) {
    mbuf_free(&v7->owned_strings);
    mbuf_free(&v7->foreign_strings);
    mbuf_free(&v7->json_visited_stack);
    free(v7);
  }
}
