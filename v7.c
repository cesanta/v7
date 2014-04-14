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

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define EXPECT(v7, cond, code) do { if (!(cond)) \
  raise_exception((v7), (code)); } while (0)

// enum { OP_ASSIGN, OP_PUSH };
struct var {
  struct var *next;
  struct v7_value name;
  struct v7_value value;
};

struct scope {
  struct var *vars;           // Namespace (variables) of this scope
  struct v7_value stack[50];  // Stack, used to calculate expressions
  int sp;                     // Stack pointer
};

struct v7 {
  struct scope scopes[20];
  int current_scope;

  const char *source_code;  // Pointer to the source code string
  const char *cursor;       // Current parsing position
  int line_no;              // Line number
  unsigned int flags;
#define NO_EXEC   1         // No-execute flag. For parsing conditionals,
                            // function definitions.
  const char *tok;          // Parsed terminal token (ident, number, string)
  int tok_len;              // Length of the parsed terminal token

  jmp_buf exception_env;    // Exception environment
};

// Forward declarations
static void parse_expression(struct v7 *);
static void parse_statement(struct v7 *);

static void raise_exception(struct v7 *v7, int error_code) {
  // Reset scope to top level to make subsequent v7_exec() valid
  v7->current_scope = 0;
  longjmp(v7->exception_env, error_code);
}

struct v7 *v7_create(void) {
  struct v7 *v7 = NULL;

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) != NULL) {
  }

  return v7;
}

static void free_scope(struct v7 *v7, int scope_index) {
  struct var *p, *tmp;
  for (p = v7->scopes[scope_index].vars; p != NULL; p = tmp) {
    tmp = p->next;
    free(p);
  }
  v7->scopes[scope_index].vars = NULL;
}

void v7_destroy(struct v7 **v7) {
  if (v7 && *v7) {
    free_scope(*v7, 0);
    free(*v7);
    *v7 = NULL;
  }
}

static int v7_no_exec(const struct v7 *v7) {
  return v7->flags & NO_EXEC;
}

static struct scope *current_scope(struct v7 *v7) {
  return &v7->scopes[v7->current_scope];
}

static struct v7_value *inc_stack(struct v7 *v7, int incr) {
  struct scope *scope = current_scope(v7);

  scope->sp += incr;
  if (scope->sp >= (int) ARRAY_SIZE(scope->stack)) {
    raise_exception(v7, V7_STACK_OVERFLOW);
  } else if (scope->sp < 0) {
    raise_exception(v7, V7_STACK_UNDERFLOW);
  }

  return &scope->stack[scope->sp - incr];
}

struct v7_value *v7_bottom(struct v7 *v7) {
  return current_scope(v7)->stack;
}

struct v7_value *v7_top(struct v7 *v7) {
  struct scope *scope = current_scope(v7);
  return &scope->stack[scope->sp];
}

int v7_sp(struct v7 *v7) {
  return v7_top(v7) - v7_bottom(v7);
}

#ifdef V7_DEBUG
static const char *v7_to_str(const struct v7_value *v) {
  static char buf[100];

  switch (v->type) {
    case V7_NUM:
      snprintf(buf, sizeof(buf), "(number) %.0lf", v->v.num);
      break;
    case V7_BOOL:
      snprintf(buf, sizeof(buf), "(boolean) %s", v->v.num ? "true" : "false");
      break;
    case V7_UNDEF:
      snprintf(buf, sizeof(buf), "%s", "undefined");
      break;
    case V7_NULL:
      snprintf(buf, sizeof(buf), "%s", "null");
      break;
    case V7_OBJ:
        snprintf(buf, sizeof(buf), "%s", "(object)");
        break;
    case V7_STR:
        snprintf(buf, sizeof(buf), "(string) [%.*s]",
                 v->v.str.len, v->v.str.buf);
        break;
    default:
      snprintf(buf, sizeof(buf), "??");
      break;
  }

  return buf;
}

static void dump_var(const struct var *var, int depth) {
  const struct var *v;
  int i;

  for (v = var; v != NULL; v = v->next) {
    for (i = 0; i < depth; i++) putchar(' ');
    printf("[%.*s]: ", v->name.v.str.len, v->name.v.str.buf);
    if (v->value.type == V7_OBJ) {
      printf("%s", "{\n");
      dump_var((struct var *) v->value.v.obj, depth + 2);
      for (i = 0; i < depth; i++) putchar(' ');
      printf("%s", "}");
    } else {
      printf("[%s]", v7_to_str(&v->value));
    }
    putchar('\n');
  }
}
#endif

