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

#include "v7.c"

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

static const char *test_v7_exec(void) {
  struct v7 *v7 = v7_create();

  ASSERT(v7_exec(v7, "") == V7_OK);
  ASSERT(v7_exec(v7, "-2;") == V7_PARSE_ERROR);
  ASSERT(v7_exec(v7, "2()") == V7_PARSE_ERROR);
  ASSERT(v7_exec(v7, " 15 +	2 \r\n * 2  / 1 - 3 * 4 ; ") == V7_OK);
  ASSERT(v7_exec(v7, "( (5  ) );") == V7_OK);
  ASSERT(v7_exec(v7, "(2 + (12 / 4));") == V7_OK);
  ASSERT(v7_exec(v7, "1;2 7") == V7_OK);
  ASSERT(v7_exec(v7, "var x = 12 + 2 - z() + foo(2,3)+ 3 / 4 * y;") == V7_OK);
  ASSERT(v7_exec(v7, "y + 2; x + 3 + 1 z = y() -2;") == V7_OK);
  ASSERT(v7_exec(v7, "1 2 3") == V7_OK);

  v7_destroy(&v7);

  return NULL;
}

static const char *test_v7_destroy(void) {
  struct v7 *v7 = v7_create();
  ASSERT(v7 != NULL);
  v7_destroy(&v7);
  ASSERT(v7 == NULL);
  v7_destroy(NULL);
  return NULL;
}

static const char *run_all_tests(void) {
  RUN_TEST(test_v7_destroy);
  RUN_TEST(test_v7_exec);
  return NULL;
}

int main(void) {
  const char *fail_msg = run_all_tests();
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", static_num_tests);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
