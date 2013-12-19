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

#include "ejs.h"

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#ifdef ENABLE_DBG
#define DBG(x) do { printf("%-20s ", __func__); printf x; putchar('\n'); \
  fflush(stdout); } while(0)
#else
#define DBG(x)
#endif

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

// a string.
struct str {
  char *buf;        // Pointer to buffer with string data
  int len;          // String length
  int buf_size;     // Buffer size, should be equal or larger then len
};
#define EMPTY_STR { NULL, 0, 0 }

struct tok {
  //struct ll link;           // Linkage in expression
  struct str str;           // Points to the source code
  int value;                // Token value, one of the TOK_*
  int line_no;              // Line number
  //struct tok *left;
  //struct tok *right;
  //struct tok *parent;
};

// Long tokens that are > 1 character in length
enum {
  TOK_VAR, TOK_EQUAL, TOK_TYPE_EQUAL, TOK_FUNCTION,
  TOK_IF, TOK_ELSE, TOK_ADD_EQUAL, TOK_SUB_EQUAL, TOK_MULTIPLY_EQUAL,
  TOK_DIVIDE_EQUAL, TOK_NULL, TOK_NOT, TOK_AND, TOK_OR, TOK_UNDEFINED,
  TOK_IDENTIFIER, TOK_INTEGER, TOK_FLOAT, TOK_STRING,
  TOK_END
};

// Variable types
enum { TYPE_OBJ, TYPE_INT, TYPE_DBL, TYPE_STR, TYPE_FUNC };

struct var {
  struct ll link;
  char *name;
  unsigned char type;
  union { struct str s; long i; double d; };
};

struct ejs {
  const char *source_code;
  const char *cursor;
  int line_no;              // Line number

  struct ll symbol_table;
  struct ll tokens_head;    // List of parsed tokens
  jmp_buf exception_env;    // Exception environment
  char error_msg[100];      // Error message placeholder
};

static int parse_expr(struct ejs *ejs);  // Forward declaration

static void die(struct ejs *vm, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(vm->error_msg, sizeof(vm->error_msg), fmt, ap);
  va_end(ap);
  vm->error_msg[sizeof(vm->error_msg) - 1] = '\0';  // If vsnprintf fails
  fprintf(stderr, "%s\n", vm->error_msg);

  longjmp(vm->exception_env, 1);
}

struct ejs *ejs_create(void) {
  struct ejs *ejs = NULL;

  if ((ejs = (struct ejs *) calloc(1, sizeof(*ejs))) != NULL) {
    LINKED_LIST_INIT(&ejs->tokens_head);
    LINKED_LIST_INIT(&ejs->symbol_table);
  }

  return ejs;
}

void ejs_destroy(struct ejs **ejs) {
  if (ejs && *ejs) {
    free(*ejs);
    *ejs = NULL;
  }
}

// Delimiters for source code tokenization. 0 means invalid character,
// 1: delimiters, 2: digits, 3: hex digits, 4: letters
static const unsigned char s_delims[128] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, //   0-15
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  16-31
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  32-47   !"#$%&'()*+,-./
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, //  48-62  0122456789:;<=>?
  1, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, //  63-79  @ABCDEFGHIJKLMNO
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 1, //  80-95  PQRSTUVWXYZ[\]^_
  1, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, //  96-111 `abcdefghijklmno
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 0, // 114-147 pqrstuvwzyz{|}~
};
#define DELIM(s)    (s_delims[* (unsigned char *) s])
#define IS_ALPHA(s) (* (unsigned char *) s < 128 && DELIM(s) > 2)
#define IS_ALNUM(s) (* (unsigned char *) s < 128 && DELIM(s) > 1)
#define IS_DIGIT(s) (* (unsigned char *) s < 128 && DELIM(s) == 2)
#define IS_SPACE(s) (*(s)==' ' || *(s)=='\t' || *(s)=='\r' || *(s)=='\n')

