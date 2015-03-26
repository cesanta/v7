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
 * license, as set out in <http://cesanta.com/products.html>.
 */

#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h> /* for O_RDWR */

#ifndef _WIN32
#include <unistd.h>
#ifndef __WATCOM__
#include <pthread.h>
#endif
#endif

#include "../v7.h"
#include "../src/internal.h"
#include "../src/gc.h"

#ifdef _WIN32
#define isinf(x) (!_finite(x))
#ifndef NAN
#define NAN atof("NAN")
#endif
/* #define INFINITY    BLAH */
#endif

extern long timezone;

#define FAIL(str, line)                           \
  do {                                            \
    printf("Fail on line %d: [%s]\n", line, str); \
    exit(1);                                      \
    return str;                                   \
  } while (0)

#define ASSERT(expr)                    \
  do {                                  \
    static_num_tests++;                 \
    if (!(expr)) FAIL(#expr, __LINE__); \
  } while (0)

#define RUN_TEST(test)                       \
  do {                                       \
    const char *msg = NULL;                  \
    if (strstr(#test, filter)) msg = test(); \
    if (msg) return msg;                     \
  } while (0)

#ifdef _WIN32
#define isnan(x) _isnan(x)
#endif

static int static_num_tests = 0;
int STOP = 0; /* For xcode breakpoints conditions */

static int check_value(struct v7 *v7, val_t v, const char *str) {
  char buf[2048];
  v7_to_json(v7, v, buf, sizeof(buf));
  if (strncmp(buf, str, sizeof(buf)) != 0) {
    printf("want %s got %s\n", str, buf);
    return 0;
  }
  return 1;
}

static int check_num(val_t v, double num) {
  int ret = isnan(num) ? isnan(v7_to_double(v)) : v7_to_double(v) == num;
  if (!ret) {
    printf("Num: want %f got %f\n", num, v7_to_double(v));
  }

  return ret;
}

static int check_num_not(val_t v, double num) {
  int ret = isnan(num) ? isnan(v7_to_double(v)) : v7_to_double(v) != num;
  if (!ret) {
    printf("Num: want %f got %f\n", num, v7_to_double(v));
  }

  return ret;
}

static int check_bool(val_t v, int is_true) {
  int b = v7_to_boolean(v);
  return is_true ? b : !b;
}

static int check_str(struct v7 *v7, val_t v, const char *str) {
  size_t n1, n2 = strlen(str);
  const char *s = v7_to_string(v7, &v, &n1);
  int ret = (n1 == n2 && memcmp(s, str, n1) == 0);
  if (!ret) {
    printf("Str: want %s got %s\n", str, s);
  }
  return ret;
}

static int test_if_expr(struct v7 *v7, const char *expr, int result) {
  val_t v;
  if (v7_exec(v7, &v, expr) != V7_OK) return 0;
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
  val_t v;
  struct v7 *v7 = v7_create();

  ASSERT(v7_exec(v7, &v, "function a(x){return function(y){return x*y}}") ==
         V7_OK);
  ASSERT(v7_exec(v7, &v, "var f1 = a(5);") == V7_OK);
  ASSERT(v7_exec(v7, &v, "var f2 = a(7);") == V7_OK);
  ASSERT(v7_exec(v7, &v, "f1(3);") == V7_OK);
  ASSERT(check_value(v7, v, "15"));
  ASSERT(v7_exec(v7, &v, "f2(3);") == V7_OK);
  ASSERT(check_value(v7, v, "21"));

  v7_destroy(v7);
  return NULL;
}

static val_t adder(struct v7 *v7, val_t this_obj, val_t args) {
  double sum = 0;
  unsigned long i;

  (void) this_obj;
  for (i = 0; i < v7_array_length(v7, args); i++) {
    sum += v7_to_double(v7_array_get(v7, args, i));
  }
  return v7_create_number(sum);
}

static const char *test_native_functions(void) {
  val_t v;
  struct v7 *v7 = v7_create();

  ASSERT(v7_set_property(v7, v7_get_global_object(v7), "adder", 5, 0,
                         v7_create_cfunction(adder)) == 0);
  ASSERT(v7_exec(v7, &v, "adder(1, 2, 3 + 4);") == V7_OK);
  ASSERT(check_value(v7, v, "10"));
  v7_destroy(v7);

  return NULL;
}

static const char *test_stdlib(void) {
  v7_val_t v;
  struct v7 *v7 = v7_create();

  ASSERT(v7_exec(v7, &v, "Boolean()") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "Boolean(0)") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "Boolean(1)") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "Boolean([])") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "new Boolean([])") == V7_OK);
  ASSERT(check_value(v7, v, "{}"));

  /* Math */
  ASSERT(v7_exec(v7, &v, "Math.sqrt(144)") == V7_OK);
  ASSERT(check_value(v7, v, "12"));

  /* Number */
  ASSERT(v7_exec(v7, &v, "Math.PI") == V7_OK);
  ASSERT(check_num(v, M_PI));
  ASSERT(v7_exec(v7, &v, "Number.NaN") == V7_OK);
  ASSERT(check_num(v, NAN));
  ASSERT(v7_exec(v7, &v, "1 == 2") == V7_OK);
  ASSERT(check_bool(v, 0));
  ASSERT(v7_exec(v7, &v, "1 + 2 * 7 === 15") == V7_OK);
  ASSERT(check_bool(v, 1));
  ASSERT(v7_exec(v7, &v, "Number(1.23) === 1.23") == V7_OK);
  ASSERT(check_bool(v, 1));
  ASSERT(v7_exec(v7, &v, "Number(1.23)") == V7_OK);
  ASSERT(check_num(v, 1.23));
  ASSERT(v7_exec(v7, &v, "new Number(21.23)") == V7_OK);

  /* String */
  ASSERT(v7_exec(v7, &v, "'hello'.charCodeAt(1)") == V7_OK);
  ASSERT(check_num(v, 'e'));
  ASSERT(v7_exec(v7, &v, "'hello'.charCodeAt(4)") == V7_OK);
  ASSERT(check_num(v, 'o'));
  ASSERT(v7_exec(v7, &v, "'hello'.charCodeAt(5)") == V7_OK);
  ASSERT(check_num(v, NAN));
  ASSERT(v7_exec(v7, &v, "'hello'.indexOf()") == V7_OK);
  ASSERT(check_num(v, -1.0));
  ASSERT(v7_exec(v7, &v, "'HTTP/1.0\\r\\n'.indexOf('\\r\\n')") == V7_OK);
  ASSERT(check_num(v, 8.0));
  ASSERT(v7_exec(v7, &v, "'hi there'.indexOf('e')") == V7_OK);
  ASSERT(check_num(v, 5.0));
  ASSERT(v7_exec(v7, &v, "'hi there'.indexOf('e', 6)") == V7_OK);
  ASSERT(check_num(v, 7.0));
  ASSERT(v7_exec(v7, &v, "'hi there'.indexOf('e', NaN)") == V7_OK);
  ASSERT(check_num(v, 5.0));
  ASSERT(v7_exec(v7, &v, "'hi there'.indexOf('e', -Infinity)") == V7_OK);
  ASSERT(check_num(v, 5.0));
  ASSERT(v7_exec(v7, &v, "'hi there'.indexOf('e', Infinity)") == V7_OK);
  ASSERT(check_num(v, -1.0));
  ASSERT(v7_exec(v7, &v, "'hi there'.indexOf('e', 8)") == V7_OK);
  ASSERT(check_num(v, -1.0));
  ASSERT(v7_exec(v7, &v, "'aabb'.indexOf('a', false)") == V7_OK);
  ASSERT(check_num(v, 0.0));
  ASSERT(v7_exec(v7, &v, "'aabb'.indexOf('a', true)") == V7_OK);
  ASSERT(check_num(v, 1.0));
  ASSERT(v7_exec(v7, &v, "'hi there'.substr(3, 2)") == V7_OK);
  ASSERT(check_str(v7, v, "th"));
  ASSERT(v7_exec(v7, &v, "'hi there'.substring(3, 5)") == V7_OK);
  ASSERT(check_str(v7, v, "th"));
  ASSERT(v7_exec(v7, &v, "'hi there'.substr(3)") == V7_OK);
  ASSERT(check_str(v7, v, "there"));
  ASSERT(v7_exec(v7, &v, "'hi there'.substr(-2)") == V7_OK);
  ASSERT(check_str(v7, v, "re"));
  ASSERT(v7_exec(v7, &v, "'hi there'.substr(NaN)") == V7_OK);
  ASSERT(check_str(v7, v, "hi there"));
  ASSERT(v7_exec(v7, &v, "'hi there'.substr(0, 300)") == V7_OK);
  ASSERT(check_str(v7, v, "hi there"));
