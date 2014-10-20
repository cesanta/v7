#include "internal.h"

V7_PRIVATE void obj_sanity_check(const struct v7_val *obj) {
  assert(obj != NULL);
  assert(obj->ref_count >= 0);
  assert(!(obj->flags & V7_VAL_DEALLOCATED));
}

V7_PRIVATE int instanceof(const struct v7_val *obj, const struct v7_val *ctor) {
  obj_sanity_check(obj);
  if (obj->type == V7_TYPE_OBJ && ctor != NULL) {
    while (obj != NULL) {
      if (obj->ctor == ctor) return 1;
      if (obj->proto == obj) break;  // Break on circular reference
      obj = obj->proto;
    }
  }
  return 0;
}

V7_PRIVATE int v7_is_class(const struct v7_val *obj, enum v7_class cls) {
  return instanceof(obj, &s_constructors[cls]);
}

V7_PRIVATE int is_string(const struct v7_val *v) {
  obj_sanity_check(v);
  return v->type == V7_TYPE_STR || v7_is_class(v, V7_CLASS_STRING);
}

V7_PRIVATE int is_num(const struct v7_val *v) {
  obj_sanity_check(v);
  return v->type == V7_TYPE_NUM || v7_is_class(v, V7_CLASS_NUMBER);
}

V7_PRIVATE int is_bool(const struct v7_val *v) {
  obj_sanity_check(v);
  return v->type == V7_TYPE_BOOL || v7_is_class(v, V7_CLASS_BOOLEAN);
}

V7_PRIVATE void inc_ref_count(struct v7_val *v) {
  obj_sanity_check(v);
  v->ref_count++;
}

V7_PRIVATE char *v7_strdup(const char *ptr, unsigned long len) {
  char *p = (char *) malloc(len + 1);
  if (p == NULL) return NULL;
  memcpy(p, ptr, len);
  p[len] = '\0';
  return p;
}

V7_PRIVATE void v7_init_str(struct v7_val *v, const char *p,
  unsigned long len, int own) {
  v->type = V7_TYPE_STR;
  v->proto = &s_prototypes[V7_CLASS_STRING];
  v->v.str.buf = (char *) p;
  v->v.str.len = len;
  v->flags &= ~V7_STR_ALLOCATED;
  if (own) {
    if (len < sizeof(v->v.str.loc) - 1) {
      v->v.str.buf = v->v.str.loc;
      memcpy(v->v.str.loc, p, len);
      v->v.str.loc[len] = '\0';
    } else {
      v->v.str.buf = v7_strdup(p, len);
      v->flags |= V7_STR_ALLOCATED;
    }
  }
}

V7_PRIVATE void v7_init_num(struct v7_val *v, double num) {
  v->type = V7_TYPE_NUM;
  v->proto = &s_prototypes[V7_CLASS_NUMBER];
  v->v.num = num;
}

V7_PRIVATE void v7_init_bool(struct v7_val *v, int is_true) {
  v->type = V7_TYPE_BOOL;
  v->proto = &s_prototypes[V7_CLASS_BOOLEAN];
  v->v.num = is_true ? 1.0 : 0.0;
}

V7_PRIVATE void v7_init_func(struct v7_val *v, v7_func_t func) {
  v7_set_class(v, V7_CLASS_FUNCTION);
  v->v.c_func = func;
}

V7_PRIVATE void v7_set_class(struct v7_val *v, enum v7_class cls) {
  v->type = V7_TYPE_OBJ;
  v->cls = cls;
  v->proto = &s_prototypes[cls];
  v->ctor = &s_constructors[cls];
}

V7_PRIVATE void free_prop(struct v7 *v7, struct v7_prop *p) {
  if (p->key != NULL) v7_freeval(v7, p->key);
  v7_freeval(v7, p->val);
  p->val = p->key = NULL;
  if (p->flags & V7_VAL_ALLOCATED) {
#ifdef V7_CACHE_OBJS
    p->next = v7->free_props;
    v7->free_props = p;
#else
    free(p);
#endif
  }
  p->flags = 0;
}

