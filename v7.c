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

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif

#ifdef V7_DEBUG
#define DBG(x) do { printf("%-20s ", __func__); printf x; putchar('\n'); \
  fflush(stdout); } while(0)
#else
#define DBG(x)
#endif

#define MAX_STRING_LITERAL_LENGTH 500
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define EXPECT(v7, cond, code) do { if (!(cond)) \
  raise_exception((v7), (code)); } while (0)

struct v7 {
  struct v7_val stack[200];
  int sp;                   // Stack pointer
  struct v7_val scopes[20]; // Namespace objects (scopes)
  int current_scope;        // Pointer to the current scope

  const char *source_code;  // Pointer to the source code string
  const char *cursor;       // Current parsing position
  int line_no;              // Line number
  int no_exec;              // No-execute flag. For parsing function defs
  const char *tok;          // Parsed terminal token (ident, number, string)
  int tok_len;              // Length of the parsed terminal token
  struct v7_val *cur_obj;   // Current namespace object ('x=1; x.y=1;', etc)

  jmp_buf exception_env;    // Exception environment
};

// Forward declarations
static void parse_expression(struct v7 *);
static int parse_statement(struct v7 *);

static void raise_exception(struct v7 *v7, int error_code) {
  // Reset scope to top level to make subsequent v7_exec() valid
  v7->current_scope = 0;
  longjmp(v7->exception_env, error_code);
}

struct v7 *v7_create(void) {
  struct v7 *v7 = NULL;
  size_t i;

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) != NULL) {
    for (i = 0; i < ARRAY_SIZE(v7->scopes); i++) {
      v7->scopes[i].type = V7_OBJ;
    }
  }

  return v7;
}

static void free_val(struct v7_val *v) {
  DBG(("%s %p", __func__, v));
  if (v->type == V7_OBJ) {
    struct v7_map *p, *tmp;
    for (p = v->v.map; p != NULL; p = tmp) {
      tmp = p->next;
      free_val(&p->key);
      free_val(&p->val);
      free(p);
    }
    v->v.map = NULL;
  }
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

static struct v7_val *inc_stack(struct v7 *v7, int incr) {
  v7->sp += incr;
  if (v7->sp >= (int) ARRAY_SIZE(v7->stack)) {
    raise_exception(v7, V7_STACK_OVERFLOW);
  } else if (v7->sp < 0) {
    raise_exception(v7, V7_STACK_UNDERFLOW);
  }

  return &v7->stack[v7->sp - incr];
}

struct v7_val *v7_stk(struct v7 *v7) {
  return v7->stack;
}

struct v7_val *v7_top(struct v7 *v7) {
  return &v7->stack[v7->sp];
}

int v7_sp(struct v7 *v7) {
  return v7_top(v7) - v7_stk(v7);
}

static int v7_is_true(const struct v7_val *v) {
  return (v->type == V7_BOOL || v->type == V7_NUM) && v->v.num != 0;
}

static const char *to_string(const struct v7_val *v, char *buf, size_t bsiz) {
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
      {
        const struct v7_map *kv;
        size_t n = snprintf(buf, bsiz, "%s", "{ ");
        char b1[500], b2[500];
        for (kv = v->v.map; kv != NULL; kv = kv->next) {
          n += snprintf(buf + n , bsiz - n, "%s%s: %s",
                        kv == v->v.map ? "" : ", ",
                        to_string(&kv->key, b1, sizeof(b1)),
                        to_string(&kv->val, b2, sizeof(b2)));
        }
        n += snprintf(buf + n, bsiz - n, "%s", " }");
      }
      break;
    case V7_STR:
        snprintf(buf, bsiz, "%.*s", v->v.str.len, v->v.str.buf);
        break;
    case V7_FUNC:
        snprintf(buf, bsiz, "function%s", v->v.func);
        break;
    case V7_C_FUNC:
      snprintf(buf, bsiz, "'c_function_at_%p'", v->v.c_func);
      break;
    default:
      snprintf(buf, bsiz, "??");
      break;
  }

  return buf;
}