#ifndef V7_DISABLE_REGEX
  ASSERT(v7_exec(v7, &v, "'dew dee'.match(/\\d+/)") == V7_OK);
  ASSERT(v == V7_NULL);
  ASSERT(v7_exec(v7, &v, "m = 'foo 1234 bar'.match(/\\S+ (\\d+)/)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "m.length") == V7_OK);
  ASSERT(check_num(v, 2.0));
  ASSERT(v7_exec(v7, &v, "m[0]") == V7_OK);
  ASSERT(check_str(v7, v, "foo 1234"));
  ASSERT(v7_exec(v7, &v, "m[1]") == V7_OK);
  ASSERT(check_str(v7, v, "1234"));
  ASSERT(v7_exec(v7, &v, "m[2]") == V7_OK);
  ASSERT(v7_is_undefined(v));
  ASSERT(v7_exec(v7, &v,
                 "m = 'should match empty string at index 0'.match(/x*/)") ==
         V7_OK);
  ASSERT(v7_exec(v7, &v, "m.length") == V7_OK);
  ASSERT(check_num(v, 1.0));
  ASSERT(v7_exec(v7, &v, "m[0]") == V7_OK);
  ASSERT(check_str(v7, v, ""));
  ASSERT(v7_exec(v7, &v, "m = 'aa bb cc'.split(); m.length") == V7_OK);
  ASSERT(check_num(v, 1.0));
  ASSERT(v7_exec(v7, &v, "m = 'aa bb cc'.split(''); m.length") == V7_OK);
  ASSERT(check_num(v, 8.0));
  ASSERT(v7_exec(v7, &v, "m = 'aa bb cc'.split(RegExp('')); m.length") ==
         V7_OK);
  ASSERT(check_num(v, 8.0));
  ASSERT(v7_exec(v7, &v, "m = 'aa bb cc'.split(/x*/); m.length") == V7_OK);
  ASSERT(check_num(v, 8.0));
  ASSERT(v7_exec(v7, &v, "m = 'aa bb cc'.split(/(x)*/); m.length") == V7_OK);
  ASSERT(check_num(v, 16.0));
  ASSERT(v7_exec(v7, &v, "m[0]") == V7_OK);
  ASSERT(check_str(v7, v, "a"));
  ASSERT(v7_exec(v7, &v, "m[1]") == V7_OK);
  ASSERT(v7_is_undefined(v));
  ASSERT(v7_exec(v7, &v, "m = 'aa bb cc'.split(' '); m.length") == V7_OK);
  ASSERT(check_num(v, 3.0));
  ASSERT(v7_exec(v7, &v, "m = 'aa bb cc'.split(' ', 2); m.length") == V7_OK);
  ASSERT(check_num(v, 2.0));
  ASSERT(v7_exec(v7, &v, "m = 'aa bb cc'.split(/ /, 2); m.length") == V7_OK);
  ASSERT(check_num(v, 2.0));
  ASSERT(v7_exec(v7, &v, "'aa bb cc'.substr(0, 4).split(' ').length") == V7_OK);
  ASSERT(check_num(v, 2.0));
  ASSERT(v7_exec(v7, &v, "'aa bb cc'.substr(0, 4).split(' ')[1]") == V7_OK);
  ASSERT(check_str(v7, v, "b"));
  ASSERT(v7_exec(v7, &v,
                 "({z: '123456'}).z"
                 ".toString().substr(0, 3).split('').length") == V7_OK);
  ASSERT(check_num(v, 3.0));
#endif /* V7_DISABLE_REGEX */
  ASSERT(v7_exec(v7, &v, "String('hi')") == V7_OK);
  ASSERT(check_str(v7, v, "hi"));
  ASSERT(v7_exec(v7, &v, "new String('blah')") == V7_OK);

  /* Date() tests interact with external object (local date & time), so
      if host have strange date/time setting it won't be work */

  ASSERT(v7_exec(v7, &v, "Number(new Date('IncDec 01 2015 00:00:00'))") ==
         V7_OK);
  ASSERT(check_value(v7, v, "NaN"));
  ASSERT(v7_exec(v7, &v, "Number(new Date('My Jul 01 2015 00:00:00'))") ==
         V7_OK);
  ASSERT(check_value(v7, v, "NaN"));

#if 0
  /* Date */
  tzset();

#define ADJTZ(x) (x + 2 * 3600000 + timezone * 1000)

#if 0
    /* this part of tests is tz & lang depended */

  ASSERT(v7_exec(v7, &v, "new Date(-999, 10, 9, 15, 40, 50, 777).toString()") == V7_OK);
  ASSERT(check_str(v7, v, "Tue Nov 09 -999 15:40:50 GMT+0200 (EET)"));
  ASSERT(v7_exec(v7, &v, "new Date(999, 10, 9, 15, 40, 50, 777).toString()") == V7_OK);
  ASSERT(check_str(v7, v, "Sun Nov 09 999 15:40:50 GMT+0200 (EET)"));
  ASSERT(v7_exec(v7, &v, "new Date(1999, 10, 9, 15, 40, 50, 777).toString()") == V7_OK);
  ASSERT(check_str(v7, v, "Tue Nov 09 1999 15:40:50 GMT+0200 (EET)"));
  ASSERT(v7_exec(v7, &v, "new Date(-999, 10, 9, 15, 40, 50, 777).toUTCString()") == V7_OK);
  ASSERT(check_str(v7, v, "Tue Nov 09 -999 13:40:50 GMT"));
  ASSERT(v7_exec(v7, &v, "new Date(999, 10, 9, 15, 40, 50, 777).toUTCString()") == V7_OK);
  ASSERT(check_str(v7, v, "Sun Nov 09 999 13:40:50 GMT"));
  ASSERT(v7_exec(v7, &v, "new Date(1999, 10, 9, 15, 40, 50, 777).toUTCString()") == V7_OK);
  ASSERT(check_str(v7, v, "Tue Nov 09 1999 13:40:50 GMT"));
  ASSERT(v7_exec(v7, &v, "new Date(-999, 10, 9, 15, 40, 50, 777).toDateString()") == V7_OK);
  ASSERT(check_str(v7, v, "Tue Nov 09 -999"));
  ASSERT(v7_exec(v7, &v, "new Date(999, 10, 9, 15, 40, 50, 777).toDateString()") == V7_OK);
  ASSERT(check_str(v7, v, "Sun Nov 09 999"));
  ASSERT(v7_exec(v7, &v, "new Date(1999, 10, 9, 15, 40, 50, 777).toDateString()") == V7_OK);
  ASSERT(check_str(v7, v, "Tue Nov 09 1999"));
  ASSERT(v7_exec(v7, &v, "new Date(-999, 10, 9, 15, 40, 50, 777).toDateString()") == V7_OK);
  ASSERT(check_str(v7, v, "Tue Nov 09 -999"));
  ASSERT(v7_exec(v7, &v, "new Date(999, 10, 9, 15, 40, 50, 777).toDateString()") == V7_OK);
  ASSERT(check_str(v7, v, "Sun Nov 09 999"));
  ASSERT(v7_exec(v7, &v, "new Date(1999, 10, 9, 15, 40, 50, 777).toDateString()") == V7_OK);
  ASSERT(check_str(v7, v, "Tue Nov 09 1999"));
  ASSERT(v7_exec(v7, &v, "new Date(-999, 10, 9, 15, 40, 50, 777).toTimeString()") == V7_OK);
  ASSERT(check_str(v7, v, "15:40:50 GMT+0200 (EET)"));
  ASSERT(v7_exec(v7, &v, "new Date(999, 10, 9, 15, 40, 50, 777).toTimeString()") == V7_OK);
  ASSERT(check_str(v7, v, "15:40:50 GMT+0200 (EET)"));
  ASSERT(v7_exec(v7, &v, "new Date(1999, 10, 9, 15, 40, 50, 777).toTimeString()") == V7_OK);
  ASSERT(check_str(v7, v, "15:40:50 GMT+0200 (EET)"));
  ASSERT(v7_exec(v7, &v, "new Date(1968, 10, 9, 15, 40, 50, 777).toLocaleString()") == V7_OK);
  ASSERT(check_str(v7, v, "воскресенье,  9 ноября 1968 г. 15:40:50"));
  ASSERT(v7_exec(v7, &v, "new Date(-100, 10, 9, 15, 40, 50, 777).toLocaleString()") == V7_OK);
  ASSERT(check_str(v7, v, "суббота,  9 ноября -100 г. 15:40:50"));
  ASSERT(v7_exec(v7, &v, "new Date(2015, 10, 9, 15, 40, 50, 777).toLocaleString()") == V7_OK);
  ASSERT(check_str(v7, v, "понедельник,  9 ноября 2015 г. 15:40:50"));
  ASSERT(v7_exec(v7, &v, "new Date(1968, 10, 9, 15, 40, 50, 777).toLocaleDateString()") == V7_OK);
  ASSERT(check_str(v7, v, "09.11.1968"));
  ASSERT(v7_exec(v7, &v, "new Date(-100, 10, 9, 15, 40, 50, 777).toLocaleDateString()") == V7_OK);
  ASSERT(check_str(v7, v, "09.11.-100"));
  ASSERT(v7_exec(v7, &v, "new Date(2015, 10, 9, 15, 40, 50, 777).toLocaleDateString()") == V7_OK);
  ASSERT(check_str(v7, v, "09.11.2015"));
  ASSERT(v7_exec(v7, &v, "new Date(1968, 10, 9, 15, 40, 50, 777).toLocaleTimeString()") == V7_OK);
  ASSERT(check_str(v7, v, "15:40:50"));
  ASSERT(v7_exec(v7, &v, "new Date(-100, 10, 9, 15, 40, 50, 777).toLocaleTimeString()") == V7_OK);
  ASSERT(check_str(v7, v, "15:40:50"));
  ASSERT(v7_exec(v7, &v, "new Date(2015, 10, 9, 15, 40, 50, 777).toLocaleTimeString()") == V7_OK);
  ASSERT(check_str(v7, v, "15:40:50"));

#endif

  ASSERT(v7_exec(v7, &v, "new Date(\"1959-12-31T23:59:59.999Z\").toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "1959-12-31T23:59:59.999Z"));
  ASSERT(v7_exec(v7, &v, "new Date(\"1969-12-31T23:59:59.999Z\").toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "1969-12-31T23:59:59.999Z"));
  ASSERT(v7_exec(v7, &v, "new Date(\"1979-12-31T23:59:59.999Z\").toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "1979-12-31T23:59:59.999Z"));
  ASSERT(v7_exec(v7, &v, "new Date(\"1979-01-01T00:01:01.001Z\").toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "1979-01-01T00:01:01.001Z"));
  ASSERT(v7_exec(v7, &v, "new Date(\"1970-01-01T00:01:01.001Z\").toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "1970-01-01T00:01:01.001Z"));\
  ASSERT(v7_exec(v7, &v, "new Date(\"959-12-31T23:59:59.999Z\").toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "959-12-31T23:59:59.999Z"));
  ASSERT(v7_exec(v7, &v, "new Date(\"-959-12-31T23:59:59.999Z\").toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "-000959-12-31T23:59:59.999Z"));
  ASSERT(v7_exec(v7, &v, "new Date(\"1970\").valueOf()") == V7_OK);
  ASSERT(check_num(v, 1970));
 ASSERT(v7_exec(v7, &v, "new Date(Date.UTC(Number(1999), Number(10), 15)).toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "1999-11-15T00:00:00.000Z"));
  ASSERT(v7_exec(v7, &v, "new Date(Date.UTC(-1000, 10, 10)).valueOf()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(-93696998400000)));
  ASSERT(v7_exec(v7, &v, "var d = new Date(Date.UTC(-1000, 10, 5))") == V7_OK);
  ASSERT(v7_exec(v7, &v, "d.getFullYear()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(-1000)));
  ASSERT(v7_exec(v7, &v, "d.getDate()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(5)));
  ASSERT(v7_exec(v7, &v, "d.getMonth()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(10)));
  ASSERT(v7_exec(v7, &v, "d.toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "-001000-11-05T00:00:00.000Z"));
  ASSERT(v7_exec(v7, &v, "d.setUTCMonth(9)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "d.toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "-001000-10-05T00:00:00.000Z"));
  ASSERT(v7_exec(v7, &v, "new Date(99,10).valueOf()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941407200000)));
  ASSERT(v7_exec(v7, &v, "new Date(99,10,5).valueOf()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941752800000)));
  ASSERT(v7_exec(v7, &v, "new Date(\"99\",\"10\",\"5\").valueOf()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941752800000)));
  ASSERT(v7_exec(v7, &v, "new Date(99,10,5,11).valueOf()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941792400000)));
  ASSERT(v7_exec(v7, &v, "new Date(99,10,5,11,35).valueOf()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941794500000)));
  ASSERT(v7_exec(v7, &v, "new Date(99,10,5,11,35,45).valueOf()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941794545000)));
  ASSERT(v7_exec(v7, &v, "new Date(99,10,5,11,35,45,567).valueOf()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941794545567)));
  ASSERT(v7_exec(v7, &v, "var d = new Date(1999,10,5,11,35,45,567)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "d.getTime()") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941794545567)));
  ASSERT(v7_exec(v7, &v, "d.getFullYear()") == V7_OK);
  ASSERT(check_num(v, 1999));
  ASSERT(v7_exec(v7, &v, "d.getUTCFullYear()") == V7_OK);
  ASSERT(check_num(v, 1999));
  ASSERT(v7_exec(v7, &v, "d.getMonth()") == V7_OK);
  ASSERT(check_num(v, 10));
  ASSERT(v7_exec(v7, &v, "d.getUTCMonth()") == V7_OK);
  ASSERT(check_num(v, 10));
  ASSERT(v7_exec(v7, &v, "d.getDate()") == V7_OK);
  ASSERT(check_num(v, 5));
  ASSERT(v7_exec(v7, &v, "d.getUTCDate()") == V7_OK);
  ASSERT(check_num(v, 5));
  ASSERT(v7_exec(v7, &v, "d.getDay()") == V7_OK);
  ASSERT(check_num(v, 5));
  ASSERT(v7_exec(v7, &v, "d.getUTCDay()") == V7_OK);
  ASSERT(check_num(v, 5));
  ASSERT(v7_exec(v7, &v, "d.getHours()") == V7_OK);
  ASSERT(check_num(v, 11));
  ASSERT(v7_exec(v7, &v, "d.getUTCHours()") == V7_OK);
  ASSERT(check_num(v, 11+timezone/3600));
  ASSERT(v7_exec(v7, &v, "d.getMinutes()") == V7_OK);
  ASSERT(check_num(v, 35));
  ASSERT(v7_exec(v7, &v, "d.getUTCMinutes()") == V7_OK);
  ASSERT(check_num(v, 35));
  ASSERT(v7_exec(v7, &v, "d.getSeconds()") == V7_OK);
  ASSERT(check_num(v, 45));
  ASSERT(v7_exec(v7, &v, "d.getUTCSeconds()") == V7_OK);
  ASSERT(check_num(v, 45));
  ASSERT(v7_exec(v7, &v, "d.getMilliseconds()") == V7_OK);
  ASSERT(check_num(v, 567));
  ASSERT(v7_exec(v7, &v, "d.getUTCMilliseconds()") == V7_OK);
  ASSERT(check_num(v, 567));
  ASSERT(v7_exec(v7, &v, "d.getTimezoneOffset()") == V7_OK);
  ASSERT(check_num(v, timezone/60));
  ASSERT(v7_exec(v7, &v, "d.setTime(10)") == V7_OK);
  ASSERT(check_num(v, 10));
  ASSERT(v7_exec(v7, &v, "d.valueOf()") == V7_OK);
  ASSERT(check_num(v, 10));
  ASSERT(v7_exec(v7, &v, "var j = new Date(1999,10,5,11,35,45,567)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "j.setMilliseconds(10)") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941794545010)));
  ASSERT(v7_exec(v7, &v, "j.setUTCMilliseconds(100)") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941794545100)));
  ASSERT(v7_exec(v7, &v, "j.setSeconds(10)") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941794510100)));
  ASSERT(v7_exec(v7, &v, "j.setUTCSeconds(30)") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941794530100)));
  ASSERT(v7_exec(v7, &v, "j.setMinutes(10)") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941793030100)));
  ASSERT(v7_exec(v7, &v, "j.setUTCMinutes(30)") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941794230100)));
  ASSERT(v7_exec(v7, &v, "j.setHours(10)") == V7_OK);
  ASSERT(check_num(v, ADJTZ(941790630100)));
  ASSERT(v7_exec(v7, &v, "j.setUTCHours(20)") == V7_OK);
  ASSERT(check_num(v, 941833830100));
  ASSERT(v7_exec(v7, &v, "j.setDate(15)") == V7_OK);
  ASSERT(check_num(v, 942697830100));
  ASSERT(v7_exec(v7, &v, "j.setUTCDate(20)") == V7_OK);
  ASSERT(check_num(v, 943129830100));
  ASSERT(v7_exec(v7, &v, "j.setMonth(10)") == V7_OK);
  ASSERT(check_num(v, 943129830100));
  ASSERT(v7_exec(v7, &v, "j.setUTCMonth(11)") == V7_OK);
  ASSERT(check_num(v, 945721830100));
  ASSERT(v7_exec(v7, &v, "j.setFullYear(2014)") == V7_OK);
  ASSERT(check_num(v, 1419107430100));
  ASSERT(v7_exec(v7, &v, "j.setUTCFullYear(2015)") == V7_OK);
  ASSERT(check_num(v, 1450643430100));
  ASSERT(v7_exec(v7, &v, "new Date(Date.UTC(1999,10,5,10,20,30,400)).toISOString()") == V7_OK);
  ASSERT(check_str(v7, v, "1999-11-05T10:20:30.400Z"));
  ASSERT(v7_exec(v7, &v, "new Date(Date.UTC(1999,10,5,10,20,30,400)).toJSON()") == V7_OK);
  ASSERT(check_str(v7, v, "1999-11-05T10:20:30.400Z"));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"2015-02-15T10:42:43.629Z\")") == V7_OK);
  ASSERT(check_num(v, 1423996963629));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"Sun Feb 15 2015 13:01:12 GMT+0200 (EET)\")") == V7_OK);
  ASSERT(check_num(v, 1423998072000));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"Sun Feb 15 2015 11:27:10 GMT\")") == V7_OK);
  ASSERT(check_num(v, 1423999630000));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"Sun Feb 15 2015\")") == V7_OK);
  ASSERT(check_num(v, ADJTZ(1423951200000)));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"02/15/2015\")") == V7_OK);
  ASSERT(check_num(v, ADJTZ(1423951200000)));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"2015-02-15\")") == V7_OK);
  ASSERT(check_num(v, ADJTZ(1423951200000)));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"15.02.2015\")") == V7_OK);
  ASSERT(check_num(v, ADJTZ(1423951200000)));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"02/15/2015 12:30\")") == V7_OK);
  ASSERT(check_num(v, ADJTZ(1423996200000)));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"02/15/2015 12:30:45\")") == V7_OK);
  ASSERT(check_num(v, ADJTZ(1423996245000)));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"10/15/2015 12:30:45 GMT+200\")") == V7_OK);
  ASSERT(check_num(v, 1444905045000));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"10/15/2015 12:30 GMT+200\")") == V7_OK);
  ASSERT(check_num(v, 1444905000000));
  ASSERT(v7_exec(v7, &v, "Date.parse(\"10/15/2015 12:30 GMT\")") == V7_OK);
  ASSERT(check_num(v, 1444912200000));
#endif

#if 0
  /* Regexp */
  ASSERT(v7_exec(v7, &v, "re = /GET (\\S+) HTTP/; re")) != NULL);
  ASSERT(v7_exec(v7, &v, "re = /GET (\\S+) HTTP/;")) != NULL);
  ASSERT(v7_exec(v7, &v, "re = /GET (\\S+) HTTP/ ")) != NULL);
  ASSERT(v7_exec(v7, &v, "re = /GET (\\S+) HTTP/\n")) != NULL);
  ASSERT(v7_exec(v7, &v, "re = /GET (\\S+) HTTP/")) != NULL);
#endif

#ifndef V7_DISABLE_SOCKETS

  ASSERT(v7_exec(v7, &v, "var d = new Socket()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "d.close()") == V7_OK);

  ASSERT(v7_exec(v7, &v, "var d = Socket()") != V7_OK);

  ASSERT(v7_exec(v7, &v, "var d = new Socket(Socket.AF_INET)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "d.close()") == V7_OK);

#ifdef V7_ENABLE_IPV6
  ASSERT(v7_exec(v7, &v, "var d = new Socket(Socket.AF_INET6)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "d.close()") == V7_OK);
#endif

  ASSERT(v7_exec(v7, &v,
                 "var d = new Socket(Socket.AF_INET,"
                 "Socket.SOCK_STREAM)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "d.close()") == V7_OK);

  ASSERT(v7_exec(v7, &v,
                 "var d = new Socket(Socket.AF_INET,"
                 "Socket.SOCK_DGRAM)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "d.close()") == V7_OK);

  ASSERT(v7_exec(v7, &v,
                 "var d = new Socket(Socket.AF_INET,"
                 "Socket.SOCK_STREAM, Socket.RECV_STRING)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "d.close()") == V7_OK);

  ASSERT(v7_exec(v7, &v,
                 "var d = new Socket(Socket.AF_INET,"
                 "Socket.SOCK_STREAM, Socket.RECV_RAW)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "d.close()") == V7_OK);

  ASSERT(v7_exec(v7, &v, "var s = new Socket()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "s.close()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "var s = new Socket()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "s.bind(60000)") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.localPort") == V7_OK);
  ASSERT(check_num(v, 60000));
  ASSERT(v7_exec(v7, &v, "s.bind(60000)") == V7_OK);
  ASSERT(check_num_not(v, 0));
  ASSERT(v7_exec(v7, &v, "s.close()") == V7_OK);

  ASSERT(v7_exec(v7, &v, "var t = new Socket()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "t.bind(\"non_number\")") == V7_OK);
  ASSERT(check_num_not(v, 0));
  ASSERT(v7_exec(v7, &v, "t.errno") == V7_OK);
  ASSERT(check_num(v, 22));
  ASSERT(v7_exec(v7, &v, "t.bind(12345.25)") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "t.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "t.close()") == V7_OK);

  ASSERT(v7_exec(v7, &v, "var s = new Socket()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "s.bind(12345)") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.listen()") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.close()") == V7_OK);

  ASSERT(v7_exec(v7, &v, "var s = new Socket()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "s.bind(12345, \"127.0.0.1\")") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.listen()") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.close()") == V7_OK);

  ASSERT(v7_exec(v7, &v, "var s = new Socket()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "s.bind(600000)") == V7_OK);
  ASSERT(check_num_not(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 22));
  ASSERT(v7_exec(v7, &v, "s.close()") == V7_OK);

#if 0
  /* start fossa/examples/tcp_echo_server for running these tests */
  ASSERT(v7_exec(v7, &v, "var s = new Socket()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "s.connect(\"127.0.0.1\", 17000)") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.localAddress") == V7_OK);
  ASSERT(check_str(v7, v, "127.0.0.1"));
  ASSERT(v7_exec(v7, &v, "s.remoteAddress") == V7_OK);
  ASSERT(check_str(v7, v, "127.0.0.1"));
  ASSERT(v7_exec(v7, &v, "s.remotePort") == V7_OK);
  ASSERT(check_num(v, 17000));
  ASSERT(v7_exec(v7, &v, "s.send(\"Hello, world!\")") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.recv()") == V7_OK);
  ASSERT(check_str(v7, v, "Hello, world!"));
  ASSERT(v7_exec(v7, &v, "var arr = [102, 117, 99, 107]") == V7_OK);
  ASSERT(v7_exec(v7, &v, "s.send(arr)") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "var r = s.recv(Socket.RECV_RAW)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "r.toString()") == V7_OK);
  ASSERT(check_str(v7, v, "102,117,99,107"));
  ASSERT(v7_exec(v7, &v, "s.close()") == V7_OK);

  /* ensure internet connection for running these tests */
  ASSERT(v7_exec(v7, &v, "var s = new Socket()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "s.connect(\"microsoft.com\", 80)") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.close()") == V7_OK);

  /* start fossa/examples/coap_server for running these tests */
  ASSERT(v7_exec(v7, &v,
                 "var s = new Socket(Socket.AF_INET,"
                 "Socket.SOCK_DGRAM)") == V7_OK);
  ASSERT(v7_exec(v7, &v, "var arr = [96, 0, 233, 27]") == V7_OK);
  ASSERT(v7_exec(v7, &v, "s.sendto(\"127.0.0.1\", 5683, arr)") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.close()") == V7_OK);

  ASSERT(v7_exec(v7, &v,
                 "var s = new Socket(Socket.AF_INET,"
                 "Socket.SOCK_DGRAM, Socket.RECV_RAW)") == V7_OK);
  ASSERT(v7_exec(v7, &v,
                 "var arr = [66, 1, 233, 27, 7, 144, "
                 "184, 115, 101, 112, 97, 114, 97, 116, 101, "
                 "16, 209, 35, 17]") == V7_OK);
  ASSERT(v7_exec(v7, &v, "s.sendto(\"127.0.0.1\", 5683, arr)") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "s.errno") == V7_OK);
  ASSERT(check_num(v, 0));
  ASSERT(v7_exec(v7, &v, "var t = s.recvfrom()") == V7_OK);
  ASSERT(v7_exec(v7, &v, "t.src.port") == V7_OK);
  ASSERT(check_num(v, 5683));
  ASSERT(v7_exec(v7, &v, "t.src.address") == V7_OK);
  ASSERT(check_str(v7, v, "127.0.0.1"));
  ASSERT(v7_exec(v7, &v, "t.src.family") == V7_OK);
  ASSERT(check_num(v, 2));
  ASSERT(v7_exec(v7, &v, "var y = t.data") == V7_OK);
  ASSERT(v7_exec(v7, &v, "y.toString()") == V7_OK);
  ASSERT(check_str(v7, v, "96,0,233,27"));
  ASSERT(v7_exec(v7, &v, "s.close()") == V7_OK);

#endif

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
  unsigned int i = 1;

  skip_to_next_tok(&p);

  /* Make sure divisions are parsed correctly - set previous token */
  while ((tok = get_tok(&p, &num, i > TOK_REGEX_LITERAL ? TOK_NUMBER : tok)) !=
         TOK_END_OF_INPUT) {
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
  const char *s;
  int i;

  v = v7_create_null();
  ASSERT(v7_is_null(v));

  v = v7_create_undefined();
  ASSERT(v7_is_undefined(v));

  v = v7_create_number(1.0);
  ASSERT(val_type(v7, v) == V7_TYPE_NUMBER);
  ASSERT(v7_to_double(v) == 1.0);
  ASSERT(check_value(v7, v, "1"));

  v = v7_create_number(1.5);
  ASSERT(v7_to_double(v) == 1.5);
  ASSERT(check_value(v7, v, "1.5"));

  v = v7_create_boolean(1);
  ASSERT(val_type(v7, v) == V7_TYPE_BOOLEAN);
  ASSERT(v7_to_boolean(v) == 1);
  ASSERT(check_value(v7, v, "true"));

  v = v7_create_boolean(0);
  ASSERT(check_value(v7, v, "false"));

  v = v7_create_string(v7, "foo", 3, 1);
  ASSERT(val_type(v7, v) == V7_TYPE_STRING);
  v7_to_string(v7, &v, &n);
  ASSERT(n == 3);
  s = "\"foo\"";
  ASSERT(check_value(v7, v, s));

  v = v7_create_object(v7);
  ASSERT(val_type(v7, v) == V7_TYPE_GENERIC_OBJECT);
  ASSERT(v7_to_object(v) != NULL);
  ASSERT(v7_to_object(v)->prototype != NULL);
  ASSERT(v7_to_object(v)->prototype->prototype == NULL);

  ASSERT(v7_set_property(v7, v, "foo", -1, 0, v7_create_null()) == 0);
  ASSERT((p = v7_get_property(v7, v, "foo", -1)) != NULL);
  ASSERT(p->attributes == 0);
  ASSERT(v7_is_null(p->value));
  ASSERT(check_value(v7, p->value, "null"));

  ASSERT(v7_set_property(v7, v, "foo", -1, 0, v7_create_undefined()) == 0);
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

  ASSERT(v7_del_property(v7, v, "foo", ~0) == 0);
  ASSERT(v7_to_object(v)->properties == NULL);
  ASSERT(v7_del_property(v7, v, "foo", -1) == -1);
  ASSERT(v7_set_property(v7, v, "foo", -1, 0,
                         v7_create_string(v7, "bar", 3, 1)) == 0);
  ASSERT(v7_set_property(v7, v, "bar", -1, 0,
                         v7_create_string(v7, "foo", 3, 1)) == 0);
  ASSERT(v7_set_property(v7, v, "aba", -1, 0,
                         v7_create_string(v7, "bab", 3, 1)) == 0);
  ASSERT(v7_del_property(v7, v, "foo", -1) == 0);
  ASSERT((p = v7_get_property(v7, v, "foo", -1)) == NULL);
  ASSERT(v7_del_property(v7, v, "aba", -1) == 0);
  ASSERT((p = v7_get_property(v7, v, "aba", -1)) == NULL);
  ASSERT(v7_del_property(v7, v, "bar", -1) == 0);
  ASSERT((p = v7_get_property(v7, v, "bar", -1)) == NULL);

  v = v7_create_object(v7);
  ASSERT(v7_set_property(v7, v, "foo", -1, 0, v7_create_number(1.0)) == 0);
  ASSERT((p = v7_get_property(v7, v, "foo", -1)) != NULL);
  ASSERT((p = v7_get_property(v7, v, "f", -1)) == NULL);

  v = v7_create_object(v7);
  ASSERT(v7_set_property(v7, v, "foo", -1, 0, v) == 0);
  s = "{\"foo\":[Circular]}";
  ASSERT(check_value(v7, v, s));

  v = v7_create_object(v7);
  ASSERT(v7_set_property(v7, v, "foo", -1, V7_PROPERTY_DONT_DELETE,
                         v7_create_number(1.0)) == 0);
  s = "{\"foo\":1}";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_set(v7, v, "foo", -1, v7_create_number(2.0)) == 0);
  s = "{\"foo\":2}";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_to_double(v7_get(v7, v, "foo", -1)) == 2.0);
  ASSERT(v7_get_property(v7, v, "foo", -1)->attributes &
         V7_PROPERTY_DONT_DELETE);
  ASSERT(v7_set_property(v7, v, "foo", -1, V7_PROPERTY_READ_ONLY,
                         v7_create_number(1.0)) == 0);
  ASSERT(v7_set(v7, v, "foo", -1, v7_create_number(2.0)) != 0);
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

static const char *test_dense_arrays(void) {
  struct v7 *v7 = v7_create();
  val_t a;

  a = v7_create_dense_array(v7);

  v7_array_set(v7, a, 0, v7_create_number(42));
  ASSERT(check_num(v7_array_get(v7, a, 0), 42));
  ASSERT(v7_array_length(v7, a) == 1);

  v7_array_set(v7, a, 1, v7_create_number(24));
  ASSERT(check_num(v7_array_get(v7, a, 0), 42));
  ASSERT(check_num(v7_array_get(v7, a, 1), 24));
  ASSERT(v7_array_length(v7, a) == 2);

  a = v7_create_dense_array(v7);
  v7_array_set(v7, a, 0, v7_create_number(42));
  v7_array_set(v7, a, 2, v7_create_number(42));
  ASSERT(v7_array_length(v7, a) == 3);

  a = v7_create_dense_array(v7);
  v7_array_set(v7, a, 1, v7_create_number(42));
  ASSERT(v7_array_length(v7, a) == 2);

  ASSERT(v7_exec(v7, &a, "function mka(){return arguments}") == V7_OK);

  ASSERT(v7_exec(v7, &a, "a=mka(1,2,3);a.splice(0,1);a") == V7_OK);
  ASSERT(check_value(v7, a, "[2,3]"));
  ASSERT(v7_exec(v7, &a, "a=mka(1,2,3);a.splice(2,1);a") == V7_OK);
  ASSERT(check_value(v7, a, "[1,2]"));
  ASSERT(v7_exec(v7, &a, "a=mka(1,2,3);a.splice(1,1);a") == V7_OK);
  ASSERT(check_value(v7, a, "[1,3]"));

  ASSERT(v7_exec(v7, &a, "a=mka(1,2,3);a.slice(0,1)") == V7_OK);
  ASSERT(check_value(v7, a, "[1]"));
  ASSERT(v7_exec(v7, &a, "a=mka(1,2,3);a.slice(2,3)") == V7_OK);
  ASSERT(check_value(v7, a, "[3]"));
  ASSERT(v7_exec(v7, &a, "a=mka(1,2,3);a.slice(1,3)") == V7_OK);
  ASSERT(check_value(v7, a, "[2,3]"));

  ASSERT(v7_exec(v7, &a, "a=mka(1,2,3);a.indexOf(3)") == V7_OK);
  ASSERT(check_num(a, 2));

  v7_destroy(v7);
  return NULL;
}

static const char *test_parser(void) {
  int i;
  struct ast a;
  struct v7 *v7 = v7_create();
  const char *cases[] = {
      "1", "true", "false", "null", "undefined", "1+2", "1-2", "1*2", "1/2",
      "1%2", "1/-2", "(1 + 2) * x + 3", "1 + (2, 3) * 4, 5", "(a=1 + 2) + 3",
      "1 ? 2 : 3", "1 ? 2 : 3 ? 4 : 5", "1 ? 2 : (3 ? 4 : 5)", "1 || 2 + 2",
      "1 && 2 || 3 && 4 + 5", "1|2 && 3|3", "1^2|3^4", "1&2^2&4|5&6", "1==2&3",
      "1<2", "1<=2", "1>=2", "1>2", "1==1<2", "a instanceof b", "1 in b",
      "1!=2&3", "1!==2&3", "1===2", "1<<2", "1>>2", "1>>>2", "1<<2<3", "1/2/2",
      "(1/2)/2", "1 + + 1", "1- -2", "!1", "~0", "void x()", "delete x",
      "typeof x", "++x", "--i", "x++", "i--", "a.b", "a.b.c", "a[0]", "a[0].b",
      "a[0][1]", "a[b[0].c]", "a()", "a(0)", "a(0, 1)", "a(0, (1, 2))",
      "1 + a(0, (1, 2)) + 2", "new x", "new x(0, 1)", "new x.y(0, 1)",
      "new x.y", "1;", "1;2;", "1;2", "1\nx", "p()\np()\np();p()", ";1",
      "if (1) 2", "if (1) 2; else 3", "if (1) {2;3}", "if (1) {2;3}; 4",
      "if (1) {2;3} else {4;5}", "while (1);", "while(1) {}", "while (1) 2;",
      "while (1) {2;3}", "for (i = 0; i < 3; i++) i++", "for (i=0; i<3;) i++",
      "for (i=0;;) i++", "for (;i<3;) i++", "for (;;) i++", "debugger",
      "while(1) break", "while(1) break loop", "while(1) continue",
      "while(1) continue loop", "function f() {return}",
      "function f() {return 1+2}", "function f() {if (1) {return;}}",
      "function f() {if (1) {return 2}}", "throw 1+2", "try { 1 }",
      "try { 1 } catch (e) { 2 }", "try {1} finally {3}",
      "try {1} catch (e) {2} finally {3}", "var x", "var x, y", "var x=1, y",
      "var y, x=y=1", "function x(a) {return a}", "function x() {return 1}",
      "[1,2,3]", "[1+2,3+4,5+6]", "[1,[2,[[3]]]]", "({a: 1})", "({a: 1, b: 2})",
      "({})", "(function(a) { return a + 1 })",
      "(function f(a) { return a + 1 })", "(function f() { return; 1;})",
      "function f() {while (1) {return;2}}", "switch(a) {case 1: break;}",
      "switch(a) {case 1: p(); break;}", "switch(a) {case 1: a; case 2: b; c;}",
      "switch(a) {case 1: a; b; default: c;}",
      "switch(a) {case 1: p(); break; break; }",
      "switch(a) {case 1: break; case 2: 1; case 3:}",
      "switch(a) {case 1: break; case 2: 1; case 3: default: break}",
      "switch(a) {case 1: break; case 3: default: break; case 2: 1}",
      "for (var i = 0; i < 3; i++) i++",
      "for (var i=0, j=i; i < 3; i++, j++) i++", "a%=1", "a*=1", "a/=1", "a+=1",
      "a-=1", "a|=1", "a&=1", "a<<=1", "a>>2", "a>>=1", "a>>>=1", "a=b=c+=1",
      "a=(b=(c+=1))", "\"foo\"", "var undefined = 1", "undefined",
      "{var x=1;2;}", "({get a() { return 1 }})", "({set a(b) { this.x = b }})",
      "({get a() { return 1 }, set b(c) { this.x = c }, d: 0})",
      "({get: function() {return 42;}})",
      "Object.defineProperty(o, \"foo\", {get: function() {return 42;}});",
      "({a: 0, \"b\": 1})", "({a: 0, 42: 1})", "({a: 0, 42.99: 1})",
      "({a: 0, })", "({true: 0, null: 1, undefined: 2, this: 3})", "[]", "[,2]",
      "[,]", "[,2]", "[,,,1,2]", "delete z", "delete (1+2)",
      "delete (delete z)", "delete delete z", "+ + + 2", "throw 'ex'",
      "switch(a) {case 1: try { 1; } catch (e) { 2; } finally {}; break}",
      "switch(a) {case 1: try { 1; } catch (e) { 2; } finally {break}; break}",
      "switch(a) {case 1: try { 1; } catch (e) { 2; } finally {break}; break; "
      "default: 1; break;}",
      "try {1} catch(e){}\n1", "try {1} catch(e){} 1",
      "switch(v) {case 0: break;} 1",
      "switch(a) {case 1: break; case 3: default: break; case 2: 1; default: "
      "2}",
      "do { i-- } while (i > 0)", "if (false) 1; 1;", "while(true) 1; 1;",
      "while(true) {} 1;", "do {} while(false) 1;", "with (a) 1; 2;",
      "with (a) {1} 2;", "for(var i in a) {1}", "for(i in a) {1}",
      "!function(){function d(){}var x}();",
      "({get a() { function d(){} return 1 }})",
      "({set a(v) { function d(a){} d(v) }})", "{function d(){}var x}",
      "try{function d(){}var x}catch(e){function d(){}var x}finally{function "
      "d(){}var x}",
      "{} {}", "if(1){function d(){}var x}",
      "if(1){} else {function d(){}var x}",
      "var \\u0076, _\\u0077, a\\u0078b, жабоскрипт;", "a.in + b.for",
      "var x = { null: 5, else: 4 }", "lab: x=1",
      "'use strict';0;'use strict';", "'use strict';if(0){'use strict';}",
      "(function(){'use strict';0;'use strict';})()"};
  const char *invalid[] = {
      "function(a) { return 1 }", "i\n++", "{a: 1, b: 2}", "({, a: 0})",
      "break", "break loop", "continue", "continue loop", "return",
      "return 1+2", "if (1) {return;}", "if (1) {return 2}",
      "(function(){'use strict'; with({}){}})", "v = [", "var v = ["};
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
  fread(want_ast, sizeof(want_ast), 1, fp);
  ASSERT(feof(fp));
  fclose(fp);

  for (i = 0; i < (int) ARRAY_SIZE(cases); i++) {
    char *current_want_ast = next_want_ast;
    ASSERT((next_want_ast = strchr(current_want_ast, '\0') + 1) != NULL);
    want_ast_len = (size_t)(next_want_ast - current_want_ast - 1);
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
    ASSERT(strncmp(got_ast, current_want_ast, sizeof(got_ast)) == 0);
#endif
  }

#else /* SAVE_AST */

  (void) got_ast;
  (void) next_want_ast;
  (void) want_ast_len;
  ASSERT((fp = fopen(want_ast_db, "w")) != NULL);
  for (i = 0; i < (int) ARRAY_SIZE(cases); i++) {
    ast_free(&a);
    ASSERT(parse(v7, &a, cases[i], 1) == V7_OK);
    ast_dump(fp, &a, 0);
    fwrite("\0", 1, 1, fp);
  }
  fclose(fp);

#endif /* SAVE_AST */

  for (i = 0; i < (int) ARRAY_SIZE(invalid); i++) {
    ast_free(&a);
#if 0
    printf("-- Parsing \"%s\"\n", invalid[i]);
#endif
    ASSERT(parse(v7, &a, invalid[i], 0) == V7_SYNTAX_ERROR);
  }

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

    v7 = v7_create();
    if (i == 1231 || i == 1250 || i == 1252 || i == 1253 || i == 1251 ||
        i == 1255 || i == 2649 || i == 2068 || i == 7445 || i == 7446 ||
        i == 3400 || i == 3348 || i == 3349 || i == 3401 || i == 89 ||
        i == 462) {
      fprintf(r, "%i\tSKIP %s\n", i, tail_cmd);
      continue;
    }

#if V7_VERBOSE_ECMA
    printf("-- Parsing %d: \"%s\"\n", i, current_case);
#endif
    ASSERT(parse(v7, &a, current_case, 1) == V7_OK);
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
  }
  printf("ECMA tests coverage: %.2lf%% (%d of %d)\n",
         (double) passed / i * 100.0, passed, i);

  free(db);
  fclose(r);
  rename(".ecma_report.txt", "ecma_report.txt");
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
  const char *s;

  v7_set_property(v7, v7->global_object, "x", -1, 0, v7_create_number(42.0));

  ASSERT(v7_exec(v7, &v, "1%2/2") == V7_OK);
  ASSERT(check_value(v7, v, "0.5"));

  ASSERT(v7_exec(v7, &v, "1+x") == V7_OK);
  ASSERT(check_value(v7, v, "43"));
  ASSERT(v7_exec(v7, &v, "2-'1'") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "1+2") == V7_OK);
  ASSERT(check_value(v7, v, "3"));
  ASSERT(v7_exec(v7, &v, "'1'+'2'") == V7_OK);
  s = "\"12\"";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v, "'1'+2") == V7_OK);
  ASSERT(check_value(v7, v, s));

  ASSERT(v7_exec(v7, &v, "false+1") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "true+1") == V7_OK);
  ASSERT(check_value(v7, v, "2"));

  ASSERT(v7_exec(v7, &v, "'1'<2") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'1'>2") == V7_OK);
  ASSERT(check_value(v7, v, "false"));

  ASSERT(v7_exec(v7, &v, "1==1") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "1==2") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "'1'==1") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'1'!=0") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'-1'==-1") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "a={};a===a") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "a={};a!==a") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "a={};a==a") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "a={};a!=a") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "a={};b={};a===b") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "a={};b={};a!==b") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "a={};b={};a==b") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "a={};b={};a!=b") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "1-{}") == V7_OK);
  ASSERT(check_value(v7, v, "NaN"));
  ASSERT(v7_exec(v7, &v, "a={};a===(1-{})") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "a={};a!==(1-{})") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "a={};a==(1-{})") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "a={};a!=(1-{})") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "a={};a===1") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "a={};a!==1") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "a={};a==1") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "a={};a!=1") == V7_OK);
  ASSERT(check_value(v7, v, "true"));

  ASSERT(v7_exec(v7, &v, "'x'=='x'") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'x'==='x'") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'object'=='object'") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'stringlong'=='longstring'") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "'object'==='object'") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'a'<'b'") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'b'>'a'") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'a'>='a'") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'a'<='a'") == V7_OK);
  ASSERT(check_value(v7, v, "true"));

  ASSERT(v7_exec(v7, &v, "+'1'") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "-'-1'") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "v=[10+1,20*2,30/3]") == V7_OK);
  ASSERT(val_type(v7, v) == V7_TYPE_ARRAY_OBJECT);
  ASSERT(v7_array_length(v7, v) == 3);
  ASSERT(check_value(v7, v, "[11,40,10]"));
  ASSERT(v7_exec(v7, &v, "v[0]") == V7_OK);
  ASSERT(check_value(v7, v, "11"));
  ASSERT(v7_exec(v7, &v, "v[1]") == V7_OK);
  ASSERT(check_value(v7, v, "40"));
  ASSERT(v7_exec(v7, &v, "v[2]") == V7_OK);
  ASSERT(check_value(v7, v, "10"));

  ASSERT(v7_exec(v7, &v, "v=[10+1,undefined,30/3]") == V7_OK);
  ASSERT(v7_array_length(v7, v) == 3);
  ASSERT(check_value(v7, v, "[11,undefined,10]"));

  ASSERT(v7_exec(v7, &v, "v=[10+1,,30/3]") == V7_OK);
  ASSERT(v7_array_length(v7, v) == 3);
  ASSERT(check_value(v7, v, "[11,,10]"));

  ASSERT(v7_exec(v7, &v, "3,2,1") == V7_OK);
  ASSERT(check_value(v7, v, "1"));

  ASSERT(v7_exec(v7, &v, "x=1") == V7_OK);
  ASSERT(check_value(v7, v, "1"));

  ASSERT(v7_exec(v7, &v, "1+2; 1") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "x=42; x") == V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "x=y=42; x+y") == V7_OK);
  ASSERT(check_value(v7, v, "84"));

  ASSERT(v7_exec(v7, &v, "o={a: 1, b: 2}") == V7_OK);
  ASSERT(v7_exec(v7, &v, "o['a'] + o['b']") == V7_OK);
  ASSERT(check_value(v7, v, "3"));

  ASSERT(v7_exec(v7, &v, "o.a + o.b") == V7_OK);
  ASSERT(check_value(v7, v, "3"));

  ASSERT(v7_exec(v7, &v, "Array(1,2)") == V7_OK);
  ASSERT(check_value(v7, v, "[1,2]"));
  ASSERT(v7_exec(v7, &v, "new Array(1,2)") == V7_OK);
  ASSERT(check_value(v7, v, "[1,2]"));
  ASSERT(
      v7_exec(v7, &v,
              "Object.isPrototypeOf(Array(1,2), Object.getPrototypeOf([]))") ==
      V7_OK);
  ASSERT(v7_exec(v7, &v, "a=[];r=a.push(1,2,3);[r,a]") == V7_OK);
  ASSERT(check_value(v7, v, "[3,[1,2,3]]"));

  ASSERT(v7_exec(v7, &v, "x=1;if(x>0){x=2};x") == V7_OK);
  ASSERT(check_value(v7, v, "2"));
  ASSERT(v7_exec(v7, &v, "x=1;if(x<0){x=2};x") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "x=0;if(true)x=2;else x=3;x") == V7_OK);
  ASSERT(check_value(v7, v, "2"));
  ASSERT(v7_exec(v7, &v, "x=0;if(false)x=2;else x=3;x") == V7_OK);
  ASSERT(check_value(v7, v, "3"));

  ASSERT(v7_exec(v7, &v, "y=1;x=5;while(x > 0){y=y*x;x=x-1};y") == V7_OK);
  ASSERT(check_value(v7, v, "120"));
  ASSERT(v7_exec(v7, &v, "y=1;x=5;do{y=y*x;x=x-1}while(x>0);y") == V7_OK);
  ASSERT(check_value(v7, v, "120"));
  ASSERT(v7_exec(v7, &v, "for(y=1,i=1;i<=5;i=i+1)y=y*i;y") == V7_OK);
  ASSERT(check_value(v7, v, "120"));
  ASSERT(v7_exec(v7, &v, "for(i=0;1;i++)if(i==5)break;i") == V7_OK);
  ASSERT(check_value(v7, v, "5"));
  ASSERT(v7_exec(v7, &v, "for(i=0;1;i++)if(i==5)break;i") == V7_OK);
  ASSERT(check_value(v7, v, "5"));
  ASSERT(v7_exec(v7, &v, "i=0;while(++i)if(i==5)break;i") == V7_OK);
  ASSERT(check_value(v7, v, "5"));
  ASSERT(v7_exec(v7, &v, "i=0;do{if(i==5)break}while(++i);i") == V7_OK);
  ASSERT(check_value(v7, v, "5"));
  ASSERT(v7_exec(v7, &v,
                 "(function(){i=0;do{if(i==5)break}while(++i);i+=10})();i") ==
         V7_OK);
  ASSERT(check_value(v7, v, "15"));
  ASSERT(v7_exec(v7, &v,
                 "(function(){x=i=0;do{if(i==5)break;if(i%2)continue;x++}while("
                 "++i);i+=10})();[i,x]") == V7_OK);
  ASSERT(check_value(v7, v, "[15,3]"));
  ASSERT(v7_exec(v7, &v,
                 "(function(){i=0;while(++i){if(i==5)break};i+=10})();i") ==
         V7_OK);
  ASSERT(check_value(v7, v, "15"));
  ASSERT(v7_exec(v7, &v,
                 "(function(){x=i=0;while(++i){if(i==5)break;if(i%2)continue;x+"
                 "+};i+=10})();[i,x]") == V7_OK);
  ASSERT(check_value(v7, v, "[15,2]"));
  ASSERT(v7_exec(v7, &v,
                 "(function(){for(i=0;1;++i){if(i==5)break};i+=10})();i") ==
         V7_OK);
  ASSERT(check_value(v7, v, "15"));
  ASSERT(v7_exec(v7, &v,
                 "(function(){x=0;for(i=0;1;++i){if(i==5)break;if(i%2)continue;"
                 "x++};i+=10})();[i,x]") == V7_OK);
  ASSERT(check_value(v7, v, "[15,3]"));
  ASSERT(
      v7_exec(v7, &v, "a=1,[(function(){function a(){1+2}; return a})(),a]") ==
      V7_OK);
  ASSERT(check_value(v7, v, "[[function a()],1]"));
  ASSERT(
      v7_exec(
          v7, &v,
          "x=0;(function(){try{ff; x=42}catch(e){x=1};function ff(){}})();x") ==
      V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(
      v7_exec(v7, &v, "a=1,[(function(){return a; function a(){1+2}})(),a]") ==
      V7_OK);
  ASSERT(check_value(v7, v, "[[function a()],1]"));
  ASSERT(v7_exec(v7, &v, "(function(){f=42;function f(){};return f})()") ==
         V7_OK);
  ASSERT(check_value(v7, v, "42"));

  ASSERT(v7_exec(v7, &v, "x=0;try{x=1};x") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "x=0;try{x=1}finally{x=x+1};x") == V7_OK);
  ASSERT(check_value(v7, v, "2"));
  ASSERT(v7_exec(v7, &v, "x=0;try{x=1}catch(e){x=100}finally{x=x+1};x") ==
         V7_OK);
  ASSERT(check_value(v7, v, "2"));

  ASSERT(v7_exec(v7, &v, "x=0;try{xxx;var xxx;x=42}catch(e){x=1};x") == V7_OK);
  ASSERT(check_value(v7, v, "42"));

  ASSERT(v7_exec(v7, &v, "(function(a) {return a})") == V7_OK);
  ASSERT(check_value(v7, v, "[function(a)]"));
  ASSERT(v7_exec(v7, &v, "(function() {var x=1,y=2; return x})") == V7_OK);
  ASSERT(check_value(v7, v, "[function(){var x,y}]"));
  ASSERT(v7_exec(v7, &v, "(function(a) {var x=1,y=2; return x})") == V7_OK);
  ASSERT(check_value(v7, v, "[function(a){var x,y}]"));
  ASSERT(v7_exec(v7, &v, "(function(a,b) {var x=1,y=2; return x; var z})") ==
         V7_OK);
  ASSERT(check_value(v7, v, "[function(a,b){var x,y,z}]"));
  ASSERT(v7_exec(v7, &v, "(function(a) {var x=1; for(var y in x){}; var z})") ==
         V7_OK);
  ASSERT(check_value(v7, v, "[function(a){var x,y,z}]"));
  ASSERT(v7_exec(v7, &v,
                 "(function(a) {var x=1; for(var y=0;y<x;y++){}; var z})") ==
         V7_OK);
  ASSERT(check_value(v7, v, "[function(a){var x,y,z}]"));
  ASSERT(
      v7_exec(v7, &v, "(function() {var x=(function y(){for(var z;;){}})})") ==
      V7_OK);
  ASSERT(check_value(v7, v, "[function(){var x}]"));
  ASSERT(v7_exec(v7, &v, "function square(x){return x*x;};square") == V7_OK);
  ASSERT(check_value(v7, v, "[function square(x)]"));
  ASSERT(v7_exec(v7, &v, "0;f=(function(x){return x*x;})") == V7_OK);
  ASSERT(check_value(v7, v, "[function(x)]"));

  ASSERT(v7_exec(v7, &v, "f=(function(x){return x*x;}); f(2)") == V7_OK);
  ASSERT(check_value(v7, v, "4"));
  ASSERT(v7_exec(v7, &v, "(function(x){x*x;})(2)") == V7_OK);
  ASSERT(check_value(v7, v, "undefined"));
  ASSERT(v7_exec(v7, &v, "f=(function(x){return x*x;x});v=f(2);v*2") == V7_OK);
  ASSERT(check_value(v7, v, "8"));
  ASSERT(v7_exec(v7, &v, "(function(x,y){return x+y;})(40,2)") == V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "(function(x,y){if(x==40)return x+y})(40,2)") ==
         V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "(function(x,y){return x+y})(40)") == V7_OK);
  ASSERT(check_value(v7, v, "NaN"));
  ASSERT(v7_exec(v7, &v, "(function(x){return x+y; var y})(40)") == V7_OK);
  ASSERT(check_value(v7, v, "NaN"));
  ASSERT(v7_exec(v7, &v,
                 "x=1;(function(a){return a})(40,(function(){x=x+1})())+x") ==
         V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "(function(){x=42;return;x=0})();x") == V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "(function(){for(i=0;1;i++)if(i==5)return i})()") ==
         V7_OK);
  ASSERT(check_value(v7, v, "5"));
  ASSERT(v7_exec(v7, &v, "(function(){i=0;while(++i)if(i==5)return i})()") ==
         V7_OK);
  ASSERT(check_value(v7, v, "5"));
  ASSERT(
      v7_exec(v7, &v, "(function(){i=0;do{if(i==5)return i}while(++i)})()") ==
      V7_OK);
  ASSERT(check_value(v7, v, "5"));

  ASSERT(v7_exec(
             v7, &v,
             "(function(x,y){return x+y})(40,2,(function(){return fail})())") ==
         V7_EXEC_EXCEPTION);

  ASSERT(v7_exec(v7, &v, "x=42; (function(){return x})()") == V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "x=2; (function(x){return x})(40)+x") == V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "x=1; (function(y){x=x+1; return y})(40)+x") == V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v,
                 "x=0;f=function(){x=42; return function() {return x}; var "
                 "x};f()()") == V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "x=42;o={x:66,f:function(){return this}};o.f().x") ==
         V7_OK);
  ASSERT(check_value(v7, v, "66"));
  ASSERT(
      v7_exec(v7, &v, "x=42;o={x:66,f:function(){return this}};(1,o.f)().x") ==
      V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "x=66;o={x:42,f:function(){return this.x}};o.f()") ==
         V7_OK);
  ASSERT(check_value(v7, v, "42"));

  ASSERT(v7_exec(v7, &v, "o={};o.x=24") == V7_OK);
  ASSERT(check_value(v7, v, "24"));
  ASSERT(v7_exec(v7, &v, "o.a={};o.a.b={c:66};o.a.b.c") == V7_OK);
  ASSERT(check_value(v7, v, "66"));
  ASSERT(v7_exec(v7, &v, "o['a']['b'].c") == V7_OK);
  ASSERT(check_value(v7, v, "66"));
  ASSERT(v7_exec(v7, &v, "o={a:1}; o['a']=2;o.a") == V7_OK);
  ASSERT(check_value(v7, v, "2"));
  ASSERT(v7_exec(v7, &v, "a={f:function(){return {b:55}}};a.f().b") == V7_OK);
  ASSERT(check_value(v7, v, "55"));
  ASSERT(v7_exec(v7, &v, "(function(){fox=1})();fox") == V7_OK);
  ASSERT(check_value(v7, v, "1"));

  ASSERT(v7_exec(v7, &v,
                 "fin=0;(function(){while(1){try{xxxx}finally{fin=1;return "
                 "1}}})();fin") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v,
                 "ca=0;fin=0;(function(){try{(function(){try{xxxx}finally{fin="
                 "1}})()}catch(e){ca=1}})();fin+ca") == V7_OK);
  ASSERT(check_value(v7, v, "2"));
  ASSERT(v7_exec(v7, &v, "x=0;try{throw 1}catch(e){x=42};x") == V7_OK);
  ASSERT(check_value(v7, v, "42"));

  ASSERT(v7_exec(v7, &v, "x=1;x=x<<3;x") == V7_OK);
  ASSERT(check_value(v7, v, "8"));
  ASSERT(v7_exec(v7, &v, "x=1;x<<=4;x") == V7_OK);
  ASSERT(check_value(v7, v, "16"));
  ASSERT(v7_exec(v7, &v, "x=1;x++") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "x") == V7_OK);
  ASSERT(check_value(v7, v, "2"));
  ASSERT(v7_exec(v7, &v, "x=1;++x") == V7_OK);
  ASSERT(check_value(v7, v, "2"));
  ASSERT(v7_exec(v7, &v, "x") == V7_OK);
  ASSERT(check_value(v7, v, "2"));
  ASSERT(v7_exec(v7, &v, "o={x:1};o.x++") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "o.x") == V7_OK);
  ASSERT(check_value(v7, v, "2"));

  ASSERT(v7_exec(v7, &v, "typeof dummyx") == V7_OK);
  s = "\"undefined\"";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v, "typeof null") == V7_OK);
  s = "\"object\"";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v, "typeof 1") == V7_OK);
  s = "\"number\"";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v, "typeof (1+2)") == V7_OK);
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v, "typeof 'test'") == V7_OK);
  s = "\"string\"";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v, "typeof [1,2]") == V7_OK);
  s = "\"object\"";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v, "typeof function(){}") == V7_OK);
  s = "\"function\"";
  ASSERT(check_value(v7, v, s));

  ASSERT(v7_exec(v7, &v, "void(1+2)") == V7_OK);
  ASSERT(check_value(v7, v, "undefined"));
  ASSERT(v7_exec(v7, &v, "true?1:2") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "false?1:2") == V7_OK);
  ASSERT(check_value(v7, v, "2"));
  ASSERT(v7_exec(v7, &v, "'a' in {a:1}") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "'b' in {a:1}") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "1 in [10,20]") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "20 in [10,20]") == V7_OK);
  ASSERT(check_value(v7, v, "false"));

  ASSERT(v7_exec(v7, &v, "x=1; delete x; typeof x") == V7_OK);
  s = "\"undefined\"";
  ASSERT(check_value(v7, v, s));
  ASSERT(
      v7_exec(v7, &v, "x=1; (function(){x=2;delete x; return typeof x})()") ==
      V7_OK);
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v, "x=1; (function(){x=2;delete x})(); typeof x") ==
         V7_OK);
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v, "x=1; (function(){var x=2;delete x})(); x") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "o={a:1};delete o.a;o") == V7_OK);
  ASSERT(check_value(v7, v, "{}"));
  ASSERT(v7_exec(v7, &v, "o={a:1};delete o['a'];o") == V7_OK);
  ASSERT(check_value(v7, v, "{}"));
  ASSERT(v7_exec(v7, &v, "x=0;if(delete 1 == true)x=42;x") == V7_OK);
  ASSERT(check_value(v7, v, "42"));

  ASSERT(v7_exec(v7, &v, "o={};a=[o];o.a=a;a") == V7_OK);
  s = "[{\"a\":[Circular]}]";
  ASSERT(check_value(v7, v, s));

  ASSERT(v7_exec(v7, &v, "new TypeError instanceof Error") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "new TypeError instanceof TypeError") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "new Error instanceof Object") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "new Error instanceof TypeError") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  /* TODO(mkm): fix parser: should not require parenthesis */
  ASSERT(v7_exec(v7, &v, "({}) instanceof Object") == V7_OK);
  ASSERT(check_value(v7, v, "true"));

  ASSERT(v7_exec(v7, &v, "") == V7_OK && v7_is_undefined(v));
