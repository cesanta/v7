/* Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
 * Copyright (c) 2013-2015 Cesanta Software Limited
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
 * license, as set out in <https://www.cesanta.com/license>.
 */

#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h> /* for O_RDWR */

#ifndef _WIN32
#include <unistd.h>
#ifndef __WATCOM__
#include <pthread.h>
#endif
#endif

#ifdef V7_UNAMALGAMATED_UNIT_TEST
#include "../v7.h"
#include "../src/internal.h"
#include "../src/gc.h"
#include "../src/compiler.h"
#else
#include "../v7.c"
#endif

#ifdef _WIN32
#define isinf(x) (!_finite(x))
#ifndef NAN
#define NAN atof("NAN")
#endif
/* #define INFINITY    BLAH */
#endif

extern long timezone;

#include "test_util.h"

#ifdef _WIN32
#define isnan(x) _isnan(x)
#endif

static enum v7_err parse_js(struct v7 *v7, const char *src, struct ast *a) {
  enum v7_err parse_result = parse(v7, a, src, 1 /* verbose */);
  if (parse_result != V7_OK) {
    fprintf(stderr, "Parse error. Expression:\n  %s\nMessage:\n  %s\n", src,
            v7->error_msg);
  }
  return parse_result;
}

int STOP = 0; /* For xcode breakpoints conditions */

static int check_value(struct v7 *v7, val_t v, const char *str) {
  int res = 1;
  char buf[2048];
  char *p = v7_to_json(v7, v, buf, sizeof(buf));
  if (strcmp(p, str) != 0) {
    _strfail(p, str, -1);
    res = 0;
  }
  if (p != buf) {
    free(p);
  }
  return res;
}

static int check_num(struct v7 *v7, val_t v, double num) {
  int ret = isnan(num) ? isnan(v7_to_number(v)) : v7_to_number(v) == num;
  (void) v7;
  if (!ret) {
    printf("Num: want %f got %f\n", num, v7_to_number(v));
  }

  return ret;
}

static int check_bool(val_t v, int is_true) {
  int b = v7_to_boolean(v);
  return is_true ? b : !b;
}

static int check_str(struct v7 *v7, val_t v, const char *expected) {
  size_t n1, n2 = strlen(expected);
  const char *actual = v7_to_string(v7, &v, &n1);
  int ret = (n1 == n2 && memcmp(actual, expected, n1) == 0);
  if (!ret) {
    _strfail(actual, expected, -1);
  }
  return ret;
}

static int test_if_expr(struct v7 *v7, const char *expr, int result) {
  val_t v;
  if (v7_exec(v7, &v, expr) != V7_OK) return 0;
  return result == (v7_is_true(v7, v) ? 1 : 0);
}