// Return a pointer to the next token, or NULL
static const char *skip_to_delimiter(const char *s, int len) {
  if (s == NULL || len <= 0) return NULL;
  for (; len > 0; s++, len--) {
    if (DELIM(s) == 0) return NULL;
    if (DELIM(s) == 1) break;
  }
  return s;
}

static int match_long_token(const char *str, int len, int *value) {
  static const struct { const char *s; int len; int value; } toks[] = {
    { "var",      3, TOK_VAR },
    { "==",       2, TOK_EQUAL },
    { "===",      2, TOK_TYPE_EQUAL },
    { "if",       2, TOK_IF },
    { "else",     4, TOK_ELSE },
    { "function", 8, TOK_FUNCTION },
    { "V",        0, TOK_IDENTIFIER },
    { "D",        0, TOK_INTEGER },
    { NULL,      -1, -1 },
  };
  int i;

  //DBG(("%s: [%.*s]", __func__, len, str));
  for (i = 0; toks[i].s != NULL; i++) {
    if (toks[i].len > 0 && len == toks[i].len && !memcmp(toks[i].s, str, len)) {
      *value = toks[i].value;
      return len;
    } else if (toks[i].s[0] == 'V' && (str[0] == '_' || IS_ALPHA(str))) {
      int k = 1;
      while (k < len && (str[k] == '_' || IS_ALNUM(str + k))) k++;
      *value = TOK_IDENTIFIER;
      return k;
    } else if (toks[i].s[0] == 'D' && IS_DIGIT(str)) {
      int k = 1;
      while (k < len && IS_DIGIT(str + k)) k++;
      *value = TOK_INTEGER;
      return k;
    }
  }

  *value = TOK_END;
  return 0;
}

static int resize_str(struct str *str, int increment) {
  char *p;
  if (str->buf_size < 0 || increment < 0) return 0;
  if (increment == 0) return 1;
  p = (char *) realloc(str->buf, str->buf_size + increment);
  if (p == NULL) return 0;
  str->buf = p;
  str->buf_size += increment;
  return 1;
}

// Parse the buffer, link all tokens into the list head.
// Return total number of tokens, including last TOK_END
static struct tok *tokenize(const char *s, int len) {
  const char *p, *eof = s + len;
  int num_tokens = 0, line_no = 1, increment = 1000 * sizeof(struct tok);
  struct str str = EMPTY_STR;
  struct tok *tok = NULL;

  //for (; (p = skip(s, eof - s)) != NULL; s = p + 1) {
  while (s != NULL && s <= eof) {
    if (s[0] == '\n') line_no++;
    if (s[0] == ' ' || s[0] == '\t' || s[0] == '\r' || s[0] == '\n') {
      s++;
      continue;
    }

    // Resize tokens array if necessary. We keep it in a string.
    assert(str.len <= str.buf_size);
    if (str.len >= str.buf_size && !resize_str(&str, increment)) {
      free(str.buf);
      return NULL;
    }

    // Initialize token
    tok = (struct tok *) (str.buf + num_tokens * sizeof(*tok));
    tok->str.buf = (char *) s;
    tok->line_no = line_no;
    num_tokens++;
    str.len = num_tokens * sizeof(*tok);

    // Find where next token starts
    p = skip_to_delimiter(s, eof - s);
    //DBG(("skip(%.*s) -> [%.*s]", eof - s, s, p ? p - s : eof - s, s));

    if (p == s) {
      tok->str.len = 1;
      tok->value = *s++;
    } else {
      tok->str.len = match_long_token(s, p ? p - s : eof - s, &tok->value);
      s += tok->str.len;
    }
    //DBG(("[%.*s] -> %d", tok->str.len, tok->str.buf, tok->value));
    if (tok->value == TOK_END) break;
  }

  return (struct tok *) str.buf;
}