#if 0
  ASSERT(v7_exec(v7, &v, "x=0;a=1;o={a:2};with(o){x=a};x") == V7_OK);
  ASSERT(check_value(v7, v, "2"));
#endif

  ASSERT(v7_exec(v7, &v,
                 "(function(){try {throw new Error}catch(e){c=e}})();c "
                 "instanceof Error") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v,
                 "delete e;(function(){try {throw new "
                 "Error}catch(e){}})();typeof e") == V7_OK);
  s = "\"undefined\"";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v,
                 "x=(function(){c=1;try {throw 1}catch(e){c=0};return c})()") ==
         V7_OK);
  ASSERT(check_value(v7, v, "0"));
  ASSERT(v7_exec(
             v7, &v,
             "x=(function(){var c=1;try {throw 1}catch(e){c=0};return c})()") ==
         V7_OK);
  ASSERT(check_value(v7, v, "0"));
  ASSERT(v7_exec(v7, &v,
                 "c=1;x=(function(){try {throw 1}catch(e){var c=0};return "
                 "c})();[c,x]") == V7_OK);
  ASSERT(check_value(v7, v, "[1,0]"));
  ASSERT(
      v7_exec(
          v7, &v,
          "c=1;x=(function(){try {throw 1}catch(e){c=0};return c})();[c,x]") ==
      V7_OK);
  ASSERT(check_value(v7, v, "[0,0]"));

  ASSERT(v7_exec(v7, &v, "Object.keys(new Boolean(1))") == V7_OK);
  ASSERT(check_value(v7, v, "[]"));
  ASSERT(v7_exec(v7, &v, "b={c:1};a=Object.create(b); a.d=4;Object.keys(a)") ==
         V7_OK);
  s = "[\"d\"]";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v, "Object.getOwnPropertyNames(new Boolean(1))") ==
         V7_OK);
  ASSERT(check_value(v7, v, "[]"));
  ASSERT(
      v7_exec(
          v7, &v,
          "b={c:1};a=Object.create(b); a.d=4;Object.getOwnPropertyNames(a)") ==
      V7_OK);
  ASSERT(check_value(v7, v, s));
  s = "o={};Object.defineProperty(o, \"x\", {value:2});[o.x,o]";
  ASSERT(v7_exec(v7, &v, s) == V7_OK);
  ASSERT(check_value(v7, v, "[2,{}]"));
  ASSERT(v7_exec(v7, &v,
                 "o={};Object.defineProperties(o,{x:{value:2},y:{value:3,"
                 "enumerable:true}});[o.x,o.y,o]") == V7_OK);
  s = "[2,3,{\"y\":3}]";
  ASSERT(check_value(v7, v, s));
  s = "o={};Object.defineProperty(o, \"x\", {value:2,enumerable:true});[o.x,o]";
  ASSERT(v7_exec(v7, &v, s) == V7_OK);
  s = "[2,{\"x\":2}]";
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v,
                 "o={};Object.defineProperty(o,'a',{value:1});o."
                 "propertyIsEnumerable('a')") == V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v,
                 "o={};Object.defineProperty(o,'a',{value:1,enumerable:true});"
                 "o.propertyIsEnumerable('a')") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v, "o={a:1};o.propertyIsEnumerable('a')") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v,
                 "b={a:1};o=Object.create(b);o.propertyIsEnumerable('a')") ==
         V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "b={a:1};o=Object.create(b);o.hasOwnProperty('a')") ==
         V7_OK);
  ASSERT(check_value(v7, v, "false"));
  ASSERT(v7_exec(v7, &v, "o={a:1};o.hasOwnProperty('a')") == V7_OK);
  ASSERT(check_value(v7, v, "true"));
  ASSERT(v7_exec(v7, &v,
                 "o={a:1};d=Object.getOwnPropertyDescriptor(o, 'a');"
                 "[d.value,d.writable,d.enumerable,d.configurable]") == V7_OK);
  ASSERT(check_value(v7, v, "[1,true,true,true]"));
  ASSERT(v7_exec(v7, &v,
                 "o={};Object.defineProperty(o,'a',{value:1,enumerable:true});"
                 "d=Object.getOwnPropertyDescriptor(o, 'a');"
                 "[d.value,d.writable,d.enumerable,d.configurable]") == V7_OK);
  ASSERT(check_value(v7, v, "[1,false,true,false]"));
  ASSERT(v7_exec(v7, &v,
                 "o=Object.defineProperty({},'a',{value:1,enumerable:true});o."
                 "a=2;o.a") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v,
                 "o=Object.defineProperty({},'a',{value:1,enumerable:true});r="
                 "delete o.a;[r,o.a]") == V7_OK);
  ASSERT(check_value(v7, v, "[false,1]"));

  ASSERT(v7_exec(v7, &v, "r=0;o={a:1,b:2};for(i in o){r+=o[i]};r") == V7_OK);
  ASSERT(check_value(v7, v, "3"));
  ASSERT(v7_exec(v7, &v, "r=0;o={a:1,b:2};for(var i in o){r+=o[i]};r") ==
         V7_OK);
  ASSERT(check_value(v7, v, "3"));
  ASSERT(v7_exec(v7, &v, "r=1;for(var i in null){r=0};r") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "r=1;for(var i in undefined){r=0};r") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "r=1;for(var i in 42){r=0};r") == V7_OK);
  ASSERT(check_value(v7, v, "1"));

  ASSERT(v7_exec_with(v7, &v, "this", v7_create_number(42)) == V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec_with(v7, &v, "a=666;(function(a){return a})(this)",
                      v7_create_number(42)) == V7_OK);
  ASSERT(check_value(v7, v, "42"));

  ASSERT(
      v7_exec(v7, &v, "a='aa', b='bb';(function(){return a + ' ' + b;})()") ==
      V7_OK);
  s = "\"aa bb\"";
  ASSERT(check_value(v7, v, s));

  s = "{\"fall\":2,\"one\":1}";
  ASSERT(v7_exec(v7, &v,
                 "o={};switch(1) {case 1: o.one=1; case 2: o.fall=2; break; "
                 "case 3: o.three=1; };o") == V7_OK);
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v,
                 "o={};for(i=0;i<1;i++) switch(1) {case 1: o.one=1; case 2: "
                 "o.fall=2; continue; case 3: o.three=1; };o") == V7_OK);
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v,
                 "(function(){o={};switch(1) {case 1: o.one=1; case 2: "
                 "o.fall=2; return o; case 3: o.three=1; }})()") == V7_OK);
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v,
                 "o={};switch(1) {case 1: o.one=1; default: o.fall=2; break; "
                 "case 3: o.three=1; };o") == V7_OK);
  ASSERT(check_value(v7, v, s));
  ASSERT(v7_exec(v7, &v,
                 "o={};switch(10) {case 1: o.one=1; case 2: o.fall=2; break; "
                 "case 3: o.three=1; break; default: o.def=1};o") == V7_OK);
  s = "{\"def\":1}";
  ASSERT(check_value(v7, v, s));

  ASSERT(v7_exec(v7, &v, "o={get x(){return 42}};o.x") == V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "o={set x(a){this.y=a}};o.x=42;o.y") == V7_OK);
  ASSERT(check_value(v7, v, "42"));
  ASSERT(v7_exec(v7, &v, "o={get x(){return 10},set x(v){}};o.x") == V7_OK);
  ASSERT(check_value(v7, v, "10"));
  ASSERT(v7_exec(v7, &v, "o={set x(v){},get x(){return 10}};o.x") == V7_OK);
  ASSERT(check_value(v7, v, "10"));
  ASSERT(
      v7_exec(v7, &v, "r=0;o={get x() {return 10}, set x(v){r=v}};o.x=10;r") ==
      V7_OK);
  ASSERT(check_value(v7, v, "10"));
  ASSERT(v7_exec(v7, &v,
                 "g=0;function O() {}; O.prototype = {set x(v) {g=v}};o=new "
                 "O;o.x=42;[g,Object.keys(o)]") == V7_OK);
  ASSERT(check_value(v7, v, "[42,[]]"));

  ASSERT(v7_exec(v7, &v, "String(new Number(42))") == V7_OK);
  s = "\"42\"";
  ASSERT(check_value(v7, v, s));

  ASSERT(
      v7_exec(v7, &v,
              "L: for(i=0;i<10;i++){for(j=4;j<10;j++){if(i==j) break L}};i") ==
      V7_OK);
  ASSERT(check_value(v7, v, "4"));
  ASSERT(v7_exec(
             v7, &v,
             "L: for(i=0;i<10;i++){M:for(j=4;j<10;j++){if(i==j) break L}};i") ==
         V7_OK);
  ASSERT(check_value(v7, v, "4"));
  ASSERT(v7_exec(v7, &v,
                 "x=0;L: for(i=0;i<10;i++){try{for(j=4;j<10;j++){if(i==j) "
                 "break L}}finally{x++}};x") == V7_OK);
  ASSERT(check_value(v7, v, "5"));
  ASSERT(v7_exec(v7, &v,
                 "x=0;L: for(i=0;i<11;i++) {if(i==5) continue L; x+=i}; x") ==
         V7_OK);
  ASSERT(check_value(v7, v, "50"));
  ASSERT(
      v7_exec(
          v7, &v,
          "x=0;L: if(true) for(i=0;i<11;i++) {if(i==5) continue L; x+=i}; x") ==
      V7_OK);
  ASSERT(check_value(v7, v, "50"));
  ASSERT(
      v7_exec(
          v7, &v,
          "x=0;L: if(true) for(i=0;i<11;i++) {if(i==5) continue L; x+=i}; x") ==
      V7_OK);
  ASSERT(check_value(v7, v, "50"));
  ASSERT(v7_exec(v7, &v, "L:do {i=0;continue L;}while(i>0);i") == V7_OK);
  ASSERT(check_value(v7, v, "0"));
  ASSERT(v7_exec(v7, &v, "i=1; L:while(i>0){i=0;continue L;};i") == V7_OK);
  ASSERT(check_value(v7, v, "0"));

  ASSERT(v7_exec(v7, &v, "1 | NaN") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "NaN | 1") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "NaN | NaN") == V7_OK);
  ASSERT(check_value(v7, v, "0"));

  ASSERT(v7_exec(v7, &v, "0 || 1") == V7_OK);
  ASSERT(check_value(v7, v, "1"));
  ASSERT(v7_exec(v7, &v, "0 || {}") == V7_OK);
  ASSERT(check_value(v7, v, "{}"));
  ASSERT(v7_exec(v7, &v, "1 && 0") == V7_OK);
  ASSERT(check_value(v7, v, "0"));
  ASSERT(v7_exec(v7, &v, "1 && {}") == V7_OK);
  ASSERT(check_value(v7, v, "{}"));
  ASSERT(v7_exec(v7, &v, "'' && {}") == V7_OK);
  s = "\"\"";
  ASSERT(check_value(v7, v, s));

  ASSERT(v7_exec_with(v7, &v, "a=this;a", v7_create_foreign((void *) "foo")) ==
         V7_OK);
  ASSERT(v7_is_foreign(v));
  ASSERT(strcmp((char *) v7_to_foreign(v), "foo") == 0);

  ASSERT(v7_exec(v7, &v, "a=[1,2,3];a.splice(0,1);a") == V7_OK);
  ASSERT(check_value(v7, v, "[2,3]"));
  ASSERT(v7_exec(v7, &v, "a=[1,2,3];a.splice(2,1);a") == V7_OK);
  ASSERT(check_value(v7, v, "[1,2]"));
  ASSERT(v7_exec(v7, &v, "a=[1,2,3];a.splice(1,1);a") == V7_OK);
  ASSERT(check_value(v7, v, "[1,3]"));

  ASSERT(v7_exec(v7, &v, "a=[1,2,3];a.slice(0,1)") == V7_OK);
  ASSERT(check_value(v7, v, "[1]"));
  ASSERT(v7_exec(v7, &v, "a=[1,2,3];a.slice(2,3)") == V7_OK);
  ASSERT(check_value(v7, v, "[3]"));
  ASSERT(v7_exec(v7, &v, "a=[1,2,3];a.slice(1,3)") == V7_OK);
  ASSERT(check_value(v7, v, "[2,3]"));

  /* here temporarily because test_stdlib has memory violations */
  ASSERT(v7_exec(v7, &v, "a=[2,1];a.sort();a") == V7_OK);
  ASSERT(check_value(v7, v, "[1,2]"));

  /* check execution failure caused by bad parsing */
  ASSERT(v7_exec(v7, &v, "function") == V7_SYNTAX_ERROR);

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
  ASSERT(memcmp(&s, "\x02\x68\x69\x00\x00\x00\xfa\xff", sizeof(s)) == 0);
  ASSERT(v7->foreign_strings.len == 0);
  ASSERT(v7->owned_strings.len == off);

  /* Make sure strings with length 5 & 6 are nan-packed */
  s = v7_create_string(v7, "length", 4, 1);
  ASSERT(v7->owned_strings.len == off);
  s = v7_create_string(v7, "length", 5, 1);
  ASSERT(v7->owned_strings.len == off);
  ASSERT(memcmp(&s, "\x6c\x65\x6e\x67\x74\x00\xf9\xff", sizeof(s)) == 0);

  s = v7_create_string(v7, "longer one", 10, 1);
  ASSERT(v7->owned_strings.len == off + 12);
  ASSERT(memcmp(v7->owned_strings.buf + off, "\x0alonger one\x00", 12) == 0);

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

  ASSERT(v7_exec(v7, &v, "'1234'") == V7_OK);
  ASSERT((v & V7_TAG_MASK) == V7_TAG_STRING_I);
  ASSERT(v7_exec(v7, &v, "'12345'") == V7_OK);
  ASSERT((v & V7_TAG_MASK) == V7_TAG_STRING_5);
  ASSERT(v7_exec(v7, &v, "'123456'") == V7_OK);
  ASSERT((v & V7_TAG_MASK) == V7_TAG_STRING_O);

  ASSERT(v7_exec(v7, &v, "'123'.length") == V7_OK);
  ASSERT(check_num(v, 3));
  ASSERT(v7_exec(v7, &v, "'123\\n'.length") == V7_OK);
  ASSERT(check_num(v, 4));
  ASSERT(v7_exec(v7, &v, "'123\\n\\n'.length") == V7_OK);
  ASSERT(check_num(v, 5));
  ASSERT(v7_exec(v7, &v, "'123\\n\\n\\n'.length") == V7_OK);
  ASSERT(check_num(v, 6));
  ASSERT(v7_exec(v7, &v, "'123\\n\\n\\n\\n'.length") == V7_OK);
  ASSERT(check_num(v, 7));
  ASSERT(v7_exec(v7, &v, "'123\\n\\n\\n\\n\\n'.length") == V7_OK);
  ASSERT(check_num(v, 8));

  ASSERT(v7_exec(v7, &v, "'123\\\\\\\\'.length == '1234\\\\\\\\'.length") ==
         V7_OK);
  ASSERT(check_value(v7, v, "false"));

  ASSERT(v7_exec(v7, &v, "'123'.length") == V7_OK);
  ASSERT(check_num(v, 3));
  ASSERT(v7_exec(v7, &v, "'123\\\\'.length") == V7_OK);
  ASSERT(check_num(v, 4));
  ASSERT(v7_exec(v7, &v, "'123\\\\\\\\'.length") == V7_OK);
  ASSERT(check_num(v, 5));
  ASSERT(v7_exec(v7, &v, "'123\\\\\\\\\\\\'.length") == V7_OK);
  ASSERT(check_num(v, 6));
  ASSERT(v7_exec(v7, &v, "'123\\\\\\\\\\\\\\\\'.length") == V7_OK);
  ASSERT(check_num(v, 7));
  ASSERT(v7_exec(v7, &v, "'123\\\\\\\\\\\\\\\\\\\\'.length") == V7_OK);
  ASSERT(check_num(v, 8));

  ASSERT(v7_exec(v7, &v, "'1234\\\\\\\\'") == V7_OK);
  ASSERT((v & V7_TAG_MASK) == V7_TAG_STRING_O);

  v7_destroy(v7);
  return NULL;
}