#define ASSERT_EVAL_OK(v7, js_expr)                     \
  do {                                                  \
    v7_val_t v;                                         \
    enum v7_err e;                                      \
    num_tests++;                                        \
    e = v7_exec(v7, &v, js_expr);                       \
    if (e != V7_OK) {                                   \
      printf("Exec '%s' failed, err=%d\n", js_expr, e); \
      FAIL("ASSERT_EVAL_OK(" #js_expr ")", __LINE__);   \
    }                                                   \
  } while (0)

#define _ASSERT_EVAL_EQ(v7, js_expr, expected, check_fun)            \
  do {                                                               \
    v7_val_t v;                                                      \
    enum v7_err e;                                                   \
    int r = 1;                                                       \
    num_tests++;                                                     \
    e = v7_exec(v7, &v, js_expr);                                    \
    if (e != V7_OK) {                                                \
      printf("Exec '%s' failed, err=%d\n", js_expr, e);              \
      r = 0;                                                         \
    } else {                                                         \
      r = check_fun(v7, v, expected);                                \
    }                                                                \
    if (r == 0) {                                                    \
      FAIL("ASSERT_EVAL_EQ(" #js_expr ", " #expected ")", __LINE__); \
    }                                                                \
  } while (0)

#define ASSERT_EVAL_EQ(v7, js_expr, expected) \
  _ASSERT_EVAL_EQ(v7, js_expr, expected, check_value)
#define ASSERT_EVAL_NUM_EQ(v7, js_expr, expected) \
  _ASSERT_EVAL_EQ(v7, js_expr, expected, check_num)
#define ASSERT_EVAL_STR_EQ(v7, js_expr, expected) \
  _ASSERT_EVAL_EQ(v7, js_expr, expected, check_str)

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
  ASSERT(test_if_expr(v7, "Infinity", 1));
  ASSERT(test_if_expr(v7, "-Infinity", 1));
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
  struct v7 *v7 = v7_create();

  ASSERT_EVAL_OK(v7, "function a(x){return function(y){return x*y}}");
  ASSERT_EVAL_OK(v7, "var f1 = a(5);");
  ASSERT_EVAL_OK(v7, "var f2 = a(7);");
  ASSERT_EVAL_EQ(v7, "f1(3);", "15");
  ASSERT_EVAL_EQ(v7, "f2(3);", "21");

  v7_destroy(v7);
  return NULL;
}

static val_t adder(struct v7 *v7, val_t this_obj, val_t args) {
  double sum = 0;
  unsigned long i;

  (void) this_obj;
  for (i = 0; i < v7_array_length(v7, args); i++) {
    sum += v7_to_number(v7_array_get(v7, args, i));
  }
  return v7_create_number(sum);
}

static const char *test_native_functions(void) {
  struct v7 *v7 = v7_create();

  ASSERT_EQ(v7_set_property(v7, v7_get_global_object(v7), "adder", 5, 0,
                            v7_create_cfunction(adder)),
            0);
  ASSERT_EVAL_EQ(v7, "adder(1, 2, 3 + 4);", "10");
  v7_destroy(v7);

  return NULL;
}

static const char *test_stdlib(void) {
  v7_val_t v;
  struct v7 *v7 = v7_create();

  ASSERT_EVAL_EQ(v7, "Boolean()", "false");
  ASSERT_EVAL_EQ(v7, "Boolean(0)", "false");
  ASSERT_EVAL_EQ(v7, "Boolean(1)", "true");
  ASSERT_EVAL_EQ(v7, "Boolean([])", "true");
  ASSERT_EVAL_EQ(v7, "new Boolean([])", "{}");

  /* Math */
  ASSERT_EVAL_EQ(v7, "Math.sqrt(144)", "12");

  /* Number */
  ASSERT_EVAL_NUM_EQ(v7, "Math.PI", M_PI);
  ASSERT_EVAL_NUM_EQ(v7, "Number.NaN", NAN);
  ASSERT_EQ(v7_exec(v7, &v, "1 == 2"), V7_OK);
  ASSERT(check_bool(v, 0));
  ASSERT_EQ(v7_exec(v7, &v, "1 + 2 * 7 === 15"), V7_OK);
  ASSERT(check_bool(v, 1));
  ASSERT_EQ(v7_exec(v7, &v, "Number(1.23) === 1.23"), V7_OK);
  ASSERT(check_bool(v, 1));
  ASSERT_EVAL_NUM_EQ(v7, "Number(1.23)", 1.23);
  ASSERT_EVAL_OK(v7, "new Number(21.23)");

  /* Cesanta-specific String API */
  ASSERT_EVAL_NUM_EQ(v7, "'ы'.length", 1);
  ASSERT_EVAL_NUM_EQ(v7, "'ы'.charCodeAt(0)", 1099);
  ASSERT_EVAL_NUM_EQ(v7, "'ы'.blen", 2);
  ASSERT_EVAL_NUM_EQ(v7, "'ы'.at(0)", 0xd1);
  ASSERT_EVAL_NUM_EQ(v7, "'ы'.at(1)", 0x8b);
  ASSERT_EVAL_NUM_EQ(v7, "'ы'.at(2)", NAN);

  /* String */
  ASSERT_EVAL_NUM_EQ(v7, "'hello'.charCodeAt(1)", 'e');
  ASSERT_EVAL_NUM_EQ(v7, "'hello'.charCodeAt(4)", 'o');
  ASSERT_EVAL_NUM_EQ(v7, "'hello'.charCodeAt(5)", NAN);
  ASSERT_EVAL_NUM_EQ(v7, "'hello'.indexOf()", -1.0);
  ASSERT_EVAL_NUM_EQ(v7, "'HTTP/1.0\\r\\n'.indexOf('\\r\\n')", 8.0);
  ASSERT_EVAL_NUM_EQ(v7, "'hi there'.indexOf('e')", 5.0);
  ASSERT_EVAL_NUM_EQ(v7, "'hi there'.indexOf('e', 6)", 7.0);
  ASSERT_EVAL_NUM_EQ(v7, "'hi there'.indexOf('e', NaN)", 5.0);
  ASSERT_EVAL_NUM_EQ(v7, "'hi there'.indexOf('e', -Infinity)", 5.0);
  ASSERT_EVAL_NUM_EQ(v7, "'hi there'.indexOf('e', Infinity)", -1.0);
  ASSERT_EVAL_NUM_EQ(v7, "'hi there'.indexOf('e', 8)", -1.0);
  ASSERT_EVAL_NUM_EQ(v7, "'aabb'.indexOf('a', false)", 0.0);
  ASSERT_EVAL_NUM_EQ(v7, "'aabb'.indexOf('a', true)", 1.0);
  ASSERT_EVAL_STR_EQ(v7, "'hi there'.substr(3, 2)", "th");
  ASSERT_EVAL_STR_EQ(v7, "'hi there'.substring(3, 5)", "th");
  ASSERT_EVAL_STR_EQ(v7, "'hi there'.substr(3)", "there");
  ASSERT_EVAL_STR_EQ(v7, "'hi there'.substr(-2)", "re");
  ASSERT_EVAL_STR_EQ(v7, "'hi there'.substr(NaN)", "hi there");
  ASSERT_EVAL_STR_EQ(v7, "'hi there'.substr(0, 300)", "hi there");
#if V7_ENABLE__RegExp
  ASSERT_EQ(v7_exec(v7, &v, "'dew dee'.match(/\\d+/)"), V7_OK);
  ASSERT_EQ(v, v7_create_null());
  ASSERT_EVAL_OK(v7, "m = 'foo 1234 bar'.match(/\\S+ (\\d+)/)");
  ASSERT_EVAL_NUM_EQ(v7, "m.length", 2.0);
  ASSERT_EVAL_STR_EQ(v7, "m[0]", "foo 1234");
  ASSERT_EVAL_STR_EQ(v7, "m[1]", "1234");
  ASSERT_EQ(v7_exec(v7, &v, "m[2]"), V7_OK);
  ASSERT(v7_is_undefined(v));
  ASSERT_EVAL_OK(v7, "m = 'should match empty string at index 0'.match(/x*/)");
  ASSERT_EVAL_NUM_EQ(v7, "m.length", 1.0);
  ASSERT_EVAL_STR_EQ(v7, "m[0]", "");
  ASSERT_EVAL_NUM_EQ(v7, "m = 'aa bb cc'.split(); m.length", 1.0);
  ASSERT_EVAL_NUM_EQ(v7, "m = 'aa bb cc'.split(''); m.length", 8.0);
  ASSERT_EVAL_NUM_EQ(v7, "m = 'aa bb cc'.split(RegExp('')); m.length", 8.0);
  ASSERT_EVAL_NUM_EQ(v7, "m = 'aa bb cc'.split(/x*/); m.length", 8.0);
  ASSERT_EVAL_NUM_EQ(v7, "m = 'aa bb cc'.split(/(x)*/); m.length", 16.0);
  ASSERT_EVAL_STR_EQ(v7, "m[0]", "a");
  ASSERT_EQ(v7_exec(v7, &v, "m[1]"), V7_OK);
  ASSERT(v7_is_undefined(v));
  ASSERT_EVAL_NUM_EQ(v7, "m = 'aa bb cc'.split(' '); m.length", 3.0);
  ASSERT_EVAL_NUM_EQ(v7, "m = 'aa bb cc'.split(' ', 2); m.length", 2.0);
  ASSERT_EVAL_NUM_EQ(v7, "m = 'aa bb cc'.split(/ /, 2); m.length", 2.0);
  ASSERT_EVAL_NUM_EQ(v7, "'aa bb cc'.substr(0, 4).split(' ').length", 2.0);
  ASSERT_EVAL_STR_EQ(v7, "'aa bb cc'.substr(0, 4).split(' ')[1]", "b");
  ASSERT_EVAL_NUM_EQ(
      v7, "({z: '123456'}).z.toString().substr(0, 3).split('').length", 3.0);
#endif /* V7_ENABLE__RegExp */
  ASSERT_EVAL_STR_EQ(v7, "String('hi')", "hi");
  ASSERT_EVAL_OK(v7, "new String('blah')");
  ASSERT_EVAL_NUM_EQ(v7, "(String.fromCharCode(0,1) + '\\x00\\x01').length", 4);
  ASSERT_EVAL_NUM_EQ(
      v7, "(String.fromCharCode(1,0) + '\\x00\\x01').charCodeAt(1)", 0);
  ASSERT_EVAL_NUM_EQ(
      v7, "(String.fromCharCode(0,1) + '\\x00\\x01').charCodeAt(1)", 1);

  /* Date() tests interact with external object (local date & time), so
      if host have strange date/time setting it won't be work */

  ASSERT_EVAL_EQ(v7, "Number(new Date('IncDec 01 2015 00:00:00'))", "NaN");
  ASSERT_EVAL_EQ(v7, "Number(new Date('My Jul 01 2015 00:00:00'))", "NaN");

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
  unsigned int i = 1;

  skip_to_next_tok(&p);

  /* Make sure divisions are parsed correctly - set previous token */
  while ((tok = get_tok(&p, &num, i > TOK_REGEX_LITERAL ? TOK_NUMBER : tok)) !=
         TOK_END_OF_INPUT) {
    skip_to_next_tok(&p);
    ASSERT_EQ(tok, i);
    i++;
  }
  ASSERT_EQ(i, TOK_BREAK);

  p = "/foo/";
  ASSERT_EQ(get_tok(&p, &num, TOK_NUMBER), TOK_DIV);

  p = "/foo/";
  ASSERT_EQ(get_tok(&p, &num, TOK_COMMA), TOK_REGEX_LITERAL);

  p = "/foo";
  ASSERT_EQ(get_tok(&p, &num, TOK_COMMA), TOK_DIV);

  p = "/fo\\/o";
  ASSERT_EQ(get_tok(&p, &num, TOK_COMMA), TOK_DIV);

  return NULL;
}

static const char *test_runtime(void) {
  struct v7 *v7 = v7_create();
  val_t v;
  struct v7_property *p;
  size_t n;
  const char *s;
  int i;
  char test_str[] =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
      "sed do eiusmod tempor incididunt ut labore et dolore magna "
      "aliqua. Ut enim ad minim veniam, quis nostrud exercitation "
      "ullamco laboris nisi ut aliquip ex ea commodo consequat. "
      "Duis aute irure dolor in reprehenderit in voluptate velit "
      "esse cillum dolore eu fugiat nulla pariatur. Excepteur "
      "sint occaecat cupidatat non proident, sunt in culpa qui "
      "officia deserunt mollit anim id est laborum.";

  v = v7_create_null();
  ASSERT(v7_is_null(v));

  v = v7_create_undefined();
  ASSERT(v7_is_undefined(v));

  v = v7_create_number(1.0);
  ASSERT_EQ(val_type(v7, v), V7_TYPE_NUMBER);
  ASSERT_EQ(v7_to_number(v), 1.0);
  ASSERT(check_value(v7, v, "1"));

  v = v7_create_number(1.5);
  ASSERT_EQ(v7_to_number(v), 1.5);
  ASSERT(check_value(v7, v, "1.5"));

  v = v7_create_boolean(1);
  ASSERT_EQ(val_type(v7, v), V7_TYPE_BOOLEAN);
  ASSERT_EQ(v7_to_boolean(v), 1);
  ASSERT(check_value(v7, v, "true"));

  v = v7_create_boolean(0);
  ASSERT(check_value(v7, v, "false"));

  v = v7_create_string(v7, "foo", 3, 1);
  ASSERT_EQ(val_type(v7, v), V7_TYPE_STRING);
  v7_to_string(v7, &v, &n);
  ASSERT_EQ(n, 3);
  s = "\"foo\"";
  ASSERT(check_value(v7, v, s));

  for (i = 1; i < (int) sizeof(test_str); i++) {
    v = v7_create_string(v7, 0, i, 1);
    s = v7_to_string(v7, &v, &n);
    memcpy((char *) s, test_str, i);
    ASSERT_EQ(val_type(v7, v), V7_TYPE_STRING);
    s = v7_to_string(v7, &v, &n);
    ASSERT(n == (size_t) i);
    ASSERT(memcmp(s, test_str, n) == 0);
  }

  v = v7_create_object(v7);
  ASSERT_EQ(val_type(v7, v), V7_TYPE_GENERIC_OBJECT);
  ASSERT(v7_to_object(v) != NULL);
  ASSERT(v7_to_object(v)->prototype != NULL);
  ASSERT(v7_to_object(v)->prototype->prototype == NULL);

  ASSERT_EQ(v7_set_property(v7, v, "foo", -1, 0, v7_create_null()), 0);
  ASSERT((p = v7_get_property(v7, v, "foo", -1)) != NULL);
  ASSERT_EQ(p->attributes, 0);
  ASSERT(v7_is_null(p->value));
  ASSERT(check_value(v7, p->value, "null"));

  ASSERT_EQ(v7_set_property(v7, v, "foo", -1, 0, v7_create_undefined()), 0);
  ASSERT((p = v7_get_property(v7, v, "foo", -1)) != NULL);
  ASSERT(check_value(v7, p->value, "undefined"));

  ASSERT(v7_set_property(v7, v, "foo", -1, 0,
                         v7_create_string(v7, "bar", 3, 1)) == 0);
  ASSERT((p = v7_get_property(v7, v, "foo", -1)) != NULL);
  s = "\"bar\"";
  ASSERT(check_value(v7, p->value, s));

  ASSERT(v7_set_property(v7, v, "foo", -1, 0,
                         v7_create_string(v7, "zar", 3, 1)) == 0);
  ASSERT((p = v7_get_property(v7, v, "foo", -1)) != NULL);
  s = "\"zar\"";
  ASSERT(check_value(v7, p->value, s));

  ASSERT_EQ(v7_del_property(v7, v, "foo", ~0), 0);
  ASSERT(v7_to_object(v)->properties == NULL);
  ASSERT_EQ(v7_del_property(v7, v, "foo", -1), -1);
  ASSERT(v7_set_property(v7, v, "foo", -1, 0,
                         v7_create_string(v7, "bar", 3, 1)) == 0);
  ASSERT(v7_set_property(v7, v, "bar", -1, 0,
                         v7_create_string(v7, "foo", 3, 1)) == 0);
  ASSERT(v7_set_property(v7, v, "aba", -1, 0,
                         v7_create_string(v7, "bab", 3, 1)) == 0);
  ASSERT_EQ(v7_del_property(v7, v, "foo", -1), 0);
  ASSERT((p = v7_get_property(v7, v, "foo", -1)) == NULL);
  ASSERT_EQ(v7_del_property(v7, v, "aba", -1), 0);
  ASSERT((p = v7_get_property(v7, v, "aba", -1)) == NULL);
  ASSERT_EQ(v7_del_property(v7, v, "bar", -1), 0);
  ASSERT((p = v7_get_property(v7, v, "bar", -1)) == NULL);

  v = v7_create_object(v7);
  ASSERT_EQ(v7_set_property(v7, v, "foo", -1, 0, v7_create_number(1.0)), 0);
  ASSERT((p = v7_get_property(v7, v, "foo", -1)) != NULL);
  ASSERT((p = v7_get_property(v7, v, "f", -1)) == NULL);

  v = v7_create_object(v7);
  ASSERT_EQ(v7_set_property(v7, v, "foo", -1, 0, v), 0);
  s = "{\"foo\":[Circular]}";
  ASSERT(check_value(v7, v, s));

  v = v7_create_object(v7);
  ASSERT(v7_set_property(v7, v, "foo", -1, V7_PROPERTY_DONT_DELETE,
                         v7_create_number(1.0)) == 0);
  s = "{\"foo\":1}";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_set(v7, v, "foo", -1, V7_PROPERTY_DONT_DELETE,
                v7_create_number(2.0)) == 0);
  s = "{\"foo\":2}";
  ASSERT(check_value(v7, v, s));
  ASSERT_EQ(v7_to_number(v7_get(v7, v, "foo", -1)), 2.0);
  ASSERT(v7_get_property(v7, v, "foo", -1)->attributes &
         V7_PROPERTY_DONT_DELETE);
  ASSERT_EQ(v7_set_property(v7, v, "foo", -1, V7_PROPERTY_READ_ONLY,
                            v7_create_number(1.0)),
            0);
  ASSERT(v7_set(v7, v, "foo", -1, 0, v7_create_number(2.0)) != 0);
  s = "{\"foo\":1}";
  ASSERT(check_value(v7, v, s));

  v = v7_create_string(v7, "fooakbar", 8, 1);
  for (i = 0; i < 100; i++) {
    s = v7_to_string(v7, &v, &n);
    v7_create_string(v7, s, 8, 1);
  }

  v7_destroy(v7);
  return NULL;
}

