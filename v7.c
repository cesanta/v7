// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013-2014 Cesanta Software Limited
// All rights reserved
//
// This library is dual-licensed: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation. For the terms of this
// license, see <http://www.gnu.org/licenses/>.
//
// You are free to use this library under the terms of the GNU General
// Public License, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// Alternatively, you can license this library under a commercial
// license, as set out in <http://cesanta.com/products.html>.

#include "v7.h"

// Forward declarations
static enum v7_err parse_expression(struct v7 *);
static enum v7_err parse_statement(struct v7 *, int *is_return);

#define CHECK(cond, code) do { if (!(cond)) return(code); } while (0)
#define TRY(call) do { enum v7_err e = call; CHECK(e == V7_OK, e); } while (0)

struct v7 *v7_create(void) {
  struct v7 *v7 = NULL;
  size_t i;

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) == NULL) return NULL;

  LL_INIT(&v7->values);

  for (i = 0; i < ARRAY_SIZE(v7->scopes); i++) {
    v7->scopes[i].type = V7_OBJ;
  }

  return v7;
}

static void free_val(struct v7_val *v) {
  if (!v->not_owned) {
    //char buf[1000];
    //DBG(("[%s]", v7_to_string(v, buf, sizeof(buf))));
    if (v->type == V7_OBJ) {
      struct v7_map *p, *tmp;
      for (p = v->v.map; p != NULL; p = tmp) {
        tmp = p->next;
        //free_val(&p->key);
        //free_val(&p->val);
        //free(p);
      }
    } else if (v->type == V7_STR) {
      free(v->v.str.buf);
    } else if (v->type == V7_FUNC) {
      free(v->v.func);
    }
  }
  v->type = V7_UNDEF;
}

void v7_destroy(struct v7 **v7) {
  if (v7 && *v7) {
    size_t i;
    for (i = 0; i < ARRAY_SIZE((*v7)->scopes); i++) {
      free_val(&(*v7)->scopes[i]);
    }
    free(*v7);
    *v7 = NULL;
  }
}

struct v7_val *v7_get_root_namespace(struct v7 *v7) {
  return &v7->scopes[0];
}

static enum v7_err inc_stack(struct v7 *v7, int incr) {
  //struct v7_val *top = v7_top(v7);
  int i;

  CHECK(v7->sp + incr < (int) ARRAY_SIZE(v7->stack), V7_STACK_OVERFLOW);
  CHECK(v7->sp + incr >= 0, V7_STACK_UNDERFLOW);

  // Free values pushed on stack (like string literals and functions)
  for (i = 0; incr < 0 && i < -incr && i < v7->sp; i++) {
    free_val(v7->stack[v7->sp - (i + 1)]);
  }

  v7->sp += incr;
  return V7_OK;
}

struct v7_val **v7_stk(struct v7 *v7) {
  return v7->stack;
}

struct v7_val **v7_top(struct v7 *v7) {
  return &v7->stack[v7->sp];
}

int v7_sp(struct v7 *v7) {
  return v7_top(v7) - v7_stk(v7);
}

static int v7_is_true(const struct v7_val *v) {
  return (v->type == V7_BOOL || v->type == V7_NUM) && v->v.num != 0;
}

static void obj_v7_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_map *kv;
  int n = snprintf(buf, bsiz, "%s", "{");

  for (kv = v->v.map; kv != NULL && n < bsiz - 1; kv = kv->next) {
    if (kv != v->v.map) n += snprintf(buf + n , bsiz - n, "%s", ", ");
    v7_to_string(&kv->key, buf + n, bsiz - n);
    n = strlen(buf);
    n += snprintf(buf + n , bsiz - n, "%s", ": ");
    v7_to_string(&kv->val, buf + n, bsiz - n);
    n = strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "}");
}

