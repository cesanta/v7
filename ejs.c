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
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

// Piece of memory, a string.
struct vec { const char *ptr; int len; };

struct ejs {
  struct ll tokens_head;    // List of parsed tokens
  jmp_buf jmp_buf;          // Exception environment
  char error_msg[100];      // Error message placeholder
};

struct tok {
  struct ll link;           // Linkage in expression
  struct vec vec;           // Points to the source code
  int val;                  // Token value, one of the TOK_*
  int line_no;              // Line number
  struct tok *left;
  struct tok *right;
  struct tok *parent;
};

enum {
  TOK_EOF = 0,
  TOK_INTEGER,
  TOK_FLOAT,
  TOK_IDENTIFIER,
  TOK_ASSIGN,
  TOK_EQUAL,
  TOK_TYPE_EQUAL,
  NUM_TOKENS  // Should be the last element
};

static void die(struct ejs *vm, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(vm->error_msg, sizeof(vm->error_msg), fmt, ap);
  va_end(ap);
  vm->error_msg[sizeof(vm->error_msg) - 1] = '\0';  // If vsnprintf fails

  longjmp(vm->jmp_buf, 1);
}

struct ejs *ejs_create(void) {
  struct ejs *ejs = NULL;

  if ((ejs = calloc(1, sizeof(*ejs))) != NULL) {
  }

  return ejs;
}

void ejs_destroy(struct ejs **ejs) {
  if (ejs && *ejs) {
    free(*ejs);
    *ejs = NULL;
  }
}

int ejs_exec(struct ejs *ejs, const char *str, int size) {
  if (setjmp(ejs->jmp_buf) != 0) return 0;  // Catches exception
  die(ejs, "[%.*s]: %s", size, str, "ejs_exec is not implemented");
  return 1;
}


#ifdef EJS_UNIT_TEST
#define FAIL(str, line) do {                    \
  printf("Fail on line %d: [%s]\n", line, str); \
  return str;                                   \
} while (0)

#define ASSERT(expr) do {                       \
  static_num_tests++;                           \
  if (!(expr)) FAIL(#expr, __LINE__);           \
} while (0)

#define RUN_TEST(test) do { const char *msg = test(); \
  if (msg) return msg; } while (0)

static int static_num_tests = 0;

static const char *test_ejs_exec(void) {
  struct ejs *ejs = ejs_create();
  ASSERT(ejs_exec(ejs, "", 0) == 0);
  ASSERT(ejs->error_msg[0] != '\0');
  ejs_destroy(&ejs);
  return NULL;
}

static const char *test_ejs_destroy(void) {
  struct ejs *ejs = ejs_create();
  ASSERT(ejs != NULL);
  ejs_destroy(&ejs);
  ASSERT(ejs == NULL);
  ejs_destroy(NULL);
  return NULL;
}

static const char *run_all_tests(void) {
  RUN_TEST(test_ejs_destroy);
  RUN_TEST(test_ejs_exec);
  return NULL;
}

int main(void) {
  const char *fail_msg = run_all_tests();
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", static_num_tests);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
