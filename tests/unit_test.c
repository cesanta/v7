/* Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
 * Copyright (c) 2013-2014 Cesanta Software Limited
 * All rights reserved
 *
 * This library is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this library under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively, you can license this library under a commercial
 * license, as set out in <http://cesanta.com/products.html>.
 */

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../v7.h"
#include "../src/internal.h"

#ifdef _WIN32
#define isinf(x) (!_finite(x))
#ifndef NAN
#define NAN         atof("NAN")
#endif
/* #define INFINITY    BLAH */
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

#define RUN_TEST(test) do {                 \
  const char *msg = NULL;                   \
  if (strstr(# test, filter)) msg = test(); \
  if (msg) return msg;                      \
} while (0)

#ifdef _WIN32
#define isnan(x) _isnan(x)
#endif


static int static_num_tests = 0;
int STOP = 0;  /* For xcode breakpoints conditions */

static int check_value(struct v7 *v7, val_t v, const char *str) {
  char buf[2048];
  v7_to_json(v7, v, buf, sizeof(buf));
  if (strncmp(buf, str, sizeof(buf)) != 0) {
    printf("want %s got %s\n", str, buf);
    return 0;
  }
  return 1;
}

static int test_if_expr(struct v7 *v7, const char *expr, int result) {
  val_t v = v7_exec(v7, expr);
  return result == (v7_is_true(v7, v) ? 1 : 0);
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
  ASSERT(test_if_expr(v7, "var x = {}", 1));
  ASSERT(test_if_expr(v7, "[[]]", 1));
  ASSERT(test_if_expr(v7, "[0]", 1));
  ASSERT(test_if_expr(v7, "[1]", 1));
  ASSERT(test_if_expr(v7, "NaN", 0));

  v7_destroy(v7);
  return NULL;
}

static const char *test_closure(void) {
  val_t v;
  struct v7 *v7 = v7_create();

  ASSERT((v = v7_exec(v7, "function a(x){return function(y){return x*y}}")) != V7_UNDEFINED);
  ASSERT((v = v7_exec(v7, "var f1 = a(5);")) != V7_UNDEFINED);
  ASSERT((v = v7_exec(v7, "var f2 = a(7);")) != V7_UNDEFINED);
  ASSERT((v = v7_exec(v7, "f1(3);")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "15"));
  ASSERT((v = v7_exec(v7, "f2(3);")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "21"));

  v7_destroy(v7);
  return NULL;
}

static val_t adder(struct v7 *v7, val_t this_obj, val_t args) {
  double sum = 0;
  int i;

  (void) this_obj;
  for (i = 0; i < v7_array_length(v7, args); i++) {
    sum += val_to_double(v7_array_at(v7, args, i));
  }
  return v7_double_to_value(sum);
}

static const char *test_native_functions(void) {
  val_t v;
  struct v7 *v7 = v7_create();

  ASSERT(v7_set_property(v7, v7_get_global_object(v7), "adder", 5, 0,
         v7_create_cfunction(adder)) == 0);
  ASSERT((v = v7_exec(v7, "adder(1, 2, 3 + 4);")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "10"));
  v7_destroy(v7);

  return NULL;
}

static const char *test_stdlib(void) {
  v7_val_t v;
  struct v7 *v7 = v7_create();

  ASSERT((v = v7_exec(v7, "Boolean()")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "Boolean(0)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "Boolean(1)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "Boolean([])")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "new Boolean([])")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "{}"));

