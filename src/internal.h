// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013-2014 Cesanta Software Limited
// All rights reserved
//
// This software is dual-licensed: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation. For the terms of this
// license, see <http://www.gnu.org/licenses/>.
//
// You are free to use this software under the terms of the GNU General
// Public License, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// Alternatively, you can license this software under a commercial
// license, as set out in <http://cesanta.com/products.html>.

#include "v7.h"

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#define __unused
#else
#include <stdint.h>
#endif

// MSVC6 doesn't have standard C math constants defined
#ifndef M_PI
#define M_E         2.71828182845904523536028747135266250
#define M_LOG2E     1.44269504088896340735992468100189214
#define M_LOG10E    0.434294481903251827651128918916605082
#define M_LN2       0.693147180559945309417232121458176568
#define M_LN10      2.30258509299404568401799145468436421
#define M_PI        3.14159265358979323846264338327950288
#define M_SQRT2     1.41421356237309504880168872420969808
#define M_SQRT1_2   0.707106781186547524400844362104849039
#define NAN         atof("NAN")
#define INFINITY    atof("INFINITY")  // TODO: fix this
#endif

//#define V7_CACHE_OBJS
#define MAX_STRING_LITERAL_LENGTH 2000
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define CHECK(cond, code) do { if (!(cond)) return (code); } while (0)
#define TRY(call) do { enum v7_err e = call; CHECK(e == V7_OK, e); } while (0)
#define TRACE_OBJ(O) do { char x[4000]; printf("==> %s [%s]\n", __func__, \
  O == NULL ? "@" : v7_to_string(O, x, sizeof(x))); } while (0)
#define MKOBJ(_proto) {0,(_proto),0,0,{0},V7_TYPE_OBJ,V7_CLASS_OBJECT,0,0}

#define SET_RO_PROP_V(obj, name, val) \
  do { \
    static struct v7_val key = MKOBJ(&s_prototypes[V7_CLASS_STRING]); \
    static struct v7_prop prop = { NULL, &key, &val, 0 }; \
    v7_init_str(&key, (char *) (name), strlen(name), 0); \
    prop.next = obj.props; \
    obj.props = &prop; \
  } while (0)

#define SET_RO_PROP2(_o, _name, _t, _proto, _attr, _initializer, _fl) \
  do { \
    static struct v7_val _val = MKOBJ(_proto); \
    _val.v._attr = (_initializer); \
    _val.type = (_t); \
    _val.flags = (_fl); \
    SET_RO_PROP_V(_o, _name, _val); \
  } while (0)

#define SET_RO_PROP(obj, name, _t, attr, _v) \
    SET_RO_PROP2(obj, name, _t, &s_prototypes[V7_CLASS_OBJECT], attr, _v, 0)

#define SET_PROP_FUNC(_obj, _name, _func) \
    SET_RO_PROP2(_obj, _name, V7_TYPE_NULL, 0, prop_func, _func, V7_PROP_FUNC)

#define SET_METHOD(_obj, _name, _func) \
  do {  \
    static struct v7_val _val = MKOBJ(&s_prototypes[V7_CLASS_STRING]); \
    v7_set_class(&_val, V7_CLASS_FUNCTION); \
    _val.v.c_func = (_func); \
    SET_RO_PROP_V(_obj, _name, _val); \
  } while (0)

enum {
  OP_INVALID,

  // Relational ops
  OP_GREATER_THEN,    //  >
  OP_LESS_THEN,       //  <
  OP_GREATER_EQUAL,   //  >=
  OP_LESS_EQUAL,      //  <=

  // Equality ops
  OP_EQUAL,           //  ==
  OP_NOT_EQUAL,       //  !=
  OP_EQUAL_EQUAL,     //  ===
  OP_NOT_EQUAL_EQUAL, //  !==

  // Assignment ops
  OP_ASSIGN,          //  =
  OP_PLUS_ASSIGN,     //  +=
  OP_MINUS_ASSIGN,    //  -=
  OP_MUL_ASSIGN,      //  *=
  OP_DIV_ASSIGN,      //  /=
  OP_REM_ASSIGN,      //  %=
  OP_AND_ASSIGN,      //  &=
  OP_XOR_ASSIGN,      //  ^=
  OP_OR_ASSIGN,       //  |=
  OP_RSHIFT_ASSIGN,   //  >>=
  OP_LSHIFT_ASSIGN,   //  <<=
  OP_RRSHIFT_ASSIGN,  //  >>>=

  NUM_OPS
};

// 
typedef unsigned long uint64_t;
typedef unsigned long u_int64_t;
typedef unsigned int uint32_t;
typedef unsigned int u_int32_t;
typedef unsigned char uint8_t;
typedef unsigned char u_int8_t;

#define bcopy memcpy

// Forward declarations
static enum v7_err parse_expression(struct v7 *);
static enum v7_err parse_statement(struct v7 *, int *is_return);
static int cmp(const struct v7_val *a, const struct v7_val *b);
static enum v7_err do_exec(struct v7 *v7, const char *, int);
static void init_stdlib(void);
static void skip_whitespaces_and_comments(struct v7 *v7);