const char *v7_to_string(const struct v7_val *v, char *buf, int bsiz) {
  switch (v->type) {
    case V7_NUM:
      snprintf(buf, bsiz, "%.0lf", v->v.num);
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
      obj_v7_to_string(v, buf, bsiz);
      break;
    case V7_REF:
      obj_v7_to_string(v->v.ref, buf, bsiz);
      break;
    case V7_STR:
        snprintf(buf, bsiz, "%.*s", v->v.str.len, v->v.str.buf);
        break;
    case V7_FUNC:
        snprintf(buf, bsiz, "'function%s'", v->v.func);
        break;
    case V7_C_FUNC:
      snprintf(buf, bsiz, "'c_function_at_%p'", v->v.c_func);
      break;
    default:
      snprintf(buf, bsiz, "??");
      break;
  }
  buf[bsiz - 1] = '\0';
  return buf;
}

static char *v7_strdup(const char *ptr, size_t len) {
  char *p = (char *) malloc(len + 1);
  if (p == NULL) return NULL;
  memcpy(p, ptr, len);
  p[len] = '\0';
  return p;
}


struct v7_val *v7_push(struct v7 *v7, enum v7_type type) {
  struct v7_val *v = (struct v7_val *) calloc(1, sizeof(*v));
  v->type = type;
  inc_stack(v7, 1);
  v7->stack[v7->sp - 1] = v;
  return v;
}

static v7_err do_arithmetic_op(struct v7 *v7, int op) {
  struct v7_val **v = v7_top(v7) - 2;

  if (v7->no_exec) return V7_OK;
  CHECK(v[0]->type == V7_NUM && v[1]->type == V7_NUM, V7_TYPE_MISMATCH);

  switch (op) {
    case '+': v[0]->v.num += v[1]->v.num; break;
    case '-': v[0]->v.num -= v[1]->v.num; break;
    case '*': v[0]->v.num *= v[1]->v.num; break;
    case '/': v[0]->v.num /= v[1]->v.num; break;
  }

  TRY(inc_stack(v7, -1));
  return V7_OK;
}

static struct v7_val str_to_val(const char *buf, size_t len) {
  struct v7_val v;
  v.type = V7_STR;
  v.v.str.buf = (char *) buf;
  v.v.str.len = v.v.str.buf_size = len;
  return v;
}

struct v7_val v7_str_to_val(const char *buf) {
  return str_to_val((char *) buf, strlen(buf));
}

static int cmp(const struct v7_val *a, const struct v7_val *b) {
  return a->type == V7_STR &&
    a->type == b->type &&
    a->v.str.len == b->v.str.len &&
    memcmp(a->v.str.buf, b->v.str.buf, a->v.str.len) == 0;
}

struct v7_val *v7_get(struct v7_val *obj, const struct v7_val *key) {
  struct v7_map *m;
  if (obj != NULL && obj->type == V7_REF) obj = obj->v.ref;
  if (obj == NULL || obj->type != V7_OBJ) return NULL;
  for (m = obj->v.map; m != NULL; m = m->next) {
    if (cmp(&m->key, key)) return &m->val;
  }
  return NULL;
}

static void v7_copy(const struct v7_val *src, struct v7_val *dst) {
  *dst = *src;
  if (src->type == V7_STR) {
    dst->v.str.buf = v7_strdup(src->v.str.buf, src->v.str.len);
  } else if (src->type == V7_FUNC) {
    dst->v.func = v7_strdup(src->v.func, strlen(src->v.func));
  }
}

static struct v7_val *vinsert(struct v7_map **h, const struct v7_val *key) {
  struct v7_map *var = (struct v7_map *) calloc(1, sizeof(*var));

  if (var == NULL) return NULL;
  var->val.type = V7_UNDEF;
  v7_copy(key, &var->key);
  var->next = *h;
  *h = var;

  return &var->val;
}

static struct v7_val *find(struct v7 *v7, const struct v7_val *key, int alloc) {
  struct v7_val *val = NULL;
  int i;

  if (v7->no_exec) return NULL;

  // Search for the name, traversing scopes up to the top level scope
  for (i = v7->current_scope; i >= 0; i--) {
    if ((val = v7_get(&v7->scopes[i], key)) != NULL) return val;
  }

  // Not found, create a new variable
  if (alloc) {
    val = vinsert(&v7->scopes[v7->current_scope].v.map, key);
  }

  return val;
}

