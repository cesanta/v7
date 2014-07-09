// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013-2014 Cesanta Software Limited
// All rights reserved
//
// This software is dual-licensed: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation. For the terms of this
// license, see <http://www.gnu.org/licenses/>.
//
// You are free to use this software under the terms of the GNU General
// Public License, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// Alternatively, you can license this software under a commercial
// license, as set out in <http://cesanta.com/products.html>.

#include "v7.h"

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#endif

// MSVC6 doesn't have standard C math constants defined
#ifndef M_PI
#define M_E         2.71828182845904523536028747135266250
#define M_LOG2E     1.44269504088896340735992468100189214
#define M_LOG10E    0.434294481903251827651128918916605082
#define M_LN2       0.693147180559945309417232121458176568
#define M_LN10      2.30258509299404568401799145468436421
#define M_PI        3.14159265358979323846264338327950288
#define M_SQRT2     1.41421356237309504880168872420969808
#define M_SQRT1_2   0.707106781186547524400844362104849039
#define NAN         atof("NAN")
#define INFINITY    atof("INFINITY")  // TODO: fix this
#endif

#define V7_CACHE_OBJS
#define MAX_STRING_LITERAL_LENGTH 500
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define CHECK(cond, code) do { if (!(cond)) return (code); } while (0)
#define TRY(call) do { enum v7_err e = call; CHECK(e == V7_OK, e); } while (0)
#define TRACE_OBJ(O) do { char x[4000]; printf("==> %s [%s]\n", __func__, \
  O == NULL ? "@" : v7_to_string(O, x, sizeof(x))); } while (0)
#define RO_OBJ(t) {0,0,(t),0,V7_RDONLY_VAL|V7_RDONLY_STR|V7_RDONLY_PROP,{0}}
#define SET_RO_PROP_V(obj, name, val)                   \
  do {                                                  \
    static struct v7_val key = RO_OBJ(V7_STR);          \
    static struct v7_prop prop = {NULL, &key, &val};    \
    key.v.str.buf = (char *) (name);                    \
    key.v.str.len = strlen(key.v.str.buf);              \
    prop.next = obj.v.props;                            \
    obj.v.props = &prop;                                \
  } while (0)
#define SET_RO_PROP(obj, name, type, attr, initializer) \
  do {                                                  \
    static struct v7_val val = RO_OBJ(type);            \
    val.v.attr = (initializer);                         \
    SET_RO_PROP_V(obj, name, val);                      \
  } while (0)

// Possible values for struct v7_val::flags 
enum { V7_RDONLY_VAL = 1, V7_RDONLY_STR = 2, V7_RDONLY_PROP = 4 };

// Forward declarations
static enum v7_err parse_expression(struct v7 *);
static enum v7_err parse_statement(struct v7 *, int *is_return);

// Static variables
static struct v7_val s_object = RO_OBJ(V7_OBJ);
static struct v7_val s_string = RO_OBJ(V7_OBJ);
static struct v7_val s_math = RO_OBJ(V7_OBJ);
static struct v7_val s_number = RO_OBJ(V7_OBJ);
static struct v7_val s_array = RO_OBJ(V7_OBJ);
static struct v7_val s_stdlib = RO_OBJ(V7_OBJ);

static char *v7_strdup(const char *ptr, unsigned long len) {
  char *p = (char *) malloc(len + 1);
  if (p == NULL) return NULL;
  memcpy(p, ptr, len);
  p[len] = '\0';
  return p;
}

static void Obj_toString(struct v7 *v7, struct v7_val *this_obj,
                         struct v7_val *result, struct v7_val **args,
                         int num_args) {
  char buf[4000];
  (void) v7; (void) args; (void) num_args;
  v7_to_string(this_obj, buf, sizeof(buf));
  result->type = V7_STR;
  result->v.str.len = strlen(buf);
  result->v.str.buf = v7_strdup(buf, result->v.str.len);
}

static void Str_length(struct v7_val *this_obj, struct v7_val *result) {
  result->type = V7_NUM;
  result->v.num = this_obj->v.str.len;
}

static void Str_charCodeAt(struct v7 *v7, struct v7_val *this_obj,
                           struct v7_val *result, struct v7_val **args,
                           int num_args) {
  double idx = num_args > 0 && args[0]->type == V7_NUM ? args[0]->v.num : NAN;
  const struct v7_str *str = &this_obj->v.str;

  (void) v7;
  result->type = V7_NUM;
  result->v.num = NAN;

  if (!isnan(idx) && this_obj->type == V7_STR && fabs(idx) < str->len) {
    result->v.num = ((unsigned char *) str->buf)[(int) idx];
  }
}

static void Str_charAt(struct v7 *v7, struct v7_val *this_obj,
                       struct v7_val *result, struct v7_val **args,
                       int num_args) {
  double idx = num_args > 0 && args[0]->type == V7_NUM ? args[0]->v.num : NAN;
  const struct v7_str *str = &this_obj->v.str;

  (void) v7;
  result->type = V7_UNDEF;

  if (!isnan(idx) && this_obj->type == V7_STR && fabs(idx) < str->len) {
    result->type = V7_STR;
    result->v.str.len = 1;
    result->v.str.buf = v7_strdup("x", 1);
    result->v.str.buf[0] = ((unsigned char *) str->buf)[(int) idx];
  }
}

static void Str_indexOf(struct v7 *v7, struct v7_val *this_obj,
                        struct v7_val *result, struct v7_val **args,
                        int num_args) {
  (void) v7; (void) this_obj; (void) result; (void) num_args;

  result->type = V7_NUM;
  result->v.num = -1.0;

  if (this_obj->type == V7_STR && num_args > 0 && args[0]->type == V7_STR) {
    int i = num_args > 1 && args[1]->type == V7_NUM ? (int) args[1]->v.num : 0;
    const struct v7_str *a = &this_obj->v.str, *b = &args[0]->v.str;

    // Scan the string, advancing one byte at a time
    for (; i >= 0 && a->len >= b->len && i <= (int) (a->len - b->len); i++) {
      if (memcmp(a->buf + i, b->buf, b->len) == 0) {
        result->v.num = i;
        break;
      }
    }
  }
}

static void Str_substr(struct v7 *v7, struct v7_val *this_obj,
                        struct v7_val *result, struct v7_val **args,
                        int num_args) {
  (void) v7; (void) this_obj; (void) result; (void) num_args; (void) args;

  result->type = V7_STR;
  result->v.str.buf = (char *) "";
  result->v.str.len = 0;
  result->flags = V7_RDONLY_STR;

  if (num_args > 0 && args[0]->type == V7_NUM) {
    long start = (long) args[0]->v.num;
    if (start < 0) {
      start += (long) this_obj->v.str.len;
    }
    if (start >= 0 && start < (long) this_obj->v.str.len) {
      long n = this_obj->v.str.len - start;
      if (num_args > 1 && args[1]->type == V7_NUM) {
        n = args[1]->v.num;
      }
      if (n > 0 && n <= ((long) this_obj->v.str.len - start)) {
        result->v.str.len = n;
        result->v.str.buf = v7_strdup(this_obj->v.str.buf + start, n);
        result->flags = 0;
      }
    }
  }
}