static const char *test_to_json(void) {
  char buf[10], *p;
  struct v7 *v7 = v7_create();
  val_t v;

  v7_exec(v7, &v, "123.45");
  ASSERT((p = v7_to_json(v7, v, buf, sizeof(buf))) == buf);
  ASSERT(strcmp(p, "123.45") == 0);
/* TODO(mkm): fix to_json alloc */
#if 0
  ASSERT((p = v7_to_json(v7, v, buf, 3)) != buf);
  ASSERT(strcmp(p, "123.45") == 0);
  free(p);
#endif

  v7_destroy(v7);
  return NULL;
}

static const char *test_unescape(void) {
  char buf[100];
  ASSERT(unescape("\\n", 2, buf) == 1);
  ASSERT(buf[0] == '\n');
  ASSERT(unescape("\\u0061", 6, buf) == 1);
  ASSERT(buf[0] == 'a');
  ASSERT(unescape("гы", 4, buf) == 4);
  ASSERT(memcmp(buf, "\xd0\xb3\xd1\x8b", 4) == 0);
  ASSERT(unescape("\\\"", 2, buf) == 1);
  ASSERT(memcmp(buf, "\"", 1) == 0);
  ASSERT(unescape("\\'", 2, buf) == 1);
  ASSERT(memcmp(buf, "'", 1) == 0);
  ASSERT(unescape("\\\n", 2, buf) == 1);
  ASSERT(memcmp(buf, "\n", 1) == 0);
  return NULL;
}