static char *v7_strdup(const char *ptr, size_t len) {
  char *p = (char *) malloc(len + 1);
  if (p == NULL) return NULL;
  memcpy(p, ptr, len);
  p[len] = '\0';
  return p;
}

struct v7_value *v7_push_double(struct v7 *v7, double num) {
  struct v7_value *v = inc_stack(v7, 1);
  v->type = V7_NUM;
  v->v.num = num;
  return v;
}

struct v7_value *v7_push_null(struct v7 *v7) {
  struct v7_value *v = inc_stack(v7, 1);
  v->type = V7_NULL;
  return v;
}

struct v7_value *v7_push_boolean(struct v7 *v7, int val) {
  struct v7_value *v = inc_stack(v7, 1);
  v->type = V7_BOOL;
  v->v.num = val;
  return v;
}

struct v7_value *v7_push_object(struct v7 *v7) {
  struct v7_value *v = inc_stack(v7, 1);
  v->type = V7_OBJ;
  v->v.obj = NULL;
  return v;
}

struct v7_value *v7_push_function(struct v7 *v7, char *code) {
  struct v7_value *v = inc_stack(v7, 1);
  v->type = V7_FUNC;
  v->v.func = code;
  return v;
}

struct v7_value *v7_push_string(struct v7 *v7, const char *str, int len) {  
  struct v7_value *v = inc_stack(v7, 1);
  v->type = V7_STR;
  v->v.str.len = v->v.str.buf_size = 0;
  v->v.str.buf = NULL;
  if (len > 0 && str != NULL) {
    v->v.str.len = v->v.str.buf_size = len;
    v->v.str.buf = v7_strdup(str, len);
  }
  return v;
}