static void Arr_length(struct v7_val *this_obj, struct v7_val *result) {
  struct v7_prop *p;
  result->type = V7_NUM;
  result->v.num = 0.0;
  for (p = this_obj->v.props; p != NULL; p = p->next) {
    result->v.num += 1.0;
  }
}

static void Std_print(struct v7 *v7, struct v7_val *this_obj,
                      struct v7_val *result,
                      struct v7_val **args, int num_args) {
  char buf[4000];
  int i;

  (void) v7; (void) this_obj; (void) result; (void) num_args;
  for (i = 0; i < num_args; i++) {
    printf("%s", v7_to_string(args[i], buf, sizeof(buf)));
  }
}

static void Std_exit(struct v7 *v7, struct v7_val *this_obj,
                     struct v7_val *result,
                     struct v7_val **args, int num_args) {
  int exit_code = num_args > 0 ? (int) args[0]->v.num : EXIT_SUCCESS;
  (void) v7; (void) this_obj; (void) result; (void) num_args;
  exit(exit_code);
}

static void init_stdlib(void) {
  static int prototypes_initialized;
  if (prototypes_initialized) return;
  prototypes_initialized++;
  s_string.proto = s_number.proto = s_array.proto = &s_object;

  SET_RO_PROP(s_object, "toString", V7_C_FUNC, c_func, Obj_toString);

  SET_RO_PROP(s_number, "MAX_VALUE", V7_NUM, num, LONG_MAX);
  SET_RO_PROP(s_number, "MIN_VALUE", V7_NUM, num, LONG_MIN);
  SET_RO_PROP(s_number, "NaN", V7_NUM, num, NAN);

  SET_RO_PROP(s_array, "length", V7_RO_PROP, prop_func, Arr_length);

  SET_RO_PROP(s_string, "length", V7_RO_PROP, prop_func, Str_length);
  SET_RO_PROP(s_string, "charCodeAt", V7_C_FUNC, c_func, Str_charCodeAt);
  SET_RO_PROP(s_string, "charAt", V7_C_FUNC, c_func, Str_charAt);
  SET_RO_PROP(s_string, "indexOf", V7_C_FUNC, c_func, Str_indexOf);
  SET_RO_PROP(s_string, "substr", V7_C_FUNC, c_func, Str_substr);

  SET_RO_PROP(s_math, "E", V7_NUM, num, M_E);
  SET_RO_PROP(s_math, "PI", V7_NUM, num, M_PI);
  SET_RO_PROP(s_math, "LN2", V7_NUM, num, M_LN2);
  SET_RO_PROP(s_math, "LN10", V7_NUM, num, M_LN10);
  SET_RO_PROP(s_math, "LOG2E", V7_NUM, num, M_LOG2E);
  SET_RO_PROP(s_math, "LOG10E", V7_NUM, num, M_LOG10E);
  SET_RO_PROP(s_math, "SQRT1_2", V7_NUM, num, M_SQRT1_2);
  SET_RO_PROP(s_math, "SQRT2", V7_NUM, num, M_SQRT2);

  SET_RO_PROP(s_stdlib, "print", V7_C_FUNC, c_func, Std_print);
  SET_RO_PROP(s_stdlib, "exit", V7_C_FUNC, c_func, Std_exit);

  SET_RO_PROP_V(s_stdlib, "Object", s_object);
  SET_RO_PROP_V(s_stdlib, "Number", s_number);
  SET_RO_PROP_V(s_stdlib, "Math", s_math);
  SET_RO_PROP_V(s_stdlib, "String", s_string);
  SET_RO_PROP_V(s_stdlib, "Array", s_array);
}

struct v7 *v7_create(void) {
  struct v7 *v7 = NULL;
  size_t i;

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) == NULL) return NULL;

  for (i = 0; i < ARRAY_SIZE(v7->scopes); i++) {
    v7->scopes[i].ref_count = 1;
    v7->scopes[i].type = V7_OBJ;
    v7->scopes[i].flags = V7_RDONLY_VAL;
  }

  init_stdlib();
  v7->scopes[0].proto = &s_stdlib;

  return v7;
}

void v7_freeval(struct v7 *v7, struct v7_val *v) {
  v->ref_count--;
  assert(v->ref_count >= 0);
  if (v->ref_count > 0) return;
  if ((v->type == V7_OBJ || v->type == V7_ARRAY) &&
      !(v->flags & V7_RDONLY_PROP)) {
    struct v7_prop *p, *tmp;
    for (p = v->v.props; p != NULL; p = tmp) {
      tmp = p->next;
      v7_freeval(v7, p->key);
      v7_freeval(v7, p->val);
      p->val = p->key = NULL;
#ifdef V7_CACHE_OBJS
      p->next = v7->free_props;
      v7->free_props = p;
#else
      free(p);
#endif
    }
    v->v.props = NULL;
  } else if (v->type == V7_STR && !(v->flags & V7_RDONLY_STR)) {
    free(v->v.str.buf);
  } else if (v->type == V7_FUNC && !(v->flags & V7_RDONLY_STR)) {
    free(v->v.func);
  }
  if (!(v->flags & V7_RDONLY_VAL)) {
    memset(v, 0, sizeof(*v));
#ifdef V7_CACHE_OBJS
    v->next = v7->free_values;
    v7->free_values = v;
#else
    free(v);
#endif
  }
}

struct v7_val *v7_get_root_namespace(struct v7 *v7) {
  return &v7->scopes[0];
}

static enum v7_err inc_stack(struct v7 *v7, int incr) {
  int i;

  CHECK(v7->sp + incr < (int) ARRAY_SIZE(v7->stack), V7_STACK_OVERFLOW);
  CHECK(v7->sp + incr >= 0, V7_STACK_UNDERFLOW);

  // Free values pushed on stack (like string literals and functions)
  for (i = 0; incr < 0 && i < -incr && i < v7->sp; i++) {
    v7_freeval(v7, v7->stack[v7->sp - (i + 1)]);
    v7->stack[v7->sp - (i + 1)] = NULL;
  }

  v7->sp += incr;
  return V7_OK;
}

enum v7_err v7_pop(struct v7 *v7, int incr) {
  CHECK(incr >= 0, V7_INTERNAL_ERROR);
  return inc_stack(v7, -incr);
}

static void free_scopes(struct v7 *v7, int i) {
  for (; i < (int) ARRAY_SIZE(v7->scopes); i++) {
    v7->scopes[i].ref_count = 1;   // Force v7_freeval() below to free memory
    v7_freeval(v7, &v7->scopes[i]);
  }
}

