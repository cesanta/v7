// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013 Cesanta Software Limited
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

#if 0
// Linked list interface
struct ll { struct ll *prev, *next; };
#define LINKED_LIST_INIT(N)  ((N)->next = (N)->prev = (N))
#define LINKED_LIST_ENTRY(P,T,N)  ((T *)((char *)(P) - offsetof(T, N)))
#define LINKED_LIST_IS_EMPTY(N)  ((N)->next == (N))
#define LINKED_LIST_FOREACH(H,N,T) \
  for (N = (H)->next, T = (N)->next; N != (H); N = (T), T = (N)->next)
#define LINKED_LIST_ADD_TO_FRONT(H,N) do { ((H)->next)->prev = (N); \
  (N)->next = ((H)->next);  (N)->prev = (H); (H)->next = (N); } while (0)
#define LINKED_LIST_ADD_TO_TAIL(H,N) do { ((H)->prev)->next = (N); \
  (N)->prev = ((H)->prev); (N)->next = (H); (H)->prev = (N); } while (0)
#define LINKED_LIST_REMOVE(N) do { ((N)->next)->prev = ((N)->prev); \
  ((N)->prev)->next = ((N)->next); LINKED_LIST_INIT(N); } while (0)
#endif

#ifdef ENABLE_DBG
#define DBG(x) do { printf("%-20s ", __func__); printf x; putchar('\n'); \
  fflush(stdout); } while(0)
#else
#define DBG(x)
#endif

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define EXPECT(l, cond, code) do { if (!(cond)) \
  raise_exception((l)->v7, (code)); } while (0)

// A string.
struct str {
  char *buf;        // Pointer to buffer with string data
  int len;          // String length
};
#define EMPTY_STR { NULL, 0 }

enum { TYPE_NIL, TYPE_OBJ, TYPE_DBL, TYPE_STR, TYPE_FUNC, TYPE_C_FUNC };

struct value {
  unsigned char type;
  union { struct str str; double num; v7_func_t func; } v;
};

struct var {
  struct var *next;
  //struct ll link;
  char *name;
  struct value value;
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
  struct var *vars;         // Variables currently present in this scope
  struct value stack[50];   // Stack, used to calculate expressions
  int sp;                   // Stack pointer
};

struct v7 {
  struct scope scopes[20];
  int current_scope;
  jmp_buf exception_env;    // Exception environment
};

static void parse_expression(struct lexer *);  // Forward declaration

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

void v7_destroy(struct v7 **v7) {
  if (v7 && *v7) {
    free(*v7);
    *v7 = NULL;
  }
}

static int inc_stack(struct v7 *v7, int incr) {
  return v7->scopes[v7->current_scope].sp += incr;
}

static char *v7_strdup(struct v7 *v7, const char *ptr, size_t len) {
  char *p = (char *) malloc(len + 1);
  if (p == NULL) raise_exception(v7, V7_OUT_OF_MEMORY);
  memcpy(p, ptr, len);
  p[len] = '\0';
  return p;
}

static struct var *lookup(struct v7 *v7, const char *name, int allocate) {
  struct var *var = NULL;
  int i;

  // Search for the name, traversing scopes up to the top level scope
  for (i = v7->current_scope; i >= 0; i--) {
    for (var = v7->scopes[i].vars; var != NULL; var = var->next) {
      if (strcmp(var->name, name) == 0) return var;
    }
  }

  // Not found, create a new variable
  if (allocate && (var = (struct var *) calloc(1, sizeof(*var))) == NULL) {
    raise_exception(v7, V7_OUT_OF_MEMORY);
  }

  // Initialize new variable
  var->value.type = TYPE_NIL;
  var->name = v7_strdup(v7, name, strlen(name));

  // Add it to the scope
  var->next = v7->scopes[v7->current_scope].vars;
  v7->scopes[v7->current_scope].vars = var;

  return var;
}

int v7_define_func(struct v7 *v7, const char *name, v7_func_t func) {
  int error_code = V7_OK;
  struct var *var = NULL;

  if ((error_code = setjmp(v7->exception_env)) != 0) return error_code;
  if ((var = lookup(v7, name, 1)) != NULL) {
    var->value.type = TYPE_C_FUNC;
    var->value.v.func = func;
  }

  return error_code;
}

static int char_class(const char *s) {
  // Character classes for source code tokenization.
  // 0 means invalid character, 1: delimiters, 2: digits,
  // 3: hex digits, 4: letters
  static const unsigned char tab[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, //   0-15
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  16-31
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  32-47   !"#$%&'()*+,-./
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, //  48-62  0122456789:;<=>?
    1, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, //  63-79  @ABCDEFGHIJKLMNO
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 1, //  80-95  PQRSTUVWXYZ[\]^_
    1, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, //  96-111 `abcdefghijklmno
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 0  // 114-147 pqrstuvwzyz{|}~
  };
  return tab[* (unsigned char *) s];
}
static int is_alpha(const char *s) { return char_class(s) > 2;  };
static int is_alnum(const char *s) { return char_class(s) > 1;  };
static int is_digit(const char *s) { return char_class(s) == 2; };
static int is_space(const char *s) {
  return *s == ' ' || *s == '\t' || *s == '\r' || *s == '\n';
};