static void do_arithmetic_op(struct v7 *v7, int op) {
  struct v7_value *v = v7_top(v7) - 2;

  if (v7->flags & NO_EXEC) return;
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

static int cmp_strings(const struct v7_value *a, const struct v7_value *b) {
  return a->type == V7_STR &&
    a->type == b->type &&
    a->v.str.len == b->v.str.len &&
    memcmp(a->v.str.buf, b->v.str.buf, a->v.str.len) == 0;
}

static int cmp_strings2(const struct v7_value *a, const char *s, int len) {
  struct v7_value tmp = { V7_STR, { { (char *) s, len, len } } };
  return cmp_strings(a, &tmp);
}

static struct var *vlookup(struct var *v, const char *name, size_t len) {
  for (; v != NULL; v = v->next) {
    if (cmp_strings2(&v->name, name, len)) return v;
  }
  return NULL;
}

static struct var *vinsert(struct var **head, const char *name, size_t len) {
  struct var *var;

  if ((var = (struct var *) calloc(1, sizeof(*var))) == NULL) return NULL;

  // Initialize new variable
  var->value.type = V7_UNDEF;
  var->name.type = V7_STR;
  var->name.v.str.buf = v7_strdup(name, len);
  var->name.v.str.len = var->name.v.str.buf_size = len;

  // Add it to the namespace
  var->next = *head;
  *head = var;

  return var;
}

static struct var *lookup(struct v7 *v7, const char *name, size_t len,
                          int allocate) {
  struct var *var = NULL;
  int i;

  if (v7_no_exec(v7)) return NULL;

  // Search for the name, traversing scopes up to the top level scope
  for (i = v7->current_scope; i >= 0; i--) {
    if ((var = vlookup(v7->scopes[i].vars, name, len)) != NULL) return var;
  }

  // Not found, create a new variable
  if (allocate) {
    var = vinsert(&v7->scopes[v7->current_scope].vars, name, len);
  }

  if (var == NULL) {
    raise_exception(v7, V7_UNDEFINED_VARIABLE);
  }

  return var;
}

// Stack: <object> <name_str> <value>
int v7_assign(struct v7 *v7) {
  struct v7_value *value = v7_top(v7) - 1;
  struct v7_str *name = &value[-1].v.str;
  struct var *var, **vars = (struct var **) &value[-2].v.obj;

  if (v7_no_exec(v7)) return V7_OK;
  if (&value[-2] < v7_bottom(v7)) return V7_STACK_UNDERFLOW;
  if (value[-2].type != V7_OBJ) return V7_TYPE_MISMATCH;
  if (value[-1].type != V7_STR) return V7_TYPE_MISMATCH;

  // Find attribute inside object
  if ((var = vlookup(*vars, name->buf, name->len)) == NULL) {
    var = vinsert(vars, name->buf, name->len);
  }

  // Deallocate previously held value
  assert(var->value.type == V7_UNDEF);
  DBG(("%p [%.*s] -> [%s]", &value[-2], name->len, name->buf,
      v7_to_str(value)));

  // Assign new value
  var->value = *value;

  inc_stack(v7, -2);
  return V7_OK;
}

int v7_define_func(struct v7 *v7, const char *name, v7_func_t c_func) {
  int error_code = V7_OK;
  struct var *var = NULL;

  if ((error_code = setjmp(v7->exception_env)) != 0) return error_code;
  if ((var = lookup(v7, name, strlen(name), 1)) != NULL) {
    var->value.type = V7_C_FUNC;
    var->value.v.c_func = c_func;
  }

  return error_code;
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
  struct v7_value *value = v7_push_double(v7, 0);
  int is_negative = 0;

  if (*v7->cursor == '-') {
    is_negative = 1;
    match(v7, *v7->cursor);
  }

  EXPECT(v7, is_digit(*v7->cursor), V7_SYNTAX_ERROR);
  v7->tok = v7->cursor;
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

//  call        =   identifier "(" { expression} ")"
static void parse_function_call(struct v7 *v7) {
  int num_arguments = 0;
  struct var *func = lookup(v7, v7->tok, v7->tok_len, 0);

  EXPECT(v7, func != NULL, V7_UNDEFINED_VARIABLE);
  EXPECT(v7, func->value.type == V7_FUNC || func->value.type == V7_C_FUNC,
         V7_TYPE_MISMATCH);

  match(v7, '(');
  while (*v7->cursor != ')') {
    parse_expression(v7);
    if (*v7->cursor == ',') match(v7, ',');
    num_arguments++;
  }
  match(v7, ')');

  // Perform a call
  if (func != NULL && func->value.type == V7_C_FUNC && !v7_no_exec(v7)) {
    func->value.v.c_func(v7, num_arguments);
  }
}

static void parse_string_literal(struct v7 *v7) {
  const char *begin = v7->cursor++;
  while (*v7->cursor != *begin && *v7->cursor != '\0') v7->cursor++;
  v7_push_string(v7, begin + 1, v7->cursor - (begin + 1));
  match(v7, *begin);
  skip_whitespaces_and_comments(v7);
}

static void parse_object_literal(struct v7 *v7) {
  v7_push_object(v7);
  match(v7, '{');
  while (*v7->cursor != '}') {
    if (*v7->cursor == '\'' || *v7->cursor == '"') {
      parse_string_literal(v7);
    } else {
      parse_identifier(v7);
      v7_push_string(v7, v7->tok, v7->tok_len);
    }
    match(v7, ':');
    parse_expression(v7);
    v7_assign(v7);
    test_and_skip_char(v7, ',');
  }
  match(v7, '}');
}

// function_defition = "function" "(" func_params ")" "{" func_body "}"
static void parse_function_definition(struct v7 *v7) {
  unsigned old_flags = v7->flags;
  v7->flags |= NO_EXEC;
  v7_push_function(v7, (char *) v7->cursor);
  match(v7, '(');
  while (*v7->cursor != ')') {
    parse_identifier(v7);
    if (!test_and_skip_char(v7, ',')) break;
  }
  match(v7, ')');
  match(v7, '{');
  while (*v7->cursor != '}') {
    parse_statement(v7);
  }
  match(v7, '}');
  v7->flags = old_flags;
}

//  factor  =   number | string | call | "(" expression ")" | identifier |
//              this | null | true | false |
//              "{" object_literal "}" |
//              "[" array_literal "]" |
//              function_defition
static void parse_factor2(struct v7 *v7) {
  int sp = v7_sp(v7);
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
      v7_push_null(v7);
    } else if (test_token(v7, "true", 4)) {
      v7_push_boolean(v7, 1);
    } else if (test_token(v7, "false", 5)) {
      v7_push_boolean(v7, 0);
    } else if (test_token(v7, "function", 8)) {
      parse_function_definition(v7);
    } else if (*v7->cursor == '(') {
      parse_function_call(v7);
    } else {
      struct var *var = lookup(v7, v7->tok, v7->tok_len, 0);
      inc_stack(v7, 1)[0] = var->value;
    }
  } else {
    parse_num(v7);
  }

  // Don't leave anything on stack if no execution flag is set
  if (v7_no_exec(v7)) {
    current_scope(v7)->sp = sp;
  }
}