static void free_values(struct v7 *v7) {
  struct v7_val *v;
  while (v7->free_values != NULL) {
    v = v7->free_values->next;
    free(v7->free_values); 
    v7->free_values = v;
  }
}

static void free_props(struct v7 *v7) {
  struct v7_prop *p;
  while (v7->free_props != NULL) {
    p = v7->free_props->next;
    free(v7->free_props); 
    v7->free_props = p;
  }
}

void v7_destroy(struct v7 **v7) {
  if (v7 == NULL || v7[0] == NULL) return;
  assert(v7[0]->sp >= 0);
  inc_stack(v7[0], -v7[0]->sp);
  free_scopes(v7[0], 0);
  free_values(v7[0]);
  free_props(v7[0]);
  free(v7[0]);
  v7[0] = NULL;
}

struct v7_val **v7_top(struct v7 *v7) {
  return &v7->stack[v7->sp];
}

int v7_sp(struct v7 *v7) {
  return (int) (v7_top(v7) - v7->stack);
}

int v7_is_true(const struct v7_val *v) {
  return  (v->type == V7_BOOL && v->v.num != 0.0) ||
          (v->type == V7_NUM && v->v.num != 0.0 && !isnan(v->v.num)) ||
          (v->type == V7_STR && v->v.str.len > 0) ||
          (v->type == V7_OBJ) ||
          (v->type == V7_ARRAY);
}

static void obj_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m;
  int n = snprintf(buf, bsiz, "%s", "{");

  for (m = v->v.props; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != v->v.props) n += snprintf(buf + n , bsiz - n, "%s", ", ");
    v7_to_string(m->key, buf + n, bsiz - n);
    n = (int) strlen(buf);
    n += snprintf(buf + n , bsiz - n, "%s", ": ");
    v7_to_string(m->val, buf + n, bsiz - n);
    n = (int) strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "}");
}

static void arr_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m;
  int n = snprintf(buf, bsiz, "%s", "[");

  for (m = v->v.props; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != v->v.props) n += snprintf(buf + n , bsiz - n, "%s", ", ");
    v7_to_string(m->val, buf + n, bsiz - n);
    n = (int) strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "]");
}

const char *v7_to_string(const struct v7_val *v, char *buf, int bsiz) {
  switch (v->type) {
    case V7_NUM:
      snprintf(buf, bsiz, "%lg", v->v.num);
      break;
    case V7_BOOL:
      snprintf(buf, bsiz, "%s", v->v.num ? "true" : "false");
      break;
    case V7_UNDEF:
      snprintf(buf, bsiz, "%s", "undefined");
      break;
    case V7_NULL:
      snprintf(buf, bsiz, "%s", "null");
      break;
    case V7_OBJ:
      obj_to_string(v, buf, bsiz);
      break;
    case V7_ARRAY:
      arr_to_string(v, buf, bsiz);
      break;
    case V7_STR:
        snprintf(buf, bsiz, "%.*s", (int) v->v.str.len, v->v.str.buf);
        break;
    case V7_FUNC:
        snprintf(buf, bsiz, "'function%s'", v->v.func);
        break;
    case V7_C_FUNC:
      snprintf(buf, bsiz, "'c_func_%p'", v->v.c_func);
      break;
    case V7_RO_PROP:
      snprintf(buf, bsiz, "'c_prop_%p'", v->v.prop_func);
      break;
    default:
      snprintf(buf, bsiz, "??");
      break;
  }
  buf[bsiz - 1] = '\0';
  return buf;
}

struct v7_val *v7_mkval(struct v7 *v7, enum v7_type type) {
  struct v7_val *v = NULL;

  if ((v = v7->free_values) != NULL) {
    v7->free_values = v->next;
  } else {
    v = (struct v7_val *) calloc(1, sizeof(*v));
  }

  if (v != NULL) {
    v->type = type;
    switch (type) {
      case V7_STR: v->proto = &s_string; break;
      case V7_NUM: v->proto = &s_number; break;
      case V7_ARRAY: v->proto = &s_array; break;
      default: v->proto = &s_object; break;
    }
  }
  return v;
}

static struct v7_val *v7_mkval_str(struct v7 *v7, const char *buf,
                                   unsigned long len, int own) {
  struct v7_val *v = v7_mkval(v7, V7_STR);
  v->v.str.len = len;
  v->v.str.buf = own ? v7_strdup(buf, len) : (char *) buf;
  if (!own) v->flags = V7_RDONLY_STR;
  return v;
}

enum v7_err v7_push(struct v7 *v7, struct v7_val *v) {
  v->ref_count++;
  TRY(inc_stack(v7, 1));
  v7->stack[v7->sp - 1] = v;
  return V7_OK;
}

enum v7_err v7_make_and_push(struct v7 *v7, enum v7_type type) {
  struct v7_val *v = v7_mkval(v7, type);
  CHECK(v != NULL, V7_OUT_OF_MEMORY);
  return v7_push(v7, v);
}

static enum v7_err do_arithmetic_op(struct v7 *v7, int op) {
  struct v7_val **v = v7_top(v7) - 2;

  if (v7->no_exec) return V7_OK;
  CHECK(v7->sp >= 2, V7_STACK_UNDERFLOW);

  if (v[0]->type == V7_STR && v[1]->type == V7_STR && op == '+') {
    struct v7_val *res = NULL;
    char *str = (char *) malloc(v[0]->v.str.len + v[1]->v.str.len + 1);
    CHECK(str != NULL, V7_OUT_OF_MEMORY);
    res = v7_mkval(v7, V7_STR);
    CHECK(res != NULL, V7_OUT_OF_MEMORY);  // TODO: free(str)
    res->v.str.len = v[0]->v.str.len + v[1]->v.str.len;
    res->v.str.buf = str;
    memcpy(str, v[0]->v.str.buf, v[0]->v.str.len);
    memcpy(str + v[0]->v.str.len, v[1]->v.str.buf, v[1]->v.str.len);
    str[res->v.str.len] = '\0';
    TRY(inc_stack(v7, -2));
    TRY(v7_push(v7, res));
    return V7_OK;
  } else if (v[0]->type == V7_NUM && v[1]->type == V7_NUM) {
    double a = v[0]->v.num, b = v[1]->v.num, res = 0;
    switch (op) {
      case '+': res = a + b; break;
      case '-': res = a - b; break;
      case '*': res = a * b; break;
      case '/': res = a / b; break;
    }
    TRY(inc_stack(v7, -2));
    TRY(v7_make_and_push(v7, V7_NUM));
    v7_top(v7)[-1]->v.num = res;
    return V7_OK;
  } else {
    return V7_TYPE_MISMATCH;
  }
}

