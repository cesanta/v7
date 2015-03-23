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
      if (v7_is_null(v)) {
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
                 v7_to_object(v7->function_prototype)) {
        return V7_TYPE_CFUNCTION_OBJECT;
      } else if (v7_to_object(v)->prototype ==
                 v7_to_object(v7->date_prototype)) {
        return V7_TYPE_DATE_OBJECT;
      } else {
        return V7_TYPE_GENERIC_OBJECT;
      }
    case V7_TAG_STRING_I:
    case V7_TAG_STRING_O:
    case V7_TAG_STRING_F:
    case V7_TAG_STRING_5:
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
  return t == V7_TAG_STRING_I || t == V7_TAG_STRING_F || t == V7_TAG_STRING_O ||
         t == V7_TAG_STRING_5;
}

int v7_is_boolean(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_BOOLEAN;
}

int v7_is_regexp(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_REGEXP;
}

int v7_is_foreign(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_FOREIGN;
}

V7_PRIVATE struct v7_regexp *v7_to_regexp(val_t v) {
  return (struct v7_regexp *) v7_to_pointer(v);
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
  return is_prototype_of(v7, v, v7->error_prototype);
}

V7_PRIVATE val_t v7_pointer_to_value(void *p) {
  return ((uint64_t)(uintptr_t) p) & ~V7_TAG_MASK;
}

V7_PRIVATE void *v7_to_pointer(val_t v) {
  struct {
    uint64_t s : 48;
  } h;
  return (void *) (uintptr_t)(h.s = v);
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
    *(double *) &res = v;
  }
  return res;
}

double v7_to_double(val_t v) {
  return *(double *) &v;
}

V7_PRIVATE val_t v_get_prototype(struct v7 *v7, val_t obj) {
  if (v7_is_function(obj)) {
    return v7->function_prototype;
  }
  return v7_object_to_value(v7_to_object(obj)->prototype);
}

V7_PRIVATE val_t create_object(struct v7 *v7, val_t prototype) {
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
  val_t a = create_object(v7, v7->array_prototype);
#if 0
  v7_set_property(v7, a, "", 0, V7_PROPERTY_HIDDEN, V7_NULL);
#endif
  return a;
}

/*
 * Dense arrays are backed by mbuf. Currently the array can only grow by
 * appending (i.e. setting an element whose index == array.length)
 *
 * TODO(mkm): automatically promote dense arrays to normal objects
 *            when they are used as sparse arrays or to store arbitrary keys
 *            (perhaps a hybrid approach)
 * TODO(mkm): small sparsness doesn't have to promote the array,
 *            we can just fill empty slots with a tag. In JS missing array
 *            indices are subtly different from indices with an undefined value
 *            (key iteration).
 * TODO(mkm): change the interpreter so it can set elements in dense arrays
 */
V7_PRIVATE val_t v7_create_dense_array(struct v7 *v7) {
  val_t a = v7_create_array(v7);
  v7_set_property(v7, a, "", 0, V7_PROPERTY_HIDDEN, V7_NULL);
  v7_to_object(a)->attributes |= V7_OBJ_DENSE_ARRAY;
  return a;
}

v7_val_t v7_create_regexp(struct v7 *v7, const char *re, size_t re_len,
                          const char *flags, size_t flags_len) {
  struct slre_prog *p = NULL;
  struct v7_regexp *rp;

  if (slre_compile(re, re_len, flags, flags_len, &p, 1) != SLRE_OK ||
      p == NULL) {
    throw_exception(v7, TYPE_ERROR, "Invalid regex");
    return V7_UNDEFINED;
  } else {
    rp = (struct v7_regexp *) malloc(sizeof(*rp));
    rp->regexp_string = v7_create_string(v7, re, re_len, 1);
    rp->compiled_regexp = p;
    rp->lastIndex = 0;

    return v7_pointer_to_value(rp) | V7_TAG_REGEXP;
  }
}

v7_val_t v7_create_foreign(void *p) {
  return v7_pointer_to_value(p) | V7_TAG_FOREIGN;
}

