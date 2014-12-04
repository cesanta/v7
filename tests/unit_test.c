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

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "v7.h"

#ifdef _WIN32
#define isinf(x) (!_finite(x))
#define NAN         atof("NAN")
//#define INFINITY    BLAH
#endif

#define FAIL(str, line) do {                    \
  printf("Fail on line %d: [%s]\n", line, str); \
  exit(1);                                      \
  return str;                                   \
} while (0)

#define ASSERT(expr) do {                       \
  static_num_tests++;                           \
  if (!(expr)) FAIL(#expr, __LINE__);           \
} while (0)

#define RUN_TEST(test) do { const char *msg = test(); \
  if (msg) return msg; } while (0)

#ifdef _WIN32
#define isnan(x) _isnan(x)
#endif


static int static_num_tests = 0;
int STOP = 0;  // For xcode breakpoints conditions

static enum v7_err adder(struct v7_c_func_arg *cfa) {
  double sum = 0;
  int i;

  for (i = 0; i < cfa->num_args; i++) {
    sum += v7_number(cfa->args[i]);
  }
  v7_push_number(cfa->v7, sum);

  return V7_OK;
}

static int check_bool(struct v7 *v7, struct v7_val *v, int is_true) {
  _prop_func_2_value(v7, &v);
  return v7_type(v) == V7_TYPE_BOOL && !!v7_number(v) == !!is_true;
}

static int check_str(struct v7 *v7, struct v7_val *v, const char *val) {
  unsigned long len;
  _prop_func_2_value(v7, &v);
  return v7_type(v) == V7_TYPE_STR && !strcmp(v7_string(v, &len), val);
}

static int check_num(struct v7 *v7, struct v7_val *v, double an) {
  _prop_func_2_value(v7, &v);
  double bn = v7_number(v);
  return v7_type(v) == V7_TYPE_NUM &&
  ((an == bn) || (isinf(an) && isinf(bn)) || (isnan(an) && isnan(bn)));
}