static struct v7_val str_to_val(const char *buf, size_t len) {
  struct v7_val v;
  v.type = V7_STR;
  v.v.str.buf = (char *) buf;
  v.v.str.len = len;
  return v;
}

struct v7_val v7_str_to_val(const char *buf) {
  return str_to_val((char *) buf, strlen(buf));
}

static int cmp(const struct v7_val *a, const struct v7_val *b) {
  if (a == NULL || b == NULL) return 0;
  if (a->type != b->type) return 0;
  {
    double an = a->v.num, bn = b->v.num;
    const struct v7_str *as = &a->v.str, *bs = &b->v.str;

    switch (a->type) {
      case V7_NUM:
        return (an == bn) || (isinf(an) && isinf(bn)) ||
          (isnan(an) && isnan(bn));
      case V7_BOOL:
        return an == bn;
      case V7_STR:
        return as->len == bs->len && !memcmp(as->buf, bs->buf, as->len);
      case V7_UNDEF:
        return 0;
      default:
        return a == b;
    }
  }
}

static struct v7_prop *v7_get(struct v7_val *obj, const struct v7_val *key) {
  struct v7_prop *m;
  for (; obj != NULL; obj = obj->proto) {
    if (obj->type == V7_ARRAY && key->type == V7_NUM) {
      int i = (int) key->v.num;
      for (m = obj->v.props; m != NULL; m = m->next) {
        if (i-- == 0) return m;
      }
    } else if (obj->type == V7_OBJ) {
      for (m = obj->v.props; m != NULL; m = m->next) {
        if (cmp(m->key, key)) return m;
      }
    }
  }
  return NULL;
}

static struct v7_val *get2(struct v7_val *obj, const struct v7_val *key) {
  struct v7_prop *m = v7_get(obj, key);
  return (m == NULL) ? NULL : m->val;
}

struct v7_val *v7_lookup(struct v7_val *obj, const char *key) {
  struct v7_val k = v7_str_to_val(key);
  return get2(obj, &k);
}

static enum v7_err v7_make_and_push_string(struct v7 *v7, const char *s,
                                           unsigned long len, int do_copy) {
  struct v7_val **v = v7_top(v7);
  TRY(v7_make_and_push(v7, V7_STR));
  v[0]->v.str.len = len;
  v[0]->v.str.buf = do_copy ? v7_strdup(s, len) : (char *) s;
  v[0]->flags = do_copy ? 0 : V7_RDONLY_STR;
  return V7_OK;
}

static struct v7_prop *mkprop(struct v7 *v7) {
  struct v7_prop *m;
  if ((m = v7->free_props) != NULL) {
    v7->free_props = m->next;
  } else {
    m = (struct v7_prop *) calloc(1, sizeof(*m));
  }
  return m;
}

static enum v7_err vinsert(struct v7 *v7, struct v7_prop **h,
                           struct v7_val *key, struct v7_val *val) {
  struct v7_prop *m = mkprop(v7);
  CHECK(m != NULL, V7_OUT_OF_MEMORY);

  key->ref_count++;
  val->ref_count++;
  m->key = key;
  m->val = val;
  m->next = *h;
  *h = m;

  return V7_OK;
}

static struct v7_val *find(struct v7 *v7, struct v7_val *key) {
  struct v7_val *v;
  int i;

  if (v7->no_exec) return NULL;
  // Search for the name, traversing scopes up to the top level scope
  for (i = v7->current_scope; i >= 0; i--) {
    if ((v = get2(&v7->scopes[i], key)) != NULL) return v;
  }
  return NULL;
}

enum v7_err v7_set(struct v7 *v7, struct v7_val *obj, struct v7_val *k,
                   struct v7_val *v) {
  struct v7_prop *m = NULL;

  CHECK(obj != NULL && k != NULL && v != NULL, V7_INTERNAL_ERROR);
  CHECK(obj->type == V7_OBJ || obj->type == V7_ARRAY, V7_TYPE_MISMATCH);

  // Find attribute inside object
  if ((m = v7_get(obj, k)) != NULL) {
    v7_freeval(v7, m->val);
    v->ref_count++;
    m->val = v;
  } else {
    TRY(vinsert(v7, &obj->v.props, k, v));
  }

  return V7_OK;
}

enum v7_err v7_set_func(struct v7 *v7, struct v7_val *obj,
                        const char *key, v7_func_t c_func) {
  struct v7_val *k = v7_mkval_str(v7, key, strlen(key), 1);
  struct v7_val *v = v7_mkval(v7, V7_C_FUNC);
  CHECK(k != NULL && v != NULL, V7_OUT_OF_MEMORY);
  v->v.c_func = c_func;
  return v7_set(v7, obj, k, v);
}

enum v7_err v7_set_num(struct v7 *v7, struct v7_val *obj,
                       const char *key, double num) {
  struct v7_val *k = v7_mkval_str(v7, key, strlen(key), 1);
  struct v7_val *v = v7_mkval(v7, V7_NUM);
  CHECK(k != NULL && v != NULL, V7_OUT_OF_MEMORY);
  v->v.num = num;
  return v7_set(v7, obj, k, v);
}

enum v7_err v7_set_str(struct v7 *v7, struct v7_val *obj, const char *key,
                       const char *str, unsigned long len, int own) {
  struct v7_val *k = v7_mkval_str(v7, key, strlen(key), 1);
  struct v7_val *v = v7_mkval_str(v7, str, len, own);
  CHECK(k != NULL && v != NULL, V7_OUT_OF_MEMORY);
  return v7_set(v7, obj, k, v);
}

enum v7_err v7_set_obj(struct v7 *v7, struct v7_val *obj, const char *key,
                       struct v7_val *val) {
  struct v7_val *k = v7_mkval_str(v7, key, strlen(key), 1);
  CHECK(k != NULL, V7_OUT_OF_MEMORY);
  return v7_set(v7, obj, k, val);
}

