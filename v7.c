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
#define EXPECT(l, cond, code) do { if (!(cond)) \
  raise_exception((l)->v7, (code)); } while (0)

struct var {
  struct var *next;
  struct v7_value name;
  struct v7_value value;
};

struct lexer {
  struct v7 *v7;            // Pointer to v7 context
  const char *source_code;  // Pointer to the source code string
  const char *cursor;       // Current parsing position
  int line_no;              // Line number
  const char *tok;          // Parsed terminal token (ident, number, string)
  int tok_len;              // Length of the parsed terminal token
};

struct scope {
  struct var *vars;           // Namespace (variables) of this scope
  struct v7_value stack[50];  // Stack, used to calculate expressions
  int sp;                     // Stack pointer
};

struct v7 {
  struct scope scopes[20];
  int current_scope;
  jmp_buf exception_env;    // Exception environment
};

// Forward declarations
static void parse_expression(struct lexer *);

static void raise_exception(struct v7 *v7, int error_code) {
  // Reset scope to top level to make subsequent v7_exec() valid
  v7->current_scope = 0;
  longjmp(v7->exception_env, error_code);
}

struct v7 *v7_create(void) {
  struct v7 *v7 = NULL;

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) != NULL) {
    //LINKED_LIST_INIT(&v7->symbol_table);
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

int v7_assign(struct v7 *v7) {
  struct v7_value *value = v7_top(v7) - 1;
  struct v7_str *name = &value[-1].v.str;
  struct var *var, **vars = (struct var **) &value[-2].v.obj;

  if (&value[-2] < v7_bottom(v7)) return V7_STACK_UNDERFLOW;
  if (value[-2].type != V7_OBJ) return V7_TYPE_MISMATCH;
  if (value[-1].type != V7_STR) return V7_TYPE_MISMATCH;

  // Find attribute inside object
  if ((var = vlookup(*vars, name->buf, name->len)) == NULL) {
    var = vinsert(vars, name->buf, name->len);
  }

  // Deallocate previously held value
  assert(var->value.type == V7_UNDEF);
  DBG(("%p [%.*s] -> [%s]", &value[-2], name->len, name->buf, v7_to_str(value)));

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
};

static int is_alnum(int ch) {
  return is_digit(ch) || is_alpha(ch);
}

static int is_space(int ch) {
  return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
};

static void skip_whitespaces_and_comments(struct lexer *l) {
  const char *s = l->cursor;
  if (is_space(*s)) {
    while (*s != '\0' && is_space(*s)) {
      if (*s == '\n') l->line_no++;
      s++;
    }
    if (s[0] == '/' && s[1] == '/') {
      s += 2;
      while (*s != '\0' && *s != '\n') s++;
    }
  }
  l->cursor = s;
}

static void match(struct lexer *l, int ch) {
  EXPECT(l, *l->cursor++ == ch, V7_SYNTAX_ERROR);
  skip_whitespaces_and_comments(l);
}

static int test_and_skip_char(struct lexer *l, int ch) {
  if (*l->cursor == ch) {
    l->cursor++;
    skip_whitespaces_and_comments(l);
    return 1;
  }
  return 0;
}

static int test_token(struct lexer *l, const char *kw, int kwlen) {
  return kwlen == l->tok_len && memcmp(l->tok, kw, kwlen) == 0;
}

//  number      =   { digit }
static void parse_num(struct lexer *l) {
  struct v7_value *value = inc_stack(l->v7, 1);
  int is_negative = 0;

  value->type = V7_NUM;
  value->v.num = 0;

  if (*l->cursor == '-') {
    is_negative = 1;
    match(l, *l->cursor);
  }

  EXPECT(l, is_digit(*l->cursor), V7_SYNTAX_ERROR);
  l->tok = l->cursor;
  while (is_digit(*l->cursor)) {
    value->v.num *= 10;
    value->v.num += *l->cursor++ - '0';
  }

  if (is_negative) {
    value->v.num = -value->v.num;
  }

  l->tok_len = l->cursor - l->tok;
  skip_whitespaces_and_comments(l);
}

//  identifier  =   letter { letter | digit }
static void parse_identifier(struct lexer *l) {
  EXPECT(l, is_alpha(*l->cursor) || *l->cursor == '_', V7_SYNTAX_ERROR);
  l->tok = l->cursor;
  l->cursor++;
  while (is_alnum(*l->cursor) || *l->cursor == '_') l->cursor++;
  l->tok_len = l->cursor - l->tok;
  skip_whitespaces_and_comments(l);
}

//  call        =   identifier "(" { expression} ")"
static void parse_function_call(struct lexer *l) {
  int num_arguments = 0;
  struct var *func = lookup(l->v7, l->tok, l->tok_len, 0);

  EXPECT(l, func != NULL, V7_UNDEFINED_VARIABLE);
  EXPECT(l, func->value.type == V7_FUNC || func->value.type == V7_C_FUNC,
         V7_TYPE_MISMATCH);

  match(l, '(');
  while (*l->cursor != ')') {
    parse_expression(l);
    if (*l->cursor == ',') match(l, ',');
    num_arguments++;
  }
  match(l, ')');

  // Perform a call
  if (func != NULL && func->value.type == V7_C_FUNC) {
    func->value.v.c_func(l->v7, num_arguments);
  }
}


static void parse_string_literal(struct lexer *l) {
  const char *begin = l->cursor++;
  while (*l->cursor != *begin && *l->cursor != '\0') l->cursor++;
  v7_push_string(l->v7, begin + 1, l->cursor - (begin + 1));
  match(l, *begin);
  skip_whitespaces_and_comments(l);
}

static void parse_object_literal(struct lexer *l) {
  v7_push_object(l->v7);
  match(l, '{');
  while (*l->cursor != '}') {
    if (*l->cursor == '\'' || *l->cursor == '"') {
      parse_string_literal(l);
    } else {
      parse_identifier(l);
      v7_push_string(l->v7, l->tok, l->tok_len);
    }
    match(l, ':');
    parse_expression(l);
    v7_assign(l->v7);
    test_and_skip_char(l, ',');
  }
  match(l, '}');
}

//  factor  =   number | string | call | "(" expression ")" | identifier |
//              this | null | true | false |
//              "{" object_literal "}" |
//              "[" array_literal "]"
static void parse_factor2(struct lexer *l) {
  if (*l->cursor == '(') {
    match(l, '(');
    parse_expression(l);
    match(l, ')');
  } else if (*l->cursor == '\'' || *l->cursor == '"') {
    parse_string_literal(l);
  } else if (*l->cursor == '{') {
    parse_object_literal(l);
  } else if (is_alpha(*l->cursor) || *l->cursor == '_') {
    parse_identifier(l);
    if (test_token(l, "this", 4)) {
    } else if (test_token(l, "null", 4)) {
      v7_push_null(l->v7);
    } else if (test_token(l, "true", 4)) {
      v7_push_boolean(l->v7, 1);
    } else if (test_token(l, "false", 5)) {
      v7_push_boolean(l->v7, 0);
    } else if (*l->cursor == '(') {
      parse_function_call(l);
    } else {
      struct var *var = lookup(l->v7, l->tok, l->tok_len, 0);
      inc_stack(l->v7, 1)[0] = var->value;
    }
  } else {
    parse_num(l);
  }
}

// factor = factor2 { '.' factor2 } |
//          factor2 { '[' factor2 ']' }
static void parse_factor(struct lexer *l) {
  struct var *v;
  parse_factor2(l);
  while (*l->cursor == '.' || *l->cursor == '[') {
    struct v7_value *val = &v7_top(l->v7)[-1];
    struct var *v, *obj = (struct var *) val->v.obj;
    int ch = *l->cursor;

    match(l, ch);
    EXPECT(l, val->type == V7_OBJ, V7_SYNTAX_ERROR);

    if (ch == '.') {
      parse_identifier(l);
      v = vlookup(obj, l->tok, l->tok_len);
    } else {
      parse_expression(l);
      match(l, ']');
      EXPECT(l, val == &v7_top(l->v7)[-2], V7_INTERNAL_ERROR);
      EXPECT(l, val[1].type == V7_STR, V7_TYPE_MISMATCH);
      v = vlookup(obj, val[1].v.str.buf, val[1].v.str.len);
      inc_stack(l->v7, -1);
    }

    if (v != NULL) {
      *val = v->value;
    } else {
      val->type = V7_UNDEF;
    }
  }
}

//  term        =   factor { mul_op factor }
//  mul_op      =   "*" | "/"
static void parse_term(struct lexer *l) {
  parse_factor(l);
  while (*l->cursor == '*' || *l->cursor == '/') {
    int ch = *l->cursor;
    match(l, ch);
    parse_factor(l);
    do_arithmetic_op(l->v7, ch);
  }
}

//  expression  =   term { add_op term }
//  add_op      =   "+" | "-"
static void parse_expression(struct lexer *l) {
  parse_term(l);
  while (*l->cursor == '-' || *l->cursor == '+') {
    int ch = *l->cursor;
    match(l, ch);
    parse_term(l);
    do_arithmetic_op(l->v7, ch);
  }
}

//  assignment  =   identifier "=" expression
static void parse_assignment(struct lexer *l) {
  struct var *var;

  parse_identifier(l);
  // NOTE(lsm): Important to lookup just after parse_identifier()
  var = lookup(l->v7, l->tok, l->tok_len, 1);
  match(l, '=');
  parse_expression(l);

  // Do the assignment: get the value from the top of the stack,
  // where parse_expression() should have left calculated value.
  EXPECT(l, current_scope(l->v7)->sp > 0, V7_INTERNAL_ERROR);
  var->value = v7_top(l->v7)[-1];
}

//  declaration =   "var" identifier [ "=" expression ] [ "," { i [ "=" e ] } ]
static void parse_declaration(struct lexer *l) {
  struct var *var;
  parse_identifier(l);
  EXPECT(l, l->tok_len == 3 && memcmp(l->tok, "var", 3) == 0, V7_SYNTAX_ERROR);
  do {
    parse_identifier(l);
    var = lookup(l->v7, l->tok, l->tok_len, 1);
    if (*l->cursor == '=') {
      match(l, '=');
      parse_expression(l);
      var->value = v7_top(l->v7)[-1];
      //inc_stack(l->v7, -1);
    }
  } while (test_and_skip_char(l, ','));
}

//  statement   =   declaration | assignment | expression [ ";" ]
static void parse_statement(struct lexer *l) {
  const char *next_tok;
#ifdef V7_DEBUG
  const char *stmt_str = l->cursor;
#endif

  if (is_alpha(*l->cursor)) {
    // Identifier is ahead of us.
    parse_identifier(l);    // Load identifier into l->tok, l->tok_len
    next_tok = l->cursor;   // Remember the next token
    l->cursor = l->tok;     // Jump back

    if (test_token(l, "var", 3)) {
      parse_declaration(l);
    } else if (*next_tok == '=') {
      parse_assignment(l);
    } else {
      parse_expression(l);
    }
  } else {
    parse_expression(l);
  }

  // Skip optional semicolons
  while (*l->cursor == ';') match(l, *l->cursor);

  DBG(("[%.*s] %d [%s]", (int) (l->cursor - stmt_str), stmt_str,
       (int) (v7_top(l->v7) - v7_bottom(l->v7)),
       v7_to_str(v7_top(l->v7) - 1)));
}

//  code        =   { statement }
int v7_exec(struct v7 *v7, const char *source_code) {
  int error_code = V7_OK;
  struct lexer lexer;

  // Initialize lexer
  memset(&lexer, 0, sizeof(lexer));
  lexer.source_code = lexer.cursor = source_code;
  skip_whitespaces_and_comments(&lexer);
  lexer.v7 = v7;

  // Setup exception environment. This is the exception catching point.
  if ((error_code = setjmp(v7->exception_env)) != 0) return error_code;

  // The following code may raise an exception and jump to the previous line,
  // returning non-zero from the setjmp() call
  v7->current_scope = 0;
  current_scope(v7)->sp = 0;
  v7_push_null(v7);

  while (*lexer.cursor != '\0') {
    inc_stack(v7, -1);
    if (current_scope(v7)->sp != 0) raise_exception(v7, V7_INTERNAL_ERROR);
    parse_statement(&lexer);
  }

  return error_code;
}