V7_PRIVATE void init_standard_constructor(enum v7_class cls, v7_func_t ctor) {
  s_prototypes[cls].type = s_constructors[cls].type = V7_TYPE_OBJ;
  s_prototypes[cls].ref_count = s_constructors[cls].ref_count = 1;
  s_prototypes[cls].proto = &s_prototypes[V7_CLASS_OBJECT];
  s_prototypes[cls].ctor = &s_constructors[cls];
  s_constructors[cls].proto = &s_prototypes[V7_CLASS_OBJECT];
  s_constructors[cls].ctor = &s_constructors[V7_CLASS_FUNCTION];
  s_constructors[cls].v.c_func = ctor;
}

V7_PRIVATE void v7_freeval(struct v7 *v7, struct v7_val *v) {
  assert(v->ref_count > 0);
  if (--v->ref_count > 0) return;

  if (v->type == V7_TYPE_OBJ) {
    struct v7_prop *p, *tmp;
    for (p = v->props; p != NULL; p = tmp) {
      tmp = p->next;
      free_prop(v7, p);
    }
    v->props = NULL;
  }

  if (v7_is_class(v, V7_CLASS_ARRAY)) {
    struct v7_prop *p, *tmp;
    for (p = v->v.array; p != NULL; p = tmp) {
      tmp = p->next;
      free_prop(v7, p);
    }
    v->v.array = NULL;
  } else if (v->type == V7_TYPE_STR && (v->flags & V7_STR_ALLOCATED)) {
    free(v->v.str.buf);
  } else if (v7_is_class(v, V7_CLASS_REGEXP)) {
    if(v->v.re.prog){
      if(v->v.re.prog->start) reg_free(v->v.re.prog->start);
      reg_free(v->v.re.prog);
    }
    if(v->v.re.buf && (v->flags & V7_STR_ALLOCATED)) free(v->v.re.buf);
  } else if (v7_is_class(v, V7_CLASS_FUNCTION)) {
    if ((v->flags & V7_STR_ALLOCATED) && (v->flags & V7_JS_FUNC)) {
      free(v->v.func.source_code);
      v7_freeval(v7, v->v.func.var_obj);
    }
  }

  if (v->flags & V7_VAL_ALLOCATED) {
    v->flags &= ~V7_VAL_ALLOCATED;
    v->flags |= ~V7_VAL_DEALLOCATED;
    memset(v, 0, sizeof(*v));
#ifdef V7_CACHE_OBJS
    v->next = v7->free_values;
    v7->free_values = v;
#else
    free(v);
#endif
  }
}