static const char *v7_to_str(const struct v7_val *v) {
  static char buf[500];
  return to_string(v, buf, sizeof(buf));
}

static char *v7_strdup(const char *ptr, size_t len) {
  char *p = (char *) malloc(len + 1);
  if (p == NULL) return NULL;
  memcpy(p, ptr, len);
  p[len] = '\0';
  return p;
}

struct v7_val *v7_push(struct v7 *v7, enum v7_type type) {
  struct v7_val *v = inc_stack(v7, 1);
  v->type = type;
  v->v.map = NULL;
  return v;
}

static void do_arithmetic_op(struct v7 *v7, int op) {
  struct v7_val *v = v7_top(v7) - 2;

  if (v7->no_exec) return;
  if (v->type != V7_NUM || v[1].type != V7_NUM) {
    raise_exception(v7, V7_TYPE_MISMATCH);
  }

  switch (op) {
    case '+': v[0].v.num += v[1].v.num; break;
    case '-': v[0].v.num -= v[1].v.num; break;
    case '*': v[0].v.num *= v[1].v.num; break;
    case '/': v[0].v.num /= v[1].v.num; break;
  }

  inc_stack(v7, -1);
}

static struct v7_val str_to_val(char *buf, size_t len) {
  struct v7_val v;
  v.type = V7_STR;
  v.v.str.buf = buf;
  v.v.str.len = v.v.str.buf_size = len;
  return v;
}

static int cmp(const struct v7_val *a, const struct v7_val *b) {
  return a->type == V7_STR &&
    a->type == b->type &&
    a->v.str.len == b->v.str.len &&
    memcmp(a->v.str.buf, b->v.str.buf, a->v.str.len) == 0;
}

static struct v7_val *vlookup(struct v7_map *v, const struct v7_val *key) {
  for (; v != NULL; v = v->next) if (cmp(&v->key, key)) return &v->val;
  return NULL;
}

static struct v7_val *vinsert(struct v7_map **h, const struct v7_val *key) {
  struct v7_map *var = (struct v7_map *) calloc(1, sizeof(*var));

  if (var == NULL) return NULL;
  var->val.type = V7_UNDEF;
  var->key = *key;
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
    if ((val = vlookup(v7->scopes[i].v.map, key)) != NULL) return val;
  }

  // Not found, create a new variable
  if (alloc) {
    val = vinsert(&v7->scopes[v7->current_scope].v.map, key);
  }

  return val;
}

static struct v7_val *lookup(struct v7 *v7, const char *name, size_t len,
                             int allocate) {
  struct v7_val key = str_to_val((char *) name, len);
  return find(v7, &key, allocate);
}

static void free_value(struct v7_val *v) {
  // TODO(lsm): free strings, functions, objects
  v->type = V7_UNDEF;
}

struct v7_val *v7_set(struct v7_val *obj, struct v7_val *k, struct v7_val *v) {
  struct v7_val *val = NULL;

  if (obj == NULL || obj->type != V7_OBJ) return val;

  // Find attribute inside object
  if ((val = vlookup(obj->v.map, k)) == NULL) {
    val = vinsert(&obj->v.map, k);
  }

  if (val != NULL) {
    free_value(val);  // Deallocate previously held value
    *val = *v;      // Assign new value
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
  v.v.str.len = len;
  v.v.str.buf = (char *) str;
  return v7_set(obj, &k, &v);
}

struct v7_val *v7_set_obj(struct v7_val *obj, const char *key) {
  struct v7_val k, v;
  k = str_to_val((char *) key, strlen(key));
  v.type = V7_OBJ;
  v.v.map = NULL;
  return v7_set(obj, &k, &v);
}

struct v7_val *v7_set_func(struct v7 *v7, const char *name, v7_func_t f) {
  enum v7_err error_code = V7_OK;
  struct v7_val *val = NULL;

  if ((val = lookup(v7, name, strlen(name), 1)) != NULL) {
    val->type = V7_C_FUNC;
    val->v.c_func = f;
  }