static struct v7_val *lookup(struct v7 *v7, const char *name, size_t len,
                             int allocate) {
  struct v7_val key = str_to_val(name, len);
  return find(v7, &key, allocate);
}

struct v7_val *v7_set(struct v7_val *obj, struct v7_val *k, struct v7_val *v) {
  struct v7_val *val = NULL;

  if (obj == NULL || obj->type != V7_OBJ) return val;

  // Find attribute inside object
  if ((val = v7_get(obj, k)) == NULL) {
    val = vinsert(&obj->v.map, k);
  }

  if (val != NULL) {
    free_val(val);    // Deallocate previous value
    v7_copy(v, val);  // Set new value
  }

  return val;
}

struct v7_val *v7_set_num(struct v7_val *obj, const char *key, double num) {
  struct v7_val k, v;
  k = str_to_val((char *) key, strlen(key));
  v.type = V7_NUM;
  v.v.num = num;
  return v7_set(obj, &k, &v);
}

struct v7_val *v7_set_str(struct v7_val *obj, const char *key,
                          const char *str, int len) {
  struct v7_val k, v;
  k = str_to_val((char *) key, strlen(key));
  v.type = V7_STR;
  v.v.str.len = v.v.str.buf_size = len;
  v.v.str.buf = v7_strdup(str, len);
  return v7_set(obj, &k, &v);
}

struct v7_val *v7_set_obj(struct v7_val *obj, const char *key) {
  struct v7_val k, v;
  k = str_to_val((char *) key, strlen(key));
  v.type = V7_OBJ;
  v.v.map = NULL;
  return v7_set(obj, &k, &v);
}

struct v7_val *v7_set_func(struct v7_val *obj, const char *key, v7_func_t f) {
  //struct v7_val *val = NULL;
  struct v7_val k, v;
  k = str_to_val((char *) key, strlen(key));
  v.type = V7_C_FUNC;
  v.v.c_func = f;
  return v7_set(obj, &k, &v);
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
      while (s[0] != '\0' && !(s[-1] == '/' && s[-2] == '*')) s++;
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

static int test_token(struct v7 *v7, const char *kw, int kwlen) {
  return kwlen == v7->tok_len && memcmp(v7->tok, kw, kwlen) == 0;
}

//  number      =   { digit }
static enum v7_err parse_num(struct v7 *v7) {
  struct v7_val *value = v7_push(v7, V7_NUM);
  int is_negative = 0;

  if (*v7->cursor == '-') {
    is_negative = 1;
    TRY(match(v7, *v7->cursor));
  }

  CHECK(is_digit(*v7->cursor), V7_SYNTAX_ERROR);
  v7->tok = v7->cursor;
  value->v.num = 0;
  while (is_digit(*v7->cursor)) {
    value->v.num *= 10;
    value->v.num += *v7->cursor++ - '0';
  }

  if (is_negative) {
    value->v.num = -value->v.num;
  }

  v7->tok_len = v7->cursor - v7->tok;
  skip_whitespaces_and_comments(v7);