V7_PRIVATE enum v7_err inc_stack(struct v7 *v7, int incr) {
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

V7_PRIVATE void free_values(struct v7 *v7) {
  struct v7_val *v;
  while (v7->free_values != NULL) {
    v = v7->free_values->next;
    free(v7->free_values);
    v7->free_values = v;
  }
}

V7_PRIVATE void free_props(struct v7 *v7) {
  struct v7_prop *p;
  while (v7->free_props != NULL) {
    p = v7->free_props->next;
    free(v7->free_props);
    v7->free_props = p;
  }
}

V7_PRIVATE struct v7_val *make_value(struct v7 *v7, enum v7_type type) {
  struct v7_val *v = NULL;

  if ((v = v7->free_values) != NULL) {
    v7->free_values = v->next;
  } else {
    v = (struct v7_val *) calloc(1, sizeof(*v));
  }

  if (v != NULL) {
    assert(v->ref_count == 0);
    v->flags = V7_VAL_ALLOCATED;
    v->type = type;
    switch (type) {
      case V7_TYPE_NUM: v->proto = &s_prototypes[V7_CLASS_NUMBER]; break;
      case V7_TYPE_STR: v->proto = &s_prototypes[V7_CLASS_STRING]; break;
      case V7_TYPE_BOOL: v->proto = &s_prototypes[V7_CLASS_BOOLEAN]; break;
      default: break;
    }
  }
  return v;
}

V7_PRIVATE struct v7_prop *mkprop(struct v7 *v7) {
  struct v7_prop *m;
  if ((m = v7->free_props) != NULL) {
    v7->free_props = m->next;
  } else {
    m = (struct v7_prop *) calloc(1, sizeof(*m));
  }
  if (m != NULL) m->flags = V7_PROP_ALLOCATED;
  return m;
}

V7_PRIVATE struct v7_val str_to_val(const char *buf, size_t len) {
  struct v7_val v;
  memset(&v, 0, sizeof(v));
  v.type = V7_TYPE_STR;
  v.v.str.buf = (char *) buf;
  v.v.str.len = len;
  return v;
}

V7_PRIVATE struct v7_val v7_str_to_val(const char *buf) {
  return str_to_val((char *) buf, strlen(buf));
}

V7_PRIVATE int cmp(const struct v7_val *a, const struct v7_val *b) {
  int res;
  double an, bn;
  const struct v7_string *as, *bs;
  struct v7_val ta = MKOBJ(0), tb = MKOBJ(0);

  if (a == NULL || b == NULL) return 1;
  if ((a->type == V7_TYPE_UNDEF || a->type == V7_TYPE_NULL) &&
      (b->type == V7_TYPE_UNDEF || b->type == V7_TYPE_NULL)) return 0;

  if (is_num(a) && is_num(b)) {
    v7_init_num(&ta, a->v.num);
    v7_init_num(&tb, b->v.num);
    a = &ta; b = &tb;
  }

  if (is_string(a) && is_string(b)) {
    v7_init_str(&ta, a->v.str.buf, a->v.str.len, 0);
    v7_init_str(&tb, b->v.str.buf, b->v.str.len, 0);
    a = &ta; b = &tb;
  }

  if (a->type != b->type) return 1;

  an = a->v.num, bn = b->v.num;
  as = &a->v.str, bs = &b->v.str;

  switch (a->type) {
    case V7_TYPE_NUM:
      return (isinf(an) && isinf(bn)) ||
      (isnan(an) && isnan(bn)) ? 0 : an - bn;
    case V7_TYPE_BOOL:
      return an != bn;
    case V7_TYPE_STR:
      res = memcmp(as->buf, bs->buf, as->len < bs->len ? as->len : bs->len);
      return res != 0 ? res : (int) as->len - (int) bs->len;
      return as->len != bs->len || memcmp(as->buf, bs->buf, as->len) != 0;
    default:
      return (int) (a - b);
  }
}

V7_PRIVATE struct v7_prop *v7_get2(struct v7_val *obj, const struct v7_val *key,
                              int own_prop) {
  struct v7_prop *m;
  for (; obj != NULL; obj = obj->proto) {
    if (v7_is_class(obj, V7_CLASS_ARRAY) && key->type == V7_TYPE_NUM) {
      int i = (int) key->v.num;
      for (m = obj->v.array; m != NULL; m = m->next) {
        if (i-- == 0) return m;
      }
    } else if (obj->type == V7_TYPE_OBJ) {
      for (m = obj->props; m != NULL; m = m->next) {
        if (cmp(m->key, key) == 0) return m;
      }
    }
    if (own_prop) break;
    if (obj->proto == obj) break;
  }
  return NULL;
}

V7_PRIVATE struct v7_val *get2(struct v7_val *obj, const struct v7_val *key) {
  struct v7_prop *m = v7_get2(obj, key, 0);
  return (m == NULL) ? NULL : m->val;
}

V7_PRIVATE enum v7_err vinsert(struct v7 *v7, struct v7_prop **h,
                           struct v7_val *key, struct v7_val *val) {
  struct v7_prop *m = mkprop(v7);
  CHECK(m != NULL, V7_OUT_OF_MEMORY);

  inc_ref_count(key);
  inc_ref_count(val);
  m->key = key;
  m->val = val;
  m->next = *h;
  *h = m;

  return V7_OK;
}

V7_PRIVATE struct v7_val *find(struct v7 *v7, const struct v7_val *key) {
  struct v7_val *v, *f;

  if (!EXECUTING(v7->flags)) return NULL;

  for (f = v7->ctx; f != NULL; f = f->next) {
    if ((v = get2(f, key)) != NULL) return v;
  }

  if (v7->cf && (v = get2(v7->cf->v.func.var_obj, key)) != NULL) return v;

  return NULL;
}

V7_PRIVATE enum v7_err v7_set2(struct v7 *v7, struct v7_val *obj,
                              struct v7_val *k, struct v7_val *v) {
  struct v7_prop *m = NULL;

  CHECK(obj != NULL && k != NULL && v != NULL, V7_INTERNAL_ERROR);
  CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_ERROR);

  // Find attribute inside object
  if ((m = v7_get2(obj, k, 1)) != NULL) {
    v7_freeval(v7, m->val);
    inc_ref_count(v);
    m->val = v;
  } else {
    TRY(vinsert(v7, &obj->props, k, v));
  }

  return V7_OK;
}