#define EXPECT(ejs, cond) do { if (!(cond)) \
  die((ejs), "[%.*s]: %s", 10, (ejs)->cursor, #cond); } while (0)
#define IS(ejs, ch) (*(ejs)->cursor == (ch))

static void skip_whitespaces_and_comments(struct ejs *ejs) {
  if (IS_SPACE(ejs->cursor)) {
    while (ejs->cursor != '\0' && IS_SPACE(ejs->cursor)) {
      if (*ejs->cursor == '\n') ejs->line_no++;
      ejs->cursor++;
    }
    if (ejs->cursor[0] == '/' && ejs->cursor[1] == '/') {
      ejs->cursor += 2;
      while (*ejs->cursor != '\0' && *ejs->cursor != '\n') ejs->cursor++;
    }
  }
}

static void match(struct ejs *ejs, int ch) {
  EXPECT(ejs, *ejs->cursor++ == ch);
  skip_whitespaces_and_comments(ejs);
}

static int parse_num(struct ejs *ejs) {
  int result = 0;

  EXPECT(ejs, IS_DIGIT(ejs->cursor));
  while (IS_DIGIT(ejs->cursor)) {
    result *= 10;
    result += *ejs->cursor++ - '0';
  }
  skip_whitespaces_and_comments(ejs);

  return result;
}

static int parse_factor(struct ejs *ejs) {
  int result;

  if (*ejs->cursor == '(') {
    match(ejs, '(');
    result = parse_expr(ejs);
    match(ejs, ')');
  } else {
    result = parse_num(ejs);
    while (*ejs->cursor == '*' || *ejs->cursor == '/') {
      if (*ejs->cursor == '*') {
        match(ejs, '*');
        result *= parse_num(ejs);
      } else if (*ejs->cursor == '/') {
        match(ejs, '/');
        result /= parse_num(ejs);
      }
    }
  }

  return result;
}

static int parse_identifier(struct ejs *ejs) {
  EXPECT(ejs, IS_ALPHA(ejs->cursor) || *ejs->cursor == '_');
  ejs->cursor++;
  while (IS_ALNUM(ejs->cursor) || *ejs->cursor == '_') ejs->cursor++;
  return 0;
}

static int parse_expr(struct ejs *ejs) {
  int result = parse_factor(ejs);

  while (*ejs->cursor == '-' || *ejs->cursor == '+') {
    if (*ejs->cursor == '+') {
      match(ejs, '+');
      result += parse_factor(ejs);
    } else if (*ejs->cursor == '-') {
      match(ejs, '-');
      result -= parse_factor(ejs);
    }
  }

  return result;
}

static int parse_statement(struct ejs *ejs) {
  int result = parse_expr(ejs);
  match(ejs, ';');
  return result;
}

//                              GRAMMAR
//
//  code        =   { statement } ;
//  statement   =   [ "var" identifier assign_op ] expression ";"
//  expression  =   term { addop term }
//  term        =   factor { mulop factor }
//  factor      =   number | string_literal | "(" expression ")" | identifier
//  mul_op      =   "*" | "/"
//  add_op      =   "+" | "-"
//  assign_op   =   "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "^="
//  identifier  =   letter { letter | digit }
//  number      =   [ "-" ] { digit }
int ejs_exec2(struct ejs *ejs, const char *source_code, int *result) {
  ejs->source_code = ejs->cursor = source_code;
  skip_whitespaces_and_comments(ejs);
  if (setjmp(ejs->exception_env) != 0) return 0;  // Catches exception
  while (*ejs->cursor != '\0') {
    *result = parse_statement(ejs);
  }
  return 1;
}

int ejs_exec(struct ejs *ejs, const char *str, int size) {
  struct tok *arr;

  if (setjmp(ejs->exception_env) != 0) return 0;  // Catches exception
  arr = tokenize(str, size);
  die(ejs, "[%.*s]: %s", size, str, "ejs_exec is not implemented");

  return 1;
}