static const char *test_native_functions(void) {
  struct v7_val *v;
  struct v7 *v7 = v7_create();

  ASSERT(v7_set(v7, v7_global(v7), "adder", v7_push_func(v7, adder)) == V7_OK);
  ASSERT((v = v7_exec(v7, "adder(1, 2, 3 + 4);")) != NULL);
  ASSERT(check_num(v7, v, 10.0));
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
  struct v7_val *v;
  struct v7 *v7 = v7_create();

  ASSERT((v = v7_exec(v7, "")) == NULL);
  ASSERT((v = v7_exec(v7, "-2;")) != NULL);
  ASSERT(check_num(v7, v, -2.0));
  ASSERT((v = v7_exec(v7, "3 + 4")) != NULL);
  ASSERT(check_num(v7, v, 7.0));
  ASSERT((v = v7_exec(v7, "123.456")) != NULL);
  ASSERT(check_num(v7, v, 123.456));
  ASSERT((v = v7_exec(v7, "NaN")) != NULL);
  ASSERT(check_num(v7, v, NAN));

  // TODO: fix infinity handling under MSVC6
#ifndef _WIN32
  ASSERT((v = v7_exec(v7, "Infinity")) != NULL);
  ASSERT(check_num(v7, v, INFINITY));
  ASSERT((v = v7_exec(v7, "-Infinity")) != NULL);
  ASSERT(check_num(v7, v, -INFINITY));
#endif

  ASSERT((v = v7_exec(v7, "2()")) == NULL);
  ASSERT((v = v7_exec(v7, " 15 +	2 \r\n * 2  / 1 - 3 * 4 ; ")) != NULL);

  ASSERT((v = v7_exec(v7, "( (5  ) );")) != NULL);
  ASSERT(check_num(v7, v, 5.0));

  ASSERT((v = v7_exec(v7, "(2 + (12 / 4));")) != NULL);
  ASSERT(check_num(v7, v, 5.0));

  ASSERT((v = v7_exec(v7, "1;2 7")) != NULL);
  ASSERT(check_num(v7, v, 7.0));

  v = v7_exec(v7, "a=undefined; a + 5");
  ASSERT(check_num(v7, v, NAN));

  /* ReferenceErrors are not thrown yet, treat as undefined */
  v = v7_exec(v7, "a + 5");
  ASSERT(check_num(v7, v, NAN));

  ASSERT((v = v7_exec(v7, "print();")) != NULL);
  ASSERT((v = v7_exec(v7, "print(this);")) != NULL);

  ASSERT((v = v7_exec(v7, "a = 7;")) != NULL);
  ASSERT(check_num(v7, v, 7.0));
  ASSERT((v = v7_exec(v7, "print(this);")) != NULL);

  ASSERT((v = v7_exec(v7, "b = a + 3;")) != NULL);
  ASSERT(check_num(v7, v, 10.0));
  ASSERT((v = v7_exec(v7, "c = b * (a + 3) / 2;")) != NULL);
  ASSERT(check_num(v7, v, 50.0));
  ASSERT((v = v7_exec(v7, "print(this);")) != NULL);

  ASSERT((v = v7_exec(v7, "var x = 1.23; x")) != NULL);
  ASSERT(check_num(v7, v, 1.23));

  ASSERT((v = v7_exec(v7, "b = 7; a = b *= 4;")) != NULL);

  ASSERT((v = v7_exec(v7, "var x = 12 + 2 - a + b+ 3 / 4 * a; x;")) != NULL);
  ASSERT((v = v7_exec(v7, "x + 1")) != NULL);
  ASSERT((v = v7_exec(v7, "b + 2; x + 3 + 1 z = x -2;")) != NULL);
  ASSERT((v = v7_exec(v7, "x; var y, z;")) != NULL);
  ASSERT((v = v7_exec(v7, "1 2 3")) != NULL);

  ASSERT((v = v7_exec(v7, "var k = true; k ")) != NULL);
  ASSERT(check_bool(v7, v, 1));

  ASSERT((v = v7_exec(v7, "var blah = 'kuku'; blah")) != NULL);
  ASSERT(check_str(v7, v, "kuku"));

  // Test that k.y does exist
  ASSERT((v = v7_exec(v7, "k = { y: 17 };")) != NULL);
  ASSERT((v = v7_exec(v7, "k.y")) != NULL);
  ASSERT(check_num(v7, v, 17.0));
  v7_exec(v7, "print(this);");

  // Delete k.y and make sure it's gone
  ASSERT((v = v7_exec(v7, "delete k.y;")) != NULL);
  ASSERT((v = v7_exec(v7, "k.y;")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_UNDEF);
  ASSERT((v = v7_exec(v7, "delete b; b;")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_UNDEF);

  ASSERT((v = v7_exec(v7, "k = { key1: {x:3}, key2: ':-)', y: 5 };")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_OBJ);

  ASSERT((v = v7_exec(v7, "k.x = 47;")) != NULL);
  ASSERT((v = v7_exec(v7, "k.qwe = { foo: 5 };")) != NULL);
  ASSERT((v = v7_exec(v7, "k.qwe.foo = 15;")) != NULL);

  ASSERT((v = v7_exec(v7, "k.key1.x + 4")) != NULL);
  ASSERT(check_num(v7, v, 7.0));

  ASSERT((v = v7_exec(v7, "k.foo")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_UNDEF);

  ASSERT((v = v7_exec(v7, "var z = 'key1'; k[z]['x']")) != NULL);
  ASSERT(check_num(v7, v, 3.0));

  ASSERT((v = v7_exec(v7, "var stk = 1; stk")) != NULL);
  ASSERT(check_num(v7, v, 1.0));

  ASSERT((v = v7_exec(v7, "var f1 = function(x, y) { }; typeof f1 ")) != NULL);
  ASSERT(check_str(v7, v, "function"));

  ASSERT((v = v7_exec(v7, "var f1 = function(x, y) { return x * y }")) != NULL);
  //ASSERT(v7_is_class(v7_top(v7)[-1], V7_CLASS_FUNCTION));
  ASSERT((v = v7_exec(v7, "f1(2, 3)")) != NULL);
  ASSERT(check_num(v7, v, 6.0));
  ASSERT((v = v7_exec(v7, "f1(12, 4) + 1;")) != NULL);
  ASSERT(check_num(v7, v, 49.0));
  ASSERT((v = v7_exec(v7, "f1(12, 4) * 2;")) != NULL);
  ASSERT(check_num(v7, v, 96.0));

  ASSERT((v = v7_exec(v7, "f = function(x,y,z) {print(this);};")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_OBJ);
  //ASSERT(v7_is_class(v7->stack[0], V7_CLASS_FUNCTION));
  ASSERT((v = v7_exec(v7, "f();")) != NULL);
  ASSERT((v = v7_exec(v7, "f({});")) != NULL);
  ASSERT((v = v7_exec(v7, "f(1, 2);")) != NULL);
  ASSERT((v = v7_exec(v7, "f(123, {});")) != NULL);

  ASSERT((v = v7_exec(v7, "if (0) f1 = 2; ")) == NULL);
  ASSERT((v = v7_exec(v7, "if (5) { f1 = 3; f2 = function(){}; } ")) != NULL);

  ASSERT((v = v7_exec(v7, "0 ? 1 : 2;")) != NULL);
  ASSERT(check_num(v7, v, 2.0));

  ASSERT((v = v7_exec(v7, "k = true ? 1 : 2;")) != NULL);
  ASSERT(check_num(v7, v, 1.0));

  ASSERT((v = v7_exec(v7, "var f = function(){var x=12; return x + 1;};")) != NULL);

  ASSERT((v = v7_exec(v7, "k = f(1,2,3);")) != NULL);
  ASSERT(check_num(v7, v, 13.0));

  ASSERT((v = v7_exec(v7, "(function() { return f() + 7; })()")) != NULL);
  ASSERT(check_num(v7, v, 20.0));

  ASSERT((v = v7_exec(v7, "var a = 1; if (a == 1) { a = 2; }; a;")) != NULL);
  ASSERT(check_num(v7, v, 2.0));

  ASSERT((v = v7_exec(v7, "var a = 'foo'; a == 'foo';")) != NULL);
  ASSERT(check_bool(v7, v, 1));

  ASSERT((v = v7_exec(v7, "a = { x: function(p) { print(this); } }")) != NULL);
  ASSERT((v = v7_exec(v7, "a.x(2);")) != NULL);

  ASSERT((v = v7_exec(v7, "(74).toString()")) != NULL);
  ASSERT(check_str(v7, v, "74"));
  ASSERT((v = v7_exec(v7, "'hello'.length")) != NULL);
  ASSERT(check_num(v7, v, 5.0));

  ASSERT((v = v7_exec(v7, "k = { x : function() { if (1) 2; } }")) != NULL);

  ASSERT((v = v7_exec(v7, "'foo' + 'bar'")) != NULL);
  ASSERT(check_str(v7, v, "foobar"));

  ASSERT((v = v7_exec(v7, "var x = [1, 'foo', true, 7];")) != NULL);
  ASSERT((v = v7_exec(v7, "x.length")) != NULL);
  ASSERT(check_num(v7, v, 4.0));
  ASSERT((v = v7_exec(v7, "x[1]")) != NULL);
  ASSERT(check_str(v7, v, "foo"));

  ASSERT((v = v7_exec(v7, "var f1 = function() { 1; };")) != NULL);
  ASSERT((v = v7_exec(v7, "var f2 = function(x) { if (x) return x; };")) != NULL);
  ASSERT((v = v7_exec(v7, "f1()")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_UNDEF);
  ASSERT((v = v7_exec(v7, "f2(false)")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_UNDEF);
  ASSERT((v = v7_exec(v7, "f2(17)")) != NULL);
  ASSERT(check_num(v7, v, 17.0));
  ASSERT((v = v7_exec(v7, "f2(true)")) != NULL);
  ASSERT(check_bool(v7, v, 1.0));

  ASSERT((v = v7_exec(v7, "1 <= 2 ? 7 : 8")) != NULL);
  ASSERT(check_num(v7, v, 7.0));

  ASSERT((v = v7_exec(v7, "function a (t) { return t * t }; ")) != NULL);
  ASSERT((v = v7_exec(v7, "a(2)")) != NULL);
  ASSERT(check_num(v7, v, 4.0));
  ASSERT((v = v7_exec(v7, "a(0)")) != NULL);
  ASSERT(check_num(v7, v, 0.0));

  ASSERT((v = v7_exec(v7, "function fac(x) { "
         "return x <= 1 ? 1 : x * fac(x - 1); }")) != NULL);
  ASSERT((v = v7_exec(v7, "fac(1)")) != NULL);
  ASSERT(check_num(v7, v, 1.0));
  ASSERT((v = v7_exec(v7, "fac(5)")) != NULL);
  ASSERT(check_num(v7, v, 120.0));
  ASSERT((v = v7_exec(v7, "fac(20)")) != NULL);

  ASSERT((v = v7_exec(v7, "function qq(a,b) { return a + b; }")) != NULL);
  ASSERT((v = v7_exec(v7, "qq(1,2)")) != NULL);
  ASSERT(check_num(v7, v, 3.0));

  ASSERT((v = v7_exec(v7, "1 < 2 == 2 < 3")) != NULL);
  ASSERT(check_bool(v7, v, 1.0));

  ASSERT((v = v7_exec(v7, "5 % 3 * 3")) != NULL);
  ASSERT(check_num(v7, v, 6.0));
  ASSERT((v = v7_exec(v7, "76 & 13")) != NULL);
  ASSERT(check_num(v7, v, 12.0));
  ASSERT((v = v7_exec(v7, "34325 ^ 190992 & 74832")) != NULL);
  ASSERT(check_num(v7, v, 42501.0));

  ASSERT((v = v7_exec(v7, "a = 12;")) != NULL);
  ASSERT((v = v7_exec(v7, "a += 44; a;")) != NULL);
  ASSERT(check_num(v7, v, 56.0));
  ASSERT((v = v7_exec(v7, "a -= a / 2; a;")) != NULL);
  ASSERT(check_num(v7, v, 28.0));
  ASSERT((v = v7_exec(v7, "a *= 0.5; a;")) != NULL);
  ASSERT(check_num(v7, v, 14.0));
#ifndef _WIN32
  ASSERT((v = v7_exec(v7, "a /= 0; a;")) != NULL);
  ASSERT(check_num(v7, v, INFINITY));
#endif

  ASSERT((v = v7_exec(v7, "!5")) != NULL);
  ASSERT(check_bool(v7, v, 0.0));
  ASSERT((v = v7_exec(v7, "!''")) != NULL);
  ASSERT(check_bool(v7, v, 1.0));
  ASSERT((v = v7_exec(v7, "1 != 2")) != NULL);
  ASSERT(check_bool(v7, v, 1.0));
  ASSERT((v = v7_exec(v7, "7 >= 0")) != NULL);
  ASSERT(check_bool(v7, v, 1.0));

  ASSERT((v = v7_exec(v7, "if (false) 3; ")) == NULL);
  ASSERT((v = v7_exec(v7, "if (true) { if (1) {2;} 5; } ")) != NULL);
  ASSERT(check_num(v7, v, 5.0));
  ASSERT((v = v7_exec(v7, "if ('') 3; ")) == NULL);
  ASSERT((v = v7_exec(v7, "if ('0') 9; ")) != NULL);
  ASSERT(check_num(v7, v, 9.0));
  ASSERT((v = v7_exec(v7, "if (false) 1; else 3;")) != NULL);
  ASSERT(check_num(v7, v, 3.0));
  ASSERT((v = v7_exec(v7, "if (false) 1; else if (0) { 3 } else { 2 }")) != NULL);
  ASSERT(check_num(v7, v, 2.0));
  ASSERT((v = v7_exec(v7, "if (false) 1; else if (1) { 3 } else { 2 }")) != NULL);
  ASSERT(check_num(v7, v, 3.0));

  ASSERT((v = v7_exec(v7, "a = 32; 2 + a++;")) != NULL);
  ASSERT(check_num(v7, v, 34.0));

  ASSERT((v = v7_exec(v7, "print()")) != NULL);
  ASSERT((v = v7_exec(v7, "print(['hi', 1, true, null, /\\s+/])")) != NULL);

  ASSERT((v = v7_exec(v7, "a = {};")) != NULL);
  ASSERT((v = v7_exec(v7, "a.foo = function(x) { var y = "
                 "x.substr(1).split() }")) != NULL);

  ASSERT((v = v7_exec(v7, "typeof 2")) != NULL);
  ASSERT(check_str(v7, v, "number"));

  ASSERT((v = v7_exec(v7, "a = { b: { k: 44 } };")) != NULL);
  STOP = 1;
  ASSERT((v = v7_exec(v7, "a.b['x'] = 79;")) != NULL);
  ASSERT((v = v7_exec(v7, "a.b.x")) != NULL);
  ASSERT(check_num(v7, v, 79.0));

  v7_destroy(&v7);
  return NULL;
}

static const char *test_stdlib(void) {
  struct v7_val *v;
  struct v7 *v7 = v7_create();

  // Number
#ifndef _WIN32
  ASSERT((v = v7_exec(v7, "Math.PI")) != NULL);
  ASSERT(check_num(v7, v, M_PI));
  ASSERT((v = v7_exec(v7, "Number.NaN")) != NULL);
  ASSERT(check_num(v7, v, NAN));
#endif
  ASSERT((v = v7_exec(v7, "1 == 2")) != NULL);
  ASSERT(check_bool(v7, v, 0));
  ASSERT((v = v7_exec(v7, "1 + 2 * 7 === 15")) != NULL);
  ASSERT(check_bool(v7, v, 1));
  ASSERT((v = v7_exec(v7, "Number(1.23) === 1.23")) != NULL);
  ASSERT(check_bool(v7, v, 1));
  ASSERT((v = v7_exec(v7, "Number(1.23)")) != NULL);
  ASSERT(check_num(v7, v, 1.23));
#ifdef TODO /* New operator: Assertion failed: (v7->root_scope.proto == &s_global), function do_exec, file src/util.c, line 557. */
  ASSERT((v = v7_exec(v7, "new Number(21.23)")) != NULL);
#endif

  // String
  ASSERT((v = v7_exec(v7, "'hello'.charCodeAt(1)")) != NULL);
  ASSERT(check_num(v7, v, 'e'));
  ASSERT((v = v7_exec(v7, "'hello'.charCodeAt(4)")) != NULL);
  ASSERT(check_num(v7, v, 'o'));
  ASSERT((v = v7_exec(v7, "'hello'.charCodeAt(5) == Number.NaN")) != NULL);
  ASSERT(check_bool(v7, v, 1.0));
  ASSERT((v = v7_exec(v7, "'hello'.indexOf()")) != NULL);
  ASSERT(check_num(v7, v, -1.0));
  ASSERT((v = v7_exec(v7, "'HTTP/1.0\\r\\n'.indexOf('\\r\\n')")) != NULL);
  ASSERT(check_num(v7, v, 8.0));
  ASSERT((v = v7_exec(v7, "'hi there'.indexOf('e')")) != NULL);
  ASSERT(check_num(v7, v, 5.0));
  ASSERT((v = v7_exec(v7, "'hi there'.indexOf('e', 6)")) != NULL);
  ASSERT(check_num(v7, v, 7.0));
  ASSERT((v = v7_exec(v7, "'hi there'.substr(3, 2)")) != NULL);
  ASSERT(check_str(v7, v, "th"));
  ASSERT((v = v7_exec(v7, "'hi there'.substring(3, 5)")) != NULL);
  ASSERT(check_str(v7, v, "th"));
  ASSERT((v = v7_exec(v7, "'hi there'.substr(3)")) != NULL);
  ASSERT(check_str(v7, v, "there"));
#ifdef TODO /* negative substring beginning not working */
  ASSERT((v = v7_exec(v7, "'hi there'.substr(-2)")) != NULL);
  ASSERT(check_str(v7, v, "re"));
  ASSERT((v = v7_exec(v7, "'hi there'.substr(-20)")) != NULL);
  ASSERT(check_str(v7, v, ""));
#endif
  ASSERT((v = v7_exec(v7, "'hi there'.substr(0, 300)")) != NULL);
  ASSERT(check_str(v7, v, "hi there"));
  ASSERT((v = v7_exec(v7, "'dew dee'.match(/\\d+/)")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_NULL);
  ASSERT((v = v7_exec(v7, "m = 'foo 1234 bar'.match(/\\S+ (\\d+)/)")) != NULL);
  ASSERT((v = v7_exec(v7, "m.length")) != NULL);
#ifdef TODO /* got: [foo 1234] i.e. 1 length array with input string !! */
  ASSERT(check_num(v7, v, 2.0));
  ASSERT((v = v7_exec(v7, "m[0]")) != NULL);
  ASSERT(check_str(v7, v, "foo 1234"));
  ASSERT((v = v7_exec(v7, "m[1]")) != NULL);
  ASSERT(check_str(v7, v, "1234"));
  ASSERT((v = v7_exec(v7, "m[2]")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_UNDEF);
#endif
  ASSERT((v = v7_exec(v7, "m = 'aa bb cc'.split(); m.length")) != NULL);
  ASSERT(check_num(v7, v, 1.0));
#ifdef TODO  /* doesn't split at every char */
  ASSERT((v = v7_exec(v7, "m = 'aa bb cc'.split(''); m.length")) != NULL);
  ASSERT(check_num(v7, v, 8.0));
#endif
  ASSERT((v = v7_exec(v7, "m = 'aa bb cc'.split(' '); m.length")) != NULL);
  ASSERT(check_num(v7, v, 3.0));
  ASSERT((v = v7_exec(v7, "m = 'aa bb cc'.split(' ', 2); m.length")) != NULL);
  ASSERT(check_num(v7, v, 2.0));
  ASSERT((v = v7_exec(v7, "m = 'aa bb cc'.split(/ /, 2); m.length")) != NULL);
  ASSERT(check_num(v7, v, 2.0));
  ASSERT((v = v7_exec(v7, "'aa bb cc'.substr(0, 4).split(' ').length")) != NULL);
  ASSERT(check_num(v7, v, 2.0));
  ASSERT((v = v7_exec(v7, "'aa bb cc'.substr(0, 4).split(' ')[1]")) != NULL);
  ASSERT(check_str(v7, v, "b"));
#ifdef TODO  /* doesn't split at every char */
  ASSERT((v = v7_exec(v7, "{z: '123456'}.z.substr(0, 3).split('').length")) != NULL);
  ASSERT(check_num(v7, v, 3.0));
#endif
  ASSERT((v = v7_exec(v7, "String('hi')")) != NULL);
  ASSERT(check_str(v7, v, "hi"));
#ifdef TODO /* New operator: Assertion failed: (v7->root_scope.proto == &s_global), function do_exec, file src/util.c, line 557. */
  ASSERT((v = v7_exec(v7, "new String('blah')")) != NULL);
#endif

  // Math
  ASSERT((v = v7_exec(v7, "Math.sqrt(144)")) != NULL);
  ASSERT(check_num(v7, v, 12.0));

  // Regexp
  ASSERT((v = v7_exec(v7, "re = /GET (\\S+) HTTP/; re")) != NULL);
  ASSERT((v = v7_exec(v7, "re = /GET (\\S+) HTTP/;")) != NULL);
  ASSERT((v = v7_exec(v7, "re = /GET (\\S+) HTTP/ ")) != NULL);
  ASSERT((v = v7_exec(v7, "re = /GET (\\S+) HTTP/\n")) != NULL);
  ASSERT((v = v7_exec(v7, "re = /GET (\\S+) HTTP/")) != NULL);

  v7_destroy(&v7);
  return NULL;
}

static int test_if_expr(struct v7 *v7, const char *expr, int result) {
  struct v7_val *v = v7_exec(v7, expr);
  return v != NULL && (v7_is_true(v) ? 1 : 0) == result;
}

static const char *test_is_true(void) {
  struct v7 *v7 = v7_create();

  ASSERT(test_if_expr(v7, "true", 1));
  ASSERT(test_if_expr(v7, "false", 0));
  ASSERT(test_if_expr(v7, "1", 1));
  ASSERT(test_if_expr(v7, "123.24876", 1));
  ASSERT(test_if_expr(v7, "0", 0));
  ASSERT(test_if_expr(v7, "-1", 1));
  ASSERT(test_if_expr(v7, "'true'", 1));
  ASSERT(test_if_expr(v7, "'false'", 1));
  ASSERT(test_if_expr(v7, "'hi'", 1));
  ASSERT(test_if_expr(v7, "'1'", 1));
  ASSERT(test_if_expr(v7, "'0'", 1));
  ASSERT(test_if_expr(v7, "'-1'", 1));
  ASSERT(test_if_expr(v7, "''", 0));
  ASSERT(test_if_expr(v7, "null", 0));
  ASSERT(test_if_expr(v7, "undefined", 0));
#ifndef _WIN32
  ASSERT(test_if_expr(v7, "Infinity", 1));
  ASSERT(test_if_expr(v7, "-Infinity", 1));
#endif
  ASSERT(test_if_expr(v7, "[]", 1));
  ASSERT(test_if_expr(v7, "{}", 1));
  ASSERT(test_if_expr(v7, "[[]]", 1));
  ASSERT(test_if_expr(v7, "[0]", 1));
  ASSERT(test_if_expr(v7, "[1]", 1));
  ASSERT(test_if_expr(v7, "NaN", 0));

  v7_destroy(&v7);
  return NULL;
}

static const char *test_closure(void) {
  struct v7_val *v;
  struct v7 *v7 = v7_create();

  ASSERT((v = v7_exec(v7, "function a(x){return function(y){return x*y}}")) != NULL);
  ASSERT((v = v7_exec(v7, "var f1 = a(5);")) != NULL);
  ASSERT((v = v7_exec(v7, "var f2 = a(7);")) != NULL);
  ASSERT((v = v7_exec(v7, "f1(3);")) != NULL);
  ASSERT(check_num(v7, v, 15.0));
  ASSERT((v = v7_exec(v7, "f2(3);")) != NULL);
  ASSERT(check_num(v7, v, 21.0));
  v7_destroy(&v7);
  return NULL;
}

static const char *run_all_tests(void) {
  RUN_TEST(test_v7_destroy);
  RUN_TEST(test_is_true);
  RUN_TEST(test_v7_exec);
  RUN_TEST(test_closure);
  RUN_TEST(test_native_functions);
  RUN_TEST(test_stdlib);
  return NULL;
}

int main(void) {
  const char *fail_msg = run_all_tests();
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", static_num_tests);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