V7_PRIVATE struct v7_val *v7_mkvv(struct v7 *v7, enum v7_type t, va_list *ap) {
  struct v7_val *v = make_value(v7, t);

  // TODO: check for make_value() failure
  switch (t) {
      //case V7_C_FUNC: v->v.c_func = va_arg(*ap, v7_func_t); break;
    case V7_TYPE_NUM:
      v->v.num = va_arg(*ap, double);
      break;
    case V7_TYPE_STR: {
      char *buf = va_arg(*ap, char *);
      unsigned long len = va_arg(*ap, unsigned long);
      int own = va_arg(*ap, int);
      v7_init_str(v, buf, len, own);
    }
      break;
    default:
      break;
  }

  return v;
}

V7_PRIVATE struct v7_val *v7_mkv(struct v7 *v7, enum v7_type t, ...) {
  struct v7_val *v = NULL;
  va_list ap;

  va_start(ap, t);
  v = v7_mkvv(v7, t, &ap);
  va_end(ap);

  return v;
}

V7_PRIVATE enum v7_err v7_setv(struct v7 *v7, struct v7_val *obj,
                          enum v7_type key_type, enum v7_type val_type, ...) {
  struct v7_val *k = NULL, *v = NULL;
  va_list ap;

  va_start(ap, val_type);
  k = key_type == V7_TYPE_OBJ ?
  va_arg(ap, struct v7_val *) : v7_mkvv(v7, key_type, &ap);
  v = val_type == V7_TYPE_OBJ ?
  va_arg(ap, struct v7_val *) : v7_mkvv(v7, val_type, &ap);
  va_end(ap);

  // TODO: do not leak here
  CHECK(k != NULL && v != NULL, V7_OUT_OF_MEMORY);

  inc_ref_count(k);
  TRY(v7_set2(v7, obj, k, v));
  v7_freeval(v7, k);

  return V7_OK;
}

V7_PRIVATE enum v7_err push_number(struct v7 *v7, double num) {
  TRY(v7_make_and_push(v7, V7_TYPE_NUM));
  v7_init_num(v7_top_val(v7), num);
  return V7_OK;
}

V7_PRIVATE enum v7_err push_bool(struct v7 *v7, int is_true) {
  TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
  v7_init_bool(v7_top_val(v7), is_true);
  return V7_OK;
}

V7_PRIVATE enum v7_err push_string(struct v7 *v7, const char *str,
                                   unsigned long n, int own) {
  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v7_init_str(v7_top_val(v7), str, n, own);
  return V7_OK;
}