// factor = factor2 { '.' factor2 } |
//          factor2 { '[' factor2 ']' }
static void parse_factor(struct v7 *v7) {
  parse_factor2(v7);
  while (*v7->cursor == '.' || *v7->cursor == '[') {
    struct v7_value *val = &v7_top(v7)[-1];
    struct var *v = NULL, *obj = (struct var *) val->v.obj;
    int ch = *v7->cursor;

    match(v7, ch);
    if (!v7_no_exec(v7)) {
      EXPECT(v7, val->type == V7_OBJ, V7_SYNTAX_ERROR);
    }

    if (ch == '.') {
      parse_identifier(v7);
      if (!v7_no_exec(v7)) {
        v = vlookup(obj, v7->tok, v7->tok_len);
      }
    } else {
      parse_expression(v7);
      match(v7, ']');
      if (!v7_no_exec(v7)) {
        EXPECT(v7, val == &v7_top(v7)[-2], V7_INTERNAL_ERROR);
        EXPECT(v7, val[1].type == V7_STR, V7_TYPE_MISMATCH);
        v = vlookup(obj, val[1].v.str.buf, val[1].v.str.len);
        inc_stack(v7, -1);
      }
    }

    if (!v7_no_exec(v7)){
      if (v != NULL) {
        *val = v->value;
      } else {
        val->type = V7_UNDEF;
      }
    }
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

//  expression  =   term { add_op term }
//  add_op      =   "+" | "-"
static void parse_expression(struct v7 *v7) {
  parse_term(v7);
  while (*v7->cursor == '-' || *v7->cursor == '+') {
    int ch = *v7->cursor;
    match(v7, ch);
    parse_term(v7);
    do_arithmetic_op(v7, ch);
  }
}

//  assignment  =   identifier "=" expression
static void parse_assignment(struct v7 *v7) {
  struct var *var;
  // NOTE(lsm): Important to lookup just after parse_identifier()
  var = lookup(v7, v7->tok, v7->tok_len, 1);
  match(v7, '=');
  parse_expression(v7);

  // Do the assignment: get the value from the top of the stack,
  // where parse_expression() should have left calculated value.
  EXPECT(v7, current_scope(v7)->sp > 0, V7_INTERNAL_ERROR);
  var->value = v7_top(v7)[-1];
}

//  declaration =   "var" identifier [ "=" expression ] [ "," { i [ "=" e ] } ]
static void parse_declaration(struct v7 *v7) {
  struct var *var;
  int sp = v7_sp(v7);

  do {
    if (v7_sp(v7) > sp) {
      current_scope(v7)->sp = sp;
    }
    parse_identifier(v7);
    var = lookup(v7, v7->tok, v7->tok_len, 1);
    if (*v7->cursor == '=') {
      match(v7, '=');
      parse_expression(v7);
      if (var != NULL) {
        var->value = v7_top(v7)[-1];
      }
    }
  } while (test_and_skip_char(v7, ','));
}

//  statement   =   declaration | assignment | expression [ ";" ]
static void parse_statement(struct v7 *v7) {
#ifdef V7_DEBUG
  const char *stmt_str = v7->cursor;
#endif

  if (is_alpha(*v7->cursor)) {
    parse_identifier(v7);    // Load identifier into v7->tok, v7->tok_len
    if (test_token(v7, "var", 3)) {
      parse_declaration(v7);
    } else if (test_token(v7, "return", 6)) {
      parse_expression(v7);
    } else if (*v7->cursor == '=') {
      parse_assignment(v7);
    } else {
      v7->cursor = v7->tok;
      parse_expression(v7);
    }
  } else {
    parse_expression(v7);
  }

  // Skip optional semicolons
  while (*v7->cursor == ';') match(v7, *v7->cursor);

  DBG(("[%.*s] %d [%s]", (int) (v7->cursor - stmt_str), stmt_str,
       (int) (v7_top(v7) - v7_bottom(v7)),
       v7_to_str(v7_top(v7) - 1)));
}

//  code        =   { statement }
int v7_exec(struct v7 *v7, const char *source_code) {
  int error_code = V7_OK;
  
  v7->source_code = v7->cursor = source_code;
  skip_whitespaces_and_comments(v7);

  // Setup exception environment. This is the exception catching point.
  if ((error_code = setjmp(v7->exception_env)) != 0) return error_code;

  // The following code may raise an exception and jump to the previous line,
  // returning non-zero from the setjmp() call
  v7->current_scope = 0;
  current_scope(v7)->sp = 0;
  v7_push_null(v7);

  while (*v7->cursor != '\0') {
    inc_stack(v7, -1);
    if (current_scope(v7)->sp != 0) raise_exception(v7, V7_INTERNAL_ERROR);
    parse_statement(v7);
  }

  return error_code;
}