static const char *test_apply(void) {
  struct v7 *v7 = v7_create();
  val_t v, fn;

  fn = v7_get(v7, v7->global_object, "test0", 5); // no such function
  ASSERT_EQ(v7_apply(v7, &v, fn, v7->global_object, v7_create_undefined()), V7_EXEC_EXCEPTION);

  ASSERT_EQ(v7_exec(v7, &v, "function test1(){return 1}"), V7_OK);
  fn = v7_get(v7, v7->global_object, "test1", 5);
  ASSERT_EQ(v7_apply(v7, &v, fn, v7->global_object, v7_create_undefined()), V7_OK);
  ASSERT(check_num(v7, v, 1));

  ASSERT_EQ(v7_exec(v7, &v, "function test2(){throw 2}"), V7_OK);
  fn = v7_get(v7, v7->global_object, "test2", 5);
  ASSERT_EQ(v7_apply(v7, &v, fn, v7->global_object, v7_create_undefined()), V7_EXEC_EXCEPTION);
  ASSERT(check_num(v7, v, 2));

  v7_destroy(v7);
  return NULL;
}

static const char *test_dense_arrays(void) {
  struct v7 *v7 = v7_create();
  val_t a;

  a = v7_create_dense_array(v7);

  v7_array_set(v7, a, 0, v7_create_number(42));
  ASSERT(check_num(v7, v7_array_get(v7, a, 0), 42));
  ASSERT_EQ(v7_array_length(v7, a), 1);

  v7_array_set(v7, a, 1, v7_create_number(24));
  ASSERT(check_num(v7, v7_array_get(v7, a, 0), 42));
  ASSERT(check_num(v7, v7_array_get(v7, a, 1), 24));
  ASSERT_EQ(v7_array_length(v7, a), 2);

  a = v7_create_dense_array(v7);
  v7_array_set(v7, a, 0, v7_create_number(42));
  v7_array_set(v7, a, 2, v7_create_number(42));
  ASSERT_EQ(v7_array_length(v7, a), 3);

  a = v7_create_dense_array(v7);
  v7_array_set(v7, a, 1, v7_create_number(42));
  ASSERT_EQ(v7_array_length(v7, a), 2);

  ASSERT_EVAL_OK(v7, "function mka(){return arguments}");

  ASSERT_EVAL_EQ(v7, "a=mka(1,2,3);a.splice(0,1);a", "[2,3]");
  ASSERT_EVAL_EQ(v7, "a=mka(1,2,3);a.splice(2,1);a", "[1,2]");
  ASSERT_EVAL_EQ(v7, "a=mka(1,2,3);a.splice(1,1);a", "[1,3]");

  ASSERT_EVAL_EQ(v7, "a=mka(1,2,3);a.slice(0,1)", "[1]");
  ASSERT_EVAL_EQ(v7, "a=mka(1,2,3);a.slice(2,3)", "[3]");
  ASSERT_EVAL_EQ(v7, "a=mka(1,2,3);a.slice(1,3)", "[2,3]");

  ASSERT_EVAL_NUM_EQ(v7, "a=mka(1,2,3);a.indexOf(3)", 2);

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
    "while(1) break",
    "while(1) break loop",
    "while(1) continue",
    "while(1) continue loop",
    "function f() {return}",
    "function f() {return 1+2}",
    "function f() {if (1) {return;}}",
    "function f() {if (1) {return 2}}",
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
    "(function f() { return; 1;})",
    "function f() {while (1) {return;2}}",
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
    "u",
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
    "switch(a) {case 1: try { 1; } catch (e) { 2; } finally {break}; break; "
    "default: 1; break;}",
    "try {1} catch(e){}\n1",
    "try {1} catch(e){} 1",
    "switch(v) {case 0: break;} 1",
    "switch(a) {case 1: break; case 3: default: break; case 2: 1; default: "
    "2}",
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
    "try{function d(){}var x}catch(e){function d(){}var x}finally{function "
    "d(){}var x}",
    "{} {}",
    "if(1){function d(){}var x}",
    "if(1){} else {function d(){}var x}",
#if V7_ENABLE__UTF
    "var \\u0076, _\\u0077, a\\u0078b, жабоскрипт;",
#else
    "",
#endif
    "a.in + b.for",
    "var x = { null: 5, else: 4 }",
    "lab: x=1",
    "'use strict';0;'use strict';",
    "'use strict';if(0){'use strict';}",
    "(function(){'use strict';0;'use strict';})()"
  };
  const char *invalid[] = {
      "function(a) { return 1 }", "i\n++", "{a: 1, b: 2}", "({, a: 0})",
      "break", "break loop", "continue", "continue loop", "return",
      "return 1+2", "if (1) {return;}", "if (1) {return 2}", "({g x() {}})'",
      "({s x() {}})'", "(function(){'use strict'; with({}){}})", "v = [",
      "var v = ["};
  FILE *fp;
  const char *want_ast_db = "want_ast.db";
  char got_ast[102400];
  char want_ast[102400];
  char *next_want_ast = want_ast;
  size_t want_ast_len;
  ast_init(&a, 0);

/* Save with `make save_want_ast` */
#ifndef SAVE_AST

  ASSERT((fp = fopen(want_ast_db, "r")) != NULL);
  memset(want_ast, 0, sizeof(want_ast));
  if (fread(want_ast, sizeof(want_ast), 1, fp) < sizeof(want_ast)) {
    ASSERT_EQ(ferror(fp), 0);
  }
  ASSERT(feof(fp));
  fclose(fp);

  for (i = 0; i < (int) ARRAY_SIZE(cases); i++) {
    char *current_want_ast = next_want_ast;
    ast_free(&a);
    ASSERT((next_want_ast = strchr(current_want_ast, '\0') + 1) != NULL);
    if (cases[i][0] == '\0') continue;
    want_ast_len = (size_t)(next_want_ast - current_want_ast - 1);
    ASSERT((fp = fopen("/tmp/got_ast", "w")) != NULL);
#if 0
      printf("-- Parsing \"%s\"\n", cases[i]);
#endif
    ASSERT_EQ(parse_js(v7, cases[i], &a), V7_OK);

    if (want_ast_len == 0) {
      printf("Test case not found in %s:\n", want_ast_db);
      v7_compile(cases[i], 0, stdout);
      abort();
    }
    v7_compile(cases[i], 0, fp);
    fclose(fp);

    ASSERT((fp = fopen("/tmp/got_ast", "r")) != NULL);
    memset(got_ast, 0, sizeof(got_ast));
    if (fread(got_ast, sizeof(got_ast), 1, fp) < sizeof(got_ast)) {
      ASSERT_EQ(ferror(fp), 0);
    }
    ASSERT(feof(fp));
    fclose(fp);
#if !defined(_WIN32)
    if (strncmp(got_ast, current_want_ast, sizeof(got_ast)) != 0) {
      fp = fopen("/tmp/want_ast", "w");
      fwrite(current_want_ast, want_ast_len, 1, fp);
      fclose(fp);
      ASSERT(system("diff -u /tmp/want_ast /tmp/got_ast") != -1);
    }
    ASSERT_EQ(strncmp(got_ast, current_want_ast, sizeof(got_ast)), 0);
#endif
  }

#else /* SAVE_AST */

  (void) got_ast;
  (void) next_want_ast;
  (void) want_ast_len;
  ASSERT((fp = fopen(want_ast_db, "w")) != NULL);
  for (i = 0; i < (int) ARRAY_SIZE(cases); i++) {
    ast_free(&a);
    ASSERT_EQ(parse_js(v7, cases[i], &a), V7_OK);
    v7_compile(cases[i], 0, fp);
    fwrite("\0", 1, 1, fp);
  }
  fclose(fp);

#endif /* SAVE_AST */

  for (i = 0; i < (int) ARRAY_SIZE(invalid); i++) {
    ast_free(&a);
#if 0
    printf("-- Parsing \"%s\"\n", invalid[i]);
#endif
    ASSERT_EQ(parse(v7, &a, invalid[i], 0), V7_SYNTAX_ERROR);
  }

  ast_free(&a);
  v7_destroy(v7);
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
      if (fread(data, 1, *size, fp) < *size) {
        if (ferror(fp) == 0) return NULL;
      }
      data[*size] = '\0';
    }
    fclose(fp);
  }
  return data;
}

static const char *test_parser_large_ast(void) {
  struct ast a;
  struct v7 *v7 = v7_create();
  size_t script_len;
  char *script = read_file("large_ast.js", &script_len);

  ast_init(&a, 0);
  ASSERT_EQ(parse(v7, &a, script, 0), V7_AST_TOO_LARGE);
  return NULL;
}