#ifndef V7_DISABLE_GC
static const char *test_gc_mark(void) {
  struct v7 *v7 = v7_create();
  val_t v;

  v7_exec(v7, &v, "o=({a:{b:1},c:{d:2},e:null});o.e=o;o");
  gc_mark(v7, v);
  ASSERT(MARKED(v7_to_object(v)));
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v, "o=({a:{b:1},c:{d:2},e:null});o.e=o;o");
  gc_mark(v7, v7->global_object);
  ASSERT(MARKED(v7_to_object(v)));
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v, "function f() {}; o=new f;o");
  gc_mark(v7, v);
  ASSERT(MARKED(v7_to_object(v)));
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v, "function f() {}; Object.getPrototypeOf(new f)");
  gc_mark(v7, v7->global_object);
  ASSERT(MARKED(v7_to_object(v)));
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v, "({a:1})");
  gc_mark(v7, v7->global_object);
  ASSERT(!MARKED(v7_to_object(v)));
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v,
          "var f;(function() {var x={a:1};f=function(){return x};return x})()");
  gc_mark(v7, v7->global_object);
  /* `x` is reachable through `f`'s closure scope */
  ASSERT(MARKED(v7_to_object(v)));
  v7_destroy(v7);
  v7 = v7_create();

  v7_exec(v7, &v,
          "(function() {var x={a:1};var f=function(){return x};return x})()");
  gc_mark(v7, v7->global_object);
  /* `f` is unreachable, hence `x` is not marked through the scope */
  ASSERT(!MARKED(v7_to_object(v)));
  v7_destroy(v7);
  v7 = v7_create();

  v7_destroy(v7);
  return NULL;
}