v7_val_t v7_create_function(struct v7 *v7) {
  struct v7_function *f = new_function(v7);
  val_t proto = v7_create_undefined(), fval = v7_function_to_value(f);
  struct gc_tmp_frame tf = new_tmp_frame(v7);
  if (f == NULL) {
    fval = v7_create_null();
    goto cleanup;
  }
  tmp_stack_push(&tf, &proto);
  tmp_stack_push(&tf, &fval);

  f->properties = NULL;
  f->scope = NULL;
  f->attributes = 0;
  /* TODO(mkm): lazily create these properties on first access */
  proto = v7_create_object(v7);
#ifndef V7_DISABLE_PREDEFINED_STRINGS
  v7_set_property_v(v7, proto,
                    v7->predefined_strings[PREDEFINED_STR_CONSTRUCTOR],
                    V7_PROPERTY_DONT_ENUM, fval);
  v7_set_property_v(v7, fval, v7->predefined_strings[PREDEFINED_STR_PROTOTYPE],
                    V7_PROPERTY_DONT_ENUM | V7_PROPERTY_DONT_DELETE, proto);
#else
  v7_set_property(v7, proto, "constructor", 11, V7_PROPERTY_DONT_ENUM, fval);
  v7_set_property(v7, fval, "prototype", 9,
                  V7_PROPERTY_DONT_ENUM | V7_PROPERTY_DONT_DELETE, proto);
#endif

cleanup:
  tmp_frame_cleanup(&tf);
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

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

V7_PRIVATE int to_str(struct v7 *v7, val_t v, char *buf, size_t size,
                      int as_json) {
  char *vp;
  double num;
  for (vp = v7->json_visited_stack.buf;
       vp < v7->json_visited_stack.buf + v7->json_visited_stack.len;
       vp += sizeof(val_t)) {
    if (*(val_t *) vp == v) {
      strncpy(buf, "[Circular]", size);
      return MIN(10, size);
    }
  }

  switch (val_type(v7, v)) {
    case V7_TYPE_NULL:
      strncpy(buf, "null", size);
      return MIN(4, size);
    case V7_TYPE_UNDEFINED:
      strncpy(buf, "undefined", size);
      return MIN(9, size);
    case V7_TYPE_BOOLEAN:
      if (v7_to_boolean(v)) {
        strncpy(buf, "true", size);
        return MIN(4, size);
      } else {
        strncpy(buf, "false", size);
        return MIN(5, size);
      }
    case V7_TYPE_NUMBER:
      if (v == V7_TAG_NAN) {
        return v_sprintf_s(buf, size, "NaN");
      }
      num = v7_to_double(v);
      if (isinf(num)) {
        return v_sprintf_s(buf, size, "%sInfinity", num < 0.0 ? "-" : "");
      }
      {
        const char *fmt = num > 1e10 ? "%.21g" : "%.10g";
        return v_sprintf_s(buf, size, fmt, num);
      }
    case V7_TYPE_STRING: {
      size_t n;
      const char *str = v7_to_string(v7, &v, &n);
      if (as_json) {
        return v_sprintf_s(buf, size, "\"%.*s\"", (int) n, str);
      } else {
        return v_sprintf_s(buf, size, "%.*s", (int) n, str);
      }
    }
    case V7_TYPE_REGEXP_OBJECT: {
      size_t n1, n2 = 0;
      char s2[3] = {0};
      struct v7_regexp *rp = (struct v7_regexp *) v7_to_pointer(v);
      const char *s1 = v7_to_string(v7, &rp->regexp_string, &n1);
      int flags = slre_get_flags(rp->compiled_regexp);
      if (flags & SLRE_FLAG_G) s2[n2++] = 'g';
      if (flags & SLRE_FLAG_I) s2[n2++] = 'i';
      if (flags & SLRE_FLAG_M) s2[n2++] = 'm';
      return v_sprintf_s(buf, size, "/%.*s/%.*s", (int) n1, s1, (int) n2, s2);
    }
    case V7_TYPE_CFUNCTION:
#ifdef V7_UNIT_TEST
      return v_sprintf_s(buf, size, "cfunc_xxxxxx", v7_to_pointer(v));
#else
      return v_sprintf_s(buf, size, "cfunc_%p", v7_to_pointer(v));
#endif
    case V7_TYPE_CFUNCTION_OBJECT:
      v = i_value_of(v7, v);
      return v_sprintf_s(buf, size, "Function cfunc_%p", v7_to_pointer(v));
    case V7_TYPE_GENERIC_OBJECT:
    case V7_TYPE_BOOLEAN_OBJECT:
    case V7_TYPE_STRING_OBJECT:
    case V7_TYPE_NUMBER_OBJECT:
    case V7_TYPE_DATE_OBJECT:
    case V7_TYPE_ERROR_OBJECT: {
      char *b = buf;
      struct v7_property *p;
      mbuf_append(&v7->json_visited_stack, (char *) &v, sizeof(v));
      b += v_sprintf_s(b, size - (b - buf), "{");
      for (p = v7_to_object(v)->properties; p && (size - (b - buf));
           p = p->next) {
        size_t n;
        const char *s;
        if (p->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)) {
          continue;
        }
        s = v7_to_string(v7, &p->name, &n);
        b += v_sprintf_s(b, size - (b - buf), "\"%.*s\":", (int) n, s);
        b += to_str(v7, p->value, b, size - (b - buf), 1);
        if (p->next) {
          b += v_sprintf_s(b, size - (b - buf), ",");
        }
      }
      b += v_sprintf_s(b, size - (b - buf), "}");
      v7->json_visited_stack.len -= sizeof(v);
      return b - buf;
    }
    case V7_TYPE_ARRAY_OBJECT: {
      val_t el;
      int has;
      char *b = buf;
      size_t i, len = v7_array_length(v7, v);
      mbuf_append(&v7->json_visited_stack, (char *) &v, sizeof(v));
      if (as_json) {
        b += v_sprintf_s(b, size - (b - buf), "[");
      }
      for (i = 0; i < len; i++) {
        el = v7_array_get2(v7, v, i, &has);
        if (has) {
          b += to_str(v7, el, b, size - (b - buf), 1);
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
    case V7_TYPE_FUNCTION_OBJECT: {
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
      /* TODO(mkm) cleanup this - 1 */
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

            b += v_sprintf_s(b, size - (b - buf), "%.*s", (int) name_len, name);
            if (var < var_end || next) {
              b += v_sprintf_s(b, size - (b - buf), ",");
            }
          }
          if (next > 0) {
            var = next - 1; /* TODO(mkm): cleanup */
          }
        } while (next != 0);
        b += v_sprintf_s(b, size - (b - buf), "}");
      }
      b += v_sprintf_s(b, size - (b - buf), "]");
      return b - buf;
    }
    case V7_TYPE_FOREIGN:
      return v_sprintf_s(buf, size, "[foreign]");
    default:
      printf("NOT IMPLEMENTED YET %d\n", val_type(v7, v)); /* LCOV_EXCL_LINE */
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

int v7_stringify_value(struct v7 *v7, val_t v, char *buf, size_t size) {
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
  p->name = v7_create_undefined();
  p->value = v7_create_undefined();
  p->attributes = 0;
  return p;
}

V7_PRIVATE struct v7_property *v7_get_own_property2(struct v7 *v7, val_t obj,
                                                    const char *name,
                                                    size_t len,
                                                    unsigned int attrs) {
  struct v7_property *p;
  struct v7_object *o;
  val_t ss;
  if (!v7_is_object(obj)) {
    return NULL;
  }
  if (len == (size_t) ~0) {
    len = strlen(name);
  }

  o = v7_to_object(obj);
  /*
   * len check is needed to allow getting the mbuf from the hidden property.
   * TODO(mkm): however hidden properties cannot be safely represented with
   * a zero length string anyway, so this will change.
   */
  if (o->attributes & V7_OBJ_DENSE_ARRAY && len > 0) {
    int ok, has;
    unsigned long i = cstr_to_ulong(name, len, &ok);
    if (ok) {
      v7->cur_dense_prop->value = v7_array_get2(v7, obj, i, &has);
      return has ? v7->cur_dense_prop : NULL;
    }
  }

  if (len <= 5) {
    ss = v7_create_string(v7, name, len, 1);
    for (p = o->properties; p != NULL; p = p->next) {
      if (p->name == ss && (attrs == 0 || (p->attributes & attrs))) {
        return p;
      }
    }
  } else {
    for (p = o->properties; p != NULL; p = p->next) {
      size_t n;
      const char *s = v7_to_string(v7, &p->name, &n);
      if (n == len && strncmp(s, name, len) == 0 &&
          (attrs == 0 || (p->attributes & attrs))) {
        return p;
      }
    }
  }
  return NULL;
}

V7_PRIVATE struct v7_property *v7_get_own_property(struct v7 *v7, val_t obj,
                                                   const char *name,
                                                   size_t len) {
  return v7_get_own_property2(v7, obj, name, len, 0);
}

struct v7_property *v7_get_property(struct v7 *v7, val_t obj, const char *name,
                                    size_t len) {
  if (!v7_is_object(obj)) {
    return NULL;
  }
  for (; obj != V7_NULL; obj = v_get_prototype(v7, obj)) {
    struct v7_property *prop;
    if ((prop = v7_get_own_property(v7, obj, name, len)) != NULL) {
      return prop;
    }
  }
  return NULL;
}

v7_val_t v7_get(struct v7 *v7, val_t obj, const char *name, size_t name_len) {
  val_t v = obj;
  if (v7_is_string(obj)) {
    v = v7->string_prototype;
  } else if (v7_is_regexp(obj)) {
    v = v7->regexp_prototype;
  } else if (v7_is_double(obj)) {
    v = v7->number_prototype;
  } else if (v7_is_boolean(obj)) {
    v = v7->boolean_prototype;
  } else if (v7_is_undefined(obj)) {
    throw_exception(v7, TYPE_ERROR, "cannot read property '%.*s' of undefined",
                    (int) name_len, name);
  } else if (v7_is_cfunction(obj)) {
    return V7_UNDEFINED;
  }
  return v7_property_value(v7, obj, v7_get_property(v7, v, name, name_len));
}

V7_PRIVATE void v7_destroy_property(struct v7_property **p) {
  *p = NULL;
}

int v7_set_v(struct v7 *v7, val_t obj, val_t name, val_t val) {
  size_t len;
  const char *n = v7_to_string(v7, &name, &len);
  struct v7_property *p = v7_get_own_property(v7, obj, n, len);
  if (p == NULL || !(p->attributes & V7_PROPERTY_READ_ONLY)) {
    return v7_set_property_v(v7, obj, name, p == NULL ? 0 : p->attributes, val);
  }
  return -1;
}

int v7_set(struct v7 *v7, val_t obj, const char *name, size_t len, val_t val) {
  struct v7_property *p = v7_get_own_property(v7, obj, name, len);
  if (p == NULL || !(p->attributes & V7_PROPERTY_READ_ONLY)) {
    return v7_set_property(v7, obj, name, len, p == NULL ? 0 : p->attributes,
                           val);
  }
  return -1;
}

V7_PRIVATE void v7_invoke_setter(struct v7 *v7, struct v7_property *prop,
                                 val_t obj, val_t val) {
  val_t setter = prop->value, args = v7_create_dense_array(v7);
  if (prop->attributes & V7_PROPERTY_GETTER) {
    setter = v7_array_get(v7, prop->value, 1);
  }
  v7_array_set(v7, args, 0, val);
  v7_apply(v7, setter, obj, args);
}

V7_PRIVATE struct v7_property *v7_set_prop(struct v7 *v7, val_t obj, val_t name,
                                           unsigned int attributes, val_t val) {
  struct v7_property *prop;
  size_t len;
  const char *n = v7_to_string(v7, &name, &len);

  if (!v7_is_object(obj)) {
    return NULL;
  }

  if (v7_to_object(obj)->attributes & V7_OBJ_NOT_EXTENSIBLE) {
    if (v7->strict_mode) {
      throw_exception(v7, TYPE_ERROR, "Object is not extensible");
    }
    return NULL;
  }

  prop = v7_get_own_property(v7, obj, n, len);
  if (prop == NULL) {
    if ((prop = v7_create_property(v7)) == NULL) {
      return NULL; /* LCOV_EXCL_LINE */
    }
    prop->next = v7_to_object(obj)->properties;
    v7_to_object(obj)->properties = prop;
  }

  if (v7_is_undefined(prop->name)) {
    prop->name = name;
  }
  if (prop->attributes & V7_PROPERTY_SETTER) {
    v7_invoke_setter(v7, prop, obj, val);
    return 0;
  }

  prop->value = val;
  prop->attributes = attributes;
  return prop;
}

int v7_set_property_v(struct v7 *v7, val_t obj, val_t name,
                      unsigned int attributes, v7_val_t val) {
  struct v7_property *prop = v7_set_prop(v7, obj, name, attributes, val);
  return prop == NULL ? -1 : 0;
}

int v7_set_property(struct v7 *v7, val_t obj, const char *name, size_t len,
                    unsigned int attributes, v7_val_t val) {
  val_t n;
  if (len == (size_t) ~0) {
    len = strlen(name);
  }

  n = v7_create_string(v7, name, len, 1);
  return v7_set_property_v(v7, obj, n, attributes, val);
}

int v7_del_property(struct v7 *v7, val_t obj, const char *name, size_t len) {
  struct v7_property *prop, *prev;

  if (!v7_is_object(obj)) {
    return -1;
  }
  if (len == (size_t) ~0) {
    len = strlen(name);
  }
  for (prev = NULL, prop = v7_to_object(obj)->properties; prop != NULL;
       prev = prop, prop = prop->next) {
    size_t n;
    const char *s = v7_to_string(v7, &prop->name, &n);
    if (n == len && strncmp(s, name, len) == 0) {
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

V7_PRIVATE v7_val_t
v7_create_cfunction_object(struct v7 *v7, v7_cfunction_t f, int num_args) {
  val_t obj = create_object(v7, v7->function_prototype);
  struct gc_tmp_frame tf = new_tmp_frame(v7);
  tmp_stack_push(&tf, &obj);
  v7_set_property(v7, obj, "", 0, V7_PROPERTY_HIDDEN, v7_create_cfunction(f));
#ifndef V7_DISABLE_PREDEFINED_STRINGS
  v7_set_property_v(
      v7, obj, v7->predefined_strings[PREDEFINED_STR_LENGTH],
      V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_ENUM | V7_PROPERTY_DONT_DELETE,
      v7_create_number(num_args));
#else
  v7_set_property(
      v7, obj, "length", 6,
      V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_ENUM | V7_PROPERTY_DONT_DELETE,
      v7_create_number(num_args));
#endif
  tmp_frame_cleanup(&tf);
  return obj;
}

V7_PRIVATE v7_val_t v7_create_cfunction_ctor(struct v7 *v7, val_t proto,
                                             v7_cfunction_t f, int num_args) {
  val_t res = v7_create_cfunction_object(v7, f, num_args);

#ifndef V7_DISABLE_PREDEFINED_STRINGS
  v7_set_property_v(
      v7, res, v7->predefined_strings[PREDEFINED_STR_PROTOTYPE],
      V7_PROPERTY_DONT_ENUM | V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_DELETE,
      proto);

  v7_set_property_v(v7, proto,
                    v7->predefined_strings[PREDEFINED_STR_CONSTRUCTOR],
                    V7_PROPERTY_DONT_ENUM, res);
#else
  v7_set_property(
      v7, res, "prototype", 9,
      V7_PROPERTY_DONT_ENUM | V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_DELETE,
      proto);
  v7_set_property(v7, proto, "constructor", 11, V7_PROPERTY_DONT_ENUM, res);
#endif
  return res;
}

V7_PRIVATE int set_cfunc_obj_prop(struct v7 *v7, val_t o, const char *name,
                                  v7_cfunction_t f, int num_args) {
  return v7_set_property(v7, o, name, strlen(name), V7_PROPERTY_DONT_ENUM,
                         v7_create_cfunction_object(v7, f, num_args));
}

V7_PRIVATE int set_cfunc_prop(struct v7 *v7, val_t o, const char *name,
                              v7_cfunction_t f) {
  return v7_set_property(v7, o, name, strlen(name), 0, v7_create_cfunction(f));
}

V7_PRIVATE val_t
v7_property_value(struct v7 *v7, val_t obj, struct v7_property *p) {
  if (p == NULL) {
    return V7_UNDEFINED;
  }
  if (p->attributes & V7_PROPERTY_GETTER) {
    val_t getter = p->value;
    if (p->attributes & V7_PROPERTY_SETTER) {
      getter = v7_array_get(v7, p->value, 0);
    }
    return v7_apply(v7, getter, obj, V7_UNDEFINED);
  }
  return p->value;
}

unsigned long v7_array_length(struct v7 *v7, val_t v) {
  struct v7_property *p;
  unsigned long key, len = 0;
  char *end;

  if (!v7_is_object(v)) {
    return 0;
  }

  if (v7_to_object(v)->attributes & V7_OBJ_DENSE_ARRAY) {
    struct v7_property *p =
        v7_get_own_property2(v7, v, "", 0, V7_PROPERTY_HIDDEN);
    struct mbuf *abuf;
    if (p == NULL) return 0;
    abuf = (struct mbuf *) v7_to_foreign(p->value);
    if (abuf == NULL) return 0;
    return abuf->len / sizeof(val_t);
  }

  for (p = v7_to_object(v)->properties; p != NULL; p = p->next) {
    size_t n;
    const char *s = v7_to_string(v7, &p->name, &n);
    key = strtoul(s, &end, 10);
    /* Array length could not be more then 2^32 */
    if (end > s && *end == '\0' && key >= len && key < 4294967295UL) {
      len = key + 1;
    }
  }

  return len;
}

int v7_array_set(struct v7 *v7, val_t arr, unsigned long index, val_t v) {
  int res = -1;
  if (v7_is_object(arr)) {
    if (v7_to_object(arr)->attributes & V7_OBJ_DENSE_ARRAY) {
      struct v7_property *p =
          v7_get_own_property2(v7, arr, "", 0, V7_PROPERTY_HIDDEN);
      struct mbuf *abuf;
      unsigned long len;
      assert(p != NULL);
      abuf = (struct mbuf *) v7_to_foreign(p->value);

      if (v7_to_object(arr)->attributes & V7_OBJ_NOT_EXTENSIBLE) {
        if (v7->strict_mode) {
          throw_exception(v7, TYPE_ERROR, "Object is not extensible");
        }
        return res;
      }

      if (abuf == NULL) {
        abuf = (struct mbuf *) malloc(sizeof(*abuf));
        mbuf_init(abuf, sizeof(val_t) * (index + 1));
        p->value = v7_create_foreign(abuf);
      }
      len = abuf->len / sizeof(val_t);
      /* TODO(mkm): possibly promote to sparse array */
      if (index > len) {
        unsigned long i;
        val_t s = V7_TAG_NOVALUE;
        for (i = len; i < index; i++) {
          mbuf_append(abuf, (char *) &s, sizeof(val_t));
        }
        len = index;
      }

      if (index == len) {
        mbuf_append(abuf, (char *) &v, sizeof(val_t));
      } else {
        memcpy(abuf->buf + index * sizeof(val_t), &v, sizeof(val_t));
      }
    } else {
      char buf[20];
      int n = v_sprintf_s(buf, sizeof(buf), "%lu", index);
      res = v7_set(v7, arr, buf, n, v);
    }
  }
  return res;
}

int v7_array_push(struct v7 *v7, v7_val_t arr, v7_val_t v) {
  return v7_array_set(v7, arr, v7_array_length(v7, arr), v);
}

val_t v7_array_get(struct v7 *v7, val_t arr, unsigned long index) {
  return v7_array_get2(v7, arr, index, NULL);
}

val_t v7_array_get2(struct v7 *v7, val_t arr, unsigned long index, int *has) {
  if (v7_is_object(arr)) {
    if (v7_to_object(arr)->attributes & V7_OBJ_DENSE_ARRAY) {
      struct v7_property *p =
          v7_get_own_property2(v7, arr, "", 0, V7_PROPERTY_HIDDEN);
      struct mbuf *abuf = NULL;
      unsigned long len;
      if (p != NULL) {
        abuf = (struct mbuf *) v7_to_foreign(p->value);
      }
      if (abuf == NULL) {
        if (has != NULL) {
          *has = 0;
        }
        return v7_create_undefined();
      }
      len = abuf->len / sizeof(val_t);
      if (index >= len) {
        return v7_create_undefined();
      } else {
        val_t res;
        memcpy(&res, abuf->buf + index * sizeof(val_t), sizeof(val_t));
        if (has != NULL) {
          *has = res != V7_TAG_NOVALUE;
        }
        if (res == V7_TAG_NOVALUE) {
          res = v7_create_undefined();
        }
        return res;
      }
    } else {
      struct v7_property *p;
      char buf[20];
      int n = v_sprintf_s(buf, sizeof(buf), "%lu", index);
      p = v7_get_property(v7, arr, buf, n);
      if (has != NULL) {
        *has = (p != NULL);
      }
      return v7_property_value(v7, arr, p);
    }
  } else {
    return v7_create_undefined();
  }
}

int nextesc(const char **p); /* from SLRE */
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
        default: {
          const char *tmp_s = s;
          int i = nextesc(&s);
          switch (i) {
            case -SLRE_INVALID_ESC_CHAR:
              r = '\\';
              s = tmp_s;
              n += runetochar(to == NULL ? tmp : to + n, &r);
              s += chartorune(&r, s);
              break;
            case -SLRE_INVALID_HEX_DIGIT:
            default:
              r = i;
          }
        }
      }
    }
    n += runetochar(to == NULL ? tmp : to + n, &r);
  }

  return n;
}

/* Insert a string into mbuf at specified offset */
V7_PRIVATE void embed_string(struct mbuf *m, size_t offset, const char *p,
                             size_t len, int zero_term, int unesc) {
  char *old_base = m->buf;
  int p_backed_by_mbuf = p >= old_base && p < old_base + m->len;
  size_t n = unesc ? unescape(p, len, NULL) : len;
  int k = calc_llen(n); /* Calculate how many bytes length takes */
  size_t tot_len = k + n + zero_term;
  mbuf_insert(m, offset, NULL, tot_len); /* Allocate  buffer */
  /* Fixup p if it was relocated by mbuf_insert() above */
  if (p_backed_by_mbuf) {
    p += m->buf - old_base;
  }
  encode_varint(n, (unsigned char *) m->buf + offset); /* Write length */
  /* Write string */
  if (unesc) {
    unescape(p, len, m->buf + offset + k);
  } else {
    memcpy(m->buf + offset + k, p, len);
  }
  if (zero_term) {
    m->buf[offset + tot_len - 1] = '\0';
  }
}

/* Create a string */
v7_val_t v7_create_string(struct v7 *v7, const char *p, size_t len, int own) {
  struct mbuf *m = own ? &v7->owned_strings : &v7->foreign_strings;
  val_t offset = m->len, tag = V7_TAG_STRING_F;

  if (len <= 4) {
    char *s = GET_VAL_NAN_PAYLOAD(offset) + 1;
    offset = 0;
    memcpy(s, p, len);
    s[-1] = len;
    tag = V7_TAG_STRING_I;
  } else if (len == 5) {
    char *s = GET_VAL_NAN_PAYLOAD(offset);
    offset = 0;
    memcpy(s, p, len);
    tag = V7_TAG_STRING_5;
  } else if (own) {
    embed_string(m, m->len, p, len, 1, 0);
    tag = V7_TAG_STRING_O;
  } else {
    /* TODO(mkm): this doesn't set correctly the foreign string length */
    embed_string(m, m->len, (char *) &p, sizeof(p), 0, 0);
  }

  /* NOTE(lsm): don't use v7_pointer_to_value, 32-bit ptrs will truncate */
  return (offset & ~V7_TAG_MASK) | tag;
}

V7_PRIVATE val_t to_string(struct v7 *v7, val_t v) {
  char buf[100], *p, *s;
  val_t res;
  if (v7_is_string(v)) {
    return v;
  }

  s = p = v7_to_json(v7, i_value_of(v7, v), buf, sizeof(buf));
  if (p[0] == '"') {
    p[strlen(p) - 1] = '\0';
    s++;
  }
  res = v7_create_string(v7, s, strlen(s), 1);
  if (p != buf) {
    free(p);
  }

  return res;
}

/*
 * Get a pointer to string and string length.
 *
 * Beware that V7 strings are not null terminated!
 */
const char *v7_to_string(struct v7 *v7, val_t *v, size_t *sizep) {
  uint64_t tag = v[0] & V7_TAG_MASK;
  char *p;
  int llen;

  if (tag == V7_TAG_STRING_I) {
    p = GET_VAL_NAN_PAYLOAD(*v) + 1;
    *sizep = p[-1];
  } else if (tag == V7_TAG_STRING_5) {
    p = GET_VAL_NAN_PAYLOAD(*v);
    *sizep = 5;
  } else {
    struct mbuf *m =
        (tag == V7_TAG_STRING_O) ? &v7->owned_strings : &v7->foreign_strings;
    size_t offset = (size_t) v7_to_pointer(*v);
    char *s = m->buf + offset;

    *sizep = decode_varint((uint8_t *) s, &llen);
    if (tag == V7_TAG_STRING_O) {
      p = s + llen;
    } else {
      memcpy(&p, s + llen, sizeof(p));
    }
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
  if (a_len > b_len) {
    return 1;
  } else if (a_len < b_len) {
    return -1;
  } else {
    return 0;
  }
}

V7_PRIVATE val_t s_concat(struct v7 *v7, val_t a, val_t b) {
  size_t a_len, b_len;
  const char *a_ptr, *b_ptr;
  char *s = NULL;
  uint64_t tag = V7_TAG_STRING_F;
  val_t offset = v7->owned_strings.len;

  a_ptr = v7_to_string(v7, &a, &a_len);
  b_ptr = v7_to_string(v7, &b, &b_len);

  /* Create a new string which is a concatenation a + b */
  if (a_len + b_len <= 4) {
    offset = 0;
    /* TODO(mkm): make it work on big endian too */
    s = GET_VAL_NAN_PAYLOAD(offset) + 1;
    s[-1] = a_len + b_len;
    tag = V7_TAG_STRING_I;
  } else if (a_len + b_len == 5) {
    offset = 0;
    /* TODO(mkm): make it work on big endian too */
    s = GET_VAL_NAN_PAYLOAD(offset);
    tag = V7_TAG_STRING_5;
  } else {
    int llen = calc_llen(a_len + b_len);
    mbuf_append(&v7->owned_strings, NULL, a_len + b_len + llen + 1);
    /* all pointers might have been relocated */
    s = v7->owned_strings.buf + offset;
    encode_varint(a_len + b_len, (unsigned char *) s); /* Write length */
    s += llen;
    a_ptr = v7_to_string(v7, &a, &a_len);
    b_ptr = v7_to_string(v7, &b, &b_len);
    tag = V7_TAG_STRING_O;
  }
  memcpy(s, a_ptr, a_len);
  memcpy(s + a_len, b_ptr, b_len);
  /* Inlined strings are already 0-terminated, but still, why not. */
  s[a_len + b_len] = '\0';

  /* NOTE(lsm): don't use v7_pointer_to_value, 32-bit ptrs will truncate */
  return (offset & ~V7_TAG_MASK) | tag;
}

/* Create V7 strings for integers such as array indices */
V7_PRIVATE val_t ulong_to_str(struct v7 *v7, unsigned long n) {
  char buf[100];
  int len;
  len = snprintf(buf, sizeof(buf), "%lu", n);
  return v7_create_string(v7, buf, len, 1);
}

/*
 * Convert a V7 string to to an unsigned integer.
 * `ok` will be set to true if the string conforms to
 * an unsigned long.
 */
V7_PRIVATE unsigned long cstr_to_ulong(const char *s, size_t len, int *ok) {
  char *e;
  unsigned long res = strtoul(s, &e, 10);
  *ok = (e == s + len);
  return res;
}

/*
 * Convert a C string to to an unsigned integer.
 * `ok` will be set to true if the string conforms to
 * an unsigned long.
 */
V7_PRIVATE unsigned long str_to_ulong(struct v7 *v7, val_t v, int *ok) {
  char buf[100];
  size_t len = v7_stringify_value(v7, v, buf, sizeof(buf));
  return cstr_to_ulong(buf, len, ok);
}

V7_PRIVATE int is_prototype_of(struct v7 *v7, val_t o, val_t p) {
  struct v7_object *obj, *proto;
  if (!v7_is_object(o) || !v7_is_object(p)) {
    return 0;
  }
  if (v7_is_function(o)) {
    return p == v7->function_prototype ||
           is_prototype_of(v7, v7->function_prototype, p);
  }
  proto = v7_to_object(p);
  for (obj = v7_to_object(o); obj; obj = obj->prototype) {
    if (obj->prototype == proto) {
      return 1;
    }
  }
  return 0;
}

int v7_is_true(struct v7 *v7, val_t v) {
  size_t len;
  return ((v7_is_boolean(v) && v7_to_boolean(v)) ||
          (v7_is_double(v) && v7_to_double(v) != 0.0) ||
          (v7_is_string(v) && v7_to_string(v7, &v, &len) && len > 0) ||
          (v7_is_object(v))) &&
         v != V7_TAG_NAN;
}

static void object_destructor(struct v7 *v7, void *ptr) {
  struct v7_object *o = (struct v7_object *) ptr;
  struct v7_property *p;
  struct mbuf *abuf;
  if (o->attributes & V7_OBJ_DENSE_ARRAY) {
    p = v7_get_own_property2(v7, v7_object_to_value(o), "", 0,
                             V7_PROPERTY_HIDDEN);
    if (p != NULL &&
        ((abuf = (struct mbuf *) v7_to_foreign(p->value)) != NULL)) {
      mbuf_free(abuf);
    }
  }
}

struct v7 *v7_create(void) {
  struct v7 *v7 = NULL;
  val_t *p;
  char z = 0;

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) != NULL) {
    v7->cur_dense_prop =
        (struct v7_property *) calloc(1, sizeof(struct v7_property));
#define GC_SIZE (64 * 10)
    gc_arena_init(v7, &v7->object_arena, sizeof(struct v7_object), GC_SIZE,
                  "object");
    v7->object_arena.destructor = object_destructor;
    gc_arena_init(v7, &v7->function_arena, sizeof(struct v7_function), GC_SIZE,
                  "function");
    gc_arena_init(v7, &v7->property_arena, sizeof(struct v7_property),
                  GC_SIZE * 3, "property");

    /*
     * The compacting GC exploits the null terminator of the previous
     * string as marker.
     */
    mbuf_append(&v7->owned_strings, &z, 1);

    p = v7->predefined_strings;
    p[PREDEFINED_STR_LENGTH] = v7_create_string(v7, "length", 6, 1);
    p[PREDEFINED_STR_PROTOTYPE] = v7_create_string(v7, "prototype", 9, 1);
    p[PREDEFINED_STR_CONSTRUCTOR] = v7_create_string(v7, "constructor", 11, 1);
    p[PREDEFINED_STR_ARGUMENTS] = v7_create_string(v7, "arguments", 9, 1);

    init_stdlib(v7);
    v7->thrown_error = v7_create_undefined();
  }

  return v7;
}

val_t v7_get_global_object(struct v7 *v7) {
  return v7->global_object;
}

void v7_destroy(struct v7 *v7) {
  struct ast **a;
  if (v7 != NULL) {
    mbuf_free(&v7->owned_strings);
    mbuf_free(&v7->foreign_strings);
    mbuf_free(&v7->json_visited_stack);
    mbuf_free(&v7->tmp_stack);

    for (a = (struct ast **) v7->allocated_asts.buf;
         (char *) a < v7->allocated_asts.buf + v7->allocated_asts.len; a++) {
      ast_free(*a);
      free(*a);
    }
    mbuf_free(&v7->allocated_asts);

    gc_arena_destroy(v7, &v7->object_arena);
    gc_arena_destroy(v7, &v7->function_arena);
    gc_arena_destroy(v7, &v7->property_arena);

    free(v7);
  }
}