static const char *test_ecmac(void) {
  struct ast a;
  int i, passed = 0;
  size_t db_len, driver_len;
  char *db = read_file("ecmac.db", &db_len);
  char *driver = read_file("ecma_driver.js", &driver_len);
  char *next_case = db - 1;
  FILE *r;
  struct v7 *v7;
  val_t res;

#ifdef _WIN32
  fprintf(stderr, "Skipping ecma tests on windows\n");
  return NULL;
#endif

  ASSERT((r = fopen(".ecma_report.txt", "wb")) != NULL);

  ast_init(&a, 0);

  for (i = 0; next_case < db + db_len; i++) {
    char tail_cmd[100];
    char *current_case = next_case + 1;
    char *chap_begin = NULL, *chap_end = NULL;
    int chap_len = 0;
    clock_t start_time = clock(), execution_time;
    ASSERT((next_case = strchr(current_case, '\0')) != NULL);
    if ((chap_begin = strstr(current_case, " * @path ")) != NULL) {
      chap_begin += 9;
      if ((chap_end = strchr(chap_begin, '\r')) != NULL ||
          (chap_end = strchr(chap_begin, '\n')) != NULL) {
        chap_len = chap_end - chap_begin;
      }
    }
    snprintf(tail_cmd, sizeof(tail_cmd),
             "%.*s (tail -c +%lu tests/ecmac.db|head -c %lu)", chap_len,
             chap_begin == NULL ? "" : chap_begin,
             (unsigned long) (current_case - db + 1),
             (unsigned long) (next_case - current_case));

    if (i == 1231 || i == 1250 || i == 1252 || i == 1253 || i == 1251 ||
        i == 1255 || i == 2649 || i == 2068 || i == 7445 || i == 7446 ||
        i == 3400 || i == 3348 || i == 3349 || i == 3401 || i == 89 ||
        i == 462 ||

        /*
         * TODO(lsm): re-enable these slow tests
         * This list is created by running the unit test with execution time
         * tracing (see printf at the end of the loop block) and piping through
         * this filter:
         * grep '^--' | sort -nk2 | tail -60 | cut -d' ' -f3 | sort | uniq | \
         *    while read i ; do echo "i == $i ||"; done | xargs
         */
        i == 3247 || i == 3287 || i == 3423 || i == 3424 || i == 3425 ||
        i == 3426 || i == 3427 || i == 3451 || i == 3452 || i == 3453 ||
        i == 3454 || i == 3455 || i == 8101 || i == 8315 || i == 8710 ||
        i == 8929) {
      fprintf(r, "%i\tSKIP %s\n", i, tail_cmd);
      continue;
    }

    v7 = v7_create();

#if V7_VERBOSE_ECMA
    printf("-- Parsing %d: \"%s\"\n", i, current_case);
#endif
    ASSERT_EQ(parse_js(v7, current_case, &a), V7_OK);
    ast_free(&a);

    if (v7_exec(v7, &res, driver) != V7_OK) {
      fprintf(stderr, "%s: %s\n", "Cannot load ECMA driver", v7->error_msg);
    } else {
      if (v7_exec(v7, &res, current_case) != V7_OK) {
        char buf[2048], *err_str = v7_to_json(v7, res, buf, sizeof(buf));
        fprintf(r, "%i\tFAIL %s: [%s]\n", i, tail_cmd, err_str);
        if (err_str != buf) {
          free(err_str);
        }
      } else {
        passed++;
        fprintf(r, "%i\tPASS %s\n", i, tail_cmd);
      }
    }
    v7_destroy(v7);
    execution_time = clock() - start_time;
    (void) execution_time;
#if 0
    printf("--> %g %d [%s]\n",
           execution_time / (double) CLOCKS_PER_SEC, i, tail_cmd);
#endif
  }
  printf("ECMA tests coverage: %.2f%% (%d of %d)\n",
         (double) passed / i * 100.0, passed, i);

  free(db);
  free(driver);
  fclose(r);
  rename(".ecma_report.txt", "ecma_report.txt");
  return NULL;
}

static const char *test_string_encoding(void) {
  unsigned char buf[10] = ":-)";
  int llen;

  ASSERT_EQ(encode_varint(3, buf), 1);
  ASSERT_EQ(decode_varint(buf, &llen), 3);
  ASSERT_EQ(buf[0], 3);
  ASSERT_EQ(llen, 1);

  ASSERT_EQ(encode_varint(127, buf), 1);
  ASSERT_EQ(decode_varint(buf, &llen), 127);
  ASSERT_EQ(buf[0], 127);
  ASSERT_EQ(llen, 1);

  ASSERT_EQ(encode_varint(128, buf), 2);
  ASSERT_EQ(decode_varint(buf, &llen), 128);
  ASSERT_EQ(buf[0], 128);
  ASSERT_EQ(buf[1], 1);
  ASSERT_EQ(llen, 2);

  ASSERT_EQ(encode_varint(0x4000, buf), 3);
  ASSERT_EQ(decode_varint(buf, &llen), 0x4000);
  ASSERT_EQ(buf[0], 128);
  ASSERT_EQ(buf[1], 128);
  ASSERT_EQ(buf[2], 1);
  ASSERT_EQ(llen, 3);

  return NULL;
}