static const char *test_gc_sweep(void) {
  struct v7 *v7 = v7_create();
  val_t v;
  uint32_t alive;

  v7_gc(v7);
  alive = v7->object_arena.alive;
  v7_exec(v7, &v, "x=({a:1})");
  v7_to_object(v);
  v7_gc(v7);
  ASSERT(v7->object_arena.alive > alive);
  v7_exec(v7, &v, "x.a");
  ASSERT(check_value(v7, v, "1"));

  v7_exec(v7, &v, "x=null");
  v7_gc(v7);
  ASSERT(v7->object_arena.alive == alive);
  v7_destroy(v7);

  v7 = v7_create();
  v7_gc(v7);
  v7_exec(
      v7, &v,
      "for(i=0;i<9;i++)({});for(i=0;i<7;i++){x=(new Number(1))+({} && 1)};x");
  ASSERT(check_value(v7, v, "2"));
  v7_gc(v7);

  v7_destroy(v7);
  return NULL;
}
#endif

static const char *test_file(void) {
  v7_val_t v;
  size_t file_len, string_len;
  char *file_data = read_file("unit_test.c", &file_len);
  const char *s;
  struct v7 *v7 = v7_create();
  char buf[100];

  /* Read file in C and Javascript, then compare respective strings */
  ASSERT(v7_exec(v7, &v, "var fd = File.open('unit_test.c')") == V7_OK);
  ASSERT(v7_exec(v7, &v,
                 "var a = '', b; while ((b = File.read(fd)) != '') "
                 "{ a += b; }; a") == V7_OK);
  s = v7_to_string(v7, &v, &string_len);
  ASSERT(string_len == file_len);
  ASSERT(memcmp(s, file_data, string_len) == 0);
  free(file_data);
  ASSERT(v7_exec(v7, &v, "File.close(fd)") == V7_OK);
  ASSERT(check_value(v7, v, "0"));

  /* Create file, write into it, then remove it. 0x202 is O_RDWR | O_CREAT */
  snprintf(buf, sizeof(buf), "fd = File.open('foo.txt', %d, %d);",
           O_RDWR | O_CREAT, 0644);
  ASSERT(v7_exec(v7, &v, buf) == V7_OK);
  ASSERT(v7_exec(v7, &v, "File.write(fd, 'hi there');") == V7_OK);
  ASSERT(check_value(v7, v, "0"));
  ASSERT(v7_exec(v7, &v, "File.close(fd)") == V7_OK);
  ASSERT(check_value(v7, v, "0"));
  ASSERT((file_data = read_file("foo.txt", &file_len)) != NULL);
  ASSERT(file_len == 8);
  ASSERT(memcmp(file_data, "hi there", 8) == 0);
  free(file_data);
  ASSERT(v7_exec(v7, &v, "File.remove('foo.txt')") == V7_OK);
  ASSERT(check_value(v7, v, "0"));
  ASSERT(fopen("foo.txt", "r") == NULL);

  v7_destroy(v7);
  return NULL;
}

static const char *run_all_tests(const char *filter) {
  RUN_TEST(test_unescape);
  RUN_TEST(test_to_json);
  RUN_TEST(test_tokenizer);
  RUN_TEST(test_string_encoding);
  RUN_TEST(test_is_true);
  RUN_TEST(test_closure);
  RUN_TEST(test_native_functions);
  RUN_TEST(test_file);
  RUN_TEST(test_stdlib);
  RUN_TEST(test_runtime);
  RUN_TEST(test_parser);
  RUN_TEST(test_interpreter);
  RUN_TEST(test_interp_unescape);
  RUN_TEST(test_strings);
  RUN_TEST(test_dense_arrays);
  RUN_TEST(test_ecmac);
#ifndef V7_DISABLE_GC
  RUN_TEST(test_gc_mark);
  RUN_TEST(test_gc_sweep);
#endif
  return NULL;
}

int main(int argc, char *argv[]) {
  const char *filter = argc > 1 ? argv[1] : "";
  const char *fail_msg = run_all_tests(filter);
  printf("%s, tests run: %d\n", fail_msg ? "FAIL" : "PASS", static_num_tests);
  return fail_msg == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