static int is_alpha(int ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static int is_digit(int ch) {
  return ch >= '0' && ch <= '9';
}

static int is_alnum(int ch) {
  return is_digit(ch) || is_alpha(ch);
}

static int is_space(int ch) {
  return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static void skip_whitespaces_and_comments(struct v7 *v7) {
  const char *s = v7->cursor, *p = NULL;
  while (s != p && *s != '\0' && (is_space(*s) || *s == '/')) {
    p = s;
    while (*s != '\0' && is_space(*s)) {
      if (*s == '\n') v7->line_no++;
      s++;
    }
    if (s[0] == '/' && s[1] == '/') {
      s += 2;
      while (s[0] != '\0' && s[0] != '\n') s++;
    }
    if (s[0] == '/' && s[1] == '*') {
      s += 2;
      while (s[0] != '\0' && !(s[-1] == '/' && s[-2] == '*')) {
        if (s[0] == '\n') v7->line_no++;
        s++;
      }
    }
  }
  v7->cursor = s;
}

static enum v7_err match(struct v7 *v7, int ch) {
  CHECK(*v7->cursor++ == ch, V7_SYNTAX_ERROR);
  skip_whitespaces_and_comments(v7);
  return V7_OK;
}

static int test_and_skip_char(struct v7 *v7, int ch) {
  if (*v7->cursor == ch) {
    v7->cursor++;
    skip_whitespaces_and_comments(v7);
    return 1;
  }
  return 0;
}

static int test_token(struct v7 *v7, const char *kw, unsigned long kwlen) {
  return kwlen == v7->tok_len && memcmp(v7->tok, kw, kwlen) == 0;
}

//  number      =   { digit }
static enum v7_err parse_num(struct v7 *v7) {
  double value = 0;
  char *end;
  
  value = strtod(v7->cursor, &end);
  // Handle case like 123.toString()
  if (end != NULL && (v7->cursor < &end[-1]) && end[-1] == '.') end--;
  CHECK(value != 0 || end > v7->cursor, V7_SYNTAX_ERROR);
  v7->cursor = end;
  v7->tok_len = (unsigned long) (v7->cursor - v7->tok);
  skip_whitespaces_and_comments(v7);

  if (!v7->no_exec) {
    TRY(v7_make_and_push(v7, V7_NUM));
    v7_top(v7)[-1]->v.num = value;
  }

  return V7_OK;
}

//  identifier  =   letter { letter | digit }
static enum v7_err parse_identifier(struct v7 *v7) {
  CHECK(is_alpha(*v7->cursor) || *v7->cursor == '_', V7_SYNTAX_ERROR);
  v7->tok = v7->cursor;
  v7->cursor++;
  while (is_alnum(*v7->cursor) || *v7->cursor == '_') v7->cursor++;
  v7->tok_len = (unsigned long) (v7->cursor - v7->tok);
  skip_whitespaces_and_comments(v7);
  return V7_OK;
}

static enum v7_err parse_compound_statement(struct v7 *v7, int *has_return) {
  if (*v7->cursor == '{') {
    int old_sp = v7->sp;
    TRY(match(v7, '{'));
    while (*v7->cursor != '}') {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(parse_statement(v7, has_return));
      if (*has_return && !v7->no_exec) return V7_OK;
    }
    TRY(match(v7, '}'));
  } else {
    TRY(parse_statement(v7, has_return));
  }
  return V7_OK;
}

// function_defition = "function" "(" func_params ")" "{" func_body "}"
static enum v7_err parse_function_definition(struct v7 *v7, struct v7_val **v,
                                             int num_params) {
  int i = 0, old_no_exec = v7->no_exec, old_sp = v7->sp, len, has_return = 0;
  const char *src = v7->cursor, *func_name = NULL;

  if (*v7->cursor != '(') {
    // function name is given, e.g. function foo() {}
    CHECK(v == NULL, V7_SYNTAX_ERROR);
    TRY(parse_identifier(v7));
    func_name = v7->tok;
    len = v7->tok_len;
    src = v7->cursor;
  }

  // If 'v' (func to call) is NULL, that means we're just parsing function
  // definition to save it's body.
  v7->no_exec = v == NULL;
  TRY(match(v7, '('));

  // Initialize new scope
  if (!v7->no_exec) {
    v7->current_scope++;
    CHECK(v7->current_scope < (int) ARRAY_SIZE(v7->scopes),
          V7_RECURSION_TOO_DEEP);
    CHECK(v7->scopes[v7->current_scope].v.props == NULL, V7_INTERNAL_ERROR);
    CHECK(v7->scopes[v7->current_scope].type == V7_OBJ, V7_INTERNAL_ERROR);
  }

  while (*v7->cursor != ')') {
    TRY(parse_identifier(v7));
    if (!v7->no_exec) {
      struct v7_val *key = v7_mkval_str(v7, v7->tok, v7->tok_len, 1);
      struct v7_val *val = i < num_params ? v[i + 1] : v7_mkval(v7, V7_UNDEF);
      TRY(v7_set(v7, &v7->scopes[v7->current_scope], key, val));
    }
    i++;
    if (!test_and_skip_char(v7, ',')) break;
  }
  TRY(match(v7, ')'));
  TRY(parse_compound_statement(v7, &has_return));

  if (v7->no_exec) {
    TRY(v7_make_and_push(v7, V7_FUNC));
    v7_top(v7)[-1]->v.func = v7_strdup(src, v7->cursor - src);

    if (func_name != NULL) {
      struct v7_val *key = v7_mkval_str(v7, func_name, len, 1);
      TRY(v7_set(v7, &v7->scopes[v7->current_scope], key, v7_top(v7)[-1]));
    }
  }

  if (!v7->no_exec) {
    // If function didn't have return statement, return UNDEF
    if (!has_return) {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(v7_make_and_push(v7, V7_UNDEF));
    }
    // Clean up function scope
    v7->scopes[v7->current_scope].ref_count = 1;  // Force v7_freeval() below
    v7_freeval(v7, &v7->scopes[v7->current_scope]);
    v7->current_scope--;
    CHECK(v7->current_scope >= 0, V7_INTERNAL_ERROR);
  } 

  v7->no_exec = old_no_exec;
  return V7_OK;
}

enum v7_err v7_call(struct v7 *v7, struct v7_val *this_obj, int num_args) {
  struct v7_val **top = v7_top(v7), **v = top - (num_args + 1), *f = v[0];

  if (v7->no_exec) return V7_OK;
  CHECK(v7->sp > num_args, V7_INTERNAL_ERROR);
  CHECK(f->type == V7_FUNC || f->type == V7_C_FUNC, V7_CALLED_NON_FUNCTION);

  // Return value will substitute function objest on a stack
  v[0] = v7_mkval(v7, V7_UNDEF);  // Set return value to 'undefined'
  v[0]->ref_count++;

  if (f->type == V7_FUNC) {
    const char *src = v7->cursor;
    v7->cursor = f->v.func;     // Move control flow to function body
    TRY(parse_function_definition(v7, v, num_args));  // Execute function body
    v7->cursor = src;           // Return control flow
    if (v7_top(v7) > top) {     // If function body pushed some value on stack,
      v7_freeval(v7, v[0]);
      v[0] = top[0];            // use that value as return value
      v[0]->ref_count++;
    }
  } else if (f->type == V7_C_FUNC) {
    f->v.c_func(v7, this_obj, v[0], v + 1, num_args);
  }
  v7_freeval(v7, f);

  TRY(inc_stack(v7, - (int) (v7_top(v7) - (v + 1))));  // Clean up stack
  return V7_OK;
}

//  function_call  = expression "(" { expression} ")"
static enum v7_err parse_function_call(struct v7 *v7, struct v7_val *this_obj) {
  struct v7_val **v = v7_top(v7) - 1;
  int num_args = 0;

  CHECK(v7->no_exec || v[0]->type == V7_FUNC || v[0]->type == V7_C_FUNC,
        V7_CALLED_NON_FUNCTION);

  // Push arguments on stack
  TRY(match(v7, '('));
  while (*v7->cursor != ')') {
    TRY(parse_expression(v7));
    test_and_skip_char(v7, ',');
    num_args++;
  }
  TRY(match(v7, ')'));

  if (!v7->no_exec) {
    TRY(v7_call(v7, this_obj, num_args));
  }
  return V7_OK;
}

static enum v7_err parse_string_literal(struct v7 *v7) {
  char buf[MAX_STRING_LITERAL_LENGTH];
  const char *begin = v7->cursor++;
  struct v7_val *v;
  size_t i = 0;

  TRY(v7_make_and_push(v7, V7_STR));
  v = v7_top(v7)[-1];

  // Scan string literal into the buffer, handle escape sequences
  while (*v7->cursor != *begin && *v7->cursor != '\0') {
    switch (*v7->cursor) {
      case '\\':
        v7->cursor++;
        switch (*v7->cursor) {
          case 'n': buf[i++] = '\n'; break;
          case 't': buf[i++] = '\t'; break;
          case '\\': buf[i++] = '\\'; break;
          default: if (*v7->cursor == *begin) buf[i++] = *begin; break;
        }
        break;
      default:
        buf[i++] = *v7->cursor;
        break;
    }
    if (i >= sizeof(buf) - 1) i = sizeof(buf) - 1;
    v7->cursor++;
  }

  v->v.str.len = v7->no_exec ? 0 : i;
  v->v.str.buf = v7->no_exec ? NULL : v7_strdup(buf, v->v.str.len);
  TRY(match(v7, *begin));
  skip_whitespaces_and_comments(v7);

  return V7_OK;
}

enum v7_err v7_append(struct v7 *v7, struct v7_val *arr, struct v7_val *val) {
  struct v7_val *key = v7_mkval(v7, V7_NUM);
  struct v7_prop **head, *prop;
  CHECK(arr->type == V7_ARRAY, V7_INTERNAL_ERROR);
  // Append using a dummy key. TODO: do not use dummy keys, they eat RAM
  // Make dummy key unique, or v7_set() may override an existing key.
  // Also, we want to append to the end of the list, to make indexing work
  key->v.num = (unsigned long) key;
  for (head = &arr->v.props; *head != NULL; head = &head[0]->next);
  prop = mkprop(v7);
  CHECK(prop != NULL, V7_OUT_OF_MEMORY);
  prop->next = *head;
  *head = prop;
  prop->key = key;
  prop->val = val;
  key->ref_count++;
  val->ref_count++;
  //TRY(vinsert(v7, &arr->v.props, key, val));
  return V7_OK;
}

static enum v7_err parse_array_literal(struct v7 *v7) {
  // Push empty array on stack
  TRY(v7_make_and_push(v7, V7_ARRAY));
  TRY(match(v7, '['));

  // Scan array literal, append elements one by one
  while (*v7->cursor != ']') {
    // Push new element on stack
    TRY(parse_expression(v7));
    if (!v7->no_exec) {
      TRY(v7_append(v7, v7_top(v7)[-2], v7_top(v7)[-1]));
      TRY(inc_stack(v7, -1));
    }
    test_and_skip_char(v7, ',');
  }

  TRY(match(v7, ']'));
  return V7_OK;
}

static enum v7_err parse_object_literal(struct v7 *v7) {
  // Push empty object on stack
  TRY(v7_make_and_push(v7, V7_OBJ));
  TRY(match(v7, '{'));

  // Assign key/values to the object, until closing "}" is found
  while (*v7->cursor != '}') {
    // Push key on stack
    if (*v7->cursor == '\'' || *v7->cursor == '"') {
      TRY(parse_string_literal(v7));
    } else {
      TRY(parse_identifier(v7));
      TRY(v7_make_and_push_string(v7, v7->tok, v7->tok_len, 1));
    }

    // Push value on stack
    TRY(match(v7, ':'));
    TRY(parse_expression(v7));

    // Stack should now have object, key, value. Assign, and remove key/value
    if (!v7->no_exec) {
      struct v7_val **v = v7_top(v7) - 3;
      CHECK(v[0]->type == V7_OBJ, V7_INTERNAL_ERROR);
      TRY(v7_set(v7, v[0], v[1], v[2]));
      TRY(inc_stack(v7, -2));
    }
    test_and_skip_char(v7, ',');
  }
  TRY(match(v7, '}'));
  return V7_OK;
}

// variable = identifier { '.' identifier | '[' expression ']' }
static enum v7_err parse_prop_accessor(struct v7 *v7) {
  struct v7_val **top = NULL, *v = NULL, *ns = NULL;

  if (!v7->no_exec) {
    top = &v7_top(v7)[-1];
    v = v7_mkval(v7, V7_UNDEF);
    v->ref_count++;
    ns = top[0];
  }

  while (*v7->cursor == '.' || *v7->cursor == '[') {
    int ch = *v7->cursor;

    TRY(match(v7, ch));
    CHECK(v7->no_exec || ns != NULL, V7_SYNTAX_ERROR);
    v7->cur_obj = ns;

    if (ch == '.') {
      TRY(parse_identifier(v7));
      if (!v7->no_exec) {
        struct v7_val key = str_to_val(v7->tok, v7->tok_len);
        ns = get2(ns, &key);
        if (ns != NULL && ns->type == V7_RO_PROP) {
          ns->v.prop_func(v7->cur_obj, v);
          ns = v;
        }
      }
    } else {
      TRY(parse_expression(v7));
      TRY(match(v7, ']'));
      if (!v7->no_exec) {
        ns = get2(ns, v7_top(v7)[-1]);
        if (ns != NULL && ns->type == V7_RO_PROP) {
          ns->v.prop_func(v7->cur_obj, v);
          ns = v;
        }
        TRY(inc_stack(v7, -1));
      }
    }
  }

  if (top != NULL) {
    struct v7_val *old = top[0];
    top[0] = ns == NULL ? v : ns;
    top[0]->ref_count++;

    // parse_function_call() might use v7->cur_obj, which can be
    // deallocated by the v7_freeval() calls below. Therefore, we call
    // parse_function_call() right here, before cleaning up.
    // NOTE: find a better way of doing this.
    if (*v7->cursor == '(') {
      TRY(parse_function_call(v7, v7->cur_obj));
    }

    v7_freeval(v7, v);
    v7_freeval(v7, old);
  }

  return V7_OK;
}

enum v7_err v7_del(struct v7 *v7, struct v7_val *obj, struct v7_val *key) {
  struct v7_prop **p;
  CHECK(obj->type == V7_OBJ || obj->type == V7_ARRAY, V7_TYPE_MISMATCH);
  for (p = &obj->v.props; *p != NULL; p = &p[0]->next) {
    if (cmp(key, p[0]->key)) {
      struct v7_prop *next = p[0]->next;
      v7_freeval(v7, p[0]->key);
      v7_freeval(v7, p[0]->val);
      free(p[0]);
      p[0] = next;
      break;
    }
  }
  return V7_OK;
}

static enum v7_err parse_delete(struct v7 *v7) {
  struct v7_val key;
  TRY(parse_expression(v7));
  key = str_to_val(v7->tok, v7->tok_len);  // Must go after parse_expression
  TRY(v7_del(v7, v7->cur_obj, &key));
  return V7_OK;
}

static enum v7_err parse_variable(struct v7 *v7) {
  struct v7_val key = str_to_val(v7->tok, v7->tok_len), *v = NULL;
  if (!v7->no_exec) {
    v = find(v7, &key);
    if (v == NULL) {
      TRY(v7_make_and_push(v7, V7_UNDEF));
    } else {
      TRY(v7_push(v7, v));
    }
  }
  return V7_OK;
}

//  factor  =   number | string_literal | "(" expression ")" |
//              variable | "this" | "null" | "true" | "false" |
//              "{" object_literal "}" |
//              "[" array_literal "]" |
//              function_definition |
//              function_call
static enum v7_err parse_factor(struct v7 *v7) {
  int old_sp = v7_sp(v7);

  if (*v7->cursor == '!') {
    TRY(match(v7, '!'));
    TRY(parse_expression(v7));
    if (!v7->no_exec) {
      int is_true = v7_is_true(v7_top(v7)[-1]);
      TRY(v7_pop(v7, 1));
      TRY(v7_make_and_push(v7, V7_BOOL));
      v7_top(v7)[-1]->v.num = is_true ? 0.0 : 1.0;
    }
  } else if (*v7->cursor == '(') {
    TRY(match(v7, '('));
    TRY(parse_expression(v7));
    TRY(match(v7, ')'));
  } else if (*v7->cursor == '\'' || *v7->cursor == '"') {
    TRY(parse_string_literal(v7));
  } else if (*v7->cursor == '{') {
    TRY(parse_object_literal(v7));
  } else if (*v7->cursor == '[') {
    TRY(parse_array_literal(v7));
  } else if (is_alpha(*v7->cursor) || *v7->cursor == '_') {
    TRY(parse_identifier(v7));
    if (test_token(v7, "this", 4)) {
      TRY(v7_push(v7, &v7->scopes[v7->current_scope]));
    } else if (test_token(v7, "null", 4)) {
      TRY(v7_make_and_push(v7, V7_NULL));
    } else if (test_token(v7, "undefined", 9)) {
      TRY(v7_make_and_push(v7, V7_UNDEF));
    } else if (test_token(v7, "true", 4)) {
      TRY(v7_make_and_push(v7, V7_BOOL));
      v7_top(v7)[-1]->v.num = 1;
    } else if (test_token(v7, "false", 5)) {
      TRY(v7_make_and_push(v7, V7_BOOL));
      v7_top(v7)[-1]->v.num = 0;
    } else if (test_token(v7, "function", 8)) {
      TRY(parse_function_definition(v7, NULL, 0));
    } else if (test_token(v7, "delete", 6)) {
      TRY(parse_delete(v7));
    } else if (test_token(v7, "NaN", 3)) {
      TRY(v7_make_and_push(v7, V7_NUM));
      v7_top(v7)[-1]->v.num = NAN;
    } else if (test_token(v7, "Infinity", 8)) {
      TRY(v7_make_and_push(v7, V7_NUM));
      v7_top(v7)[-1]->v.num = INFINITY;
    } else {
      TRY(parse_variable(v7));
    }
  } else {
    TRY(parse_num(v7));
  }

  while (*v7->cursor == '.' || *v7->cursor == '[') {
    TRY(parse_prop_accessor(v7));
  }

  if (*v7->cursor == '(') {
    TRY(parse_function_call(v7, v7->cur_obj));
  }

  // Don't leave anything on stack if no execution flag is set
  if (v7->no_exec) {
    inc_stack(v7, old_sp - v7->sp);
  }

  return V7_OK;
}

//  term        =   factor { mul_op factor }
//  mul_op      =   "*" | "/"
static enum v7_err parse_term(struct v7 *v7) {
  TRY(parse_factor(v7));
  while (*v7->cursor == '*' || *v7->cursor == '/') {
    int ch = *v7->cursor;
    TRY(match(v7, ch));
    TRY(parse_factor(v7));
    TRY(do_arithmetic_op(v7, ch));
  }
  return V7_OK;
}

enum { OP_XX, OP_GT, OP_LT, OP_GE, OP_LE, OP_EQ, OP_NE };

static int is_logical_op(const char *s) {
  switch (s[0]) {
    case '>': return s[1] == '=' ? OP_GE : OP_GT;
    case '<': return s[1] == '=' ? OP_LE : OP_LT;
    case '=': return s[1] == '=' ? OP_EQ : OP_XX;
    case '!': return s[1] == '=' ? OP_NE : OP_XX;
    default: return OP_XX;
  }
}

static enum v7_err do_logical_op(struct v7 *v7, int op) {
  struct v7_val **v = v7_top(v7) - 2;
  int res = 0;

  if (v7->no_exec) return V7_OK;
  if (v[0]->type == V7_NUM && v[1]->type == V7_NUM) {
    switch (op) {
      case OP_GT: res = v[0]->v.num >  v[1]->v.num; break;
      case OP_GE: res = v[0]->v.num >= v[1]->v.num; break;
      case OP_LT: res = v[0]->v.num <  v[1]->v.num; break;
      case OP_LE: res = v[0]->v.num <= v[1]->v.num; break;
      case OP_EQ: res = cmp(v[0], v[1]); break;
      case OP_NE: res = !cmp(v[0], v[1]); break;
    }
  } else if (op == OP_EQ) {
    res = cmp(v[0], v[1]);
  } else if (op == OP_NE) {
    res = !cmp(v[0], v[1]);
  }
  TRY(inc_stack(v7, -2));
  TRY(v7_make_and_push(v7, V7_BOOL));
  v[0]->v.num = res ? 1.0 : 0.0;
  return V7_OK;
}

static enum v7_err parse_assignment(struct v7 *v7, struct v7_val *obj) {
  const char *tok = v7->tok;
  unsigned long tok_len = v7->tok_len;

  TRY(match(v7, '='));
  TRY(parse_expression(v7));

  if (!v7->no_exec) {
    struct v7_val **top = v7_top(v7), *key = v7_mkval_str(v7, tok, tok_len, 1);
    key->ref_count++;
    TRY(v7_set(v7, obj, key, top[-1]));
    v7_freeval(v7, key);
    v7_freeval(v7, top[-2]);
    top[-2] = top[-1];
    v7->sp--;
  }

  return V7_OK;
}

//  expression  =   term { add_op term } |
//                  expression "?" expression ":" expression
//                  expression "." expression
//                  expression logical_op expression
//                  variable "=" expression
//  add_op      =   "+" | "-"
static enum v7_err parse_expression(struct v7 *v7) {
#ifdef V7_DEBUG
  const char *stmt_str = v7->cursor;
#endif
  int op;

  v7->cur_obj = &v7->scopes[v7->current_scope];
  TRY(parse_term(v7));

  while (*v7->cursor == '-' || *v7->cursor == '+') {
    int ch = *v7->cursor;
    TRY(match(v7, ch));
    TRY(parse_term(v7));
    TRY(do_arithmetic_op(v7, ch));
  }

  if ((op = is_logical_op(v7->cursor)) > OP_XX) {
    v7->cursor += op == OP_LT || op == OP_GT ? 1 : 2;
    skip_whitespaces_and_comments(v7);
    TRY(parse_term(v7));
    TRY(do_logical_op(v7, op));
  }

  // Parse assignment
  if (*v7->cursor == '=') {
    TRY(parse_assignment(v7, v7->cur_obj));
  }

  // Parse ternary operator
  if (*v7->cursor == '?') {
    int old_no_exec = v7->no_exec;
    int condition_true = 1;

    if (!v7->no_exec) {
      CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
      condition_true = v7_is_true(v7_top(v7)[-1]);
      TRY(inc_stack(v7, -1));   // Remove condition result
    }

    TRY(match(v7, '?'));
    v7->no_exec = old_no_exec || !condition_true;
    TRY(parse_expression(v7));
    TRY(match(v7, ':'));
    v7->no_exec = old_no_exec || condition_true;
    TRY(parse_expression(v7));
    v7->no_exec = old_no_exec;
  }

  return V7_OK;
}

//  declaration =   "var" identifier [ "=" expression ] [ "," { i [ "=" e ] } ]
static enum v7_err parse_declaration(struct v7 *v7) {
  int sp = v7_sp(v7);

  do {
    inc_stack(v7, sp - v7_sp(v7));  // Clean up the stack after prev decl
    TRY(parse_identifier(v7));
    if (*v7->cursor == '=') {
      if (!v7->no_exec) v7_make_and_push(v7, V7_UNDEF);
      TRY(parse_assignment(v7, &v7->scopes[v7->current_scope]));
    }
  } while (test_and_skip_char(v7, ','));

  return V7_OK;
}

static enum v7_err parse_if_statement(struct v7 *v7, int *has_return) {
  int old_no_exec = v7->no_exec;  // Remember execution flag
  TRY(match(v7, '('));
  TRY(parse_expression(v7));      // Evaluate condition, pushed on stack
  TRY(match(v7, ')'));
  if (!old_no_exec) {
    // If condition is false, do not execute "if" body
    CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
    v7->no_exec = !v7_is_true(v7_top(v7)[-1]);
    // If condition is true, clean it from stack
    if (!v7->no_exec) {
      TRY(inc_stack(v7, -1));
    }
  }
  TRY(parse_compound_statement(v7, has_return));
  v7->no_exec = old_no_exec;  // Restore old execution flag
  return V7_OK;
}

//  statement  =  declaration | return_statement | if_statement
//                assignment | expression [ ";" ]
static enum v7_err parse_statement(struct v7 *v7, int *has_return) {
  if (*v7->cursor == '_' || is_alpha(*v7->cursor)) {
    TRY(parse_identifier(v7));    // Load identifier into v7->tok, v7->tok_len
    if (test_token(v7, "var", 3)) {
      TRY(parse_declaration(v7));
    } else if (test_token(v7, "return", 6)) {
      if (!v7->no_exec) {
        *has_return = 1;
      }
      if (*v7->cursor != ';' && *v7->cursor != '}') {
        TRY(parse_expression(v7));
      }
    } else if (test_token(v7, "if", 2)) {
      TRY(parse_if_statement(v7, has_return));
    } else {
      v7->cursor = v7->tok;
      TRY(parse_expression(v7));
    }
  } else {
    TRY(parse_expression(v7));
  }

  // Skip optional semicolons
  while (*v7->cursor == ';') match(v7, *v7->cursor);
  return V7_OK;
}

//  code        =   { statement }
enum v7_err v7_exec(struct v7 *v7, const char *source_code) {
  int has_ret = 0;

  v7->source_code = v7->cursor = source_code;
  skip_whitespaces_and_comments(v7);

  // The following code may raise an exception and jump to the previous line,
  // returning non-zero from the setjmp() call
  // Prior calls to v7_exec() may have left current_scope modified, reset now
  free_scopes(v7, 1);
  v7->current_scope = 0;  // XXX free up higher scopes?

  while (*v7->cursor != '\0') {
    inc_stack(v7, -v7->sp);               // Reset stack on each statement
    TRY(parse_statement(v7, &has_ret));   // Last expr result on stack
  }

  return V7_OK;
}

enum v7_err v7_exec_file(struct v7 *v7, const char *path) {
  FILE *fp;
  char *p;
  size_t file_size;
  enum v7_err status = V7_INTERNAL_ERROR;

  if ((fp = fopen(path, "r")) == NULL) {
  } else if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0) {
    fclose(fp);
  } else if ((p = (char *) malloc(file_size + 1)) == NULL) {
    fclose(fp);
  } else {
    rewind(fp);
    fread(p, 1, file_size, fp);
    fclose(fp);
    p[file_size] = '\0';
    v7->line_no = 1;
    status = v7_exec(v7, p);
    free(p);
  }

  return status;
}

const char *v7_strerror(enum v7_err e) {
  static const char *strings[] = {
    "no error", "syntax error", "out of memory", "internal error",
    "stack overflow", "stack underflow", "undefined variable",
    "type mismatch", "recursion too deep", "called non-function"
  };
  assert(ARRAY_SIZE(strings) == V7_NUM_ERRORS);
  return e >= (int) ARRAY_SIZE(strings) ? "?" : strings[e];
}

#ifdef V7_EXE
int main(int argc, char *argv[]) {
  struct v7 *v7 = v7_create();
  int i, error_code;

  // Execute inline code
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
      if ((error_code = v7_exec(v7, argv[i + 1])) != V7_OK) {
        fprintf(stderr, "Error executing [%s]: %s\n", argv[i + 1],
                v7_strerror(error_code));
      }
      i++;
    }
  }

  // Execute files
  for (; i < argc; i++) {
    if ((error_code = v7_exec_file(v7, argv[i])) != V7_OK) {
      fprintf(stderr, "%s line %d: %s\n", argv[i], v7->line_no,
              v7_strerror(error_code));
    }
  }

  v7_destroy(&v7);
  return EXIT_SUCCESS;
}
#endif