static const char *test_interpreter(void) {
  struct v7 *v7 = v7_create();
  val_t v;
  const char *s, *c, *c0;

  v7_set_property(v7, v7->global_object, "x", -1, 0, v7_create_number(42.0));

  ASSERT_EVAL_EQ(v7, "1%2/2", "0.5");

  ASSERT_EVAL_EQ(v7, "1+x", "43");
  ASSERT_EVAL_EQ(v7, "2-'1'", "1");
  ASSERT_EVAL_EQ(v7, "1+2", "3");
  /*
   * VC6 doesn't like string literals with escaped quotation marks to
   * be passed to macro arguments who stringify the arguments.
   *
   * With this workaround we allow us to build on windows but
   * it will make ASSERT_EVAL_EQ print out `c` instead of the actual
   * expected expression.
   *
   * TODO(mkm): since most of the expressions are constant
   * perhaps we could evaluate them instead of pasting stringifications.
   */
  c = "\"12\"";
  ASSERT_EVAL_EQ(v7, "'1'+'2'", c);
  ASSERT_EVAL_EQ(v7, "'1'+2", c);

  ASSERT_EVAL_EQ(v7, "false+1", "1");
  ASSERT_EVAL_EQ(v7, "true+1", "2");

  ASSERT_EVAL_EQ(v7, "'1'<2", "true");
  ASSERT_EVAL_EQ(v7, "'1'>2", "false");

  ASSERT_EVAL_EQ(v7, "1==1", "true");
  ASSERT_EVAL_EQ(v7, "1==2", "false");
  ASSERT_EVAL_EQ(v7, "'1'==1", "true");
  ASSERT_EVAL_EQ(v7, "'1'!=0", "true");
  ASSERT_EVAL_EQ(v7, "'-1'==-1", "true");
  ASSERT_EVAL_EQ(v7, "a={};a===a", "true");
  ASSERT_EVAL_EQ(v7, "a={};a!==a", "false");
  ASSERT_EVAL_EQ(v7, "a={};a==a", "true");
  ASSERT_EVAL_EQ(v7, "a={};a!=a", "false");
  ASSERT_EVAL_EQ(v7, "a={};b={};a===b", "false");
  ASSERT_EVAL_EQ(v7, "a={};b={};a!==b", "true");
  ASSERT_EVAL_EQ(v7, "a={};b={};a==b", "false");
  ASSERT_EVAL_EQ(v7, "a={};b={};a!=b", "true");
  ASSERT_EVAL_EQ(v7, "1-{}", "NaN");
  ASSERT_EVAL_EQ(v7, "a={};a===(1-{})", "false");
  ASSERT_EVAL_EQ(v7, "a={};a!==(1-{})", "true");
  ASSERT_EVAL_EQ(v7, "a={};a==(1-{})", "false");
  ASSERT_EVAL_EQ(v7, "a={};a!=(1-{})", "true");
  ASSERT_EVAL_EQ(v7, "a={};a===1", "false");
  ASSERT_EVAL_EQ(v7, "a={};a!==1", "true");
  ASSERT_EVAL_EQ(v7, "a={};a==1", "false");
  ASSERT_EVAL_EQ(v7, "a={};a!=1", "true");

  ASSERT_EVAL_EQ(v7, "'x'=='x'", "true");
  ASSERT_EVAL_EQ(v7, "'x'==='x'", "true");
  ASSERT_EVAL_EQ(v7, "'object'=='object'", "true");
  ASSERT_EVAL_EQ(v7, "'stringlong'=='longstring'", "false");
  ASSERT_EVAL_EQ(v7, "'object'==='object'", "true");
  ASSERT_EVAL_EQ(v7, "'a'<'b'", "true");
  ASSERT_EVAL_EQ(v7, "'b'>'a'", "true");
  ASSERT_EVAL_EQ(v7, "'a'>='a'", "true");
  ASSERT_EVAL_EQ(v7, "'a'<='a'", "true");

  ASSERT_EVAL_EQ(v7, "+'1'", "1");
  ASSERT_EVAL_EQ(v7, "-'-1'", "1");
  ASSERT_EQ(v7_exec(v7, &v, "v=[10+1,20*2,30/3]"), V7_OK);
  ASSERT_EQ(val_type(v7, v), V7_TYPE_ARRAY_OBJECT);
  ASSERT_EQ(v7_array_length(v7, v), 3);
  ASSERT(check_value(v7, v, "[11,40,10]"));
  ASSERT_EVAL_EQ(v7, "v[0]", "11");
  ASSERT_EVAL_EQ(v7, "v[1]", "40");
  ASSERT_EVAL_EQ(v7, "v[2]", "10");

  ASSERT_EQ(v7_exec(v7, &v, "v=[10+1,undefined,30/3]"), V7_OK);
  ASSERT_EQ(v7_array_length(v7, v), 3);
  ASSERT(check_value(v7, v, "[11,undefined,10]"));

  ASSERT_EQ(v7_exec(v7, &v, "v=[10+1,,30/3]"), V7_OK);
  ASSERT_EQ(v7_array_length(v7, v), 3);
  ASSERT(check_value(v7, v, "[11,,10]"));

  ASSERT_EVAL_EQ(v7, "3,2,1", "1");

  ASSERT_EVAL_EQ(v7, "x=1", "1");

  ASSERT_EVAL_EQ(v7, "1+2; 1", "1");
  ASSERT_EVAL_EQ(v7, "x=42; x", "42");
  ASSERT_EVAL_EQ(v7, "x=y=42; x+y", "84");

  ASSERT_EQ(v7_exec(v7, &v, "o={a: 1, b: 2}"), V7_OK);
  ASSERT_EVAL_EQ(v7, "o['a'] + o['b']", "3");

  ASSERT_EVAL_EQ(v7, "o.a + o.b", "3");

  ASSERT_EVAL_EQ(v7, "Array(1,2)", "[1,2]");
  ASSERT_EVAL_EQ(v7, "new Array(1,2)", "[1,2]");
  ASSERT_EVAL_OK(v7,
                 "Object.isPrototypeOf(Array(1,2), Object.getPrototypeOf([]))");
  ASSERT_EVAL_EQ(v7, "a=[];r=a.push(1,2,3);[r,a]", "[3,[1,2,3]]");

  ASSERT_EVAL_EQ(v7, "x=1;if(x>0){x=2};x", "2");
  ASSERT_EVAL_EQ(v7, "x=1;if(x<0){x=2};x", "1");
  ASSERT_EVAL_EQ(v7, "x=0;if(true)x=2;else x=3;x", "2");
  ASSERT_EVAL_EQ(v7, "x=0;if(false)x=2;else x=3;x", "3");

  ASSERT_EVAL_EQ(v7, "y=1;x=5;while(x > 0){y=y*x;x=x-1};y", "120");
  ASSERT_EVAL_EQ(v7, "y=1;x=5;do{y=y*x;x=x-1}while(x>0);y", "120");
  ASSERT_EVAL_EQ(v7, "for(y=1,i=1;i<=5;i=i+1)y=y*i;y", "120");
  ASSERT_EVAL_EQ(v7, "for(i=0;1;i++)if(i==5)break;i", "5");
  ASSERT_EVAL_EQ(v7, "for(i=0;1;i++)if(i==5)break;i", "5");
  ASSERT_EVAL_EQ(v7, "i=0;while(++i)if(i==5)break;i", "5");
  ASSERT_EVAL_EQ(v7, "i=0;do{if(i==5)break}while(++i);i", "5");
  ASSERT_EVAL_EQ(v7, "(function(){i=0;do{if(i==5)break}while(++i);i+=10})();i",
                 "15");
  ASSERT_EVAL_EQ(v7,
                 "(function(){x=i=0;do{if(i==5)break;if(i%2)continue;x++}while("
                 "++i);i+=10})();[i,x]",
                 "[15,3]");
  ASSERT_EVAL_EQ(v7, "(function(){i=0;while(++i){if(i==5)break};i+=10})();i",
                 "15");
  ASSERT_EVAL_EQ(v7,
                 "(function(){x=i=0;while(++i){if(i==5)break;if(i%2)continue;x+"
                 "+};i+=10})();[i,x]",
                 "[15,2]");
  ASSERT_EVAL_EQ(v7, "(function(){for(i=0;1;++i){if(i==5)break};i+=10})();i",
                 "15");
  ASSERT_EVAL_EQ(v7,
                 "(function(){x=0;for(i=0;1;++i){if(i==5)break;if(i%2)continue;"
                 "x++};i+=10})();[i,x]",
                 "[15,3]");
  ASSERT_EVAL_EQ(v7, "a=1,[(function(){function a(){1+2}; return a})(),a]",
                 "[[function a()],1]");
  ASSERT_EVAL_EQ(
      v7, "x=0;(function(){try{ff; x=42}catch(e){x=1};function ff(){}})();x",
      "42");
  ASSERT_EVAL_EQ(v7, "a=1,[(function(){return a; function a(){1+2}})(),a]",
                 "[[function a()],1]");
  ASSERT_EVAL_EQ(v7, "(function(){f=42;function f(){};return f})()", "42");

  ASSERT_EVAL_EQ(v7, "x=0;try{x=1};x", "1");
  ASSERT_EVAL_EQ(v7, "x=0;try{x=1}finally{x=x+1};x", "2");
  ASSERT_EVAL_EQ(v7, "x=0;try{x=1}catch(e){x=100}finally{x=x+1};x", "2");

  ASSERT_EVAL_EQ(v7, "x=0;try{xxx;var xxx;x=42}catch(e){x=1};x", "42");

  ASSERT_EVAL_EQ(v7, "(function(a) {return a})", "[function(a)]");
  ASSERT_EVAL_EQ(v7, "(function() {var x=1,y=2; return x})",
                 "[function(){var x,y}]");
  ASSERT_EVAL_EQ(v7, "(function(a) {var x=1,y=2; return x})",
                 "[function(a){var x,y}]");
  ASSERT_EVAL_EQ(v7, "(function(a,b) {var x=1,y=2; return x; var z})",
                 "[function(a,b){var x,y,z}]");
  ASSERT_EVAL_EQ(v7, "(function(a) {var x=1; for(var y in x){}; var z})",
                 "[function(a){var x,y,z}]");
  ASSERT_EVAL_EQ(v7, "(function(a) {var x=1; for(var y=0;y<x;y++){}; var z})",
                 "[function(a){var x,y,z}]");
  ASSERT_EVAL_EQ(v7, "(function() {var x=(function y(){for(var z;;){}})})",
                 "[function(){var x}]");
  ASSERT_EVAL_EQ(v7, "function square(x){return x*x;};square",
                 "[function square(x)]");
  ASSERT_EVAL_EQ(v7, "0;f=(function(x){return x*x;})", "[function(x)]");

  ASSERT_EVAL_EQ(v7, "f=(function(x){return x*x;}); f(2)", "4");
  ASSERT_EVAL_EQ(v7, "(function(x){x*x;})(2)", "undefined");
  ASSERT_EVAL_EQ(v7, "f=(function(x){return x*x;x});v=f(2);v*2", "8");
  ASSERT_EVAL_EQ(v7, "(function(x,y){return x+y;})(40,2)", "42");
  ASSERT_EVAL_EQ(v7, "(function(x,y){if(x==40)return x+y})(40,2)", "42");
  ASSERT_EVAL_EQ(v7, "(function(x,y){return x+y})(40)", "NaN");
  ASSERT_EVAL_EQ(v7, "(function(x){return x+y; var y})(40)", "NaN");
  ASSERT_EVAL_EQ(v7, "x=1;(function(a){return a})(40,(function(){x=x+1})())+x",
                 "42");
  ASSERT_EVAL_EQ(v7, "(function(){x=42;return;x=0})();x", "42");
  ASSERT_EVAL_EQ(v7, "(function(){for(i=0;1;i++)if(i==5)return i})()", "5");
  ASSERT_EVAL_EQ(v7, "(function(){i=0;while(++i)if(i==5)return i})()", "5");
  ASSERT_EVAL_EQ(v7, "(function(){i=0;do{if(i==5)return i}while(++i)})()", "5");

  ASSERT_EQ(
      v7_exec(v7, &v,
              "(function(x,y){return x+y})(40,2,(function(){return fail})())"),
      V7_EXEC_EXCEPTION);

  ASSERT_EVAL_EQ(v7, "x=42; (function(){return x})()", "42");
  ASSERT_EVAL_EQ(v7, "x=2; (function(x){return x})(40)+x", "42");
  ASSERT_EVAL_EQ(v7, "x=1; (function(y){x=x+1; return y})(40)+x", "42");
  ASSERT_EVAL_EQ(
      v7, "x=0;f=function(){x=42; return function() {return x}; var x};f()()",
      "42");
  ASSERT_EVAL_EQ(v7, "x=42;o={x:66,f:function(){return this}};o.f().x", "66");
  ASSERT_EVAL_EQ(v7, "x=42;o={x:66,f:function(){return this}};(1,o.f)().x",
                 "42");
  ASSERT_EVAL_EQ(v7, "x=66;o={x:42,f:function(){return this.x}};o.f()", "42");

  ASSERT_EVAL_EQ(v7, "o={};o.x=24", "24");
  ASSERT_EVAL_EQ(v7, "o.a={};o.a.b={c:66};o.a.b.c", "66");
  ASSERT_EVAL_EQ(v7, "o['a']['b'].c", "66");
  ASSERT_EVAL_EQ(v7, "o={a:1}; o['a']=2;o.a", "2");
  ASSERT_EVAL_EQ(v7, "a={f:function(){return {b:55}}};a.f().b", "55");
  ASSERT_EVAL_EQ(v7, "(function(){fox=1})();fox", "1");

  ASSERT_EVAL_EQ(
      v7,
      "fin=0;(function(){while(1){try{xxxx}finally{fin=1;return 1}}})();fin",
      "1");
  ASSERT_EVAL_EQ(v7,
                 "ca=0;fin=0;(function(){try{(function(){try{xxxx}finally{fin="
                 "1}})()}catch(e){ca=1}})();fin+ca",
                 "2");
  ASSERT_EVAL_EQ(v7, "x=0;try{throw 1}catch(e){x=42};x", "42");

  ASSERT_EVAL_EQ(v7, "x=1;x=x<<3;x", "8");
  ASSERT_EVAL_EQ(v7, "x=1;x<<=4;x", "16");
  ASSERT_EVAL_EQ(v7, "x=1;x++", "1");
  ASSERT_EVAL_EQ(v7, "x", "2");
  ASSERT_EVAL_EQ(v7, "x=1;++x", "2");
  ASSERT_EVAL_EQ(v7, "x", "2");
  ASSERT_EVAL_EQ(v7, "o={x:1};o.x++", "1");
  ASSERT_EVAL_EQ(v7, "o.x", "2");

  c = "\"undefined\"";
  ASSERT_EVAL_EQ(v7, "typeof dummyx", c);
  c = "\"object\"";
  ASSERT_EVAL_EQ(v7, "typeof null", c);
  c = "\"number\"";
  ASSERT_EVAL_EQ(v7, "typeof 1", c);
  ASSERT_EVAL_EQ(v7, "typeof (1+2)", c);
  c = "\"string\"";
  ASSERT_EVAL_EQ(v7, "typeof 'test'", c);
  c = "\"object\"";
  ASSERT_EVAL_EQ(v7, "typeof [1,2]", c);
  c = "\"function\"";
  ASSERT_EVAL_EQ(v7, "typeof function(){}", c);

  ASSERT_EVAL_EQ(v7, "void(1+2)", "undefined");
  ASSERT_EVAL_EQ(v7, "true?1:2", "1");
  ASSERT_EVAL_EQ(v7, "false?1:2", "2");
  ASSERT_EVAL_EQ(v7, "'a' in {a:1}", "true");
  ASSERT_EVAL_EQ(v7, "'b' in {a:1}", "false");
  ASSERT_EVAL_EQ(v7, "1 in [10,20]", "true");
  ASSERT_EVAL_EQ(v7, "20 in [10,20]", "false");

  c = "\"undefined\"";
  ASSERT_EVAL_EQ(v7, "x=1; delete x; typeof x", c);
  ASSERT_EVAL_EQ(v7, "x=1; (function(){x=2;delete x; return typeof x})()", c);
  ASSERT_EVAL_EQ(v7, "x=1; (function(){x=2;delete x})(); typeof x", c);
  ASSERT_EVAL_EQ(v7, "x=1; (function(){var x=2;delete x})(); x", "1");
  ASSERT_EVAL_EQ(v7, "o={a:1};delete o.a;o", "{}");
  ASSERT_EVAL_EQ(v7, "o={a:1};delete o['a'];o", "{}");
  ASSERT_EVAL_EQ(v7, "x=0;if(delete 1 == true)x=42;x", "42");

  c = "[{\"a\":[Circular]}]";
  ASSERT_EVAL_EQ(v7, "o={};a=[o];o.a=a;a", c);

  ASSERT_EVAL_EQ(v7, "new TypeError instanceof Error", "true");
  ASSERT_EVAL_EQ(v7, "new TypeError instanceof TypeError", "true");
  ASSERT_EVAL_EQ(v7, "new Error instanceof Object", "true");
  ASSERT_EVAL_EQ(v7, "new Error instanceof TypeError", "false");
  /* TODO(mkm): fix parser: should not require parenthesis */
  ASSERT_EVAL_EQ(v7, "({}) instanceof Object", "true");

  ASSERT_EQ(v7_exec(v7, &v, ""), V7_OK);
  ASSERT(v7_is_undefined(v));
#if 0
  ASSERT_EVAL_EQ(v7, "x=0;a=1;o={a:2};with(o){x=a};x", "2");
#endif

  ASSERT_EVAL_EQ(
      v7,
      "(function(){try {throw new Error}catch(e){c=e}})();c instanceof Error",
      "true");
  c = "\"undefined\"";
  ASSERT_EVAL_EQ(
      v7, "delete e;(function(){try {throw new Error}catch(e){}})();typeof e",
      c);
  ASSERT_EVAL_EQ(
      v7, "x=(function(){c=1;try {throw 1}catch(e){c=0};return c})()", "0");
  ASSERT_EVAL_EQ(
      v7, "x=(function(){var c=1;try {throw 1}catch(e){c=0};return c})()", "0");
  ASSERT_EVAL_EQ(
      v7, "c=1;x=(function(){try {throw 1}catch(e){var c=0};return c})();[c,x]",
      "[1,0]");
  ASSERT_EVAL_EQ(
      v7, "c=1;x=(function(){try {throw 1}catch(e){c=0};return c})();[c,x]",
      "[0,0]");

  ASSERT_EVAL_EQ(v7, "Object.keys(new Boolean(1))", "[]");
  c = "[\"d\"]";
  ASSERT_EVAL_EQ(v7, "b={c:1};a=Object.create(b); a.d=4;Object.keys(a)", c);
  ASSERT_EVAL_EQ(v7, "Object.getOwnPropertyNames(new Boolean(1))", "[]");
  c = "[\"d\"]";
  ASSERT_EVAL_EQ(
      v7, "b={c:1};a=Object.create(b); a.d=4;Object.getOwnPropertyNames(a)", c);
  c = "o={};Object.defineProperty(o, \"x\", {value:2});[o.x,o]";
  ASSERT_EVAL_EQ(v7, c, "[2,{}]");
  c = "[2,3,{\"y\":3}]";
  ASSERT_EVAL_EQ(v7,
                 "o={};Object.defineProperties(o,{x:{value:2},y:{value:3,"
                 "enumerable:true}});[o.x,o.y,o]",
                 c);
  c0 =
      "o={};Object.defineProperty(o, \"x\", {value:2,enumerable:true});[o.x,o]";
  c = "[2,{\"x\":2}]";
  ASSERT_EVAL_EQ(v7, c0, c);
  ASSERT_EVAL_EQ(
      v7,
      "o={};Object.defineProperty(o,'a',{value:1});o.propertyIsEnumerable('a')",
      "false");
  ASSERT_EVAL_EQ(v7,
                 "o={};Object.defineProperty(o,'a',{value:1,enumerable:true});"
                 "o.propertyIsEnumerable('a')",
                 "true");
  ASSERT_EVAL_EQ(v7, "o={a:1};o.propertyIsEnumerable('a')", "true");
  ASSERT_EVAL_EQ(v7, "b={a:1};o=Object.create(b);o.propertyIsEnumerable('a')",
                 "false");
  ASSERT_EVAL_EQ(v7, "b={a:1};o=Object.create(b);o.hasOwnProperty('a')",
                 "false");
  ASSERT_EVAL_EQ(v7, "o={a:1};o.hasOwnProperty('a')", "true");
  ASSERT_EVAL_EQ(v7,
                 "o={a:1};d=Object.getOwnPropertyDescriptor(o, 'a');"
                 "[d.value,d.writable,d.enumerable,d.configurable]",
                 "[1,true,true,true]");
  ASSERT_EVAL_EQ(v7,
                 "o={};Object.defineProperty(o,'a',{value:1,enumerable:true});"
                 "d=Object.getOwnPropertyDescriptor(o, 'a');"
                 "[d.value,d.writable,d.enumerable,d.configurable]",
                 "[1,false,true,false]");
  ASSERT_EVAL_EQ(v7,
                 "o=Object.defineProperty({},'a',{value:1,enumerable:true});o."
                 "a=2;o.a",
                 "1");
  ASSERT_EVAL_EQ(v7,
                 "o=Object.defineProperty({},'a',{value:1,enumerable:true});r="
                 "delete o.a;[r,o.a]",
                 "[false,1]");

  ASSERT_EVAL_EQ(v7, "r=0;o={a:1,b:2};for(i in o){r+=o[i]};r", "3");
  ASSERT_EVAL_EQ(v7, "r=0;o={a:1,b:2};for(var i in o){r+=o[i]};r", "3");
  ASSERT_EVAL_EQ(v7, "r=1;for(var i in null){r=0};r", "1");
  ASSERT_EVAL_EQ(v7, "r=1;for(var i in undefined){r=0};r", "1");
  ASSERT_EVAL_EQ(v7, "r=1;for(var i in 42){r=0};r", "1");

  ASSERT_EQ(v7_exec_with(v7, &v, "this", v7_create_number(42)), V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec_with(v7, &v, "a=666;(function(a){return a})(this)",
                      v7_create_number(42)) == V7_OK);
  ASSERT(check_value(v7, v, "42"));

  c = "\"aa bb\"";
  ASSERT_EVAL_EQ(v7, "a='aa', b='bb';(function(){return a + ' ' + b;})()", c);

  s = "{\"fall\":2,\"one\":1}";
  ASSERT_EVAL_EQ(v7,
                 "o={};switch(1) {case 1: o.one=1; case 2: o.fall=2; break; "
                 "case 3: o.three=1; };o",
                 s);
  ASSERT_EVAL_EQ(v7,
                 "o={};for(i=0;i<1;i++) switch(1) {case 1: o.one=1; case 2: "
                 "o.fall=2; continue; case 3: o.three=1; };o",
                 s);
  ASSERT_EVAL_EQ(v7,
                 "(function(){o={};switch(1) {case 1: o.one=1; case 2: "
                 "o.fall=2; return o; case 3: o.three=1; }})()",
                 s);
  ASSERT_EVAL_EQ(v7,
                 "o={};switch(1) {case 1: o.one=1; default: o.fall=2; break; "
                 "case 3: o.three=1; };o",
                 s);
  c = "{\"def\":1}";
  ASSERT_EVAL_EQ(v7,
                 "o={};switch(10) {case 1: o.one=1; case 2: o.fall=2; break; "
                 "case 3: o.three=1; break; default: o.def=1};o",
                 c);

  ASSERT_EVAL_EQ(v7, "o={get x(){return 42}};o.x", "42");
  ASSERT_EVAL_EQ(v7, "o={set x(a){this.y=a}};o.x=42;o.y", "42");
  ASSERT_EVAL_EQ(v7, "o={get x(){return 10},set x(v){}};o.x", "10");
  ASSERT_EVAL_EQ(v7, "o={set x(v){},get x(){return 10}};o.x", "10");
  ASSERT_EVAL_EQ(v7, "r=0;o={get x() {return 10}, set x(v){r=v}};o.x=10;r",
                 "10");
  ASSERT_EVAL_EQ(v7,
                 "g=0;function O() {}; O.prototype = {set x(v) {g=v}};o=new "
                 "O;o.x=42;[g,Object.keys(o)]",
                 "[42,[]]");

  c = "\"42\"";
  ASSERT_EVAL_EQ(v7, "String(new Number(42))", c);

  ASSERT_EVAL_EQ(
      v7, "L: for(i=0;i<10;i++){for(j=4;j<10;j++){if(i==j) break L}};i", "4");
  ASSERT_EVAL_EQ(
      v7, "L: for(i=0;i<10;i++){M:for(j=4;j<10;j++){if(i==j) break L}};i", "4");
  ASSERT_EVAL_EQ(v7,
                 "x=0;L: for(i=0;i<10;i++){try{for(j=4;j<10;j++){if(i==j) "
                 "break L}}finally{x++}};x",
                 "5");
  ASSERT_EVAL_EQ(v7, "x=0;L: for(i=0;i<11;i++) {if(i==5) continue L; x+=i}; x",
                 "50");
  ASSERT_EVAL_EQ(
      v7, "x=0;L: if(true) for(i=0;i<11;i++) {if(i==5) continue L; x+=i}; x",
      "50");
  ASSERT_EVAL_EQ(
      v7, "x=0;L: if(true) for(i=0;i<11;i++) {if(i==5) continue L; x+=i}; x",
      "50");
  ASSERT_EVAL_EQ(v7, "L:do {i=0;continue L;}while(i>0);i", "0");
  ASSERT_EVAL_EQ(v7, "i=1; L:while(i>0){i=0;continue L;};i", "0");

  ASSERT_EVAL_EQ(v7, "1 | NaN", "1");
  ASSERT_EVAL_EQ(v7, "NaN | 1", "1");
  ASSERT_EVAL_EQ(v7, "NaN | NaN", "0");

  ASSERT_EVAL_EQ(v7, "0 || 1", "1");
  ASSERT_EVAL_EQ(v7, "0 || {}", "{}");
  ASSERT_EVAL_EQ(v7, "1 && 0", "0");
  ASSERT_EVAL_EQ(v7, "1 && {}", "{}");
  c = "\"\"";
  ASSERT_EVAL_EQ(v7, "'' && {}", c);

  ASSERT_EQ(v7_exec_with(v7, &v, "a=this;a", v7_create_foreign((void *) "foo")),
            V7_OK);
  ASSERT(v7_is_foreign(v));
  ASSERT_EQ(strcmp((char *) v7_to_foreign(v), "foo"), 0);

  ASSERT_EVAL_EQ(v7, "a=[1,2,3];a.splice(0,1);a", "[2,3]");
  ASSERT_EVAL_EQ(v7, "a=[1,2,3];a.splice(2,1);a", "[1,2]");
  ASSERT_EVAL_EQ(v7, "a=[1,2,3];a.splice(1,1);a", "[1,3]");

  ASSERT_EVAL_EQ(v7, "a=[1,2,3];a.slice(0,1)", "[1]");
  ASSERT_EVAL_EQ(v7, "a=[1,2,3];a.slice(2,3)", "[3]");
  ASSERT_EVAL_EQ(v7, "a=[1,2,3];a.slice(1,3)", "[2,3]");

  c = "[\"b\",\"a\"]";
  ASSERT_EVAL_EQ(v7,
                 "function foo(){}; foo.prototype={a:1}; f=new foo; f.b=2; "
                 "r=[];for(p in f) r.push(p); r",
                 c);

  /* here temporarily because test_stdlib has memory violations */
  ASSERT_EVAL_EQ(v7, "a=[2,1];a.sort();a", "[1,2]");

  /* check execution failure caused by bad parsing */
  ASSERT_EQ(v7_exec(v7, &v, "function"), V7_SYNTAX_ERROR);

  v7_destroy(v7);
  return NULL;
} /* test_interpreter */

