/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 * This software is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively, you can license this software under a commercial
 * license, as set out in <http://cesanta.com/>.
 */

#ifndef NS_TEST_UTIL_HEADER_INCLUDED
#define NS_TEST_UTIL_HEADER_INCLUDED

#include <string.h>
#include <stdio.h>

extern int num_tests;

#ifdef NS_TEST_ABORT_ON_FAIL
#define NS_TEST_ABORT abort()
#else
#define NS_TEST_ABORT
#endif

int _assert_streq(const char *actual, const char *expected);
int _assert_streq_nz(const char *actual, const char *expected);
void _strfail(const char *a, const char *e, int len);
double _now();

#define FAIL(str, line)                                              \
  do {                                                               \
    printf("%s:%d:1 [%s] (in %s)\n", __FILE__, line, str, __func__); \
    NS_TEST_ABORT;                                                   \
    return str;                                                      \
  } while (0)

#define ASSERT(expr)                    \
  do {                                  \
    num_tests++;                        \
    if (!(expr)) FAIL(#expr, __LINE__); \
  } while (0)

#define RUN_TEST(test)                          \
  do {                                          \
    const char *msg = NULL;                     \
    if (strstr(#test, filter)) {                \
      double elapsed = _now();                  \
      msg = test();                             \
      elapsed = _now() - elapsed;               \
      printf("  [%.3f] %s\n", elapsed, #test); \
      *total_elapsed += elapsed;                \
    }                                           \
    if (msg) return msg;                        \
  } while (0)

/* VC6 doesn't know how to cast an unsigned 64-bit int to double */
#if (defined(_MSC_VER) && _MSC_VER <= 1200)
#define AS_DOUBLE(d) (double)(int64_t)(d)
#else
#define AS_DOUBLE(d) (double)(d)
#endif

/*
 * Numeric equality assertion. Compariosn is made in native types but for
 * printing both are convetrted to double.
 */
#define ASSERT_EQ(actual, expected)                                   \
  do {                                                                \
    num_tests++;                                                      \
    if (!(actual == expected)) {                                      \
      printf("%f != %f\n", AS_DOUBLE(actual), AS_DOUBLE(expected)); \
      FAIL(#actual " == " #expected, __LINE__);                       \
    }                                                                 \
  } while (0)

/* Assert that actual == expected, where both are NUL-terminated. */
#define ASSERT_STREQ(actual, expected)                            \
  do {                                                            \
    num_tests++;                                                  \
    if (!_assert_streq(actual, expected)) {                       \
      FAIL("ASSERT_STREQ(" #actual ", " #expected ")", __LINE__); \
    }                                                             \
  } while (0)

/* Same as STREQ, but only expected is NUL-terminated. */
#define ASSERT_STREQ_NZ(actual, expected)                            \
  do {                                                               \
    num_tests++;                                                     \
    if (!_assert_streq_nz(actual, expected)) {                       \
      FAIL("ASSERT_STREQ_NZ(" #actual ", " #expected ")", __LINE__); \
    }                                                                \
  } while (0)

#endif /* NS_TEST_UTIL_HEADER_INCLUDED */