static void skip_whitespaces_and_comments(struct lexer *l) {
  const char *s = l->cursor;
  if (is_space(s)) {
    while (*s != '\0' && is_space(s)) {
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
  EXPECT(l, *l->cursor++ == ch, V7_PARSE_ERROR);
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

static struct scope *current_scope(struct v7 *v7) {
  return &v7->scopes[v7->current_scope];
}

static struct value *current_stack_top(struct v7 *v7) {
  struct scope *scope = current_scope(v7);
  return &scope->stack[scope->sp];
}

static void parse_num(struct lexer *l) {
  struct scope *scope = current_scope(l->v7);
  struct value *value = current_stack_top(l->v7);

  if (scope->sp >= (int) ARRAY_SIZE(scope->stack)) {
    raise_exception(l->v7, V7_STACK_OVERFLOW);
  }
  inc_stack(l->v7, 1);

  value->type = TYPE_DBL;
  value->v.num = 0;

  EXPECT(l, is_digit(l->cursor), V7_PARSE_ERROR);
  l->tok = l->cursor;
  while (is_digit(l->cursor)) {
    value->v.num *= 10;
    value->v.num += *l->cursor++ - '0';
  }
  l->tok_len = l->cursor - l->tok;
  skip_whitespaces_and_comments(l);
}

static void parse_identifier(struct lexer *l) {
  EXPECT(l, is_alpha(l->cursor) || *l->cursor == '_', V7_PARSE_ERROR);
  l->tok = l->cursor;
  l->cursor++;
  while (is_alnum(l->cursor) || *l->cursor == '_') l->cursor++;
  l->tok_len = l->cursor - l->tok;
  //printf("IDENT: [%.*s]\n", l->tok_len, l->tok);
  skip_whitespaces_and_comments(l);
}

static void parse_function_call(struct lexer *l) {
  match(l, '(');
  while (*l->cursor != ')') {
    parse_expression(l);
    if (*l->cursor == ',') match(l, ',');
  }
  match(l, ')');
}

static void parse_factor(struct lexer *l) {
  if (*l->cursor == '(') {
    match(l, '(');
    parse_expression(l);
    match(l, ')');
  } else if (is_alpha(l->cursor)) {
    parse_identifier(l);
    if (*l->cursor == '(') {
      parse_function_call(l);
    }
    inc_stack(l->v7, 1);
  } else {
    parse_num(l);
  }
}

static void parse_term(struct lexer *l) {
  parse_factor(l);
  while (*l->cursor == '*' || *l->cursor == '/') {
    match(l, *l->cursor);
    parse_factor(l);
    inc_stack(l->v7, -1);
  }
}

static void parse_expression(struct lexer *l) {
  parse_term(l);
  while (*l->cursor == '-' || *l->cursor == '+') {
    match(l, *l->cursor);
    parse_term(l);
    inc_stack(l->v7, -1);
  }
}

static void parse_declaration(struct lexer *l) {
  parse_identifier(l);
  EXPECT(l, l->tok_len == 3 && memcmp(l->tok, "var", 3) == 0, V7_PARSE_ERROR);
  do {
    parse_identifier(l);
    match(l, '=');
    parse_expression(l);
  } while (test_and_skip_char(l, ','));
}

static void parse_assignment(struct lexer *l) {
  char name[256];
  struct var *var;

  parse_identifier(l);

  // Save variable name
  EXPECT(l, l->tok_len < (int) sizeof(name) - 1, V7_OUT_OF_MEMORY);
  memcpy(name, l->tok, l->tok_len);
  name[l->tok_len] = '\0';

  match(l, '=');
  parse_expression(l);

  // Do the assignment: get the value from the top of the stack,
  // where parse_expression() should have left calculated value.
  EXPECT(l, current_scope(l->v7)->sp > 0, V7_INTERNAL_ERROR);
  var = lookup(l->v7, name, 1);
  var->value = current_stack_top(l->v7)[-1];
}

static void parse_statement(struct lexer *l) {
  const char *next_tok;

  if (is_alpha(l->cursor)) {
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
}

//                              GRAMMAR
//
//  code        =   { statement }
//  statement   =   declaration | assignment | expression [ ";" ]
//  declaration =   "var" assignment [ "," {assignment} ]
//  assignment  =   identifier "=" expression
//  expression  =   term { add_op term }
//  term        =   factor { mul_op factor }
//  factor      =   number | string | call | "(" expression ")" | identifier
//  call        =   identifier "(" { expression} ")"
//  mul_op      =   "*" | "/"
//  add_op      =   "+" | "-"
//  identifier  =   letter { letter | digit }
//  number      =   { digit }
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

  // The following code may raise an exception and jump back to after setjmp()
  v7->current_scope = 0;
  v7->scopes[0].sp = 1;
  v7->scopes[0].stack[0].type = TYPE_NIL;

  while (*lexer.cursor != '\0') {
    inc_stack(v7, -1);
    assert(inc_stack(v7, 0) == 0);
    //printf("%d [%s]\n", inc_stack(v7, 0), lexer.cursor);
    parse_statement(&lexer);
  }

  return error_code;
}