  return V7_OK;
}

//  identifier  =   letter { letter | digit }
static enum v7_err parse_identifier(struct v7 *v7) {
  CHECK(is_alpha(*v7->cursor) || *v7->cursor == '_', V7_SYNTAX_ERROR);
  v7->tok = v7->cursor;
  v7->cursor++;
  while (is_alnum(*v7->cursor) || *v7->cursor == '_') v7->cursor++;
  v7->tok_len = v7->cursor - v7->tok;
  skip_whitespaces_and_comments(v7);
  return V7_OK;
}

static enum v7_err parse_return_statement(struct v7 *v7) {
  if (*v7->cursor == ';' || *v7->cursor == '}') {
    if (!v7->no_exec) v7_push(v7, V7_UNDEF);
  } else {
    TRY(parse_expression(v7));
  }
  return V7_OK;
}

static enum v7_err parse_compound_statement(struct v7 *v7) {
  if (*v7->cursor == '{') {
    int old_sp = v7->sp;
    match(v7, '{');
    while (*v7->cursor != '}') {
      if (v7_sp(v7) > old_sp) v7->sp = old_sp;
      TRY(parse_statement(v7, NULL));
    }
    match(v7, '}');
  } else {
    TRY(parse_statement(v7, NULL));
  }
  return V7_OK;
}

// function_defition = "function" "(" func_params ")" "{" func_body "}"
static enum v7_err parse_function_definition(struct v7 *v7, struct v7_val **v,
                                             int num_params) {
  int i = 0, old_no_exec = v7->no_exec, old_sp = v7->sp;
  const char *src = v7->cursor;

  // If 'v' (func to call) is NULL, that means we're just parsing function
  // definition to save it's body.
  v7->no_exec = v == NULL;
  TRY(match(v7, '('));

  // Initialize new scope
  if (!v7->no_exec) {
    v7->current_scope++;
    CHECK(v7->current_scope < (int) ARRAY_SIZE(v7->scopes),
          V7_RECURSION_TOO_DEEP);
  }

  while (*v7->cursor != ')') {
    TRY(parse_identifier(v7));
    if (!v7->no_exec && i < num_params) {
      struct v7_val key = str_to_val((char *) v7->tok, v7->tok_len);
      v7_set(&v7->scopes[v7->current_scope], &key, v[i + 1]);
    }
    i++;
    if (!test_and_skip_char(v7, ',')) break;
  }
  TRY(match(v7, ')'));
  TRY(match(v7, '{'));

  while (*v7->cursor != '}') {
    int is_return_statement = 0;
    v7->sp = old_sp;                  // Clean up the stack from prev stmt
    TRY(parse_statement(v7, &is_return_statement));
    if (is_return_statement) break;   // Leave statement value on stack
  }

  if (v7->no_exec) {
    v7_push(v7, V7_FUNC)->v.func = v7_strdup(src, (v7->cursor + 1) - src);
  }
  TRY(match(v7, '}'));

  // Deinitialize scope
  if (!v7->no_exec) {
    free_val(v7->scopes + v7->current_scope);
    v7->current_scope--;
    assert(v7->current_scope >= 0);
  }

  v7->no_exec = old_no_exec;
  return V7_OK;
}

enum v7_err v7_call(struct v7 *v7, int num_args) {
  struct v7_val **top = v7_top(v7), **v = top - (num_args + 1);

  if (v7->no_exec) return V7_OK;
  CHECK(v7->sp > num_args, V7_INTERNAL_ERROR);
  CHECK(v[0]->type == V7_FUNC || v[0]->type == V7_C_FUNC, V7_TYPE_MISMATCH);

  if (v[0]->type == V7_FUNC) {
    const char *src = v7->cursor;

    // Return value will substitute function objest on a stack
    v[0]->type = V7_UNDEF;      // Set return value to 'undefined'
    v7->cursor = v[0]->v.func;  // Move control flow to function body

    TRY(parse_function_definition(v7, v, num_args));  // Execute function body
    v7->cursor = src;           // Return control flow
    if (v7_top(v7) > top) {     // If function body pushed some value on stack,
      v[0][0] = top[0][0];      // use that value as return value  XXX
    }
  } else if (v[0]->type == V7_C_FUNC) {
    v[0]->v.c_func(v7, v7->cur_obj, v[0], v + 1, num_args);
  }

  TRY(inc_stack(v7, -(v7_top(v7) - (v + 1))));  // Clean up stack
  return V7_OK;
}

//  function_call  = expression "(" { expression} ")"
static enum v7_err parse_function_call(struct v7 *v7) {
  struct v7_val **v = v7_top(v7) - 1;
  int num_args = 0;

  if (!v7->no_exec) {
    CHECK(v[0]->type == V7_FUNC || v[0]->type == V7_C_FUNC, V7_TYPE_MISMATCH);
  }

  // Push arguments on stack
  TRY(match(v7, '('));
  while (*v7->cursor != ')') {
    parse_expression(v7);
    test_and_skip_char(v7, ',');
    num_args++;
  }
  TRY(match(v7, ')'));