  return val;
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
  const char *s = v7->cursor;
  if (is_space(*s)) {
    while (*s != '\0' && is_space(*s)) {
      if (*s == '\n') v7->line_no++;
      s++;
    }
    if (s[0] == '/' && s[1] == '/') {
      s += 2;
      while (*s != '\0' && *s != '\n') s++;
    }
  }
  v7->cursor = s;
}

static void match(struct v7 *v7, int ch) {
  EXPECT(v7, *v7->cursor++ == ch, V7_SYNTAX_ERROR);
  skip_whitespaces_and_comments(v7);
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
static void parse_num(struct v7 *v7) {
  struct v7_val *value = v7_push(v7, V7_NUM);
  int is_negative = 0;

  if (*v7->cursor == '-') {
    is_negative = 1;
    match(v7, *v7->cursor);
  }

  EXPECT(v7, is_digit(*v7->cursor), V7_SYNTAX_ERROR);
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
}

//  identifier  =   letter { letter | digit }
static void parse_identifier(struct v7 *v7) {
  EXPECT(v7, is_alpha(*v7->cursor) || *v7->cursor == '_', V7_SYNTAX_ERROR);
  v7->tok = v7->cursor;
  v7->cursor++;
  while (is_alnum(*v7->cursor) || *v7->cursor == '_') v7->cursor++;
  v7->tok_len = v7->cursor - v7->tok;
  skip_whitespaces_and_comments(v7);
}

static void parse_return_statement(struct v7 *v7) {
  if (*v7->cursor == ';' || *v7->cursor == '}') {
    if (!v7->no_exec) v7_push(v7, V7_UNDEF);
  } else {
    parse_expression(v7);
  }
}

static void parse_compound_statement(struct v7 *v7) {
  if (*v7->cursor == '{') { 
    int old_sp = v7->sp;
    match(v7, '{');
    while (*v7->cursor != '}') {
      if (v7_sp(v7) > old_sp) v7->sp = old_sp;
      parse_statement(v7);
    }
    match(v7, '}');
  } else {
    parse_statement(v7);
  }
}

// function_defition = "function" "(" func_params ")" "{" func_body "}"
static void parse_function_definition(struct v7 *v7, struct v7_val *v,
                                      int num_params) {
  int i = 0, old_no_exec = v7->no_exec, old_sp = v7->sp;
  const char *src = v7->cursor;
  
  // If 'v' (func to call) is NULL, that means we're just parsing function
  // definition to save it's body.
  v7->no_exec = v == NULL;
  match(v7, '(');

  // Initialize new scope
  if (!v7->no_exec) {
    v7->current_scope++;
    EXPECT(v7, v7->current_scope < (int) ARRAY_SIZE(v7->scopes),
           V7_RECURSION_TOO_DEEP);
  }

  while (*v7->cursor != ')') {
    parse_identifier(v7);
    if (!v7->no_exec && i < num_params) {
      struct v7_val key = str_to_val((char *) v7->tok, v7->tok_len);
      v7_set(&v7->scopes[v7->current_scope], &key, &v[i + 1]);
    }
    i++;
    if (!test_and_skip_char(v7, ',')) break;
  }
  match(v7, ')');
  match(v7, '{');
  
  while (*v7->cursor != '}') {
    v7->sp = old_sp;                // Clean up the stack from prev stmt
    if (parse_statement(v7)) break; // Leave statement value on stack
  }
  
  if (v7->no_exec) {
    v7_push(v7, V7_FUNC)->v.func = v7_strdup(src, (v7->cursor + 1) - src);
  }
  match(v7, '}');
  
  // Deinitialize scope
  if (!v7->no_exec) {
    v7->current_scope--;
    assert(v7->current_scope >= 0);
  }

  v7->no_exec = old_no_exec;
}

void v7_call(struct v7 *v7, struct v7_val *v, int num_params) {
  if (v == NULL || v7->no_exec) return;
  if (v->type == V7_FUNC) {
    struct v7_val *val = v7_top(v7);
    const char *src = v7->cursor;

    // Return value will substitute function objest on a stack
    v->type = V7_UNDEF;       // Set return value to 'undefined'
    v7->cursor = v->v.func;   // Move control flow to function body

    parse_function_definition(v7, v, num_params);  // Execute function body
    v7->cursor = src;         // Return control flow
    if (v7_top(v7) > val) {   // If function body pushed some value on stack,
      *v = *val;              // use that value as return value
    }
  } else if (v->type == V7_C_FUNC) {
    // C function, it must clean up the stack and push the result
    v->v.c_func(v7, v, v + 1, v7_top(v7) - v - 1);
  }
  v7->sp = v - v7->stack + 1;
}

//  function_call  = expression "(" { expression} ")"
static void parse_function_call(struct v7 *v7) {
  struct v7_val *v = v7_top(v7) - 1;
  int num_params = 0;

  if (!v7->no_exec) {
    EXPECT(v7, v->type == V7_FUNC || v->type == V7_C_FUNC, V7_TYPE_MISMATCH);
  }

  // Push arguments on stack
  match(v7, '(');
  while (*v7->cursor != ')') {
    parse_expression(v7);
    num_params++;
    test_and_skip_char(v7, ',');
  }
  match(v7, ')');

  if (!v7->no_exec) {
    v7_call(v7, v, num_params);
  }
}

static void parse_string_literal(struct v7 *v7) {
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

  v->v.str.len = v->v.str.buf_size = i;
  v->v.str.buf = v7_strdup(buf, v->v.str.len);
  match(v7, *begin);
  skip_whitespaces_and_comments(v7);
}

static void parse_object_literal(struct v7 *v7) {
  struct v7_val *v;

  v7_push(v7, V7_OBJ)->v.map = NULL;
  match(v7, '{');
  while (*v7->cursor != '}') {
    if (*v7->cursor == '\'' || *v7->cursor == '"') {
      parse_string_literal(v7);
    } else {
      parse_identifier(v7);
      v = v7_push(v7, V7_STR);
      v->v.str.len = v->v.str.buf_size = v7->tok_len;
      v->v.str.buf = (char *) v7->tok;
    }
    match(v7, ':');
    parse_expression(v7);
    if (!v7->no_exec) {
      v = v7_top(v7) - 3;
      EXPECT(v7, v->type == V7_OBJ, V7_TYPE_MISMATCH);
      v7_set(v, v + 1, v + 2);
      inc_stack(v7, -2);
    }
    test_and_skip_char(v7, ',');
  }
  match(v7, '}');
}

// variable = identifier { '.' identifier | '[' expression ']' }
static void parse_variable(struct v7 *v7) {
  struct v7_val *ns, *v = v7->no_exec ? NULL : v7_push(v7, V7_UNDEF);
  struct v7_val key = str_to_val((char *) v7->tok, v7->tok_len);

  if (v7->tok_len == 6 && memcmp(v7->tok, "__ns__", 6) == 0) {
    ns = &v7->scopes[v7->current_scope];
  } else {
    ns = find(v7, &key, 0);
  }

  while (*v7->cursor == '.' || *v7->cursor == '[') {
    int ch = *v7->cursor;
    match(v7, ch);
    if (!v7->no_exec) {
      EXPECT(v7, ns != NULL && ns->type == V7_OBJ, V7_SYNTAX_ERROR);
    }

    v7->cur_obj = ns;
    if (ch == '.') {
      parse_identifier(v7);
      if (!v7->no_exec) {
        key = str_to_val((char *) v7->tok, v7->tok_len);
        ns = vlookup(ns->v.map, &key);
      }
    } else {
      parse_expression(v7);
      match(v7, ']');
      if (!v7->no_exec) {
        ns = v7->cur_obj = vlookup(ns->v.map, v7_top(v7) - 1);
        inc_stack(v7, -1);
      }
    }
  }

  if (!v7->no_exec) {
    if (ns != NULL) {
      *v = *ns;
    } else {
      v->type = V7_UNDEF;
    }
  }  
}

//  factor  =   number | string_literal | "(" expression ")" |
//              variable | "this" | "null" | "true" | "false" |
//              "{" object_literal "}" |
//              "[" array_literal "]" |
//              function_definition |
//              function_call
static void parse_factor(struct v7 *v7) {
  int old_sp = v7_sp(v7);

  if (*v7->cursor == '(') {
    match(v7, '(');
    parse_expression(v7);
    match(v7, ')');
  } else if (*v7->cursor == '\'' || *v7->cursor == '"') {
    parse_string_literal(v7);
  } else if (*v7->cursor == '{') {
    parse_object_literal(v7);
  } else if (is_alpha(*v7->cursor) || *v7->cursor == '_') {
    parse_identifier(v7);
    if (test_token(v7, "this", 4)) {
    } else if (test_token(v7, "null", 4)) {
      v7_push(v7, V7_NULL);
    } else if (test_token(v7, "true", 4)) {
      v7_push(v7, V7_BOOL)->v.num = 1;
    } else if (test_token(v7, "false", 5)) {
      v7_push(v7, V7_BOOL)->v.num = 0;
    } else if (test_token(v7, "function", 8)) {
      parse_function_definition(v7, NULL, 0);
    } else {
      parse_variable(v7);
    }
  } else {
    parse_num(v7);
  }

  if (*v7->cursor == '(') {
    parse_function_call(v7);
  }

  // Don't leave anything on stack if no execution flag is set
  if (v7->no_exec) {
    v7->sp = old_sp;
  }
}

//  term        =   factor { mul_op factor }
//  mul_op      =   "*" | "/"
static void parse_term(struct v7 *v7) {
  parse_factor(v7);
  while (*v7->cursor == '*' || *v7->cursor == '/') {
    int ch = *v7->cursor;
    match(v7, ch);
    parse_factor(v7);
    do_arithmetic_op(v7, ch);
  }
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

static void do_logical_op(struct v7 *v7, int op) {
  struct v7_val *v = v7_top(v7) - 2;

  if (v7->no_exec) return;
  if (v->type != V7_NUM || v[1].type != V7_NUM) {
    raise_exception(v7, V7_TYPE_MISMATCH);
  }

  switch (op) {
    case OP_GT: v[0].v.num = v[0].v.num >  v[1].v.num ? 1 : 0; break;
    case OP_GE: v[0].v.num = v[0].v.num >= v[1].v.num ? 1 : 0; break;
    case OP_LT: v[0].v.num = v[0].v.num <  v[1].v.num ? 1 : 0; break;
    case OP_LE: v[0].v.num = v[0].v.num <= v[1].v.num ? 1 : 0; break;
    case OP_EQ: v[0].v.num = v[0].v.num == v[1].v.num ? 1 : 0; break;
  }

  inc_stack(v7, -1);
}

//  expression  =   term { add_op term } |
//                  expression "?" expression ":" expression
//                  expression logical_op expression
//                  variable "=" expression
//  add_op      =   "+" | "-"
static void parse_expression(struct v7 *v7) {
#ifdef V7_DEBUG
  const char *stmt_str = v7->cursor;
#endif
  struct v7_val key, *cur_obj;
  int op;

  v7->cur_obj = &v7->scopes[v7->current_scope];
#define DECSTK(v7)   if (!v7->no_exec) { v7_top(v7)[-2] = v7_top(v7)[-1]; \
                                          inc_stack(v7, -1); }

  parse_term(v7);
  cur_obj = v7->cur_obj;
  key = str_to_val((char *) v7->tok, v7->tok_len);

  while (*v7->cursor == '-' || *v7->cursor == '+') {
    int ch = *v7->cursor;
    match(v7, ch);
    parse_term(v7);
    do_arithmetic_op(v7, ch);
  }

  if ((op = is_logical_op(v7->cursor)) > OP_XX) {
    v7->cursor += op == OP_LT || op == OP_GT ? 1 : 2;
    skip_whitespaces_and_comments(v7);
    parse_expression(v7);
    do_logical_op(v7, op);
  }

  // Parse assignment
  if (*v7->cursor == '=') {
    match(v7, '=');
    parse_expression(v7);
    v7_set(cur_obj, &key, v7_top(v7) - 1);
    DECSTK(v7);
  }

  // Parse ternary operator
  if (*v7->cursor == '?') {
    int condition_true = v7_is_true(&v7_top(v7)[-1]);
    int old_no_exec = v7->no_exec;

    match(v7, '?');
    v7->no_exec = old_no_exec || !condition_true;
    parse_expression(v7);
    match(v7, ':');
    v7->no_exec = old_no_exec || condition_true;
    parse_expression(v7);
    v7->no_exec = old_no_exec;
  }

  DBG(("%d [%s] [%.*s]", v7_sp(v7), v7_to_str(v7_top(v7) - 1),
       (int) (v7->cursor - stmt_str), stmt_str));
}

//  declaration =   "var" identifier [ "=" expression ] [ "," { i [ "=" e ] } ]
static void parse_declaration(struct v7 *v7) {
  struct v7_val *val;
  int sp = v7_sp(v7);

  do {
    if (v7_sp(v7) > sp) {
      v7->sp = sp;
    }
    parse_identifier(v7);
    val = lookup(v7, v7->tok, v7->tok_len, 1);
    if (*v7->cursor == '=') {
      match(v7, '=');
      parse_expression(v7);
      if (val != NULL) {
        *val = v7_top(v7)[-1];
      }
    }
  } while (test_and_skip_char(v7, ','));
}

static void parse_if_statement(struct v7 *v7) {
  int old_no_exec = v7->no_exec;

  match(v7, '(');
  parse_expression(v7);
  match(v7, ')');
  if (!v7_is_true(&v7_top(v7)[-1])) {
    v7->no_exec = 1;
  }
  parse_compound_statement(v7);
  v7->no_exec = old_no_exec;
}

//  statement  =  declaration | return_statement | if_statement
//                assignment | expression [ ";" ]
static int parse_statement(struct v7 *v7) {
  int is_return_statement = 0;

  if (*v7->cursor == '_' || is_alpha(*v7->cursor)) {
    parse_identifier(v7);    // Load identifier into v7->tok, v7->tok_len
    if (test_token(v7, "var", 3)) {
      parse_declaration(v7);
    } else if (test_token(v7, "return", 6)) {
      parse_return_statement(v7);
      is_return_statement = 1;
    } else if (test_token(v7, "if", 2)) {
      parse_if_statement(v7);
    } else {
      v7->cursor = v7->tok;
      parse_expression(v7);
    }
  } else {
    parse_expression(v7);
  }

  // Skip optional semicolons
  while (*v7->cursor == ';') match(v7, *v7->cursor);

  return is_return_statement;
}

//  code        =   { statement }
enum v7_err v7_exec(struct v7 *v7, const char *source_code) {
  enum v7_err error_code = V7_OK;

  v7->source_code = v7->cursor = source_code;
  skip_whitespaces_and_comments(v7);

  // Setup exception environment. This is the exception catching point.
  if ((error_code = (enum v7_err) setjmp(v7->exception_env)) != 0) {
    return error_code;
  }

  // The following code may raise an exception and jump to the previous line,
  // returning non-zero from the setjmp() call
  // Prior calls to v7_exec() may have left current_scope modified, reset now
  v7->current_scope = 0;

  while (*v7->cursor != '\0') {
    v7->sp = 0;           // Reset stack on each statement
    parse_statement(v7);  // Leave the result of last expression on stack
  }

  return error_code;
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

static void stdlib_print(struct v7 *v7, struct v7_val *result,
                         struct v7_val *args, int num_args) {
  int i;
  (void) v7;
  result->type = V7_UNDEF;
  for (i = 0; i < num_args; i++) {
    printf("%s", v7_to_str(args + i));
  }
}

void v7_init_stdlib(struct v7 *v7) {
  v7_set_func(v7, "print", stdlib_print);
}