#if 0
  /* Number */
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

  /* String */
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
  ASSERT((v = v7_exec(v7, "'hi there'.substr(-2)")) != NULL);
  ASSERT(check_str(v7, v, "hi there"));
  ASSERT((v = v7_exec(v7, "'hi there'.substr(NaN)")) != NULL);
  ASSERT(check_str(v7, v, "hi there"));
  ASSERT((v = v7_exec(v7, "'hi there'.substr(0, 300)")) != NULL);
  ASSERT(check_str(v7, v, "hi there"));
  ASSERT((v = v7_exec(v7, "'dew dee'.match(/\\d+/)")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_NULL);
  ASSERT((v = v7_exec(v7, "m = 'foo 1234 bar'.match(/\\S+ (\\d+)/)")) != NULL);
  ASSERT((v = v7_exec(v7, "m.length")) != NULL);
  ASSERT(check_num(v7, v, 2.0));
  ASSERT((v = v7_exec(v7, "m[0]")) != NULL);
  ASSERT(check_str(v7, v, "foo 1234"));
  ASSERT((v = v7_exec(v7, "m[1]")) != NULL);
  ASSERT(check_str(v7, v, "1234"));
  ASSERT((v = v7_exec(v7, "m[2]")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_UNDEF);
  ASSERT((v = v7_exec(v7, "m = 'should match empty string at index 0'.match(/x*/)")) != NULL);
  ASSERT((v = v7_exec(v7, "m.length")) != NULL);
  ASSERT(check_num(v7, v, 1.0));
  ASSERT((v = v7_exec(v7, "m[0]")) != NULL);
  ASSERT(check_str(v7, v, ""));
  ASSERT((v = v7_exec(v7, "m = 'aa bb cc'.split(); m.length")) != NULL);
  ASSERT(check_num(v7, v, 1.0));
  ASSERT((v = v7_exec(v7, "m = 'aa bb cc'.split(''); m.length")) != NULL);
  ASSERT(check_num(v7, v, 8.0));
  ASSERT((v = v7_exec(v7, "m = 'aa bb cc'.split(RegExp('')); m.length")) != NULL);
  ASSERT(check_num(v7, v, 8.0));
  ASSERT((v = v7_exec(v7, "m = 'aa bb cc'.split(/x*/); m.length")) != NULL);
  ASSERT(check_num(v7, v, 8.0));
  ASSERT((v = v7_exec(v7, "m = 'aa bb cc'.split(/(x)*/); m.length")) != NULL);
  ASSERT(check_num(v7, v, 16.0));
  ASSERT((v = v7_exec(v7, "m[0]")) != NULL);
  ASSERT(check_str(v7, v, "a"));
  ASSERT((v = v7_exec(v7, "m[1]")) != NULL);
  ASSERT(v7_type(v) == V7_TYPE_UNDEF);
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
  ASSERT((v = v7_exec(v7, "{z: '123456'}.z.substr(0, 3).split('').length")) != NULL);
  ASSERT(check_num(v7, v, 3.0));
  ASSERT((v = v7_exec(v7, "String('hi')")) != NULL);
  ASSERT(check_str(v7, v, "hi"));
  ASSERT((v = v7_exec(v7, "new String('blah')")) != NULL);

  /* Math */
  ASSERT((v = v7_exec(v7, "Math.sqrt(144)")) != NULL);
  ASSERT(check_num(v7, v, 12.0));

  /* Regexp */
  ASSERT((v = v7_exec(v7, "re = /GET (\\S+) HTTP/; re")) != NULL);
  ASSERT((v = v7_exec(v7, "re = /GET (\\S+) HTTP/;")) != NULL);
  ASSERT((v = v7_exec(v7, "re = /GET (\\S+) HTTP/ ")) != NULL);
  ASSERT((v = v7_exec(v7, "re = /GET (\\S+) HTTP/\n")) != NULL);
  ASSERT((v = v7_exec(v7, "re = /GET (\\S+) HTTP/")) != NULL);
#endif

  v7_destroy(v7);
  return NULL;
}

static const char *test_tokenizer(void) {
  static const char *str =
    "1.23e-15 'fo\\'\\'o\\x25\n\\'' /\\s+/ $_12foo{}(),[].:;== === != !== "
    "= %= *= /= ^= += -= |= &= <<= >>= >>>= & || + - ++ -- "
    "&&|?~%*/^ <= < >= > << >> >>> !";
  enum v7_tok tok = TOK_END_OF_INPUT;
  double num;
  const char *p = str;
  int i = 1;

  skip_to_next_tok(&p);

  /* Make sure divisions are parsed correctly - set previous token */
  while ((tok = get_tok(&p, &num, i > TOK_REGEX_LITERAL ? TOK_NUMBER: tok))
         != TOK_END_OF_INPUT) {
    skip_to_next_tok(&p);
    ASSERT(tok == i);
    i++;
  }
  ASSERT(i == TOK_BREAK);

  p = "/foo/";
  ASSERT(get_tok(&p, &num, TOK_NUMBER) == TOK_DIV);

  p = "/foo/";
  ASSERT(get_tok(&p, &num, TOK_COMMA) == TOK_REGEX_LITERAL);

  p = "/foo";
  ASSERT(get_tok(&p, &num, TOK_COMMA) == TOK_DIV);

  p = "/fo\\/o";
  ASSERT(get_tok(&p, &num, TOK_COMMA) == TOK_DIV);

  return NULL;
}

static const char *test_runtime(void) {
  struct v7 *v7 = v7_create();
  val_t v;
  struct v7_property *p;
  size_t n;

  v = v7_create_null();
  ASSERT(v == V7_NULL);

  v = v7_create_undefined();
  ASSERT(v == V7_UNDEFINED);

  v = v7_create_number(1.0);
  ASSERT(val_type(v7, v) == V7_TYPE_NUMBER);
  ASSERT(val_to_double(v) == 1.0);
  ASSERT(check_value(v7, v, "1"));

  v = v7_create_number(1.5);
  ASSERT(val_to_double(v) == 1.5);
  ASSERT(check_value(v7, v, "1.5"));

  v = v7_create_boolean(1);
  ASSERT(val_type(v7, v) == V7_TYPE_BOOLEAN);
  ASSERT(val_to_boolean(v) == 1);
  ASSERT(check_value(v7, v, "true"));

  v = v7_create_boolean(0);
  ASSERT(check_value(v7, v, "false"));

  v = v7_create_string(v7, "foo", 3, 1);
  ASSERT(val_type(v7, v) == V7_TYPE_STRING);
  val_to_string(v7, &v, &n);
  ASSERT(n == 3);
  ASSERT(check_value(v7, v, "\"foo\""));

  v = v7_create_object(v7);
  ASSERT(val_type(v7, v) == V7_TYPE_GENERIC_OBJECT);
  ASSERT(val_to_object(v) != NULL);
  ASSERT(val_to_object(v)->prototype != NULL);
  ASSERT(val_to_object(v)->prototype->prototype == NULL);

  ASSERT(v7_set_property(v7, v, "foo", -1, 0, v7_create_null()) == 0);
  ASSERT((p = v7_get_property(v, "foo", -1)) != NULL);
  ASSERT(p->attributes == 0);
  ASSERT(p->value == V7_NULL);
  ASSERT(check_value(v7, p->value, "null"));

  ASSERT(v7_set_property(v7, v, "foo", -1, 0, v7_create_undefined()) == 0);
  ASSERT((p = v7_get_property(v, "foo", -1)) != NULL);
  ASSERT(check_value(v7, p->value, "undefined"));

  ASSERT(v7_set_property(v7, v, "foo", -1, 0,
         v7_create_string(v7, "bar", 3, 1)) == 0);
  ASSERT((p = v7_get_property(v, "foo", -1)) != NULL);
  ASSERT(check_value(v7, p->value, "\"bar\""));

  ASSERT(v7_set_property(v7, v, "foo", -1, 0,
         v7_create_string(v7, "zar", 3, 1)) == 0);
  ASSERT((p = v7_get_property(v, "foo", -1)) != NULL);
  ASSERT(check_value(v7, p->value, "\"zar\""));

  ASSERT(v7_del_property(v, "foo", ~0) == 0);
  ASSERT(val_to_object(v)->properties == NULL);
  ASSERT(v7_del_property(v, "foo", -1) == -1);
  ASSERT(v7_set_property(v7, v, "foo", -1, 0,
         v7_create_string(v7, "bar", 3, 1)) == 0);
  ASSERT(v7_set_property(v7, v, "bar", -1, 0,
         v7_create_string(v7, "foo", 3, 1)) == 0);
  ASSERT(v7_set_property(v7, v, "aba", -1, 0,
         v7_create_string(v7, "bab", 3, 1)) == 0);
  ASSERT(v7_del_property(v, "foo", -1) == 0);
  ASSERT((p = v7_get_property(v, "foo", -1)) == NULL);
  ASSERT(v7_del_property(v, "aba", -1) == 0);
  ASSERT((p = v7_get_property(v, "aba", -1)) == NULL);
  ASSERT(v7_del_property(v, "bar", -1) == 0);
  ASSERT((p = v7_get_property(v, "bar", -1)) == NULL);

  v = v7_create_object(v7);
  ASSERT(v7_set_property(v7, v, "foo", -1, 0, v7_create_number(1.0)) == 0);
  ASSERT((p = v7_get_property(v, "foo", -1)) != NULL);
  ASSERT((p = v7_get_property(v, "f", -1)) == NULL);

  v = v7_create_object(v7);
  ASSERT(v7_set_property(v7, v, "foo", -1, 0, v) == 0);
  ASSERT(check_value(v7, v, "{\"foo\":[Circular]}"));

  v7_destroy(v7);
  return NULL;
}

static const char *test_parser(void) {
  int i;
  struct ast a;
  struct v7 *v7 = v7_create();
  const char *cases[] = {
    "1",
    "true",
    "false",
    "null",
    "undefined",
    "1+2",
    "1-2",
    "1*2",
    "1/2",
    "1%2",
    "1/-2",
    "(1 + 2) * x + 3",
    "1 + (2, 3) * 4, 5",
    "(a=1 + 2) + 3",
    "1 ? 2 : 3",
    "1 ? 2 : 3 ? 4 : 5",
    "1 ? 2 : (3 ? 4 : 5)",
    "1 || 2 + 2",
    "1 && 2 || 3 && 4 + 5",
    "1|2 && 3|3",
    "1^2|3^4",
    "1&2^2&4|5&6",
    "1==2&3",
    "1<2",
    "1<=2",
    "1>=2",
    "1>2",
    "1==1<2",
    "a instanceof b",
    "1 in b",
    "1!=2&3",
    "1!==2&3",
    "1===2",
    "1<<2",
    "1>>2",
    "1>>>2",
    "1<<2<3",
    "1/2/2",
    "(1/2)/2",
    "1 + + 1",
    "1- -2",
    "!1",
    "~0",
    "void x()",
    "delete x",
    "typeof x",
    "++x",
    "--i",
    "x++",
    "i--",
    "a.b",
    "a.b.c",
    "a[0]",
    "a[0].b",
    "a[0][1]",
    "a[b[0].c]",
    "a()",
    "a(0)",
    "a(0, 1)",
    "a(0, (1, 2))",
    "1 + a(0, (1, 2)) + 2",
    "new x",
    "new x(0, 1)",
    "new x.y(0, 1)",
    "new x.y",
    "1;",
    "1;2;",
    "1;2",
    "1\nx",
    "p()\np()\np();p()",
    ";1",
    "if (1) 2",
    "if (1) 2; else 3",
    "if (1) {2;3}",
    "if (1) {2;3}; 4",
    "if (1) {2;3} else {4;5}",
    "while (1);",
    "while(1) {}",
    "while (1) 2;",
    "while (1) {2;3}",
    "for (i = 0; i < 3; i++) i++",
    "for (i=0; i<3;) i++",
    "for (i=0;;) i++",
    "for (;i<3;) i++",
    "for (;;) i++",
    "debugger",
    "break",
    "break loop",
    "continue",
    "continue loop",
    "return",
    "return 1+2",
    "if (1) {return;}",
    "if (1) {return 2}",
    "throw 1+2",
    "try { 1 }",
    "try { 1 } catch (e) { 2 }",
    "try {1} finally {3}",
    "try {1} catch (e) {2} finally {3}",
    "var x",
    "var x, y",
    "var x=1, y",
    "var y, x=y=1",
    "function x(a) {return a}",
    "function x() {return 1}",
    "[1,2,3]",
    "[1+2,3+4,5+6]",
    "[1,[2,[[3]]]]",
    "({a: 1})",
    "({a: 1, b: 2})",
    "({})",
    "(function(a) { return a + 1 })",
    "(function f(a) { return a + 1 })",
    "return; 1;",
    "while (1) {return;2}",
    "switch(a) {case 1: break;}",
    "switch(a) {case 1: p(); break;}",
    "switch(a) {case 1: a; case 2: b; c;}",
    "switch(a) {case 1: a; b; default: c;}",
    "switch(a) {case 1: p(); break; break; }",
    "switch(a) {case 1: break; case 2: 1; case 3:}",
    "switch(a) {case 1: break; case 2: 1; case 3: default: break}",
    "switch(a) {case 1: break; case 3: default: break; case 2: 1}",
    "for (var i = 0; i < 3; i++) i++",
    "for (var i=0, j=i; i < 3; i++, j++) i++",
    "a%=1",
    "a*=1",
    "a/=1",
    "a+=1",
    "a-=1",
    "a|=1",
    "a&=1",
    "a<<=1",
    "a>>2",
    "a>>=1",
    "a>>>=1",
    "a=b=c+=1",
    "a=(b=(c+=1))",
    "\"foo\"",
    "var undefined = 1",
    "undefined",
    "{var x=1;2;}",
    "({get a() { return 1 }})",
    "({set a(b) { this.x = b }})",
    "({get a() { return 1 }, set b(c) { this.x = c }, d: 0})",
    "({get: function() {return 42;}})",
    "Object.defineProperty(o, \"foo\", {get: function() {return 42;}});",
    "({a: 0, \"b\": 1})",
    "({a: 0, 42: 1})",
    "({a: 0, 42.99: 1})",
    "({a: 0, })",
    "({true: 0, null: 1, undefined: 2, this: 3})",
    "[]",
    "[,2]",
    "[,]",
    "[,2]",
    "[,,,1,2]",
    "delete z",
    "delete (1+2)",
    "delete (delete z)",
    "delete delete z",
    "+ + + 2",
    "throw 'ex'",
    "switch(a) {case 1: try { 1; } catch (e) { 2; } finally {}; break}",
    "switch(a) {case 1: try { 1; } catch (e) { 2; } finally {break}; break}",
    "switch(a) {case 1: try { 1; } catch (e) { 2; } finally {break}; break; default: 1; break;}",
    "try {1} catch(e){}\n1",
    "try {1} catch(e){} 1",
    "switch(v) {case 0: break;} 1",
    "switch(a) {case 1: break; case 3: default: break; case 2: 1; default: 2}",
    "do { i-- } while (i > 0)",
    "if (false) 1; 1;",
    "while(true) 1; 1;",
    "while(true) {} 1;",
    "do {} while(false) 1;",
    "with (a) 1; 2;",
    "with (a) {1} 2;",
    "for(var i in a) {1}",
    "for(i in a) {1}",
    "!function(){function d(){}var x}();",
    "({get a() { function d(){} return 1 }})",
    "({set a(v) { function d(a){} d(v) }})",
    "{function d(){}var x}",
    "try{function d(){}var x}catch(e){function d(){}var x}finally{function d(){}var x}",
    "{} {}",
    "if(1){function d(){}var x}",
    "if(1){} else {function d(){}var x}",
    "var \\u0076, _\\u0077, a\\u0078b, жабоскрипт;",
    "a.in + b.for",
    "var x = { null: 5, else: 4 }",
    "lab: x=1"
  };
  const char *invalid[] = {
    "function(a) { return 1 }",
    "i\n++",
    "{a: 1, b: 2}",
    "({, a: 0})",
  };
  FILE *fp;
  const char *want_ast_db = "want_ast.db";
  char got_ast[102400];
  char want_ast[102400];
  char *next_want_ast = want_ast - 1;
  size_t want_ast_len;
  ast_init(&a, 0);

  /* Save with `make save_want_ast` */
#ifndef SAVE_AST

  ASSERT((fp = fopen(want_ast_db, "r")) != NULL);
  memset(want_ast, 0, sizeof(want_ast));
  fread(want_ast, sizeof(want_ast), 1, fp);
  ASSERT(feof(fp));
  fclose(fp);

  for (i = 0; i < (int) ARRAY_SIZE(cases); i++ ) {
    char *current_want_ast = next_want_ast + 1;
    ASSERT((next_want_ast = strchr(current_want_ast, '\0')) != NULL);
    want_ast_len = (size_t) (next_want_ast - current_want_ast);
    ASSERT((fp = fopen("/tmp/got_ast", "w")) != NULL);
    ast_free(&a);
    #if 0
    printf("-- Parsing \"%s\"\n", cases[i]);
    #endif
    ASSERT(parse(v7, &a, cases[i], 1) == V7_OK);

#ifdef VERBOSE_AST
    ast_dump(stdout, &a, 0);
#endif
    if (want_ast_len == 0) {
      printf("Test case not found in %s:\n", want_ast_db);
      ast_dump(stdout, &a, 0);
      abort();
    }
    ast_dump(fp, &a, 0);
    fclose(fp);

    ASSERT((fp = fopen("/tmp/got_ast", "r")) != NULL);
    memset(got_ast, 0, sizeof(got_ast));
    fread(got_ast, sizeof(got_ast), 1, fp);
    ASSERT(feof(fp));
    fclose(fp);
#if !defined(_WIN32)
    if (strncmp(got_ast, current_want_ast, sizeof(got_ast)) != 0) {
      fp = fopen("/tmp/want_ast", "w");
      fwrite(current_want_ast, want_ast_len, 1, fp);
      fclose(fp);
      system("diff -u /tmp/want_ast /tmp/got_ast");
    }
#endif
    ASSERT(strncmp(got_ast, current_want_ast, sizeof(got_ast)) == 0);
  }

#else /* SAVE_AST */

  (void) got_ast;
  (void) next_want_ast;
  (void) want_ast_len;
  ASSERT((fp = fopen(want_ast_db, "w")) != NULL);
  for (i = 0; i < (int) ARRAY_SIZE(cases); i++ ) {
    ast_free(&a);
    ASSERT(parse(v7, &a, cases[i], 1) == V7_OK);
    ast_dump(fp, &a, 0);
    fwrite("\0", 1, 1, fp);
  }
  fclose(fp);

#endif /* SAVE_AST */

  for (i = 0; i < (int) ARRAY_SIZE(invalid); i++ ) {
    ast_free(&a);
    ASSERT(parse(v7, &a, invalid[i], 0) == V7_ERROR);
  }

  return NULL;
}

static char *read_file(const char *path, size_t *size) {
  FILE *fp;
  struct stat st;
  char *data = NULL;
  if ((fp = fopen(path, "rb")) != NULL && !fstat(fileno(fp), &st)) {
    *size = st.st_size;
    data = (char *) malloc(*size + 1);
    if (data != NULL) {
      fread(data, 1, *size, fp);
      data[*size] = '\0';
    }
    fclose(fp);
  }
  return data;
}

static const char *test_ecmac(void) {
  struct ast a;
  int i, passed = 0;
  size_t db_len, driver_len;
  char *db = read_file("ecmac.db", &db_len);
  char *driver = read_file("ecma_driver.js", &driver_len);
  char *next_case = db - 1;
  struct v7 *v7;
#ifdef ECMA_FORK
  pid_t child;
#endif

  ast_init(&a, 0);

  for (i = 0; next_case < db + db_len; i++ ) {
    char *current_case = next_case + 1;
    ASSERT((next_case = strchr(current_case, '\0')) != NULL);

#if 0
    printf("-- Parsing %d: \"%s\"\n", i, current_case);
#endif
    v7 = v7_create();
    ASSERT(parse(v7, &a, current_case, 1) == V7_OK);
    ast_free(&a);
#ifdef ECMA_FORK
    if ((child = fork()) == 0) {
#endif
      if (v7_exec(v7, driver) == V7_UNDEFINED) {
        fprintf(stderr, "%s: %s\n", "Cannot load ECMA driver", v7->error_msg);
      } else {
        if (v7_exec(v7, current_case) == V7_UNDEFINED) {
          #if 0
          printf("FAILED ECMA TEST: [%s] -> [%s]\n", current_case, v7->error_msg);
          #endif
#ifdef ECMA_FORK
          exit(1);
#endif
        } else {
          passed++;
#ifdef ECMA_FORK
          exit(0);
#endif
        }
      }
#ifdef ECMA_FORK
    } else {
      int status;
      waitpid(child, &status, WNOHANG);
      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        passed++;
      }
    }
#endif
    v7_destroy(v7);
#if 0
    ast_dump(stdout, &a, 0);
#endif
  }
  printf("ECMA tests coverage: %.2lf%%\n", (double) passed / i * 100.0);

  free(db);
  return NULL;
}