  if (!v7->no_exec) {
    TRY(v7_call(v7, num_args));
  }
  return V7_OK;
}

static enum v7_err parse_string_literal(struct v7 *v7) {
  char buf[MAX_STRING_LITERAL_LENGTH];
  const char *begin = v7->cursor++;
  struct v7_val *v = v7_push(v7, V7_STR);
  size_t i = 0;

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

  v->v.str.len = v->v.str.buf_size = v7->no_exec ? 0 : i;
  v->v.str.buf = v7->no_exec ? NULL : v7_strdup(buf, v->v.str.len);
  TRY(match(v7, *begin));
  skip_whitespaces_and_comments(v7);

  return V7_OK;
}

static enum v7_err parse_object_literal(struct v7 *v7) {
  v7_push(v7, V7_OBJ)->v.map = NULL;
  TRY(match(v7, '{'));
  while (*v7->cursor != '}') {
    if (*v7->cursor == '\'' || *v7->cursor == '"') {
      TRY(parse_string_literal(v7));
    } else {
      struct v7_val *v;
      TRY(parse_identifier(v7));
      v = v7_push(v7, V7_STR);
      v->v.str.len = v->v.str.buf_size = v7->tok_len;
      v->v.str.buf = v7_strdup(v7->tok, v7->tok_len);
    }
    TRY(match(v7, ':'));
    TRY(parse_expression(v7));
    if (!v7->no_exec) {
      struct v7_val **v = v7_top(v7) - 3;
      CHECK(v[0]->type == V7_OBJ, V7_TYPE_MISMATCH);
      v7_set(v[0], v[1], v[2]);
      v7->sp -= 2;
      //inc_stack(v7, -2);
    }
    test_and_skip_char(v7, ',');
  }
  TRY(match(v7, '}'));
  return V7_OK;
}

// variable = identifier { '.' identifier | '[' expression ']' }
static enum v7_err parse_variable(struct v7 *v7) {
  struct v7_val *ns, *v = v7->no_exec ? NULL : v7_push(v7, V7_UNDEF);

  if (v7->tok_len == 6 && memcmp(v7->tok, "__ns__", 6) == 0) {
    ns = &v7->scopes[v7->current_scope];
  } else {
    struct v7_val key = str_to_val(v7->tok, v7->tok_len);
    ns = find(v7, &key, 0);
  }

  while (*v7->cursor == '.' || *v7->cursor == '[') {
    int ch = *v7->cursor;
    TRY(match(v7, ch));
    if (!v7->no_exec) {
      CHECK(ns != NULL && ns->type == V7_OBJ, V7_SYNTAX_ERROR);
    }

    v7->cur_obj = ns;
    if (ch == '.') {
      TRY(parse_identifier(v7));
      if (!v7->no_exec) {
        struct v7_val key = str_to_val(v7->tok, v7->tok_len);
        ns = v7_get(ns, &key);
      }
    } else {
      TRY(parse_expression(v7));
      TRY(match(v7, ']'));
      if (!v7->no_exec) {
        ns = v7->cur_obj = v7_get(ns, v7_top(v7)[-1]);
        TRY(inc_stack(v7, -1));
      }
    }
  }