static const char *test_strings(void) {
  val_t s;
  struct v7 *v7;
  size_t off;

  v7 = v7_create();
  off = v7->owned_strings.len;
  ASSERT(off > 0); /* properties names use it */

  s = v7_create_string(v7, "hi", 2, 1);
  ASSERT_EQ(memcmp(&s, "\x02\x68\x69\x00\x00\x00\xfa\xff", sizeof(s)), 0);
  ASSERT_EQ(v7->foreign_strings.len, 0);
  ASSERT_EQ(v7->owned_strings.len, off);

  /* Make sure strings with length 5 & 6 are nan-packed */
  s = v7_create_string(v7, "length", 4, 1);
  ASSERT_EQ(v7->owned_strings.len, off);
  s = v7_create_string(v7, "length", 5, 1);
  ASSERT_EQ(v7->owned_strings.len, off);
  ASSERT_EQ(memcmp(&s, "\x6c\x65\x6e\x67\x74\x00\xf9\xff", sizeof(s)), 0);

  s = v7_create_string(v7, "longer one", ~0 /* use strlen */, 1);
  ASSERT(v7->owned_strings.len == off + 12);
  ASSERT_EQ(memcmp(v7->owned_strings.buf + off, "\x0alonger one\x00", 12), 0);

  s = v7_create_string(v7, "with embedded \x00 one", 19, 1);

  ASSERT(v7->owned_strings.len == off + 33);
  ASSERT(memcmp(v7->owned_strings.buf + off,
                "\x0alonger one\x00"
                "\x13with embedded \x00 one\x00",
                33) == 0);

  v7_destroy(v7);

  return NULL;
}

