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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "v7.h"

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

static void adder(struct v7 *v7, struct v7_val *obj, struct v7_val *result,
                  struct v7_val **args, int num_args) {
  int i;

  (void) v7; (void) obj;
  result->type = V7_NUM;
  result->v.num = 0;

  for (i = 0; i < num_args; i++) {
    result->v.num += args[i]->v.num;
  }
}

static int check_bool(struct v7 *v7, double val) {
  return v7->sp == 1 &&
    v7_top(v7)[-1]->type == V7_BOOL && val == v7_top(v7)[-1]->v.num;
}

static int check_str(struct v7 *v7, const char *val) {
  return v7->sp == 1 &&
    v7_top(v7)[-1]->type == V7_STR &&
    strlen(val) == v7_top(v7)[-1]->v.str.len &&
    memcmp(val, v7_top(v7)[-1]->v.str.buf, strlen(val)) == 0;
}

static int check_num(struct v7 *v7, double num) {
  const struct v7_val *v = v7_top(v7)[-1];
  return v7->sp == 1 && v->type == V7_NUM &&
    ((isnan(num) && isnan(v->v.num)) || (v->v.num == num));
}

static const char *test_native_functions(void) {
  struct v7 *v7 = v7_create();
  v7_set_func(v7, v7_get_root_namespace(v7), "adder", adder);
  ASSERT(v7_exec(v7, "adder(1, 2, 3 + 4);") == V7_OK);
  ASSERT(v7_top(v7)[-1]->type == V7_NUM);
  ASSERT(v7_top(v7)[-1]->v.num == 10.0);
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

static const char *test_v7_exec(void) {
  struct v7 *v7 = v7_create();

  ASSERT(v7_exec(v7, "") == V7_OK);
  ASSERT(v7_exec(v7, "print();") == V7_OK);
  ASSERT(v7_exec(v7, "print('\n');") == V7_OK);
  ASSERT(v7_exec(v7, "print(this, '\n');") == V7_OK);

  ASSERT(v7_exec(v7, "-2;") == V7_OK);
  ASSERT(check_num(v7, -2.0));

  ASSERT(v7_exec(v7, "3 + 4") == V7_OK);
  ASSERT(check_num(v7, 7.0));

  ASSERT(v7_exec(v7, "2()") == V7_CALLED_NON_FUNCTION);
  ASSERT(v7_exec(v7, " 15 +	2 \r\n * 2  / 1 - 3 * 4 ; ") == V7_OK);

  ASSERT(v7_exec(v7, "( (5  ) );") == V7_OK);
  ASSERT(check_num(v7, 5.0));

  ASSERT(v7_exec(v7, "(2 + (12 / 4));") == V7_OK);
  ASSERT(v7_top(v7)[-1]->type == V7_NUM);
  ASSERT(v7_top(v7)[-1]->v.num == 5.0);

  ASSERT(v7_exec(v7, "1;2 7") == V7_OK);
  ASSERT(check_num(v7, 7.0));
  ASSERT(v7_exec(v7, "a + 5") == V7_TYPE_MISMATCH);

  ASSERT(v7_exec(v7, "a = 7;") == V7_OK);
  ASSERT(check_num(v7, 7.0));
  ASSERT(v7_exec(v7, "print(this, '\n');") == V7_OK);

  ASSERT(v7_exec(v7, "b = a + 3;") == V7_OK);
  ASSERT(check_num(v7, 10.0));
  ASSERT(v7_exec(v7, "print(this, '\n');") == V7_OK);

  ASSERT(v7_exec(v7, "c = b * (a + 3) / 2;") == V7_OK);
  ASSERT(check_num(v7, 50.0));
  ASSERT(v7_exec(v7, "print(this, '\n');") == V7_OK);

  ASSERT(v7_exec(v7, "var x = 12 + 2 - a + b+ 3 / 4 * a;") == V7_OK);
  ASSERT(v7_exec(v7, "b + 2; x + 3 + 1 z = x -2;") == V7_OK);
  ASSERT(v7_exec(v7, "x; var y, z;") == V7_OK);
  ASSERT(v7_exec(v7, "1 2 3") == V7_OK);

  ASSERT(v7_exec(v7, "var k = true;") == V7_OK);
  ASSERT(v7_top(v7)[-1]->type == V7_BOOL);
  ASSERT(v7_top(v7)[-1]->v.num != 0.0);

  ASSERT(v7_exec(v7, "var blah = 'kuku';") == V7_OK);
  ASSERT(v7_top(v7)[-1]->type == V7_STR);

  // Test that k.y does exist
  ASSERT(v7_exec(v7, "k = { y: 17 };") == V7_OK);
  ASSERT(v7_exec(v7, "k.y") == V7_OK);
  ASSERT(check_num(v7, 17.0));
  v7_exec(v7, "print(this, '\n');");

  // Delete k.y and make sure it's gone
  ASSERT(v7_exec(v7, "delete k.y;") == V7_OK);
  ASSERT(v7_exec(v7, "k.y;") == V7_OK);
  ASSERT(v7_top(v7)[-1]->type == V7_UNDEF);
  ASSERT(v7_sp(v7) == 1);
  ASSERT(v7_exec(v7, "delete b; b;") == V7_OK);
  ASSERT(v7_top(v7)[-1]->type == V7_UNDEF);
  ASSERT(v7_sp(v7) == 1);

  ASSERT(v7_exec(v7, "k = { key1: {x:3}, key2: ':-)', y: 5 };") == V7_OK);
  ASSERT(v7_top(v7)[-1]->type == V7_OBJ);
  ASSERT(v7_sp(v7) == 1);

  ASSERT(v7_exec(v7, "k.x = 47;") == V7_OK);
  ASSERT(v7_exec(v7, "k.qwe = { foo: 5 };") == V7_OK);
  v7_exec(v7, "print(k, '\n');");
  ASSERT(v7_exec(v7, "k.qwe.foo = 15;") == V7_OK);
  v7_exec(v7, "print(k, '\n');");

  ASSERT(v7_exec(v7, "k.key1.x + 4") == V7_OK);
  ASSERT(check_num(v7, 7.0));

  ASSERT(v7_exec(v7, "k.foo") == V7_OK);
  ASSERT(v7_top(v7)[-1]->type == V7_UNDEF);

  ASSERT(v7_exec(v7, "var z = 'key1'; k[z]['x']") == V7_OK);
  ASSERT(check_num(v7, 3.0));

  ASSERT(v7_exec(v7, "var stk = 1;") == V7_OK);
  ASSERT(check_num(v7, 1.0));

  ASSERT(v7_exec(v7, "var f1 = function(x, y) { } ; ") == V7_OK);
  ASSERT(v7_sp(v7) == 1);
  ASSERT(v7_top(v7)[-1]->type == V7_FUNC);
  ASSERT(strcmp(v7_top(v7)[-1]->v.func, "(x, y) { }") == 0);

  ASSERT(v7_exec(v7, "var f1 = function(x, y) { return x * y; };") == V7_OK);
  ASSERT(v7_top(v7)[-1]->type == V7_FUNC);

  ASSERT(v7_exec(v7, "f1(12, 4) + 1;") == V7_OK);
  ASSERT(check_num(v7, 49.0));

  ASSERT(v7_exec(v7, "f = function(x,y,z) { print(this, '\n'); };") == V7_OK);
  ASSERT(v7_sp(v7) == 1);
  ASSERT(v7_exec(v7, "f();") == V7_OK);
  ASSERT(v7_exec(v7, "f({});") == V7_OK);
  ASSERT(v7_exec(v7, "f(1, 2);") == V7_OK);
  ASSERT(v7_exec(v7, "f(123, {});") == V7_OK);

  ASSERT(v7_exec(v7, "if (0) f1 = 2; ") == V7_OK);
  ASSERT(v7_exec(v7, "if (5) { f1 = 3; f2 = function(){}; } ") == V7_OK);

  ASSERT(v7_exec(v7, "0 ? 1 : 2;") == V7_OK);
  ASSERT(check_num(v7, 2.0));

  ASSERT(v7_exec(v7, "k = true ? 1 : 2;") == V7_OK);
  ASSERT(check_num(v7, 1.0));

  ASSERT(v7_exec(v7, "var f = function(){var x=12; return x + 1;};") == V7_OK);
  ASSERT(v7_sp(v7) == 1);

  ASSERT(v7_exec(v7, "k = f(1,2,3);") == V7_OK);
  ASSERT(check_num(v7, 13.0));

  ASSERT(v7_exec(v7, "(function() { return f() + 7; })()") == V7_OK);
  ASSERT(check_num(v7, 20.0));

  ASSERT(v7_exec(v7, "var a = 1; if (a == 1) { a = 2; }; a;") == V7_OK);
  ASSERT(check_num(v7, 2.0));

  ASSERT(v7_exec(v7, "a = { x: function(p) { print(this, '\n'); } }") == V7_OK);
  ASSERT(v7_exec(v7, "a.x(2);") == V7_OK);

  ASSERT(v7_exec(v7, "74.toString()") == V7_OK);
  ASSERT(check_str(v7, "74"));

  ASSERT(v7_exec(v7, "'hello'.length") == V7_OK);
  ASSERT(check_num(v7, 5.0));

  //ASSERT(v7_exec(v7, "'foo' + 'bar") == V7_OK);
  //ASSERT(check_str(v7, "foobar"));

  v7_destroy(&v7);
  return NULL;
}

static const char *test_stdlib(void) {
  struct v7 *v7 = v7_create();

  // Number
  ASSERT(v7_exec(v7, "Math.PI") == V7_OK);
  ASSERT(check_num(v7, M_PI));
  ASSERT(v7_exec(v7, "Number.NaN") == V7_OK);
  ASSERT(check_num(v7, NAN));
  ASSERT(v7_exec(v7, "1 == 2") == V7_OK);
  //ASSERT(v7_exec(v7, "print(this, '\n')") == V7_OK);
  ASSERT(check_bool(v7, 0));

  // String
  ASSERT(v7_exec(v7, "'hello'.charCodeAt(1)") == V7_OK);
  ASSERT(check_num(v7, 'e'));
  ASSERT(v7_exec(v7, "'hello'.charCodeAt(4)") == V7_OK);
  ASSERT(check_num(v7, 'o'));
  ASSERT(v7_exec(v7, "'hello'.charCodeAt(5) == Number.NaN") == V7_OK);
  ASSERT(check_bool(v7, 1.0));

  v7_destroy(&v7);
  return NULL;
}

static const char *run_all_tests(void) {
  RUN_TEST(test_v7_destroy);
  RUN_TEST(test_v7_exec);
  RUN_TEST(test_native_functions);
  RUN_TEST(test_stdlib);
  return NULL;
}

int main(void) {
  const char *fail_msg = run_all_tests();
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", static_num_tests);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
