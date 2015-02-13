/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_INTERNAL_H_INCLUDED
#define V7_INTERNAL_H_INCLUDED

#include "license.h"

/* Check whether we're compiling in an environment with no filesystem */
#if defined(ARDUINO) && (ARDUINO == 106)
#define V7_NO_FS
#endif

#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#ifdef _WIN32
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#define __unused
typedef unsigned __int64 uint64_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
char *stpncpy(char *, const char *, size_t);
#else
#include <stdint.h>
#endif

/* Public API. Implemented in api.c */
#include "../v7.h"

/* Private API */
#include "utf.h"
#include "mbuf.h"
#include "tokenizer.h"
#include "slre.h"
#include "varint.h"
#include "ast.h"
#include "parser.h"

/* Max captures for String.replace() */
#define V7_RE_MAX_REPL_SUB 255

/* MSVC6 doesn't have standard C math constants defined */
#ifndef M_E
#define M_E 2.71828182845904523536028747135266250
#endif

#ifndef M_LOG2E
#define M_LOG2E 1.44269504088896340735992468100189214
#endif

#ifndef M_LOG10E
#define M_LOG10E 0.434294481903251827651128918916605082
#endif

#ifndef M_LN2
#define M_LN2 0.693147180559945309417232121458176568
#endif

#ifndef M_LN10
#define M_LN10 2.30258509299404568401799145468436421
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880168872420969808
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2 0.707106781186547524400844362104849039
#endif

#ifndef NAN
#define NAN atof("NAN")
#endif

#ifndef INFINITY
#define INFINITY atof("INFINITY") /* TODO: fix this */
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

/* TODO(lsm): move VM definitions to vm.h */
#ifndef VM_H_INCLUDED
#define V7_VALUE_DEFINED
typedef uint64_t val_t;
#endif

/*
 * JavaScript value is either a primitive, or an object.
 * There are 5 primitive types: Undefined, Null, Boolean, Number, String.
 * Non-primitive type is an Object type. There are several classes of Objects,
 * see description of `struct v7_object` below for more details.
 * This enumeration combines types and object classes in one enumeration.
 * NOTE(lsm): compile with `-fshort-enums` to reduce sizeof(enum v7_type) to 1.
 */
enum v7_type {
  /* Primitive types */
  V7_TYPE_UNDEFINED,
  V7_TYPE_NULL,
  V7_TYPE_BOOLEAN,
  V7_TYPE_NUMBER,
  V7_TYPE_STRING,
  V7_TYPE_FOREIGN,
  V7_TYPE_CFUNCTION,

  /* Different classes of Object type */
  V7_TYPE_GENERIC_OBJECT,
  V7_TYPE_BOOLEAN_OBJECT,
  V7_TYPE_STRING_OBJECT,
  V7_TYPE_NUMBER_OBJECT,
  V7_TYPE_FUNCTION_OBJECT,
  V7_TYPE_CFUNCTION_OBJECT,
  V7_TYPE_REGEXP_OBJECT,
  V7_TYPE_ARRAY_OBJECT,
  V7_TYPE_DATE_OBJECT,
  V7_TYPE_ERROR_OBJECT,
  V7_TYPE_MAX_OBJECT_TYPE,

  V7_NUM_TYPES
};

#include "vm.h"

struct v7 {
  val_t global_object;
  val_t object_prototype;
  val_t array_prototype;
  val_t boolean_prototype;
  val_t error_prototype;
  val_t string_prototype;
  val_t number_prototype;
  val_t cfunction_prototype;
  val_t this_object;

  /*
   * Stack of execution contexts.
   * Each execution context object in the call stack has hidden properties:
   *  *  "_p": Parent context (for closures)
   *  *  "_e": Exception environment
   *
   * Hidden properties have V7_PROPERTY_HIDDEN flag set.
   * Execution contexts should be allocated on heap, because they might not be
   * on a call stack but still referenced (closures).
   */
  val_t call_stack;

  struct mbuf owned_strings;    /* Sequence of (varint len, char data[]) */
  struct mbuf foreign_strings;  /* Sequence of (varint len, char *data) */

  int strict_mode;  /* true if currently in strict mode */

#if defined(__cplusplus)
  ::jmp_buf jmp_buf;
  ::jmp_buf abort_jmp_buf;
#else
  jmp_buf jmp_buf;              /* Exception environment for v7_exec() */
  /* Handle implementation errors that shouldn't be caught from JS */
  jmp_buf abort_jmp_buf;
#endif
  val_t thrown_error;
  char error_msg[60];           /* Exception message */
  int creating_exception;  /* Avoids reentrant exception creation */

#if defined(__cplusplus)
  ::jmp_buf label_jmp_buf;
#else
  jmp_buf label_jmp_buf;  /* Target for non local (labeled) breaks */
#endif
  char *label;            /* Inner label */
  size_t label_len;       /* Inner label length */
  int lab_cont; /* True if re-entering a loop with labeled continue */

  struct mbuf json_visited_stack;  /* Detecting cycle in to_json */

  /* Parser state */
  struct v7_pstate pstate; /* Parsing state */
  enum v7_tok cur_tok;     /* Current token */
  const char *tok;         /* Parsed terminal token (ident, number, string) */
  unsigned long tok_len;   /* Length of the parsed terminal token */
  size_t last_var_node;    /* Offset of last var node or function/script node */
  int after_newline;       /* True if the cur_tok starts a new line */
  double cur_tok_dbl;      /* When tokenizing, parser stores TOK_NUMBER here */

  /* TODO(mkm): remove when AST are GC-ed */
  struct ast *last_ast; /* AST generated by last v7_exec invocation */
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

#define V7_STATIC_ASSERT(COND, MSG) \
      typedef char static_assertion_##MSG[2*(!!(COND)) - 1]

#define V7_CHECK(v7, COND)                                              \
  do { if (!(COND))                                                     \
      throw_exception(v7, "InternalError", "%s line %d: %s",            \
                      __func__, __LINE__, #COND);                       \
  } while (0)

#define TRACE_VAL(v7, val)                                              \
  do {                                                                  \
    char buf[200], *p = v7_to_json(v7, val, buf, sizeof(buf));          \
    printf("%s %d: [%s]\n", __func__, __LINE__, p);                     \
    if (p != buf) free(p);                                              \
  } while (0)


#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

V7_PRIVATE void throw_value(struct v7 *, val_t);
V7_PRIVATE void throw_exception(struct v7 *, const char *, const char *, ...);
V7_PRIVATE size_t unescape(const char *s, size_t len, char *to);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif /* V7_INTERNAL_H_INCLUDED */