  if (v != NULL && ns != NULL) {
    if (ns->type == V7_OBJ) {
      v->type = V7_REF;
      v->v.ref = ns;
    } else {
      *v = *ns;
      v->not_owned = 1;
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

  if (*v7->cursor == '(') {
    TRY(match(v7, '('));
    TRY(parse_expression(v7));
    TRY(match(v7, ')'));
  } else if (*v7->cursor == '\'' || *v7->cursor == '"') {
    TRY(parse_string_literal(v7));
  } else if (*v7->cursor == '{') {
    TRY(parse_object_literal(v7));
  } else if (is_alpha(*v7->cursor) || *v7->cursor == '_') {
    TRY(parse_identifier(v7));
    if (test_token(v7, "this", 4)) {
    } else if (test_token(v7, "null", 4)) {
      v7_push(v7, V7_NULL);
    } else if (test_token(v7, "true", 4)) {
      v7_push(v7, V7_BOOL)->v.num = 1;
    } else if (test_token(v7, "false", 5)) {
      v7_push(v7, V7_BOOL)->v.num = 0;
    } else if (test_token(v7, "function", 8)) {
      TRY(parse_function_definition(v7, NULL, 0));
    } else {
      TRY(parse_variable(v7));
    }
  } else {
    TRY(parse_num(v7));
  }

  if (*v7->cursor == '(') {
    TRY(parse_function_call(v7));
  }

  // Don't leave anything on stack if no execution flag is set
  if (v7->no_exec) {
    v7->sp = old_sp;
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

enum { OP_XX, OP_GT, OP_LT, OP_GE, OP_LE, OP_EQ };

static int is_logical_op(const char *s) {
  switch (s[0]) {
    case '>': return s[1] == '=' ? OP_GE : OP_GT;
    case '<': return s[1] == '=' ? OP_LE : OP_LT;
    case '=': return s[1] == '=' ? OP_EQ : OP_XX;
    default: return OP_XX;
  }
}

static enum v7_err do_logical_op(struct v7 *v7, int op) {
  struct v7_val **v = v7_top(v7) - 2;

  if (v7->no_exec) return V7_OK;
  CHECK(v[0]->type == V7_NUM && v[1]->type == V7_NUM, V7_TYPE_MISMATCH);

  switch (op) {
    case OP_GT: v[0]->v.num = v[0]->v.num >  v[1]->v.num ? 1 : 0; break;
    case OP_GE: v[0]->v.num = v[0]->v.num >= v[1]->v.num ? 1 : 0; break;
    case OP_LT: v[0]->v.num = v[0]->v.num <  v[1]->v.num ? 1 : 0; break;
    case OP_LE: v[0]->v.num = v[0]->v.num <= v[1]->v.num ? 1 : 0; break;
    case OP_EQ: v[0]->v.num = v[0]->v.num == v[1]->v.num ? 1 : 0; break;
  }

  TRY(inc_stack(v7, -1));
  return V7_OK;
}

//  expression  =   term { add_op term } |
//                  expression "?" expression ":" expression
//                  expression logical_op expression
//                  variable "=" expression
//  add_op      =   "+" | "-"
static enum v7_err parse_expression(struct v7 *v7) {
#ifdef V7_DEBUG
  const char *stmt_str = v7->cursor;
#endif
  struct v7_val *cur_obj;
  int op;

  v7->cur_obj = &v7->scopes[v7->current_scope];

  TRY(parse_term(v7));
  cur_obj = v7->cur_obj;

  while (*v7->cursor == '-' || *v7->cursor == '+') {
    int ch = *v7->cursor;
    TRY(match(v7, ch));
    TRY(parse_term(v7));
    TRY(do_arithmetic_op(v7, ch));
  }

  if ((op = is_logical_op(v7->cursor)) > OP_XX) {
    v7->cursor += op == OP_LT || op == OP_GT ? 1 : 2;
    skip_whitespaces_and_comments(v7);
    TRY(parse_expression(v7));
    TRY(do_logical_op(v7, op));
  }

  // Parse assignment
  if (*v7->cursor == '=') {
    struct v7_val **top = v7_top(v7);
    struct v7_val key = { {0,0}, V7_STR, 1, { { v7_strdup(v7->tok, v7->tok_len),
      v7->tok_len, v7->tok_len } }};
    TRY(match(v7, '='));
    TRY(parse_expression(v7));

    v7_set(cur_obj, &key, top[0]);
    if (!v7->no_exec) {
      //free_val(top - 2);
      top[-1] = top[0];
      v7->sp--;
    }
  }

  // Parse ternary operator
  if (*v7->cursor == '?') {
    int condition_true = v7_is_true(v7_top(v7)[-1]);
    int old_no_exec = v7->no_exec;

    TRY(match(v7, '?'));
    v7->no_exec = old_no_exec || !condition_true;
    TRY(parse_expression(v7));
    TRY(match(v7, ':'));
    v7->no_exec = old_no_exec || condition_true;
    TRY(parse_expression(v7));
    v7->no_exec = old_no_exec;
  }

#if 0
  if (v7->no_exec && v7->sp > old_sp) {
    inc_stack(v7, old_sp - v7->sp);
  }

  //DBG(("%d [%s] [%.*s]", v7_sp(v7), v7_to_str(v7_top(v7) - 1),
  //     (int) (v7->cursor - stmt_str), stmt_str));
#endif
  return V7_OK;
}

//  declaration =   "var" identifier [ "=" expression ] [ "," { i [ "=" e ] } ]
static enum v7_err parse_declaration(struct v7 *v7) {
  struct v7_val *val;
  int sp = v7_sp(v7);

  do {
    if (v7_sp(v7) > sp) {
      v7->sp = sp;
    }
    TRY(parse_identifier(v7));
    // In case when no assignment is done, just a declaration, we create
    // an undefined variable here.
    val = lookup(v7, v7->tok, v7->tok_len, 1);
    if (*v7->cursor == '=') {
      TRY(match(v7, '='));
      TRY(parse_expression(v7));
      if (val != NULL) {
        *val = v7_top(v7)[-1][0];  // Assignment, set a value. XXX
      }
    }
  } while (test_and_skip_char(v7, ','));

  return V7_OK;
}

static enum v7_err parse_if_statement(struct v7 *v7) {
  int old_no_exec = v7->no_exec;

  TRY(match(v7, '('));
  TRY(parse_expression(v7));
  TRY(match(v7, ')'));
  assert(v7->no_exec || v7->sp > 0);  // Stack may be empty if v7->no_exec
  if (!v7->no_exec && !v7_is_true(v7_top(v7)[-1])) {
    v7->no_exec = 1;
  }
  TRY(parse_compound_statement(v7));
  v7->no_exec = old_no_exec;

  return V7_OK;
}

//  statement  =  declaration | return_statement | if_statement
//                assignment | expression [ ";" ]
static enum v7_err parse_statement(struct v7 *v7, int *is_return_statement) {
  if (*v7->cursor == '_' || is_alpha(*v7->cursor)) {
    TRY(parse_identifier(v7));    // Load identifier into v7->tok, v7->tok_len
    if (test_token(v7, "var", 3)) {
      TRY(parse_declaration(v7));
    } else if (test_token(v7, "return", 6)) {
      TRY(parse_return_statement(v7));
      if (is_return_statement != NULL) *is_return_statement = 1;
    } else if (test_token(v7, "if", 2)) {
      TRY(parse_if_statement(v7));
    } else {
      v7->cursor = v7->tok;
      TRY(parse_expression(v7));
    }
  } else {
    TRY(parse_expression(v7));
  }

  // Skip optional semicolons
  while (*v7->cursor == ';') match(v7, *v7->cursor);

  //return is_return_statement;
  return V7_OK;
}

//  code        =   { statement }
enum v7_err v7_exec(struct v7 *v7, const char *source_code) {
  v7->source_code = v7->cursor = source_code;
  skip_whitespaces_and_comments(v7);

  // The following code may raise an exception and jump to the previous line,
  // returning non-zero from the setjmp() call
  // Prior calls to v7_exec() may have left current_scope modified, reset now
  v7->current_scope = 0;
  v7->sp = 0;

  while (*v7->cursor != '\0') {
    inc_stack(v7, -v7->sp);         // Reset stack on each statement
    TRY(parse_statement(v7, 0));    // Leave the result of last expr on stack
  }

  return V7_OK;
}

enum v7_err v7_exec_file(struct v7 *v7, const char *path) {
  FILE *fp;
  char *p;
  long file_size;
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
    status = v7_exec(v7, p);
    free(p);
  }

  return status;
}

static void stdlib_print(struct v7 *v7, struct v7_val *obj,
                         struct v7_val *result,
                         struct v7_val **args, int num_args) {
  char buf[2000];
  int i;
  (void) v7; (void) obj;
  result->type = V7_UNDEF;
  for (i = 0; i < num_args; i++) {
    printf("%s", v7_to_string(args[i], buf, sizeof(buf)));
  }
}

void v7_init_stdlib(struct v7 *v7) {
  v7_set_func(v7_get_root_namespace(v7), "print", stdlib_print);
}