static const char *test_interp_unescape(void) {
  struct v7 *v7 = v7_create();
  val_t v;

  ASSERT_EQ(v7_exec(v7, &v, "'1234'"), V7_OK);
  ASSERT_EQ((v & V7_TAG_MASK), V7_TAG_STRING_I);
  ASSERT_EQ(v7_exec(v7, &v, "'12345'"), V7_OK);
  ASSERT_EQ((v & V7_TAG_MASK), V7_TAG_STRING_5);
  ASSERT_EQ(v7_exec(v7, &v, "'123456'"), V7_OK);
  ASSERT_EQ((v & V7_TAG_MASK), V7_TAG_STRING_O);

  ASSERT_EVAL_NUM_EQ(v7, "'123'.length", 3);
  ASSERT_EVAL_NUM_EQ(v7, "'123\\n'.length", 4);
  ASSERT_EVAL_NUM_EQ(v7, "'123\\n\\n'.length", 5);
  ASSERT_EVAL_NUM_EQ(v7, "'123\\n\\n\\n'.length", 6);
  ASSERT_EVAL_NUM_EQ(v7, "'123\\n\\n\\n\\n'.length", 7);
  ASSERT_EVAL_NUM_EQ(v7, "'123\\n\\n\\n\\n\\n'.length", 8);

  ASSERT_EVAL_EQ(v7, "'123\\\\\\\\'.length == '1234\\\\\\\\'.length", "false");

  ASSERT_EVAL_NUM_EQ(v7, "'123'.length", 3);
  ASSERT_EVAL_NUM_EQ(v7, "'123\\\\'.length", 4);
  ASSERT_EVAL_NUM_EQ(v7, "'123\\\\\\\\'.length", 5);
  ASSERT_EVAL_NUM_EQ(v7, "'123\\\\\\\\\\\\'.length", 6);
  ASSERT_EVAL_NUM_EQ(v7, "'123\\\\\\\\\\\\\\\\'.length", 7);
  ASSERT_EVAL_NUM_EQ(v7, "'123\\\\\\\\\\\\\\\\\\\\'.length", 8);

  ASSERT_EQ(v7_exec(v7, &v, "'1234\\\\\\\\'"), V7_OK);
  ASSERT_EQ((v & V7_TAG_MASK), V7_TAG_STRING_O);

  v7_destroy(v7);
  return NULL;
}

static const char *test_to_json(void) {
  char buf[10], *p;
  const char *c;
  struct v7 *v7 = v7_create();
  val_t v;

  v7_exec(v7, &v, "123.45");
  ASSERT((p = v7_to_json(v7, v, buf, sizeof(buf))) == buf);
  ASSERT_STREQ(p, "123.45");

  v7_exec(v7, &v, "'foo'");
  ASSERT((p = v7_to_json(v7, v, buf, sizeof(buf))) == buf);
  c = "\"foo\"";
  ASSERT_STREQ(p, c);

  v7_exec(v7, &v, "'\"foo\"'");
  ASSERT((p = v7_to_json(v7, v, buf, sizeof(buf))) == buf);
  c = "\"\\\"foo\\\"\"";
  ASSERT_STREQ(p, c);

/* TODO(mkm): fix to_json alloc */
#if 0
  ASSERT((p = v7_to_json(v7, v, buf, 3)) != buf);
  ASSERT_EQ(strcmp(p, "123.45"), 0);
  free(p);
#endif

  v7_destroy(v7);
  return NULL;
}

static const char *test_json_parse(void) {
  struct v7 *v7 = v7_create();
  const char *c1, *c2;

  ASSERT_EVAL_NUM_EQ(v7, "JSON.parse(42)", 42);
  c1 = "JSON.parse('\"foo\"')";
  c2 = "\"foo\"";
  ASSERT_EVAL_EQ(v7, c1, c2);
  c2 = "\"foo\"";
  ASSERT_EVAL_EQ(v7, "JSON.parse(JSON.stringify('foo'))", c2);
  c2 = "{\"foo\":\"bar\"}";
  ASSERT_EVAL_EQ(v7, "JSON.parse(JSON.stringify({'foo':'bar'}))", c2);

  c1 =
      "JSON.parse(JSON.stringify('"
      "foooooooooooooooooooooooooooooooooooooooooooooooo"
      "ooooooooooooooooooooooooooooooooooooooooooooooooo'))",

  c2 =
      "\"foooooooooooooooooooooooooooooooooooooooooooooooo"
      "ooooooooooooooooooooooooooooooooooooooooooooooooo\"";

  /* big string, will cause malloc */
  ASSERT_EVAL_EQ(v7, c1, c2);

  v7_destroy(v7);
  return NULL;
}

static const char *test_unescape(void) {
  char buf[100];
  ASSERT_EQ(unescape("\\n", 2, buf), 1);
  ASSERT(buf[0] == '\n');
  ASSERT_EQ(unescape("\\u0061", 6, buf), 1);
  ASSERT(buf[0] == 'a');
  ASSERT_EQ(unescape("гы", 4, buf), 4);
  ASSERT_EQ(memcmp(buf, "\xd0\xb3\xd1\x8b", 4), 0);
  ASSERT_EQ(unescape("\\\"", 2, buf), 1);
  ASSERT_EQ(memcmp(buf, "\"", 1), 0);
  ASSERT_EQ(unescape("\\'", 2, buf), 1);
  ASSERT_EQ(memcmp(buf, "'", 1), 0);
  ASSERT_EQ(unescape("\\\n", 2, buf), 1);
  ASSERT_EQ(memcmp(buf, "\n", 1), 0);
  return NULL;
}

#ifndef V7_DISABLE_GC
static const char *test_gc_mark(void) {
  struct v7 *v7 = v7_create();
  val_t v;

  v7_exec(v7, &v, "o=({a:{b:1},c:{d:2},e:null});o.e=o;o");
  gc_mark(v7, v);
  ASSERT(MARKED(v7_to_object(v)));
  v7_gc(v7, 0); /* cleanup marks */
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v, "o=({a:{b:1},c:{d:2},e:null});o.e=o;o");
  gc_mark(v7, v7->global_object);
  ASSERT(MARKED(v7_to_object(v)));
  v7_gc(v7, 0); /* cleanup marks */
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v, "function f() {}; o=new f;o");
  gc_mark(v7, v);
  ASSERT(MARKED(v7_to_object(v)));
  v7_gc(v7, 0); /* cleanup marks */
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v, "function f() {}; Object.getPrototypeOf(new f)");
  gc_mark(v7, v7->global_object);
  ASSERT(MARKED(v7_to_object(v)));
  v7_gc(v7, 0); /* cleanup marks */
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v, "({a:1})");
  gc_mark(v7, v7->global_object);
  ASSERT(!MARKED(v7_to_object(v)));
  v7_gc(v7, 0); /* cleanup marks */
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v,
          "var f;(function() {var x={a:1};f=function(){return x};return x})()");
  gc_mark(v7, v7->global_object);
  /* `x` is reachable through `f`'s closure scope */
  ASSERT(MARKED(v7_to_object(v)));
  v7_gc(v7, 0); /* cleanup marks */
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v,
          "(function() {var x={a:1};var f=function(){return x};return x})()");
  gc_mark(v7, v7->global_object);
  /* `f` is unreachable, hence `x` is not marked through the scope */
  ASSERT(!MARKED(v7_to_object(v)));
  v7_gc(v7, 0); /* cleanup marks */
  v7_destroy(v7);

  return NULL;
}

static const char *test_gc_sweep(void) {
  struct v7 *v7 = v7_create();
  val_t v;
  uint32_t alive;

  v7_gc(v7, 0);
  alive = v7->object_arena.alive;
  v7_exec(v7, &v, "x=({a:1})");
  v7_to_object(v);
  v7_gc(v7, 0);
  ASSERT(v7->object_arena.alive > alive);
  ASSERT_EVAL_EQ(v7, "x.a", "1");

  ASSERT_EVAL_OK(v7, "x=null");
  v7_gc(v7, 0);
  ASSERT_EQ(v7->object_arena.alive, alive);
  v7_destroy(v7);

  v7 = v7_create();
  v7_gc(v7, 0);
  ASSERT_EVAL_EQ(
      v7,
      "for(i=0;i<9;i++)({});for(i=0;i<7;i++){x=(new Number(1))+({} && 1)};x",
      "2");
  v7_gc(v7, 0);

  v7_destroy(v7);
  return NULL;
}

