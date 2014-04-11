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
  ASSERT(v7_top(v7) - v7_bottom(v7) == 1);
  ASSERT(v7_top(v7)[-1].type == V7_NULL);

  ASSERT(v7_exec(v7, "-2;") == V7_OK);
  ASSERT(v7_top(v7)[-1].type == V7_NUM);
  ASSERT(v7_top(v7)[-1].v.num == -2);

  ASSERT(v7_exec(v7, "3 + 4") == V7_OK);
  ASSERT(v7_top(v7)[-1].v.num == 7);

  ASSERT(v7_exec(v7, "2()") == V7_SYNTAX_ERROR);
  ASSERT(v7_exec(v7, " 15 +	2 \r\n * 2  / 1 - 3 * 4 ; ") == V7_OK);

  ASSERT(v7_exec(v7, "( (5  ) );") == V7_OK);
  ASSERT(v7_top(v7)[-1].type == V7_NUM);
  ASSERT(v7_top(v7)[-1].v.num == 5);

  ASSERT(v7_exec(v7, "(2 + (12 / 4));") == V7_OK);
  ASSERT(v7_top(v7)[-1].type == V7_NUM);
  ASSERT(v7_top(v7)[-1].v.num == 5);

  ASSERT(v7_exec(v7, "1;2 7") == V7_OK);
  ASSERT(v7_exec(v7, "a + 5") == V7_UNDEFINED_VARIABLE);

  ASSERT(v7_exec(v7, "a = 7;") == V7_OK);
  ASSERT(v7_exec(v7, "b = a + 3;") == V7_OK);
  ASSERT(v7_top(v7)[-1].v.num == 10);

  ASSERT(v7_exec(v7, "c = b * (a + 3) / 2;") == V7_OK);
  ASSERT(v7_top(v7)[-1].v.num == 50);

  ASSERT(v7_exec(v7, "var x = 12 + 2 - a + b+ 3 / 4 * a;") == V7_OK);
  ASSERT(v7_exec(v7, "b + 2; x + 3 + 1 z = x -2;") == V7_OK);
  ASSERT(v7_exec(v7, "x; var y, z;") == V7_OK);
  ASSERT(v7_exec(v7, "1 2 3") == V7_OK);

  ASSERT(v7_exec(v7, "var k = true;") == V7_OK);
  ASSERT(v7_top(v7)[-1].type == V7_BOOL);
  ASSERT(v7_top(v7)[-1].v.num != 0);

  ASSERT(v7_exec(v7, "var blah = 'kuku';") == V7_OK);
  ASSERT(v7_top(v7)[-1].type == V7_STR);

  ASSERT(v7_exec(v7, "k = { key1: 12, key2: ':-)' };") == V7_OK);
  ASSERT(v7_top(v7)[-1].type == V7_OBJ);

#ifdef V7_DEBUG
  dump_var(v7->scopes[0].vars, 0);
#endif

  v7_destroy(&v7);

  return NULL;
}

static void adder(struct v7 *v7, int num_params) {
  double sum = 0;
  inc_stack(v7, -num_params);
  while (num_params-- > 0) {
    sum += v7_top(v7)[num_params].v.num;
  }
  v7_push_double(v7, sum);
}

static const char *test_native_functions(void) {
  struct v7 *v7 = v7_create();
  ASSERT(v7_define_func(v7, "adder", adder) == V7_OK);
  ASSERT(v7_exec(v7, "adder(1, 2, 3 + 4);") == V7_OK);
  ASSERT(v7_top(v7)[-1].type == V7_NUM);
  ASSERT(v7_top(v7)[-1].v.num == 10);
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
  RUN_TEST(test_native_functions);
  return NULL;
}

int main(void) {
  const char *fail_msg = run_all_tests();
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", static_num_tests);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