V7_PRIVATE enum v7_err push_func(struct v7 *v7, v7_func_t func) {
  TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
  v7_init_func(v7_top_val(v7), func);
  return V7_OK;
}

V7_PRIVATE enum v7_err push_new_object(struct v7 *v7) {
  TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
  v7_set_class(v7_top_val(v7), V7_CLASS_OBJECT);
  return V7_OK;
}

V7_PRIVATE const char *v7_strerror(enum v7_err e) {
  V7_PRIVATE const char *strings[] = {
    "no error", "error", "eval error", "range error", "reference error",
    "syntax error", "type error", "URI error",
    "out of memory", "internal error", "stack overflow", "stack underflow",
    "called non-function", "not implemented", "string literal too long",
    "RegExp error"
  };
  assert(ARRAY_SIZE(strings) == V7_NUM_ERRORS);
  return e >= (int) ARRAY_SIZE(strings) ? "?" : strings[e];
}

V7_PRIVATE struct v7_val **v7_top(struct v7 *v7) {
  return &v7->stack[v7->sp];
}

V7_PRIVATE int v7_sp(struct v7 *v7) {
  return (int) (v7_top(v7) - v7->stack);
}

V7_PRIVATE struct v7_val *v7_top_val(struct v7 *v7) {
  return v7->sp > 0 ? v7->stack[v7->sp - 1] : NULL;
}

V7_PRIVATE enum v7_err v7_push(struct v7 *v7, struct v7_val *v) {
  inc_ref_count(v);
  TRY(inc_stack(v7, 1));
  v7->stack[v7->sp - 1] = v;
  return V7_OK;
}

V7_PRIVATE enum v7_err v7_make_and_push(struct v7 *v7, enum v7_type type) {
  struct v7_val *v = make_value(v7, type);
  CHECK(v != NULL, V7_OUT_OF_MEMORY);
  return v7_push(v7, v);
}

V7_PRIVATE enum v7_err v7_del2(struct v7 *v7, struct v7_val *obj,
  const char *key, unsigned long n) {
  struct v7_val k = str_to_val(key, n);
  struct v7_prop **p;
  CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_ERROR);
  for (p = &obj->props; *p != NULL; p = &p[0]->next) {
    if (cmp(&k, p[0]->key) == 0) {
      struct v7_prop *next = p[0]->next;
      free_prop(v7, p[0]);
      p[0] = next;
      break;
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err do_exec(struct v7 *v7, const char *file_name,
  const char *source_code, int sp) {
  int has_ret = 0;
  struct v7_pstate old_pstate = v7->pstate;
  enum v7_err err = V7_OK;

  v7->pstate.source_code = v7->pstate.pc = source_code;
  v7->pstate.file_name = file_name;
  v7->pstate.line_no = 1;

  // Prior calls to v7_exec() may have left current_scope modified, reset now
  // TODO(lsm): free scope chain
  v7->this_obj = &v7->root_scope;

  next_tok(v7);
  while ((err == V7_OK) && (v7->cur_tok != TOK_END_OF_INPUT)) {
    // Reset stack on each statement
    if ((err = inc_stack(v7, sp - v7->sp)) == V7_OK) {
      err = parse_statement(v7, &has_ret);
    }
  }

  assert(v7->root_scope.proto == &s_global);
  v7->pstate = old_pstate;

  return err;
}

// Convert object to string, push string on stack
V7_PRIVATE enum v7_err toString(struct v7 *v7, struct v7_val *obj) {
  struct v7_val *f = NULL;

  if ((f = v7_get(obj, "toString")) == NULL) {
    f = v7_get(&s_prototypes[V7_CLASS_OBJECT], "toString");
  }
  CHECK(f != NULL, V7_INTERNAL_ERROR);
  TRY(v7_push(v7, f));
  TRY(v7_call2(v7, obj, 0, 0));

  return V7_OK;
}