static const char *test_string_encoding(void) {
  unsigned char buf[10] = ":-)";
  int llen;

  ASSERT(encode_varint(3, buf) == 1);
  ASSERT(decode_varint(buf, &llen) == 3);
  ASSERT(buf[0] == 3);
  ASSERT(llen == 1);

  ASSERT(encode_varint(127, buf) == 1);
  ASSERT(decode_varint(buf, &llen) == 127);
  ASSERT(buf[0] == 127);
  ASSERT(llen == 1);

  ASSERT(encode_varint(128, buf) == 2);
  ASSERT(decode_varint(buf, &llen) == 128);
  ASSERT(buf[0] == 128);
  ASSERT(buf[1] == 1);
  ASSERT(llen == 2);

  ASSERT(encode_varint(0x4000, buf) == 3);
  ASSERT(decode_varint(buf, &llen) == 0x4000);
  ASSERT(buf[0] == 128);
  ASSERT(buf[1] == 128);
  ASSERT(buf[2] == 1);
  ASSERT(llen == 3);

  return NULL;
}

static const char *test_interpreter(void) {
  struct v7 *v7 = v7_create();
  val_t v;

  v7_set_property(v7, v7->global_object, "x", -1, 0, v7_create_number(42.0));

  ASSERT((v = v7_exec(v7, "1%2/2")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "0.5"));

  ASSERT((v = v7_exec(v7, "1+x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "43"));
  ASSERT((v = v7_exec(v7, "2-'1'")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "1+2")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "3"));
  ASSERT((v = v7_exec(v7, "'1'+'2'")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"12\""));
  ASSERT((v = v7_exec(v7, "'1'+2")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"12\""));

  ASSERT((v = v7_exec(v7, "false+1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "true+1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));

  ASSERT((v = v7_exec(v7, "'1'<2")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "'1'>2")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));

  ASSERT((v = v7_exec(v7, "1==1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "1==2")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "'1'==1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "'1'!=0")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "'-1'==-1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "a={};a===a")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "a={};a!==a")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "a={};a==a")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "a={};a!=a")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "a={};b={};a===b")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "a={};b={};a!==b")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "a={};b={};a==b")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "a={};b={};a!=b")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "1-{}")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "NaN"));
  ASSERT((v = v7_exec(v7, "a={};a===(1-{})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "a={};a!==(1-{})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "a={};a==(1-{})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "a={};a!=(1-{})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "a={};a===1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "a={};a!==1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "a={};a==1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "a={};a!=1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));

  ASSERT((v = v7_exec(v7, "+'1'")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "-'-1'")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "v=[10+1,20*2,30/3]")) != V7_UNDEFINED);
  ASSERT(val_type(v7, v) == V7_TYPE_ARRAY_OBJECT);
  ASSERT(v7_array_length(v7, v) == 3);
  ASSERT(check_value(v7, v, "[11,40,10]"));
  ASSERT((v = v7_exec(v7, "v[0]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "11"));
  ASSERT((v = v7_exec(v7, "v[1]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "40"));
  ASSERT((v = v7_exec(v7, "v[2]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "10"));

  ASSERT((v = v7_exec(v7, "v=[10+1,undefined,30/3]")) != V7_UNDEFINED);
  ASSERT(v7_array_length(v7, v) == 3);
  ASSERT(check_value(v7, v, "[11,undefined,10]"));

  ASSERT((v = v7_exec(v7, "v=[10+1,,30/3]")) != V7_UNDEFINED);
  ASSERT(v7_array_length(v7, v) == 3);
  ASSERT(check_value(v7, v, "[11,,10]"));

  ASSERT((v = v7_exec(v7, "3,2,1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));

  ASSERT((v = v7_exec(v7, "x=1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));

  ASSERT((v = v7_exec(v7, "1+2; 1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "x=42; x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));
  ASSERT((v = v7_exec(v7, "x=y=42; x+y")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "84"));

  ASSERT((v = v7_exec(v7, "o={a: 1, b: 2}")) != V7_UNDEFINED);
  ASSERT((v = v7_exec(v7, "o['a'] + o['b']")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "3"));

  ASSERT((v = v7_exec(v7, "o.a + o.b")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "3"));

  ASSERT((v = v7_exec(v7, "Array(1,2)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[1,2]"));
  ASSERT((v = v7_exec(v7, "new Array(1,2)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[1,2]"));
  ASSERT((v = v7_exec(v7, "Object.isPrototypeOf(Array(1,2), Object.getPrototypeOf([]))")) != V7_UNDEFINED);
  ASSERT((v = v7_exec(v7, "a=[];r=a.push(1,2,3);[r,a]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[3,[1,2,3]]"));

  ASSERT((v = v7_exec(v7, "x=1;if(x>0){x=2};x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));
  ASSERT((v = v7_exec(v7, "x=1;if(x<0){x=2};x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "x=0;if(true)x=2;else x=3;x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));
  ASSERT((v = v7_exec(v7, "x=0;if(false)x=2;else x=3;x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "3"));

  ASSERT((v = v7_exec(v7, "y=1;x=5;while(x > 0){y=y*x;x=x-1};y")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "120"));
  ASSERT((v = v7_exec(v7, "y=1;x=5;do{y=y*x;x=x-1}while(x>0);y")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "120"));
  ASSERT((v = v7_exec(v7, "for(y=1,i=1;i<=5;i=i+1)y=y*i;y")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "120"));
  ASSERT((v = v7_exec(v7, "for(i=0;1;i++)if(i==5)break;i")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "5"));
  ASSERT((v = v7_exec(v7, "for(i=0;1;i++)if(i==5)break;i")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "5"));
  ASSERT((v = v7_exec(v7, "i=0;while(++i)if(i==5)break;i")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "5"));
  ASSERT((v = v7_exec(v7, "i=0;do{if(i==5)break}while(++i);i")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "5"));
  ASSERT((v = v7_exec(v7, "(function(){i=0;do{if(i==5)break}while(++i);i+=10})();i")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "15"));
  ASSERT((v = v7_exec(v7, "(function(){x=i=0;do{if(i==5)break;if(i%2)continue;x++}while(++i);i+=10})();[i,x]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[15,3]"));
  ASSERT((v = v7_exec(v7, "(function(){i=0;while(++i){if(i==5)break};i+=10})();i")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "15"));
  ASSERT((v = v7_exec(v7, "(function(){x=i=0;while(++i){if(i==5)break;if(i%2)continue;x++};i+=10})();[i,x]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[15,2]"));
  ASSERT((v = v7_exec(v7, "(function(){for(i=0;1;++i){if(i==5)break};i+=10})();i")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "15"));
  ASSERT((v = v7_exec(v7, "(function(){x=0;for(i=0;1;++i){if(i==5)break;if(i%2)continue;x++};i+=10})();[i,x]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[15,3]"));

  ASSERT((v = v7_exec(v7, "x=0;try{x=1};x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "x=0;try{x=1}finally{x=x+1};x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));
  ASSERT((v = v7_exec(v7, "x=0;try{x=1}catch(e){x=100}finally{x=x+1};x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));

  ASSERT((v = v7_exec(v7, "(function(a) {return a})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[function(a)]"));
  ASSERT((v = v7_exec(v7, "(function() {var x=1,y=2; return x})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[function(){var x,y}]"));
  ASSERT((v = v7_exec(v7, "(function(a) {var x=1,y=2; return x})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[function(a){var x,y}]"));
  ASSERT((v = v7_exec(v7, "(function(a,b) {var x=1,y=2; return x; var z})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[function(a,b){var x,y,z}]"));
  ASSERT((v = v7_exec(v7, "(function(a) {var x=1; for(var y in x){}; var z})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[function(a){var x,y,z}]"));
  ASSERT((v = v7_exec(v7, "(function(a) {var x=1; for(var y=0;y<x;y++){}; var z})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[function(a){var x,y,z}]"));
  ASSERT((v = v7_exec(v7, "(function() {var x=(function y(){for(var z;;){}})})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[function(){var x}]"));
  ASSERT((v = v7_exec(v7, "function square(x){return x*x;}")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[function square(x)]"));
  ASSERT((v = v7_exec(v7, "0;f=(function(x){return x*x;})")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[function(x)]"));

  ASSERT((v = v7_exec(v7, "f=(function(x){return x*x;}); f(2)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "4"));
  ASSERT((v = v7_exec(v7, "(function(x){x*x;})(2)")) == V7_UNDEFINED);
  ASSERT((v = v7_exec(v7, "f=(function(x){return x*x;x});v=f(2);v*2")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "8"));
  ASSERT((v = v7_exec(v7, "(function(x,y){return x+y;})(40,2)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));
  ASSERT((v = v7_exec(v7, "(function(x,y){if(x==40)return x+y})(40,2)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));
  ASSERT((v = v7_exec(v7, "(function(x,y){return x+y})(40)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "NaN"));
  ASSERT((v = v7_exec(v7, "(function(x){return x+y; var y})(40)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "NaN"));
  ASSERT((v = v7_exec(v7, "x=1;(function(a){return a})(40,(function(){x=x+1})())+x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));
  ASSERT((v = v7_exec(v7, "(function(){x=42;return;x=0})();x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));
  ASSERT((v = v7_exec(v7, "(function(){for(i=0;1;i++)if(i==5)return i})()")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "5"));
  ASSERT((v = v7_exec(v7, "(function(){i=0;while(++i)if(i==5)return i})()")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "5"));
  ASSERT((v = v7_exec(v7, "(function(){i=0;do{if(i==5)return i}while(++i)})()")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "5"));

  /* TODO(mkm): check for reference error being thrown */
  /* ASSERT((v = v7_exec(v7, "(function(x,y){return x+y})(40,2,(function(){return fail})())")) != V7_UNDEFINED); */

  ASSERT((v = v7_exec(v7, "x=42; (function(){return x})()")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));
  ASSERT((v = v7_exec(v7, "x=2; (function(x){return x})(40)+x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));
  ASSERT((v = v7_exec(v7, "x=1; (function(y){x=x+1; return y})(40)+x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));
  ASSERT((v = v7_exec(v7, "x=0;f=function(){x=42; return function() {return x}; var x};f()()")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));
  ASSERT((v = v7_exec(v7, "x=42;o={x:66,f:function(){return this}};o.f().x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "66"));
  ASSERT((v = v7_exec(v7, "x=42;o={x:66,f:function(){return this}};(1,o.f)().x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));
  ASSERT((v = v7_exec(v7, "x=66;o={x:42,f:function(){return this.x}};o.f()")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));

  ASSERT((v = v7_exec(v7, "o={};o.x=24")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "24"));
  ASSERT((v = v7_exec(v7, "o.a={};o.a.b={c:66};o.a.b.c")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "66"));
  ASSERT((v = v7_exec(v7, "o['a']['b'].c")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "66"));
  ASSERT((v = v7_exec(v7, "o={a:1}; o['a']=2;o.a")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));
  ASSERT((v = v7_exec(v7, "a={f:function(){return {b:55}}};a.f().b")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "55"));
  ASSERT((v = v7_exec(v7, "(function(){fox=1})();fox")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));

  ASSERT((v = v7_exec(v7, "fin=0;(function(){while(1){try{xxxx}finally{fin=1;return 1}}})();fin")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "ca=0;fin=0;(function(){try{(function(){try{xxxx}finally{fin=1}})()}catch(e){ca=1}})();fin+ca")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));
  ASSERT((v = v7_exec(v7, "x=0;try{throw 1}catch(e){x=42};x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));

  ASSERT((v = v7_exec(v7, "x=1;x=x<<3;x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "8"));
  ASSERT((v = v7_exec(v7, "x=1;x<<=4;x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "16"));
  ASSERT((v = v7_exec(v7, "x=1;x++")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));
  ASSERT((v = v7_exec(v7, "x=1;++x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));
  ASSERT((v = v7_exec(v7, "x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));
  ASSERT((v = v7_exec(v7, "o={x:1};o.x++")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "o.x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));

  ASSERT((v = v7_exec(v7, "typeof dummyx")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"undefined\""));
  ASSERT((v = v7_exec(v7, "typeof null")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"object\""));
  ASSERT((v = v7_exec(v7, "typeof 1")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"number\""));
  ASSERT((v = v7_exec(v7, "typeof (1+2)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"number\""));
  ASSERT((v = v7_exec(v7, "typeof 'test'")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"string\""));
  ASSERT((v = v7_exec(v7, "typeof [1,2]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"object\""));
  ASSERT((v = v7_exec(v7, "typeof function(){}")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"function\""));

  ASSERT((v = v7_exec(v7, "void(1+2)")) == V7_UNDEFINED);
  ASSERT((v = v7_exec(v7, "true?1:2")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "false?1:2")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));
  ASSERT((v = v7_exec(v7, "'a' in {a:1}")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "'b' in {a:1}")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "1 in [10,20]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "20 in [10,20]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));

  ASSERT((v = v7_exec(v7, "x=1; delete x; typeof x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"undefined\""));
  ASSERT((v = v7_exec(v7, "x=1; (function(){x=2;delete x; return typeof x})()")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"undefined\""));
  ASSERT((v = v7_exec(v7, "x=1; (function(){x=2;delete x})(); typeof x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"undefined\""));
  ASSERT((v = v7_exec(v7, "x=1; (function(){var x=2;delete x})(); x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "o={a:1};delete o.a;o")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "{}"));
  ASSERT((v = v7_exec(v7, "o={a:1};delete o['a'];o")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "{}"));
  ASSERT((v = v7_exec(v7, "x=0;if(delete 1 == true)x=42;x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "42"));

  ASSERT((v = v7_exec(v7, "o={};a=[o];o.a=a;a")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[{\"a\":[Circular]}]"));

  ASSERT((v = v7_exec(v7, "new TypeError instanceof Error")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "new TypeError instanceof TypeError")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "new Error instanceof Object")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "new Error instanceof TypeError")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  /* TODO(mkm): fix parser: should not require parenthesis */
  ASSERT((v = v7_exec(v7, "({}) instanceof Object")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));

  ASSERT((v = v7_exec(v7, "")) == V7_NULL);
#if 0
  ASSERT((v = v7_exec(v7, "x=0;a=1;o={a:2};with(o){x=a};x")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "2"));
#endif

  ASSERT((v = v7_exec(v7, "(function(){try {throw new Error}catch(e){c=e}})();c instanceof Error")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "delete e;(function(){try {throw new Error}catch(e){}})();typeof e")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "\"undefined\""));
  ASSERT((v = v7_exec(v7, "x=(function(){c=1;try {throw 1}catch(e){c=0};return c})()")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "0"));
  ASSERT((v = v7_exec(v7, "x=(function(){var c=1;try {throw 1}catch(e){c=0};return c})()")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "0"));
  ASSERT((v = v7_exec(v7, "c=1;x=(function(){try {throw 1}catch(e){var c=0};return c})();[c,x]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[1,0]"));
  ASSERT((v = v7_exec(v7, "c=1;x=(function(){try {throw 1}catch(e){c=0};return c})();[c,x]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[0,0]"));

  ASSERT((v = v7_exec(v7, "Object.keys(new Boolean(1))")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[]"));
  ASSERT((v = v7_exec(v7, "b={c:1};a=Object.create(b); a.d=4;Object.keys(a)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[\"d\"]"));
  ASSERT((v = v7_exec(v7, "Object.getOwnPropertyNames(new Boolean(1))")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[]"));
  ASSERT((v = v7_exec(v7, "b={c:1};a=Object.create(b); a.d=4;Object.getOwnPropertyNames(a)")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[\"d\"]"));
  ASSERT((v = v7_exec(v7, "o={};Object.defineProperty(o, \"x\", {value:2});[o.x,o]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[2,{}]"));
  ASSERT((v = v7_exec(v7, "o={};Object.defineProperties(o,{x:{value:2},y:{value:3,enumerable:true}});[o.x,o.y,o]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[2,3,{\"y\":3}]"));
  ASSERT((v = v7_exec(v7, "o={};Object.defineProperty(o, \"x\", {value:2,enumerable:true});[o.x,o]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[2,{\"x\":2}]"));
  ASSERT((v = v7_exec(v7, "o={};Object.defineProperty(o,'a',{value:1});o.propertyIsEnumerable('a')")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "o={};Object.defineProperty(o,'a',{value:1,enumerable:true});o.propertyIsEnumerable('a')")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "o={a:1};o.propertyIsEnumerable('a')")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "b={a:1};o=Object.create(b);o.propertyIsEnumerable('a')")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "b={a:1};o=Object.create(b);o.hasOwnProperty('a')")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "false"));
  ASSERT((v = v7_exec(v7, "o={a:1};o.hasOwnProperty('a')")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "true"));
  ASSERT((v = v7_exec(v7, "o={a:1};d=Object.getOwnPropertyDescriptor(o, 'a');"
                      "[d.value,d.writable,d.enumerable,d.configurable]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[1,true,true,true]"));
  ASSERT((v = v7_exec(v7, "o={};Object.defineProperty(o,'a',{value:1,enumerable:true});"
                      "d=Object.getOwnPropertyDescriptor(o, 'a');"
                      "[d.value,d.writable,d.enumerable,d.configurable]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[1,false,true,false]"));
  ASSERT((v = v7_exec(v7, "o=Object.defineProperty({},'a',{value:1,enumerable:true});o.a=2;o.a")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "o=Object.defineProperty({},'a',{value:1,enumerable:true});r=delete o.a;[r,o.a]")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "[false,1]"));

  ASSERT((v = v7_exec(v7, "r=0;o={a:1,b:2};for(i in o){r+=o[i]};r")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "3"));
  ASSERT((v = v7_exec(v7, "r=0;o={a:1,b:2};for(var i in o){r+=o[i]};r")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "3"));
  ASSERT((v = v7_exec(v7, "r=1;for(var i in null){r=0};r")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "r=1;for(var i in undefined){r=0};r")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));
  ASSERT((v = v7_exec(v7, "r=1;for(var i in 42){r=0};r")) != V7_UNDEFINED);
  ASSERT(check_value(v7, v, "1"));

  /* check execution failure caused by bad parsing */
  ASSERT((v = v7_exec(v7, "function")) == V7_UNDEFINED);
  return NULL;
}

static const char *test_strings(void) {
  val_t s = 0;
  struct v7 *v7;

  v7 = v7_create();

  s = v7_string_to_value(v7, "hi", 2, 1);
  ASSERT(memcmp(&s, "\x02\x68\x69\x00\x00\x00\xfa\xff", sizeof(s)) == 0);
  ASSERT(v7->owned_strings.len == 0);
  ASSERT(v7->foreign_strings.len == 0);

  s = v7_string_to_value(v7, "longer one", 10, 1);
  ASSERT(v7->owned_strings.len == 11);
  ASSERT(memcmp(v7->owned_strings.buf, "\x0alonger one", 11) == 0);
  ASSERT(memcmp(&s, "\x00\x00\x00\x00\x00\x00\xf9\xff", sizeof(s)) == 0);

  s = v7_string_to_value(v7, "with embedded \x00 one", 19, 1);

  ASSERT(v7->owned_strings.len == 31);
  ASSERT(memcmp(&s, "\x0b\x00\x00\x00\x00\x00\xf9\xff", sizeof(s)) == 0);
  ASSERT(memcmp(v7->owned_strings.buf, "\x0alonger one"
         "\x13with embedded \x00 one" , 31) == 0);

  v7_destroy(v7);

  return NULL;
}

static const char *test_to_json(void) {
  char buf[10], *p;
  struct v7 *v7 = v7_create();
  val_t v;

  v = v7_exec(v7, "123.45");
  ASSERT((p = v7_to_json(v7, v, buf, sizeof(buf))) == buf);
  ASSERT(strcmp(p, "123.45") == 0);
  ASSERT((p = v7_to_json(v7, v, buf, 3)) != buf);
  ASSERT(strcmp(p, "123.45") == 0);
  free(p);

  return NULL;
}

static const char *run_all_tests(const char *filter) {
  RUN_TEST(test_to_json);
  RUN_TEST(test_tokenizer);
  RUN_TEST(test_string_encoding);
  RUN_TEST(test_is_true);
  RUN_TEST(test_closure);
  RUN_TEST(test_native_functions);
  RUN_TEST(test_stdlib);
  RUN_TEST(test_runtime);
  RUN_TEST(test_parser);
  RUN_TEST(test_ecmac);
  RUN_TEST(test_interpreter);
  RUN_TEST(test_strings);
  return NULL;
}

int main(int argc, char *argv[]) {
  const char *filter = argc > 1 ? argv[1] : "";
  const char *fail_msg = run_all_tests(filter);
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", static_num_tests);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