static const char *test_gc_own(void) {
  struct v7 *v7 = v7_create();
  val_t v1, v2;
  const char *s;
  size_t len;

  v1 = v7_create_string(v7, "foobar", 6, 1);
  v2 = v7_create_string(v7, "barfoo", 6, 1);

  v7_own(v7, &v1);
  v7_own(v7, &v2);

  /*
   * fully gc will shrink the mbuf. given that v2 is the last entry
   * if it were not correctly rooted, it will now lie outside realloced
   * area and ASAN will complain.
   */
  v7_gc(v7, 1);
  s = v7_to_string(v7, &v2, &len);
  ASSERT_STREQ(s, "barfoo");

  ASSERT_EQ(v7_disown(v7, &v2), 1);

  v7_gc(v7, 1);
  s = v7_to_string(v7, &v1, &len);
  ASSERT_STREQ(s, "foobar");

  ASSERT_EQ(v7_disown(v7, &v2), 0);

  v7_destroy(v7);
  return NULL;
}
#endif

#ifdef V7_ENABLE_FILE
static int check_file(struct v7 *v7, v7_val_t s, const char *file_name) {
  size_t n1, n2;
  char *s1 = read_file(file_name, &n1);
  const char *s2 = v7_to_string(v7, &s, &n2);
  int result = n1 == n2 && memcmp(s1, s2, n1) == 0;
  free(s1);
  if (result == 0) {
    printf("want '%.*s' (len %d), got '%.*s' (len %d)\n", (int) n2, s2,
           (int) n2, (int) n1, s1, (int) n1);
  }
  return result;
}

static const char *test_file(void) {
  const char *data = "some test string", *test_file_name = "ft.txt";
  struct v7 *v7 = v7_create();
  v7_val_t v, data_str = v7_create_string(v7, data, strlen(data), 1);

  v7_own(v7, &data_str);
  v7_set(v7, v7_get_global_object(v7), "ts", 2, 0, data_str);
  ASSERT(v7_exec(v7, &v,
                 "f = File.open('ft.txt', 'w+'); "
                 " f.write(ts); f.close();") == V7_OK);
  ASSERT(check_file(v7, data_str, test_file_name));
  ASSERT_EQ(remove(test_file_name), 0);
  ASSERT_EQ(v7_exec(v7, &v, "f = File.open('test.mk'); f.readAll()"), V7_OK);
  ASSERT(check_file(v7, v, "test.mk"));
  ASSERT_EQ(v7_exec(v7, &v, "l = File.list('non existent directory')"), V7_OK);
  ASSERT(v7_is_undefined(v));
  ASSERT_EQ(v7_exec(v7, &v, "l = File.list('.');"), V7_OK);
  ASSERT(v7_is_array(v7, v));
  ASSERT_EVAL_EQ(v7, "l.indexOf('unit_test.c') >= 0", "true");

  v7_disown(v7, &data_str);
  v7_destroy(v7);
  return NULL;
}
#endif

#ifdef V7_ENABLE_SOCKET
static const char *test_socket(void) {
  struct v7 *v7 = v7_create();
  const char *c;

  ASSERT_EVAL_OK(v7, "s1 = Socket.listen(1239, '127.0.0.1'); ");
  ASSERT_EVAL_OK(v7, "s2 = Socket.connect('127.0.0.1', 1239); ");
  ASSERT_EVAL_OK(v7, "s2.send('hi'); ");
  c = "\"hi\"";
  ASSERT_EVAL_EQ(v7, "s3 = s1.accept(); s3.recv();", c);
  ASSERT_EVAL_OK(v7, "s1.close(); s2.close(); s3.close();");

  v7_destroy(v7);
  return NULL;
}
#endif

#ifdef V7_ENABLE_CRYPTO
static const char *test_crypto(void) {
  struct v7 *v7 = v7_create();
  const char *c;

  c = "\"\"";
  ASSERT_EVAL_EQ(v7, "Crypto.base64_encode('');", c);
  c = "\"IA==\"";
  ASSERT_EVAL_EQ(v7, "Crypto.base64_encode(' ');", c);
  c = "\"Oi0p\"";
  ASSERT_EVAL_EQ(v7, "Crypto.base64_encode(':-)');", c);
  c = "\"0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33\"";
  ASSERT_EVAL_EQ(v7, "Crypto.sha1_hex('foo');", c);
  c = "\"acbd18db4cc2f85cedef654fccc4a4d8\"";
  ASSERT_EVAL_EQ(v7, "Crypto.md5_hex('foo');", c);

  v7_destroy(v7);
  return NULL;
}
#endif

#define MK_OP_PUSH_LIT(n) OP_PUSH_LIT, (enum opcode)(n)
#define MK_OP_PUSH_VAR_NAME(n) OP_PUSH_VAR_NAME, (enum opcode)(n)
#define MK_OP_GET_VAR(n) OP_GET_VAR, (enum opcode)(n)
#define MK_OP_SET_VAR(n) OP_SET_VAR, (enum opcode)(n)

#ifdef V7_ENABLE_BCODE

static const char *test_bcode(void) {
  struct v7 *v7 = v7_create();
  uint8_t ops[] = {
#if 0 /* for clang-format */
#endif
    MK_OP_GET_VAR(2),
    MK_OP_PUSH_LIT(4),
    MK_OP_PUSH_LIT(0),
    OP_PUSH_ONE,
    OP_ADD,
    MK_OP_PUSH_LIT(0),
    OP_SUB,
    MK_OP_GET_VAR(1),
    OP_ADD,
    MK_OP_GET_VAR(2),
    MK_OP_PUSH_LIT(3),
    OP_GET,
    OP_MUL,
    OP_SET,
    MK_OP_SET_VAR(5), /* assign to non-existing */
    MK_OP_SET_VAR(5), /* assign to existing */
    MK_OP_PUSH_LIT(6),
    OP_EQ
  };
  struct bcode bcode;
  val_t y, y_proto;
  char buf[512];

  v7->call_stack = v7_create_object(v7);
  v7_to_object(v7->call_stack)->prototype =
      v7_to_object(v7_get_global_object(v7));

  v7_set(v7, v7_get_global_object(v7), "x", 1, 0, v7_create_number(666));
  y_proto = v7_create_object(v7);
  y = create_object(v7, y_proto);
  v7_set(v7, y_proto, "b", 1, 0, v7_create_number(10));
  v7_set(v7, y, "a", 1, 0, v7_create_number(20));
  v7_set(v7, v7->call_stack, "y", 1, 0, y);
#if 0
  v7_set(v7, v7_get_global_object(v7), "r", 1, 0, v7_create_undefined());
#endif
  v7_set(v7, v7_get_global_object(v7), "gy", 2, 0, y);
  v7_set(v7, v7_get_global_object(v7), "gyp", 3, 0, y_proto);
  v7_set(v7, v7_get_global_object(v7), "scope", 5, 0, v7->call_stack);

  memset(&bcode, 0, sizeof(bcode));
  bcode.ops = ops;
  bcode.ops_len = ARRAY_SIZE(ops);
  bcode.lit[0] = v7_create_number(42);
  bcode.lit[1] = v7_create_string(v7, "x", 1, 1);
  bcode.lit[2] = v7_create_string(v7, "y", 1, 1);
  bcode.lit[3] = v7_create_string(v7, "a", 1, 1);
  bcode.lit[4] = v7_create_string(v7, "b", 1, 1);
  bcode.lit[5] = v7_create_string(v7, "r", 1, 1);
  bcode.lit[6] = v7_create_number((42 + 1 - 42 + 666) * 20);

  ASSERT_EQ(v7->sp, 0);
  eval_bcode(v7, &bcode);

  v7_stringify_value(v7, v7->stack[v7->sp - 1], buf, sizeof(buf));
  printf("TOS: '%s'\n", buf);

  ASSERT_EQ(v7->sp, 1);

#if 0
  ASSERT(check_num(v7, v7->stack[v7->sp - 1], (42 + 1 - 42 + 666) * 20));
#else
  ASSERT(check_bool(v7->stack[v7->sp - 1], 1));
#endif

  ASSERT_EVAL_EQ(v7, "gy.b", "13340");
  ASSERT_EVAL_EQ(v7, "gyp.b", "10");

  ASSERT_EVAL_EQ(v7, "scope.r", "13340");
  ASSERT_EVAL_EQ(v7, "r", "13340");

  v7_destroy(v7);
  return NULL;
}

#endif /* V7_ENABLE_BCODE */

static const char *run_all_tests(const char *filter, double *total_elapsed) {
  RUN_TEST(test_unescape);
  RUN_TEST(test_to_json);
  RUN_TEST(test_json_parse);
  RUN_TEST(test_tokenizer);
  RUN_TEST(test_string_encoding);
  RUN_TEST(test_is_true);
  RUN_TEST(test_closure);
  RUN_TEST(test_native_functions);
  RUN_TEST(test_stdlib);
  RUN_TEST(test_runtime);
  RUN_TEST(test_apply);
  RUN_TEST(test_parser);
#ifndef V7_LARGE_AST
  RUN_TEST(test_parser_large_ast);
#endif
  RUN_TEST(test_interpreter);
  RUN_TEST(test_interp_unescape);
  RUN_TEST(test_strings);
  RUN_TEST(test_dense_arrays);
#ifdef V7_ENABLE_FILE
  RUN_TEST(test_file);
#endif
#ifdef V7_ENABLE_SOCKET
  RUN_TEST(test_socket);
#endif
#ifdef V7_ENABLE_CRYPTO
  RUN_TEST(test_crypto);
#endif
#ifndef V7_DISABLE_GC
  RUN_TEST(test_gc_mark);
  RUN_TEST(test_gc_sweep);
  RUN_TEST(test_gc_own);
#endif
  RUN_TEST(test_ecmac);
#ifdef V7_ENABLE_BCODE
  RUN_TEST(test_bcode);
#endif
  return NULL;
}

int main(int argc, char *argv[]) {
  const char *filter = argc > 1 ? argv[1] : "";
  double total_elapsed = 0.0;
  const char *fail_msg = run_all_tests(filter, &total_elapsed);
  printf("%s, run %d in %.3fs\n", fail_msg ? "FAIL" : "PASS", num_tests,
         total_elapsed);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
