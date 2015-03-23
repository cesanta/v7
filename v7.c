/*
 * Copyright (c) 2013-2014 Cesanta Software Limited
 * All rights reserved
 *
 * This software is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http: *www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively, you can license this software under a commercial
 * license, as set out in <http://cesanta.com/products.html>.
 */

#ifndef V7_HEADER_INCLUDED
#define V7_HEADER_INCLUDED

#define _POSIX_C_SOURCE 200809L

#include <stddef.h> /* For size_t */

#define V7_VERSION "1.0"

enum v7_err { V7_OK, V7_SYNTAX_ERROR, V7_EXEC_EXCEPTION };

struct v7;     /* Opaque structure. V7 engine handler. */
struct v7_val; /* Opaque structure. Holds V7 value, which has v7_type type. */

#if (defined(_WIN32) && !defined(__MINGW32__) && !defined(__MINGW64__)) || (defined(_MSC_VER) && _MSC_VER <= 1200)
#define V7_WINDOWS
#endif

#ifdef V7_WINDOWS
typedef unsigned __int64 uint64_t;
#else
#include <inttypes.h>
#endif
typedef uint64_t v7_val_t;

typedef v7_val_t (*v7_cfunction_t)(struct v7 *, v7_val_t, v7_val_t);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct v7 *v7_create(void);
void v7_destroy(struct v7 *);
enum v7_err v7_exec(struct v7 *, v7_val_t *, const char *str);
enum v7_err v7_exec_file(struct v7 *, v7_val_t *, const char *path);
enum v7_err v7_exec_with(struct v7 *, v7_val_t *, const char *str, v7_val_t);

void v7_gc(struct v7 *);

v7_val_t v7_create_object(struct v7 *v7);
v7_val_t v7_create_array(struct v7 *v7);
v7_val_t v7_create_cfunction(v7_cfunction_t func);
v7_val_t v7_create_number(double num);
v7_val_t v7_create_boolean(int is_true);
v7_val_t v7_create_null(void);
v7_val_t v7_create_undefined(void);
v7_val_t v7_create_string(struct v7 *v7, const char *, size_t, int);
v7_val_t v7_create_regexp(struct v7 *, const char *, size_t, const char *,
                          size_t);
v7_val_t v7_create_foreign(void *);

int v7_is_object(v7_val_t);
int v7_is_function(v7_val_t);
int v7_is_cfunction(v7_val_t);
int v7_is_string(v7_val_t);
int v7_is_boolean(v7_val_t);
int v7_is_double(v7_val_t);
int v7_is_null(v7_val_t);
int v7_is_undefined(v7_val_t);
int v7_is_regexp(v7_val_t);
int v7_is_foreign(v7_val_t);

void *v7_to_foreign(v7_val_t);
int v7_to_boolean(v7_val_t);
double v7_to_double(v7_val_t);
v7_cfunction_t v7_to_cfunction(v7_val_t);
const char *v7_to_string(struct v7 *, v7_val_t *, size_t *);

v7_val_t v7_get_global_object(struct v7 *);
v7_val_t v7_get(struct v7 *v7, v7_val_t obj, const char *name, size_t len);
int v7_set(struct v7 *v7, v7_val_t obj, const char *, size_t, v7_val_t val);
char *v7_to_json(struct v7 *, v7_val_t, char *, size_t);
int v7_is_true(struct v7 *v7, v7_val_t v);
v7_val_t v7_apply(struct v7 *, v7_val_t, v7_val_t, v7_val_t);

unsigned long v7_array_length(struct v7 *v7, v7_val_t arr);
int v7_array_set(struct v7 *v7, v7_val_t arr, unsigned long index, v7_val_t v);
int v7_array_push(struct v7 *, v7_val_t arr, v7_val_t v);
v7_val_t v7_array_get(struct v7 *, v7_val_t arr, unsigned long index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* V7_HEADER_INCLUDED */
/*
 * Copyright (c) 2013-2014 Cesanta Software Limited
 * All rights reserved
 *
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
 * license, as set out in <http://cesanta.com/products.html>.
 */

#ifdef V7_EXPOSE_PRIVATE
#define V7_PRIVATE
#define V7_EXTERN extern
#else
#define V7_PRIVATE static
#define V7_EXTERN static
#endif
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef _UTF_H_
#define _UTF_H_ 1

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

typedef unsigned char uchar;

typedef unsigned short Rune; /* 16 bits */

#define nelem(a) (sizeof(a) / sizeof(a)[0])

enum {
  UTFmax = 3,               /* maximum bytes per rune */
  Runesync = 0x80,          /* cannot represent part of a UTF sequence (<) */
  Runeself = 0x80,          /* rune and UTF sequences are the same (<) */
  Runeerror = 0xFFFD        /* decoding error in UTF */
  /* Runemax    = 0xFFFC */ /* maximum rune value */
};

/* Edit .+1,/^$/ | cfn $PLAN9/src/lib9/utf/?*.c | grep -v static |grep -v __ */
int chartorune(Rune *rune, const char *str);
int fullrune(char *str, int n);
int isdigitrune(Rune c);
int isnewline(Rune c);
int iswordchar(Rune c);
int isalpharune(Rune c);
int islowerrune(Rune c);
int isspacerune(Rune c);
int istitlerune(Rune c);
int isupperrune(Rune c);
int runelen(Rune c);
int runenlen(Rune *r, int nrune);
Rune *runestrcat(Rune *s1, Rune *s2);
Rune *runestrchr(Rune *s, Rune c);
int runestrcmp(Rune *s1, Rune *s2);
Rune *runestrcpy(Rune *s1, Rune *s2);
Rune *runestrdup(Rune *s);
Rune *runestrecpy(Rune *s1, Rune *es1, Rune *s2);
long runestrlen(Rune *s);
Rune *runestrncat(Rune *s1, Rune *s2, long n);
int runestrncmp(Rune *s1, Rune *s2, long n);
Rune *runestrncpy(Rune *s1, Rune *s2, long n);
Rune *runestrrchr(Rune *s, Rune c);
Rune *runestrstr(Rune *s1, Rune *s2);
int runetochar(char *str, Rune *rune);
Rune tolowerrune(Rune c);
Rune totitlerune(Rune c);
Rune toupperrune(Rune c);
char *utfecpy(char *to, char *e, char *from);
int utflen(char *s);
int utfnlen(char *s, long m);
char *utfnshift(char *s, long m);
char *utfrrune(char *s, long c);
char *utfrune(char *s, long c);
char *utfutf(char *s1, char *s2);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif /* _UTF_H_ */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_TOKENIZER_H_INCLUDED
#define V7_TOKENIZER_H_INCLUDED


enum v7_tok {
  TOK_END_OF_INPUT,
  TOK_NUMBER,
  TOK_STRING_LITERAL,
  TOK_REGEX_LITERAL,
  TOK_IDENTIFIER,

  /* Punctuators */
  TOK_OPEN_CURLY,
  TOK_CLOSE_CURLY,
  TOK_OPEN_PAREN,
  TOK_CLOSE_PAREN,
  TOK_COMMA,
  TOK_OPEN_BRACKET,
  TOK_CLOSE_BRACKET,
  TOK_DOT,
  TOK_COLON,
  TOK_SEMICOLON,

  /* Equality ops, in this order */
  TOK_EQ,
  TOK_EQ_EQ,
  TOK_NE,
  TOK_NE_NE,

  /* Assigns */
  TOK_ASSIGN,
  TOK_REM_ASSIGN,
  TOK_MUL_ASSIGN,
  TOK_DIV_ASSIGN,
  TOK_XOR_ASSIGN,
  TOK_PLUS_ASSIGN,
  TOK_MINUS_ASSIGN,
  TOK_OR_ASSIGN,
  TOK_AND_ASSIGN,
  TOK_LSHIFT_ASSIGN,
  TOK_RSHIFT_ASSIGN,
  TOK_URSHIFT_ASSIGN,
  TOK_AND,
  TOK_LOGICAL_OR,
  TOK_PLUS,
  TOK_MINUS,
  TOK_PLUS_PLUS,
  TOK_MINUS_MINUS,
  TOK_LOGICAL_AND,
  TOK_OR,
  TOK_QUESTION,
  TOK_TILDA,
  TOK_REM,
  TOK_MUL,
  TOK_DIV,
  TOK_XOR,

  /* Relational ops, must go in this order */
  TOK_LE,
  TOK_LT,
  TOK_GE,
  TOK_GT,
  TOK_LSHIFT,
  TOK_RSHIFT,
  TOK_URSHIFT,
  TOK_NOT,

  /* Keywords. must be in the same order as tokenizer.c::s_keywords array */
  TOK_BREAK,
  TOK_CASE,
  TOK_CATCH,
  TOK_CONTINUE,
  TOK_DEBUGGER,
  TOK_DEFAULT,
  TOK_DELETE,
  TOK_DO,
  TOK_ELSE,
  TOK_FALSE,
  TOK_FINALLY,
  TOK_FOR,
  TOK_FUNCTION,
  TOK_IF,
  TOK_IN,
  TOK_INSTANCEOF,
  TOK_NEW,
  TOK_NULL,
  TOK_RETURN,
  TOK_SWITCH,
  TOK_THIS,
  TOK_THROW,
  TOK_TRUE,
  TOK_TRY,
  TOK_TYPEOF,
  TOK_VAR,
  TOK_VOID,
  TOK_WHILE,
  TOK_WITH,

  /* TODO(lsm): process these reserved words too */
  TOK_CLASS,
  TOK_ENUM,
  TOK_EXTENDS,
  TOK_SUPER,
  TOK_CONST,
  TOK_EXPORT,
  TOK_IMPORT,
  TOK_IMPLEMENTS,
  TOK_LET,
  TOK_PRIVATE,
  TOK_PUBLIC,
  TOK_INTERFACE,
  TOK_PACKAGE,
  TOK_PROTECTED,
  TOK_STATIC,
  TOK_YIELD,

  NUM_TOKENS
};

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

V7_PRIVATE int skip_to_next_tok(const char **ptr);
V7_PRIVATE enum v7_tok get_tok(const char **s, double *n, enum v7_tok prev_tok);
V7_PRIVATE int is_reserved_word_token(enum v7_tok tok);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* V7_TOKENIZER_H_INCLUDED */
/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MBUF_H_INCLUDED
#define MBUF_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>

#ifndef MBUF_SIZE_MULTIPLIER
#define MBUF_SIZE_MULTIPLIER 1.5
#endif

struct mbuf {
  char *buf;
  size_t len;
  size_t size;
};

#ifdef V7_EXPOSE_PRIVATE
#define V7_PRIVATE
#define V7_EXTERN extern
#else
#define V7_PRIVATE static
#define V7_EXTERN static
#endif

V7_PRIVATE void mbuf_init(struct mbuf *, size_t);
V7_PRIVATE void mbuf_free(struct mbuf *);
V7_PRIVATE void mbuf_resize(struct mbuf *, size_t);
V7_PRIVATE void mbuf_trim(struct mbuf *);
V7_PRIVATE size_t mbuf_insert(struct mbuf *, size_t, const char *, size_t);
V7_PRIVATE size_t mbuf_append(struct mbuf *, const char *, size_t);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MBUF_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

enum ast_tag {
  AST_NOP,
  AST_SCRIPT,
  AST_VAR,
  AST_VAR_DECL,
  AST_FUNC_DECL,
  AST_IF,
  AST_FUNC,

  AST_ASSIGN,
  AST_REM_ASSIGN,
  AST_MUL_ASSIGN,
  AST_DIV_ASSIGN,
  AST_XOR_ASSIGN,
  AST_PLUS_ASSIGN,
  AST_MINUS_ASSIGN,
  AST_OR_ASSIGN,
  AST_AND_ASSIGN,
  AST_LSHIFT_ASSIGN,
  AST_RSHIFT_ASSIGN,
  AST_URSHIFT_ASSIGN,

  AST_NUM,
  AST_IDENT,
  AST_STRING,
  AST_REGEX,
  AST_LABEL,

  AST_SEQ,
  AST_WHILE,
  AST_DOWHILE,
  AST_FOR,
  AST_FOR_IN,
  AST_COND,

  AST_DEBUGGER,
  AST_BREAK,
  AST_LABELED_BREAK,
  AST_CONTINUE,
  AST_LABELED_CONTINUE,
  AST_RETURN,
  AST_VALUE_RETURN,
  AST_THROW,

  AST_TRY,
  AST_SWITCH,
  AST_CASE,
  AST_DEFAULT,
  AST_WITH,

  AST_LOGICAL_OR,
  AST_LOGICAL_AND,
  AST_OR,
  AST_XOR,
  AST_AND,

  AST_EQ,
  AST_EQ_EQ,
  AST_NE,
  AST_NE_NE,

  AST_LE,
  AST_LT,
  AST_GE,
  AST_GT,
  AST_IN,
  AST_INSTANCEOF,

  AST_LSHIFT,
  AST_RSHIFT,
  AST_URSHIFT,

  AST_ADD,
  AST_SUB,

  AST_REM,
  AST_MUL,
  AST_DIV,

  AST_POSITIVE,
  AST_NEGATIVE,
  AST_NOT,
  AST_LOGICAL_NOT,
  AST_VOID,
  AST_DELETE,
  AST_TYPEOF,
  AST_PREINC,
  AST_PREDEC,

  AST_POSTINC,
  AST_POSTDEC,

  AST_MEMBER,
  AST_INDEX,
  AST_CALL,

  AST_NEW,

  AST_ARRAY,
  AST_OBJECT,
  AST_PROP,
  AST_GETTER,
  AST_SETTER,

  AST_THIS,
  AST_TRUE,
  AST_FALSE,
  AST_NULL,
  AST_UNDEFINED,

  AST_USE_STRICT,

  AST_MAX_TAG
};

struct ast {
  struct mbuf mbuf;
};

typedef unsigned long ast_off_t;

struct ast_node_def {
  const char *name;           /* tag name, for debugging and serialization */
  unsigned char has_varint;   /* has a varint body */
  unsigned char has_inlined;  /* inlined data whose size is in varint field */
  unsigned char num_skips;    /* number of skips */
  unsigned char num_subtrees; /* number of fixed subtrees */
};
extern const struct ast_node_def ast_node_defs[];

enum ast_which_skip {
  AST_END_SKIP = 0,
  AST_VAR_NEXT_SKIP = 1,
  AST_SCRIPT_FIRST_VAR_SKIP = AST_VAR_NEXT_SKIP,
  AST_FOR_BODY_SKIP = 1,
  AST_DO_WHILE_COND_SKIP = 1,
  AST_END_IF_TRUE_SKIP = 1,
  AST_TRY_CATCH_SKIP = 1,
  AST_TRY_FINALLY_SKIP = 2,
  AST_FUNC_FIRST_VAR_SKIP = AST_VAR_NEXT_SKIP,
  AST_FUNC_BODY_SKIP = 2,
  AST_SWITCH_DEFAULT_SKIP = 1
};

V7_PRIVATE void ast_init(struct ast *, size_t);
V7_PRIVATE void ast_optimize(struct ast *);
V7_PRIVATE void ast_free(struct ast *);
V7_PRIVATE ast_off_t ast_add_node(struct ast *, enum ast_tag);
V7_PRIVATE ast_off_t ast_insert_node(struct ast *, ast_off_t, enum ast_tag);
V7_PRIVATE ast_off_t ast_set_skip(struct ast *, ast_off_t, enum ast_which_skip);
V7_PRIVATE ast_off_t ast_get_skip(struct ast *, ast_off_t, enum ast_which_skip);
V7_PRIVATE ast_off_t
ast_modify_skip(struct ast *, ast_off_t, ast_off_t, enum ast_which_skip);
V7_PRIVATE enum ast_tag ast_fetch_tag(struct ast *, ast_off_t *);
V7_PRIVATE void ast_move_to_children(struct ast *, ast_off_t *);

V7_PRIVATE void ast_add_inlined_node(struct ast *, enum ast_tag, const char *,
                                     size_t);
V7_PRIVATE void ast_insert_inlined_node(struct ast *, ast_off_t, enum ast_tag,
                                        const char *, size_t);

V7_PRIVATE char *ast_get_inlined_data(struct ast *, ast_off_t, size_t *);
V7_PRIVATE void ast_get_num(struct ast *, ast_off_t, double *);

V7_PRIVATE void ast_skip_tree(struct ast *, ast_off_t *);
V7_PRIVATE void ast_dump(FILE *, struct ast *, ast_off_t);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* AST_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_PARSER_H_INCLUDED
#define V7_PARSER_H_INCLUDED


#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

struct v7_pstate {
  const char *file_name;
  const char *source_code;
  const char *pc;   /* Current parsing position */
  int line_no;      /* Line number */
  int prev_line_no; /* Line number of previous token */
  int inhibit_in;   /* True while `in` expressions are inhibited */
  int in_function;  /* True if in a function */
  int in_loop;      /* True if in a loop */
  int in_switch;    /* True if in a switch block */
  int in_strict;    /* True if in strict mode */
};

V7_PRIVATE enum v7_err parse(struct v7 *, struct ast *, const char *, int);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* V7_PARSER_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MM_H_INCLUDED
#define MM_H_INCLUDED


typedef void (*gc_cell_destructor_t)(struct v7 *v7, void *);

struct gc_arena {
  char *base;
  size_t size;
  char *free; /* head of free list */
  size_t cell_size;

  unsigned long allocations; /* cumulative counter of allocations */
  unsigned long alive;       /* number of living cells */

  gc_cell_destructor_t destructor;

  int verbose;
  const char *name; /* for debugging purposes */
};

#endif /* GC_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_INTERNAL_H_INCLUDED
#define V7_INTERNAL_H_INCLUDED


/* Check whether we're compiling in an environment with no filesystem */
#if defined(ARDUINO) && (ARDUINO == 106)
#define V7_NO_FS
#endif

/*
 * DO NOT SUBMIT: remove this when adding support
 * for predefined strings as roots
 */
#define V7_DISABLE_PREDEFINED_STRINGS

#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>

/* Public API. Implemented in api.c */

#ifdef V7_WINDOWS
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#define __unused
typedef __int64 int64_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned long uintptr_t;
#define __func__ ""
#else
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#endif

/* Private API */

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

enum cached_strings {
  PREDEFINED_STR_LENGTH,
  PREDEFINED_STR_PROTOTYPE,
  PREDEFINED_STR_CONSTRUCTOR,
  PREDEFINED_STR_ARGUMENTS,

  PREDEFINED_STR_MAX
};

enum error_ctor {
  TYPE_ERROR,
  SYNTAX_ERROR,
  REFERENCE_ERROR,
  INTERNAL_ERROR,
  RANGE_ERROR,

  ERROR_CTOR_MAX
};


struct v7 {
  val_t global_object;
  val_t this_object;

  val_t object_prototype;
  val_t array_prototype;
  val_t boolean_prototype;
  val_t error_prototype;
  val_t string_prototype;
  val_t regexp_prototype;
  val_t number_prototype;
  val_t date_prototype;
  val_t function_prototype;
#ifndef V7_DISABLE_SOCKETS
  val_t socket_prototype;
#endif

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

  struct mbuf owned_strings;   /* Sequence of (varint len, char data[]) */
  struct mbuf foreign_strings; /* Sequence of (varint len, char *data) */

  struct mbuf tmp_stack; /* Stack of val_t* elements, used as root set */

  struct gc_arena object_arena;
  struct gc_arena function_arena;
  struct gc_arena property_arena;

  int strict_mode; /* true if currently in strict mode */

  val_t error_objects[ERROR_CTOR_MAX];

  val_t thrown_error;
  char error_msg[60];     /* Exception message */
  int creating_exception; /* Avoids reentrant exception creation */
#if defined(__cplusplus)
  ::jmp_buf jmp_buf;
  ::jmp_buf label_jmp_buf;
#else
  jmp_buf jmp_buf;       /* Exception environment for v7_exec() */
  jmp_buf label_jmp_buf; /* Target for non local (labeled) breaks */
#endif
  char *label;      /* Inner label */
  size_t label_len; /* Inner label length */
  int lab_cont;     /* True if re-entering a loop with labeled continue */

  struct mbuf json_visited_stack; /* Detecting cycle in to_json */

  /* Parser state */
  struct v7_pstate pstate; /* Parsing state */
  enum v7_tok cur_tok;     /* Current token */
  const char *tok;         /* Parsed terminal token (ident, number, string) */
  unsigned long tok_len;   /* Length of the parsed terminal token */
  size_t last_var_node;    /* Offset of last var node or function/script node */
  int after_newline;       /* True if the cur_tok starts a new line */
  double cur_tok_dbl;      /* When tokenizing, parser stores TOK_NUMBER here */

  /* TODO(mkm): remove when AST are GC-ed */
  struct mbuf allocated_asts;

  val_t predefined_strings[PREDEFINED_STR_MAX];
  /* singleton, pointer because of amalgamation */
  struct v7_property *cur_dense_prop;
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

#define V7_STATIC_ASSERT(COND, MSG) \
  typedef char static_assertion_##MSG[2 * (!!(COND)) - 1]

#define V7_CHECK(v7, COND)                                            \
  do {                                                                \
    if (!(COND))                                                      \
      throw_exception(v7, INTERNAL_ERROR, "%s line %d: %s", __func__, \
                      __LINE__, #COND);                               \
  } while (0)

#define TRACE_VAL(v7, val)                                     \
  do {                                                         \
    char buf[200], *p = v7_to_json(v7, val, buf, sizeof(buf)); \
    printf("%s %d: [%s]\n", __func__, __LINE__, p);            \
    if (p != buf) free(p);                                     \
  } while (0)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

V7_PRIVATE void throw_value(struct v7 *, val_t);
V7_PRIVATE void throw_exception(struct v7 *, enum error_ctor, const char *,
                                ...);
V7_PRIVATE size_t unescape(const char *s, size_t len, char *to);

V7_PRIVATE void init_js_stdlib(struct v7 *);

V7_PRIVATE val_t Regex_ctor(struct v7 *v7, val_t this_obj, val_t args);

V7_PRIVATE val_t rx_exec(struct v7 *v7, val_t rx, val_t str, int lind);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* V7_INTERNAL_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED


/* TODO(mkm): remove ifdef once v7 has been moved here */
#ifndef V7_VALUE_DEFINED
typedef uint64_t val_t;
#endif

#define V7_TAG_OBJECT ((uint64_t) 0xFFFF << 48)
#define V7_TAG_FOREIGN ((uint64_t) 0xFFFE << 48)
#define V7_TAG_UNDEFINED ((uint64_t) 0xFFFD << 48)
#define V7_TAG_BOOLEAN ((uint64_t) 0xFFFC << 48)
#define V7_TAG_NAN ((uint64_t) 0xFFFB << 48)
#define V7_TAG_STRING_I ((uint64_t) 0xFFFA << 48)  /* Inlined string len < 5 */
#define V7_TAG_STRING_5 ((uint64_t) 0xFFF9 << 48)  /* Inlined string len 5 */
#define V7_TAG_STRING_O ((uint64_t) 0xFFF8 << 48)  /* Owned string */
#define V7_TAG_STRING_F ((uint64_t) 0xFFF7 << 48)  /* Foreign string */
#define V7_TAG_STRING_C ((uint64_t) 0xFFF6 << 48)  /* String chunk */
#define V7_TAG_FUNCTION ((uint64_t) 0xFFF5 << 48)  /* JavaScript function */
#define V7_TAG_CFUNCTION ((uint64_t) 0xFFF4 << 48) /* C function */
#define V7_TAG_GETSETTER ((uint64_t) 0xFFF3 << 48) /* getter+setter */
#define V7_TAG_REGEXP ((uint64_t) 0xFFF2 << 48)    /* Regex */
#define V7_TAG_NOVALUE ((uint64_t) 0xFFF1 << 48)   /* Sentinel for no value */
#define V7_TAG_MASK ((uint64_t) 0xFFFF << 48)

#define V7_NULL V7_TAG_FOREIGN
#define V7_UNDEFINED V7_TAG_UNDEFINED

struct v7_property {
  struct v7_property *next; /* Linkage in struct v7_object::properties */
  val_t name;               /* Property name (a string) */
  val_t value;              /* Property value */

  unsigned int attributes;
#define V7_PROPERTY_READ_ONLY 1
#define V7_PROPERTY_DONT_ENUM 2
#define V7_PROPERTY_DONT_DELETE 4
#define V7_PROPERTY_HIDDEN 8
#define V7_PROPERTY_GETTER 16
#define V7_PROPERTY_SETTER 32
};

/*
 * An object is an unordered collection of properties.
 * A function stored in a property of an object is called a method.
 * A property has a name, a value, and set of attributes.
 * Attributes are: ReadOnly, DontEnum, DontDelete, Internal.
 *
 * A constructor is a function that creates and initializes objects.
 * Each constructor has an associated prototype object that is used for
 * inheritance and shared properties. When a constructor creates an object,
 * the new object references the constructor’s prototype.
 *
 * Objects could be a "generic objects" which is a collection of properties,
 * or a "typed object" which also hold an internal value like String or Number.
 * Those values are implicit, unnamed properties of the respective types,
 * and can be coerced into primitive types by calling a respective constructor
 * as a function:
 *    var a = new Number(123);
 *    typeof(a) == 'object';
 *    typeof(Number(a)) == 'number';
 */
struct v7_object {
  /* First HIDDEN property in a chain is an internal object value */
  struct v7_property *properties;
  struct v7_object *prototype;
  uint8_t attributes;
#define V7_OBJ_NOT_EXTENSIBLE 1 /* TODO(lsm): store this in LSB */
#define V7_OBJ_DENSE_ARRAY 2    /* TODO(mkm): store in some tag */
};

/*
 * Variables are function-scoped and are hoisted.
 * Lexical scoping & closures: each function has a chain of scopes, defined
 * by the lexicographic order of function definitions.
 * Scope is different from the execution context.
 * Execution context carries "variable object" which is variable/value
 * mapping for all variables defined in a function, and `this` object.
 * If function is not called as a method, then `this` is a global object.
 * Otherwise, `this` is an object that contains called method.
 * New execution context is created each time a function call is performed.
 * Passing arguments through recursion is done using execution context, e.g.
 *
 *    var factorial = function(num) {
 *      return num < 2 ? 1 : num * factorial(num - 1);
 *    };
 *
 * Here, recursion calls the same function `factorial` several times. Execution
 * contexts for each call form a stack. Each context has different variable
 * object, `vars`, with different values of `num`.
 */

struct v7_function {
  /*
   * Functions are objects. This has to be the first field so that function
   * objects can be managed by the GC.
   */
  struct v7_property *properties;
  struct v7_object *scope; /* lexical scope of the closure */
  uintptr_t debug;
  struct ast *ast;         /* AST, used as a byte code for execution */
  unsigned int ast_off;    /* Position of the function node in the AST */
  unsigned int attributes; /* Function attributes */
#define V7_FUNCTION_STRICT 1
};

struct v7_regexp {
  val_t regexp_string;
  struct slre_prog *compiled_regexp;
  long lastIndex;
};

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* TODO(mkm): possibly replace those with macros for inlining */
enum v7_type val_type(struct v7 *v7, val_t);
int v7_is_error(struct v7 *v7, val_t);
V7_PRIVATE val_t v7_pointer_to_value(void *);

V7_PRIVATE struct v7_regexp *v7_to_regexp(val_t);
val_t v7_object_to_value(struct v7_object *);
val_t v7_function_to_value(struct v7_function *);

struct v7_object *v7_to_object(val_t);
struct v7_function *v7_to_function(val_t);
V7_PRIVATE void *v7_to_pointer(val_t v);

V7_PRIVATE void init_object(struct v7 *v7);
V7_PRIVATE void init_array(struct v7 *v7);
V7_PRIVATE void init_error(struct v7 *v7);
V7_PRIVATE void init_boolean(struct v7 *v7);
V7_PRIVATE void init_math(struct v7 *v7);
V7_PRIVATE void init_string(struct v7 *v7);
V7_PRIVATE void init_regex(struct v7 *v7);
V7_PRIVATE void init_number(struct v7 *v7);
V7_PRIVATE void init_json(struct v7 *v7);
V7_PRIVATE void init_date(struct v7 *v7);
V7_PRIVATE void init_function(struct v7 *v7);
V7_PRIVATE void init_stdlib(struct v7 *v7);
V7_PRIVATE void init_socket(struct v7 *v7);

V7_PRIVATE int set_cfunc_prop(struct v7 *, val_t, const char *, v7_cfunction_t);
V7_PRIVATE v7_val_t
v7_create_cfunction_object(struct v7 *, v7_cfunction_t, int);
V7_PRIVATE v7_val_t
v7_create_cfunction_ctor(struct v7 *, val_t, v7_cfunction_t, int);

V7_PRIVATE int set_cfunc_obj_prop(struct v7 *, val_t obj, const char *name,
                                  v7_cfunction_t f, int num_args);

V7_PRIVATE val_t v_get_prototype(struct v7 *, val_t);
V7_PRIVATE int is_prototype_of(struct v7 *, val_t, val_t);

/* TODO(lsm): NaN payload location depends on endianness, make crossplatform */
#define GET_VAL_NAN_PAYLOAD(v) ((char *) &(v))

V7_PRIVATE val_t create_object(struct v7 *, val_t);
V7_PRIVATE v7_val_t v7_create_function(struct v7 *v7);
V7_PRIVATE v7_val_t v7_create_dense_array(struct v7 *v7);
V7_PRIVATE int v7_stringify_value(struct v7 *, val_t, char *, size_t);
V7_PRIVATE struct v7_property *v7_create_property(struct v7 *);

V7_PRIVATE struct v7_property *v7_get_own_property(struct v7 *, val_t,
                                                   const char *, size_t);
V7_PRIVATE struct v7_property *v7_get_own_property2(struct v7 *, val_t obj,
                                                    const char *name, size_t,
                                                    unsigned int attrs);

/* If `len` is -1/MAXUINT/~0, then `name` must be 0-terminated */
V7_PRIVATE struct v7_property *v7_get_property(struct v7 *, val_t obj,
                                               const char *name, size_t);
V7_PRIVATE void v7_invoke_setter(struct v7 *, struct v7_property *, val_t,
                                 val_t);
V7_PRIVATE int v7_set_v(struct v7 *, v7_val_t, v7_val_t, v7_val_t);
V7_PRIVATE int v7_set_property_v(struct v7 *, v7_val_t obj, v7_val_t name,
                                 unsigned int attributes, v7_val_t val);
V7_PRIVATE int v7_set_property(struct v7 *, v7_val_t obj, const char *name,
                               size_t len, unsigned int attributes,
                               v7_val_t val);
V7_PRIVATE struct v7_property *v7_set_prop(struct v7 *v7, val_t obj, val_t name,
                                           unsigned int attributes, val_t val);

/* Return address of property value or NULL if the passed property is NULL */
V7_PRIVATE val_t v7_property_value(struct v7 *, val_t, struct v7_property *);

/*
 * If `len` is -1/MAXUINT/~0, then `name` must be 0-terminated.
 * Return 0 on success, -1 on error.
 */
V7_PRIVATE int v7_del_property(struct v7 *, val_t, const char *, size_t);

V7_PRIVATE val_t v7_array_get2(struct v7 *, v7_val_t, unsigned long, int *);
V7_PRIVATE long arg_long(struct v7 *v7, val_t args, int n, long default_value);
V7_PRIVATE int to_str(struct v7 *v7, val_t v, char *buf, size_t size,
                      int as_json);
V7_PRIVATE void v7_destroy_property(struct v7_property **p);
V7_PRIVATE val_t i_value_of(struct v7 *v7, val_t v);
V7_PRIVATE val_t Std_eval(struct v7 *v7, val_t t, val_t args);

/* String API */
V7_PRIVATE int s_cmp(struct v7 *, val_t a, val_t b);
V7_PRIVATE val_t s_concat(struct v7 *, val_t, val_t);
V7_PRIVATE val_t ulong_to_str(struct v7 *, unsigned long);
V7_PRIVATE unsigned long str_to_ulong(struct v7 *, val_t, int *);
V7_PRIVATE unsigned long cstr_to_ulong(const char *, size_t len, int *);
V7_PRIVATE void embed_string(struct mbuf *, size_t, const char *, size_t, int,
                             int);
/* TODO(mkm): rename after regexp merge */
V7_PRIVATE val_t to_string(struct v7 *v7, val_t v);

V7_PRIVATE val_t Obj_valueOf(struct v7 *, val_t, val_t);
V7_PRIVATE double i_as_num(struct v7 *, val_t);
V7_PRIVATE val_t n_to_str(struct v7 *, val_t, val_t, const char *);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* VM_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef GC_H_INCLUDED
#define GC_H_INCLUDED


/* Disable GC on 32-bit platform for now */
#if ULONG_MAX == 4294967295
#define V7_DISABLE_GC
#endif

#define MARK(p) (((struct gc_cell *) (p))->word |= 1)
#define UNMARK(p) (((struct gc_cell *) (p))->word &= ~1)
#define MARKED(p) (((struct gc_cell *) (p))->word & 1)

struct gc_tmp_frame {
  struct v7 *v7;
  size_t pos;
};

struct gc_cell {
  uintptr_t word;
};

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

V7_PRIVATE struct v7_object *new_object(struct v7 *);
V7_PRIVATE struct v7_property *new_property(struct v7 *);
V7_PRIVATE struct v7_function *new_function(struct v7 *);

V7_PRIVATE void gc_mark(struct v7 *, val_t);

V7_PRIVATE void gc_arena_init(struct v7 *, struct gc_arena *, size_t, size_t,
                              const char *);
V7_PRIVATE void gc_arena_grow(struct v7 *, struct gc_arena *, size_t);
V7_PRIVATE void gc_arena_destroy(struct v7 *, struct gc_arena *a);
V7_PRIVATE void gc_sweep(struct v7 *, struct gc_arena *, size_t);
V7_PRIVATE void *gc_alloc_cell(struct v7 *, struct gc_arena *);

V7_PRIVATE struct gc_tmp_frame new_tmp_frame(struct v7 *);
V7_PRIVATE void tmp_frame_cleanup(struct gc_tmp_frame *);
V7_PRIVATE void tmp_stack_push(struct gc_tmp_frame *, val_t *);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* GC_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 *
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

#ifndef SLRE_HEADER_INCLUDED
#define SLRE_HEADER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Regex flags */
#define SLRE_FLAG_G 1  /* Global - match in the whole string */
#define SLRE_FLAG_I 2  /* Ignore case */
#define SLRE_FLAG_M 4  /* Multiline */
#define SLRE_FLAG_RE 8 /* flag RegExp/String */

/* Describes single capture */
struct slre_cap {
  const char *start; /* points to the beginning of the capture group */
  const char *end;   /* points to the end of the capture group */
};

/* Describes all captures */
#define SLRE_MAX_CAPS 32
struct slre_loot {
  int num_captures;
  struct slre_cap caps[SLRE_MAX_CAPS];
};

/* Opaque structure that holds compiled regular expression */
struct slre_prog;

/* Return codes for slre_compile() */
enum slre_error {
  SLRE_OK,
  SLRE_INVALID_DEC_DIGIT,
  SLRE_INVALID_HEX_DIGIT,
  SLRE_INVALID_ESC_CHAR,
  SLRE_UNTERM_ESC_SEQ,
  SLRE_SYNTAX_ERROR,
  SLRE_UNMATCH_LBR,
  SLRE_UNMATCH_RBR,
  SLRE_NUM_OVERFLOW,
  SLRE_INF_LOOP_M_EMP_STR,
  SLRE_TOO_MANY_CHARSETS,
  SLRE_INV_CHARSET_RANGE,
  SLRE_CHARSET_TOO_LARGE,
  SLRE_MALFORMED_CHARSET,
  SLRE_INVALID_BACK_REFERENCE,
  SLRE_TOO_MANY_CAPTURES,
  SLRE_INVALID_QUANTIFIER,
  SLRE_BAD_CHAR_AFTER_USD
};

int slre_compile(const char *regexp, size_t regexp_len, const char *flags,
                 size_t flags_len, struct slre_prog **, int is_regex);
int slre_exec(struct slre_prog *prog, int flag_g, const char *start,
              const char *end, struct slre_loot *loot);
void slre_free(struct slre_prog *prog);

int slre_match(const char *, size_t, const char *, size_t, const char *, size_t,
               struct slre_loot *);
int slre_replace(struct slre_loot *loot, const char *src, size_t src_len,
                 const char *replace, size_t rep_len, struct slre_loot *dst);
int slre_get_flags(struct slre_prog *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SLRE_HEADER_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_VARINT_H_INCLUDED
#define V7_VARINT_H_INCLUDED


#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

V7_PRIVATE int encode_varint(size_t len, unsigned char *p);
V7_PRIVATE size_t decode_varint(const unsigned char *p, int *llen);
V7_PRIVATE int calc_llen(size_t len);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* V7_VARINT_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


/* Initializes mbuf. */
V7_PRIVATE void mbuf_init(struct mbuf *mbuf, size_t initial_size) {
  mbuf->len = mbuf->size = 0;
  mbuf->buf = NULL;
  mbuf_resize(mbuf, initial_size);
}

/* Frees the space allocated for the iobuffer and resets the iobuf structure. */
V7_PRIVATE void mbuf_free(struct mbuf *mbuf) {
  if (mbuf->buf != NULL) {
    free(mbuf->buf);
    mbuf_init(mbuf, 0);
  }
}

/*
 * Resize mbuf.
 *
 * If `new_size` is smaller than buffer's `len`, the
 * resize is not performed.
 */
V7_PRIVATE void mbuf_resize(struct mbuf *a, size_t new_size) {
  char *p;
  if ((new_size > a->size || (new_size < a->size && new_size >= a->len)) &&
      (p = (char *) realloc(a->buf, new_size)) != NULL) {
    a->size = new_size;
    a->buf = p;
  }
}

/* Shrinks mbuf size to just fit it's length. */
V7_PRIVATE void mbuf_trim(struct mbuf *mbuf) {
  mbuf_resize(mbuf, mbuf->len);
}

/*
 * Appends data to the mbuf.
 *
 * It returns the amount of bytes appended.
 */
V7_PRIVATE size_t mbuf_append(struct mbuf *a, const char *buf, size_t len) {
  return mbuf_insert(a, a->len, buf, len);
}

/*
 * Inserts data at a specified offset in the mbuf.
 *
 * Existing data will be shifted forwards and the buffer will
 * be grown if necessary.
 * It returns the amount of bytes inserted.
 */
V7_PRIVATE size_t
mbuf_insert(struct mbuf *a, size_t off, const char *buf, size_t len) {
  char *p = NULL;

  assert(a != NULL);
  assert(a->len <= a->size);
  assert(off <= a->len);

  /* check overflow */
  if (~(size_t) 0 - (size_t) a->buf < len) return 0;

  if (a->len + len <= a->size) {
    memmove(a->buf + off + len, a->buf + off, a->len - off);
    if (buf != NULL) {
      memcpy(a->buf + off, buf, len);
    }
    a->len += len;
  } else if ((p = (char *) realloc(
                  a->buf, (a->len + len) * MBUF_SIZE_MULTIPLIER)) != NULL) {
    a->buf = p;
    memmove(a->buf + off + len, a->buf + off, a->len - off);
    if (buf != NULL) {
      memcpy(a->buf + off, buf, len);
    }
    a->len += len;
    a->size = a->len * MBUF_SIZE_MULTIPLIER;
  } else {
    len = 0;
  }

  return len;
}
/*
 * The authors of this software are Rob Pike and Ken Thompson.
 *              Copyright (c) 2002 by Lucent Technologies.
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR LUCENT TECHNOLOGIES MAKE
 * ANY REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */
#include <stdarg.h>
#include <string.h>

enum
{
	Bit1	= 7,
	Bitx	= 6,
	Bit2	= 5,
	Bit3	= 4,
	Bit4	= 3,
	Bit5	= 2,

	T1	= ((1<<(Bit1+1))-1) ^ 0xFF,	/* 0000 0000 */
	Tx	= ((1<<(Bitx+1))-1) ^ 0xFF,	/* 1000 0000 */
	T2	= ((1<<(Bit2+1))-1) ^ 0xFF,	/* 1100 0000 */
	T3	= ((1<<(Bit3+1))-1) ^ 0xFF,	/* 1110 0000 */
	T4	= ((1<<(Bit4+1))-1) ^ 0xFF,	/* 1111 0000 */
	T5	= ((1<<(Bit5+1))-1) ^ 0xFF,	/* 1111 1000 */

	Rune1	= (1<<(Bit1+0*Bitx))-1,		/* 0000 0000 0000 0000 0111 1111 */
	Rune2	= (1<<(Bit2+1*Bitx))-1,		/* 0000 0000 0000 0111 1111 1111 */
	Rune3	= (1<<(Bit3+2*Bitx))-1,		/* 0000 0000 1111 1111 1111 1111 */
	Rune4	= (1<<(Bit4+3*Bitx))-1,		/* 0011 1111 1111 1111 1111 1111 */

	Maskx	= (1<<Bitx)-1,			/* 0011 1111 */
	Testx	= Maskx ^ 0xFF,			/* 1100 0000 */

	Bad	= Runeerror
};

int
chartorune(Rune *rune, const char *str)
{
	int c, c1, c2/* , c3 */;
	unsigned short l;

	/*
	 * one character sequence
	 *	00000-0007F => T1
	 */
	c = *(uchar*)str;
	if(c < Tx) {
		*rune = c;
		return 1;
	}

	/*
	 * two character sequence
	 *	0080-07FF => T2 Tx
	 */
	c1 = *(uchar*)(str+1) ^ Tx;
	if(c1 & Testx)
		goto bad;
	if(c < T3) {
		if(c < T2)
			goto bad;
		l = ((c << Bitx) | c1) & Rune2;
		if(l <= Rune1)
			goto bad;
		*rune = l;
		return 2;
	}

	/*
	 * three character sequence
	 *	0800-FFFF => T3 Tx Tx
	 */
	c2 = *(uchar*)(str+2) ^ Tx;
	if(c2 & Testx)
		goto bad;
	if(c < T4) {
		l = ((((c << Bitx) | c1) << Bitx) | c2) & Rune3;
		if(l <= Rune2)
			goto bad;
		*rune = l;
		return 3;
	}

	/*
	 * four character sequence
	 *	10000-10FFFF => T4 Tx Tx Tx
	 */
	/* if(UTFmax >= 4) {
		c3 = *(uchar*)(str+3) ^ Tx;
		if(c3 & Testx)
			goto bad;
		if(c < T5) {
			l = ((((((c << Bitx) | c1) << Bitx) | c2) << Bitx) | c3) & Rune4;
			if(l <= Rune3)
				goto bad;
			if(l > Runemax)
				goto bad;
			*rune = l;
			return 4;
		}
	} */

	/*
	 * bad decoding
	 */
bad:
	*rune = Bad;
	return 1;
}

int
runetochar(char *str, Rune *rune)
{
	unsigned short c;

	/*
	 * one character sequence
	 *	00000-0007F => 00-7F
	 */
	c = *rune;
	if(c <= Rune1) {
		str[0] = c;
		return 1;
	}

	/*
	 * two character sequence
	 *	00080-007FF => T2 Tx
	 */
	if(c <= Rune2) {
		str[0] = T2 | (c >> 1*Bitx);
		str[1] = Tx | (c & Maskx);
		return 2;
	}

	/*
	 * three character sequence
	 *	00800-0FFFF => T3 Tx Tx
	 */
	/* if(c > Runemax) 
		c = Runeerror; */
	/* if(c <= Rune3) { */
		str[0] = T3 |  (c >> 2*Bitx);
		str[1] = Tx | ((c >> 1*Bitx) & Maskx);
		str[2] = Tx |  (c & Maskx);
		return 3;
	/* } */

	/*
	 * four character sequence
	 *	010000-1FFFFF => T4 Tx Tx Tx
	 */
	/* str[0] = T4 |  (c >> 3*Bitx);
	str[1] = Tx | ((c >> 2*Bitx) & Maskx);
	str[2] = Tx | ((c >> 1*Bitx) & Maskx);
	str[3] = Tx |  (c & Maskx);
	return 4; */
}

/* int
runelen(long c)
{
	Rune rune;
	char str[10];

	rune = c;
	return runetochar(str, &rune);
} */
int runelen(Rune c){
	if(c <= Rune1)					return 1;
	if(c <= Rune2)					return 2;
	/* if(c <= Rune3 || c > Runemax) */	return 3;
	/* return 4; */
}

/* int
runenlen(Rune *r, int nrune)
{
	int nb, c;

	nb = 0;
	while(nrune--) {
		c = *r++;
		if(c <= Rune1)
			nb++;
		else
		if(c <= Rune2)
			nb += 2;
		else
		if(c <= Rune3 || c > Runemax)
			nb += 3;
		else
			nb += 4;
	}
	return nb;
} */
int runenlen(Rune *r, int nrune){
	int nb = 0;
	while(nrune--) nb += runelen(*r++);
	return nb;
}

int
fullrune(char *str, int n)
{
	int c;

	if(n <= 0)
		return 0;
	c = *(uchar*)str;
	if(c < Tx)
		return 1;
	if(c < T3)
		return n >= 2;
	if(UTFmax == 3 || c < T4)
		return n >= 3;
	return n >= 4;
}

int
utfnlen(char *s, long m)
{
	int c;
	long n;
	Rune rune;
	char *es;

	es = s + m;
	for(n = 0; s < es; n++) {
		c = *(uchar*)s;
		if(c < Runeself){
			if(c == '\0')
				break;
			s++;
			continue;
		}
		if(!fullrune(s, es-s))
			break;
		s += chartorune(&rune, s);
	}
	return n;
}

char*
utfnshift(char *s, long m)
{
	int c;
	long n;
	Rune rune;

	for(n = 0; n < m; n++) {
		c = *(uchar*)s;
		if(c < Runeself){
			if(c == '\0')
				break;
			s++;
			continue;
		}
		s += chartorune(&rune, s);
	}
	return s;
}

#if 0

char*
utfecpy(char *to, char *e, char *from)
{
	char *end;

	if(to >= e)
		return to;
	end = memccpy(to, from, '\0', e - to);
	if(end == NULL){
		end = e-1;
		while(end>to && (*--end&0xC0)==0x80)
			;
		*end = '\0';
	}else{
		end--;
	}
	return end;
}

int
utflen(char *s)
{
	int c;
	long n;
	Rune rune;

	n = 0;
	for(;;) {
		c = *(uchar*)s;
		if(c < Runeself) {
			if(c == 0)
				return n;
			s++;
		} else
			s += chartorune(&rune, s);
		n++;
	}
}

char*
utfrrune(char *s, long c)
{
	long c1;
	Rune r;
	char *s1;

	if(c < Runesync)		/* not part of utf sequence */
		return strrchr(s, c);

	s1 = 0;
	for(;;) {
		c1 = *(uchar*)s;
		if(c1 < Runeself) {	/* one byte rune */
			if(c1 == 0)
				return s1;
			if(c1 == c)
				s1 = s;
			s++;
			continue;
		}
		c1 = chartorune(&r, s);
		if(r == c)
			s1 = s;
		s += c1;
	}
}

char*
utfrune(char *s, long c)
{
	long c1;
	Rune r;
	int n;

	if(c < Runesync)		/* not part of utf sequence */
		return strchr(s, c);

	for(;;) {
		c1 = *(uchar*)s;
		if(c1 < Runeself) {	/* one byte rune */
			if(c1 == 0)
				return 0;
			if(c1 == c)
				return s;
			s++;
			continue;
		}
		n = chartorune(&r, s);
		if(r == c)
			return s;
		s += n;
	}
}

/*
 * Return pointer to first occurrence of s2 in s1,
 * 0 if none
 */
char*
utfutf(char *s1, char *s2)
{
	char *p;
	long f, n1, n2;
	Rune r;

	n1 = chartorune(&r, s2);
	f = r;
	if(f <= Runesync)		/* represents self */
		return strstr(s1, s2);

	n2 = strlen(s2);
	for(p=s1; NULL != (p=utfrune(p, f)); p+=n1)
		if(strncmp(p, s2, n2) == 0)
			return p;
	return 0;
}
#endif
/*
 * The authors of this software are Rob Pike and Ken Thompson.
 *              Copyright (c) 2002 by Lucent Technologies.
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR LUCENT TECHNOLOGIES MAKE
 * ANY REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */
#include <stdarg.h>
#include <string.h>

/*
 * alpha ranges -
 *	only covers ranges not in lower||upper
 */
static
Rune	__alpha2[] =
{
	0x00d8,	0x00f6,	/* Ø - ö */
	0x00f8,	0x01f5,	/* ø - ǵ */
	0x0250,	0x02a8,	/* ɐ - ʨ */
	0x038e,	0x03a1,	/* Ύ - Ρ */
	0x03a3,	0x03ce,	/* Σ - ώ */
	0x03d0,	0x03d6,	/* ϐ - ϖ */
	0x03e2,	0x03f3,	/* Ϣ - ϳ */
	0x0490,	0x04c4,	/* Ґ - ӄ */
	0x0561,	0x0587,	/* ա - և */
	0x05d0,	0x05ea,	/* א - ת */
	0x05f0,	0x05f2,	/* װ - ײ */
	0x0621,	0x063a,	/* ء - غ */
	0x0640,	0x064a,	/* ـ - ي */
	0x0671,	0x06b7,	/* ٱ - ڷ */
	0x06ba,	0x06be,	/* ں - ھ */
	0x06c0,	0x06ce,	/* ۀ - ێ */
	0x06d0,	0x06d3,	/* ې - ۓ */
	0x0905,	0x0939,	/* अ - ह */
	0x0958,	0x0961,	/* क़ - ॡ */
	0x0985,	0x098c,	/* অ - ঌ */
	0x098f,	0x0990,	/* এ - ঐ */
	0x0993,	0x09a8,	/* ও - ন */
	0x09aa,	0x09b0,	/* প - র */
	0x09b6,	0x09b9,	/* শ - হ */
	0x09dc,	0x09dd,	/* ড় - ঢ় */
	0x09df,	0x09e1,	/* য় - ৡ */
	0x09f0,	0x09f1,	/* ৰ - ৱ */
	0x0a05,	0x0a0a,	/* ਅ - ਊ */
	0x0a0f,	0x0a10,	/* ਏ - ਐ */
	0x0a13,	0x0a28,	/* ਓ - ਨ */
	0x0a2a,	0x0a30,	/* ਪ - ਰ */
	0x0a32,	0x0a33,	/* ਲ - ਲ਼ */
	0x0a35,	0x0a36,	/* ਵ - ਸ਼ */
	0x0a38,	0x0a39,	/* ਸ - ਹ */
	0x0a59,	0x0a5c,	/* ਖ਼ - ੜ */
	0x0a85,	0x0a8b,	/* અ - ઋ */
	0x0a8f,	0x0a91,	/* એ - ઑ */
	0x0a93,	0x0aa8,	/* ઓ - ન */
	0x0aaa,	0x0ab0,	/* પ - ર */
	0x0ab2,	0x0ab3,	/* લ - ળ */
	0x0ab5,	0x0ab9,	/* વ - હ */
	0x0b05,	0x0b0c,	/* ଅ - ଌ */
	0x0b0f,	0x0b10,	/* ଏ - ଐ */
	0x0b13,	0x0b28,	/* ଓ - ନ */
	0x0b2a,	0x0b30,	/* ପ - ର */
	0x0b32,	0x0b33,	/* ଲ - ଳ */
	0x0b36,	0x0b39,	/* ଶ - ହ */
	0x0b5c,	0x0b5d,	/* ଡ଼ - ଢ଼ */
	0x0b5f,	0x0b61,	/* ୟ - ୡ */
	0x0b85,	0x0b8a,	/* அ - ஊ */
	0x0b8e,	0x0b90,	/* எ - ஐ */
	0x0b92,	0x0b95,	/* ஒ - க */
	0x0b99,	0x0b9a,	/* ங - ச */
	0x0b9e,	0x0b9f,	/* ஞ - ட */
	0x0ba3,	0x0ba4,	/* ண - த */
	0x0ba8,	0x0baa,	/* ந - ப */
	0x0bae,	0x0bb5,	/* ம - வ */
	0x0bb7,	0x0bb9,	/* ஷ - ஹ */
	0x0c05,	0x0c0c,	/* అ - ఌ */
	0x0c0e,	0x0c10,	/* ఎ - ఐ */
	0x0c12,	0x0c28,	/* ఒ - న */
	0x0c2a,	0x0c33,	/* ప - ళ */
	0x0c35,	0x0c39,	/* వ - హ */
	0x0c60,	0x0c61,	/* ౠ - ౡ */
	0x0c85,	0x0c8c,	/* ಅ - ಌ */
	0x0c8e,	0x0c90,	/* ಎ - ಐ */
	0x0c92,	0x0ca8,	/* ಒ - ನ */
	0x0caa,	0x0cb3,	/* ಪ - ಳ */
	0x0cb5,	0x0cb9,	/* ವ - ಹ */
	0x0ce0,	0x0ce1,	/* ೠ - ೡ */
	0x0d05,	0x0d0c,	/* അ - ഌ */
	0x0d0e,	0x0d10,	/* എ - ഐ */
	0x0d12,	0x0d28,	/* ഒ - ന */
	0x0d2a,	0x0d39,	/* പ - ഹ */
	0x0d60,	0x0d61,	/* ൠ - ൡ */
	0x0e01,	0x0e30,	/* ก - ะ */
	0x0e32,	0x0e33,	/* า - ำ */
	0x0e40,	0x0e46,	/* เ - ๆ */
	0x0e5a,	0x0e5b,	/* ๚ - ๛ */
	0x0e81,	0x0e82,	/* ກ - ຂ */
	0x0e87,	0x0e88,	/* ງ - ຈ */
	0x0e94,	0x0e97,	/* ດ - ທ */
	0x0e99,	0x0e9f,	/* ນ - ຟ */
	0x0ea1,	0x0ea3,	/* ມ - ຣ */
	0x0eaa,	0x0eab,	/* ສ - ຫ */
	0x0ead,	0x0eae,	/* ອ - ຮ */
	0x0eb2,	0x0eb3,	/* າ - ຳ */
	0x0ec0,	0x0ec4,	/* ເ - ໄ */
	0x0edc,	0x0edd,	/* ໜ - ໝ */
	0x0f18,	0x0f19,	/* ༘ - ༙ */
	0x0f40,	0x0f47,	/* ཀ - ཇ */
	0x0f49,	0x0f69,	/* ཉ - ཀྵ */
	0x10d0,	0x10f6,	/* ა - ჶ */
	0x1100,	0x1159,	/* ᄀ - ᅙ */
	0x115f,	0x11a2,	/* ᅟ - ᆢ */
	0x11a8,	0x11f9,	/* ᆨ - ᇹ */
	0x1e00,	0x1e9b,	/* Ḁ - ẛ */
	0x1f50,	0x1f57,	/* ὐ - ὗ */
	0x1f80,	0x1fb4,	/* ᾀ - ᾴ */
	0x1fb6,	0x1fbc,	/* ᾶ - ᾼ */
	0x1fc2,	0x1fc4,	/* ῂ - ῄ */
	0x1fc6,	0x1fcc,	/* ῆ - ῌ */
	0x1fd0,	0x1fd3,	/* ῐ - ΐ */
	0x1fd6,	0x1fdb,	/* ῖ - Ί */
	0x1fe0,	0x1fec,	/* ῠ - Ῥ */
	0x1ff2,	0x1ff4,	/* ῲ - ῴ */
	0x1ff6,	0x1ffc,	/* ῶ - ῼ */
	0x210a,	0x2113,	/* ℊ - ℓ */
	0x2115,	0x211d,	/* ℕ - ℝ */
	0x2120,	0x2122,	/* ℠ - ™ */
	0x212a,	0x2131,	/* K - ℱ */
	0x2133,	0x2138,	/* ℳ - ℸ */
	0x3041,	0x3094,	/* ぁ - ゔ */
	0x30a1,	0x30fa,	/* ァ - ヺ */
	0x3105,	0x312c,	/* ㄅ - ㄬ */
	0x3131,	0x318e,	/* ㄱ - ㆎ */
	0x3192,	0x319f,	/* ㆒ - ㆟ */
	0x3260,	0x327b,	/* ㉠ - ㉻ */
	0x328a,	0x32b0,	/* ㊊ - ㊰ */
	0x32d0,	0x32fe,	/* ㋐ - ㋾ */
	0x3300,	0x3357,	/* ㌀ - ㍗ */
	0x3371,	0x3376,	/* ㍱ - ㍶ */
	0x337b,	0x3394,	/* ㍻ - ㎔ */
	0x3399,	0x339e,	/* ㎙ - ㎞ */
	0x33a9,	0x33ad,	/* ㎩ - ㎭ */
	0x33b0,	0x33c1,	/* ㎰ - ㏁ */
	0x33c3,	0x33c5,	/* ㏃ - ㏅ */
	0x33c7,	0x33d7,	/* ㏇ - ㏗ */
	0x33d9,	0x33dd,	/* ㏙ - ㏝ */
	0x4e00,	0x9fff,	/* 一 - 鿿 */
	0xac00,	0xd7a3,	/* 가 - 힣 */
	0xf900,	0xfb06,	/* 豈 - ﬆ */
	0xfb13,	0xfb17,	/* ﬓ - ﬗ */
	0xfb1f,	0xfb28,	/* ײַ - ﬨ */
	0xfb2a,	0xfb36,	/* שׁ - זּ */
	0xfb38,	0xfb3c,	/* טּ - לּ */
	0xfb40,	0xfb41,	/* נּ - סּ */
	0xfb43,	0xfb44,	/* ףּ - פּ */
	0xfb46,	0xfbb1,	/* צּ - ﮱ */
	0xfbd3,	0xfd3d,	/* ﯓ - ﴽ */
	0xfd50,	0xfd8f,	/* ﵐ - ﶏ */
	0xfd92,	0xfdc7,	/* ﶒ - ﷇ */
	0xfdf0,	0xfdf9,	/* ﷰ - ﷹ */
	0xfe70,	0xfe72,	/* ﹰ - ﹲ */
	0xfe76,	0xfefc,	/* ﹶ - ﻼ */
	0xff66,	0xff6f,	/* ｦ - ｯ */
	0xff71,	0xff9d,	/* ｱ - ﾝ */
	0xffa0,	0xffbe,	/* ﾠ - ﾾ */
	0xffc2,	0xffc7,	/* ￂ - ￇ */
	0xffca,	0xffcf,	/* ￊ - ￏ */
	0xffd2,	0xffd7,	/* ￒ - ￗ */
	0xffda,	0xffdc,	/* ￚ - ￜ */
};

/*
 * alpha singlets -
 *	only covers ranges not in lower||upper
 */
static
Rune	__alpha1[] =
{
	0x00aa,	/* ª */
	0x00b5,	/* µ */
	0x00ba,	/* º */
	0x03da,	/* Ϛ */
	0x03dc,	/* Ϝ */
	0x03de,	/* Ϟ */
	0x03e0,	/* Ϡ */
	0x06d5,	/* ە */
	0x09b2,	/* ল */
	0x0a5e,	/* ਫ਼ */
	0x0a8d,	/* ઍ */
	0x0ae0,	/* ૠ */
	0x0b9c,	/* ஜ */
	0x0cde,	/* ೞ */
	0x0e4f,	/* ๏ */
	0x0e84,	/* ຄ */
	0x0e8a,	/* ຊ */
	0x0e8d,	/* ຍ */
	0x0ea5,	/* ລ */
	0x0ea7,	/* ວ */
	0x0eb0,	/* ະ */
	0x0ebd,	/* ຽ */
	0x1fbe,	/* ι */
	0x207f,	/* ⁿ */
	0x20a8,	/* ₨ */
	0x2102,	/* ℂ */
	0x2107,	/* ℇ */
	0x2124,	/* ℤ */
	0x2126,	/* Ω */
	0x2128,	/* ℨ */
	0xfb3e,	/* מּ */
	0xfe74,	/* ﹴ */
};

/*
 * space ranges
 */
static
Rune	__space2[] =
{
	0x0009,	0x000a,	/* tab and newline */
	0x0020,	0x0020,	/* space */
	0x00a0,	0x00a0,	/*   */
	0x2000,	0x200b,	/*   - ​ */
	0x2028,	0x2029,	/*   -   */
	0x3000,	0x3000,	/* 　 */
	0xfeff,	0xfeff,	/* ﻿ */
};

/*
 * lower case ranges
 *	3rd col is conversion excess 500
 */
static
Rune	__toupper2[] =
{
	0x0061,	0x007a, 468,	/* a-z A-Z */
	0x00e0,	0x00f6, 468,	/* à-ö À-Ö */
	0x00f8,	0x00fe, 468,	/* ø-þ Ø-Þ */
	0x0256,	0x0257, 295,	/* ɖ-ɗ Ɖ-Ɗ */
	0x0258,	0x0259, 298,	/* ɘ-ə Ǝ-Ə */
	0x028a,	0x028b, 283,	/* ʊ-ʋ Ʊ-Ʋ */
	0x03ad,	0x03af, 463,	/* έ-ί Έ-Ί */
	0x03b1,	0x03c1, 468,	/* α-ρ Α-Ρ */
	0x03c3,	0x03cb, 468,	/* σ-ϋ Σ-Ϋ */
	0x03cd,	0x03ce, 437,	/* ύ-ώ Ύ-Ώ */
	0x0430,	0x044f, 468,	/* а-я А-Я */
	0x0451,	0x045c, 420,	/* ё-ќ Ё-Ќ */
	0x045e,	0x045f, 420,	/* ў-џ Ў-Џ */
	0x0561,	0x0586, 452,	/* ա-ֆ Ա-Ֆ */
	0x1f00,	0x1f07, 508,	/* ἀ-ἇ Ἀ-Ἇ */
	0x1f10,	0x1f15, 508,	/* ἐ-ἕ Ἐ-Ἕ */
	0x1f20,	0x1f27, 508,	/* ἠ-ἧ Ἠ-Ἧ */
	0x1f30,	0x1f37, 508,	/* ἰ-ἷ Ἰ-Ἷ */
	0x1f40,	0x1f45, 508,	/* ὀ-ὅ Ὀ-Ὅ */
	0x1f60,	0x1f67, 508,	/* ὠ-ὧ Ὠ-Ὧ */
	0x1f70,	0x1f71, 574,	/* ὰ-ά Ὰ-Ά */
	0x1f72,	0x1f75, 586,	/* ὲ-ή Ὲ-Ή */
	0x1f76,	0x1f77, 600,	/* ὶ-ί Ὶ-Ί */
	0x1f78,	0x1f79, 628,	/* ὸ-ό Ὸ-Ό */
	0x1f7a,	0x1f7b, 612,	/* ὺ-ύ Ὺ-Ύ */
	0x1f7c,	0x1f7d, 626,	/* ὼ-ώ Ὼ-Ώ */
	0x1f80,	0x1f87, 508,	/* ᾀ-ᾇ ᾈ-ᾏ */
	0x1f90,	0x1f97, 508,	/* ᾐ-ᾗ ᾘ-ᾟ */
	0x1fa0,	0x1fa7, 508,	/* ᾠ-ᾧ ᾨ-ᾯ */
	0x1fb0,	0x1fb1, 508,	/* ᾰ-ᾱ Ᾰ-Ᾱ */
	0x1fd0,	0x1fd1, 508,	/* ῐ-ῑ Ῐ-Ῑ */
	0x1fe0,	0x1fe1, 508,	/* ῠ-ῡ Ῠ-Ῡ */
	0x2170,	0x217f, 484,	/* ⅰ-ⅿ Ⅰ-Ⅿ */
	0x24d0,	0x24e9, 474,	/* ⓐ-ⓩ Ⓐ-Ⓩ */
	0xff41,	0xff5a, 468,	/* ａ-ｚ Ａ-Ｚ */
};

/*
 * lower case singlets
 *	2nd col is conversion excess 500
 */
static
Rune	__toupper1[] =
{
	0x00ff, 621,	/* ÿ Ÿ */
	0x0101, 499,	/* ā Ā */
	0x0103, 499,	/* ă Ă */
	0x0105, 499,	/* ą Ą */
	0x0107, 499,	/* ć Ć */
	0x0109, 499,	/* ĉ Ĉ */
	0x010b, 499,	/* ċ Ċ */
	0x010d, 499,	/* č Č */
	0x010f, 499,	/* ď Ď */
	0x0111, 499,	/* đ Đ */
	0x0113, 499,	/* ē Ē */
	0x0115, 499,	/* ĕ Ĕ */
	0x0117, 499,	/* ė Ė */
	0x0119, 499,	/* ę Ę */
	0x011b, 499,	/* ě Ě */
	0x011d, 499,	/* ĝ Ĝ */
	0x011f, 499,	/* ğ Ğ */
	0x0121, 499,	/* ġ Ġ */
	0x0123, 499,	/* ģ Ģ */
	0x0125, 499,	/* ĥ Ĥ */
	0x0127, 499,	/* ħ Ħ */
	0x0129, 499,	/* ĩ Ĩ */
	0x012b, 499,	/* ī Ī */
	0x012d, 499,	/* ĭ Ĭ */
	0x012f, 499,	/* į Į */
	0x0131, 268,	/* ı I */
	0x0133, 499,	/* ĳ Ĳ */
	0x0135, 499,	/* ĵ Ĵ */
	0x0137, 499,	/* ķ Ķ */
	0x013a, 499,	/* ĺ Ĺ */
	0x013c, 499,	/* ļ Ļ */
	0x013e, 499,	/* ľ Ľ */
	0x0140, 499,	/* ŀ Ŀ */
	0x0142, 499,	/* ł Ł */
	0x0144, 499,	/* ń Ń */
	0x0146, 499,	/* ņ Ņ */
	0x0148, 499,	/* ň Ň */
	0x014b, 499,	/* ŋ Ŋ */
	0x014d, 499,	/* ō Ō */
	0x014f, 499,	/* ŏ Ŏ */
	0x0151, 499,	/* ő Ő */
	0x0153, 499,	/* œ Œ */
	0x0155, 499,	/* ŕ Ŕ */
	0x0157, 499,	/* ŗ Ŗ */
	0x0159, 499,	/* ř Ř */
	0x015b, 499,	/* ś Ś */
	0x015d, 499,	/* ŝ Ŝ */
	0x015f, 499,	/* ş Ş */
	0x0161, 499,	/* š Š */
	0x0163, 499,	/* ţ Ţ */
	0x0165, 499,	/* ť Ť */
	0x0167, 499,	/* ŧ Ŧ */
	0x0169, 499,	/* ũ Ũ */
	0x016b, 499,	/* ū Ū */
	0x016d, 499,	/* ŭ Ŭ */
	0x016f, 499,	/* ů Ů */
	0x0171, 499,	/* ű Ű */
	0x0173, 499,	/* ų Ų */
	0x0175, 499,	/* ŵ Ŵ */
	0x0177, 499,	/* ŷ Ŷ */
	0x017a, 499,	/* ź Ź */
	0x017c, 499,	/* ż Ż */
	0x017e, 499,	/* ž Ž */
	0x017f, 200,	/* ſ S */
	0x0183, 499,	/* ƃ Ƃ */
	0x0185, 499,	/* ƅ Ƅ */
	0x0188, 499,	/* ƈ Ƈ */
	0x018c, 499,	/* ƌ Ƌ */
	0x0192, 499,	/* ƒ Ƒ */
	0x0199, 499,	/* ƙ Ƙ */
	0x01a1, 499,	/* ơ Ơ */
	0x01a3, 499,	/* ƣ Ƣ */
	0x01a5, 499,	/* ƥ Ƥ */
	0x01a8, 499,	/* ƨ Ƨ */
	0x01ad, 499,	/* ƭ Ƭ */
	0x01b0, 499,	/* ư Ư */
	0x01b4, 499,	/* ƴ Ƴ */
	0x01b6, 499,	/* ƶ Ƶ */
	0x01b9, 499,	/* ƹ Ƹ */
	0x01bd, 499,	/* ƽ Ƽ */
	0x01c5, 499,	/* ǅ Ǆ */
	0x01c6, 498,	/* ǆ Ǆ */
	0x01c8, 499,	/* ǈ Ǉ */
	0x01c9, 498,	/* ǉ Ǉ */
	0x01cb, 499,	/* ǋ Ǌ */
	0x01cc, 498,	/* ǌ Ǌ */
	0x01ce, 499,	/* ǎ Ǎ */
	0x01d0, 499,	/* ǐ Ǐ */
	0x01d2, 499,	/* ǒ Ǒ */
	0x01d4, 499,	/* ǔ Ǔ */
	0x01d6, 499,	/* ǖ Ǖ */
	0x01d8, 499,	/* ǘ Ǘ */
	0x01da, 499,	/* ǚ Ǚ */
	0x01dc, 499,	/* ǜ Ǜ */
	0x01df, 499,	/* ǟ Ǟ */
	0x01e1, 499,	/* ǡ Ǡ */
	0x01e3, 499,	/* ǣ Ǣ */
	0x01e5, 499,	/* ǥ Ǥ */
	0x01e7, 499,	/* ǧ Ǧ */
	0x01e9, 499,	/* ǩ Ǩ */
	0x01eb, 499,	/* ǫ Ǫ */
	0x01ed, 499,	/* ǭ Ǭ */
	0x01ef, 499,	/* ǯ Ǯ */
	0x01f2, 499,	/* ǲ Ǳ */
	0x01f3, 498,	/* ǳ Ǳ */
	0x01f5, 499,	/* ǵ Ǵ */
	0x01fb, 499,	/* ǻ Ǻ */
	0x01fd, 499,	/* ǽ Ǽ */
	0x01ff, 499,	/* ǿ Ǿ */
	0x0201, 499,	/* ȁ Ȁ */
	0x0203, 499,	/* ȃ Ȃ */
	0x0205, 499,	/* ȅ Ȅ */
	0x0207, 499,	/* ȇ Ȇ */
	0x0209, 499,	/* ȉ Ȉ */
	0x020b, 499,	/* ȋ Ȋ */
	0x020d, 499,	/* ȍ Ȍ */
	0x020f, 499,	/* ȏ Ȏ */
	0x0211, 499,	/* ȑ Ȑ */
	0x0213, 499,	/* ȓ Ȓ */
	0x0215, 499,	/* ȕ Ȕ */
	0x0217, 499,	/* ȗ Ȗ */
	0x0253, 290,	/* ɓ Ɓ */
	0x0254, 294,	/* ɔ Ɔ */
	0x025b, 297,	/* ɛ Ɛ */
	0x0260, 295,	/* ɠ Ɠ */
	0x0263, 293,	/* ɣ Ɣ */
	0x0268, 291,	/* ɨ Ɨ */
	0x0269, 289,	/* ɩ Ɩ */
	0x026f, 289,	/* ɯ Ɯ */
	0x0272, 287,	/* ɲ Ɲ */
	0x0283, 282,	/* ʃ Ʃ */
	0x0288, 282,	/* ʈ Ʈ */
	0x0292, 281,	/* ʒ Ʒ */
	0x03ac, 462,	/* ά Ά */
	0x03cc, 436,	/* ό Ό */
	0x03d0, 438,	/* ϐ Β */
	0x03d1, 443,	/* ϑ Θ */
	0x03d5, 453,	/* ϕ Φ */
	0x03d6, 446,	/* ϖ Π */
	0x03e3, 499,	/* ϣ Ϣ */
	0x03e5, 499,	/* ϥ Ϥ */
	0x03e7, 499,	/* ϧ Ϧ */
	0x03e9, 499,	/* ϩ Ϩ */
	0x03eb, 499,	/* ϫ Ϫ */
	0x03ed, 499,	/* ϭ Ϭ */
	0x03ef, 499,	/* ϯ Ϯ */
	0x03f0, 414,	/* ϰ Κ */
	0x03f1, 420,	/* ϱ Ρ */
	0x0461, 499,	/* ѡ Ѡ */
	0x0463, 499,	/* ѣ Ѣ */
	0x0465, 499,	/* ѥ Ѥ */
	0x0467, 499,	/* ѧ Ѧ */
	0x0469, 499,	/* ѩ Ѩ */
	0x046b, 499,	/* ѫ Ѫ */
	0x046d, 499,	/* ѭ Ѭ */
	0x046f, 499,	/* ѯ Ѯ */
	0x0471, 499,	/* ѱ Ѱ */
	0x0473, 499,	/* ѳ Ѳ */
	0x0475, 499,	/* ѵ Ѵ */
	0x0477, 499,	/* ѷ Ѷ */
	0x0479, 499,	/* ѹ Ѹ */
	0x047b, 499,	/* ѻ Ѻ */
	0x047d, 499,	/* ѽ Ѽ */
	0x047f, 499,	/* ѿ Ѿ */
	0x0481, 499,	/* ҁ Ҁ */
	0x0491, 499,	/* ґ Ґ */
	0x0493, 499,	/* ғ Ғ */
	0x0495, 499,	/* ҕ Ҕ */
	0x0497, 499,	/* җ Җ */
	0x0499, 499,	/* ҙ Ҙ */
	0x049b, 499,	/* қ Қ */
	0x049d, 499,	/* ҝ Ҝ */
	0x049f, 499,	/* ҟ Ҟ */
	0x04a1, 499,	/* ҡ Ҡ */
	0x04a3, 499,	/* ң Ң */
	0x04a5, 499,	/* ҥ Ҥ */
	0x04a7, 499,	/* ҧ Ҧ */
	0x04a9, 499,	/* ҩ Ҩ */
	0x04ab, 499,	/* ҫ Ҫ */
	0x04ad, 499,	/* ҭ Ҭ */
	0x04af, 499,	/* ү Ү */
	0x04b1, 499,	/* ұ Ұ */
	0x04b3, 499,	/* ҳ Ҳ */
	0x04b5, 499,	/* ҵ Ҵ */
	0x04b7, 499,	/* ҷ Ҷ */
	0x04b9, 499,	/* ҹ Ҹ */
	0x04bb, 499,	/* һ Һ */
	0x04bd, 499,	/* ҽ Ҽ */
	0x04bf, 499,	/* ҿ Ҿ */
	0x04c2, 499,	/* ӂ Ӂ */
	0x04c4, 499,	/* ӄ Ӄ */
	0x04c8, 499,	/* ӈ Ӈ */
	0x04cc, 499,	/* ӌ Ӌ */
	0x04d1, 499,	/* ӑ Ӑ */
	0x04d3, 499,	/* ӓ Ӓ */
	0x04d5, 499,	/* ӕ Ӕ */
	0x04d7, 499,	/* ӗ Ӗ */
	0x04d9, 499,	/* ә Ә */
	0x04db, 499,	/* ӛ Ӛ */
	0x04dd, 499,	/* ӝ Ӝ */
	0x04df, 499,	/* ӟ Ӟ */
	0x04e1, 499,	/* ӡ Ӡ */
	0x04e3, 499,	/* ӣ Ӣ */
	0x04e5, 499,	/* ӥ Ӥ */
	0x04e7, 499,	/* ӧ Ӧ */
	0x04e9, 499,	/* ө Ө */
	0x04eb, 499,	/* ӫ Ӫ */
	0x04ef, 499,	/* ӯ Ӯ */
	0x04f1, 499,	/* ӱ Ӱ */
	0x04f3, 499,	/* ӳ Ӳ */
	0x04f5, 499,	/* ӵ Ӵ */
	0x04f9, 499,	/* ӹ Ӹ */
	0x1e01, 499,	/* ḁ Ḁ */
	0x1e03, 499,	/* ḃ Ḃ */
	0x1e05, 499,	/* ḅ Ḅ */
	0x1e07, 499,	/* ḇ Ḇ */
	0x1e09, 499,	/* ḉ Ḉ */
	0x1e0b, 499,	/* ḋ Ḋ */
	0x1e0d, 499,	/* ḍ Ḍ */
	0x1e0f, 499,	/* ḏ Ḏ */
	0x1e11, 499,	/* ḑ Ḑ */
	0x1e13, 499,	/* ḓ Ḓ */
	0x1e15, 499,	/* ḕ Ḕ */
	0x1e17, 499,	/* ḗ Ḗ */
	0x1e19, 499,	/* ḙ Ḙ */
	0x1e1b, 499,	/* ḛ Ḛ */
	0x1e1d, 499,	/* ḝ Ḝ */
	0x1e1f, 499,	/* ḟ Ḟ */
	0x1e21, 499,	/* ḡ Ḡ */
	0x1e23, 499,	/* ḣ Ḣ */
	0x1e25, 499,	/* ḥ Ḥ */
	0x1e27, 499,	/* ḧ Ḧ */
	0x1e29, 499,	/* ḩ Ḩ */
	0x1e2b, 499,	/* ḫ Ḫ */
	0x1e2d, 499,	/* ḭ Ḭ */
	0x1e2f, 499,	/* ḯ Ḯ */
	0x1e31, 499,	/* ḱ Ḱ */
	0x1e33, 499,	/* ḳ Ḳ */
	0x1e35, 499,	/* ḵ Ḵ */
	0x1e37, 499,	/* ḷ Ḷ */
	0x1e39, 499,	/* ḹ Ḹ */
	0x1e3b, 499,	/* ḻ Ḻ */
	0x1e3d, 499,	/* ḽ Ḽ */
	0x1e3f, 499,	/* ḿ Ḿ */
	0x1e41, 499,	/* ṁ Ṁ */
	0x1e43, 499,	/* ṃ Ṃ */
	0x1e45, 499,	/* ṅ Ṅ */
	0x1e47, 499,	/* ṇ Ṇ */
	0x1e49, 499,	/* ṉ Ṉ */
	0x1e4b, 499,	/* ṋ Ṋ */
	0x1e4d, 499,	/* ṍ Ṍ */
	0x1e4f, 499,	/* ṏ Ṏ */
	0x1e51, 499,	/* ṑ Ṑ */
	0x1e53, 499,	/* ṓ Ṓ */
	0x1e55, 499,	/* ṕ Ṕ */
	0x1e57, 499,	/* ṗ Ṗ */
	0x1e59, 499,	/* ṙ Ṙ */
	0x1e5b, 499,	/* ṛ Ṛ */
	0x1e5d, 499,	/* ṝ Ṝ */
	0x1e5f, 499,	/* ṟ Ṟ */
	0x1e61, 499,	/* ṡ Ṡ */
	0x1e63, 499,	/* ṣ Ṣ */
	0x1e65, 499,	/* ṥ Ṥ */
	0x1e67, 499,	/* ṧ Ṧ */
	0x1e69, 499,	/* ṩ Ṩ */
	0x1e6b, 499,	/* ṫ Ṫ */
	0x1e6d, 499,	/* ṭ Ṭ */
	0x1e6f, 499,	/* ṯ Ṯ */
	0x1e71, 499,	/* ṱ Ṱ */
	0x1e73, 499,	/* ṳ Ṳ */
	0x1e75, 499,	/* ṵ Ṵ */
	0x1e77, 499,	/* ṷ Ṷ */
	0x1e79, 499,	/* ṹ Ṹ */
	0x1e7b, 499,	/* ṻ Ṻ */
	0x1e7d, 499,	/* ṽ Ṽ */
	0x1e7f, 499,	/* ṿ Ṿ */
	0x1e81, 499,	/* ẁ Ẁ */
	0x1e83, 499,	/* ẃ Ẃ */
	0x1e85, 499,	/* ẅ Ẅ */
	0x1e87, 499,	/* ẇ Ẇ */
	0x1e89, 499,	/* ẉ Ẉ */
	0x1e8b, 499,	/* ẋ Ẋ */
	0x1e8d, 499,	/* ẍ Ẍ */
	0x1e8f, 499,	/* ẏ Ẏ */
	0x1e91, 499,	/* ẑ Ẑ */
	0x1e93, 499,	/* ẓ Ẓ */
	0x1e95, 499,	/* ẕ Ẕ */
	0x1ea1, 499,	/* ạ Ạ */
	0x1ea3, 499,	/* ả Ả */
	0x1ea5, 499,	/* ấ Ấ */
	0x1ea7, 499,	/* ầ Ầ */
	0x1ea9, 499,	/* ẩ Ẩ */
	0x1eab, 499,	/* ẫ Ẫ */
	0x1ead, 499,	/* ậ Ậ */
	0x1eaf, 499,	/* ắ Ắ */
	0x1eb1, 499,	/* ằ Ằ */
	0x1eb3, 499,	/* ẳ Ẳ */
	0x1eb5, 499,	/* ẵ Ẵ */
	0x1eb7, 499,	/* ặ Ặ */
	0x1eb9, 499,	/* ẹ Ẹ */
	0x1ebb, 499,	/* ẻ Ẻ */
	0x1ebd, 499,	/* ẽ Ẽ */
	0x1ebf, 499,	/* ế Ế */
	0x1ec1, 499,	/* ề Ề */
	0x1ec3, 499,	/* ể Ể */
	0x1ec5, 499,	/* ễ Ễ */
	0x1ec7, 499,	/* ệ Ệ */
	0x1ec9, 499,	/* ỉ Ỉ */
	0x1ecb, 499,	/* ị Ị */
	0x1ecd, 499,	/* ọ Ọ */
	0x1ecf, 499,	/* ỏ Ỏ */
	0x1ed1, 499,	/* ố Ố */
	0x1ed3, 499,	/* ồ Ồ */
	0x1ed5, 499,	/* ổ Ổ */
	0x1ed7, 499,	/* ỗ Ỗ */
	0x1ed9, 499,	/* ộ Ộ */
	0x1edb, 499,	/* ớ Ớ */
	0x1edd, 499,	/* ờ Ờ */
	0x1edf, 499,	/* ở Ở */
	0x1ee1, 499,	/* ỡ Ỡ */
	0x1ee3, 499,	/* ợ Ợ */
	0x1ee5, 499,	/* ụ Ụ */
	0x1ee7, 499,	/* ủ Ủ */
	0x1ee9, 499,	/* ứ Ứ */
	0x1eeb, 499,	/* ừ Ừ */
	0x1eed, 499,	/* ử Ử */
	0x1eef, 499,	/* ữ Ữ */
	0x1ef1, 499,	/* ự Ự */
	0x1ef3, 499,	/* ỳ Ỳ */
	0x1ef5, 499,	/* ỵ Ỵ */
	0x1ef7, 499,	/* ỷ Ỷ */
	0x1ef9, 499,	/* ỹ Ỹ */
	0x1f51, 508,	/* ὑ Ὑ */
	0x1f53, 508,	/* ὓ Ὓ */
	0x1f55, 508,	/* ὕ Ὕ */
	0x1f57, 508,	/* ὗ Ὗ */
	0x1fb3, 509,	/* ᾳ ᾼ */
	0x1fc3, 509,	/* ῃ ῌ */
	0x1fe5, 507,	/* ῥ Ῥ */
	0x1ff3, 509,	/* ῳ ῼ */
};

/*
 * upper case ranges
 *	3rd col is conversion excess 500
 */
static
Rune	__tolower2[] =
{
	0x0041,	0x005a, 532,	/* A-Z a-z */
	0x00c0,	0x00d6, 532,	/* À-Ö à-ö */
	0x00d8,	0x00de, 532,	/* Ø-Þ ø-þ */
	0x0189,	0x018a, 705,	/* Ɖ-Ɗ ɖ-ɗ */
	0x018e,	0x018f, 702,	/* Ǝ-Ə ɘ-ə */
	0x01b1,	0x01b2, 717,	/* Ʊ-Ʋ ʊ-ʋ */
	0x0388,	0x038a, 537,	/* Έ-Ί έ-ί */
	0x038e,	0x038f, 563,	/* Ύ-Ώ ύ-ώ */
	0x0391,	0x03a1, 532,	/* Α-Ρ α-ρ */
	0x03a3,	0x03ab, 532,	/* Σ-Ϋ σ-ϋ */
	0x0401,	0x040c, 580,	/* Ё-Ќ ё-ќ */
	0x040e,	0x040f, 580,	/* Ў-Џ ў-џ */
	0x0410,	0x042f, 532,	/* А-Я а-я */
	0x0531,	0x0556, 548,	/* Ա-Ֆ ա-ֆ */
	0x10a0,	0x10c5, 548,	/* Ⴀ-Ⴥ ა-ჵ */
	0x1f08,	0x1f0f, 492,	/* Ἀ-Ἇ ἀ-ἇ */
	0x1f18,	0x1f1d, 492,	/* Ἐ-Ἕ ἐ-ἕ */
	0x1f28,	0x1f2f, 492,	/* Ἠ-Ἧ ἠ-ἧ */
	0x1f38,	0x1f3f, 492,	/* Ἰ-Ἷ ἰ-ἷ */
	0x1f48,	0x1f4d, 492,	/* Ὀ-Ὅ ὀ-ὅ */
	0x1f68,	0x1f6f, 492,	/* Ὠ-Ὧ ὠ-ὧ */
	0x1f88,	0x1f8f, 492,	/* ᾈ-ᾏ ᾀ-ᾇ */
	0x1f98,	0x1f9f, 492,	/* ᾘ-ᾟ ᾐ-ᾗ */
	0x1fa8,	0x1faf, 492,	/* ᾨ-ᾯ ᾠ-ᾧ */
	0x1fb8,	0x1fb9, 492,	/* Ᾰ-Ᾱ ᾰ-ᾱ */
	0x1fba,	0x1fbb, 426,	/* Ὰ-Ά ὰ-ά */
	0x1fc8,	0x1fcb, 414,	/* Ὲ-Ή ὲ-ή */
	0x1fd8,	0x1fd9, 492,	/* Ῐ-Ῑ ῐ-ῑ */
	0x1fda,	0x1fdb, 400,	/* Ὶ-Ί ὶ-ί */
	0x1fe8,	0x1fe9, 492,	/* Ῠ-Ῡ ῠ-ῡ */
	0x1fea,	0x1feb, 388,	/* Ὺ-Ύ ὺ-ύ */
	0x1ff8,	0x1ff9, 372,	/* Ὸ-Ό ὸ-ό */
	0x1ffa,	0x1ffb, 374,	/* Ὼ-Ώ ὼ-ώ */
	0x2160,	0x216f, 516,	/* Ⅰ-Ⅿ ⅰ-ⅿ */
	0x24b6,	0x24cf, 526,	/* Ⓐ-Ⓩ ⓐ-ⓩ */
	0xff21,	0xff3a, 532,	/* Ａ-Ｚ ａ-ｚ */
};

/*
 * upper case singlets
 *	2nd col is conversion excess 500
 */
static
Rune	__tolower1[] =
{
	0x0100, 501,	/* Ā ā */
	0x0102, 501,	/* Ă ă */
	0x0104, 501,	/* Ą ą */
	0x0106, 501,	/* Ć ć */
	0x0108, 501,	/* Ĉ ĉ */
	0x010a, 501,	/* Ċ ċ */
	0x010c, 501,	/* Č č */
	0x010e, 501,	/* Ď ď */
	0x0110, 501,	/* Đ đ */
	0x0112, 501,	/* Ē ē */
	0x0114, 501,	/* Ĕ ĕ */
	0x0116, 501,	/* Ė ė */
	0x0118, 501,	/* Ę ę */
	0x011a, 501,	/* Ě ě */
	0x011c, 501,	/* Ĝ ĝ */
	0x011e, 501,	/* Ğ ğ */
	0x0120, 501,	/* Ġ ġ */
	0x0122, 501,	/* Ģ ģ */
	0x0124, 501,	/* Ĥ ĥ */
	0x0126, 501,	/* Ħ ħ */
	0x0128, 501,	/* Ĩ ĩ */
	0x012a, 501,	/* Ī ī */
	0x012c, 501,	/* Ĭ ĭ */
	0x012e, 501,	/* Į į */
	0x0130, 301,	/* İ i */
	0x0132, 501,	/* Ĳ ĳ */
	0x0134, 501,	/* Ĵ ĵ */
	0x0136, 501,	/* Ķ ķ */
	0x0139, 501,	/* Ĺ ĺ */
	0x013b, 501,	/* Ļ ļ */
	0x013d, 501,	/* Ľ ľ */
	0x013f, 501,	/* Ŀ ŀ */
	0x0141, 501,	/* Ł ł */
	0x0143, 501,	/* Ń ń */
	0x0145, 501,	/* Ņ ņ */
	0x0147, 501,	/* Ň ň */
	0x014a, 501,	/* Ŋ ŋ */
	0x014c, 501,	/* Ō ō */
	0x014e, 501,	/* Ŏ ŏ */
	0x0150, 501,	/* Ő ő */
	0x0152, 501,	/* Œ œ */
	0x0154, 501,	/* Ŕ ŕ */
	0x0156, 501,	/* Ŗ ŗ */
	0x0158, 501,	/* Ř ř */
	0x015a, 501,	/* Ś ś */
	0x015c, 501,	/* Ŝ ŝ */
	0x015e, 501,	/* Ş ş */
	0x0160, 501,	/* Š š */
	0x0162, 501,	/* Ţ ţ */
	0x0164, 501,	/* Ť ť */
	0x0166, 501,	/* Ŧ ŧ */
	0x0168, 501,	/* Ũ ũ */
	0x016a, 501,	/* Ū ū */
	0x016c, 501,	/* Ŭ ŭ */
	0x016e, 501,	/* Ů ů */
	0x0170, 501,	/* Ű ű */
	0x0172, 501,	/* Ų ų */
	0x0174, 501,	/* Ŵ ŵ */
	0x0176, 501,	/* Ŷ ŷ */
	0x0178, 379,	/* Ÿ ÿ */
	0x0179, 501,	/* Ź ź */
	0x017b, 501,	/* Ż ż */
	0x017d, 501,	/* Ž ž */
	0x0181, 710,	/* Ɓ ɓ */
	0x0182, 501,	/* Ƃ ƃ */
	0x0184, 501,	/* Ƅ ƅ */
	0x0186, 706,	/* Ɔ ɔ */
	0x0187, 501,	/* Ƈ ƈ */
	0x018b, 501,	/* Ƌ ƌ */
	0x0190, 703,	/* Ɛ ɛ */
	0x0191, 501,	/* Ƒ ƒ */
	0x0193, 705,	/* Ɠ ɠ */
	0x0194, 707,	/* Ɣ ɣ */
	0x0196, 711,	/* Ɩ ɩ */
	0x0197, 709,	/* Ɨ ɨ */
	0x0198, 501,	/* Ƙ ƙ */
	0x019c, 711,	/* Ɯ ɯ */
	0x019d, 713,	/* Ɲ ɲ */
	0x01a0, 501,	/* Ơ ơ */
	0x01a2, 501,	/* Ƣ ƣ */
	0x01a4, 501,	/* Ƥ ƥ */
	0x01a7, 501,	/* Ƨ ƨ */
	0x01a9, 718,	/* Ʃ ʃ */
	0x01ac, 501,	/* Ƭ ƭ */
	0x01ae, 718,	/* Ʈ ʈ */
	0x01af, 501,	/* Ư ư */
	0x01b3, 501,	/* Ƴ ƴ */
	0x01b5, 501,	/* Ƶ ƶ */
	0x01b7, 719,	/* Ʒ ʒ */
	0x01b8, 501,	/* Ƹ ƹ */
	0x01bc, 501,	/* Ƽ ƽ */
	0x01c4, 502,	/* Ǆ ǆ */
	0x01c5, 501,	/* ǅ ǆ */
	0x01c7, 502,	/* Ǉ ǉ */
	0x01c8, 501,	/* ǈ ǉ */
	0x01ca, 502,	/* Ǌ ǌ */
	0x01cb, 501,	/* ǋ ǌ */
	0x01cd, 501,	/* Ǎ ǎ */
	0x01cf, 501,	/* Ǐ ǐ */
	0x01d1, 501,	/* Ǒ ǒ */
	0x01d3, 501,	/* Ǔ ǔ */
	0x01d5, 501,	/* Ǖ ǖ */
	0x01d7, 501,	/* Ǘ ǘ */
	0x01d9, 501,	/* Ǚ ǚ */
	0x01db, 501,	/* Ǜ ǜ */
	0x01de, 501,	/* Ǟ ǟ */
	0x01e0, 501,	/* Ǡ ǡ */
	0x01e2, 501,	/* Ǣ ǣ */
	0x01e4, 501,	/* Ǥ ǥ */
	0x01e6, 501,	/* Ǧ ǧ */
	0x01e8, 501,	/* Ǩ ǩ */
	0x01ea, 501,	/* Ǫ ǫ */
	0x01ec, 501,	/* Ǭ ǭ */
	0x01ee, 501,	/* Ǯ ǯ */
	0x01f1, 502,	/* Ǳ ǳ */
	0x01f2, 501,	/* ǲ ǳ */
	0x01f4, 501,	/* Ǵ ǵ */
	0x01fa, 501,	/* Ǻ ǻ */
	0x01fc, 501,	/* Ǽ ǽ */
	0x01fe, 501,	/* Ǿ ǿ */
	0x0200, 501,	/* Ȁ ȁ */
	0x0202, 501,	/* Ȃ ȃ */
	0x0204, 501,	/* Ȅ ȅ */
	0x0206, 501,	/* Ȇ ȇ */
	0x0208, 501,	/* Ȉ ȉ */
	0x020a, 501,	/* Ȋ ȋ */
	0x020c, 501,	/* Ȍ ȍ */
	0x020e, 501,	/* Ȏ ȏ */
	0x0210, 501,	/* Ȑ ȑ */
	0x0212, 501,	/* Ȓ ȓ */
	0x0214, 501,	/* Ȕ ȕ */
	0x0216, 501,	/* Ȗ ȗ */
	0x0386, 538,	/* Ά ά */
	0x038c, 564,	/* Ό ό */
	0x03e2, 501,	/* Ϣ ϣ */
	0x03e4, 501,	/* Ϥ ϥ */
	0x03e6, 501,	/* Ϧ ϧ */
	0x03e8, 501,	/* Ϩ ϩ */
	0x03ea, 501,	/* Ϫ ϫ */
	0x03ec, 501,	/* Ϭ ϭ */
	0x03ee, 501,	/* Ϯ ϯ */
	0x0460, 501,	/* Ѡ ѡ */
	0x0462, 501,	/* Ѣ ѣ */
	0x0464, 501,	/* Ѥ ѥ */
	0x0466, 501,	/* Ѧ ѧ */
	0x0468, 501,	/* Ѩ ѩ */
	0x046a, 501,	/* Ѫ ѫ */
	0x046c, 501,	/* Ѭ ѭ */
	0x046e, 501,	/* Ѯ ѯ */
	0x0470, 501,	/* Ѱ ѱ */
	0x0472, 501,	/* Ѳ ѳ */
	0x0474, 501,	/* Ѵ ѵ */
	0x0476, 501,	/* Ѷ ѷ */
	0x0478, 501,	/* Ѹ ѹ */
	0x047a, 501,	/* Ѻ ѻ */
	0x047c, 501,	/* Ѽ ѽ */
	0x047e, 501,	/* Ѿ ѿ */
	0x0480, 501,	/* Ҁ ҁ */
	0x0490, 501,	/* Ґ ґ */
	0x0492, 501,	/* Ғ ғ */
	0x0494, 501,	/* Ҕ ҕ */
	0x0496, 501,	/* Җ җ */
	0x0498, 501,	/* Ҙ ҙ */
	0x049a, 501,	/* Қ қ */
	0x049c, 501,	/* Ҝ ҝ */
	0x049e, 501,	/* Ҟ ҟ */
	0x04a0, 501,	/* Ҡ ҡ */
	0x04a2, 501,	/* Ң ң */
	0x04a4, 501,	/* Ҥ ҥ */
	0x04a6, 501,	/* Ҧ ҧ */
	0x04a8, 501,	/* Ҩ ҩ */
	0x04aa, 501,	/* Ҫ ҫ */
	0x04ac, 501,	/* Ҭ ҭ */
	0x04ae, 501,	/* Ү ү */
	0x04b0, 501,	/* Ұ ұ */
	0x04b2, 501,	/* Ҳ ҳ */
	0x04b4, 501,	/* Ҵ ҵ */
	0x04b6, 501,	/* Ҷ ҷ */
	0x04b8, 501,	/* Ҹ ҹ */
	0x04ba, 501,	/* Һ һ */
	0x04bc, 501,	/* Ҽ ҽ */
	0x04be, 501,	/* Ҿ ҿ */
	0x04c1, 501,	/* Ӂ ӂ */
	0x04c3, 501,	/* Ӄ ӄ */
	0x04c7, 501,	/* Ӈ ӈ */
	0x04cb, 501,	/* Ӌ ӌ */
	0x04d0, 501,	/* Ӑ ӑ */
	0x04d2, 501,	/* Ӓ ӓ */
	0x04d4, 501,	/* Ӕ ӕ */
	0x04d6, 501,	/* Ӗ ӗ */
	0x04d8, 501,	/* Ә ә */
	0x04da, 501,	/* Ӛ ӛ */
	0x04dc, 501,	/* Ӝ ӝ */
	0x04de, 501,	/* Ӟ ӟ */
	0x04e0, 501,	/* Ӡ ӡ */
	0x04e2, 501,	/* Ӣ ӣ */
	0x04e4, 501,	/* Ӥ ӥ */
	0x04e6, 501,	/* Ӧ ӧ */
	0x04e8, 501,	/* Ө ө */
	0x04ea, 501,	/* Ӫ ӫ */
	0x04ee, 501,	/* Ӯ ӯ */
	0x04f0, 501,	/* Ӱ ӱ */
	0x04f2, 501,	/* Ӳ ӳ */
	0x04f4, 501,	/* Ӵ ӵ */
	0x04f8, 501,	/* Ӹ ӹ */
	0x1e00, 501,	/* Ḁ ḁ */
	0x1e02, 501,	/* Ḃ ḃ */
	0x1e04, 501,	/* Ḅ ḅ */
	0x1e06, 501,	/* Ḇ ḇ */
	0x1e08, 501,	/* Ḉ ḉ */
	0x1e0a, 501,	/* Ḋ ḋ */
	0x1e0c, 501,	/* Ḍ ḍ */
	0x1e0e, 501,	/* Ḏ ḏ */
	0x1e10, 501,	/* Ḑ ḑ */
	0x1e12, 501,	/* Ḓ ḓ */
	0x1e14, 501,	/* Ḕ ḕ */
	0x1e16, 501,	/* Ḗ ḗ */
	0x1e18, 501,	/* Ḙ ḙ */
	0x1e1a, 501,	/* Ḛ ḛ */
	0x1e1c, 501,	/* Ḝ ḝ */
	0x1e1e, 501,	/* Ḟ ḟ */
	0x1e20, 501,	/* Ḡ ḡ */
	0x1e22, 501,	/* Ḣ ḣ */
	0x1e24, 501,	/* Ḥ ḥ */
	0x1e26, 501,	/* Ḧ ḧ */
	0x1e28, 501,	/* Ḩ ḩ */
	0x1e2a, 501,	/* Ḫ ḫ */
	0x1e2c, 501,	/* Ḭ ḭ */
	0x1e2e, 501,	/* Ḯ ḯ */
	0x1e30, 501,	/* Ḱ ḱ */
	0x1e32, 501,	/* Ḳ ḳ */
	0x1e34, 501,	/* Ḵ ḵ */
	0x1e36, 501,	/* Ḷ ḷ */
	0x1e38, 501,	/* Ḹ ḹ */
	0x1e3a, 501,	/* Ḻ ḻ */
	0x1e3c, 501,	/* Ḽ ḽ */
	0x1e3e, 501,	/* Ḿ ḿ */
	0x1e40, 501,	/* Ṁ ṁ */
	0x1e42, 501,	/* Ṃ ṃ */
	0x1e44, 501,	/* Ṅ ṅ */
	0x1e46, 501,	/* Ṇ ṇ */
	0x1e48, 501,	/* Ṉ ṉ */
	0x1e4a, 501,	/* Ṋ ṋ */
	0x1e4c, 501,	/* Ṍ ṍ */
	0x1e4e, 501,	/* Ṏ ṏ */
	0x1e50, 501,	/* Ṑ ṑ */
	0x1e52, 501,	/* Ṓ ṓ */
	0x1e54, 501,	/* Ṕ ṕ */
	0x1e56, 501,	/* Ṗ ṗ */
	0x1e58, 501,	/* Ṙ ṙ */
	0x1e5a, 501,	/* Ṛ ṛ */
	0x1e5c, 501,	/* Ṝ ṝ */
	0x1e5e, 501,	/* Ṟ ṟ */
	0x1e60, 501,	/* Ṡ ṡ */
	0x1e62, 501,	/* Ṣ ṣ */
	0x1e64, 501,	/* Ṥ ṥ */
	0x1e66, 501,	/* Ṧ ṧ */
	0x1e68, 501,	/* Ṩ ṩ */
	0x1e6a, 501,	/* Ṫ ṫ */
	0x1e6c, 501,	/* Ṭ ṭ */
	0x1e6e, 501,	/* Ṯ ṯ */
	0x1e70, 501,	/* Ṱ ṱ */
	0x1e72, 501,	/* Ṳ ṳ */
	0x1e74, 501,	/* Ṵ ṵ */
	0x1e76, 501,	/* Ṷ ṷ */
	0x1e78, 501,	/* Ṹ ṹ */
	0x1e7a, 501,	/* Ṻ ṻ */
	0x1e7c, 501,	/* Ṽ ṽ */
	0x1e7e, 501,	/* Ṿ ṿ */
	0x1e80, 501,	/* Ẁ ẁ */
	0x1e82, 501,	/* Ẃ ẃ */
	0x1e84, 501,	/* Ẅ ẅ */
	0x1e86, 501,	/* Ẇ ẇ */
	0x1e88, 501,	/* Ẉ ẉ */
	0x1e8a, 501,	/* Ẋ ẋ */
	0x1e8c, 501,	/* Ẍ ẍ */
	0x1e8e, 501,	/* Ẏ ẏ */
	0x1e90, 501,	/* Ẑ ẑ */
	0x1e92, 501,	/* Ẓ ẓ */
	0x1e94, 501,	/* Ẕ ẕ */
	0x1ea0, 501,	/* Ạ ạ */
	0x1ea2, 501,	/* Ả ả */
	0x1ea4, 501,	/* Ấ ấ */
	0x1ea6, 501,	/* Ầ ầ */
	0x1ea8, 501,	/* Ẩ ẩ */
	0x1eaa, 501,	/* Ẫ ẫ */
	0x1eac, 501,	/* Ậ ậ */
	0x1eae, 501,	/* Ắ ắ */
	0x1eb0, 501,	/* Ằ ằ */
	0x1eb2, 501,	/* Ẳ ẳ */
	0x1eb4, 501,	/* Ẵ ẵ */
	0x1eb6, 501,	/* Ặ ặ */
	0x1eb8, 501,	/* Ẹ ẹ */
	0x1eba, 501,	/* Ẻ ẻ */
	0x1ebc, 501,	/* Ẽ ẽ */
	0x1ebe, 501,	/* Ế ế */
	0x1ec0, 501,	/* Ề ề */
	0x1ec2, 501,	/* Ể ể */
	0x1ec4, 501,	/* Ễ ễ */
	0x1ec6, 501,	/* Ệ ệ */
	0x1ec8, 501,	/* Ỉ ỉ */
	0x1eca, 501,	/* Ị ị */
	0x1ecc, 501,	/* Ọ ọ */
	0x1ece, 501,	/* Ỏ ỏ */
	0x1ed0, 501,	/* Ố ố */
	0x1ed2, 501,	/* Ồ ồ */
	0x1ed4, 501,	/* Ổ ổ */
	0x1ed6, 501,	/* Ỗ ỗ */
	0x1ed8, 501,	/* Ộ ộ */
	0x1eda, 501,	/* Ớ ớ */
	0x1edc, 501,	/* Ờ ờ */
	0x1ede, 501,	/* Ở ở */
	0x1ee0, 501,	/* Ỡ ỡ */
	0x1ee2, 501,	/* Ợ ợ */
	0x1ee4, 501,	/* Ụ ụ */
	0x1ee6, 501,	/* Ủ ủ */
	0x1ee8, 501,	/* Ứ ứ */
	0x1eea, 501,	/* Ừ ừ */
	0x1eec, 501,	/* Ử ử */
	0x1eee, 501,	/* Ữ ữ */
	0x1ef0, 501,	/* Ự ự */
	0x1ef2, 501,	/* Ỳ ỳ */
	0x1ef4, 501,	/* Ỵ ỵ */
	0x1ef6, 501,	/* Ỷ ỷ */
	0x1ef8, 501,	/* Ỹ ỹ */
	0x1f59, 492,	/* Ὑ ὑ */
	0x1f5b, 492,	/* Ὓ ὓ */
	0x1f5d, 492,	/* Ὕ ὕ */
	0x1f5f, 492,	/* Ὗ ὗ */
	0x1fbc, 491,	/* ᾼ ᾳ */
	0x1fcc, 491,	/* ῌ ῃ */
	0x1fec, 493,	/* Ῥ ῥ */
	0x1ffc, 491,	/* ῼ ῳ */
};

/*
 * title characters are those between
 * upper and lower case. ie DZ Dz dz
 */
static
Rune	__totitle1[] =
{
	0x01c4, 501,	/* Ǆ ǅ */
	0x01c6, 499,	/* ǆ ǅ */
	0x01c7, 501,	/* Ǉ ǈ */
	0x01c9, 499,	/* ǉ ǈ */
	0x01ca, 501,	/* Ǌ ǋ */
	0x01cc, 499,	/* ǌ ǋ */
	0x01f1, 501,	/* Ǳ ǲ */
	0x01f3, 499,	/* ǳ ǲ */
};

static Rune*
rune_bsearch(Rune c, Rune *t, int n, int ne)
{
	Rune *p;
	int m;

	while(n > 1) {
		m = n/2;
		p = t + m*ne;
		if(c >= p[0]) {
			t = p;
			n = n-m;
		} else
			n = m;
	}
	if(n && c >= t[0])
		return t;
	return 0;
}

Rune
tolowerrune(Rune c)
{
	Rune *p;

	p = rune_bsearch(c, __tolower2, nelem(__tolower2)/3, 3);
	if(p && c >= p[0] && c <= p[1])
		return c + p[2] - 500;
	p = rune_bsearch(c, __tolower1, nelem(__tolower1)/2, 2);
	if(p && c == p[0])
		return c + p[1] - 500;
	return c;
}

Rune
toupperrune(Rune c)
{
	Rune *p;

	p = rune_bsearch(c, __toupper2, nelem(__toupper2)/3, 3);
	if(p && c >= p[0] && c <= p[1])
		return c + p[2] - 500;
	p = rune_bsearch(c, __toupper1, nelem(__toupper1)/2, 2);
	if(p && c == p[0])
		return c + p[1] - 500;
	return c;
}

Rune
totitlerune(Rune c)
{
	Rune *p;

	p = rune_bsearch(c, __totitle1, nelem(__totitle1)/2, 2);
	if(p && c == p[0])
		return c + p[1] - 500;
	return c;
}

int
islowerrune(Rune c)
{
	Rune *p;

	p = rune_bsearch(c, __toupper2, nelem(__toupper2)/3, 3);
	if(p && c >= p[0] && c <= p[1])
		return 1;
	p = rune_bsearch(c, __toupper1, nelem(__toupper1)/2, 2);
	if(p && c == p[0])
		return 1;
	return 0;
}

int
isupperrune(Rune c)
{
	Rune *p;

	p = rune_bsearch(c, __tolower2, nelem(__tolower2)/3, 3);
	if(p && c >= p[0] && c <= p[1])
		return 1;
	p = rune_bsearch(c, __tolower1, nelem(__tolower1)/2, 2);
	if(p && c == p[0])
		return 1;
	return 0;
}

int isdigitrune(Rune c){return c >= '0' && c <= '9';}

int isnewline(Rune c){return c == 0xA || c == 0xD || c == 0x2028 || c == 0x2029;}

int iswordchar(Rune c){return c == '_' || isdigitrune(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');}

int
isalpharune(Rune c)
{
	Rune *p;

	if(isupperrune(c) || islowerrune(c))
		return 1;
	p = rune_bsearch(c, __alpha2, nelem(__alpha2)/2, 2);
	if(p && c >= p[0] && c <= p[1])
		return 1;
	p = rune_bsearch(c, __alpha1, nelem(__alpha1), 1);
	if(p && c == p[0])
		return 1;
	return 0;
}

int
istitlerune(Rune c)
{
	return isupperrune(c) && islowerrune(c);
}

int
isspacerune(Rune c)
{
	Rune *p;

	p = rune_bsearch(c, __space2, nelem(__space2)/2, 2);
	if(p && c >= p[0] && c <= p[1])
		return 1;
	return 0;
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


/*
 * Strings in AST are encoded as tuples (length, string).
 * Length is variable-length: if high bit is set in a byte, next byte is used.
 * Maximum string length with such encoding is 2 ^ (7 * 4) == 256 MiB,
 * assuming that sizeof(size_t) == 4.
 * Small string length (less then 128 bytes) is encoded in 1 byte.
 */
V7_PRIVATE size_t decode_varint(const unsigned char *p, int *llen) {
  size_t i = 0, string_len = 0;

  do {
    /*
     * Each byte of varint contains 7 bits, in little endian order.
     * MSB is a continuation bit: it tells whether next byte is used.
     */
    string_len |= (p[i] & 0x7f) << (7 * i);
    /*
     * First we increment i, then check whether it is within boundary and
     * whether decoded byte had continuation bit set.
     */
  } while (++i < sizeof(size_t) && (p[i - 1] & 0x80));
  *llen = i;

  return string_len;
}

/* Return number of bytes to store length */
V7_PRIVATE int calc_llen(size_t len) {
  int n = 0;

  do {
    n++;
  } while (len >>= 7);

  assert(n <= (int) sizeof(len));

  return n;
}

V7_PRIVATE int encode_varint(size_t len, unsigned char *p) {
  int i, llen = calc_llen(len);

  for (i = 0; i < llen; i++) {
    p[i] = (len & 0x7f) | (i < llen - 1 ? 0x80 : 0);
    len >>= 7;
  }

  return llen;
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


/* Vector, describes some memory location pointed by 'p' with length 'len' */
struct v7_vec {
  const char *p;
  int len;
};
#define V7_VEC(str) \
  { (str), sizeof(str) - 1 }

/*
 * NOTE(lsm): Must be in the same order as enum for keywords. See comment
 * for function get_tok() for rationale for that.
 */
static struct v7_vec s_keywords[] = {
    V7_VEC("break"),      V7_VEC("case"),     V7_VEC("catch"),
    V7_VEC("continue"),   V7_VEC("debugger"), V7_VEC("default"),
    V7_VEC("delete"),     V7_VEC("do"),       V7_VEC("else"),
    V7_VEC("false"),      V7_VEC("finally"),  V7_VEC("for"),
    V7_VEC("function"),   V7_VEC("if"),       V7_VEC("in"),
    V7_VEC("instanceof"), V7_VEC("new"),      V7_VEC("null"),
    V7_VEC("return"),     V7_VEC("switch"),   V7_VEC("this"),
    V7_VEC("throw"),      V7_VEC("true"),     V7_VEC("try"),
    V7_VEC("typeof"),     V7_VEC("var"),      V7_VEC("void"),
    V7_VEC("while"),      V7_VEC("with")};

V7_PRIVATE int is_reserved_word_token(enum v7_tok tok) {
  return tok >= TOK_BREAK && tok <= TOK_WITH;
}

/*
 * Move ptr to the next token, skipping comments and whitespaces.
 * Return number of new line characters detected.
 */
V7_PRIVATE int skip_to_next_tok(const char **ptr) {
  const char *s = *ptr, *p = NULL;
  int num_lines = 0;

  while (s != p && *s != '\0' && (isspace((unsigned char) *s) || *s == '/')) {
    p = s;
    while (*s != '\0' && isspace((unsigned char) *s)) {
      if (*s == '\n') num_lines++;
      s++;
    }
    if (s[0] == '/' && s[1] == '/') {
      s += 2;
      while (s[0] != '\0' && s[0] != '\n') s++;
    }
    if (s[0] == '/' && s[1] == '*') {
      s += 2;
      while (s[0] != '\0' && !(s[-1] == '/' && s[-2] == '*')) {
        if (s[0] == '\n') num_lines++;
        s++;
      }
    }
  }
  *ptr = s;

  return num_lines;
}

/* Advance `s` pointer to the end of identifier  */
static void ident(const char **s) {
  const unsigned char *p = (unsigned char *) *s;
  int n;
  Rune r;

  while (p[0] != '\0') {
    if (p[0] == '$' || p[0] == '_' || isalnum(p[0])) {
      /* $, _, or any alphanumeric are valid identifier characters */
      p++;
    } else if (p[0] == '\\' && p[1] == 'u' && isxdigit(p[2]) &&
               isxdigit(p[3]) && isxdigit(p[4]) && isxdigit(p[5])) {
      /* Unicode escape, \uXXXX . Could be used like "var \u0078 = 1;" */
      p += 6;
    } else if ((n = chartorune(&r, (char *) p)) > 1 && isalpharune(r)) {
      /* Unicode alphanumeric character */
      p += n;
    } else {
      break;
    }
  }

  *s = (char *) p;
}

static enum v7_tok kw(const char *s, int len, int ntoks, enum v7_tok tok) {
  int i;

  for (i = 0; i < ntoks; i++) {
    if (s_keywords[(tok - TOK_BREAK) + i].len == len &&
        memcmp(s_keywords[(tok - TOK_BREAK) + i].p + 1, s + 1, len - 1) == 0)
      break;
  }

  return i == ntoks ? TOK_IDENTIFIER : (enum v7_tok)(tok + i);
}

static enum v7_tok punct1(const char **s, int ch1, enum v7_tok tok1,
                          enum v7_tok tok2) {
  (*s)++;
  if (s[0][0] == ch1) {
    (*s)++;
    return tok1;
  } else {
    return tok2;
  }
}

static enum v7_tok punct2(const char **s, int ch1, enum v7_tok tok1, int ch2,
                          enum v7_tok tok2, enum v7_tok tok3) {
  if (s[0][1] == ch1 && s[0][2] == ch2) {
    (*s) += 3;
    return tok2;
  }

  return punct1(s, ch1, tok1, tok3);
}

static enum v7_tok punct3(const char **s, int ch1, enum v7_tok tok1, int ch2,
                          enum v7_tok tok2, enum v7_tok tok3) {
  (*s)++;
  if (s[0][0] == ch1) {
    (*s)++;
    return tok1;
  } else if (s[0][0] == ch2) {
    (*s)++;
    return tok2;
  } else {
    return tok3;
  }
}

static void parse_number(const char *s, const char **end, double *num) {
  *num = strtod(s, (char **) end);
}

static enum v7_tok parse_str_literal(const char **p) {
  const char *s = *p;
  int quote = *s++;

  /* Scan string literal, handle escape sequences */
  for (; *s != quote && *s != '\0'; s++) {
    if (*s == '\\') {
      switch (s[1]) {
        case 'b':
        case 'f':
        case 'n':
        case 'r':
        case 't':
        case 'v':
        case '\\':
          s++;
          break;
        default:
          if (s[1] == quote) s++;
          break;
      }
    }
  }

  if (*s == quote) {
    s++;
    *p = s;
    return TOK_STRING_LITERAL;
  } else {
    return TOK_END_OF_INPUT;
  }
}

/*
 * This function is the heart of the tokenizer.
 * Organized as a giant switch statement.
 *
 * Switch statement is by the first character of the input stream. If first
 * character begins with a letter, it could be either keyword or identifier.
 * get_tok() calls ident() which shifts `s` pointer to the end of the word.
 * Now, tokenizer knows that the word begins at `p` and ends at `s`.
 * It calls function kw() to scan over the keywords that start with `p[0]`
 * letter. Therefore, keyword tokens and keyword strings must be in the
 * same order, to let kw() function work properly.
 * If kw() finds a keyword match, it returns keyword token.
 * Otherwise, it returns TOK_IDENTIFIER.
 * NOTE(lsm): `prev_tok` is a previously parsed token. It is needed for
 * correctly parsing regex literals.
 */
V7_PRIVATE enum v7_tok get_tok(const char **s, double *n,
                               enum v7_tok prev_tok) {
  const char *p = *s;

  switch (*p) {
    /* Letters */
    case 'a':
      ident(s);
      return TOK_IDENTIFIER;
    case 'b':
      ident(s);
      return kw(p, *s - p, 1, TOK_BREAK);
    case 'c':
      ident(s);
      return kw(p, *s - p, 3, TOK_CASE);
    case 'd':
      ident(s);
      return kw(p, *s - p, 4, TOK_DEBUGGER);
    case 'e':
      ident(s);
      return kw(p, *s - p, 1, TOK_ELSE);
    case 'f':
      ident(s);
      return kw(p, *s - p, 4, TOK_FALSE);
    case 'g':
    case 'h':
      ident(s);
      return TOK_IDENTIFIER;
    case 'i':
      ident(s);
      return kw(p, *s - p, 3, TOK_IF);
    case 'j':
    case 'k':
    case 'l':
    case 'm':
      ident(s);
      return TOK_IDENTIFIER;
    case 'n':
      ident(s);
      return kw(p, *s - p, 2, TOK_NEW);
    case 'o':
    case 'p':
    case 'q':
      ident(s);
      return TOK_IDENTIFIER;
    case 'r':
      ident(s);
      return kw(p, *s - p, 1, TOK_RETURN);
    case 's':
      ident(s);
      return kw(p, *s - p, 1, TOK_SWITCH);
    case 't':
      ident(s);
      return kw(p, *s - p, 5, TOK_THIS);
    case 'u':
      ident(s);
      return TOK_IDENTIFIER;
    case 'v':
      ident(s);
      return kw(p, *s - p, 2, TOK_VAR);
    case 'w':
      ident(s);
      return kw(p, *s - p, 2, TOK_WHILE);
    case 'x':
    case 'y':
    case 'z':
      ident(s);
      return TOK_IDENTIFIER;

    case '_':
    case '$':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case '\\': /* Identifier may start with unicode escape sequence */
      ident(s);
      return TOK_IDENTIFIER;

    /* Numbers */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      parse_number(p, s, n);
      return TOK_NUMBER;

    /* String literals */
    case '\'':
    case '"':
      return parse_str_literal(s);

    /* Punctuators */
    case '=':
      return punct2(s, '=', TOK_EQ, '=', TOK_EQ_EQ, TOK_ASSIGN);
    case '!':
      return punct2(s, '=', TOK_NE, '=', TOK_NE_NE, TOK_NOT);

    case '%':
      return punct1(s, '=', TOK_REM_ASSIGN, TOK_REM);
    case '*':
      return punct1(s, '=', TOK_MUL_ASSIGN, TOK_MUL);
    case '/':
      /*
       * TOK_DIV, TOK_DIV_ASSIGN, and TOK_REGEX_LITERAL start with `/` char.
       * Division can happen after an expression.
       * In expressions like this:
       *            a /= b; c /= d;
       * things between slashes is NOT a regex literal.
       * The switch below catches all cases where division happens.
       */
      switch (prev_tok) {
        case TOK_CLOSE_CURLY:
        case TOK_CLOSE_PAREN:
        case TOK_CLOSE_BRACKET:
        case TOK_IDENTIFIER:
        case TOK_NUMBER:
          return punct1(s, '=', TOK_DIV_ASSIGN, TOK_DIV);
          break;
        default:
          /* Not a division - this is a regex. Scan until closing slash */
          for (p++; *p != '\0' && *p != '\n'; p++) {
            if (*p == '\\') {
              /* Skip escape sequence */
              p++;
            } else if (*p == '/') {
              /* This is a closing slash */
              p++;
              /* Skip regex flags */
              while (*p == 'g' || *p == 'i' || *p == 'm') {
                p++;
              }
              *s = p;
              return TOK_REGEX_LITERAL;
            }
          }
          break;
      }
      return punct1(s, '=', TOK_DIV_ASSIGN, TOK_DIV);
    case '^':
      return punct1(s, '=', TOK_XOR_ASSIGN, TOK_XOR);

    case '+':
      return punct3(s, '+', TOK_PLUS_PLUS, '=', TOK_PLUS_ASSIGN, TOK_PLUS);
    case '-':
      return punct3(s, '-', TOK_MINUS_MINUS, '=', TOK_MINUS_ASSIGN, TOK_MINUS);
    case '&':
      return punct3(s, '&', TOK_LOGICAL_AND, '=', TOK_AND_ASSIGN, TOK_AND);
    case '|':
      return punct3(s, '|', TOK_LOGICAL_OR, '=', TOK_OR_ASSIGN, TOK_OR);

    case '<':
      if (s[0][1] == '=') {
        (*s) += 2;
        return TOK_LE;
      }
      return punct2(s, '<', TOK_LSHIFT, '=', TOK_LSHIFT_ASSIGN, TOK_LT);
    case '>':
      if (s[0][1] == '=') {
        (*s) += 2;
        return TOK_GE;
      }
      if (s[0][1] == '>' && s[0][2] == '>' && s[0][3] == '=') {
        (*s) += 4;
        return TOK_URSHIFT_ASSIGN;
      }
      if (s[0][1] == '>' && s[0][2] == '>') {
        (*s) += 3;
        return TOK_URSHIFT;
      }
      return punct2(s, '>', TOK_RSHIFT, '=', TOK_RSHIFT_ASSIGN, TOK_GT);

    case '{':
      (*s)++;
      return TOK_OPEN_CURLY;
    case '}':
      (*s)++;
      return TOK_CLOSE_CURLY;
    case '(':
      (*s)++;
      return TOK_OPEN_PAREN;
    case ')':
      (*s)++;
      return TOK_CLOSE_PAREN;
    case '[':
      (*s)++;
      return TOK_OPEN_BRACKET;
    case ']':
      (*s)++;
      return TOK_CLOSE_BRACKET;
    case '.':
      switch (*(*s + 1)) {
        /* Numbers */
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          parse_number(p, s, n);
          return TOK_NUMBER;
      }
      (*s)++;
      return TOK_DOT;
    case ';':
      (*s)++;
      return TOK_SEMICOLON;
    case ':':
      (*s)++;
      return TOK_COLON;
    case '?':
      (*s)++;
      return TOK_QUESTION;
    case '~':
      (*s)++;
      return TOK_TILDA;
    case ',':
      (*s)++;
      return TOK_COMMA;

    default: {
      /* Handle unicode variables */
      Rune r;
      int n;

      if ((n = chartorune(&r, *s)) > 1 && isalpharune(r)) {
        ident(s);
        return TOK_IDENTIFIER;
      }
      return TOK_END_OF_INPUT;
    }
  }
}

#ifdef TEST_RUN
int main(void) {
  const char *src =
      "for (var fo++ = -1; /= <= 1.17; x<<) { == <<=, 'x')} "
      "Infinity %=x<<=2";
  enum v7_tok tok;
  double num;
  const char *p = src;

  skip_to_next_tok(&src);
  while ((tok = get_tok(&src, &num)) != TOK_END_OF_INPUT) {
    printf("%d [%.*s]\n", tok, (int) (src - p), p);
    skip_to_next_tok(&src);
    p = src;
  }
  printf("%d [%.*s]\n", tok, (int) (src - p), p);

  return 0;
}
#endif
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


struct a_sort_data {
  struct v7 *v7;
  val_t sort_func;
};

static val_t Array_ctor(struct v7 *v7, val_t this_obj, val_t args) {
#if 0
  (void) v7;
  (void) this_obj;
  return args;
#else
  unsigned long i, len;
  val_t res = v7_create_array(v7);
  (void) v7;
  (void) this_obj;
  /*
   * The interpreter passes dense array to C functions.
   * However dense array implementation is not yet complete
   * so we don't want to propagate them at each call to Array()
   */
  len = v7_array_length(v7, args);
  for (i = 0; i < len; i++) {
    v7_array_set(v7, res, i, v7_array_get(v7, args, i));
  }
  return res;
#endif
}

static val_t Array_push(struct v7 *v7, val_t this_obj, val_t args) {
  val_t v = v7_create_undefined();
  int i, len = v7_array_length(v7, args);
  for (i = 0; i < len; i++) {
    v = v7_array_get(v7, args, i);
    v7_array_push(v7, this_obj, v);
  }
  return v;
}

static val_t Array_get_length(struct v7 *v7, val_t this_obj, val_t args) {
  long len = 0;
  (void) args;
  if (is_prototype_of(v7, this_obj, v7->array_prototype)) {
    len = v7_array_length(v7, this_obj);
  }
  return v7_create_number(len);
}

static val_t Array_set_length(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  long new_len = arg_long(v7, args, 0, -1);

  if (!v7_is_object(this_obj)) {
    throw_exception(v7, TYPE_ERROR, "Array expected");
  } else if (new_len < 0 ||
             (v7_is_double(arg0) &&
              (isnan(v7_to_double(arg0)) || isinf(v7_to_double(arg0))))) {
    throw_exception(v7, RANGE_ERROR, "Invalid array length");
  } else {
    struct v7_property **p, **next;
    long index, max_index = -1;

    /* Remove all items with an index higher then new_len */
    for (p = &v7_to_object(this_obj)->properties; *p != NULL; p = next) {
      size_t n;
      const char *s = v7_to_string(v7, &p[0]->name, &n);
      next = &p[0]->next;
      index = strtol(s, NULL, 10);
      if (index >= new_len) {
        v7_destroy_property(p);
        *p = *next;
        next = p;
      } else if (index > max_index) {
        max_index = index;
      }
    }

    /* If we have to expand, insert an item with appropriate index */
    if (new_len > 0 && max_index < new_len - 1) {
      char buf[40];
      snprintf(buf, sizeof(buf), "%ld", new_len - 1);
      v7_set_property(v7, this_obj, buf, strlen(buf), 0, V7_UNDEFINED);
    }
  }
  return v7_create_number(new_len);
}

static int a_cmp(void *user_data, const void *pa, const void *pb) {
  struct a_sort_data *sort_data = (struct a_sort_data *) user_data;
  struct v7 *v7 = sort_data->v7;
  val_t a = *(val_t *) pa, b = *(val_t *) pb, func = sort_data->sort_func;

  if (v7_is_function(func)) {
    val_t res, args = v7_create_dense_array(v7);
    v7_array_push(v7, args, a);
    v7_array_push(v7, args, b);
    res = v7_apply(v7, func, V7_UNDEFINED, args);
    return (int) -v7_to_double(res);
  } else {
    char sa[100], sb[100];
    to_str(v7, a, sa, sizeof(sa), 0);
    to_str(v7, b, sb, sizeof(sb), 0);
    sa[sizeof(sa) - 1] = sb[sizeof(sb) - 1] = '\0';
    return strcmp(sb, sa);
  }
}

static int a_partition(val_t *a, int l, int r, void *user_data) {
  val_t t, pivot = a[l];
  int i = l, j = r + 1;

  for (;;) {
    do
      ++i;
    while (i <= r && a_cmp(user_data, &a[i], &pivot) <= 0);
    do
      --j;
    while (a_cmp(user_data, &a[j], &pivot) > 0);
    if (i >= j) break;
    t = a[i];
    a[i] = a[j];
    a[j] = t;
  }
  t = a[l];
  a[l] = a[j];
  a[j] = t;
  return j;
}

static void a_qsort(val_t *a, int l, int r, void *user_data) {
  if (l < r) {
    int j = a_partition(a, l, r, user_data);
    a_qsort(a, l, j - 1, user_data);
    a_qsort(a, j + 1, r, user_data);
  }
}

static val_t a_sort(struct v7 *v7, val_t obj, val_t args,
                    int (*sorting_func)(void *, const void *, const void *)) {
  int i = 0, len = v7_array_length(v7, obj);
  val_t *arr = (val_t *) malloc(len * sizeof(arr[0]));
  val_t arg0 = v7_array_get(v7, args, 0);

  if (!v7_is_object(obj)) return obj;
  assert(obj != v7->global_object);

  for (i = 0; i < len; i++) {
    arr[i] = v7_array_get(v7, obj, i);
  }

  if (sorting_func != NULL) {
    struct a_sort_data sort_data;
    sort_data.v7 = v7;
    sort_data.sort_func = arg0;
    a_qsort(arr, 0, len - 1, &sort_data);
  }

  for (i = 0; i < len; i++) {
    v7_array_set(v7, obj, i, arr[len - (i + 1)]);
  }

  free(arr);

  return obj;
}

static val_t Array_sort(struct v7 *v7, val_t this_obj, val_t args) {
  return a_sort(v7, this_obj, args, a_cmp);
}

static val_t Array_reverse(struct v7 *v7, val_t this_obj, val_t args) {
  return a_sort(v7, this_obj, args, NULL);
}

static val_t Array_join(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t res = v7_create_undefined();
  size_t sep_size = 0;
  const char *sep = NULL;

  /* Get pointer to the separator string */
  if (!v7_is_string(arg0)) {
    /* If no separator is provided, use comma */
    arg0 = v7_create_string(v7, ",", 1, 1);
  }
  sep = v7_to_string(v7, &arg0, &sep_size);

  /* Do the actual join */
  if (is_prototype_of(v7, this_obj, v7->array_prototype)) {
    struct mbuf m;
    char buf[100], *p;
    long i, n, num_elems = v7_array_length(v7, this_obj);

    mbuf_init(&m, 0);

    for (i = 0; i < num_elems; i++) {
      /* Append separator */
      if (i > 0) {
        mbuf_append(&m, sep, sep_size);
      }

      /* Append next item from an array */
      p = buf;
      n = to_str(v7, v7_array_get(v7, this_obj, i), buf, sizeof(buf), 0);
      if (n > (long) sizeof(buf)) {
        p = (char *) malloc(n + 1);
        to_str(v7, v7_array_get(v7, this_obj, i), p, n, 0);
      }
      mbuf_append(&m, p, n);
      if (p != buf) {
        free(p);
      }
    }

    /* mbuf contains concatenated string now. Copy it to the result. */
    res = v7_create_string(v7, m.buf, m.len, 1);
    mbuf_free(&m);
  }

  return res;
}

static val_t Array_toString(struct v7 *v7, val_t this_obj, val_t args) {
  return Array_join(v7, this_obj, args);
}

static val_t a_splice(struct v7 *v7, val_t this_obj, val_t args, int mutate) {
  val_t res = v7_create_dense_array(v7);
  long i, len = v7_array_length(v7, this_obj);
  long num_args = v7_array_length(v7, args);
  long elems_to_insert = num_args > 2 ? num_args - 2 : 0;
  long arg0 = arg_long(v7, args, 0, 0);
  long arg1 = arg_long(v7, args, 1, len);

  /* Bounds check */
  if (len <= 0) return res;
  if (arg0 < 0) arg0 = len + arg0;
  if (arg0 < 0) arg0 = 0;
  if (arg0 > len) arg0 = len;
  if (mutate) {
    if (arg1 < 0) arg1 = 0;
    arg1 += arg0;
  } else if (arg1 < 0) {
    arg1 = len + arg1;
  }

  /* Create return value - slice */
  for (i = arg0; i < arg1 && i < len; i++) {
    v7_array_push(v7, res, v7_array_get(v7, this_obj, i));
  }

  if (mutate && v7_to_object(this_obj)->attributes & V7_OBJ_DENSE_ARRAY) {
    /*
     * dense arrays are spliced by memmoving leaving the trailing
     * space allocated for future appends.
     * TODO(mkm): figure out if trimming is better
     */
    struct v7_property *p =
        v7_get_own_property2(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN);
    struct mbuf *abuf;
    if (p == NULL) return res;
    abuf = (struct mbuf *) v7_to_foreign(p->value);
    if (abuf == NULL) return res;

    memmove(abuf->buf + arg0 * sizeof(val_t), abuf->buf + arg1 * sizeof(val_t),
            (len - arg1) * sizeof(val_t));
    abuf->len -= (arg1 - arg0) * sizeof(val_t);
  } else if (mutate) {
    /* If splicing, modify this_obj array: remove spliced sub-array */
    struct v7_property **p, **next;
    long i;

    for (p = &v7_to_object(this_obj)->properties; *p != NULL; p = next) {
      size_t n;
      const char *s = v7_to_string(v7, &p[0]->name, &n);
      next = &p[0]->next;
      i = strtol(s, NULL, 10);
      if (i >= arg0 && i < arg1) {
        /* Remove items from spliced sub-array */
        v7_destroy_property(p);
        *p = *next;
        next = p;
      } else if (i >= arg1) {
        /* Modify indices of the elements past sub-array */
        char key[20];
        size_t n = snprintf(key, sizeof(key), "%ld",
                            i - (arg1 - arg0) + elems_to_insert);
        p[0]->name = v7_create_string(v7, key, n, 1);
      }
    }

    /* Insert optional extra elements */
    for (i = 2; i < num_args; i++) {
      char key[20];
      size_t n = snprintf(key, sizeof(key), "%ld", arg0 + i - 2);
      v7_set(v7, this_obj, key, n, v7_array_get(v7, args, i));
    }
  }

  return res;
}

static val_t Array_slice(struct v7 *v7, val_t this_obj, val_t args) {
  return a_splice(v7, this_obj, args, 0);
}

static val_t Array_splice(struct v7 *v7, val_t this_obj, val_t args) {
  return a_splice(v7, this_obj, args, 1);
}

static void a_prep1(struct v7 *v7, val_t t, val_t args, val_t *a0, val_t *a1) {
  *a0 = v7_array_get(v7, args, 0);
  *a1 = v7_array_get(v7, args, 1);
  if (v7_is_undefined(*a1)) {
    *a1 = t;
  }
}

static val_t a_prep2(struct v7 *v7, val_t a, val_t v, val_t n, val_t t) {
  val_t params = v7_create_dense_array(v7);
  v7_array_push(v7, params, v);
  v7_array_push(v7, params, n);
  v7_array_push(v7, params, t);
  return v7_apply(v7, a, t, params);
}

static val_t Array_map(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0, arg1, el, v, res = v7_create_undefined();
  unsigned long len, i;
  int has;

  if (!v7_is_object(this_obj)) {
    throw_exception(v7, TYPE_ERROR, "Array expected");
  } else {
    a_prep1(v7, this_obj, args, &arg0, &arg1);
    res = v7_create_dense_array(v7);
    len = v7_array_length(v7, this_obj);
    for (i = 0; i < len; i++) {
      v = v7_array_get2(v7, this_obj, i, &has);
      if (!has) continue;
      el = a_prep2(v7, arg0, v, v7_create_number(i), arg1);
      v7_array_set(v7, res, i, el);
    }
  }

  return res;
}

static val_t Array_every(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0, arg1, el, v;
  unsigned long i, len;
  int has;

  if (!v7_is_object(this_obj)) {
    throw_exception(v7, TYPE_ERROR, "Array expected");
  } else {
    a_prep1(v7, this_obj, args, &arg0, &arg1);

    len = v7_array_length(v7, this_obj);
    for (i = 0; i < len; i++) {
      v = v7_array_get2(v7, this_obj, i, &has);
      if (!has) continue;
      el = a_prep2(v7, arg0, v, v7_create_number(i), arg1);
      if (!v7_is_true(v7, el)) {
        return v7_create_boolean(0);
      }
    }
  }
  return v7_create_boolean(1);
}

static val_t Array_some(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0, arg1, el, v;
  unsigned long i, len;
  int has;

  if (!v7_is_object(this_obj)) {
    throw_exception(v7, TYPE_ERROR, "Array expected");
  } else {
    a_prep1(v7, this_obj, args, &arg0, &arg1);

    len = v7_array_length(v7, this_obj);
    for (i = 0; i < len; i++) {
      v = v7_array_get2(v7, this_obj, i, &has);
      if (!has) continue;
      el = a_prep2(v7, arg0, v, v7_create_number(i), arg1);
      if (v7_is_true(v7, el)) {
        return v7_create_boolean(1);
      }
    }
  }
  return v7_create_boolean(0);
}

static val_t Array_filter(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0, arg1, el, v, res = v7_create_undefined();
  unsigned long len, i;
  int has;

  if (!v7_is_object(this_obj)) {
    throw_exception(v7, TYPE_ERROR, "Array expected");
  } else {
    a_prep1(v7, this_obj, args, &arg0, &arg1);
    res = v7_create_dense_array(v7);
    len = v7_array_length(v7, this_obj);
    for (i = 0; i < len; i++) {
      v = v7_array_get2(v7, this_obj, i, &has);
      if (!has) continue;
      el = a_prep2(v7, arg0, v, v7_create_number(i), arg1);
      if (v7_is_true(v7, el)) {
        v7_array_push(v7, res, v);
      }
    }
  }
  return res;
}

V7_PRIVATE void init_array(struct v7 *v7) {
  val_t ctor = v7_create_cfunction_object(v7, Array_ctor, 1);
  val_t length = v7_create_dense_array(v7);

  v7_set_property(v7, ctor, "prototype", 9, 0, v7->array_prototype);
  v7_set_property(v7, v7->global_object, "Array", 5, 0, ctor);

  set_cfunc_obj_prop(v7, v7->array_prototype, "push", Array_push, 1);
  set_cfunc_obj_prop(v7, v7->array_prototype, "sort", Array_sort, 1);
  set_cfunc_obj_prop(v7, v7->array_prototype, "reverse", Array_reverse, 0);
  set_cfunc_obj_prop(v7, v7->array_prototype, "join", Array_join, 1);
  set_cfunc_obj_prop(v7, v7->array_prototype, "toString", Array_toString, 0);
  set_cfunc_obj_prop(v7, v7->array_prototype, "slice", Array_slice, 2);
  set_cfunc_obj_prop(v7, v7->array_prototype, "splice", Array_splice, 2);
  set_cfunc_obj_prop(v7, v7->array_prototype, "map", Array_map, 1);
  set_cfunc_obj_prop(v7, v7->array_prototype, "every", Array_every, 1);
  set_cfunc_obj_prop(v7, v7->array_prototype, "some", Array_some, 1);
  set_cfunc_obj_prop(v7, v7->array_prototype, "filter", Array_filter, 1);

  v7_array_set(v7, length, 0, v7_create_cfunction(Array_get_length));
  v7_array_set(v7, length, 1, v7_create_cfunction(Array_set_length));
  v7_set_property(v7, v7->array_prototype, "length", 6,
                  V7_PROPERTY_GETTER | V7_PROPERTY_SETTER, length);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE val_t Boolean_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  val_t v = v7_create_boolean(0); /* false by default */

  if (v7_is_true(v7, v7_array_get(v7, args, 0))) {
    v = v7_create_boolean(1);
  }

  if (v7_is_object(this_obj) && this_obj != v7->global_object) {
    /* called as "new Boolean(...)" */
    v7_to_object(this_obj)->prototype = v7_to_object(v7->boolean_prototype);
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, v);
    v = this_obj;
  }

  return v;
}

static val_t Boolean_valueOf(struct v7 *v7, val_t this_obj, val_t args) {
  if (!v7_is_boolean(this_obj) &&
      (v7_is_object(this_obj) &&
       v7_object_to_value(v7_to_object(this_obj)->prototype) !=
           v7->boolean_prototype)) {
    throw_exception(v7, TYPE_ERROR,
                    "Boolean.valueOf called on non-boolean object");
  }
  return Obj_valueOf(v7, this_obj, args);
}

static val_t Boolean_toString(struct v7 *v7, val_t this_obj, val_t args) {
  char buf[512];
  (void) args;

  if (this_obj == v7->boolean_prototype) {
    return v7_create_string(v7, "false", 5, 1);
  }

  if (!v7_is_boolean(this_obj) &&
      !(v7_is_object(this_obj) &&
        is_prototype_of(v7, this_obj, v7->boolean_prototype))) {
    throw_exception(v7, TYPE_ERROR,
                    "Boolean.toString called on non-boolean object");
  }

  v7_stringify_value(v7, i_value_of(v7, this_obj), buf, sizeof(buf));
  return v7_create_string(v7, buf, strlen(buf), 1);
}

V7_PRIVATE void init_boolean(struct v7 *v7) {
  val_t ctor =
      v7_create_cfunction_ctor(v7, v7->boolean_prototype, Boolean_ctor, 1);
  v7_set_property(v7, v7->global_object, "Boolean", 7, 0, ctor);

  set_cfunc_prop(v7, v7->boolean_prototype, "valueOf", Boolean_valueOf);
  set_cfunc_prop(v7, v7->boolean_prototype, "toString", Boolean_toString);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


static val_t m_one_arg(struct v7 *v7, val_t args, double (*f)(double)) {
  val_t arg0 = v7_array_get(v7, args, 0);
  return v7_create_number(f(v7_to_double(arg0)));
}

static val_t m_two_arg(struct v7 *v7, val_t args, double (*f)(double, double)) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t arg1 = v7_array_get(v7, args, 1);
  return v7_create_number(f(v7_to_double(arg0), v7_to_double(arg1)));
}

#define DEFINE_WRAPPER(name, func)                                          \
  V7_PRIVATE val_t Math_##name(struct v7 *v7, val_t this_obj, val_t args) { \
    (void) this_obj;                                                        \
    return func(v7, args, name);                                            \
  }

#ifdef V7_WINDOWS
static double round(double n) {
  return n;
}
#endif

DEFINE_WRAPPER(fabs, m_one_arg)
DEFINE_WRAPPER(acos, m_one_arg)
DEFINE_WRAPPER(asin, m_one_arg)
DEFINE_WRAPPER(atan, m_one_arg)
DEFINE_WRAPPER(atan2, m_two_arg)
DEFINE_WRAPPER(ceil, m_one_arg)
DEFINE_WRAPPER(cos, m_one_arg)
DEFINE_WRAPPER(exp, m_one_arg)
DEFINE_WRAPPER(floor, m_one_arg)
DEFINE_WRAPPER(log, m_one_arg)
DEFINE_WRAPPER(pow, m_two_arg)
DEFINE_WRAPPER(round, m_one_arg)
DEFINE_WRAPPER(sin, m_one_arg)
DEFINE_WRAPPER(sqrt, m_one_arg)
DEFINE_WRAPPER(tan, m_one_arg)

V7_PRIVATE val_t Math_random(struct v7 *v7, val_t this_obj, val_t args) {
  static int srand_called = 0;

  if (!srand_called) {
    srand((unsigned) (unsigned long) v7);
    srand_called++;
  }

  (void) this_obj;
  (void) args;
  return v7_create_number((double) rand() / RAND_MAX);
}

static val_t min_max(struct v7 *v7, val_t args, int is_min) {
  double res = NAN;
  int i, len = v7_array_length(v7, args);

  for (i = 0; i < len; i++) {
    double v = v7_to_double(v7_array_get(v7, args, i));
    if (isnan(res) || (is_min && v < res) || (!is_min && v > res)) {
      res = v;
    }
  }

  return v7_create_number(res);
}

V7_PRIVATE val_t Math_min(struct v7 *v7, val_t this_obj, val_t args) {
  (void) this_obj;
  return min_max(v7, args, 1);
}

V7_PRIVATE val_t Math_max(struct v7 *v7, val_t this_obj, val_t args) {
  (void) this_obj;
  return min_max(v7, args, 0);
}

V7_PRIVATE void init_math(struct v7 *v7) {
  val_t math = v7_create_object(v7);

  set_cfunc_prop(v7, math, "abs", Math_fabs);
  set_cfunc_prop(v7, math, "acos", Math_acos);
  set_cfunc_prop(v7, math, "asin", Math_asin);
  set_cfunc_prop(v7, math, "atan", Math_atan);
  set_cfunc_prop(v7, math, "atan2", Math_atan2);
  set_cfunc_prop(v7, math, "ceil", Math_ceil);
  set_cfunc_prop(v7, math, "cos", Math_cos);
  set_cfunc_prop(v7, math, "exp", Math_exp);
  set_cfunc_prop(v7, math, "floor", Math_floor);
  set_cfunc_prop(v7, math, "log", Math_log);
  set_cfunc_prop(v7, math, "max", Math_max);
  set_cfunc_prop(v7, math, "min", Math_min);
  set_cfunc_prop(v7, math, "pow", Math_pow);
  set_cfunc_prop(v7, math, "random", Math_random);
  set_cfunc_prop(v7, math, "round", Math_round);
  set_cfunc_prop(v7, math, "sin", Math_sin);
  set_cfunc_prop(v7, math, "sqrt", Math_sqrt);
  set_cfunc_prop(v7, math, "tan", Math_tan);

  v7_set_property(v7, math, "E", 1, 0, v7_create_number(M_E));
  v7_set_property(v7, math, "PI", 2, 0, v7_create_number(M_PI));
  v7_set_property(v7, math, "LN2", 3, 0, v7_create_number(M_LN2));
  v7_set_property(v7, math, "LN10", 4, 0, v7_create_number(M_LN10));
  v7_set_property(v7, math, "LOG2E", 5, 0, v7_create_number(M_LOG2E));
  v7_set_property(v7, math, "LOG10E", 6, 0, v7_create_number(M_LOG10E));
  v7_set_property(v7, math, "SQRT1_2", 7, 0, v7_create_number(M_SQRT1_2));
  v7_set_property(v7, math, "SQRT2", 5, 0, v7_create_number(M_SQRT2));

  v7_set_property(v7, v7->global_object, "Math", 4, 0, math);
  v7_set_property(v7, v7->global_object, "NaN", 3, 0, V7_TAG_NAN);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE val_t to_string(struct v7 *, val_t);

static val_t String_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  val_t res, arg0 = v7_array_get(v7, args, 0);

  if (v7_is_string(arg0)) {
    res = arg0;
  } else if (v7_is_undefined(arg0)) {
    res = v7_create_string(v7, "", 0, 1);
  } else {
    res = to_string(v7, arg0);
  }

  if (v7_is_object(this_obj) && this_obj != v7->global_object) {
    v7_to_object(this_obj)->prototype = v7_to_object(v7->string_prototype);
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, res);
    return this_obj;
  }

  return res;
}

static val_t Str_fromCharCode(struct v7 *v7, val_t this_obj, val_t args) {
  int i, num_args = v7_array_length(v7, args);
  val_t res = v7_create_string(v7, "", 0, 1); /* Empty string */

  (void) this_obj;
  for (i = 0; i < num_args; i++) {
    char buf[10];
    val_t arg = v7_array_get(v7, args, i);
    double d = v7_to_double(arg);
    Rune r = (Rune)((int32_t)(isnan(d) || isinf(d) ? 0 : d) & 0xffff);
    int n = runetochar(buf, &r);
    val_t s = v7_create_string(v7, buf, n, 1);
    res = s_concat(v7, res, s);
  }

  return res;
}

static double s_charCodeAt(struct v7 *v7, val_t this_obj, val_t args) {
  size_t n;
  val_t s = to_string(v7, this_obj);
  const char *p = v7_to_string(v7, &s, &n);
  val_t arg = v7_array_get(v7, args, 0);
  double at = v7_to_double(arg);

  n = utfnlen((char *) p, n);
  if (v7_is_double(arg) && at >= 0 && at < n) {
    Rune r = 0;
    p = utfnshift((char *) p, at);
    chartorune(&r, (char *) p);
    return r;
  }
  return NAN;
}

static val_t Str_charCodeAt(struct v7 *v7, val_t this_obj, val_t args) {
  return v7_create_number(s_charCodeAt(v7, this_obj, args));
}

static val_t Str_charAt(struct v7 *v7, val_t this_obj, val_t args) {
  double code = s_charCodeAt(v7, this_obj, args);
  char buf[10] = {0};
  int len = 0;

  if (!isnan(code)) {
    Rune r = (Rune) code;
    len = runetochar(buf, &r);
  }
  return v7_create_string(v7, buf, len, 1);
}

static val_t Str_concat(struct v7 *v7, val_t this_obj, val_t args) {
  val_t res = to_string(v7, this_obj);
  int i, num_args = v7_array_length(v7, args);

  for (i = 0; i < num_args; i++) {
    val_t str = to_string(v7, v7_array_get(v7, args, i));
    res = s_concat(v7, res, str);
  }

  return res;
}

static val_t s_index_of(struct v7 *v7, val_t this_obj, val_t args, int last) {
  val_t arg0 = v7_array_get(v7, args, 0);
  size_t fromIndex = 0;
  double res = -1;

  if (!v7_is_undefined(arg0)) {
    const char *p1, *p2, *end;
    size_t i, n1, n2;
    val_t sub = to_string(v7, arg0);
    this_obj = to_string(v7, this_obj);
    p1 = v7_to_string(v7, &this_obj, &n1);
    p2 = v7_to_string(v7, &sub, &n2);

    if (n2 <= n1) {
      end = p1 + n1;
      n1 = utfnlen((char *) p1, n1);
      if (v7_array_length(v7, args) > 1) {
        double d = i_as_num(v7, v7_array_get(v7, args, 1));
        if (isnan(d) || d < 0) {
          d = 0.0;
        } else if (isinf(d)) {
          d = n1;
        }
        fromIndex = d;
      }
      if (fromIndex > 0) {
        if (fromIndex >= n1) return v7_create_number(-1);
        if (last)
          end = utfnshift((char *) p1, fromIndex + 1);
        else
          p1 = utfnshift((char *) p1, fromIndex);
      }
      if (!last || fromIndex != 0) {
        if (0 == n2 || end - p1 == 0)
          res = 0;
        else {
          for (i = 0; p1 <= (end - n2); i++, p1 = utfnshift((char *) p1, 1))
            if (memcmp(p1, p2, n2) == 0) {
              res = i;
              if (!last) break;
            }
        }
      }
    }
  }
  if (!last && res >= 0) res += fromIndex;
  return v7_create_number(res);
}

static val_t Str_valueOf(struct v7 *v7, val_t this_obj, val_t args) {
  if (!v7_is_string(this_obj) &&
      (v7_is_object(this_obj) &&
       v7_object_to_value(v7_to_object(this_obj)->prototype) !=
           v7->string_prototype)) {
    throw_exception(v7, TYPE_ERROR,
                    "String.valueOf called on non-string object");
  }
  return Obj_valueOf(v7, this_obj, args);
}

static val_t Str_indexOf(struct v7 *v7, val_t this_obj, val_t args) {
  return s_index_of(v7, this_obj, args, 0);
}

static val_t Str_lastIndexOf(struct v7 *v7, val_t this_obj, val_t args) {
  return s_index_of(v7, this_obj, args, 1);
}

static val_t Str_localeCompare(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = to_string(v7, v7_array_get(v7, args, 0));
  val_t s = to_string(v7, this_obj);
  return v7_create_number(s_cmp(v7, s, arg0));
}

static val_t Str_toString(struct v7 *v7, val_t this_obj, val_t args) {
  (void) args;

  if (this_obj == v7->string_prototype) {
    return v7_create_string(v7, "false", 5, 1);
  }

  if (!v7_is_string(this_obj) &&
      !(v7_is_object(this_obj) &&
        is_prototype_of(v7, this_obj, v7->string_prototype))) {
    throw_exception(v7, TYPE_ERROR,
                    "String.toString called on non-string object");
  }

  return to_string(v7, i_value_of(v7, this_obj));
}

static val_t Str_match(struct v7 *v7, val_t this_obj, val_t args) {
  val_t so, ro, arr = v7_create_null();
  long previousLastIndex = 0;
  int lastMatch = 1, n = 0, flag_g;
  struct v7_regexp *rxp;

  so = to_string(v7, this_obj);
  if (v7_array_length(v7, args) == 0) ro = v7_create_regexp(v7, "", 0, "", 0);
  else ro = i_value_of(v7, v7_array_get(v7, args, 0));
  if (!v7_is_regexp(ro)) {
    val_t arg = v7_create_dense_array(v7);
    v7_array_push(v7, arg, ro);
    ro = Regex_ctor(v7, v7_create_null(), arg);
  }

  rxp = v7_to_regexp(ro);
  flag_g = slre_get_flags(rxp->compiled_regexp) & SLRE_FLAG_G;
  if (!flag_g) return rx_exec(v7, ro, so, 0);
  
  rxp->lastIndex = 0;
  arr = v7_create_dense_array(v7);
  while (lastMatch) {
    val_t result = rx_exec(v7, ro, so, 1);
    if (v7_is_null(result)) lastMatch = 0;
    else {
      long thisIndex = rxp->lastIndex;
      if (thisIndex == previousLastIndex) {
        previousLastIndex = thisIndex + 1;
        rxp->lastIndex = previousLastIndex;
      } else previousLastIndex = thisIndex;
      v7_array_push(v7, arr, v7_array_get(v7, result, 0));
      n++;
    }
  }
  if (n == 0) return v7_create_null();
  return arr;
}

static val_t Str_replace(struct v7 *v7, val_t this_obj, val_t args) {
  const char *s;
  size_t s_len;
  val_t out_str_o;
  char *old_owned_mbuf_base = v7->owned_strings.buf;
  char *old_owned_mbuf_end = v7->owned_strings.buf + v7->owned_strings.len;
  this_obj = to_string(v7, this_obj);
  s = v7_to_string(v7, &this_obj, &s_len);

  if (s_len != 0 && v7_array_length(v7, args) > 1) {
    const char *const str_end = s + s_len;
    char *p = (char *) s;
    uint32_t out_sub_num = 0;
    val_t ro = i_value_of(v7, v7_array_get(v7, args, 0)),
          str_func = i_value_of(v7, v7_array_get(v7, args, 1));
    struct slre_prog *prog;
    struct slre_cap out_sub[V7_RE_MAX_REPL_SUB], *ptok = out_sub;
    struct slre_loot loot;
    int flag_g;

    if (!v7_is_regexp(ro)) {
      val_t arg = v7_create_dense_array(v7);
      v7_array_push(v7, arg, ro);
      ro = Regex_ctor(v7, v7_create_null(), arg);
    }
    prog = v7_to_regexp(ro)->compiled_regexp;
    flag_g = slre_get_flags(prog) & SLRE_FLAG_G;

    if (!v7_is_function(str_func)) str_func = to_string(v7, str_func);

    do {
      int i;
      if (slre_exec(prog, 0, p, str_end, &loot)) break;
      if (p != loot.caps->start) {
        ptok->start = p;
        ptok->end = loot.caps->start;
        ptok++;
        out_sub_num++;
      }

      if (v7_is_function(str_func)) { /* replace function */
        const char *rez_str;
        size_t rez_len;
        val_t arr = v7_create_dense_array(v7);

        for (i = 0; i < loot.num_captures; i++) {
          v7_array_push(v7, arr, v7_create_string(
                                     v7, loot.caps[i].start,
                                     loot.caps[i].end - loot.caps[i].start, 1));
        }
        v7_array_push(v7, arr, v7_create_number(utfnlen(
                                   (char *) s, loot.caps[0].start - s)));
        v7_array_push(v7, arr, this_obj);
        out_str_o = to_string(v7, v7_apply(v7, str_func, this_obj, arr));
        rez_str = v7_to_string(v7, &out_str_o, &rez_len);
        if (rez_len) {
          ptok->start = rez_str;
          ptok->end = rez_str + rez_len;
          ptok++;
          out_sub_num++;
        }
      } else { /* replace string */
        struct slre_loot newsub;
        size_t f_len;
        const char *f_str = v7_to_string(v7, &str_func, &f_len);
        slre_replace(&loot, s, s_len, f_str, f_len, &newsub);
        for (i = 0; i < newsub.num_captures; i++) {
          ptok->start = newsub.caps[i].start;
          ptok->end = newsub.caps[i].end;
          ptok++;
          out_sub_num++;
        }
      }
      p = (char *) loot.caps[0].end;
    } while (flag_g && p < str_end);
    if (p < str_end) {
      ptok->start = p;
      ptok->end = str_end;
      ptok++;
      out_sub_num++;
    }
    out_str_o = v7_create_string(v7, NULL, 0, 1);
    ptok = out_sub;
    do {
      size_t ln = ptok->end - ptok->start;
      const char *ps = ptok->start;
      if (ptok->start >= old_owned_mbuf_base &&
          ptok->start < old_owned_mbuf_end) {
        ps += v7->owned_strings.buf - old_owned_mbuf_base;
      }
      out_str_o = s_concat(v7, out_str_o, v7_create_string(v7, ps, ln, 1));
      p += ln;
      ptok++;
    } while (--out_sub_num);

    return out_str_o;
  }
  /* return v7_create_string(v7, s, s_len, 1); */
  return this_obj;
}

static val_t Str_search(struct v7 *v7, val_t this_obj, val_t args) {
  long utf_shift = -1;

  if (v7_array_length(v7, args) > 0) {
    size_t s_len;
    struct slre_loot sub;
    val_t so, ro = i_value_of(v7, v7_array_get(v7, args, 0));
    const char *s;
    if (!v7_is_regexp(ro)) {
    val_t arg = v7_create_dense_array(v7);
    v7_array_push(v7, arg, ro);
    ro = Regex_ctor(v7, v7_create_null(), arg);
    }

    so = to_string(v7, this_obj);
    s = v7_to_string(v7, &so, &s_len);

    if (!slre_exec(v7_to_regexp(ro)->compiled_regexp, 0, s, s + s_len, &sub))
      utf_shift =
          utfnlen((char *) s, sub.caps[0].start - s); /* calc shift for UTF-8 */
  } else
    utf_shift = 0;
  return v7_create_number(utf_shift);
}

static val_t Str_slice(struct v7 *v7, val_t this_obj, val_t args) {
  long from = 0, to = 0;
  size_t len;
  val_t so = to_string(v7, this_obj);
  const char *begin = v7_to_string(v7, &so, &len), *end;
  int num_args = v7_array_length(v7, args);

  to = len = utfnlen((char *) begin, len);
  if (num_args > 0) {
    from = arg_long(v7, args, 0, 0);
    if (from < 0) {
      from += len;
      if (from < 0) from = 0;
    } else if ((size_t) from > len)
      from = len;
    if (num_args > 1) {
      to = arg_long(v7, args, 1, 0);
      if (to < 0) {
        to += len;
        if (to < 0) to = 0;
      } else if ((size_t) to > len)
        to = len;
    }
  }
  if (from > to) to = from;
  end = utfnshift((char *) begin, to);
  begin = utfnshift((char *) begin, from);
  return v7_create_string(v7, begin, end - begin, 1);
}

static val_t s_transform(struct v7 *v7, val_t this_obj, val_t args,
                         Rune (*func)(Rune)) {
  val_t s = to_string(v7, this_obj);
  size_t i, n, len;
  const char *p = v7_to_string(v7, &s, &len);
  val_t res = v7_create_string(v7, p, len, 1);
  Rune r;

  (void) args;

  p = v7_to_string(v7, &res, &len);
  for (i = 0; i < len; i += n) {
    n = chartorune(&r, p + i);
    r = func(r);
    runetochar((char *) p + i, &r);
  }

  return res;
}

static val_t Str_toLowerCase(struct v7 *v7, val_t this_obj, val_t args) {
  return s_transform(v7, this_obj, args, tolowerrune);
}

static val_t Str_toUpperCase(struct v7 *v7, val_t this_obj, val_t args) {
  return s_transform(v7, this_obj, args, toupperrune);
}

static int s_isspace(Rune c) {
  return isspacerune(c) || isnewline(c);
}

static val_t Str_trim(struct v7 *v7, val_t this_obj, val_t args) {
  val_t s = to_string(v7, this_obj);
  size_t i, n, len, start = 0, end, state = 0;
  const char *p = v7_to_string(v7, &s, &len);
  Rune r;

  (void) args;
  end = len;
  for (i = 0; i < len; i += n) {
    n = chartorune(&r, p + i);
    if (!s_isspace(r)) {
      if (state++ == 0) start = i;
      end = i + n;
    }
  }

  return v7_create_string(v7, p + start, end - start, 1);
}

static val_t Str_length(struct v7 *v7, val_t this_obj, val_t args) {
  size_t len = 0;
  val_t s = i_value_of(v7, this_obj);

  (void) args;
  if (v7_is_string(s)) {
    const char *p = v7_to_string(v7, &s, &len);
    len = utfnlen((char *) p, len);
  }

  return v7_create_number(len);
}

V7_PRIVATE long arg_long(struct v7 *v7, val_t args, int n, long default_value) {
  char buf[40];
  size_t l;
  val_t arg_n = i_value_of(v7, v7_array_get(v7, args, n));
  double d;
  if (v7_is_double(arg_n)) {
    d = v7_to_double(arg_n);
    if (isnan(d) || (isinf(d) && d < 0)) {
      return 0;
    } else if (d > LONG_MAX) {
      return LONG_MAX;
    }
    return (long) d;
  }
  if (v7_is_null(arg_n)) return 0;
  l = to_str(v7, arg_n, buf, sizeof(buf), 0);
  if (l > 0 && isdigit(buf[0])) return strtol(buf, NULL, 10);
  return default_value;
}

static val_t s_substr(struct v7 *v7, val_t s, long start, long len) {
  size_t n;
  const char *p;
  s = to_string(v7, s);
  p = v7_to_string(v7, &s, &n);
  n = utfnlen((char *) p, n);

  if (start < (long) n && len > 0) {
    if (start < 0) start = (long) n + start;
    if (start < 0) start = 0;

    if (start > (long) n) start = n;
    if (len < 0) len = 0;
    if (len > (long) n - start) len = n - start;
    p = utfnshift((char *) p, start);
  } else
    len = 0;

  return v7_create_string(v7, p, len, 1);
}

static val_t Str_substr(struct v7 *v7, val_t this_obj, val_t args) {
  long start = arg_long(v7, args, 0, 0);
  long len = arg_long(v7, args, 1, LONG_MAX);
  return s_substr(v7, this_obj, start, len);
}

static val_t Str_substring(struct v7 *v7, val_t this_obj, val_t args) {
  long start = arg_long(v7, args, 0, 0);
  long end = arg_long(v7, args, 1, LONG_MAX);
  if (start < 0) start = 0;
  if (end < 0) end = 0;
  if (start > end) {
    long tmp = start;
    start = end;
    end = tmp;
  }
  return s_substr(v7, this_obj, start, end - start);
}

static val_t Str_split(struct v7 *v7, val_t this_obj, val_t args) {
  val_t res = v7_create_dense_array(v7);
  const char *s, *s_end;
  size_t s_len;
  long num_args = v7_array_length(v7, args);
  struct slre_prog *prog = NULL;
  this_obj = to_string(v7, this_obj);
  s = v7_to_string(v7, &this_obj, &s_len);
  s_end = s + s_len;

  if (num_args == 0 || s_len == 0) {
    v7_array_push(v7, res, this_obj);
  } else {
    val_t ro = i_value_of(v7, v7_array_get(v7, args, 0));
    long len, elem = 0, limit = arg_long(v7, args, 1, LONG_MAX);
    size_t shift = 0;
    struct slre_loot loot;
    if (!v7_is_regexp(ro)) {
      val_t arg = v7_create_dense_array(v7);
      v7_array_push(v7, arg, ro);
      ro = Regex_ctor(v7, v7_create_null(), arg);
    }
    prog = v7_to_regexp(ro)->compiled_regexp;

    for (; elem < limit && shift < s_len; elem++) {
      val_t tmp_s;
      int i;
      if (slre_exec(prog, 0, s + shift, s_end, &loot)) break;
      if (loot.caps[0].end - loot.caps[0].start == 0) {
        tmp_s = v7_create_string(v7, s + shift, 1, 1);
        shift++;
      } else {
        tmp_s =
            v7_create_string(v7, s + shift, loot.caps[0].start - s - shift, 1);
        shift = loot.caps[0].end - s;
      }
      v7_array_push(v7, res, tmp_s);

      for (i = 1; i < loot.num_captures; i++) {
        v7_array_push(
            v7, res,
            (loot.caps[i].start != NULL)
                ? v7_create_string(v7, loot.caps[i].start,
                                   loot.caps[i].end - loot.caps[i].start, 1)
                : v7_create_undefined());
      }
    }
    len = s_len - shift;
    if (len > 0 && elem < limit) {
      v7_array_push(v7, res, v7_create_string(v7, s + shift, len, 1));
    }
  }

  return res;
}

V7_PRIVATE void init_string(struct v7 *v7) {
  val_t str =
      v7_create_cfunction_ctor(v7, v7->string_prototype, String_ctor, 1);
  v7_set_property(v7, v7->global_object, "String", 6, V7_PROPERTY_DONT_ENUM,
                  str);

  set_cfunc_prop(v7, str, "fromCharCode", Str_fromCharCode);
  set_cfunc_prop(v7, v7->string_prototype, "charCodeAt", Str_charCodeAt);
  set_cfunc_prop(v7, v7->string_prototype, "charAt", Str_charAt);
  set_cfunc_prop(v7, v7->string_prototype, "concat", Str_concat);
  set_cfunc_prop(v7, v7->string_prototype, "indexOf", Str_indexOf);
  set_cfunc_prop(v7, v7->string_prototype, "substr", Str_substr);
  set_cfunc_prop(v7, v7->string_prototype, "substring", Str_substring);
  set_cfunc_prop(v7, v7->string_prototype, "valueOf", Str_valueOf);
  set_cfunc_prop(v7, v7->string_prototype, "lastIndexOf", Str_lastIndexOf);
  set_cfunc_prop(v7, v7->string_prototype, "localeCompare", Str_localeCompare);
  set_cfunc_prop(v7, v7->string_prototype, "match", Str_match);
  set_cfunc_prop(v7, v7->string_prototype, "replace", Str_replace);
  set_cfunc_prop(v7, v7->string_prototype, "search", Str_search);
  set_cfunc_prop(v7, v7->string_prototype, "slice", Str_slice);
  set_cfunc_prop(v7, v7->string_prototype, "trim", Str_trim);
  set_cfunc_prop(v7, v7->string_prototype, "toLowerCase", Str_toLowerCase);
  set_cfunc_prop(v7, v7->string_prototype, "toLocaleLowerCase",
                 Str_toLowerCase);
  set_cfunc_prop(v7, v7->string_prototype, "toUpperCase", Str_toUpperCase);
  set_cfunc_prop(v7, v7->string_prototype, "toLocaleUpperCase",
                 Str_toUpperCase);
  set_cfunc_prop(v7, v7->string_prototype, "split", Str_split);
  set_cfunc_prop(v7, v7->string_prototype, "toString", Str_toString);

  v7_set_property(v7, v7->string_prototype, "length", 6, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Str_length));
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


typedef unsigned short ast_skip_t;

/*
 * The structure of AST nodes cannot be described in portable ANSI C,
 * since they are variable length and packed (unaligned).
 *
 * Here each node's body is described with a pseudo-C structure notation.
 * The pseudo type `child` represents a variable length byte sequence
 * representing a fully serialized child node.
 *
 * `child body[]` represents a sequence of such subtrees.
 *
 * Pseudo-labels, such as `end:` represent the targets of skip fields
 * with the same name (e.g. `ast_skip_t end`).
 *
 * Skips allow skipping a subtree or sequence of subtrees.
 *
 * Sequences of subtrees (i.e. `child []`) have to be terminated by a skip:
 * they don't have a termination tag; all nodes whose position is before the
 *skip
 * are part of the sequence.
 *
 * Skips are encoded as network-byte-order 16-bit offsets counted from the
 * first byte of the node body (i.e. not counting the tag itself).
 * This currently limits the the maximum size of a function body to 64k.
 *
 * Notes:
 *
 * - Some nodes contain skips just for performance or because it simplifies
 * the implementation of the interpreter. For example, technically, the FOR
 * node doesn't need the `body` skip in order to be correctly traversed.
 * However, being able to quickly skip the `iter` expression is useful
 * also because it allows the interpreter to avoid traversing the expression
 * subtree without evaluating it, just in order to find the next subtree.
 *
 * - The name `skip` was chosen because `offset` was too overloaded in general
 * and label` is part of our domain model (i.e. JS has a label AST node type).
 *
 */
const struct ast_node_def ast_node_defs[] = {
    {"NOP", 0, 0, 0, 0}, /* struct {} */
                         /*
                          * struct {
                          *   ast_skip_t end;
                          *   ast_skip_t first_var;
                          *   child body[];
                          * end:
                          * }
                          */
    {"SCRIPT", 0, 0, 2, 0},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t next;
     *   child decls[];
     * end:
     * }
     */
    {"VAR", 0, 0, 2, 0},
    /*
     * struct {
     *   varint len;
     *   char name[len];
     *   child expr;
     * }
     */
    {"VAR_DECL", 1, 1, 0, 1},
    /*
     * struct {
     *   varint len;
     *   char name[len];
     *   child expr;
     * }
     */
    {"FUNC_DECL", 1, 1, 0, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t end_true;
     *   child cond;
     *   child iftrue[];
     * end_true:
     *   child iffalse[];
     * end:
     * }
     */
    {"IF", 0, 0, 2, 1},
    /*
     * TODO(mkm) distinguish function expressions
     * from function statements.
     * Function statements behave like vars and need a
     * next field for hoisting.
     * We can also ignore the name for function expressions
     * if it's only needed for debugging.
     *
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t first_var;
     *   ast_skip_t body;
     *   child name;
     *   child params[];
     * body:
     *   child body[];
     * end:
     * }
     */
    {"FUNC", 0, 0, 3, 1},
    {"ASSIGN", 0, 0, 0, 2},         /* struct { child left, right; } */
    {"REM_ASSIGN", 0, 0, 0, 2},     /* struct { child left, right; } */
    {"MUL_ASSIGN", 0, 0, 0, 2},     /* struct { child left, right; } */
    {"DIV_ASSIGN", 0, 0, 0, 2},     /* struct { child left, right; } */
    {"XOR_ASSIGN", 0, 0, 0, 2},     /* struct { child left, right; } */
    {"PLUS_ASSIGN", 0, 0, 0, 2},    /* struct { child left, right; } */
    {"MINUS_ASSIGN", 0, 0, 0, 2},   /* struct { child left, right; } */
    {"OR_ASSIGN", 0, 0, 0, 2},      /* struct { child left, right; } */
    {"AND_ASSIGN", 0, 0, 0, 2},     /* struct { child left, right; } */
    {"LSHIFT_ASSIGN", 0, 0, 0, 2},  /* struct { child left, right; } */
    {"RSHIFT_ASSIGN", 0, 0, 0, 2},  /* struct { child left, right; } */
    {"URSHIFT_ASSIGN", 0, 0, 0, 2}, /* struct { child left, right; } */
    {"NUM", 1, 1, 0, 0},            /* struct { varint len, char s[len]; } */
    {"IDENT", 1, 1, 0, 0},          /* struct { varint len, char s[len]; } */
    {"STRING", 1, 1, 0, 0},         /* struct { varint len, char s[len]; } */
    {"REGEX", 1, 1, 0, 0},          /* struct { varint len, char s[len]; } */
    {"LABEL", 1, 1, 0, 0},          /* struct { varint len, char s[len]; } */
                                    /*
                                     * struct {
                                     *   ast_skip_t end;
                                     *   child body[];
                                     * end:
                                     * }
                                     */
    {"SEQ", 0, 0, 1, 0},
    /*
     * struct {
     *   ast_skip_t end;
     *   child cond;
     *   child body[];
     * end:
     * }
     */
    {"WHILE", 0, 0, 1, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t cond;
     *   child body[];
     * cond:
     *   child cond;
     * end:
     * }
     */
    {"DOWHILE", 0, 0, 2, 0},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t body;
     *   child init;
     *   child cond;
     *   child iter;
     * body:
     *   child body[];
     * end:
     * }
     */
    {"FOR", 0, 0, 2, 3},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t dummy; // allows to quickly promote a for to a for in
     *   child var;
     *   child expr;
     *   child dummy;
     *   child body[];
     * end:
     * }
     */
    {"FOR_IN", 0, 0, 2, 3},
    {"COND", 0, 0, 0, 3},     /* struct { child cond, iftrue, iffalse; } */
    {"DEBUGGER", 0, 0, 0, 0}, /* struct {} */
    {"BREAK", 0, 0, 0, 0},    /* struct {} */
                              /*
                               * struct {
                               *   child label; // TODO(mkm): inline
                               * }
                               */
    {"LAB_BREAK", 0, 0, 0, 1},
    {"CONTINUE", 0, 0, 0, 0}, /* struct {} */
                              /*
                               * struct {
                               *   child label; // TODO(mkm): inline
                               * }
                               */
    {"LAB_CONTINUE", 0, 0, 0, 1},
    {"RETURN", 0, 0, 0, 0},     /* struct {} */
    {"VAL_RETURN", 0, 0, 0, 1}, /* struct { child expr; } */
    {"THROW", 0, 0, 0, 1},      /* struct { child expr; } */
                                /*
                                 * struct {
                                 *   ast_skip_t end;
                                 *   ast_skip_t catch;
                                 *   ast_skip_t finally;
                                 *   child try[];
                                 * catch:
                                 *   child var; // TODO(mkm): inline
                                 *   child catch[];
                                 * finally:
                                 *   child finally[];
                                 * end:
                                 * }
                                 */
    {"TRY", 0, 0, 3, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   ast_skip_t def;
     *   child expr;
     *   child cases[];
     * def:
     *   child default?; // optional
     * end:
     * }
     */
    {"SWITCH", 0, 0, 2, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   child val;
     *   child stmts[];
     * end:
     * }
     */
    {"CASE", 0, 0, 1, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   child stmts[];
     * end:
     * }
     */
    {"DEFAULT", 0, 0, 1, 0},
    /*
     * struct {
     *   ast_skip_t end;
     *   child expr;
     *   child body[];
     * end:
     * }
     */
    {"WITH", 0, 0, 1, 1},
    {"LOG_OR", 0, 0, 0, 2},      /* struct { child left, right; } */
    {"LOG_AND", 0, 0, 0, 2},     /* struct { child left, right; } */
    {"OR", 0, 0, 0, 2},          /* struct { child left, right; } */
    {"XOR", 0, 0, 0, 2},         /* struct { child left, right; } */
    {"AND", 0, 0, 0, 2},         /* struct { child left, right; } */
    {"EQ", 0, 0, 0, 2},          /* struct { child left, right; } */
    {"EQ_EQ", 0, 0, 0, 2},       /* struct { child left, right; } */
    {"NE", 0, 0, 0, 2},          /* struct { child left, right; } */
    {"NE_NE", 0, 0, 0, 2},       /* struct { child left, right; } */
    {"LE", 0, 0, 0, 2},          /* struct { child left, right; } */
    {"LT", 0, 0, 0, 2},          /* struct { child left, right; } */
    {"GE", 0, 0, 0, 2},          /* struct { child left, right; } */
    {"GT", 0, 0, 0, 2},          /* struct { child left, right; } */
    {"IN", 0, 0, 0, 2},          /* struct { child left, right; } */
    {"INSTANCEOF", 0, 0, 0, 2},  /* struct { child left, right; } */
    {"LSHIFT", 0, 0, 0, 2},      /* struct { child left, right; } */
    {"RSHIFT", 0, 0, 0, 2},      /* struct { child left, right; } */
    {"URSHIFT", 0, 0, 0, 2},     /* struct { child left, right; } */
    {"ADD", 0, 0, 0, 2},         /* struct { child left, right; } */
    {"SUB", 0, 0, 0, 2},         /* struct { child left, right; } */
    {"REM", 0, 0, 0, 2},         /* struct { child left, right; } */
    {"MUL", 0, 0, 0, 2},         /* struct { child left, right; } */
    {"DIV", 0, 0, 0, 2},         /* struct { child left, right; } */
    {"POS", 0, 0, 0, 1},         /* struct { child expr; } */
    {"NEG", 0, 0, 0, 1},         /* struct { child expr; } */
    {"NOT", 0, 0, 0, 1},         /* struct { child expr; } */
    {"LOGICAL_NOT", 0, 0, 0, 1}, /* struct { child expr; } */
    {"VOID", 0, 0, 0, 1},        /* struct { child expr; } */
    {"DELETE", 0, 0, 0, 1},      /* struct { child expr; } */
    {"TYPEOF", 0, 0, 0, 1},      /* struct { child expr; } */
    {"PREINC", 0, 0, 0, 1},      /* struct { child expr; } */
    {"PREDEC", 0, 0, 0, 1},      /* struct { child expr; } */
    {"POSTINC", 0, 0, 0, 1},     /* struct { child expr; } */
    {"POSTDEC", 0, 0, 0, 1},     /* struct { child expr; } */
                                 /*
                                  * struct {
                                  *   varint len;
                                  *   char ident[len];
                                  *   child expr;
                                  * }
                                  */
    {"MEMBER", 1, 1, 0, 1},
    /*
     * struct {
     *   child expr;
     *   child index;
     * }
     */
    {"INDEX", 0, 0, 0, 2},
    /*
     * struct {
     *   ast_skip_t end;
     *   child expr;
     *   child args[];
     * end:
     * }
     */
    {"CALL", 0, 0, 1, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   child expr;
     *   child args[];
     * end:
     * }
     */
    {"NEW", 0, 0, 1, 1},
    /*
     * struct {
     *   ast_skip_t end;
     *   child elements[];
     * end:
     * }
     */
    {"ARRAY", 0, 0, 1, 0},
    /*
     * struct {
     *   ast_skip_t end;
     *   child props[];
     * end:
     * }
     */
    {"OBJECT", 0, 0, 1, 0},
    /*
     * struct {
     *   varint len;
     *   char name[len];
     *   child expr;
     * }
     */
    {"PROP", 1, 1, 0, 1},
    /*
     * struct {
     *   child func;
     * }
     */
    {"GETTER", 0, 0, 0, 1},
    /*
     * struct {
     *   child func;
     * end:
     * }
     */
    {"SETTER", 0, 0, 0, 1},
    {"THIS", 0, 0, 0, 0},       /* struct {} */
    {"TRUE", 0, 0, 0, 0},       /* struct {} */
    {"FALSE", 0, 0, 0, 0},      /* struct {} */
    {"NULL", 0, 0, 0, 0},       /* struct {} */
    {"UNDEF", 0, 0, 0, 0},      /* struct {} */
    {"USE_STRICT", 0, 0, 0, 0}, /* struct {} */
};

V7_STATIC_ASSERT(AST_MAX_TAG < 256, ast_tag_should_fit_in_char);
V7_STATIC_ASSERT(AST_MAX_TAG == ARRAY_SIZE(ast_node_defs), bad_node_defs);

/*
 * Begins an AST node by appending a tag to the AST.
 *
 * It also allocates space for the fixed_size payload and the space for
 * the skips.
 *
 * The caller is responsible for appending children.
 *
 * Returns the offset of the node payload (one byte after the tag).
 * This offset can be passed to `ast_set_skip`.
 */
V7_PRIVATE ast_off_t ast_add_node(struct ast *a, enum ast_tag tag) {
  ast_off_t start = a->mbuf.len;
  uint8_t t = (uint8_t) tag;
  const struct ast_node_def *d = &ast_node_defs[tag];

  assert(tag < AST_MAX_TAG);

  mbuf_append(&a->mbuf, (char *) &t, sizeof(t));
  mbuf_append(&a->mbuf, NULL, sizeof(ast_skip_t) * d->num_skips);
  return start + 1;
}

V7_PRIVATE ast_off_t
ast_insert_node(struct ast *a, ast_off_t start, enum ast_tag tag) {
  uint8_t t = (uint8_t) tag;
  const struct ast_node_def *d = &ast_node_defs[tag];

  assert(tag < AST_MAX_TAG);

  mbuf_insert(&a->mbuf, start, NULL, sizeof(ast_skip_t) * d->num_skips);
  mbuf_insert(&a->mbuf, start, (char *) &t, sizeof(t));

  if (d->num_skips) {
    ast_set_skip(a, start + 1, AST_END_SKIP);
  }

  return start + 1;
}

V7_STATIC_ASSERT(sizeof(ast_skip_t) == 2, ast_skip_t_len_should_be_2);

/*
 * Patches a given skip slot for an already emitted node with the
 * current write cursor position (e.g. AST length).
 *
 * This is intended to be invoked when a node with a variable number
 * of child subtrees is closed, or when the consumers need a shortcut
 * to the next sibling.
 *
 * Each node type has a different number and semantic for skips,
 * all of them defined in the `ast_which_skip` enum.
 * All nodes having a variable number of child subtrees must define
 * at least the `AST_END_SKIP` skip, which effectively skips a node
 * boundary.
 *
 * Every tree reader can assume this and safely skip unknown nodes.
 */
V7_PRIVATE ast_off_t
ast_set_skip(struct ast *a, ast_off_t start, enum ast_which_skip skip) {
  return ast_modify_skip(a, start, a->mbuf.len, skip);
}

/*
 * Patches a given skip slot for an already emitted node with the value
 * (stored as delta relative to the `start` node) of the `where` argument.
 */
V7_PRIVATE ast_off_t ast_modify_skip(struct ast *a, ast_off_t start,
                                     ast_off_t where,
                                     enum ast_which_skip skip) {
  uint8_t *p = (uint8_t *) a->mbuf.buf + start + skip * sizeof(ast_skip_t);
  uint16_t delta = where - start;
  enum ast_tag tag = (enum ast_tag)(uint8_t) * (a->mbuf.buf + start - 1);
  const struct ast_node_def *def = &ast_node_defs[tag];

  /* assertion, to be optimizable out */
  assert((int) skip < def->num_skips);

  p[0] = delta >> 8;
  p[1] = delta & 0xff;
  return where;
}

V7_PRIVATE ast_off_t
ast_get_skip(struct ast *a, ast_off_t pos, enum ast_which_skip skip) {
  uint8_t *p;
  assert(pos + skip * sizeof(ast_skip_t) < a->mbuf.len);
  p = (uint8_t *) a->mbuf.buf + pos + skip * sizeof(ast_skip_t);
  return pos + (p[1] | p[0] << 8);
}

V7_PRIVATE enum ast_tag ast_fetch_tag(struct ast *a, ast_off_t *pos) {
  assert(*pos < a->mbuf.len);
  return (enum ast_tag)(uint8_t) * (a->mbuf.buf + (*pos)++);
}

/*
 * Assumes a cursor positioned right after a tag.
 *
 * TODO(mkm): add doc, find better name.
 */
V7_PRIVATE void ast_move_to_children(struct ast *a, ast_off_t *pos) {
  enum ast_tag tag = (enum ast_tag)(uint8_t) * (a->mbuf.buf + *pos - 1);
  const struct ast_node_def *def = &ast_node_defs[tag];
  assert(*pos - 1 < a->mbuf.len);
  if (def->has_varint) {
    int llen;
    size_t slen = decode_varint((unsigned char *) a->mbuf.buf + *pos, &llen);
    *pos += llen;
    if (def->has_inlined) {
      *pos += slen;
    }
  }

  *pos += def->num_skips * sizeof(ast_skip_t);
}

/* Helper to add a node with inlined data. */
V7_PRIVATE void ast_add_inlined_node(struct ast *a, enum ast_tag tag,
                                     const char *name, size_t len) {
  assert(ast_node_defs[tag].has_inlined);
  embed_string(&a->mbuf, ast_add_node(a, tag), name, len, 0, 1);
}

/* Helper to add a node with inlined data. */
V7_PRIVATE void ast_insert_inlined_node(struct ast *a, ast_off_t start,
                                        enum ast_tag tag, const char *name,
                                        size_t len) {
  assert(ast_node_defs[tag].has_inlined);
  embed_string(&a->mbuf, ast_insert_node(a, start, tag), name, len, 0, 1);
}

V7_PRIVATE char *ast_get_inlined_data(struct ast *a, ast_off_t pos, size_t *n) {
  int llen;
  assert(pos < a->mbuf.len);
  *n = decode_varint((unsigned char *) a->mbuf.buf + pos, &llen);
  return a->mbuf.buf + pos + llen;
}

V7_PRIVATE void ast_get_num(struct ast *a, ast_off_t pos, double *val) {
  char buf[512];
  char *str;
  size_t str_len;
  str = ast_get_inlined_data(a, pos, &str_len);
  if (str_len >= sizeof(buf)) {
    str_len = sizeof(buf) - 1;
  }
  memcpy(buf, str, str_len);
  buf[str_len] = '\0';
  *val = strtod(buf, NULL);
}

static void comment_at_depth(FILE *fp, const char *fmt, int depth, ...) {
  int i;
  char buf[265];
  va_list ap;
  va_start(ap, depth);

  vsnprintf(buf, sizeof(buf), fmt, ap);

  for (i = 0; i < depth; i++) {
    fprintf(fp, "  ");
  }
  fprintf(fp, "/* [%s] */\n", buf);
}

V7_PRIVATE void ast_skip_tree(struct ast *a, ast_off_t *pos) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  const struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t skips = *pos;
  int i;
  ast_move_to_children(a, pos);

  for (i = 0; i < def->num_subtrees; i++) {
    ast_skip_tree(a, pos);
  }

  if (ast_node_defs[tag].num_skips) {
    ast_off_t end = ast_get_skip(a, skips, AST_END_SKIP);

    while (*pos < end) {
      ast_skip_tree(a, pos);
    }
  }
}

static void ast_dump_tree(FILE *fp, struct ast *a, ast_off_t *pos, int depth) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  const struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t skips = *pos;
  size_t slen;
  int i, llen;

  for (i = 0; i < depth; i++) {
    fprintf(fp, "  ");
  }

  fprintf(fp, "%s", def->name);

  if (def->has_inlined) {
    slen = decode_varint((unsigned char *) a->mbuf.buf + *pos, &llen);
    fprintf(fp, " %.*s\n", (int) slen, a->mbuf.buf + *pos + llen);
  } else {
    fprintf(fp, "\n");
  }

  ast_move_to_children(a, pos);

  for (i = 0; i < def->num_subtrees; i++) {
    ast_dump_tree(fp, a, pos, depth + 1);
  }

  if (ast_node_defs[tag].num_skips) {
    /*
     * first skip always encodes end of the last children sequence.
     * so unless we care how the subtree sequences are grouped together
     * (and we currently don't) we can just read until the end of that skip.
     */
    ast_off_t end = ast_get_skip(a, skips, AST_END_SKIP);

    comment_at_depth(fp, "...", depth + 1);
    while (*pos < end) {
      int s;
      for (s = ast_node_defs[tag].num_skips - 1; s > 0; s--) {
        if (*pos == ast_get_skip(a, skips, (enum ast_which_skip) s)) {
          comment_at_depth(fp, "%d ->", depth + 1, s);
          break;
        }
      }
      ast_dump_tree(fp, a, pos, depth + 1);
    }
  }
}

V7_PRIVATE void ast_init(struct ast *ast, size_t len) {
  mbuf_init(&ast->mbuf, len);
}

V7_PRIVATE void ast_optimize(struct ast *ast) {
  mbuf_trim(&ast->mbuf);
}

V7_PRIVATE void ast_free(struct ast *ast) {
  mbuf_free(&ast->mbuf);
}

/* Dumps an AST to stdout. */
V7_PRIVATE void ast_dump(FILE *fp, struct ast *a, ast_off_t pos) {
  ast_dump_tree(fp, a, &pos, 0);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


enum v7_type val_type(struct v7 *v7, val_t v) {
  if (v7_is_double(v)) {
    return V7_TYPE_NUMBER;
  }
  switch (v & V7_TAG_MASK) {
    case V7_TAG_FOREIGN:
      if (v7_is_null(v)) {
        return V7_TYPE_NULL;
      }
      return V7_TYPE_FOREIGN;
    case V7_TAG_UNDEFINED:
      return V7_TYPE_UNDEFINED;
    case V7_TAG_OBJECT:
      if (v7_to_object(v)->prototype == v7_to_object(v7->array_prototype)) {
        return V7_TYPE_ARRAY_OBJECT;
      } else if (v7_to_object(v)->prototype ==
                 v7_to_object(v7->boolean_prototype)) {
        return V7_TYPE_BOOLEAN_OBJECT;
      } else if (v7_to_object(v)->prototype ==
                 v7_to_object(v7->string_prototype)) {
        return V7_TYPE_STRING_OBJECT;
      } else if (v7_to_object(v)->prototype ==
                 v7_to_object(v7->number_prototype)) {
        return V7_TYPE_NUMBER_OBJECT;
      } else if (v7_to_object(v)->prototype ==
                 v7_to_object(v7->function_prototype)) {
        return V7_TYPE_CFUNCTION_OBJECT;
      } else if (v7_to_object(v)->prototype ==
                 v7_to_object(v7->date_prototype)) {
        return V7_TYPE_DATE_OBJECT;
      } else {
        return V7_TYPE_GENERIC_OBJECT;
      }
    case V7_TAG_STRING_I:
    case V7_TAG_STRING_O:
    case V7_TAG_STRING_F:
    case V7_TAG_STRING_5:
      return V7_TYPE_STRING;
    case V7_TAG_BOOLEAN:
      return V7_TYPE_BOOLEAN;
    case V7_TAG_FUNCTION:
      return V7_TYPE_FUNCTION_OBJECT;
    case V7_TAG_CFUNCTION:
      return V7_TYPE_CFUNCTION;
    case V7_TAG_REGEXP:
      return V7_TYPE_REGEXP_OBJECT;
    default:
      /* TODO(mkm): or should we crash? */
      return V7_TYPE_UNDEFINED;
  }
}

int v7_is_double(val_t v) {
  return v == V7_TAG_NAN || !isnan(v7_to_double(v));
}

int v7_is_object(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_OBJECT ||
         (v & V7_TAG_MASK) == V7_TAG_FUNCTION;
}

int v7_is_function(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_FUNCTION;
}

int v7_is_string(val_t v) {
  uint64_t t = v & V7_TAG_MASK;
  return t == V7_TAG_STRING_I || t == V7_TAG_STRING_F || t == V7_TAG_STRING_O ||
         t == V7_TAG_STRING_5;
}

int v7_is_boolean(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_BOOLEAN;
}

int v7_is_regexp(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_REGEXP;
}

int v7_is_foreign(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_FOREIGN;
}

V7_PRIVATE struct v7_regexp *v7_to_regexp(val_t v) {
  return (struct v7_regexp *) v7_to_pointer(v);
}

int v7_is_null(val_t v) {
  return v == V7_NULL;
}

int v7_is_undefined(val_t v) {
  return v == V7_UNDEFINED;
}

int v7_is_cfunction(val_t v) {
  return (v & V7_TAG_MASK) == V7_TAG_CFUNCTION;
}

/* A convenience function to check exec result */
int v7_is_error(struct v7 *v7, val_t v) {
  return is_prototype_of(v7, v, v7->error_prototype);
}

V7_PRIVATE val_t v7_pointer_to_value(void *p) {
  return ((uint64_t)(uintptr_t) p) & ~V7_TAG_MASK;
}

V7_PRIVATE void *v7_to_pointer(val_t v) {
  struct {
    uint64_t s : 48;
  } h;
  return (void *) (uintptr_t)(h.s = v);
}

val_t v7_object_to_value(struct v7_object *o) {
  if (o == NULL) {
    return V7_NULL;
  }
  return v7_pointer_to_value(o) | V7_TAG_OBJECT;
}

struct v7_object *v7_to_object(val_t v) {
  return (struct v7_object *) v7_to_pointer(v);
}

val_t v7_function_to_value(struct v7_function *o) {
  return v7_pointer_to_value(o) | V7_TAG_FUNCTION;
}

struct v7_function *v7_to_function(val_t v) {
  return (struct v7_function *) v7_to_pointer(v);
}

v7_cfunction_t v7_to_cfunction(val_t v) {
  return (v7_cfunction_t) v7_to_pointer(v);
}

v7_val_t v7_create_cfunction(v7_cfunction_t f) {
  union {
    void *p;
    v7_cfunction_t f;
  } u;
  u.f = f;
  return v7_pointer_to_value(u.p) | V7_TAG_CFUNCTION;
}

void *v7_to_foreign(val_t v) {
  return v7_to_pointer(v);
}

v7_val_t v7_create_boolean(int v) {
  return (!!v) | V7_TAG_BOOLEAN;
}

int v7_to_boolean(val_t v) {
  return v & 1;
}

v7_val_t v7_create_number(double v) {
  val_t res;
  /* not every NaN is a JS NaN */
  if (isnan(v)) {
    res = V7_TAG_NAN;
  } else {
    *(double *) &res = v;
  }
  return res;
}

double v7_to_double(val_t v) {
  return *(double *) &v;
}

V7_PRIVATE val_t v_get_prototype(struct v7 *v7, val_t obj) {
  if (v7_is_function(obj)) {
    return v7->function_prototype;
  }
  return v7_object_to_value(v7_to_object(obj)->prototype);
}

V7_PRIVATE val_t create_object(struct v7 *v7, val_t prototype) {
  struct v7_object *o = new_object(v7);
  if (o == NULL) {
    return V7_NULL;
  }
  (void) v7;
  o->properties = NULL;
  o->prototype = v7_to_object(prototype);
  return v7_object_to_value(o);
}

v7_val_t v7_create_object(struct v7 *v7) {
  return create_object(v7, v7->object_prototype);
}

v7_val_t v7_create_null(void) {
  return V7_NULL;
}

v7_val_t v7_create_undefined(void) {
  return V7_UNDEFINED;
}

v7_val_t v7_create_array(struct v7 *v7) {
  val_t a = create_object(v7, v7->array_prototype);
#if 0
  v7_set_property(v7, a, "", 0, V7_PROPERTY_HIDDEN, V7_NULL);
#endif
  return a;
}

/*
 * Dense arrays are backed by mbuf. Currently the array can only grow by
 * appending (i.e. setting an element whose index == array.length)
 *
 * TODO(mkm): automatically promote dense arrays to normal objects
 *            when they are used as sparse arrays or to store arbitrary keys
 *            (perhaps a hybrid approach)
 * TODO(mkm): small sparsness doesn't have to promote the array,
 *            we can just fill empty slots with a tag. In JS missing array
 *            indices are subtly different from indices with an undefined value
 *            (key iteration).
 * TODO(mkm): change the interpreter so it can set elements in dense arrays
 */
V7_PRIVATE val_t v7_create_dense_array(struct v7 *v7) {
  val_t a = v7_create_array(v7);
  v7_set_property(v7, a, "", 0, V7_PROPERTY_HIDDEN, V7_NULL);
  v7_to_object(a)->attributes |= V7_OBJ_DENSE_ARRAY;
  return a;
}

v7_val_t v7_create_regexp(struct v7 *v7, const char *re, size_t re_len,
                          const char *flags, size_t flags_len) {
  struct slre_prog *p = NULL;
  struct v7_regexp *rp;

  if (slre_compile(re, re_len, flags, flags_len, &p, 1) != SLRE_OK ||
      p == NULL) {
    throw_exception(v7, TYPE_ERROR, "Invalid regex");
    return V7_UNDEFINED;
  } else {
    rp = (struct v7_regexp *) malloc(sizeof(*rp));
    rp->regexp_string = v7_create_string(v7, re, re_len, 1);
    rp->compiled_regexp = p;
    rp->lastIndex = 0;

    return v7_pointer_to_value(rp) | V7_TAG_REGEXP;
  }
}

v7_val_t v7_create_foreign(void *p) {
  return v7_pointer_to_value(p) | V7_TAG_FOREIGN;
}

v7_val_t v7_create_function(struct v7 *v7) {
  struct v7_function *f = new_function(v7);
  val_t proto = v7_create_undefined(), fval = v7_function_to_value(f);
  struct gc_tmp_frame tf = new_tmp_frame(v7);
  if (f == NULL) {
    fval = v7_create_null();
    goto cleanup;
  }
  tmp_stack_push(&tf, &proto);
  tmp_stack_push(&tf, &fval);

  f->properties = NULL;
  f->scope = NULL;
  f->attributes = 0;
  /* TODO(mkm): lazily create these properties on first access */
  proto = v7_create_object(v7);
#ifndef V7_DISABLE_PREDEFINED_STRINGS
  v7_set_property_v(v7, proto,
                    v7->predefined_strings[PREDEFINED_STR_CONSTRUCTOR],
                    V7_PROPERTY_DONT_ENUM, fval);
  v7_set_property_v(v7, fval, v7->predefined_strings[PREDEFINED_STR_PROTOTYPE],
                    V7_PROPERTY_DONT_ENUM | V7_PROPERTY_DONT_DELETE, proto);
#else
  v7_set_property(v7, proto, "constructor", 11, V7_PROPERTY_DONT_ENUM, fval);
  v7_set_property(v7, fval, "prototype", 9,
                  V7_PROPERTY_DONT_ENUM | V7_PROPERTY_DONT_DELETE, proto);
#endif

cleanup:
  tmp_frame_cleanup(&tf);
  return fval;
}

/* like snprintf but returns `size` if write is truncated */
static int v_sprintf_s(char *buf, size_t size, const char *fmt, ...) {
  size_t n;
  va_list ap;
  va_start(ap, fmt);
  n = vsnprintf(buf, size, fmt, ap);
  if (n > size) {
    return size;
  }
  return n;
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

V7_PRIVATE int to_str(struct v7 *v7, val_t v, char *buf, size_t size,
                      int as_json) {
  char *vp;
  double num;
  for (vp = v7->json_visited_stack.buf;
       vp < v7->json_visited_stack.buf + v7->json_visited_stack.len;
       vp += sizeof(val_t)) {
    if (*(val_t *) vp == v) {
      strncpy(buf, "[Circular]", size);
      return MIN(10, size);
    }
  }

  switch (val_type(v7, v)) {
    case V7_TYPE_NULL:
      strncpy(buf, "null", size);
      return MIN(4, size);
    case V7_TYPE_UNDEFINED:
      strncpy(buf, "undefined", size);
      return MIN(9, size);
    case V7_TYPE_BOOLEAN:
      if (v7_to_boolean(v)) {
        strncpy(buf, "true", size);
        return MIN(4, size);
      } else {
        strncpy(buf, "false", size);
        return MIN(5, size);
      }
    case V7_TYPE_NUMBER:
      if (v == V7_TAG_NAN) {
        return v_sprintf_s(buf, size, "NaN");
      }
      num = v7_to_double(v);
      if (isinf(num)) {
        return v_sprintf_s(buf, size, "%sInfinity", num < 0.0 ? "-" : "");
      }
      {
        const char *fmt = num > 1e10 ? "%.21g" : "%.10g";
        return v_sprintf_s(buf, size, fmt, num);
      }
    case V7_TYPE_STRING: {
      size_t n;
      const char *str = v7_to_string(v7, &v, &n);
      if (as_json) {
        return v_sprintf_s(buf, size, "\"%.*s\"", (int) n, str);
      } else {
        return v_sprintf_s(buf, size, "%.*s", (int) n, str);
      }
    }
    case V7_TYPE_REGEXP_OBJECT: {
      size_t n1, n2 = 0;
      char s2[3] = {0};
      struct v7_regexp *rp = (struct v7_regexp *) v7_to_pointer(v);
      const char *s1 = v7_to_string(v7, &rp->regexp_string, &n1);
      int flags = slre_get_flags(rp->compiled_regexp);
      if (flags & SLRE_FLAG_G) s2[n2++] = 'g';
      if (flags & SLRE_FLAG_I) s2[n2++] = 'i';
      if (flags & SLRE_FLAG_M) s2[n2++] = 'm';
      return v_sprintf_s(buf, size, "/%.*s/%.*s", (int) n1, s1, (int) n2, s2);
    }
    case V7_TYPE_CFUNCTION:
#ifdef V7_UNIT_TEST
      return v_sprintf_s(buf, size, "cfunc_xxxxxx", v7_to_pointer(v));
#else
      return v_sprintf_s(buf, size, "cfunc_%p", v7_to_pointer(v));
#endif
    case V7_TYPE_CFUNCTION_OBJECT:
      v = i_value_of(v7, v);
      return v_sprintf_s(buf, size, "Function cfunc_%p", v7_to_pointer(v));
    case V7_TYPE_GENERIC_OBJECT:
    case V7_TYPE_BOOLEAN_OBJECT:
    case V7_TYPE_STRING_OBJECT:
    case V7_TYPE_NUMBER_OBJECT:
    case V7_TYPE_DATE_OBJECT:
    case V7_TYPE_ERROR_OBJECT: {
      char *b = buf;
      struct v7_property *p;
      mbuf_append(&v7->json_visited_stack, (char *) &v, sizeof(v));
      b += v_sprintf_s(b, size - (b - buf), "{");
      for (p = v7_to_object(v)->properties; p && (size - (b - buf));
           p = p->next) {
        size_t n;
        const char *s;
        if (p->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)) {
          continue;
        }
        s = v7_to_string(v7, &p->name, &n);
        b += v_sprintf_s(b, size - (b - buf), "\"%.*s\":", (int) n, s);
        b += to_str(v7, p->value, b, size - (b - buf), 1);
        if (p->next) {
          b += v_sprintf_s(b, size - (b - buf), ",");
        }
      }
      b += v_sprintf_s(b, size - (b - buf), "}");
      v7->json_visited_stack.len -= sizeof(v);
      return b - buf;
    }
    case V7_TYPE_ARRAY_OBJECT: {
      val_t el;
      int has;
      char *b = buf;
      size_t i, len = v7_array_length(v7, v);
      mbuf_append(&v7->json_visited_stack, (char *) &v, sizeof(v));
      if (as_json) {
        b += v_sprintf_s(b, size - (b - buf), "[");
      }
      for (i = 0; i < len; i++) {
        el = v7_array_get2(v7, v, i, &has);
        if (has) {
          b += to_str(v7, el, b, size - (b - buf), 1);
        }
        if (i != len - 1) {
          b += v_sprintf_s(b, size - (b - buf), ",");
        }
      }
      if (as_json) {
        b += v_sprintf_s(b, size - (b - buf), "]");
      }
      v7->json_visited_stack.len -= sizeof(v);
      return b - buf;
    }
    case V7_TYPE_FUNCTION_OBJECT: {
      char *name;
      size_t name_len;
      char *b = buf;
      struct v7_function *func = v7_to_function(v);
      ast_off_t body, var, var_end, start, pos = func->ast_off;
      struct ast *a = func->ast;

      b += v_sprintf_s(b, size - (b - buf), "[function");

      V7_CHECK(v7, ast_fetch_tag(a, &pos) == AST_FUNC);
      start = pos - 1;
      body = ast_get_skip(a, pos, AST_FUNC_BODY_SKIP);
      /* TODO(mkm) cleanup this - 1 */
      var = ast_get_skip(a, pos, AST_FUNC_FIRST_VAR_SKIP) - 1;

      ast_move_to_children(a, &pos);
      if (ast_fetch_tag(a, &pos) == AST_IDENT) {
        name = ast_get_inlined_data(a, pos, &name_len);
        ast_move_to_children(a, &pos);
        b += v_sprintf_s(b, size - (b - buf), " %.*s", (int) name_len, name);
      }
      b += v_sprintf_s(b, size - (b - buf), "(");
      while (pos < body) {
        V7_CHECK(v7, ast_fetch_tag(a, &pos) == AST_IDENT);
        name = ast_get_inlined_data(a, pos, &name_len);
        ast_move_to_children(a, &pos);
        b += v_sprintf_s(b, size - (b - buf), "%.*s", (int) name_len, name);
        if (pos < body) {
          b += v_sprintf_s(b, size - (b - buf), ",");
        }
      }
      b += v_sprintf_s(b, size - (b - buf), ")");
      if (var != start) {
        ast_off_t next;
        b += v_sprintf_s(b, size - (b - buf), "{var ");

        do {
          V7_CHECK(v7, ast_fetch_tag(a, &var) == AST_VAR);
          next = ast_get_skip(a, var, AST_VAR_NEXT_SKIP);
          if (next == var) {
            next = 0;
          }

          var_end = ast_get_skip(a, var, AST_END_SKIP);
          ast_move_to_children(a, &var);
          while (var < var_end) {
            V7_CHECK(v7, ast_fetch_tag(a, &var) == AST_VAR_DECL);
            name = ast_get_inlined_data(a, var, &name_len);
            ast_move_to_children(a, &var);
            ast_skip_tree(a, &var);

            b += v_sprintf_s(b, size - (b - buf), "%.*s", (int) name_len, name);
            if (var < var_end || next) {
              b += v_sprintf_s(b, size - (b - buf), ",");
            }
          }
          if (next > 0) {
            var = next - 1; /* TODO(mkm): cleanup */
          }
        } while (next != 0);
        b += v_sprintf_s(b, size - (b - buf), "}");
      }
      b += v_sprintf_s(b, size - (b - buf), "]");
      return b - buf;
    }
    case V7_TYPE_FOREIGN:
      return v_sprintf_s(buf, size, "[foreign]");
    default:
      printf("NOT IMPLEMENTED YET %d\n", val_type(v7, v)); /* LCOV_EXCL_LINE */
      abort();
  }
}

char *v7_to_json(struct v7 *v7, val_t v, char *buf, size_t size) {
  int len = to_str(v7, v, buf, size, 1);

  if (len > (int) size) {
    /* Buffer is not large enough. Allocate a bigger one */
    char *p = (char *) malloc(len + 1);
    to_str(v7, v, p, len + 1, 1);
    p[len] = '\0';
    return p;
  } else {
    return buf;
  }
}

int v7_stringify_value(struct v7 *v7, val_t v, char *buf, size_t size) {
  if (v7_is_string(v)) {
    size_t n;
    const char *str = v7_to_string(v7, &v, &n);
    if (n >= size) {
      n = size - 1;
    }
    strncpy(buf, str, n);
    buf[n] = '\0';
    return n;
  } else {
    return to_str(v7, v, buf, size, 1);
  }
}

V7_PRIVATE struct v7_property *v7_create_property(struct v7 *v7) {
  struct v7_property *p = new_property(v7);
  p->next = NULL;
  p->name = v7_create_undefined();
  p->value = v7_create_undefined();
  p->attributes = 0;
  return p;
}

V7_PRIVATE struct v7_property *v7_get_own_property2(struct v7 *v7, val_t obj,
                                                    const char *name,
                                                    size_t len,
                                                    unsigned int attrs) {
  struct v7_property *p;
  struct v7_object *o;
  val_t ss;
  if (!v7_is_object(obj)) {
    return NULL;
  }
  if (len == (size_t) ~0) {
    len = strlen(name);
  }

  o = v7_to_object(obj);
  /*
   * len check is needed to allow getting the mbuf from the hidden property.
   * TODO(mkm): however hidden properties cannot be safely represented with
   * a zero length string anyway, so this will change.
   */
  if (o->attributes & V7_OBJ_DENSE_ARRAY && len > 0) {
    int ok, has;
    unsigned long i = cstr_to_ulong(name, len, &ok);
    if (ok) {
      v7->cur_dense_prop->value = v7_array_get2(v7, obj, i, &has);
      return has ? v7->cur_dense_prop : NULL;
    }
  }

  if (len <= 5) {
    ss = v7_create_string(v7, name, len, 1);
    for (p = o->properties; p != NULL; p = p->next) {
      if (p->name == ss && (attrs == 0 || (p->attributes & attrs))) {
        return p;
      }
    }
  } else {
    for (p = o->properties; p != NULL; p = p->next) {
      size_t n;
      const char *s = v7_to_string(v7, &p->name, &n);
      if (n == len && strncmp(s, name, len) == 0 &&
          (attrs == 0 || (p->attributes & attrs))) {
        return p;
      }
    }
  }
  return NULL;
}

V7_PRIVATE struct v7_property *v7_get_own_property(struct v7 *v7, val_t obj,
                                                   const char *name,
                                                   size_t len) {
  return v7_get_own_property2(v7, obj, name, len, 0);
}

struct v7_property *v7_get_property(struct v7 *v7, val_t obj, const char *name,
                                    size_t len) {
  if (!v7_is_object(obj)) {
    return NULL;
  }
  for (; obj != V7_NULL; obj = v_get_prototype(v7, obj)) {
    struct v7_property *prop;
    if ((prop = v7_get_own_property(v7, obj, name, len)) != NULL) {
      return prop;
    }
  }
  return NULL;
}

v7_val_t v7_get(struct v7 *v7, val_t obj, const char *name, size_t name_len) {
  val_t v = obj;
  if (v7_is_string(obj)) {
    v = v7->string_prototype;
  } else if (v7_is_regexp(obj)) {
    v = v7->regexp_prototype;
  } else if (v7_is_double(obj)) {
    v = v7->number_prototype;
  } else if (v7_is_boolean(obj)) {
    v = v7->boolean_prototype;
  } else if (v7_is_undefined(obj)) {
    throw_exception(v7, TYPE_ERROR, "cannot read property '%.*s' of undefined",
                    (int) name_len, name);
  } else if (v7_is_cfunction(obj)) {
    return V7_UNDEFINED;
  }
  return v7_property_value(v7, obj, v7_get_property(v7, v, name, name_len));
}

V7_PRIVATE void v7_destroy_property(struct v7_property **p) {
  *p = NULL;
}

int v7_set_v(struct v7 *v7, val_t obj, val_t name, val_t val) {
  size_t len;
  const char *n = v7_to_string(v7, &name, &len);
  struct v7_property *p = v7_get_own_property(v7, obj, n, len);
  if (p == NULL || !(p->attributes & V7_PROPERTY_READ_ONLY)) {
    return v7_set_property_v(v7, obj, name, p == NULL ? 0 : p->attributes, val);
  }
  return -1;
}

int v7_set(struct v7 *v7, val_t obj, const char *name, size_t len, val_t val) {
  struct v7_property *p = v7_get_own_property(v7, obj, name, len);
  if (p == NULL || !(p->attributes & V7_PROPERTY_READ_ONLY)) {
    return v7_set_property(v7, obj, name, len, p == NULL ? 0 : p->attributes,
                           val);
  }
  return -1;
}

V7_PRIVATE void v7_invoke_setter(struct v7 *v7, struct v7_property *prop,
                                 val_t obj, val_t val) {
  val_t setter = prop->value, args = v7_create_dense_array(v7);
  if (prop->attributes & V7_PROPERTY_GETTER) {
    setter = v7_array_get(v7, prop->value, 1);
  }
  v7_array_set(v7, args, 0, val);
  v7_apply(v7, setter, obj, args);
}

V7_PRIVATE struct v7_property *v7_set_prop(struct v7 *v7, val_t obj, val_t name,
                                           unsigned int attributes, val_t val) {
  struct v7_property *prop;
  size_t len;
  const char *n = v7_to_string(v7, &name, &len);

  if (!v7_is_object(obj)) {
    return NULL;
  }

  if (v7_to_object(obj)->attributes & V7_OBJ_NOT_EXTENSIBLE) {
    if (v7->strict_mode) {
      throw_exception(v7, TYPE_ERROR, "Object is not extensible");
    }
    return NULL;
  }

  prop = v7_get_own_property(v7, obj, n, len);
  if (prop == NULL) {
    if ((prop = v7_create_property(v7)) == NULL) {
      return NULL; /* LCOV_EXCL_LINE */
    }
    prop->next = v7_to_object(obj)->properties;
    v7_to_object(obj)->properties = prop;
  }

  if (v7_is_undefined(prop->name)) {
    prop->name = name;
  }
  if (prop->attributes & V7_PROPERTY_SETTER) {
    v7_invoke_setter(v7, prop, obj, val);
    return 0;
  }

  prop->value = val;
  prop->attributes = attributes;
  return prop;
}

int v7_set_property_v(struct v7 *v7, val_t obj, val_t name,
                      unsigned int attributes, v7_val_t val) {
  struct v7_property *prop = v7_set_prop(v7, obj, name, attributes, val);
  return prop == NULL ? -1 : 0;
}

int v7_set_property(struct v7 *v7, val_t obj, const char *name, size_t len,
                    unsigned int attributes, v7_val_t val) {
  val_t n;
  if (len == (size_t) ~0) {
    len = strlen(name);
  }

  n = v7_create_string(v7, name, len, 1);
  return v7_set_property_v(v7, obj, n, attributes, val);
}

int v7_del_property(struct v7 *v7, val_t obj, const char *name, size_t len) {
  struct v7_property *prop, *prev;

  if (!v7_is_object(obj)) {
    return -1;
  }
  if (len == (size_t) ~0) {
    len = strlen(name);
  }
  for (prev = NULL, prop = v7_to_object(obj)->properties; prop != NULL;
       prev = prop, prop = prop->next) {
    size_t n;
    const char *s = v7_to_string(v7, &prop->name, &n);
    if (n == len && strncmp(s, name, len) == 0) {
      if (prev) {
        prev->next = prop->next;
      } else {
        v7_to_object(obj)->properties = prop->next;
      }
      v7_destroy_property(&prop);
      return 0;
    }
  }
  return -1;
}

V7_PRIVATE v7_val_t
v7_create_cfunction_object(struct v7 *v7, v7_cfunction_t f, int num_args) {
  val_t obj = create_object(v7, v7->function_prototype);
  struct gc_tmp_frame tf = new_tmp_frame(v7);
  tmp_stack_push(&tf, &obj);
  v7_set_property(v7, obj, "", 0, V7_PROPERTY_HIDDEN, v7_create_cfunction(f));
#ifndef V7_DISABLE_PREDEFINED_STRINGS
  v7_set_property_v(
      v7, obj, v7->predefined_strings[PREDEFINED_STR_LENGTH],
      V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_ENUM | V7_PROPERTY_DONT_DELETE,
      v7_create_number(num_args));
#else
  v7_set_property(
      v7, obj, "length", 6,
      V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_ENUM | V7_PROPERTY_DONT_DELETE,
      v7_create_number(num_args));
#endif
  tmp_frame_cleanup(&tf);
  return obj;
}

V7_PRIVATE v7_val_t v7_create_cfunction_ctor(struct v7 *v7, val_t proto,
                                             v7_cfunction_t f, int num_args) {
  val_t res = v7_create_cfunction_object(v7, f, num_args);

#ifndef V7_DISABLE_PREDEFINED_STRINGS
  v7_set_property_v(
      v7, res, v7->predefined_strings[PREDEFINED_STR_PROTOTYPE],
      V7_PROPERTY_DONT_ENUM | V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_DELETE,
      proto);

  v7_set_property_v(v7, proto,
                    v7->predefined_strings[PREDEFINED_STR_CONSTRUCTOR],
                    V7_PROPERTY_DONT_ENUM, res);
#else
  v7_set_property(
      v7, res, "prototype", 9,
      V7_PROPERTY_DONT_ENUM | V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_DELETE,
      proto);
  v7_set_property(v7, proto, "constructor", 11, V7_PROPERTY_DONT_ENUM, res);
#endif
  return res;
}

V7_PRIVATE int set_cfunc_obj_prop(struct v7 *v7, val_t o, const char *name,
                                  v7_cfunction_t f, int num_args) {
  return v7_set_property(v7, o, name, strlen(name), V7_PROPERTY_DONT_ENUM,
                         v7_create_cfunction_object(v7, f, num_args));
}

V7_PRIVATE int set_cfunc_prop(struct v7 *v7, val_t o, const char *name,
                              v7_cfunction_t f) {
  return v7_set_property(v7, o, name, strlen(name), 0, v7_create_cfunction(f));
}

V7_PRIVATE val_t
v7_property_value(struct v7 *v7, val_t obj, struct v7_property *p) {
  if (p == NULL) {
    return V7_UNDEFINED;
  }
  if (p->attributes & V7_PROPERTY_GETTER) {
    val_t getter = p->value;
    if (p->attributes & V7_PROPERTY_SETTER) {
      getter = v7_array_get(v7, p->value, 0);
    }
    return v7_apply(v7, getter, obj, V7_UNDEFINED);
  }
  return p->value;
}

unsigned long v7_array_length(struct v7 *v7, val_t v) {
  struct v7_property *p;
  unsigned long key, len = 0;
  char *end;

  if (!v7_is_object(v)) {
    return 0;
  }

  if (v7_to_object(v)->attributes & V7_OBJ_DENSE_ARRAY) {
    struct v7_property *p =
        v7_get_own_property2(v7, v, "", 0, V7_PROPERTY_HIDDEN);
    struct mbuf *abuf;
    if (p == NULL) return 0;
    abuf = (struct mbuf *) v7_to_foreign(p->value);
    if (abuf == NULL) return 0;
    return abuf->len / sizeof(val_t);
  }

  for (p = v7_to_object(v)->properties; p != NULL; p = p->next) {
    size_t n;
    const char *s = v7_to_string(v7, &p->name, &n);
    key = strtoul(s, &end, 10);
    /* Array length could not be more then 2^32 */
    if (end > s && *end == '\0' && key >= len && key < 4294967295UL) {
      len = key + 1;
    }
  }

  return len;
}

int v7_array_set(struct v7 *v7, val_t arr, unsigned long index, val_t v) {
  int res = -1;
  if (v7_is_object(arr)) {
    if (v7_to_object(arr)->attributes & V7_OBJ_DENSE_ARRAY) {
      struct v7_property *p =
          v7_get_own_property2(v7, arr, "", 0, V7_PROPERTY_HIDDEN);
      struct mbuf *abuf;
      unsigned long len;
      assert(p != NULL);
      abuf = (struct mbuf *) v7_to_foreign(p->value);

      if (v7_to_object(arr)->attributes & V7_OBJ_NOT_EXTENSIBLE) {
        if (v7->strict_mode) {
          throw_exception(v7, TYPE_ERROR, "Object is not extensible");
        }
        return res;
      }

      if (abuf == NULL) {
        abuf = (struct mbuf *) malloc(sizeof(*abuf));
        mbuf_init(abuf, sizeof(val_t) * (index + 1));
        p->value = v7_create_foreign(abuf);
      }
      len = abuf->len / sizeof(val_t);
      /* TODO(mkm): possibly promote to sparse array */
      if (index > len) {
        unsigned long i;
        val_t s = V7_TAG_NOVALUE;
        for (i = len; i < index; i++) {
          mbuf_append(abuf, (char *) &s, sizeof(val_t));
        }
        len = index;
      }

      if (index == len) {
        mbuf_append(abuf, (char *) &v, sizeof(val_t));
      } else {
        memcpy(abuf->buf + index * sizeof(val_t), &v, sizeof(val_t));
      }
    } else {
      char buf[20];
      int n = v_sprintf_s(buf, sizeof(buf), "%lu", index);
      res = v7_set(v7, arr, buf, n, v);
    }
  }
  return res;
}

int v7_array_push(struct v7 *v7, v7_val_t arr, v7_val_t v) {
  return v7_array_set(v7, arr, v7_array_length(v7, arr), v);
}

val_t v7_array_get(struct v7 *v7, val_t arr, unsigned long index) {
  return v7_array_get2(v7, arr, index, NULL);
}

val_t v7_array_get2(struct v7 *v7, val_t arr, unsigned long index, int *has) {
  if (v7_is_object(arr)) {
    if (v7_to_object(arr)->attributes & V7_OBJ_DENSE_ARRAY) {
      struct v7_property *p =
          v7_get_own_property2(v7, arr, "", 0, V7_PROPERTY_HIDDEN);
      struct mbuf *abuf = NULL;
      unsigned long len;
      if (p != NULL) {
        abuf = (struct mbuf *) v7_to_foreign(p->value);
      }
      if (abuf == NULL) {
        if (has != NULL) {
          *has = 0;
        }
        return v7_create_undefined();
      }
      len = abuf->len / sizeof(val_t);
      if (index >= len) {
        return v7_create_undefined();
      } else {
        val_t res;
        memcpy(&res, abuf->buf + index * sizeof(val_t), sizeof(val_t));
        if (has != NULL) {
          *has = res != V7_TAG_NOVALUE;
        }
        if (res == V7_TAG_NOVALUE) {
          res = v7_create_undefined();
        }
        return res;
      }
    } else {
      struct v7_property *p;
      char buf[20];
      int n = v_sprintf_s(buf, sizeof(buf), "%lu", index);
      p = v7_get_property(v7, arr, buf, n);
      if (has != NULL) {
        *has = (p != NULL);
      }
      return v7_property_value(v7, arr, p);
    }
  } else {
    return v7_create_undefined();
  }
}

int nextesc(const char **p); /* from SLRE */
V7_PRIVATE size_t unescape(const char *s, size_t len, char *to) {
  const char *end = s + len;
  size_t n = 0;
  char tmp[4];
  Rune r;

  while (s < end) {
    s += chartorune(&r, s);
    if (r == '\\' && s < end) {
      switch (*s) {
        case '"':
          s++, r = '"';
          break;
        case '\'':
          s++, r = '\'';
          break;
        case '\n':
          s++, r = '\n';
          break;
        default: {
          const char *tmp_s = s;
          int i = nextesc(&s);
          switch (i) {
            case -SLRE_INVALID_ESC_CHAR:
              r = '\\';
              s = tmp_s;
              n += runetochar(to == NULL ? tmp : to + n, &r);
              s += chartorune(&r, s);
              break;
            case -SLRE_INVALID_HEX_DIGIT:
            default:
              r = i;
          }
        }
      }
    }
    n += runetochar(to == NULL ? tmp : to + n, &r);
  }

  return n;
}

/* Insert a string into mbuf at specified offset */
V7_PRIVATE void embed_string(struct mbuf *m, size_t offset, const char *p,
                             size_t len, int zero_term, int unesc) {
  char *old_base = m->buf;
  int p_backed_by_mbuf = p >= old_base && p < old_base + m->len;
  size_t n = unesc ? unescape(p, len, NULL) : len;
  int k = calc_llen(n); /* Calculate how many bytes length takes */
  size_t tot_len = k + n + zero_term;
  mbuf_insert(m, offset, NULL, tot_len); /* Allocate  buffer */
  /* Fixup p if it was relocated by mbuf_insert() above */
  if (p_backed_by_mbuf) {
    p += m->buf - old_base;
  }
  encode_varint(n, (unsigned char *) m->buf + offset); /* Write length */
  /* Write string */
  if (unesc) {
    unescape(p, len, m->buf + offset + k);
  } else {
    memcpy(m->buf + offset + k, p, len);
  }
  if (zero_term) {
    m->buf[offset + tot_len - 1] = '\0';
  }
}

/* Create a string */
v7_val_t v7_create_string(struct v7 *v7, const char *p, size_t len, int own) {
  struct mbuf *m = own ? &v7->owned_strings : &v7->foreign_strings;
  val_t offset = m->len, tag = V7_TAG_STRING_F;

  if (len <= 4) {
    char *s = GET_VAL_NAN_PAYLOAD(offset) + 1;
    offset = 0;
    memcpy(s, p, len);
    s[-1] = len;
    tag = V7_TAG_STRING_I;
  } else if (len == 5) {
    char *s = GET_VAL_NAN_PAYLOAD(offset);
    offset = 0;
    memcpy(s, p, len);
    tag = V7_TAG_STRING_5;
  } else if (own) {
    embed_string(m, m->len, p, len, 1, 0);
    tag = V7_TAG_STRING_O;
  } else {
    /* TODO(mkm): this doesn't set correctly the foreign string length */
    embed_string(m, m->len, (char *) &p, sizeof(p), 0, 0);
  }

  /* NOTE(lsm): don't use v7_pointer_to_value, 32-bit ptrs will truncate */
  return (offset & ~V7_TAG_MASK) | tag;
}

V7_PRIVATE val_t to_string(struct v7 *v7, val_t v) {
  char buf[100], *p, *s;
  val_t res;
  if (v7_is_string(v)) {
    return v;
  }

  s = p = v7_to_json(v7, i_value_of(v7, v), buf, sizeof(buf));
  if (p[0] == '"') {
    p[strlen(p) - 1] = '\0';
    s++;
  }
  res = v7_create_string(v7, s, strlen(s), 1);
  if (p != buf) {
    free(p);
  }

  return res;
}

/*
 * Get a pointer to string and string length.
 *
 * Beware that V7 strings are not null terminated!
 */
const char *v7_to_string(struct v7 *v7, val_t *v, size_t *sizep) {
  uint64_t tag = v[0] & V7_TAG_MASK;
  char *p;
  int llen;

  if (tag == V7_TAG_STRING_I) {
    p = GET_VAL_NAN_PAYLOAD(*v) + 1;
    *sizep = p[-1];
  } else if (tag == V7_TAG_STRING_5) {
    p = GET_VAL_NAN_PAYLOAD(*v);
    *sizep = 5;
  } else {
    struct mbuf *m =
        (tag == V7_TAG_STRING_O) ? &v7->owned_strings : &v7->foreign_strings;
    size_t offset = (size_t) v7_to_pointer(*v);
    char *s = m->buf + offset;

    *sizep = decode_varint((uint8_t *) s, &llen);
    if (tag == V7_TAG_STRING_O) {
      p = s + llen;
    } else {
      memcpy(&p, s + llen, sizeof(p));
    }
  }

  return p;
}

V7_PRIVATE int s_cmp(struct v7 *v7, val_t a, val_t b) {
  size_t a_len, b_len;
  const char *a_ptr, *b_ptr;

  a_ptr = v7_to_string(v7, &a, &a_len);
  b_ptr = v7_to_string(v7, &b, &b_len);

  if (a_len == b_len) {
    return memcmp(a_ptr, b_ptr, a_len);
  }
  if (a_len > b_len) {
    return 1;
  } else if (a_len < b_len) {
    return -1;
  } else {
    return 0;
  }
}

V7_PRIVATE val_t s_concat(struct v7 *v7, val_t a, val_t b) {
  size_t a_len, b_len;
  const char *a_ptr, *b_ptr;
  char *s = NULL;
  uint64_t tag = V7_TAG_STRING_F;
  val_t offset = v7->owned_strings.len;

  a_ptr = v7_to_string(v7, &a, &a_len);
  b_ptr = v7_to_string(v7, &b, &b_len);

  /* Create a new string which is a concatenation a + b */
  if (a_len + b_len <= 4) {
    offset = 0;
    /* TODO(mkm): make it work on big endian too */
    s = GET_VAL_NAN_PAYLOAD(offset) + 1;
    s[-1] = a_len + b_len;
    tag = V7_TAG_STRING_I;
  } else if (a_len + b_len == 5) {
    offset = 0;
    /* TODO(mkm): make it work on big endian too */
    s = GET_VAL_NAN_PAYLOAD(offset);
    tag = V7_TAG_STRING_5;
  } else {
    int llen = calc_llen(a_len + b_len);
    mbuf_append(&v7->owned_strings, NULL, a_len + b_len + llen + 1);
    /* all pointers might have been relocated */
    s = v7->owned_strings.buf + offset;
    encode_varint(a_len + b_len, (unsigned char *) s); /* Write length */
    s += llen;
    a_ptr = v7_to_string(v7, &a, &a_len);
    b_ptr = v7_to_string(v7, &b, &b_len);
    tag = V7_TAG_STRING_O;
  }
  memcpy(s, a_ptr, a_len);
  memcpy(s + a_len, b_ptr, b_len);
  /* Inlined strings are already 0-terminated, but still, why not. */
  s[a_len + b_len] = '\0';

  /* NOTE(lsm): don't use v7_pointer_to_value, 32-bit ptrs will truncate */
  return (offset & ~V7_TAG_MASK) | tag;
}

/* Create V7 strings for integers such as array indices */
V7_PRIVATE val_t ulong_to_str(struct v7 *v7, unsigned long n) {
  char buf[100];
  int len;
  len = snprintf(buf, sizeof(buf), "%lu", n);
  return v7_create_string(v7, buf, len, 1);
}

/*
 * Convert a V7 string to to an unsigned integer.
 * `ok` will be set to true if the string conforms to
 * an unsigned long.
 */
V7_PRIVATE unsigned long cstr_to_ulong(const char *s, size_t len, int *ok) {
  char *e;
  unsigned long res = strtoul(s, &e, 10);
  *ok = (e == s + len);
  return res;
}

/*
 * Convert a C string to to an unsigned integer.
 * `ok` will be set to true if the string conforms to
 * an unsigned long.
 */
V7_PRIVATE unsigned long str_to_ulong(struct v7 *v7, val_t v, int *ok) {
  char buf[100];
  size_t len = v7_stringify_value(v7, v, buf, sizeof(buf));
  return cstr_to_ulong(buf, len, ok);
}

V7_PRIVATE int is_prototype_of(struct v7 *v7, val_t o, val_t p) {
  struct v7_object *obj, *proto;
  if (!v7_is_object(o) || !v7_is_object(p)) {
    return 0;
  }
  if (v7_is_function(o)) {
    return p == v7->function_prototype ||
           is_prototype_of(v7, v7->function_prototype, p);
  }
  proto = v7_to_object(p);
  for (obj = v7_to_object(o); obj; obj = obj->prototype) {
    if (obj->prototype == proto) {
      return 1;
    }
  }
  return 0;
}

int v7_is_true(struct v7 *v7, val_t v) {
  size_t len;
  return ((v7_is_boolean(v) && v7_to_boolean(v)) ||
          (v7_is_double(v) && v7_to_double(v) != 0.0) ||
          (v7_is_string(v) && v7_to_string(v7, &v, &len) && len > 0) ||
          (v7_is_object(v))) &&
         v != V7_TAG_NAN;
}

static void object_destructor(struct v7 *v7, void *ptr) {
  struct v7_object *o = (struct v7_object *) ptr;
  struct v7_property *p;
  struct mbuf *abuf;
  if (o->attributes & V7_OBJ_DENSE_ARRAY) {
    p = v7_get_own_property2(v7, v7_object_to_value(o), "", 0,
                             V7_PROPERTY_HIDDEN);
    if (p != NULL &&
        ((abuf = (struct mbuf *) v7_to_foreign(p->value)) != NULL)) {
      mbuf_free(abuf);
    }
  }
}

struct v7 *v7_create(void) {
  struct v7 *v7 = NULL;
  val_t *p;
  char z = 0;

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) != NULL) {
    v7->cur_dense_prop =
        (struct v7_property *) calloc(1, sizeof(struct v7_property));
#define GC_SIZE (64 * 10)
    gc_arena_init(v7, &v7->object_arena, sizeof(struct v7_object), GC_SIZE,
                  "object");
    v7->object_arena.destructor = object_destructor;
    gc_arena_init(v7, &v7->function_arena, sizeof(struct v7_function), GC_SIZE,
                  "function");
    gc_arena_init(v7, &v7->property_arena, sizeof(struct v7_property),
                  GC_SIZE * 3, "property");

    /*
     * The compacting GC exploits the null terminator of the previous
     * string as marker.
     */
    mbuf_append(&v7->owned_strings, &z, 1);

    p = v7->predefined_strings;
    p[PREDEFINED_STR_LENGTH] = v7_create_string(v7, "length", 6, 1);
    p[PREDEFINED_STR_PROTOTYPE] = v7_create_string(v7, "prototype", 9, 1);
    p[PREDEFINED_STR_CONSTRUCTOR] = v7_create_string(v7, "constructor", 11, 1);
    p[PREDEFINED_STR_ARGUMENTS] = v7_create_string(v7, "arguments", 9, 1);

    init_stdlib(v7);
    v7->thrown_error = v7_create_undefined();
  }

  return v7;
}

val_t v7_get_global_object(struct v7 *v7) {
  return v7->global_object;
}

void v7_destroy(struct v7 *v7) {
  struct ast **a;
  if (v7 != NULL) {
    mbuf_free(&v7->owned_strings);
    mbuf_free(&v7->foreign_strings);
    mbuf_free(&v7->json_visited_stack);
    mbuf_free(&v7->tmp_stack);

    for (a = (struct ast **) v7->allocated_asts.buf;
         (char *) a < v7->allocated_asts.buf + v7->allocated_asts.len; a++) {
      ast_free(*a);
      free(*a);
    }
    mbuf_free(&v7->allocated_asts);

    gc_arena_destroy(v7, &v7->object_arena);
    gc_arena_destroy(v7, &v7->function_arena);
    gc_arena_destroy(v7, &v7->property_arena);

    free(v7);
  }
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


#ifdef V7_ENABLE_COMPACTING_GC
void gc_mark_string(struct v7 *, val_t *);
#endif

V7_PRIVATE struct v7_object *new_object(struct v7 *v7) {
  return (struct v7_object *) gc_alloc_cell(v7, &v7->object_arena);
}

V7_PRIVATE struct v7_property *new_property(struct v7 *v7) {
  return (struct v7_property *) gc_alloc_cell(v7, &v7->property_arena);
}

V7_PRIVATE struct v7_function *new_function(struct v7 *v7) {
  return (struct v7_function *) gc_alloc_cell(v7, &v7->function_arena);
}

V7_PRIVATE struct gc_tmp_frame new_tmp_frame(struct v7 *v7) {
  struct gc_tmp_frame frame;
  frame.v7 = v7;
  frame.pos = v7->tmp_stack.len;
  return frame;
}

V7_PRIVATE void tmp_frame_cleanup(struct gc_tmp_frame *tf) {
  tf->v7->tmp_stack.len = tf->pos;
}

/*
 * TODO(mkm): perhaps it's safer to keep val_t in the temporary
 * roots stack, instead of keeping val_t*, in order to be better
 * able to debug the relocating GC.
 */
V7_PRIVATE void tmp_stack_push(struct gc_tmp_frame *tf, val_t *vp) {
  mbuf_append(&tf->v7->tmp_stack, (char *) &vp, sizeof(val_t *));
}

/* Initializes a new arena. */
V7_PRIVATE void gc_arena_init(struct v7 *v7, struct gc_arena *a,
                              size_t cell_size, size_t size, const char *name) {
  assert(cell_size >= sizeof(uintptr_t));
  memset(a, 0, sizeof(*a));
  a->cell_size = cell_size;
  a->name = name;
/* Avoid arena initialization cost when GC is disabled */
#ifndef V7_DISABLE_GC
  gc_arena_grow(v7, a, size);
  assert(a->free != NULL);
#else
  (void) size;
#endif
}

V7_PRIVATE void gc_arena_destroy(struct v7 *v7, struct gc_arena *a) {
  if (a->base != NULL) {
    if (a->destructor != NULL) {
      gc_sweep(v7, a, 0);
    }
    free(a->base);
  }
}

/*
 * Grows the arena by reallocating.
 *
 * The caller is responsible of relocating all the pointers.
 *
 * TODO(mkm): An alternative is to use offsets instead of pointers or
 * instead of growing, maintain a chain of pools, which would also
 * have a smaller memory spike footprint, but it’s slightly more
 * complicated, and can be implemented in a second phase.
 */
V7_PRIVATE void gc_arena_grow(struct v7 *v7, struct gc_arena *a,
                              size_t new_size) {
  size_t free_adjust = a->free ? a->free - a->base : 0;
  size_t old_size = a->size;
  uint32_t old_alive = a->alive;
  a->size = new_size;
  a->base = (char *) realloc(a->base, a->size * a->cell_size);
  memset(a->base + old_size * a->cell_size, 0,
         (a->size - old_size) * a->cell_size);
  /* in case we grow preemptively */
  a->free += free_adjust;
  /* sweep will add the trailing zeroed memory to free list */
  gc_sweep(v7, a, old_size);
  a->alive = old_alive; /* sweeping will decrement `alive` */
}

V7_PRIVATE void *gc_alloc_cell(struct v7 *v7, struct gc_arena *a) {
#ifdef V7_DISABLE_GC
  (void) v7;
  return calloc(1, a->cell_size);
#else
  char **r;
  if (a->free == NULL) {
#if 0
    fprintf(stderr, "Exhausting arena %s, invoking GC.\n", a->name);
#endif
    v7_gc(v7);
    if (a->free == NULL) {
#if 1
      fprintf(stderr, "TODO arena grow\n");
      abort();
#else
      gc_arena_grow(a, a->size * 1.50);
/* TODO(mkm): relocate */
#endif
    }
  }
  r = (char **) a->free;

  UNMARK(r);

  a->free = *r;
  a->allocations++;
  a->alive++;

  /*
   * TODO(mkm): minor opt possible since most of the fields
   * are overwritten downstream, but not worth the yak shave time
   * when fields are added to GC-able structures */
  memset(r, 0, a->cell_size);
  return (void *) r;
#endif
}

/*
 * Scans the arena and add all unmarked cells to the free list.
 */
void gc_sweep(struct v7 *v7, struct gc_arena *a, size_t start) {
  char *cur;
  a->alive = 0;
  a->free = NULL;
  for (cur = a->base + (start * a->cell_size);
       cur < (a->base + (a->size * a->cell_size)); cur += a->cell_size) {
    if (MARKED(cur)) {
      UNMARK(cur);
      a->alive++;
    } else {
      if (a->destructor != NULL) {
        a->destructor(v7, cur);
      }
      memset(cur, 0, a->cell_size);
      *(char **) cur = a->free;
      a->free = cur;
    }
  }
}

V7_PRIVATE void gc_mark(struct v7 *v7, val_t v) {
  struct v7_object *obj;
  struct v7_property *prop;
  struct v7_property *next;

  if (!v7_is_object(v)) {
    return;
  }
  obj = v7_to_object(v);
  if (MARKED(obj)) return;

  for ((prop = obj->properties), MARK(obj); prop != NULL; prop = next) {
#ifdef V7_ENABLE_COMPACTING_GC
    gc_mark_string(v7, &prop->value);
    gc_mark_string(v7, &prop->name);
#endif
    gc_mark(v7, prop->value);

    next = prop->next;

    assert((char *) prop >= v7->property_arena.base &&
           (char *) prop <
               (v7->property_arena.base +
                v7->property_arena.size * v7->property_arena.cell_size));
    MARK(prop);
  }

  /* function scope pointer is aliased to the object's prototype pointer */
  gc_mark(v7, v7_object_to_value(obj->prototype));
}

static void gc_dump_arena_stats(const char *msg, struct gc_arena *a) {
  if (a->verbose) {
    fprintf(stderr, "%s: total allocations %lu, max %lu, alive %lu\n", msg,
            a->allocations, a->size, a->alive);
  }
}

#ifdef V7_ENABLE_COMPACTING_GC

uint64_t gc_string_val_to_offset(val_t v) {
  return ((uint64_t) v7_to_pointer(v)) & ~V7_TAG_MASK;
}

val_t gc_string_val_from_offset(uint64_t s) {
  return s | V7_TAG_STRING_O;
}

/* Mark a string value */
void gc_mark_string(struct v7 *v7, val_t *v) {
  val_t h, tmp = 0;
  char *s;

  if (((*v & V7_TAG_MASK) != V7_TAG_STRING_O) &&
      (*v & V7_TAG_MASK) != V7_TAG_STRING_C) {
    return;
  }

  /*
   * If a value points to an unmarked string we shall:
   *  1. save the first 6 bytes of the string
   *     since we need to be able to distinguish real values from
   *     the saved first 6 bytes of the string, we need to tag the chunk
   *     as V7_TAG_STRING_C
   *  2. encode value's address (v) into the first bytes of the string.
   *     the first byte is set to 0 to serve as a mark.
   *     The remaining 6 bytes are taken from v's least significant bytes.
   *  3. put the saved 8 bytes (tag + chunk) back into the value.
   *
   * If a value points to an already marked string we shall:
   *     (0, <6 bytes of a pointer to a val_t>), hence we have to skip
   *     the first byte. We tag the value pointer as a V7_TAG_FOREIGN
   *     so that it won't be followed during recursive mark.
   *
   *  ... the rest is the same
   *
   *  Note: 64-bit pointers can be represented with 48-bits
   */

  s = v7->owned_strings.buf + gc_string_val_to_offset(*v);
  if (s[-1] == '\0') {
    memcpy(&tmp, s, sizeof(tmp) - 2);
    tmp |= V7_TAG_STRING_C;
  } else {
    memcpy(&tmp, s, sizeof(tmp) - 2);
    tmp |= V7_TAG_FOREIGN;
  }

  h = (val_t) v;
  s[-1] = 1;
  memcpy(s, &h, sizeof(h) - 2);
  memcpy(v, &tmp, sizeof(tmp));
}

void gc_compact_strings(struct v7 *v7) {
  char *p = v7->owned_strings.buf + 1;
  uint64_t h, next, head = 1;
  int len, llen;

  while (p < v7->owned_strings.buf + v7->owned_strings.len) {
    if (p[-1] == '\1') {
      /* relocate and update ptrs */
      h = 0;
      memcpy(&h, p, sizeof(h) - 2);

      /*
       * relocate pointers until we find the tail.
       * The tail is marked with V7_TAG_STRING_C,
       * while val_t link pointers are tagged with V7_TAG_FOREIGN
       */
      for (; (h & V7_TAG_MASK) != V7_TAG_STRING_C; h = next) {
        h &= ~V7_TAG_MASK;
        memcpy(&next, (char *) h, sizeof(h));

        *(val_t *) h = gc_string_val_from_offset(head);
      }
      h &= ~V7_TAG_MASK;

      /*
       * the tail contains the first 6 bytes we stole from
       * the actual string.
       */
      len = decode_varint((unsigned char *) &h, &llen);
      len += llen + 1;

      /*
       * restore the saved 6 bytes
       * TODO(mkm): think about endianness
       */
      memcpy(p, &h, sizeof(h) - 2);

      /*
       * and relocate the string data by packing it to the left.
       */
      memmove(v7->owned_strings.buf + head, p, len);
      v7->owned_strings.buf[head - 1] = 0x0;
      p += len;
      head += len;
    } else {
      len = decode_varint((unsigned char *) p, &llen);
      len += llen + 1;

      p += len;
    }
  }

  v7->owned_strings.len = head;
}

void gc_dump_owned_strings(struct v7 *v7) {
  size_t i;
#if 0
  for (i = 0; i < v7->owned_strings.len; i++) {
    printf("%02x ", (uint8_t) v7->owned_strings.buf[i]);
  }
  printf("\n");
  for (i = 0; i < v7->owned_strings.len; i++) {
    if (isprint(v7->owned_strings.buf[i])) {
      printf(" %c ", v7->owned_strings.buf[i]);
    } else {
      printf(" . ");
    }
  }
#else
  for (i = 0; i < v7->owned_strings.len; i++) {
    if (isprint(v7->owned_strings.buf[i])) {
      printf("%c", v7->owned_strings.buf[i]);
    } else {
      printf(".");
    }
  }
#endif
  printf("\n");
}

#endif

/* Perform garbage collection */
void v7_gc(struct v7 *v7) {
  val_t **vp;

  gc_dump_arena_stats("Before GC objects", &v7->object_arena);
  gc_dump_arena_stats("Before GC functions", &v7->function_arena);
  gc_dump_arena_stats("Before GC properties", &v7->property_arena);

  /* TODO(mkm): paranoia? */
  gc_mark(v7, v7->object_prototype);
  gc_mark(v7, v7->array_prototype);
  gc_mark(v7, v7->boolean_prototype);
  gc_mark(v7, v7->error_prototype);
  gc_mark(v7, v7->string_prototype);
  gc_mark(v7, v7->number_prototype);
  gc_mark(v7, v7->function_prototype); /* possibly not reachable */
  gc_mark(v7, v7->this_object);

  gc_mark(v7, v7->object_prototype);
  gc_mark(v7, v7->global_object);
  gc_mark(v7, v7->this_object);
  gc_mark(v7, v7->call_stack);

  for (vp = (val_t **) v7->tmp_stack.buf;
       (char *) vp < v7->tmp_stack.buf + v7->tmp_stack.len; vp++) {
    gc_mark(v7, **vp);
  }

#ifdef V7_ENABLE_COMPACTING_GC
  gc_compact_strings(v7);
#endif

  gc_sweep(v7, &v7->object_arena, 0);
  gc_sweep(v7, &v7->function_arena, 0);
  gc_sweep(v7, &v7->property_arena, 0);

  gc_dump_arena_stats("After GC objects", &v7->object_arena);
  gc_dump_arena_stats("After GC functions", &v7->function_arena);
  gc_dump_arena_stats("After GC properties", &v7->property_arena);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


#define ACCEPT(t) (((v7)->cur_tok == (t)) ? next_tok((v7)), 1 : 0)

#define PARSE(p) TRY(parse_##p(v7, a))
#define PARSE_ARG(p, arg) TRY(parse_##p(v7, a, arg))
#define PARSE_ARG_2(p, a1, a2) TRY(parse_##p(v7, a, a1, a2))

#define TRY(call)           \
  do {                      \
    enum v7_err _e = call;  \
    CHECK(_e == V7_OK, _e); \
  } while (0)

#define THROW(err_code)                                                       \
  do {                                                                        \
    snprintf(v7->error_msg, sizeof(v7->error_msg), "Parse error: %s line %d", \
             v7->pstate.file_name, v7->pstate.line_no);                       \
    return (err_code);                                                        \
  } while (0)

#define CHECK(cond, code)     \
  do {                        \
    if (!(cond)) THROW(code); \
  } while (0)

#define EXPECT(t)                                     \
  do {                                                \
    if ((v7)->cur_tok != (t)) return V7_SYNTAX_ERROR; \
    next_tok(v7);                                     \
  } while (0)

static enum v7_err parse_expression(struct v7 *, struct ast *);
static enum v7_err parse_statement(struct v7 *, struct ast *);
static enum v7_err parse_terminal(struct v7 *, struct ast *);
static enum v7_err parse_assign(struct v7 *, struct ast *);
static enum v7_err parse_memberexpr(struct v7 *, struct ast *);
static enum v7_err parse_funcdecl(struct v7 *, struct ast *, int, int);
static enum v7_err parse_block(struct v7 *, struct ast *);
static enum v7_err parse_body(struct v7 *, struct ast *, enum v7_tok);
static enum v7_err parse_use_strict(struct v7 *, struct ast *);

static enum v7_tok lookahead(const struct v7 *v7) {
  const char *s = v7->pstate.pc;
  double d;
  return get_tok(&s, &d, v7->cur_tok);
}

static enum v7_tok next_tok(struct v7 *v7) {
  int prev_line_no = v7->pstate.prev_line_no;
  v7->pstate.prev_line_no = v7->pstate.line_no;
  v7->pstate.line_no += skip_to_next_tok(&v7->pstate.pc);
  v7->after_newline = prev_line_no != v7->pstate.line_no;
  v7->tok = v7->pstate.pc;
  v7->cur_tok = get_tok(&v7->pstate.pc, &v7->cur_tok_dbl, v7->cur_tok);
  v7->tok_len = v7->pstate.pc - v7->tok;
  v7->pstate.line_no += skip_to_next_tok(&v7->pstate.pc);
  return v7->cur_tok;
}

static enum v7_err parse_ident(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok == TOK_IDENTIFIER) {
    ast_add_inlined_node(a, AST_IDENT, v7->tok, v7->tok_len);
    next_tok(v7);
    return V7_OK;
  }
  return V7_SYNTAX_ERROR;
}

static enum v7_err parse_ident_allow_reserved_words(struct v7 *v7,
                                                    struct ast *a) {
  /* Allow reserved words as property names. */
  if (is_reserved_word_token(v7->cur_tok)) {
    ast_add_inlined_node(a, AST_IDENT, v7->tok, v7->tok_len);
    next_tok(v7);
  } else {
    PARSE(ident);
  }
  return V7_OK;
}

static enum v7_err parse_prop(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok == TOK_IDENTIFIER &&
      strncmp(v7->tok, "get", v7->tok_len) == 0 && lookahead(v7) != TOK_COLON) {
    next_tok(v7);
    ast_add_node(a, AST_GETTER);
    parse_funcdecl(v7, a, 1, 1);
  } else if (v7->cur_tok == TOK_IDENTIFIER &&
             strncmp(v7->tok, "set", v7->tok_len) == 0 &&
             lookahead(v7) != TOK_COLON) {
    next_tok(v7);
    ast_add_node(a, AST_SETTER);
    parse_funcdecl(v7, a, 1, 1);
  } else {
    /* Allow reserved words as property names. */
    if (is_reserved_word_token(v7->cur_tok) || v7->cur_tok == TOK_IDENTIFIER ||
        v7->cur_tok == TOK_NUMBER) {
      ast_add_inlined_node(a, AST_PROP, v7->tok, v7->tok_len);
    } else if (v7->cur_tok == TOK_STRING_LITERAL) {
      ast_add_inlined_node(a, AST_PROP, v7->tok + 1, v7->tok_len - 2);
    } else {
      return V7_SYNTAX_ERROR;
    }
    next_tok(v7);
    EXPECT(TOK_COLON);
    PARSE(assign);
  }
  return V7_OK;
}

static enum v7_err parse_terminal(struct v7 *v7, struct ast *a) {
  ast_off_t start;
  switch (v7->cur_tok) {
    case TOK_OPEN_PAREN:
      next_tok(v7);
      PARSE(expression);
      EXPECT(TOK_CLOSE_PAREN);
      break;
    case TOK_OPEN_BRACKET:
      next_tok(v7);
      start = ast_add_node(a, AST_ARRAY);
      while (v7->cur_tok != TOK_CLOSE_BRACKET) {
        if (v7->cur_tok == TOK_COMMA) {
          /* Array literals allow missing elements, e.g. [,,1,] */
          ast_add_node(a, AST_NOP);
        } else {
          PARSE(assign);
        }
        ACCEPT(TOK_COMMA);
      }
      EXPECT(TOK_CLOSE_BRACKET);
      ast_set_skip(a, start, AST_END_SKIP);
      break;
    case TOK_OPEN_CURLY:
      next_tok(v7);
      start = ast_add_node(a, AST_OBJECT);
      if (v7->cur_tok != TOK_CLOSE_CURLY) {
        do {
          if (v7->cur_tok == TOK_CLOSE_CURLY) {
            break;
          }
          PARSE(prop);
        } while (ACCEPT(TOK_COMMA));
      }
      EXPECT(TOK_CLOSE_CURLY);
      ast_set_skip(a, start, AST_END_SKIP);
      break;
    case TOK_THIS:
      next_tok(v7);
      ast_add_node(a, AST_THIS);
      break;
    case TOK_TRUE:
      next_tok(v7);
      ast_add_node(a, AST_TRUE);
      break;
    case TOK_FALSE:
      next_tok(v7);
      ast_add_node(a, AST_FALSE);
      break;
    case TOK_NULL:
      next_tok(v7);
      ast_add_node(a, AST_NULL);
      break;
    case TOK_STRING_LITERAL:
      ast_add_inlined_node(a, AST_STRING, v7->tok + 1, v7->tok_len - 2);
      next_tok(v7);
      break;
    case TOK_NUMBER:
      ast_add_inlined_node(a, AST_NUM, v7->tok, v7->tok_len);
      next_tok(v7);
      break;
    case TOK_REGEX_LITERAL:
      ast_add_inlined_node(a, AST_REGEX, v7->tok, v7->tok_len);
      next_tok(v7);
      break;
    case TOK_IDENTIFIER:
      if (strncmp(v7->tok, "undefined", v7->tok_len) == 0) {
        ast_add_node(a, AST_UNDEFINED);
        next_tok(v7);
        break;
      }
    /* fall through */
    default:
      PARSE(ident);
  }
  return V7_OK;
}

static enum v7_err parse_arglist(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok != TOK_CLOSE_PAREN) {
    do {
      PARSE(assign);
    } while (ACCEPT(TOK_COMMA));
  }
  return V7_OK;
}

static enum v7_err parse_newexpr(struct v7 *v7, struct ast *a) {
  ast_off_t start;
  switch (v7->cur_tok) {
    case TOK_NEW:
      next_tok(v7);
      start = ast_add_node(a, AST_NEW);
      PARSE(memberexpr);
      if (ACCEPT(TOK_OPEN_PAREN)) {
        PARSE(arglist);
        EXPECT(TOK_CLOSE_PAREN);
      }
      ast_set_skip(a, start, AST_END_SKIP);
      break;
    case TOK_FUNCTION:
      next_tok(v7);
      PARSE_ARG_2(funcdecl, 0, 0);
      break;
    default:
      PARSE(terminal);
      break;
  }
  return V7_OK;
}

static enum v7_err parse_member(struct v7 *v7, struct ast *a, ast_off_t pos) {
  switch (v7->cur_tok) {
    case TOK_DOT:
      next_tok(v7);
      /* Allow reserved words as member identifiers */
      if (is_reserved_word_token(v7->cur_tok) ||
          v7->cur_tok == TOK_IDENTIFIER) {
        ast_insert_inlined_node(a, pos, AST_MEMBER, v7->tok, v7->tok_len);
        next_tok(v7);
      } else {
        return V7_SYNTAX_ERROR;
      }
      break;
    case TOK_OPEN_BRACKET:
      next_tok(v7);
      PARSE(expression);
      EXPECT(TOK_CLOSE_BRACKET);
      ast_insert_node(a, pos, AST_INDEX);
      break;
    default:
      return V7_OK;
  }
  return V7_OK;
}

static enum v7_err parse_memberexpr(struct v7 *v7, struct ast *a) {
  ast_off_t pos = a->mbuf.len;
  PARSE(newexpr);

  for (;;) {
    switch (v7->cur_tok) {
      case TOK_DOT:
      case TOK_OPEN_BRACKET:
        PARSE_ARG(member, pos);
        break;
      default:
        return V7_OK;
    }
  }
}

static enum v7_err parse_callexpr(struct v7 *v7, struct ast *a) {
  ast_off_t pos = a->mbuf.len;
  PARSE(newexpr);

  for (;;) {
    switch (v7->cur_tok) {
      case TOK_DOT:
      case TOK_OPEN_BRACKET:
        PARSE_ARG(member, pos);
        break;
      case TOK_OPEN_PAREN:
        next_tok(v7);
        PARSE(arglist);
        EXPECT(TOK_CLOSE_PAREN);
        ast_insert_node(a, pos, AST_CALL);
        break;
      default:
        return V7_OK;
    }
  }
}

static enum v7_err parse_postfix(struct v7 *v7, struct ast *a) {
  ast_off_t pos = a->mbuf.len;
  PARSE(callexpr);

  if (v7->after_newline) {
    return V7_OK;
  }
  switch (v7->cur_tok) {
    case TOK_PLUS_PLUS:
      next_tok(v7);
      ast_insert_node(a, pos, AST_POSTINC);
      break;
    case TOK_MINUS_MINUS:
      next_tok(v7);
      ast_insert_node(a, pos, AST_POSTDEC);
      break;
    default:
      break; /* nothing */
  }
  return V7_OK;
}

enum v7_err parse_prefix(struct v7 *v7, struct ast *a) {
  for (;;) {
    switch (v7->cur_tok) {
      case TOK_PLUS:
        next_tok(v7);
        ast_add_node(a, AST_POSITIVE);
        break;
      case TOK_MINUS:
        next_tok(v7);
        ast_add_node(a, AST_NEGATIVE);
        break;
      case TOK_PLUS_PLUS:
        next_tok(v7);
        ast_add_node(a, AST_PREINC);
        break;
      case TOK_MINUS_MINUS:
        next_tok(v7);
        ast_add_node(a, AST_PREDEC);
        break;
      case TOK_TILDA:
        next_tok(v7);
        ast_add_node(a, AST_NOT);
        break;
      case TOK_NOT:
        next_tok(v7);
        ast_add_node(a, AST_LOGICAL_NOT);
        break;
      case TOK_VOID:
        next_tok(v7);
        ast_add_node(a, AST_VOID);
        break;
      case TOK_DELETE:
        next_tok(v7);
        ast_add_node(a, AST_DELETE);
        break;
      case TOK_TYPEOF:
        next_tok(v7);
        ast_add_node(a, AST_TYPEOF);
        break;
      default:
        return parse_postfix(v7, a);
    }
  }
}

static enum v7_err parse_binary(struct v7 *v7, struct ast *a, int level,
                                ast_off_t pos) {
#define NONE \
  { (enum v7_tok) 0, (enum v7_tok) 0, (enum ast_tag) 0 }

  struct {
    int len, left_to_right;
    struct {
      enum v7_tok start_tok, end_tok;
      enum ast_tag start_ast;
    } parts[2];
  } levels[] = {
      {1, 0, {{TOK_ASSIGN, TOK_URSHIFT_ASSIGN, AST_ASSIGN}, NONE}},
      {1, 0, {{TOK_QUESTION, TOK_QUESTION, AST_COND}, NONE}},
      {1, 1, {{TOK_LOGICAL_OR, TOK_LOGICAL_OR, AST_LOGICAL_OR}, NONE}},
      {1, 1, {{TOK_LOGICAL_AND, TOK_LOGICAL_AND, AST_LOGICAL_AND}, NONE}},
      {1, 1, {{TOK_OR, TOK_OR, AST_OR}, NONE}},
      {1, 1, {{TOK_XOR, TOK_XOR, AST_XOR}, NONE}},
      {1, 1, {{TOK_AND, TOK_AND, AST_AND}, NONE}},
      {1, 1, {{TOK_EQ, TOK_NE_NE, AST_EQ}, NONE}},
      {2, 1, {{TOK_LE, TOK_GT, AST_LE}, {TOK_IN, TOK_INSTANCEOF, AST_IN}}},
      {1, 1, {{TOK_LSHIFT, TOK_URSHIFT, AST_LSHIFT}, NONE}},
      {1, 1, {{TOK_PLUS, TOK_MINUS, AST_ADD}, NONE}},
      {1, 1, {{TOK_REM, TOK_DIV, AST_REM}, NONE}}};

  int i;
  enum v7_tok tok;
  enum ast_tag ast;

  if (level == (int) ARRAY_SIZE(levels) - 1) {
    PARSE(prefix);
  } else {
    TRY(parse_binary(v7, a, level + 1, a->mbuf.len));
  }

  for (i = 0; i < levels[level].len; i++) {
    tok = levels[level].parts[i].start_tok;
    ast = levels[level].parts[i].start_ast;
    do {
      if (v7->pstate.inhibit_in && tok == TOK_IN) {
        continue;
      }

      /*
       * Ternary operator sits in the middle of the binary operator
       * precedence chain. Deal with it as an exception and don't break
       * the chain.
       */
      if (tok == TOK_QUESTION && v7->cur_tok == TOK_QUESTION) {
        next_tok(v7);
        PARSE(assign);
        EXPECT(TOK_COLON);
        PARSE(assign);
        ast_insert_node(a, pos, AST_COND);
        return V7_OK;
      } else if (ACCEPT(tok)) {
        if (levels[level].left_to_right) {
          ast_insert_node(a, pos, ast);
          TRY(parse_binary(v7, a, level, pos));
        } else {
          TRY(parse_binary(v7, a, level, a->mbuf.len));
          ast_insert_node(a, pos, ast);
        }
      }
    } while (
        ast = (enum ast_tag)(ast + 1),
        tok < levels[level].parts[i].end_tok && (tok = (enum v7_tok)(tok + 1)));
  }

  return V7_OK;
}

static enum v7_err parse_assign(struct v7 *v7, struct ast *a) {
  return parse_binary(v7, a, 0, a->mbuf.len);
}

static enum v7_err parse_expression(struct v7 *v7, struct ast *a) {
  ast_off_t pos = a->mbuf.len;
  int group = 0;
  do {
    PARSE(assign);
  } while (ACCEPT(TOK_COMMA) && (group = 1));
  if (group) {
    ast_insert_node(a, pos, AST_SEQ);
  }
  return V7_OK;
}

static enum v7_err end_of_statement(struct v7 *v7) {
  if (v7->cur_tok == TOK_SEMICOLON || v7->cur_tok == TOK_END_OF_INPUT ||
      v7->cur_tok == TOK_CLOSE_CURLY || v7->after_newline) {
    return V7_OK;
  }
  return V7_SYNTAX_ERROR;
}

static enum v7_err parse_var(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_VAR);
  ast_modify_skip(a, v7->last_var_node, start, AST_FUNC_FIRST_VAR_SKIP);
  /* zero out var node pointer */
  ast_modify_skip(a, start, start, AST_FUNC_FIRST_VAR_SKIP);
  v7->last_var_node = start;
  do {
    ast_add_inlined_node(a, AST_VAR_DECL, v7->tok, v7->tok_len);
    EXPECT(TOK_IDENTIFIER);
    if (ACCEPT(TOK_ASSIGN)) {
      PARSE(assign);
    } else {
      ast_add_node(a, AST_NOP);
    }
  } while (ACCEPT(TOK_COMMA));
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static int parse_optional(struct v7 *v7, struct ast *a,
                          enum v7_tok terminator) {
  if (v7->cur_tok != terminator) {
    return 1;
  }
  ast_add_node(a, AST_NOP);
  return 0;
}

static enum v7_err parse_if(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_IF);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  PARSE(statement);
  ast_set_skip(a, start, AST_END_IF_TRUE_SKIP);
  if (ACCEPT(TOK_ELSE)) {
    PARSE(statement);
  }
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err parse_while(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_WHILE);
  int saved_in_loop = v7->pstate.in_loop;
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  v7->pstate.in_loop = 1;
  PARSE(statement);
  ast_set_skip(a, start, AST_END_SKIP);
  v7->pstate.in_loop = saved_in_loop;
  return V7_OK;
}

static enum v7_err parse_dowhile(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_DOWHILE);
  int saved_in_loop = v7->pstate.in_loop;
  v7->pstate.in_loop = 1;
  PARSE(statement);
  v7->pstate.in_loop = saved_in_loop;
  ast_set_skip(a, start, AST_DO_WHILE_COND_SKIP);
  EXPECT(TOK_WHILE);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err parse_for(struct v7 *v7, struct ast *a) {
  /* TODO(mkm): for of, for each in */
  ast_off_t start = ast_add_node(a, AST_FOR);
  int saved_in_loop = v7->pstate.in_loop;

  EXPECT(TOK_OPEN_PAREN);

  if (parse_optional(v7, a, TOK_SEMICOLON)) {
    /*
     * TODO(mkm): make this reentrant otherwise this pearl won't parse:
     * for((function(){return 1 in o.a ? o : x})().a in [1,2,3])
     */
    v7->pstate.inhibit_in = 1;
    if (ACCEPT(TOK_VAR)) {
      PARSE(var);
    } else {
      PARSE(expression);
    }
    v7->pstate.inhibit_in = 0;

    if (ACCEPT(TOK_IN)) {
      PARSE(expression);
      ast_add_node(a, AST_NOP);
      /*
       * Assumes that for and for in have the same AST format which is
       * suboptimal but avoids the need of fixing up the var offset chain.
       * TODO(mkm) improve this
       */
      a->mbuf.buf[start - 1] = AST_FOR_IN;
      goto body;
    }
  }

  EXPECT(TOK_SEMICOLON);
  if (parse_optional(v7, a, TOK_SEMICOLON)) {
    PARSE(expression);
  }
  EXPECT(TOK_SEMICOLON);
  if (parse_optional(v7, a, TOK_CLOSE_PAREN)) {
    PARSE(expression);
  }

body:
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_FOR_BODY_SKIP);
  v7->pstate.in_loop = 1;
  PARSE(statement);
  v7->pstate.in_loop = saved_in_loop;
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err parse_switch(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_SWITCH);
  int saved_in_switch = v7->pstate.in_switch;
  ast_set_skip(a, start, AST_SWITCH_DEFAULT_SKIP); /* clear out */
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  EXPECT(TOK_OPEN_CURLY);
  v7->pstate.in_switch = 1;
  while (v7->cur_tok != TOK_CLOSE_CURLY) {
    ast_off_t case_start;
    switch (v7->cur_tok) {
      case TOK_CASE:
        next_tok(v7);
        case_start = ast_add_node(a, AST_CASE);
        PARSE(expression);
        EXPECT(TOK_COLON);
        while (v7->cur_tok != TOK_CASE && v7->cur_tok != TOK_DEFAULT &&
               v7->cur_tok != TOK_CLOSE_CURLY) {
          PARSE(statement);
        }
        ast_set_skip(a, case_start, AST_END_SKIP);
        break;
      case TOK_DEFAULT:
        next_tok(v7);
        EXPECT(TOK_COLON);
        ast_set_skip(a, start, AST_SWITCH_DEFAULT_SKIP);
        case_start = ast_add_node(a, AST_DEFAULT);
        while (v7->cur_tok != TOK_CASE && v7->cur_tok != TOK_DEFAULT &&
               v7->cur_tok != TOK_CLOSE_CURLY) {
          PARSE(statement);
        }
        ast_set_skip(a, case_start, AST_END_SKIP);
        break;
      default:
        return V7_SYNTAX_ERROR;
    }
  }
  EXPECT(TOK_CLOSE_CURLY);
  ast_set_skip(a, start, AST_END_SKIP);
  v7->pstate.in_switch = saved_in_switch;
  return V7_OK;
}

static enum v7_err parse_try(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_TRY);
  PARSE(block);
  ast_set_skip(a, start, AST_TRY_CATCH_SKIP);
  if (ACCEPT(TOK_CATCH)) {
    EXPECT(TOK_OPEN_PAREN);
    PARSE(ident);
    EXPECT(TOK_CLOSE_PAREN);
    PARSE(block);
  }
  ast_set_skip(a, start, AST_TRY_FINALLY_SKIP);
  if (ACCEPT(TOK_FINALLY)) {
    PARSE(block);
  }
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err parse_with(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_WITH);
  if (v7->pstate.in_strict) {
    return V7_SYNTAX_ERROR;
  }
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  PARSE(statement);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

#define PARSE_WITH_OPT_ARG(tag, arg_tag, arg_parser) \
  do {                                               \
    if (end_of_statement(v7) == V7_OK) {             \
      ast_add_node(a, tag);                          \
    } else {                                         \
      ast_add_node(a, arg_tag);                      \
      PARSE(arg_parser);                             \
    }                                                \
  } while (0)

static enum v7_err parse_statement(struct v7 *v7, struct ast *a) {
  switch (v7->cur_tok) {
    case TOK_SEMICOLON:
      next_tok(v7);
      return V7_OK;      /* empty statement */
    case TOK_OPEN_CURLY: /* block */
      PARSE(block);
      return V7_OK; /* returning because no semicolon required */
    case TOK_IF:
      next_tok(v7);
      return parse_if(v7, a);
    case TOK_WHILE:
      next_tok(v7);
      return parse_while(v7, a);
    case TOK_DO:
      next_tok(v7);
      return parse_dowhile(v7, a);
    case TOK_FOR:
      next_tok(v7);
      return parse_for(v7, a);
    case TOK_TRY:
      next_tok(v7);
      return parse_try(v7, a);
    case TOK_SWITCH:
      next_tok(v7);
      return parse_switch(v7, a);
    case TOK_WITH:
      next_tok(v7);
      return parse_with(v7, a);
    case TOK_BREAK:
      if (!(v7->pstate.in_loop || v7->pstate.in_switch)) {
        return V7_SYNTAX_ERROR;
      }
      next_tok(v7);
      PARSE_WITH_OPT_ARG(AST_BREAK, AST_LABELED_BREAK, ident);
      break;
    case TOK_CONTINUE:
      if (!v7->pstate.in_loop) {
        return V7_SYNTAX_ERROR;
      }
      next_tok(v7);
      PARSE_WITH_OPT_ARG(AST_CONTINUE, AST_LABELED_CONTINUE, ident);
      break;
    case TOK_RETURN:
      if (!v7->pstate.in_function) {
        return V7_SYNTAX_ERROR;
      }
      next_tok(v7);
      PARSE_WITH_OPT_ARG(AST_RETURN, AST_VALUE_RETURN, expression);
      break;
    case TOK_THROW:
      next_tok(v7);
      ast_add_node(a, AST_THROW);
      PARSE(expression);
      break;
    case TOK_DEBUGGER:
      next_tok(v7);
      ast_add_node(a, AST_DEBUGGER);
      break;
    case TOK_VAR:
      next_tok(v7);
      PARSE(var);
      break;
    case TOK_IDENTIFIER:
      if (lookahead(v7) == TOK_COLON) {
        ast_add_inlined_node(a, AST_LABEL, v7->tok, v7->tok_len);
        next_tok(v7);
        EXPECT(TOK_COLON);
        return V7_OK;
      }
    /* fall through */
    default:
      PARSE(expression);
      break;
  }

  TRY(end_of_statement(v7));
  ACCEPT(TOK_SEMICOLON); /* swallow optional semicolon */
  return V7_OK;
}

static enum v7_err parse_funcdecl(struct v7 *v7, struct ast *a,
                                  int require_named, int reserved_name) {
  ast_off_t start = ast_add_node(a, AST_FUNC);
  ast_off_t outer_last_var_node = v7->last_var_node;
  int saved_in_function = v7->pstate.in_function;
  int saved_in_strict = v7->pstate.in_strict;
  v7->last_var_node = start;
  ast_modify_skip(a, start, start, AST_FUNC_FIRST_VAR_SKIP);
  if ((reserved_name ? parse_ident_allow_reserved_words : parse_ident)(v7, a) ==
      V7_SYNTAX_ERROR) {
    if (require_named) {
      return V7_SYNTAX_ERROR;
    }
    ast_add_node(a, AST_NOP);
  }
  EXPECT(TOK_OPEN_PAREN);
  PARSE(arglist);
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_FUNC_BODY_SKIP);
  v7->pstate.in_function = 1;
  EXPECT(TOK_OPEN_CURLY);
  if (parse_use_strict(v7, a) == V7_OK) {
    v7->pstate.in_strict = 1;
  }
  PARSE_ARG(body, TOK_CLOSE_CURLY);
  EXPECT(TOK_CLOSE_CURLY);
  v7->pstate.in_strict = saved_in_strict;
  v7->pstate.in_function = saved_in_function;
  ast_set_skip(a, start, AST_END_SKIP);
  v7->last_var_node = outer_last_var_node;
  return V7_OK;
}

static enum v7_err parse_block(struct v7 *v7, struct ast *a) {
  EXPECT(TOK_OPEN_CURLY);
  PARSE_ARG(body, TOK_CLOSE_CURLY);
  EXPECT(TOK_CLOSE_CURLY);
  return V7_OK;
}

static enum v7_err parse_body(struct v7 *v7, struct ast *a, enum v7_tok end) {
  ast_off_t start;
  while (v7->cur_tok != end) {
    if (ACCEPT(TOK_FUNCTION)) {
      if (v7->cur_tok != TOK_IDENTIFIER) {
        return V7_SYNTAX_ERROR;
      }
      start = ast_add_node(a, AST_VAR);
      ast_modify_skip(a, v7->last_var_node, start, AST_FUNC_FIRST_VAR_SKIP);
      /* zero out var node pointer */
      ast_modify_skip(a, start, start, AST_FUNC_FIRST_VAR_SKIP);
      v7->last_var_node = start;
      ast_add_inlined_node(a, AST_FUNC_DECL, v7->tok, v7->tok_len);

      PARSE_ARG_2(funcdecl, 1, 0);
      ast_set_skip(a, start, AST_END_SKIP);
    } else {
      PARSE(statement);
    }
  }
  return V7_OK;
}

static enum v7_err parse_use_strict(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok == TOK_STRING_LITERAL &&
      (strncmp(v7->tok, "\"use strict\"", v7->tok_len) == 0 ||
       strncmp(v7->tok, "'use strict'", v7->tok_len) == 0)) {
    next_tok(v7);
    ast_add_node(a, AST_USE_STRICT);
    return V7_OK;
  }
  return V7_SYNTAX_ERROR;
}

static enum v7_err parse_script(struct v7 *v7, struct ast *a) {
  ast_off_t start = ast_add_node(a, AST_SCRIPT);
  ast_off_t outer_last_var_node = v7->last_var_node;
  int saved_in_strict = v7->pstate.in_strict;
  v7->last_var_node = start;
  ast_modify_skip(a, start, 1, AST_FUNC_FIRST_VAR_SKIP);
  if (parse_use_strict(v7, a) == V7_OK) {
    v7->pstate.in_strict = 1;
  }
  PARSE_ARG(body, TOK_END_OF_INPUT);
  ast_set_skip(a, start, AST_END_SKIP);
  v7->pstate.in_strict = saved_in_strict;
  v7->last_var_node = outer_last_var_node;
  return V7_OK;
}

static unsigned long get_column(const char *code, const char *pos) {
  const char *p = pos;
  while (p > code && *p != '\n') {
    p--;
  }
  return p == code ? pos - p : pos - (p + 1);
}

V7_PRIVATE enum v7_err parse(struct v7 *v7, struct ast *a, const char *src,
                             int verbose) {
  enum v7_err err;
  v7->pstate.source_code = v7->pstate.pc = src;
  v7->pstate.file_name = "<stdin>";
  v7->pstate.line_no = 1;
  v7->pstate.in_function = 0;
  v7->pstate.in_loop = 0;
  v7->pstate.in_switch = 0;

  next_tok(v7);
  err = parse_script(v7, a);
  if (err == V7_OK && v7->cur_tok != TOK_END_OF_INPUT) {
    fprintf(stderr, "WARNING parse input not consumed\n");
  }
  if (verbose && err != V7_OK) {
    unsigned long col = get_column(v7->pstate.source_code, v7->tok);
    snprintf(v7->error_msg, sizeof(v7->error_msg),
             "parse error at at line %d col %lu: [%.*s]", v7->pstate.line_no,
             col, (int) (col + v7->tok_len), v7->tok - col);
  }
  return err;
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


#if defined(_WIN32) || defined(ARDUINO)
#define siglongjmp longjmp
#define sigsetjmp(buf, mask) setjmp(buf)
#endif

static enum ast_tag assign_op_map[] = {
    AST_REM, AST_MUL, AST_DIV,    AST_XOR,    AST_ADD,    AST_SUB,
    AST_OR,  AST_AND, AST_LSHIFT, AST_RSHIFT, AST_URSHIFT};

enum i_break { B_RUN, B_RETURN, B_BREAK, B_CONTINUE };

enum jmp_type { NO_JMP, THROW_JMP, BREAK_JMP, CONTINUE_JMP };

static val_t i_eval_stmts(struct v7 *, struct ast *, ast_off_t *, ast_off_t,
                          val_t, enum i_break *);
static val_t i_eval_expr(struct v7 *, struct ast *, ast_off_t *, val_t);
static val_t i_eval_call(struct v7 *, struct ast *, ast_off_t *, val_t, val_t,
                         int);
static val_t i_find_this(struct v7 *, struct ast *, ast_off_t, val_t);

V7_PRIVATE void throw_value(struct v7 *v7, val_t v) {
  v7->thrown_error = v;
  siglongjmp(v7->jmp_buf, THROW_JMP);
} /* LCOV_EXCL_LINE */

static val_t create_exception(struct v7 *v7, enum error_ctor ex,
                              const char *msg) {
  val_t e, args;
  if (v7->creating_exception) {
    fprintf(stderr, "Exception creation throws an exception %d: %s\n", ex, msg);
    return V7_UNDEFINED;
  }
  args = v7_create_dense_array(v7);
  v7_array_set(v7, args, 0, v7_create_string(v7, msg, strlen(msg), 1));
  v7->creating_exception++;
  e = create_object(v7, v7_get(v7, v7->error_objects[ex], "prototype", 9));
  v7_apply(v7, v7->error_objects[ex], e, args);
  v7->creating_exception--;
  return e;
}

V7_PRIVATE void throw_exception(struct v7 *v7, enum error_ctor ex,
                                const char *err_fmt, ...) {
  va_list ap;
  va_start(ap, err_fmt);
  vsnprintf(v7->error_msg, sizeof(v7->error_msg), err_fmt, ap);
  va_end(ap);
  throw_value(v7, create_exception(v7, ex, v7->error_msg));
} /* LCOV_EXCL_LINE */

V7_PRIVATE val_t i_value_of(struct v7 *v7, val_t v) {
  val_t f;
  if (!v7_is_object(v)) {
    return v;
  }

  if ((f = v7_get(v7, v, "valueOf", 7)) != V7_UNDEFINED) {
    /*
     * v7_apply will root all parameters since it can be called
     * from user code, hence it's not necessary to root `f`.
     * This assumes all callers of i_value_of will root their
     * temporary values.
     */
    v = v7_apply(v7, f, v, v7_create_undefined());
  }
  return v;
}

/* i_as_num expects callers to root temporary values passed as args */
V7_PRIVATE double i_as_num(struct v7 *v7, val_t v) {
  double res = 0.0;

  v = i_value_of(v7, v);
  if (v7_is_double(v)) {
    res = v7_to_double(v);
  } else if (v7_is_string(v)) {
    size_t n;
    char *e, *s = (char *) v7_to_string(v7, &v, &n);
    if (n != 0) {
      res = strtod(s, &e);
      if (e - n != s) {
        res = NAN;
      }
    }
  } else if (v7_is_boolean(v)) {
    res = (double) v7_to_boolean(v);
  } else if (v7_is_null(v)) {
    res = 0.0;
  } else {
    res = NAN;
  }
  return res;
}

static double i_num_unary_op(struct v7 *v7, enum ast_tag tag, double a) {
  switch (tag) {
    case AST_POSITIVE:
      return a;
    case AST_NEGATIVE:
      return -a;
    default:
      throw_exception(v7, INTERNAL_ERROR, "%s", __func__); /* LCOV_EXCL_LINE */
      return 0;                                            /* LCOV_EXCL_LINE */
  }
}

static double i_int_bin_op(struct v7 *v7, enum ast_tag tag, double a,
                           double b) {
  int32_t ia = isnan(a) || isinf(a) ? 0 : (int32_t)(int64_t) a;
  int32_t ib = isnan(b) || isinf(b) ? 0 : (int32_t)(int64_t) b;

  switch (tag) {
    case AST_LSHIFT:
      return (int32_t)((uint32_t) ia << ((uint32_t) ib & 31));
    case AST_RSHIFT:
      return ia >> ((uint32_t) ib & 31);
    case AST_URSHIFT:
      return (uint32_t) ia >> ((uint32_t) ib & 31);
    case AST_OR:
      return ia | ib;
    case AST_XOR:
      return ia ^ ib;
    case AST_AND:
      return ia & ib;
    default:
      throw_exception(v7, INTERNAL_ERROR, "%s", __func__); /* LCOV_EXCL_LINE */
      return 0;                                            /* LCOV_EXCL_LINE */
  }
}

#ifdef V7_WINDOWS
static int signbit(double x) {
  return x > 0;
}
#endif

static double i_num_bin_op(struct v7 *v7, enum ast_tag tag, double a,
                           double b) {
  switch (tag) {
    case AST_ADD: /* simple fixed width nodes with no payload */
      return a + b;
    case AST_SUB:
      return a - b;
    case AST_REM:
      if (b == 0 || isnan(b) || isnan(a) || isinf(b) || isinf(a)) {
        return NAN;
      }
      return (int) a % (int) b;
    case AST_MUL:
      return a * b;
    case AST_DIV:
      if (b == 0) {
        return (!signbit(a) == !signbit(b)) ? INFINITY : -INFINITY;
      }
      return a / b;
    case AST_LSHIFT:
    case AST_RSHIFT:
    case AST_URSHIFT:
    case AST_OR:
    case AST_XOR:
    case AST_AND:
      return i_int_bin_op(v7, tag, a, b);
    default:
      throw_exception(v7, INTERNAL_ERROR, "%s", __func__); /* LCOV_EXCL_LINE */
      return 0;                                            /* LCOV_EXCL_LINE */
  }
}

static int i_bool_bin_op(struct v7 *v7, enum ast_tag tag, double a, double b) {
  switch (tag) {
    case AST_EQ:
    case AST_EQ_EQ:
      return a == b;
    case AST_NE:
    case AST_NE_NE:
      return a != b;
    case AST_LT:
      return a < b;
    case AST_LE:
      return a <= b;
    case AST_GT:
      return a > b;
    case AST_GE:
      return a >= b;
    default:
      throw_exception(v7, INTERNAL_ERROR, "%s", __func__); /* LCOV_EXCL_LINE */
      return 0;                                            /* LCOV_EXCL_LINE */
  }
}

static val_t i_eval_expr(struct v7 *v7, struct ast *a, ast_off_t *pos,
                         val_t scope) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  const struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t end;
  val_t res = v7_create_undefined(), v1 = v7_create_undefined();
  val_t v2 = v7_create_undefined();
  double d1, d2, dv;
  int i;
  /*
   * TODO(mkm): put this temporary somewhere in the evaluation context
   * or use alloca.
   */
  char buf[512];
  char *name, *p;
  size_t name_len;
  struct gc_tmp_frame tf = new_tmp_frame(v7);

  tmp_stack_push(&tf, &res);
  tmp_stack_push(&tf, &v1);
  tmp_stack_push(&tf, &v2);

  switch (tag) {
    case AST_NEGATIVE:
    case AST_POSITIVE:
      res = v7_create_number(i_num_unary_op(
          v7, tag, i_as_num(v7, i_eval_expr(v7, a, pos, scope))));
      break;
    case AST_ADD:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      v1 = i_value_of(v7, v1);
      v2 = i_value_of(v7, v2);
      if (!(v7_is_undefined(v1) || v7_is_double(v1) || v7_is_boolean(v1)) ||
          !(v7_is_undefined(v2) || v7_is_double(v2) || v7_is_boolean(v2))) {
        v7_stringify_value(v7, v1, buf, sizeof(buf));
        v1 = v7_create_string(v7, buf, strlen(buf), 1);
        v7_stringify_value(v7, v2, buf, sizeof(buf));
        v2 = v7_create_string(v7, buf, strlen(buf), 1);
        res = s_concat(v7, v1, v2);
      } else {
        res = v7_create_number(
            i_num_bin_op(v7, tag, i_as_num(v7, v1), i_as_num(v7, v2)));
      }
      break;
    case AST_SUB:
    case AST_REM:
    case AST_MUL:
    case AST_DIV:
    case AST_LSHIFT:
    case AST_RSHIFT:
    case AST_URSHIFT:
    case AST_OR:
    case AST_XOR:
    case AST_AND:
      d1 = i_as_num(v7, i_eval_expr(v7, a, pos, scope));
      d2 = i_as_num(v7, i_eval_expr(v7, a, pos, scope));
      res = v7_create_number(i_num_bin_op(v7, tag, d1, d2));
      break;
    case AST_EQ_EQ:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      if (v7_is_string(v1) && v7_is_string(v2)) {
        res = v7_create_boolean(s_cmp(v7, v1, v2) == 0);
      } else if (v1 == v2 && v1 == V7_TAG_NAN) {
        res = v7_create_boolean(0);
      } else {
        res = v7_create_boolean(v1 == v2);
      }
      break;
    case AST_NE_NE:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      if (v7_is_string(v1) && v7_is_string(v2)) {
        res = v7_create_boolean(s_cmp(v7, v1, v2) != 0);
      } else if (v1 == v2 && v1 == V7_TAG_NAN) {
        res = v7_create_boolean(1);
      } else {
        res = v7_create_boolean(v1 != v2);
      }
      break;
    case AST_EQ:
    case AST_NE:
    case AST_LT:
    case AST_LE:
    case AST_GT:
    case AST_GE:
      v1 = i_value_of(v7, i_eval_expr(v7, a, pos, scope));
      v2 = i_value_of(v7, i_eval_expr(v7, a, pos, scope));
      if (tag == AST_EQ || tag == AST_NE) {
        if (((v7_is_object(v1) || v7_is_object(v2)) && v1 == v2)) {
          res = v7_create_boolean(tag == AST_EQ);
          break;
        } else if (v7_is_undefined(v1) || v7_is_null(v1)) {
          res = v7_create_boolean((tag != AST_EQ) ^
                                  (v7_is_undefined(v2) || v7_is_null(v2)));
          break;
        } else if (v7_is_undefined(v2) || v7_is_null(v2)) {
          res = v7_create_boolean((tag != AST_EQ) ^
                                  (v7_is_undefined(v1) || v7_is_null(v1)));
          break;
        }
      }
      if (v7_is_string(v1) && v7_is_string(v2)) {
        int cmp = s_cmp(v7, v1, v2);
        switch (tag) {
          case AST_EQ:
            res = v7_create_boolean(cmp == 0);
            break;
          case AST_NE:
            res = v7_create_boolean(cmp != 0);
            break;
          case AST_LT:
            res = v7_create_boolean(cmp < 0);
            break;
          case AST_LE:
            res = v7_create_boolean(cmp <= 0);
            break;
          case AST_GT:
            res = v7_create_boolean(cmp > 0);
            break;
          case AST_GE:
            res = v7_create_boolean(cmp >= 0);
            break;
          default:
            throw_exception(v7, INTERNAL_ERROR, "Unhandled op");
        }
      } else {
        res = v7_create_boolean(
            i_bool_bin_op(v7, tag, i_as_num(v7, v1), i_as_num(v7, v2)));
      }
      break;
    case AST_LOGICAL_OR:
      v1 = i_eval_expr(v7, a, pos, scope);
      if (v7_is_true(v7, v1)) {
        ast_skip_tree(a, pos);
        res = v1;
      } else {
        res = i_eval_expr(v7, a, pos, scope);
      }
      break;
    case AST_LOGICAL_AND:
      v1 = i_eval_expr(v7, a, pos, scope);
      if (!v7_is_true(v7, v1)) {
        ast_skip_tree(a, pos);
        res = v1;
      } else {
        res = i_eval_expr(v7, a, pos, scope);
      }
      break;
    case AST_LOGICAL_NOT:
      v1 = i_eval_expr(v7, a, pos, scope);
      res = v7_create_boolean(!(int64_t) v7_is_true(v7, v1));
      break;
    case AST_NOT:
      v1 = i_eval_expr(v7, a, pos, scope);
      d1 = i_as_num(v7, v1);
      if (isnan(d1) || isinf(d1)) {
        res = v7_create_number(-1);
      } else {
        res = v7_create_number(~(int64_t) d1);
      }
      break;
    case AST_ASSIGN:
    case AST_REM_ASSIGN:
    case AST_MUL_ASSIGN:
    case AST_DIV_ASSIGN:
    case AST_XOR_ASSIGN:
    case AST_PLUS_ASSIGN:
    case AST_MINUS_ASSIGN:
    case AST_OR_ASSIGN:
    case AST_AND_ASSIGN:
    case AST_LSHIFT_ASSIGN:
    case AST_RSHIFT_ASSIGN:
    case AST_URSHIFT_ASSIGN:
    case AST_PREINC:
    case AST_PREDEC:
    case AST_POSTINC:
    case AST_POSTDEC: {
      struct v7_property *prop;
      enum ast_tag op = tag;
      val_t lval = v7_create_undefined(), root = v7->global_object;
      tmp_stack_push(&tf, &lval);
      tmp_stack_push(&tf, &root);
      switch ((tag = ast_fetch_tag(a, pos))) {
        case AST_IDENT:
          lval = scope;
          name = ast_get_inlined_data(a, *pos, &name_len);
          ast_move_to_children(a, pos);
          break;
        case AST_MEMBER:
          name = ast_get_inlined_data(a, *pos, &name_len);
          ast_move_to_children(a, pos);
          lval = root = i_eval_expr(v7, a, pos, scope);
          break;
        case AST_INDEX:
          lval = root = i_eval_expr(v7, a, pos, scope);
          v1 = i_eval_expr(v7, a, pos, scope);
          name_len = v7_stringify_value(v7, v1, buf, sizeof(buf));
          name = buf;
          break;
        default:
          throw_exception(v7, REFERENCE_ERROR,
                          "Invalid left-hand side in assignment");
          /* unreacheable */
          return v7_create_undefined(); /* LCOV_EXCL_LINE */
      }

      /*
       * TODO(mkm): this will incorrectly mutate an existing property in
       * Object.prototype instead of creating a new variable in `global`.
       * `get_property` should also return a pointer to the object where
       * the property is found.
       */
      v1 = v7_create_undefined();
      prop = v7_get_property(v7, lval, name, name_len);
      if (prop != NULL) {
        v1 = prop->value;
      }

      switch (op) {
        case AST_PREINC:
          v1 = res = v7_create_number(i_as_num(v7, v1) + 1.0);
          break;
        case AST_PREDEC:
          v1 = res = v7_create_number(i_as_num(v7, v1) - 1.0);
          break;
        case AST_POSTINC:
          res = i_value_of(v7, v1);
          v1 = v7_create_number(i_as_num(v7, v1) + 1.0);
          break;
        case AST_POSTDEC:
          res = i_value_of(v7, v1);
          v1 = v7_create_number(i_as_num(v7, v1) - 1.0);
          break;
        case AST_ASSIGN:
          v1 = res = i_eval_expr(v7, a, pos, scope);
          break;
        case AST_PLUS_ASSIGN:
          res = i_eval_expr(v7, a, pos, scope);
          v1 = i_value_of(v7, v1);
          res = i_value_of(v7, res);
          if (!(v7_is_undefined(v1) || v7_is_double(v1) || v7_is_boolean(v1)) ||
              !(v7_is_undefined(res) || v7_is_double(res) ||
                v7_is_boolean(res))) {
            v7_stringify_value(v7, v1, buf, sizeof(buf));
            v1 = v7_create_string(v7, buf, strlen(buf), 1);
            v7_stringify_value(v7, res, buf, sizeof(buf));
            res = v7_create_string(v7, buf, strlen(buf), 1);
            v1 = res = s_concat(v7, v1, res);
            break;
          }
          res = v1 = v7_create_number(
              i_num_bin_op(v7, AST_ADD, i_as_num(v7, v1), i_as_num(v7, res)));
          break;
        default:
          op = assign_op_map[op - AST_ASSIGN - 1];
          res = i_eval_expr(v7, a, pos, scope);
          d1 = i_as_num(v7, v1);
          d2 = i_as_num(v7, res);
          res = v1 = v7_create_number(i_num_bin_op(v7, op, d1, d2));
      }

      if (v7_is_object(lval) &&
          v7_to_object(lval)->attributes & V7_OBJ_DENSE_ARRAY) {
        int ok;
        unsigned long i = cstr_to_ulong(name, name_len, &ok);
        if (ok) {
          v7_array_set(v7, lval, i, v1);
          break;
        }
      }

      if (prop != NULL && tag == AST_IDENT) {
        /* variables are modified where they are found in the scope chain */
        prop->value = v1;
      } else if (prop != NULL && prop->attributes & V7_PROPERTY_READ_ONLY) {
        /* nop */
      } else if (prop != NULL && prop->attributes & V7_PROPERTY_SETTER) {
        v7_invoke_setter(v7, prop, root, v1);
      } else {
        v7_set_property(v7, root, name, name_len, 0, v1);
      }
      break;
    }
    case AST_INDEX:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      v7_stringify_value(v7, v2, buf, sizeof(buf));
      res = v7_get(v7, v1, buf, -1);
      break;
    case AST_MEMBER:
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      v1 = i_eval_expr(v7, a, pos, scope);
      res = v7_get(v7, v1, name, name_len);
      break;
    case AST_SEQ:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        res = i_eval_expr(v7, a, pos, scope);
      }
      break;
    case AST_ARRAY:
      res = v7_create_array(v7);
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      for (i = 0; *pos < end; i++) {
        ast_off_t lookahead = *pos;
        tag = ast_fetch_tag(a, &lookahead);
        v1 = i_eval_expr(v7, a, pos, scope);
        if (tag != AST_NOP) {
          v7_array_set(v7, res, i, v1);
        }
      }
      break;
    case AST_OBJECT:
      res = v7_create_object(v7);
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        tag = ast_fetch_tag(a, pos);
        switch (tag) {
          case AST_PROP:
            name = ast_get_inlined_data(a, *pos, &name_len);
            ast_move_to_children(a, pos);
            v1 = i_eval_expr(v7, a, pos, scope);
            if (v7->strict_mode &&
                v7_get_own_property(v7, res, name, name_len) != NULL) {
              /* Ideally this should be thrown at parse time */
              throw_exception(v7, SYNTAX_ERROR,
                              "duplicate data property in object literal "
                              "not allowed in strict mode");
            }
            v7_set_property(v7, res, name, name_len, 0, v1);
            break;
          case AST_GETTER:
          case AST_SETTER: {
            ast_off_t func = *pos;
            unsigned int attr =
                tag == AST_GETTER ? V7_PROPERTY_GETTER : V7_PROPERTY_SETTER;
            unsigned int other =
                tag == AST_GETTER ? V7_PROPERTY_SETTER : V7_PROPERTY_GETTER;
            struct v7_property *p;
            V7_CHECK(v7, ast_fetch_tag(a, &func) == AST_FUNC);
            ast_move_to_children(a, &func);
            V7_CHECK(v7, ast_fetch_tag(a, &func) == AST_IDENT);
            name = ast_get_inlined_data(a, func, &name_len);
            v1 = i_eval_expr(v7, a, pos, scope);
            if ((p = v7_get_property(v7, res, name, name_len)) &&
                p->attributes & other) {
              val_t arr = v7_create_dense_array(v7);
              tmp_stack_push(&tf, &arr);
              v7_array_set(v7, arr, tag == AST_GETTER ? 1 : 0, p->value);
              v7_array_set(v7, arr, tag == AST_SETTER ? 1 : 0, v1);
              p->value = arr;
              p->attributes |= attr;
            } else {
              v7_set_property(v7, res, name, name_len, attr, v1);
            }
            break;
          }
          default:
            throw_exception(v7, INTERNAL_ERROR,
                            "Expecting AST_(PROP|GETTER|SETTER) got %d", tag);
        }
      }
      break;
    case AST_TRUE:
      res = v7_create_boolean(1);
      break;
    case AST_FALSE:
      res = v7_create_boolean(0);
      break;
    case AST_NULL:
      res = v7_create_null();
      break;
    case AST_USE_STRICT:
    case AST_NOP:
    case AST_UNDEFINED:
      res = v7_create_undefined();
      break;
    case AST_NUM:
      ast_get_num(a, *pos, &dv);
      ast_move_to_children(a, pos);
      res = v7_create_number(dv);
      break;
    case AST_STRING:
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      res = v7_create_string(v7, name, name_len, 1);
      break;
    case AST_REGEX:
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      for (p = name + name_len - 1; *p != '/';) p--;
      res = v7_create_regexp(v7, name + 1, p - (name + 1), p + 1,
                             (name + name_len) - p - 1);
      break;
    case AST_IDENT: {
      struct v7_property *p;
      name = ast_get_inlined_data(a, *pos, &name_len);
      ast_move_to_children(a, pos);
      if ((p = v7_get_property(v7, scope, name, name_len)) == NULL) {
        throw_exception(v7, REFERENCE_ERROR, "[%.*s] is not defined",
                        (int) name_len, name);
      }
      res = v7_property_value(v7, scope, p);
      break;
    }
    case AST_FUNC: {
      ast_off_t fbody;
      struct v7_function *funcp;
      res = v7_create_function(v7);
      funcp = v7_to_function(res);
      tmp_stack_push(&tf, &res);
      funcp->scope = v7_to_object(scope);
      funcp->ast = a;
      funcp->ast_off = *pos - 1;
      ast_move_to_children(a, pos);
      tag = ast_fetch_tag(a, pos);
      if (tag == AST_IDENT) {
        name = ast_get_inlined_data(a, *pos, &name_len);
        v7_set_property(v7, scope, name, name_len, 0, res);
      }
      *pos = ast_get_skip(a, funcp->ast_off + 1, AST_END_SKIP);
      fbody = ast_get_skip(a, funcp->ast_off + 1, AST_FUNC_BODY_SKIP);
      if (fbody < *pos && (tag = ast_fetch_tag(a, &fbody)) == AST_USE_STRICT) {
        funcp->attributes |= V7_FUNCTION_STRICT;
      }
      break;
    }
    case AST_CALL: {
      ast_off_t pp = *pos;
      ast_move_to_children(a, &pp);
      res = i_eval_call(v7, a, pos, scope, i_find_this(v7, a, pp, scope), 0);
      break;
    }
    case AST_NEW:
      v1 = v7_create_object(v7);
      res = i_eval_call(v7, a, pos, scope, v1, 1);
      if (v7_is_undefined(res) || v7_is_null(res)) {
        res = v1;
      }
      break;
    case AST_COND:
      if (v7_is_true(v7, i_eval_expr(v7, a, pos, scope))) {
        res = i_eval_expr(v7, a, pos, scope);
        ast_skip_tree(a, pos); /* TODO(mkm): change AST to include skips ? */
      } else {
        ast_skip_tree(a, pos);
        res = i_eval_expr(v7, a, pos, scope);
      }
      break;
    case AST_IN:
      v1 = i_eval_expr(v7, a, pos, scope);
      v7_stringify_value(v7, v1, buf, sizeof(buf));
      v2 = i_eval_expr(v7, a, pos, scope);
      res = v7_create_boolean(v7_get_property(v7, v2, buf, -1) != NULL);
      break;
    case AST_VAR:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      while (*pos < end) {
        struct v7_property *prop;
        tag = ast_fetch_tag(a, pos);
        /* func declarations are already set during hoisting */
        if (tag == AST_FUNC_DECL) {
          ast_move_to_children(a, pos);
          ast_skip_tree(a, pos);
          continue;
        }

        V7_CHECK(v7, tag == AST_VAR_DECL);
        name = ast_get_inlined_data(a, *pos, &name_len);
        ast_move_to_children(a, pos);
        res = i_eval_expr(v7, a, pos, scope);
        /*
         * Var decls are hoisted when the function frame is created. Vars
         * declared inside a `with` or `catch` block belong to the function
         * lexical scope, and although those clauses create an inner frame
         * no new variables should be created in it. A var decl thus
         * behaves as a normal assignment at runtime.
         */
        if ((prop = v7_get_property(v7, scope, name, name_len)) != NULL) {
          prop->value = res;
        } else {
          v7_set_property(v7, v7->global_object, name, name_len, 0, res);
        }
      }
      break;
    case AST_THIS:
      res = v7->this_object;
      break;
    case AST_TYPEOF: {
      ast_off_t peek = *pos;
      if ((tag = ast_fetch_tag(a, &peek)) == AST_IDENT) {
        name = ast_get_inlined_data(a, peek, &name_len);
        if (v7_get_property(v7, scope, name, name_len) == NULL) {
          ast_move_to_children(a, &peek);
          *pos = peek;
          /* TODO(mkm): use interned strings */
          res = v7_create_string(v7, "undefined", 9, 1);
          break;
        }
      }
      /* for some reason lcov doesn't mark the following lines as executing */
      res = i_eval_expr(v7, a, pos, scope); /* LCOV_EXCL_LINE */
      switch (val_type(v7, res)) {          /* LCOV_EXCL_LINE */
        case V7_TYPE_NUMBER:
          res = v7_create_string(v7, "number", 6, 1);
          break;
        case V7_TYPE_STRING:
          res = v7_create_string(v7, "string", 6, 1);
          break;
        case V7_TYPE_BOOLEAN:
          res = v7_create_string(v7, "boolean", 7, 1);
          break;
        case V7_TYPE_FUNCTION_OBJECT:
        case V7_TYPE_CFUNCTION_OBJECT:
        case V7_TYPE_CFUNCTION:
          res = v7_create_string(v7, "function", 8, 1);
          break;
        default:
          res = v7_create_string(v7, "object", 6, 1);
          break;
      }
      break;
    }
    case AST_DELETE: {
      struct v7_property *prop;
      val_t lval = v7_create_null(), root = v7->global_object;
      ast_off_t start = *pos;
      tmp_stack_push(&tf, &lval);
      tmp_stack_push(&tf, &root);
      switch ((tag = ast_fetch_tag(a, pos))) {
        case AST_IDENT:
          name = ast_get_inlined_data(a, *pos, &name_len);
          ast_move_to_children(a, pos);
          if (v7_get_property(v7, scope, name, name_len) ==
              v7_get_property(v7, root, name, name_len)) {
            lval = root;
          }
          if (v7->strict_mode) {
            throw_exception(v7, SYNTAX_ERROR, "Delete in strict");
          }
          break;
        case AST_MEMBER:
          name = ast_get_inlined_data(a, *pos, &name_len);
          ast_move_to_children(a, pos);
          lval = root = i_eval_expr(v7, a, pos, scope);
          break;
        case AST_INDEX:
          lval = root = i_eval_expr(v7, a, pos, scope);
          res = i_eval_expr(v7, a, pos, scope);
          name_len = v7_stringify_value(v7, res, buf, sizeof(buf));
          name = buf;
          break;
        default:
          *pos = start;
          i_eval_expr(v7, a, pos, scope);
          return v7_create_boolean(1);
      }

      if (v7_is_object(lval) &&
          v7_to_object(lval)->attributes & V7_OBJ_DENSE_ARRAY) {
        int ok;
        unsigned long i = cstr_to_ulong(name, name_len, &ok);
        if (ok) {
          int has;
          v7_array_get2(v7, lval, (unsigned long) i, &has);
          if (has) {
            v7_array_set(v7, lval, (unsigned long) i, V7_TAG_NOVALUE);
          }
          res = v7_create_boolean(1);
        }
      }

      prop = v7_get_property(v7, lval, name, name_len);
      if (prop != NULL) {
        if (prop->attributes & V7_PROPERTY_DONT_DELETE) {
          res = v7_create_boolean(0);
          break;
        }
        v7_del_property(v7, lval, name, name_len);
      }
      res = v7_create_boolean(1);
      break;
    }
    case AST_INSTANCEOF:
      v1 = i_eval_expr(v7, a, pos, scope);
      v2 = i_eval_expr(v7, a, pos, scope);
      if (!v7_is_function(v2) && !v7_is_cfunction(i_value_of(v7, v2))) {
        throw_exception(v7, TYPE_ERROR,
                        "Expecting a function in instanceof check");
      }
      res = v7_create_boolean(
          is_prototype_of(v7, v1, v7_get(v7, v2, "prototype", 9)));
      break;
    case AST_VOID:
      i_eval_expr(v7, a, pos, scope);
      res = v7_create_undefined();
      break;
    default:
      throw_exception(v7, INTERNAL_ERROR, "%s: %s", __func__,
                      def->name); /* LCOV_EXCL_LINE */
      /* unreacheable */
      break;
  }

  tmp_frame_cleanup(&tf);
  return res;
}

static val_t i_find_this(struct v7 *v7, struct ast *a, ast_off_t pos,
                         val_t scope) {
  enum ast_tag tag = ast_fetch_tag(a, &pos);
  switch (tag) {
    case AST_MEMBER:
      ast_move_to_children(a, &pos);
      return i_eval_expr(v7, a, &pos, scope);
    case AST_INDEX:
      return i_eval_expr(v7, a, &pos, scope);
    default:
      return V7_UNDEFINED;
  }
}

static void i_populate_local_vars(struct v7 *v7, struct ast *a, ast_off_t start,
                                  ast_off_t fvar, val_t frame) {
  enum ast_tag tag;
  ast_off_t next, fvar_end;
  char *name;
  size_t name_len;
  val_t val = v7_create_undefined();
  struct gc_tmp_frame tf = new_tmp_frame(v7);

  if (fvar == start) {
    tmp_frame_cleanup(&tf);
    return;
  }

  tmp_stack_push(&tf, &val);

  do {
    tag = ast_fetch_tag(a, &fvar);
    V7_CHECK(v7, tag == AST_VAR);
    next = ast_get_skip(a, fvar, AST_VAR_NEXT_SKIP);
    if (next == fvar) {
      next = 0;
    }
    V7_CHECK(v7, next < 1024 * 128);

    fvar_end = ast_get_skip(a, fvar, AST_END_SKIP);
    ast_move_to_children(a, &fvar);
    while (fvar < fvar_end) {
      val = v7_create_undefined();
      tag = ast_fetch_tag(a, &fvar);
      V7_CHECK(v7, tag == AST_VAR_DECL || tag == AST_FUNC_DECL);
      name = ast_get_inlined_data(a, fvar, &name_len);
      ast_move_to_children(a, &fvar);
      if (tag == AST_VAR_DECL) {
        ast_skip_tree(a, &fvar);
      } else {
        val = i_eval_expr(v7, a, &fvar, frame);
      }
      v7_set_property(v7, frame, name, name_len, 0, val);
    }
    if (next > 0) {
      fvar = next - 1; /* TODO(mkm): cleanup */
    }
  } while (next != 0);

  tmp_frame_cleanup(&tf);
}

V7_PRIVATE val_t i_prepare_call(struct v7 *v7, struct v7_function *func,
                                ast_off_t *pos, ast_off_t *body,
                                ast_off_t *end) {
  val_t frame;
  enum ast_tag tag;
  ast_off_t fstart, fvar;

  *pos = func->ast_off;
  fstart = *pos;
  tag = ast_fetch_tag(func->ast, pos);
  V7_CHECK(v7, tag == AST_FUNC);
  *end = ast_get_skip(func->ast, *pos, AST_END_SKIP);
  *body = ast_get_skip(func->ast, *pos, AST_FUNC_BODY_SKIP);
  fvar = ast_get_skip(func->ast, *pos, AST_FUNC_FIRST_VAR_SKIP) - 1;
  ast_move_to_children(func->ast, pos);
  ast_skip_tree(func->ast, pos);

  frame = v7_create_object(v7);
  v7_to_object(frame)->prototype = func->scope;

  i_populate_local_vars(v7, func->ast, fstart, fvar, frame);
  return frame;
}

V7_PRIVATE val_t i_invoke_function(struct v7 *v7, struct v7_function *func,
                                   val_t frame, ast_off_t body, ast_off_t end) {
  int saved_strict_mode = v7->strict_mode;
  enum i_break brk = B_RUN;
  val_t res = v7_create_undefined(), saved_call_stack = v7->call_stack;
  struct gc_tmp_frame tf = new_tmp_frame(v7);

  tmp_stack_push(&tf, &res);
  if (func->attributes & V7_FUNCTION_STRICT) {
    v7->strict_mode = 1;
  }
  v7->call_stack = frame; /* ensure GC knows about this call frame */
  res = i_eval_stmts(v7, func->ast, &body, end, frame, &brk);
  if (brk != B_RETURN) {
    res = v7_create_undefined();
  }
  v7->strict_mode = saved_strict_mode;
  v7->call_stack = saved_call_stack;

  tmp_frame_cleanup(&tf);
  return res;
}

static val_t i_eval_call(struct v7 *v7, struct ast *a, ast_off_t *pos,
                         val_t scope, val_t this_object, int is_constructor) {
  ast_off_t end, fpos, fend, fbody;
  val_t frame = v7_create_undefined(), res = v7_create_undefined();
  val_t v1 = v7_create_undefined(), args = v7_create_undefined();
  val_t cfunc = v7_create_undefined(), old_this = v7->this_object;
  struct v7_function *func;
  enum ast_tag tag;
  char *name;
  size_t name_len;
  int i;

  struct gc_tmp_frame tf = new_tmp_frame(v7);
  tmp_stack_push(&tf, &frame);
  tmp_stack_push(&tf, &res);
  tmp_stack_push(&tf, &v1);
  tmp_stack_push(&tf, &args);
  tmp_stack_push(&tf, &old_this);

  end = ast_get_skip(a, *pos, AST_END_SKIP);
  ast_move_to_children(a, pos);
  cfunc = v1 = i_eval_expr(v7, a, pos, scope);
  if (!v7_is_cfunction(v1) && !v7_is_function(v1)) {
    /* extract the hidden property from a cfunction_object */
    struct v7_property *p;
    p = v7_get_own_property2(v7, v1, "", 0, V7_PROPERTY_HIDDEN);
    if (p != NULL) {
      cfunc = p->value;
    }
  }

  if (is_constructor) {
    if (!v7_is_cfunction(v1)) {
      val_t fun_proto = v7_get(v7, v1, "prototype", 9);
      tmp_stack_push(&tf, &fun_proto);
      if (!v7_is_object(fun_proto)) {
        /* TODO(mkm): box primitive value */
        throw_exception(v7, TYPE_ERROR,
                        "Cannot set a primitive value as object prototype");
      }
      v7_to_object(this_object)->prototype = v7_to_object(fun_proto);
    }
  } else if (v7_is_undefined(this_object) && v7_is_function(v1) &&
             !(v7_to_function(v1)->attributes & V7_FUNCTION_STRICT)) {
    /*
     * null and undefined are replaced with `global` in non-strict mode,
     * as per ECMA-262 6th, 19.2.3.3.
     */
    this_object = v7->global_object;
  }

  if (v7_is_cfunction(cfunc)) {
    args = v7_create_dense_array(v7);
    for (i = 0; *pos < end; i++) {
      res = i_eval_expr(v7, a, pos, scope);
      v7_array_set(v7, args, i, res);
    }
    res = v7_to_cfunction(cfunc)(v7, this_object, args);
    goto cleanup;
  }
  if (!v7_is_function(v1)) {
    throw_exception(v7, TYPE_ERROR, "%s",
                    "value is not a function"); /* LCOV_EXCL_LINE */
  }

  func = v7_to_function(v1);
  frame = i_prepare_call(v7, func, &fpos, &fbody, &fend);

  /*
   * TODO(mkm): don't create args array if the parser didn't see
   * any `arguments` or `eval` identifier being referenced in the function.
   */
  args = v7_create_dense_array(v7);

  /* scan actual and formal arguments and updates the value in the frame */
  for (i = 0; fpos < fbody; i++) {
    tag = ast_fetch_tag(func->ast, &fpos);
    V7_CHECK(v7, tag == AST_IDENT);
    name = ast_get_inlined_data(func->ast, fpos, &name_len);
    ast_move_to_children(func->ast, &fpos);

    if (*pos < end) {
      res = i_eval_expr(v7, a, pos, scope);
      if (!v7_is_undefined(args)) {
        v7_array_set(v7, args, i, res);
      }
    } else {
      res = v7_create_undefined();
    }

    v7_set_property(v7, frame, name, name_len, 0, res);
  }

  /* evaluate trailing actual arguments for side effects */
  for (; *pos < end; i++) {
    res = i_eval_expr(v7, a, pos, scope);
    if (!v7_is_undefined(args)) {
      v7_array_set(v7, args, i, res);
    }
  }

  if (!v7_is_undefined(args)) {
#ifndef V7_DISABLE_PREDEFINED_STRINGS
    v7_set_v(v7, frame, v7->predefined_strings[PREDEFINED_STR_ARGUMENTS], args);
#else
    v7_set(v7, frame, "arguments", 9, args);
#endif
  }

  v7->this_object = this_object;
  res = i_invoke_function(v7, func, frame, fbody, fend);
  v7->this_object = old_this;

cleanup:
  tmp_frame_cleanup(&tf);
  return res;
}

static val_t i_eval_stmt(struct v7 *, struct ast *, ast_off_t *, val_t,
                         enum i_break *);

static val_t i_eval_stmts(struct v7 *v7, struct ast *a, ast_off_t *pos,
                          ast_off_t end, val_t scope, enum i_break *brk) {
  val_t res = v7_create_undefined();
  while (*pos < end && !*brk) {
    res = i_eval_stmt(v7, a, pos, scope, brk);
  }
  return res;
}

static val_t i_eval_stmt(struct v7 *v7, struct ast *a, ast_off_t *pos,
                         val_t scope, enum i_break *brk) {
  ast_off_t maybe_strict, start = *pos;
  enum ast_tag tag = ast_fetch_tag(a, pos);
  val_t res = v7_create_undefined();
  ast_off_t end, end_true, cond, iter_end, loop, iter, finally, acatch, fvar;
  int saved_strict_mode = v7->strict_mode;
  struct gc_tmp_frame tf = new_tmp_frame(v7);
  tmp_stack_push(&tf, &res);

  switch (tag) {
    case AST_SCRIPT:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      fvar = ast_get_skip(a, *pos, AST_FUNC_FIRST_VAR_SKIP) - 1;
      ast_move_to_children(a, pos);
      maybe_strict = *pos;
      if (*pos < end &&
          (tag = ast_fetch_tag(a, &maybe_strict)) == AST_USE_STRICT) {
        v7->strict_mode = 1;
        *pos = maybe_strict;
      }
      i_populate_local_vars(v7, a, start, fvar, scope);
      res = i_eval_stmts(v7, a, pos, end, scope, brk);
      v7->strict_mode = saved_strict_mode;
      break;
    case AST_IF:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      end_true = ast_get_skip(a, *pos, AST_END_IF_TRUE_SKIP);
      ast_move_to_children(a, pos);
      if (v7_is_true(v7, i_eval_expr(v7, a, pos, scope))) {
        res = i_eval_stmts(v7, a, pos, end_true, scope, brk);
        if (*brk != B_RUN) {
          break;
        }
      } else {
        res = i_eval_stmts(v7, a, &end_true, end, scope, brk);
        if (*brk != B_RUN) {
          break;
        }
      }
      *pos = end;
      break;
    case AST_WHILE:
      v7->lab_cont = 0;
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      cond = *pos;
      for (;;) {
        if (v7_is_true(v7, i_eval_expr(v7, a, pos, scope))) {
          res = i_eval_stmts(v7, a, pos, end, scope, brk);
          switch (*brk) {
            case B_RUN:
              break;
            case B_CONTINUE:
              *brk = B_RUN;
              break;
            case B_BREAK:
              *brk = B_RUN; /* fall through */
            case B_RETURN:
              *pos = end;
              goto cleanup;
          }
        } else {
          *pos = end;
          break;
        }
        *pos = cond;
      }
      break;
    case AST_DOWHILE:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      iter_end = ast_get_skip(a, *pos, AST_DO_WHILE_COND_SKIP);
      ast_move_to_children(a, pos);
      /* skip to condition if coming from a labeled continue */
      if (v7->lab_cont) {
        *pos = iter_end;
        v7->lab_cont = 0;
      }
      loop = *pos;
      for (;;) {
        res = i_eval_stmts(v7, a, pos, iter_end, scope, brk);
        switch (*brk) {
          case B_RUN:
            break;
          case B_CONTINUE:
            *pos = iter_end;
            *brk = B_RUN;
            break;
          case B_BREAK:
            *brk = B_RUN; /* fall through */
          case B_RETURN:
            *pos = end;
            goto cleanup;
        }
        if (!v7_is_true(v7, i_eval_expr(v7, a, pos, scope))) {
          break;
        }
        *pos = loop;
      }
      break;
    case AST_FOR:
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      iter_end = ast_get_skip(a, *pos, AST_FOR_BODY_SKIP);
      ast_move_to_children(a, pos);
      /* initializer */
      if (!v7->lab_cont) {
        i_eval_expr(v7, a, pos, scope);
      } else {
        ast_skip_tree(a, pos);
        iter = *pos;
        ast_skip_tree(a, &iter);
        i_eval_expr(v7, a, &iter, scope);
        v7->lab_cont = 0;
      }
      for (;;) {
        loop = *pos;
        if (!v7_is_true(v7, i_eval_expr(v7, a, &loop, scope))) {
          *pos = end;
          goto cleanup;
        }
        iter = loop;
        loop = iter_end;
        res = i_eval_stmts(v7, a, &loop, end, scope, brk);
        switch (*brk) {
          case B_RUN:
            break;
          case B_CONTINUE:
            *brk = B_RUN;
            break;
          case B_BREAK:
            *brk = B_RUN; /* fall through */
          case B_RETURN:
            *pos = end;
            goto cleanup;
        }
        i_eval_expr(v7, a, &iter, scope);
      }
    case AST_FOR_IN: {
      char *name;
      size_t name_len;
      val_t obj, key;
      ast_off_t loop;
      struct v7_property *p, *var;
      tmp_stack_push(&tf, &obj);
      tmp_stack_push(&tf, &key);

      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      tag = ast_fetch_tag(a, pos);
      /* TODO(mkm) accept any l-value */
      if (tag == AST_VAR) {
        ast_move_to_children(a, pos);
        tag = ast_fetch_tag(a, pos);
        V7_CHECK(v7, tag == AST_VAR_DECL);
        name = ast_get_inlined_data(a, *pos, &name_len);
        ast_move_to_children(a, pos);
        ast_skip_tree(a, pos);
      } else {
        V7_CHECK(v7, tag == AST_IDENT);
        name = ast_get_inlined_data(a, *pos, &name_len);
        ast_move_to_children(a, pos);
      }

      obj = i_eval_expr(v7, a, pos, scope);
      if (!v7_is_object(obj)) {
        *pos = end;
        goto cleanup;
      }
      ast_skip_tree(a, pos);
      loop = *pos;

      /* first iterate on dense array elements if any */
      /* TODO(mkm): make it DRY */
      if (v7_to_object(obj)->attributes & V7_OBJ_DENSE_ARRAY) {
        struct v7_property *p =
            v7_get_own_property2(v7, obj, "", 0, V7_PROPERTY_HIDDEN);
        struct mbuf *abuf;
        if (p != NULL) {
          abuf = (struct mbuf *) v7_to_foreign(p->value);
          if (abuf != NULL) {
            unsigned long i, len = v7_array_length(v7, obj);
            for (i = 0; i < len; i++, *pos = loop) {
              key = ulong_to_str(v7, i);
              if ((var = v7_get_property(v7, scope, name, name_len)) != NULL) {
                var->value = key;
              } else {
                v7_set_property(v7, v7->global_object, name, name_len, 0, key);
              }
              res = i_eval_stmts(v7, a, pos, end, scope,
                                 brk); /* LCOV_EXCL_LINE */
              switch (*brk) {          /* LCOV_EXCL_LINE */
                case B_RUN:
                  break;
                case B_CONTINUE:
                  *brk = B_RUN;
                  break;
                case B_BREAK:
                  *brk = B_RUN; /* fall through */
                case B_RETURN:
                  *pos = end;
                  goto cleanup;
              }
            }
          }
        }
      }
      loop = *pos;

      for (p = v7_to_object(obj)->properties; p; p = p->next, *pos = loop) {
        if (p->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)) {
          continue;
        }
        key = p->name;
        if ((var = v7_get_property(v7, scope, name, name_len)) != NULL) {
          var->value = key;
        } else {
          v7_set_property(v7, v7->global_object, name, name_len, 0, key);
        }

        /* for some reason lcov doesn't mark the following lines executing */
        res = i_eval_stmts(v7, a, pos, end, scope, brk); /* LCOV_EXCL_LINE */
        switch (*brk) {                                  /* LCOV_EXCL_LINE */
          case B_RUN:
            break;
          case B_CONTINUE:
            *brk = B_RUN;
            break;
          case B_BREAK:
            *brk = B_RUN; /* fall through */
          case B_RETURN:
            *pos = end;
            goto cleanup;
        }
      }
      *pos = end;
      break;
    }
    case AST_DEFAULT:
      /* handle fallthroughs */
      ast_move_to_children(a, pos);
      break;
    case AST_CASE:
      /* handle fallthroughs */
      ast_move_to_children(a, pos);
      ast_skip_tree(a, pos);
      break;
    case AST_SWITCH: {
      int found = 0;
      val_t test = v7_create_undefined(), val = v7_create_undefined();
      ast_off_t case_end, default_pos = 0;
      enum ast_tag case_tag;
      tmp_stack_push(&tf, &test);
      tmp_stack_push(&tf, &val);

      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      test = i_eval_expr(v7, a, pos, scope);
      while (*pos < end) {
        switch (case_tag = ast_fetch_tag(a, pos)) {
          default:
            throw_exception(v7, INTERNAL_ERROR, /* LCOV_EXCL_LINE */
                            "invalid ast node %d", case_tag);
          case AST_DEFAULT:
            default_pos = *pos;
            *pos = ast_get_skip(a, *pos, AST_END_SKIP);
            break;
          case AST_CASE:
            case_end = ast_get_skip(a, *pos, AST_END_SKIP);
            ast_move_to_children(a, pos);
            val = i_eval_expr(v7, a, pos, scope);
            /* TODO(mkm): factor out equality check from eval_expr */
            if (test != val || val == V7_TAG_NAN) {
              *pos = case_end;
              break;
            }
            res = i_eval_stmts(v7, a, pos, end, scope, brk);
            if (*brk == B_BREAK) {
              *brk = B_RUN;
            }
            *pos = end;
            found = 1;
            break;
        }
      }

      if (!found && default_pos != 0) {
        ast_move_to_children(a, &default_pos);
        res = i_eval_stmts(v7, a, &default_pos, end, scope, brk);
        if (*brk == B_BREAK) {
          *brk = B_RUN;
        }
      }
      break;
    }
    case AST_LABEL: {
      jmp_buf old_jmp;
      char *name;
      size_t name_len;
      ast_off_t saved_pos;
      size_t saved_tmp_stack_pos = v7->tmp_stack.len;
      volatile enum jmp_type j;
      memcpy(old_jmp, v7->jmp_buf, sizeof(old_jmp));
      name = ast_get_inlined_data(a, *pos, &name_len);

      ast_move_to_children(a, pos);
      saved_pos = *pos;
    /*
     * Percolate up all exceptions and labeled breaks
     * not matching the current label.
     */
    cont:
      if ((j = (enum jmp_type) sigsetjmp(v7->jmp_buf, 0)) == 0) {
        res = i_eval_stmt(v7, a, pos, scope, brk);
      } else if ((j == BREAK_JMP || j == CONTINUE_JMP) &&
                 name_len == v7->label_len &&
                 memcmp(name, v7->label, name_len) == 0) {
        v7->tmp_stack.len = saved_tmp_stack_pos;
        *pos = saved_pos;
        if (j == CONTINUE_JMP) {
          v7->lab_cont = 1;
          goto cont;
        }
        ast_skip_tree(a, pos);
      } else {
        siglongjmp(old_jmp, j);
      }
      memcpy(v7->jmp_buf, old_jmp, sizeof(old_jmp));
      break;
    }
    case AST_TRY: {
      int percolate = 0;
      jmp_buf old_jmp;
      char *name;
      size_t name_len;
      size_t saved_tmp_stack_pos = v7->tmp_stack.len;
      volatile enum jmp_type j;
      memcpy(old_jmp, v7->jmp_buf, sizeof(old_jmp));

      end = ast_get_skip(a, *pos, AST_END_SKIP);
      acatch = ast_get_skip(a, *pos, AST_TRY_CATCH_SKIP);
      finally = ast_get_skip(a, *pos, AST_TRY_FINALLY_SKIP);
      ast_move_to_children(a, pos);
      if ((j = (enum jmp_type) sigsetjmp(v7->jmp_buf, 0)) == 0) {
        res = i_eval_stmts(v7, a, pos, acatch, scope, brk);
      } else if (j == THROW_JMP && acatch != finally) {
        val_t catch_scope;
        v7->tmp_stack.len = saved_tmp_stack_pos;
        catch_scope = create_object(v7, scope);
        tmp_stack_push(&tf, &catch_scope);
        tag = ast_fetch_tag(a, &acatch);
        V7_CHECK(v7, tag == AST_IDENT);
        name = ast_get_inlined_data(a, acatch, &name_len);
        v7_set_property(v7, catch_scope, name, name_len, 0, v7->thrown_error);
        ast_move_to_children(a, &acatch);
        memcpy(v7->jmp_buf, old_jmp, sizeof(old_jmp));
        res = i_eval_stmts(v7, a, &acatch, finally, catch_scope, brk);
      } else {
        percolate = 1;
      }

      memcpy(v7->jmp_buf, old_jmp, sizeof(old_jmp));
      if (finally != end) {
        enum i_break fin_brk = B_RUN;
        res = i_eval_stmts(v7, a, &finally, end, scope, &fin_brk);
        if (fin_brk != B_RUN) {
          *brk = fin_brk;
        }
        if (!*brk && percolate) {
          siglongjmp(v7->jmp_buf, j);
        }
      }
      *pos = end;
      break;
    }
    case AST_WITH: {
      val_t with_scope = v7_create_undefined();
      tmp_stack_push(&tf, &with_scope);
      end = ast_get_skip(a, *pos, AST_END_SKIP);
      ast_move_to_children(a, pos);
      /*
       * TODO(mkm) make an actual scope chain. Possibly by mutating
       * the with expression and adding the 'outer environment
       * reference' hidden property.
       */
      with_scope = i_eval_expr(v7, a, pos, scope);
      if (!v7_is_object(with_scope)) {
        throw_exception(v7, INTERNAL_ERROR,
                        "with statement is not really implemented yet");
      }
      i_eval_stmts(v7, a, pos, end, with_scope, brk);
      break;
    }
    case AST_VALUE_RETURN:
      res = i_eval_expr(v7, a, pos, scope);
      *brk = B_RETURN;
      break;
    case AST_RETURN:
      *brk = B_RETURN;
      break;
    case AST_BREAK:
      *brk = B_BREAK;
      break;
    case AST_CONTINUE:
      *brk = B_CONTINUE;
      break;
    case AST_LABELED_BREAK:
      V7_CHECK(v7, ast_fetch_tag(a, pos) == AST_IDENT);
      v7->label = ast_get_inlined_data(a, *pos, &v7->label_len);
      siglongjmp(v7->jmp_buf, BREAK_JMP);
      break; /* unreachable */
    case AST_LABELED_CONTINUE:
      V7_CHECK(v7, ast_fetch_tag(a, pos) == AST_IDENT);
      v7->label = ast_get_inlined_data(a, *pos, &v7->label_len);
      siglongjmp(v7->jmp_buf, CONTINUE_JMP);
      break; /* unreachable */
    case AST_THROW:
      v7->thrown_error = i_eval_expr(v7, a, pos, scope);
      siglongjmp(v7->jmp_buf, THROW_JMP);
      break; /* unreachable */
    default:
      (*pos)--;
      res = i_eval_expr(v7, a, pos, scope);
      break;
  }

cleanup:
  tmp_frame_cleanup(&tf);
  return res;
}

/* Invoke a function applying the argument array */
val_t v7_apply(struct v7 *v7, val_t f, val_t this_object, val_t args) {
  struct v7_function *func;
  ast_off_t pos, body, end;
  enum ast_tag tag;
  val_t frame = v7_create_undefined(), res = v7_create_undefined();
  val_t arguments = v7_create_undefined(), saved_this = v7->this_object;
  char *name;
  size_t name_len;
  int i;

  struct gc_tmp_frame vf = new_tmp_frame(v7);
  tmp_stack_push(&vf, &frame);
  tmp_stack_push(&vf, &res);
  tmp_stack_push(&vf, &arguments);
  tmp_stack_push(&vf, &saved_this);
  /*
   * Since v7_apply can be called from user code
   * we have to treat all arguments as roots.
   */
  tmp_stack_push(&vf, &args);
  tmp_stack_push(&vf, &f);
  tmp_stack_push(&vf, &this_object);

  if (!v7_is_cfunction(f) && !v7_is_function(f)) {
    /* extract the hidden property from a cfunction_object */
    struct v7_property *p;
    p = v7_get_own_property2(v7, f, "", 0, V7_PROPERTY_HIDDEN);
    if (p != NULL) {
      f = p->value;
    }
  }

  if (v7_is_cfunction(f)) {
    res = v7_to_cfunction(f)(v7, this_object, args);
    goto cleanup;
  }
  if (!v7_is_function(f)) {
    throw_exception(v7, TYPE_ERROR, "value is not a function");
  }
  func = v7_to_function(f);
  frame = i_prepare_call(v7, func, &pos, &body, &end);

  /*
   * TODO(mkm): don't create arguments array if the parser didn't see
   * any `arguments` or `eval` identifier being referenced in the function.
   */
  arguments = v7_create_dense_array(v7);

  for (i = 0; pos < body; i++) {
    tag = ast_fetch_tag(func->ast, &pos);
    V7_CHECK(v7, tag == AST_IDENT);
    name = ast_get_inlined_data(func->ast, pos, &name_len);
    ast_move_to_children(func->ast, &pos);
    res = v7_array_get(v7, args, i);
    v7_set_property(v7, frame, name, name_len, 0, res);
    if (!v7_is_undefined(arguments)) {
      v7_array_set(v7, arguments, i, res);
    }
  }

  if (!v7_is_undefined(arguments)) {
#ifndef V7_DISABLE_PREDEFINED_STRINGS
    v7_set_v(v7, frame, v7->predefined_strings[PREDEFINED_STR_ARGUMENTS],
             arguments);
#else
    v7_set(v7, frame, "arguments", 9, arguments);
#endif
  }

  v7->this_object = this_object;
  res = i_invoke_function(v7, func, frame, body, end);
  v7->this_object = saved_this;

cleanup:
  tmp_frame_cleanup(&vf);
  return res;
}

enum v7_err v7_exec_with(struct v7 *v7, val_t *res, const char *src, val_t w) {
  /* TODO(mkm): use GC pool */
  struct ast *a = (struct ast *) malloc(sizeof(struct ast));
  val_t old_this = v7->this_object;
  enum i_break brk = B_RUN;
  ast_off_t pos = 0;
  jmp_buf saved_jmp_buf, saved_label_buf;
  size_t saved_tmp_stack_pos = v7->tmp_stack.len;
  enum v7_err err = V7_OK;
  val_t r = v7_create_undefined();

  /* Make v7_exec() reentrant: save exception environments */
  memcpy(&saved_jmp_buf, &v7->jmp_buf, sizeof(saved_jmp_buf));
  memcpy(&saved_label_buf, &v7->label_jmp_buf, sizeof(saved_label_buf));

  ast_init(a, 0);
  mbuf_append(&v7->allocated_asts, (char *) &a, sizeof(a));
  if (sigsetjmp(v7->jmp_buf, 0) != 0) {
    v7->tmp_stack.len = saved_tmp_stack_pos;
    r = v7->thrown_error;
    err = V7_EXEC_EXCEPTION;
    goto cleanup;
  }
  if (parse(v7, a, src, 1) != V7_OK) {
    v7_exec_with(v7, &r, "new SyntaxError(this)",
                 v7_create_string(v7, v7->error_msg, strlen(v7->error_msg), 1));
    err = V7_SYNTAX_ERROR;
    goto cleanup;
  }
  ast_optimize(a);

  v7->this_object = v7_is_undefined(w) ? v7->global_object : w;
  r = i_eval_stmt(v7, a, &pos, v7->global_object, &brk);

cleanup:
  if (res != NULL) {
    *res = r;
  }
  v7->this_object = old_this;
  memcpy(&v7->jmp_buf, &saved_jmp_buf, sizeof(saved_jmp_buf));
  memcpy(&v7->label_jmp_buf, &saved_label_buf, sizeof(saved_label_buf));

  return err;
}

enum v7_err v7_exec(struct v7 *v7, val_t *res, const char *src) {
  return v7_exec_with(v7, res, src, V7_UNDEFINED);
}

enum v7_err v7_exec_file(struct v7 *v7, val_t *res, const char *path) {
  FILE *fp;
  char *p;
  long file_size;
  enum v7_err err = V7_EXEC_EXCEPTION;
  *res = v7_create_undefined();

  if ((fp = fopen(path, "r")) == NULL) {
    snprintf(v7->error_msg, sizeof(v7->error_msg), "cannot open file [%s]",
             path);
    *res = create_exception(v7, SYNTAX_ERROR, v7->error_msg);
  } else if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0) {
    snprintf(v7->error_msg, sizeof(v7->error_msg), "fseek(%s): %s", path,
             strerror(errno));
    *res = create_exception(v7, SYNTAX_ERROR, v7->error_msg);
    fclose(fp);
  } else if ((p = (char *) calloc(1, (size_t) file_size + 1)) == NULL) {
    snprintf(v7->error_msg, sizeof(v7->error_msg), "cannot allocate %ld bytes",
             file_size + 1);
    fclose(fp);
  } else {
    rewind(fp);
    fread(p, 1, (size_t) file_size, fp);
    fclose(fp);
    err = v7_exec(v7, res, p);
    free(p);
  }

  return err;
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 *
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

#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


/* Limitations */
#define SLRE_MAX_RANGES 32
#define SLRE_MAX_SETS 16
#define SLRE_MAX_REP 0xFFFF
#define SLRE_MAX_THREADS 100

#define SLRE_MALLOC malloc
#define SLRE_FREE free
#define SLRE_THROW(e, err_code) longjmp((e)->jmp_buf, (err_code))

/* Parser Information */
struct slre_node {
  unsigned char type;
  union {
    Rune c;                /* character */
    struct slre_class *cp; /* class pointer */
    struct {
      struct slre_node *x;
      union {
        struct slre_node *y;
        unsigned char n;
        struct {
          unsigned char ng; /* not greedy flag */
          unsigned short min;
          unsigned short max;
        } rp;
      } y;
    } xy;
  } par;
};

struct slre_range {
  unsigned short s, e;
};

/* character class, each pair of rune's defines a range */
struct slre_class {
  struct slre_range *end;
  struct slre_range spans[SLRE_MAX_RANGES];
};

struct slre_instruction {
  unsigned char opcode;
  union {
    unsigned char n;
    Rune c;                /* character */
    struct slre_class *cp; /* class pointer */
    struct {
      struct slre_instruction *x;
      union {
        struct {
          unsigned short min;
          unsigned short max;
        } rp;
        struct slre_instruction *y;
      } y;
    } xy;
  } par;
};

struct slre_prog {
  struct slre_instruction *start, *end;
  unsigned int num_captures;
  int flags;
  struct slre_class charset[SLRE_MAX_SETS];
};

struct slre_env {
  int is_regex;
  const char *src;
  const char *src_end;
  Rune curr_rune;

  struct slre_prog *prog;
  struct slre_node *pstart, *pend;

  struct slre_node *caps[SLRE_MAX_CAPS];
  unsigned int num_captures;
  unsigned int sets_num;

  int lookahead;
  struct slre_class *curr_set;
  int min_rep, max_rep;

#if defined(__cplusplus)
  ::jmp_buf jmp_buf;
#else
  jmp_buf jmp_buf;
#endif
};

struct slre_thread {
  struct slre_instruction *pc;
  const char *start;
  struct slre_loot loot;
};

enum slre_opcode {
  I_END = 10, /* Terminate: match found */
  I_ANY,
  P_ANY = I_ANY, /* Any character except newline, . */
  I_ANYNL,       /* Any character including newline, . */
  I_BOL,
  P_BOL = I_BOL, /* Beginning of line, ^ */
  I_CH,
  P_CH = I_CH,
  I_EOL,
  P_EOL = I_EOL, /* End of line, $ */
  I_EOS,
  P_EOS = I_EOS, /* End of string, \0 */
  I_JUMP,
  I_LA,
  P_LA = I_LA,
  I_LA_N,
  P_LA_N = I_LA_N,
  I_LBRA,
  P_BRA = I_LBRA, /* Left bracket, ( */
  I_REF,
  P_REF = I_REF,
  I_REP,
  P_REP = I_REP,
  I_REP_INI,
  I_RBRA, /* Right bracket, ) */
  I_SET,
  P_SET = I_SET, /* Character set, [] */
  I_SET_N,
  P_SET_N = I_SET_N, /* Negated character set, [] */
  I_SPLIT,
  I_WORD,
  P_WORD = I_WORD,
  I_WORD_N,
  P_WORD_N = I_WORD_N,
  P_ALT, /* Alternation, | */
  P_CAT, /* Concatentation, implicit operator */
  L_CH = 256,
  L_COUNT,  /* {M,N} */
  L_EOS,    /* End of string, \0 */
  L_LA,     /* "(?=" lookahead */
  L_LA_CAP, /* "(?:" lookahead, capture */
  L_LA_N,   /* "(?!" negative lookahead */
  L_REF,    /* "\1" back-reference */
  L_SET,    /* character set */
  L_SET_N,  /* negative character set */
  L_WORD,   /* "\b" word boundary */
  L_WORD_N  /* "\B" non-word boundary */
};

static signed char dec(int c) {
  if (isdigitrune(c)) return c - '0';
  return SLRE_INVALID_DEC_DIGIT;
}

static unsigned char re_dec_digit(struct slre_env *e, int c) {
  signed char ret = dec(c);
  if (ret < 0) {
    SLRE_THROW(e, SLRE_INVALID_DEC_DIGIT);
  }
  return ret;
}

static int hex(int c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -SLRE_INVALID_HEX_DIGIT;
}

int nextesc(const char **p) {
  const unsigned char *s = (unsigned char *) (*p)++;
  switch (*s) {
    case 0:
      return -SLRE_UNTERM_ESC_SEQ;
    case 'c':
      ++*p;
      return *s & 31;
    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case 'v':
      return '\v';
    case '\\':
      return '\\';
    case 'u':
      if (isxdigit(s[1]) && isxdigit(s[2]) && isxdigit(s[3]) &&
          isxdigit(s[4])) {
        (*p) += 4;
        return hex(s[1]) << 12 | hex(s[2]) << 8 | hex(s[3]) << 4 | hex(s[4]);
      }
      return -SLRE_INVALID_HEX_DIGIT;
    case 'x':
      if (isxdigit(s[1]) && isxdigit(s[2])) {
        (*p) += 2;
        return (hex(s[1]) << 4) | hex(s[2]);
      }
      return -SLRE_INVALID_HEX_DIGIT;
    default:
      return -SLRE_INVALID_ESC_CHAR;
  }
}

static int re_nextc(Rune *r, const char **src, const char *src_end) {
  *r = 0;
  if (*src >= src_end) return 0;
  *src += chartorune(r, *src);
  if (*r == '\\') {
    const char *tmp_s = *src;
    int i = nextesc(src);
    switch (i) {
      case -SLRE_INVALID_ESC_CHAR:
        *r = '\\';
        *src = tmp_s;
        *src += chartorune(r, *src);
        break;
      case -SLRE_INVALID_HEX_DIGIT:
      default:
        *r = i;
    }
    return 1;
  }
  return 0;
}
static int re_nextc_env(struct slre_env *e) {
  return re_nextc(&e->curr_rune, &e->src, e->src_end);
}

static void re_nchset(struct slre_env *e) {
  if (e->sets_num >= nelem(e->prog->charset)) {
    SLRE_THROW(e, SLRE_TOO_MANY_CHARSETS);
  }
  e->curr_set = e->prog->charset + e->sets_num++;
  e->curr_set->end = e->curr_set->spans;
}

static void re_rng2set(struct slre_env *e, Rune start, Rune end) {
  if (start > end) {
    SLRE_THROW(e, SLRE_INV_CHARSET_RANGE);
  }
  if (e->curr_set->end + 2 == e->curr_set->spans + nelem(e->curr_set->spans)) {
    SLRE_THROW(e, SLRE_CHARSET_TOO_LARGE);
  }
  e->curr_set->end->s = start;
  e->curr_set->end->e = end;
  e->curr_set->end++;
}

#define re_char2set(e, c) re_rng2set(e, c, c)

#define re_d_2set(e) re_rng2set(e, '0', '9')

static void re_D_2set(struct slre_env *e) {
  re_rng2set(e, 0, '0' - 1);
  re_rng2set(e, '9' + 1, 0xFFFF);
}

static void re_s_2set(struct slre_env *e) {
  re_char2set(e, 0x9);
  re_rng2set(e, 0xA, 0xD);
  re_char2set(e, 0x20);
  re_char2set(e, 0xA0);
  re_rng2set(e, 0x2028, 0x2029);
  re_char2set(e, 0xFEFF);
}

static void re_S_2set(struct slre_env *e) {
  re_rng2set(e, 0, 0x9 - 1);
  re_rng2set(e, 0xD + 1, 0x20 - 1);
  re_rng2set(e, 0x20 + 1, 0xA0 - 1);
  re_rng2set(e, 0xA0 + 1, 0x2028 - 1);
  re_rng2set(e, 0x2029 + 1, 0xFEFF - 1);
  re_rng2set(e, 0xFEFF + 1, 0xFFFF);
}

static void re_w_2set(struct slre_env *e) {
  re_d_2set(e);
  re_rng2set(e, 'A', 'Z');
  re_char2set(e, '_');
  re_rng2set(e, 'a', 'z');
}

static void re_W_2set(struct slre_env *e) {
  re_rng2set(e, 0, '0' - 1);
  re_rng2set(e, '9' + 1, 'A' - 1);
  re_rng2set(e, 'Z' + 1, '_' - 1);
  re_rng2set(e, '_' + 1, 'a' - 1);
  re_rng2set(e, 'z' + 1, 0xFFFF);
}

static unsigned char re_endofcount(Rune c) {
  switch (c) {
    case ',':
    case '}':
      return 1;
  }
  return 0;
}

static void re_ex_num_overfl(struct slre_env *e) {
  SLRE_THROW(e, SLRE_NUM_OVERFLOW);
}

static enum slre_opcode re_countrep(struct slre_env *e) {
  e->min_rep = 0;
  while (e->src < e->src_end && !re_endofcount(e->curr_rune = *e->src++))
    e->min_rep = e->min_rep * 10 + re_dec_digit(e, e->curr_rune);
  if (e->min_rep >= SLRE_MAX_REP) re_ex_num_overfl(e);

  if (e->curr_rune != ',') {
    e->max_rep = e->min_rep;
    return L_COUNT;
  }
  e->max_rep = 0;
  while (e->src < e->src_end && (e->curr_rune = *e->src++) != '}')
    e->max_rep = e->max_rep * 10 + re_dec_digit(e, e->curr_rune);
  if (!e->max_rep) {
    e->max_rep = SLRE_MAX_REP;
    return L_COUNT;
  }
  if (e->max_rep >= SLRE_MAX_REP) re_ex_num_overfl(e);

  return L_COUNT;
}

static enum slre_opcode re_lexset(struct slre_env *e) {
  Rune ch;
  unsigned char esc, ch_fl = 0, dash_fl = 0;
  enum slre_opcode type = L_SET;

  re_nchset(e);

  esc = re_nextc_env(e);
  if (!esc && e->curr_rune == '^') {
    type = L_SET_N;
    esc = re_nextc_env(e);
  }

  for (; esc || e->curr_rune != ']'; esc = re_nextc_env(e)) {
    if (!e->curr_rune) {
      SLRE_THROW(e, SLRE_MALFORMED_CHARSET);
    }
    if (esc) {
      if (strchr("DdSsWw", e->curr_rune)) {
        if (ch_fl) {
          re_char2set(e, ch);
          if (dash_fl) re_char2set(e, '-');
        }
        switch (e->curr_rune) {
          case 'D':
            re_D_2set(e);
            break;
          case 'd':
            re_d_2set(e);
            break;
          case 'S':
            re_S_2set(e);
            break;
          case 's':
            re_s_2set(e);
            break;
          case 'W':
            re_W_2set(e);
            break;
          case 'w':
            re_w_2set(e);
            break;
        }
        ch_fl = dash_fl = 0;
        continue;
      }
      switch (e->curr_rune) {
        default:
          /* case '-':
          case '\\':
          case '.':
          case '/':
          case ']':
          case '|': */
          break;
        case '0':
          e->curr_rune = 0;
          break;
        case 'b':
          e->curr_rune = '\b';
          break;
          /* default:
            SLRE_THROW(e->catch_point, e->err_msg,
            SLRE_INVALID_ESC_CHAR); */
      }
    } else {
      if (e->curr_rune == '-') {
        if (ch_fl) {
          if (dash_fl) {
            re_rng2set(e, ch, '-');
            ch_fl = dash_fl = 0;
          } else
            dash_fl = 1;
        } else {
          ch = '-';
          ch_fl = 1;
        }
        continue;
      }
    }
    if (ch_fl) {
      if (dash_fl) {
        re_rng2set(e, ch, e->curr_rune);
        ch_fl = dash_fl = 0;
      } else {
        re_char2set(e, ch);
        ch = e->curr_rune;
      }
    } else {
      ch = e->curr_rune;
      ch_fl = 1;
    }
  }
  if (ch_fl) {
    re_char2set(e, ch);
    if (dash_fl) re_char2set(e, '-');
  }
  return type;
}

static int re_lexer(struct slre_env *e) {
  if (re_nextc_env(e)) {
    switch (e->curr_rune) {
      case '0':
        e->curr_rune = 0;
        return L_EOS;
      case 'b':
        return L_WORD;
      case 'B':
        return L_WORD_N;
      case 'd':
        re_nchset(e);
        re_d_2set(e);
        return L_SET;
      case 'D':
        re_nchset(e);
        re_d_2set(e);
        return L_SET_N;
      case 's':
        re_nchset(e);
        re_s_2set(e);
        return L_SET;
      case 'S':
        re_nchset(e);
        re_s_2set(e);
        return L_SET_N;
      case 'w':
        re_nchset(e);
        re_w_2set(e);
        return L_SET;
      case 'W':
        re_nchset(e);
        re_w_2set(e);
        return L_SET_N;
    }
    if (isdigitrune(e->curr_rune)) {
      e->curr_rune -= '0';
      if (isdigitrune(*e->src))
        e->curr_rune = e->curr_rune * 10 + *e->src++ - '0';
      return L_REF;
    }
    return L_CH;
  }

  if (e->is_regex) {
    switch (e->curr_rune) {
      case 0:
        return 0;
      case '$':
      case ')':
      case '*':
      case '+':
      case '.':
      case '?':
      case '^':
      case '|':
        return e->curr_rune;
      case '{':
        return re_countrep(e);
      case '[':
        return re_lexset(e);
      case '(':
        if (e->src[0] == '?') switch (e->src[1]) {
            case '=':
              e->src += 2;
              return L_LA;
            case ':':
              e->src += 2;
              return L_LA_CAP;
            case '!':
              e->src += 2;
              return L_LA_N;
          }
        return '(';
    }
  } else if (e->curr_rune == 0) {
    return 0;
  }

  return L_CH;
}

#define RE_NEXT(env) (env)->lookahead = re_lexer(env)
#define RE_ACCEPT(env, t) ((env)->lookahead == (t) ? RE_NEXT(env), 1 : 0)

static struct slre_node *re_nnode(struct slre_env *e, int type) {
  memset(e->pend, 0, sizeof(struct slre_node));
  e->pend->type = type;
  return e->pend++;
}

static unsigned char re_isemptynd(struct slre_node *nd) {
  if (!nd) return 1;
  switch (nd->type) {
    default:
      return 1;
    case P_ANY:
    case P_CH:
    case P_SET:
    case P_SET_N:
      return 0;
    case P_BRA:
    case P_REF:
      return re_isemptynd(nd->par.xy.x);
    case P_CAT:
      return re_isemptynd(nd->par.xy.x) && re_isemptynd(nd->par.xy.y.y);
    case P_ALT:
      return re_isemptynd(nd->par.xy.x) || re_isemptynd(nd->par.xy.y.y);
    case P_REP:
      return re_isemptynd(nd->par.xy.x) || !nd->par.xy.y.rp.min;
  }
}

static struct slre_node *re_nrep(struct slre_env *e, struct slre_node *nd,
                                 int ng, unsigned short min,
                                 unsigned short max) {
  struct slre_node *rep = re_nnode(e, P_REP);
  if (max == SLRE_MAX_REP && re_isemptynd(nd)) {
    SLRE_THROW(e, SLRE_INF_LOOP_M_EMP_STR);
  }
  rep->par.xy.y.rp.ng = ng;
  rep->par.xy.y.rp.min = min;
  rep->par.xy.y.rp.max = max;
  rep->par.xy.x = nd;
  return rep;
}

static struct slre_node *re_parser(struct slre_env *e);

static struct slre_node *re_parse_la(struct slre_env *e) {
  struct slre_node *nd;
  int min, max;
  switch (e->lookahead) {
    case '^':
      RE_NEXT(e);
      return re_nnode(e, P_BOL);
    case '$':
      RE_NEXT(e);
      return re_nnode(e, P_EOL);
    case L_EOS:
      RE_NEXT(e);
      return re_nnode(e, P_EOS);
    case L_WORD:
      RE_NEXT(e);
      return re_nnode(e, P_WORD);
    case L_WORD_N:
      RE_NEXT(e);
      return re_nnode(e, P_WORD_N);
  }

  switch (e->lookahead) {
    case L_CH:
      nd = re_nnode(e, P_CH);
      nd->par.c = e->curr_rune;
      RE_NEXT(e);
      break;
    case L_SET:
      nd = re_nnode(e, P_SET);
      nd->par.cp = e->curr_set;
      RE_NEXT(e);
      break;
    case L_SET_N:
      nd = re_nnode(e, P_SET_N);
      nd->par.cp = e->curr_set;
      RE_NEXT(e);
      break;
    case L_REF:
      nd = re_nnode(e, P_REF);
      if (!e->curr_rune || e->curr_rune > e->num_captures ||
          !e->caps[e->curr_rune]) {
        SLRE_THROW(e, SLRE_INVALID_BACK_REFERENCE);
      }
      nd->par.xy.y.n = e->curr_rune;
      nd->par.xy.x = e->caps[e->curr_rune];
      RE_NEXT(e);
      break;
    case '.':
      RE_NEXT(e);
      nd = re_nnode(e, P_ANY);
      break;
    case '(':
      RE_NEXT(e);
      nd = re_nnode(e, P_BRA);
      if (e->num_captures == SLRE_MAX_CAPS) {
        SLRE_THROW(e, SLRE_TOO_MANY_CAPTURES);
      }
      nd->par.xy.y.n = e->num_captures++;
      nd->par.xy.x = re_parser(e);
      e->caps[nd->par.xy.y.n] = nd;
      if (!RE_ACCEPT(e, ')')) {
        SLRE_THROW(e, SLRE_UNMATCH_LBR);
      }
      break;
    case L_LA:
      RE_NEXT(e);
      nd = re_nnode(e, P_LA);
      nd->par.xy.x = re_parser(e);
      if (!RE_ACCEPT(e, ')')) {
        SLRE_THROW(e, SLRE_UNMATCH_LBR);
      }
      break;
    case L_LA_CAP:
      RE_NEXT(e);
      nd = re_parser(e);
      if (!RE_ACCEPT(e, ')')) {
        SLRE_THROW(e, SLRE_UNMATCH_LBR);
      }
      break;
    case L_LA_N:
      RE_NEXT(e);
      nd = re_nnode(e, P_LA_N);
      nd->par.xy.x = re_parser(e);
      if (!RE_ACCEPT(e, ')')) {
        SLRE_THROW(e, SLRE_UNMATCH_LBR);
      }
      break;
    default:
      SLRE_THROW(e, SLRE_SYNTAX_ERROR);
  }

  switch (e->lookahead) {
    case '*':
      RE_NEXT(e);
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), 0, SLRE_MAX_REP);
    case '+':
      RE_NEXT(e);
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), 1, SLRE_MAX_REP);
    case '?':
      RE_NEXT(e);
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), 0, 1);
    case L_COUNT:
      min = e->min_rep, max = e->max_rep;
      RE_NEXT(e);
      if (max < min) {
        SLRE_THROW(e, SLRE_INVALID_QUANTIFIER);
      }
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), min, max);
  }
  return nd;
}

static unsigned char re_endofcat(Rune c, int is_regex) {
  switch (c) {
    case 0:
      return 1;
    case '|':
    case ')':
      if (is_regex) return 1;
  }
  return 0;
}

static struct slre_node *re_parser(struct slre_env *e) {
  struct slre_node *alt = NULL, *cat, *nd;
  if (!re_endofcat(e->lookahead, e->is_regex)) {
    cat = re_parse_la(e);
    while (!re_endofcat(e->lookahead, e->is_regex)) {
      nd = cat;
      cat = re_nnode(e, P_CAT);
      cat->par.xy.x = nd;
      cat->par.xy.y.y = re_parse_la(e);
    }
    alt = cat;
  }
  if (e->lookahead == '|') {
    RE_NEXT(e);
    nd = alt;
    alt = re_nnode(e, P_ALT);
    alt->par.xy.x = nd;
    alt->par.xy.y.y = re_parser(e);
  }
  return alt;
}

static unsigned int re_nodelen(struct slre_node *nd) {
  unsigned int n = 0;
  if (!nd) return 0;
  switch (nd->type) {
    case P_ALT:
      n = 2;
    case P_CAT:
      return re_nodelen(nd->par.xy.x) + re_nodelen(nd->par.xy.y.y) + n;
    case P_BRA:
    case P_LA:
    case P_LA_N:
      return re_nodelen(nd->par.xy.x) + 2;
    case P_REP:
      n = nd->par.xy.y.rp.max - nd->par.xy.y.rp.min;
      switch (nd->par.xy.y.rp.min) {
        case 0:
          if (!n) return 0;
          if (nd->par.xy.y.rp.max >= SLRE_MAX_REP)
            return re_nodelen(nd->par.xy.x) + 2;
        case 1:
          if (!n) return re_nodelen(nd->par.xy.x);
          if (nd->par.xy.y.rp.max >= SLRE_MAX_REP)
            return re_nodelen(nd->par.xy.x) + 1;
        default:
          n = 4;
          if (nd->par.xy.y.rp.max >= SLRE_MAX_REP) n++;
          return re_nodelen(nd->par.xy.x) + n;
      }
    default:
      return 1;
  }
}

static struct slre_instruction *re_newinst(struct slre_prog *prog, int opcode) {
  memset(prog->end, 0, sizeof(struct slre_instruction));
  prog->end->opcode = opcode;
  return prog->end++;
}

static void re_compile(struct slre_env *e, struct slre_node *nd) {
  struct slre_instruction *inst, *split, *jump, *rep;
  unsigned int n;

  if (!nd) return;

  switch (nd->type) {
    case P_ALT:
      split = re_newinst(e->prog, I_SPLIT);
      re_compile(e, nd->par.xy.x);
      jump = re_newinst(e->prog, I_JUMP);
      re_compile(e, nd->par.xy.y.y);
      split->par.xy.x = split + 1;
      split->par.xy.y.y = jump + 1;
      jump->par.xy.x = e->prog->end;
      break;

    case P_ANY:
      re_newinst(e->prog, I_ANY);
      break;

    case P_BOL:
      re_newinst(e->prog, I_BOL);
      break;

    case P_BRA:
      inst = re_newinst(e->prog, I_LBRA);
      inst->par.n = nd->par.xy.y.n;
      re_compile(e, nd->par.xy.x);
      inst = re_newinst(e->prog, I_RBRA);
      inst->par.n = nd->par.xy.y.n;
      break;

    case P_CAT:
      re_compile(e, nd->par.xy.x);
      re_compile(e, nd->par.xy.y.y);
      break;

    case P_CH:
      inst = re_newinst(e->prog, I_CH);
      inst->par.c = nd->par.c;
      break;

    case P_EOL:
      re_newinst(e->prog, I_EOL);
      break;

    case P_EOS:
      re_newinst(e->prog, I_EOS);
      break;

    case P_LA:
      split = re_newinst(e->prog, I_LA);
      re_compile(e, nd->par.xy.x);
      re_newinst(e->prog, I_END);
      split->par.xy.x = split + 1;
      split->par.xy.y.y = e->prog->end;
      break;
    case P_LA_N:
      split = re_newinst(e->prog, I_LA_N);
      re_compile(e, nd->par.xy.x);
      re_newinst(e->prog, I_END);
      split->par.xy.x = split + 1;
      split->par.xy.y.y = e->prog->end;
      break;

    case P_REF:
      inst = re_newinst(e->prog, I_REF);
      inst->par.n = nd->par.xy.y.n;
      break;

    case P_REP:
      n = nd->par.xy.y.rp.max - nd->par.xy.y.rp.min;
      switch (nd->par.xy.y.rp.min) {
        case 0:
          if (!n) break;
          if (nd->par.xy.y.rp.max >= SLRE_MAX_REP) {
            split = re_newinst(e->prog, I_SPLIT);
            re_compile(e, nd->par.xy.x);
            jump = re_newinst(e->prog, I_JUMP);
            jump->par.xy.x = split;
            split->par.xy.x = split + 1;
            split->par.xy.y.y = e->prog->end;
            if (nd->par.xy.y.rp.ng) {
              split->par.xy.y.y = split + 1;
              split->par.xy.x = e->prog->end;
            }
            break;
          }
        case 1:
          if (!n) {
            re_compile(e, nd->par.xy.x);
            break;
          }
          if (nd->par.xy.y.rp.max >= SLRE_MAX_REP) {
            inst = e->prog->end;
            re_compile(e, nd->par.xy.x);
            split = re_newinst(e->prog, I_SPLIT);
            split->par.xy.x = inst;
            split->par.xy.y.y = e->prog->end;
            if (nd->par.xy.y.rp.ng) {
              split->par.xy.y.y = inst;
              split->par.xy.x = e->prog->end;
            }
            break;
          }
        default:
          inst = re_newinst(e->prog, I_REP_INI);
          inst->par.xy.y.rp.min = nd->par.xy.y.rp.min;
          inst->par.xy.y.rp.max = n;
          rep = re_newinst(e->prog, I_REP);
          split = re_newinst(e->prog, I_SPLIT);
          re_compile(e, nd->par.xy.x);
          jump = re_newinst(e->prog, I_JUMP);
          jump->par.xy.x = rep;
          rep->par.xy.x = e->prog->end;
          split->par.xy.x = split + 1;
          split->par.xy.y.y = e->prog->end;
          if (nd->par.xy.y.rp.ng) {
            split->par.xy.y.y = split + 1;
            split->par.xy.x = e->prog->end;
          }
          if (nd->par.xy.y.rp.max >= SLRE_MAX_REP) {
            inst = split + 1;
            split = re_newinst(e->prog, I_SPLIT);
            split->par.xy.x = inst;
            split->par.xy.y.y = e->prog->end;
            if (nd->par.xy.y.rp.ng) {
              split->par.xy.y.y = inst;
              split->par.xy.x = e->prog->end;
            }
            break;
          }
          break;
      }
      break;

    case P_SET:
      inst = re_newinst(e->prog, I_SET);
      inst->par.cp = nd->par.cp;
      break;
    case P_SET_N:
      inst = re_newinst(e->prog, I_SET_N);
      inst->par.cp = nd->par.cp;
      break;

    case P_WORD:
      re_newinst(e->prog, I_WORD);
      break;
    case P_WORD_N:
      re_newinst(e->prog, I_WORD_N);
      break;
  }
}

#ifdef RE_TEST
static void print_set(struct slre_class *cp) {
  struct slre_range *p;
  for (p = cp->spans; p < cp->end; p++) {
    printf("%s", p == cp->spans ? "'" : ",'");
    printf(
        p->s >= 32 && p->s < 127 ? "%c" : (p->s < 256 ? "\\x%02X" : "\\u%04X"),
        p->s);
    if (p->s != p->e) {
      printf(p->e >= 32 && p->e < 127 ? "-%c"
                                      : (p->e < 256 ? "-\\x%02X" : "-\\u%04X"),
             p->e);
    }
    printf("'");
  }
  printf("]");
}

static void node_print(struct slre_node *nd) {
  if (!nd) {
    printf("Empty");
    return;
  }
  switch (nd->type) {
    case P_ALT:
      printf("{");
      node_print(nd->par.xy.x);
      printf(" | ");
      node_print(nd->par.xy.y.y);
      printf("}");
      break;
    case P_ANY:
      printf(".");
      break;
    case P_BOL:
      printf("^");
      break;
    case P_BRA:
      node_print(nd->par.xy.x);
      printf(")");
      break;
    case P_CAT:
      printf("{");
      node_print(nd->par.xy.x);
      printf(" & ");
      node_print(nd->par.xy.y.y);
      printf("}");
      break;
    case P_CH:
      printf(nd->par.c >= 32 && nd->par.c < 127 ? "'%c'" : "'\\u%04X'",
             nd->par.c);
      break;
    case P_EOL:
      printf("$");
      break;
    case P_EOS:
      printf("\\0");
      break;
    case P_LA:
      printf("LA(");
      node_print(nd->par.xy.x);
      printf(")");
      break;
    case P_LA_N:
      printf("LA_N(");
      node_print(nd->par.xy.x);
      printf(")");
      break;
    case P_REF:
      printf("\\%d", nd->par.xy.y.n);
      break;
    case P_REP:
      node_print(nd->par.xy.x);
      printf(nd->par.xy.y.rp.ng ? "{%d,%d}?" : "{%d,%d}", nd->par.xy.y.rp.min,
             nd->par.xy.y.rp.max);
      break;
    case P_SET:
      printf("[");
      print_set(nd->par.cp);
      break;
    case P_SET_N:
      printf("[^");
      print_set(nd->par.cp);
      break;
    case P_WORD:
      printf("\\b");
      break;
    case P_WORD_N:
      printf("\\B");
      break;
  }
}

static void program_print(struct slre_prog *prog) {
  struct slre_instruction *inst;
  for (inst = prog->start; inst < prog->end; ++inst) {
    printf("%3d: ", inst - prog->start);
    switch (inst->opcode) {
      case I_END:
        puts("end");
        break;
      case I_ANY:
        puts(".");
        break;
      case I_ANYNL:
        puts(". | '\\r' | '\\n'");
        break;
      case I_BOL:
        puts("^");
        break;
      case I_CH:
        printf(
            inst->par.c >= 32 && inst->par.c < 127 ? "'%c'\n" : "'\\u%04X'\n",
            inst->par.c);
        break;
      case I_EOL:
        puts("$");
        break;
      case I_EOS:
        puts("\\0");
        break;
      case I_JUMP:
        printf("-->%d\n", inst->par.xy.x - prog->start);
        break;
      case I_LA:
        printf("la %d %d\n", inst->par.xy.x - prog->start,
               inst->par.xy.y.y - prog->start);
        break;
      case I_LA_N:
        printf("la_n %d %d\n", inst->par.xy.x - prog->start,
               inst->par.xy.y.y - prog->start);
        break;
      case I_LBRA:
        printf("( %d\n", inst->par.n);
        break;
      case I_RBRA:
        printf(") %d\n", inst->par.n);
        break;
      case I_SPLIT:
        printf("-->%d | -->%d\n", inst->par.xy.x - prog->start,
               inst->par.xy.y.y - prog->start);
        break;
      case I_REF:
        printf("\\%d\n", inst->par.n);
        break;
      case I_REP:
        printf("repeat -->%d\n", inst->par.xy.x - prog->start);
        break;
      case I_REP_INI:
        printf("init_rep %d %d\n", inst->par.xy.y.rp.min,
               inst->par.xy.y.rp.min + inst->par.xy.y.rp.max);
        break;
      case I_SET:
        printf("[");
        print_set(inst->par.cp);
        puts("");
        break;
      case I_SET_N:
        printf("[^");
        print_set(inst->par.cp);
        puts("");
        break;
      case I_WORD:
        puts("\\w");
        break;
      case I_WORD_N:
        puts("\\W");
        break;
    }
  }
}
#endif

int slre_compile(const char *pat, size_t pat_len, const char *flags,
                 size_t fl_len, struct slre_prog **pr, int is_regex) {
  struct slre_env e;
  struct slre_node *nd;
  struct slre_instruction *split, *jump;
  int err_code;

  e.is_regex = is_regex;
  e.prog = (struct slre_prog *) SLRE_MALLOC(sizeof(struct slre_prog));
  e.pstart = e.pend =
      (struct slre_node *) SLRE_MALLOC(sizeof(struct slre_node) * pat_len * 2);
  e.prog->flags = is_regex ? SLRE_FLAG_RE : 0;

  if ((err_code = setjmp(e.jmp_buf)) != SLRE_OK) {
    SLRE_FREE(e.pstart);
    SLRE_FREE(e.prog);
    return err_code;
  }

  while (fl_len--) {
    switch (flags[fl_len]) {
      case 'g':
        e.prog->flags |= SLRE_FLAG_G;
        break;
      case 'i':
        e.prog->flags |= SLRE_FLAG_I;
        break;
      case 'm':
        e.prog->flags |= SLRE_FLAG_M;
        break;
    }
  }

  e.src = pat;
  e.src_end = pat + pat_len;
  e.sets_num = 0;
  e.num_captures = 1;
  /*e.flags = flags;*/
  memset(e.caps, 0, sizeof(e.caps));

  RE_NEXT(&e);
  nd = re_parser(&e);
  if (e.lookahead == ')') {
    SLRE_THROW(&e, SLRE_UNMATCH_RBR);
  }
  if (e.lookahead != 0) {
    SLRE_THROW(&e, SLRE_SYNTAX_ERROR);
  }

  e.prog->num_captures = e.num_captures;
  e.prog->start = e.prog->end = (struct slre_instruction *) SLRE_MALLOC(
      (re_nodelen(nd) + 6) * sizeof(struct slre_instruction));

  split = re_newinst(e.prog, I_SPLIT);
  split->par.xy.x = split + 3;
  split->par.xy.y.y = split + 1;
  re_newinst(e.prog, I_ANYNL);
  jump = re_newinst(e.prog, I_JUMP);
  jump->par.xy.x = split;
  re_newinst(e.prog, I_LBRA);
  re_compile(&e, nd);
  re_newinst(e.prog, I_RBRA);
  re_newinst(e.prog, I_END);

#ifdef RE_TEST
  node_print(nd);
  putchar('\n');
  program_print(e.prog);
#endif

  SLRE_FREE(e.pstart);

  if (pr != NULL) {
    *pr = e.prog;
  } else {
    slre_free(e.prog);
  }

  return err_code;
}

void slre_free(struct slre_prog *prog) {
  if (prog) {
    SLRE_FREE(prog->start);
    SLRE_FREE(prog);
  }
}

static void re_newthread(struct slre_thread *t, struct slre_instruction *pc,
                         const char *start, struct slre_loot *loot) {
  t->pc = pc;
  t->start = start;
  t->loot = *loot;
}

#define RE_NO_MATCH() \
  if (!(thr = 0)) continue

static unsigned char re_match(struct slre_instruction *pc, const char *current,
                              const char *end, const char *bol,
                              unsigned int flags, struct slre_loot *loot) {
  struct slre_loot sub, tmpsub;
  Rune c, r;
  struct slre_range *p;
  unsigned short thr_num = 1;
  unsigned char thr;
  size_t i;
  struct slre_thread threads[SLRE_MAX_THREADS];

  /* queue initial thread */
  re_newthread(threads, pc, current, loot);

  /* run threads in stack order */
  do {
    pc = threads[--thr_num].pc;
    current = threads[thr_num].start;
    sub = threads[thr_num].loot;
    for (thr = 1; thr;) {
      switch (pc->opcode) {
        case I_END:
          memcpy(loot->caps, sub.caps, sizeof loot->caps);
          return 1;
        case I_ANY:
        case I_ANYNL:
          if (current < end) {
            current += chartorune(&c, current);
            if (c && !(pc->opcode == I_ANY && isnewline(c))) break;
          }
          RE_NO_MATCH();

        case I_BOL:
          if (current == bol) break;
          if ((flags & SLRE_FLAG_M) && isnewline(current[-1])) break;
          RE_NO_MATCH();
        case I_CH:
          if (current < end) {
            current += chartorune(&c, current);
            if (c &&
                (c == pc->par.c || ((flags & SLRE_FLAG_I) &&
                                    tolowerrune(c) == tolowerrune(pc->par.c))))
              break;
          }
          RE_NO_MATCH();
        case I_EOL:
          if (current >= end) break;
          if ((flags & SLRE_FLAG_M) && isnewline(*current)) break;
          RE_NO_MATCH();
        case I_EOS:
          if (current >= end) break;
          RE_NO_MATCH();

        case I_JUMP:
          pc = pc->par.xy.x;
          continue;

        case I_LA:
          if (re_match(pc->par.xy.x, current, end, bol, flags, &sub)) {
            pc = pc->par.xy.y.y;
            continue;
          }
          RE_NO_MATCH();
        case I_LA_N:
          tmpsub = sub;
          if (!re_match(pc->par.xy.x, current, end, bol, flags, &tmpsub)) {
            pc = pc->par.xy.y.y;
            continue;
          }
          RE_NO_MATCH();

        case I_LBRA:
          sub.caps[pc->par.n].start = current;
          break;

        case I_REF:
          i = sub.caps[pc->par.n].end - sub.caps[pc->par.n].start;
          if (flags & SLRE_FLAG_I) {
            int num = i;
            const char *s = current, *p = sub.caps[pc->par.n].start;
            Rune rr;
            for (; num && *s && *p; num--) {
              s += chartorune(&r, s);
              p += chartorune(&rr, p);
              if (tolowerrune(r) != tolowerrune(rr)) break;
            }
            if (num) RE_NO_MATCH();
          } else if (strncmp(current, sub.caps[pc->par.n].start, i)) {
            RE_NO_MATCH();
          }
          if (i > 0) current += i;
          break;

        case I_REP:
          if (pc->par.xy.y.rp.min) {
            pc->par.xy.y.rp.min--;
            pc++;
          } else if (!pc->par.xy.y.rp.max--) {
            pc = pc->par.xy.x;
            continue;
          }
          break;

        case I_REP_INI:
          (pc + 1)->par.xy.y.rp.min = pc->par.xy.y.rp.min;
          (pc + 1)->par.xy.y.rp.max = pc->par.xy.y.rp.max;
          break;

        case I_RBRA:
          sub.caps[pc->par.n].end = current;
          break;

        case I_SET:
        case I_SET_N:
          if (current >= end) RE_NO_MATCH();
          current += chartorune(&c, current);
          if (!c) RE_NO_MATCH();

          i = 1;
          for (p = pc->par.cp->spans; i && p < pc->par.cp->end; p++)
            if (flags & SLRE_FLAG_I) {
              for (r = p->s; r <= p->e; ++r)
                if (tolowerrune(c) == tolowerrune(r)) {
                  i = 0;
                  break;
                }
            } else if (p->s <= c && c <= p->e)
              i = 0;

          if (pc->opcode == I_SET) i = !i;
          if (i) break;
          RE_NO_MATCH();

        case I_SPLIT:
          if (thr_num >= SLRE_MAX_THREADS) {
            fprintf(stderr, "re_match: backtrack overflow!\n");
            return 0;
          }
          re_newthread(&threads[thr_num++], pc->par.xy.y.y, current, &sub);
          pc = pc->par.xy.x;
          continue;

        case I_WORD:
        case I_WORD_N:
          i = (current > bol && iswordchar(current[-1]));
          if (iswordchar(current[0])) i = !i;
          if (pc->opcode == I_WORD_N) i = !i;
          if (i) break;
        /* RE_NO_MATCH(); */

        default:
          RE_NO_MATCH();
      }
      pc++;
    }
  } while (thr_num);
  return 0;
}

int slre_exec(struct slre_prog *prog, int flag_g, const char *start,
              const char *end, struct slre_loot *loot) {
  struct slre_loot tmpsub;
  const char *st = start;

  if (!loot) loot = &tmpsub;
  memset(loot, 0, sizeof(*loot));

  if (!flag_g) {
    loot->num_captures = prog->num_captures;
    return !re_match(prog->start, start, end, start, prog->flags, loot);
  }

  while (re_match(prog->start, st, end, start, prog->flags, &tmpsub)) {
    unsigned int i;
    st = tmpsub.caps[0].end;
    for (i = 0; i < prog->num_captures; i++) {
      struct slre_cap *l = &loot->caps[loot->num_captures + i];
      struct slre_cap *s = &tmpsub.caps[i];
      l->start = s->start;
      l->end = s->end;
    }
    loot->num_captures += prog->num_captures;
  }
  return !loot->num_captures;
}

int slre_replace(struct slre_loot *loot, const char *src, size_t src_len,
                 const char *rstr, size_t rstr_len, struct slre_loot *dstsub) {
  int size = 0, n;
  Rune curr_rune;
  const char *const rstr_end = rstr + rstr_len;

  memset(dstsub, 0, sizeof(*dstsub));
  while (rstr < rstr_end && !(n = re_nextc(&curr_rune, &rstr, rstr_end)) &&
         curr_rune) {
    int sz;
    if (n < 0) return n;
    if (curr_rune == '$') {
      n = re_nextc(&curr_rune, &rstr, rstr_end);
      if (n < 0) return n;
      switch (curr_rune) {
        case '&':
          sz = loot->caps[0].end - loot->caps[0].start;
          size += sz;
          dstsub->caps[dstsub->num_captures++] = loot->caps[0];
          break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
          int sbn = dec(curr_rune);
          if (0 == sbn && rstr[0] && isdigitrune(rstr[0])) {
            n = re_nextc(&curr_rune, &rstr, rstr_end);
            if (n < 0) return n;
            sz = dec(curr_rune);
            sbn = sbn * 10 + sz;
          }
          if (sbn >= loot->num_captures) break;
          sz = loot->caps[sbn].end - loot->caps[sbn].start;
          size += sz;
          dstsub->caps[dstsub->num_captures++] = loot->caps[sbn];
          break;
        }
        case '`':
          sz = loot->caps[0].start - src;
          size += sz;
          dstsub->caps[dstsub->num_captures].start = src;
          dstsub->caps[dstsub->num_captures++].end = loot->caps[0].start;
          break;
        case '\'':
          sz = src + src_len - loot->caps[0].end;
          size += sz;
          dstsub->caps[dstsub->num_captures].start = loot->caps[0].end;
          dstsub->caps[dstsub->num_captures++].end = loot->caps[0].end + sz;
          break;
        case '$':
          size++;
          dstsub->caps[dstsub->num_captures].start = rstr - 1;
          dstsub->caps[dstsub->num_captures++].end = rstr;
          break;
        default:
          return SLRE_BAD_CHAR_AFTER_USD;
      }
    } else {
      char tmps[300], *d = tmps;
      size += (sz = runetochar(d, &curr_rune));
      if (!dstsub->num_captures ||
          dstsub->caps[dstsub->num_captures - 1].end != rstr - sz) {
        dstsub->caps[dstsub->num_captures].start = rstr - sz;
        dstsub->caps[dstsub->num_captures++].end = rstr;
      } else
        dstsub->caps[dstsub->num_captures - 1].end = rstr;
    }
  }
  return size;
}

int slre_match(const char *re, size_t re_len, const char *flags, size_t fl_len,
               const char *str, size_t str_len, struct slre_loot *loot) {
  struct slre_prog *prog = NULL;
  int res;

  if ((res = slre_compile(re, re_len, flags, fl_len, &prog, 1)) == SLRE_OK) {
    res = slre_exec(prog, prog->flags & SLRE_FLAG_G, str, str + str_len, loot);
    slre_free(prog);
  }

  return res;
}

int slre_get_flags(struct slre_prog *crp) {
  return crp->flags;
}

#ifdef SLRE_TEST

#include <errno.h>

static const char *err_code_to_str(int err_code) {
  static const char *ar[] = {
      "no error", "invalid decimal digit", "invalid hex digit",
      "invalid escape character", "invalid unterminated escape sequence",
      "syntax error", "unmatched left parenthesis",
      "unmatched right parenthesis", "numeric overflow",
      "infinite loop empty string", "too many charsets",
      "invalid charset range", "charset is too large", "malformed charset",
      "invalid back reference", "too many captures", "invalid quantifier",
      "bad character after $"};

  typedef char static_assertion_err_codes_out_of_sync
      [2 * !!(((sizeof(ar) / sizeof(ar[0])) == SLRE_BAD_CHAR_AFTER_USD + 1)) -
       1];

  return err_code >= 0 && err_code < (int) (sizeof(ar) / sizeof(ar[0]))
             ? ar[err_code]
             : "invalid error code";
}

#define RE_TEST_STR_SIZE 2000

static unsigned get_flags(const char *ch) {
  unsigned int flags = 0;

  while (*ch != '\0') {
    switch (*ch) {
      case 'g':
        flags |= SLRE_FLAG_G;
        break;
      case 'i':
        flags |= SLRE_FLAG_I;
        break;
      case 'm':
        flags |= SLRE_FLAG_M;
        break;
      case 'r':
        flags |= SLRE_FLAG_RE;
        break;
      default:
        return flags;
    }
    ch++;
  }
  return flags;
}

static void show_usage_and_exit(char *argv[]) {
  fprintf(stderr, "Usage: %s [OPTIONS]\n", argv[0]);
  fprintf(stderr, "%s\n", "OPTIONS:");
  fprintf(stderr, "%s\n", "  -p <regex_pattern>     Regex pattern");
  fprintf(stderr, "%s\n", "  -o <regex_flags>       Combination of g,i,m");
  fprintf(stderr, "%s\n", "  -s <string>            String to match");
  fprintf(stderr, "%s\n", "  -f <file_name>         Match lines from file");
  fprintf(stderr, "%s\n", "  -n <cap_no>            Show given capture");
  fprintf(stderr, "%s\n", "  -r <replace_str>       Replace given capture");
  fprintf(stderr, "%s\n", "  -v                     Show verbose stats");
  exit(1);
}

static int process_line(struct slre_prog *pr, const char *flags,
                        const char *line, const char *cap_no,
                        const char *replace, const char *verbose) {
  struct slre_loot loot;
  unsigned int fl = flags == NULL ? 0 : get_flags(flags);
  int i, n = cap_no == NULL ? -1 : atoi(cap_no), err_code = 0;
  struct slre_cap *cap = &loot.caps[n];

  err_code =
      slre_exec(pr, pr->flags & SLRE_FLAG_G, line, line + strlen(line), &loot);
  if (err_code == SLRE_OK) {
    if (n >= 0 && n < loot.num_captures && replace != NULL) {
      struct slre_cap *cap = &loot.caps[n];
      printf("%.*s", (int) (cap->start - line), line);
      printf("%s", replace);
      printf("%.*s", (int) ((line + strlen(line)) - cap->end), cap->end);
    } else if (n >= 0 && n < loot.num_captures) {
      printf("%.*s\n", (int) (cap->end - cap->start), cap->start);
    }

    if (verbose != NULL) {
      fprintf(stderr, "%s\n", "Captures:");
      for (i = 0; i < loot.num_captures; i++) {
        fprintf(stderr, "%d [%.*s]\n", i,
                (int) (loot.caps[i].end - loot.caps[i].start),
                loot.caps[i].start);
      }
    }
  }

  return err_code;
}

int main(int argc, char **argv) {
  const char *str = NULL, *pattern = NULL, *replace = NULL;
  const char *flags = "", *file_name = NULL, *cap_no = NULL, *verbose = NULL;
  struct slre_prog *pr = NULL;
  int i, err_code = 0;

  /* Execute inline code */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
      pattern = argv[++i];
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      flags = argv[++i];
    } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
      str = argv[++i];
    } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
      file_name = argv[++i];
    } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
      cap_no = argv[++i];
    } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
      replace = argv[++i];
    } else if (strcmp(argv[i], "-v") == 0) {
      verbose = "";
    } else if (strcmp(argv[i], "-h") == 0) {
      show_usage_and_exit(argv);
    } else {
      show_usage_and_exit(argv);
    }
  }

  if (pattern == NULL) {
    fprintf(stderr, "%s\n", "-p option is mandatory");
    exit(1);
  } else if ((err_code = slre_compile(pattern, strlen(pattern), flags,
                                      strlen(flags), &pr, 1)) != SLRE_OK) {
    fprintf(stderr, "slre_compile(%s): %s\n", argv[0],
            err_code_to_str(err_code));
    exit(1);
  } else if (str != NULL) {
    err_code = process_line(pr, flags, str, cap_no, replace, verbose);
  } else if (file_name != NULL) {
    FILE *fp = strcmp(file_name, "-") == 0 ? stdin : fopen(file_name, "rb");
    char line[20 * 1024];
    if (fp == NULL) {
      fprintf(stderr, "Cannot open %s: %s\n", file_name, strerror(errno));
      exit(1);
    } else {
      /* Return success if at least one line matches */
      err_code = 1;
      while (fgets(line, sizeof(line), fp) != NULL) {
        if (process_line(pr, flags, line, cap_no, replace, verbose) ==
            SLRE_OK) {
          err_code = 0;
        }
      }
      fclose(fp); /* If fp == stdin, it is safe to close, too */
    }
  } else {
    fprintf(stderr, "%s\n", "Please specify one of -s or -f options");
    exit(1);
  }
  slre_free(pr);

  return err_code;
}
#endif /* SLRE_TEST */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


static val_t Obj_getPrototypeOf(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg = v7_array_get(v7, args, 0);
  (void) this_obj;
  if (!v7_is_object(arg)) {
    throw_exception(v7, TYPE_ERROR,
                    "Object.getPrototypeOf called on non-object");
  }
  return v_get_prototype(v7, arg);
}

static val_t Obj_isPrototypeOf(struct v7 *v7, val_t this_obj, val_t args) {
  val_t obj = v7_array_get(v7, args, 0);
  val_t proto = v7_array_get(v7, args, 1);
  (void) this_obj;
  return v7_create_boolean(is_prototype_of(v7, obj, proto));
}

/* Hack to ensure that the iteration order of the keys array is consistent
 * with the iteration order if properties in `for in`
 * This will be obsoleted when arrays will have a special object type. */
static void _Obj_append_reverse(struct v7 *v7, struct v7_property *p, val_t res,
                                int i, unsigned int ignore_flags) {
  while (p && p->attributes & ignore_flags) p = p->next;
  if (p == NULL) return;
  if (p->next) _Obj_append_reverse(v7, p->next, res, i + 1, ignore_flags);

  v7_array_set(v7, res, i, p->name);
}

static val_t _Obj_ownKeys(struct v7 *v7, val_t args,
                          unsigned int ignore_flags) {
  val_t obj = v7_array_get(v7, args, 0);
  val_t res = v7_create_dense_array(v7);
  if (!v7_is_object(obj)) {
    throw_exception(v7, TYPE_ERROR, "Object.keys called on non-object");
  }

  _Obj_append_reverse(v7, v7_to_object(obj)->properties, res, 0, ignore_flags);
  return res;
}

static struct v7_property *_Obj_getOwnProperty(struct v7 *v7, val_t obj,
                                               val_t name) {
  char name_buf[512];
  int name_len;
  name_len = v7_stringify_value(v7, name, name_buf, sizeof(name_buf));
  return v7_get_own_property(v7, obj, name_buf, name_len);
}

static val_t Obj_keys(struct v7 *v7, val_t this_obj, val_t args) {
  (void) this_obj;
  return _Obj_ownKeys(v7, args, V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM);
}

static val_t Obj_getOwnPropertyNames(struct v7 *v7, val_t this_obj,
                                     val_t args) {
  (void) this_obj;
  return _Obj_ownKeys(v7, args, V7_PROPERTY_HIDDEN);
}

static val_t Obj_getOwnPropertyDescriptor(struct v7 *v7, val_t this_obj,
                                          val_t args) {
  struct v7_property *prop;
  val_t obj = v7_array_get(v7, args, 0);
  val_t name = v7_array_get(v7, args, 1);
  val_t desc;
  (void) this_obj;
  if ((prop = _Obj_getOwnProperty(v7, obj, name)) == NULL) {
    return V7_UNDEFINED;
  }
  desc = v7_create_object(v7);
  v7_set_property(v7, desc, "value", 5, 0, prop->value);
  v7_set_property(
      v7, desc, "writable", 8, 0,
      v7_create_boolean(!(prop->attributes & V7_PROPERTY_READ_ONLY)));
  v7_set_property(
      v7, desc, "enumerable", 10, 0,
      v7_create_boolean(
          !(prop->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM))));
  v7_set_property(
      v7, desc, "configurable", 12, 0,
      v7_create_boolean(!(prop->attributes & V7_PROPERTY_DONT_DELETE)));

  return desc;
}

static void o_set_attr(struct v7 *v7, val_t desc, const char *name, size_t n,
                       struct v7_property *prop, unsigned int attr) {
  val_t v = v7_get(v7, desc, name, n);
  if (v7_is_true(v7, v)) {
    prop->attributes &= ~attr;
  } else {
    prop->attributes |= attr;
  }
}

static val_t _Obj_defineProperty(struct v7 *v7, val_t obj, const char *name,
                                 int name_len, val_t desc) {
  val_t val = v7_get(v7, desc, "value", 5);
  struct v7_property *prop = v7_get_own_property(v7, obj, name, name_len);

  if (prop == NULL) {
    val_t key = v7_create_string(v7, name, name_len, 1);
    prop = v7_set_prop(v7, obj, key, 0, val);
  }

  if (prop == NULL) {
    throw_exception(v7, INTERNAL_ERROR, "OOM");
  } else {
    o_set_attr(v7, desc, "enumerable", 10, prop, V7_PROPERTY_DONT_ENUM);
    o_set_attr(v7, desc, "writable", 8, prop, V7_PROPERTY_READ_ONLY);
    o_set_attr(v7, desc, "configurable", 12, prop, V7_PROPERTY_DONT_DELETE);
    if (!v7_is_undefined(val)) {
      prop->value = val;
    }
  }

  return obj;
}

static val_t Obj_defineProperty(struct v7 *v7, val_t this_obj, val_t args) {
  val_t obj = v7_array_get(v7, args, 0);
  val_t name = v7_array_get(v7, args, 1);
  val_t desc = v7_array_get(v7, args, 2);
  char name_buf[512];
  int name_len;
  (void) this_obj;
  if (!v7_is_object(obj)) {
    throw_exception(v7, TYPE_ERROR, "object expected");
  }
  name_len = v7_stringify_value(v7, name, name_buf, sizeof(name_buf));
  return _Obj_defineProperty(v7, obj, name_buf, name_len, desc);
}

static void o_define_props(struct v7 *v7, val_t obj, val_t descs) {
  struct v7_property *p;
  if (!v7_is_object(descs)) {
    throw_exception(v7, TYPE_ERROR, "object expected");
  }
  for (p = v7_to_object(descs)->properties; p; p = p->next) {
    size_t n;
    const char *s = v7_to_string(v7, &p->name, &n);
    if (p->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)) {
      continue;
    }
    _Obj_defineProperty(v7, obj, s, n, p->value);
  }
}

static val_t Obj_defineProperties(struct v7 *v7, val_t this_obj, val_t args) {
  val_t obj = v7_array_get(v7, args, 0);
  val_t descs = v7_array_get(v7, args, 1);
  (void) this_obj;
  o_define_props(v7, obj, descs);
  return obj;
}

static val_t Obj_create(struct v7 *v7, val_t this_obj, val_t args) {
  val_t res, proto = v7_array_get(v7, args, 0);
  val_t descs = v7_array_get(v7, args, 1);
  (void) this_obj;
  if (!v7_is_null(proto) && !v7_is_object(proto)) {
    throw_exception(v7, TYPE_ERROR,
                    "Object prototype may only be an Object or null");
  }
  res = create_object(v7, proto);
  if (v7_is_object(descs)) {
    o_define_props(v7, res, descs);
  }
  return res;
}

static val_t Obj_propertyIsEnumerable(struct v7 *v7, val_t this_obj,
                                      val_t args) {
  struct v7_property *prop;
  val_t name = v7_array_get(v7, args, 0);
  if ((prop = _Obj_getOwnProperty(v7, this_obj, name)) == NULL) {
    return v7_create_boolean(0);
  }
  return v7_create_boolean(
      !(prop->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)));
}

static val_t Obj_hasOwnProperty(struct v7 *v7, val_t this_obj, val_t args) {
  val_t name = v7_array_get(v7, args, 0);
  return v7_create_boolean(_Obj_getOwnProperty(v7, this_obj, name) != NULL);
}

#if 0
static enum v7_err Obj_toString(struct v7_c_func_arg *cfa) {
  char *p, buf[500];
  p = v7_stringify(cfa->this_obj, buf, sizeof(buf));
  v7_push_string(cfa->v7, p, strlen(p), 1);
  if (p != buf) free(p);
  return V7_OK;
}

static enum v7_err Obj_keys(struct v7_c_func_arg *cfa) {
  struct v7_prop *p;
  struct v7_val *result = v7_push_new_object(cfa->v7);
  v7_set_class(result, V7_CLASS_ARRAY);
  for (p = cfa->this_obj->props; p != NULL; p = p->next) {
    v7_append(cfa->v7, result, p->key);
  }
  return V7_OK;
}
#endif

V7_PRIVATE val_t Obj_valueOf(struct v7 *v7, val_t this_obj, val_t args) {
  val_t res = this_obj;
  struct v7_property *p;

  (void) args;
  p = v7_get_own_property2(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN);
  if (p != NULL) {
    res = p->value;
  }

  return res;
}

static val_t Obj_toString(struct v7 *v7, val_t this_obj, val_t args) {
  char buf[20];
  const char *type = "Object";
  (void) args;
  if (is_prototype_of(v7, this_obj, v7->array_prototype)) {
    type = "Array";
  }
  snprintf(buf, sizeof(buf), "[object %s]", type);
  return v7_create_string(v7, buf, strlen(buf), 1);
}

static val_t Obj_preventExtensions(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg = v7_array_get(v7, args, 0);
  (void) this_obj;
  if (!v7_is_object(arg)) {
    throw_exception(v7, TYPE_ERROR, "Object expected");
  }
  v7_to_object(arg)->attributes |= V7_OBJ_NOT_EXTENSIBLE;
  return arg;
}

static val_t Obj_isExtensible(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg = v7_array_get(v7, args, 0);
  (void) this_obj;
  if (!v7_is_object(arg)) {
    throw_exception(v7, TYPE_ERROR, "Object expected");
  }
  return v7_create_boolean(
      !(v7_to_object(arg)->attributes & V7_OBJ_NOT_EXTENSIBLE));
}

V7_PRIVATE void init_object(struct v7 *v7) {
  val_t object, v;
  /* TODO(mkm): initialize global object without requiring a parser */
  v7_exec(v7, &v,
          "function Object(v) {"
          "if (typeof v === 'boolean') return new Boolean(v);"
          "if (typeof v === 'number') return new Number(v);"
          "if (typeof v === 'string') return new String(v);"
          "if (typeof v === 'date') return new Date(v);"
          "}");

  object = v7_get(v7, v7->global_object, "Object", 6);
  v7_set(v7, object, "prototype", 9, v7->object_prototype);
  v7_set(v7, v7->object_prototype, "constructor", 11, object);

  set_cfunc_obj_prop(v7, v7->object_prototype, "toString", Obj_toString, 0);
  set_cfunc_prop(v7, object, "getPrototypeOf", Obj_getPrototypeOf);
  set_cfunc_prop(v7, object, "getOwnPropertyDescriptor",
                 Obj_getOwnPropertyDescriptor);
  set_cfunc_obj_prop(v7, object, "defineProperty", Obj_defineProperty, 3);
  set_cfunc_prop(v7, object, "defineProperties", Obj_defineProperties);
  set_cfunc_prop(v7, object, "create", Obj_create);
  set_cfunc_prop(v7, object, "keys", Obj_keys);
  set_cfunc_prop(v7, object, "getOwnPropertyNames", Obj_getOwnPropertyNames);
  set_cfunc_obj_prop(v7, object, "preventExtensions", Obj_preventExtensions, 1);
  set_cfunc_obj_prop(v7, object, "isExtensible", Obj_isExtensible, 1);

  set_cfunc_prop(v7, v7->object_prototype, "propertyIsEnumerable",
                 Obj_propertyIsEnumerable);
  set_cfunc_prop(v7, v7->object_prototype, "hasOwnProperty",
                 Obj_hasOwnProperty);
  set_cfunc_prop(v7, v7->object_prototype, "isPrototypeOf", Obj_isPrototypeOf);
  set_cfunc_prop(v7, v7->object_prototype, "valueOf", Obj_valueOf);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


static val_t Error_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t res;

  if (v7_is_object(this_obj) && this_obj != v7->global_object) {
    res = this_obj;
  } else {
    res = create_object(v7, v7->error_prototype);
  }
  /* TODO(mkm): set non enumerable but provide toString method */
  v7_set_property(v7, res, "message", 7, 0, arg0);

  return res;
}

static const char *error_names[] = {"TypeError", "SyntaxError",
                                    "ReferenceError", "InternalError",
                                    "RangeError"};
V7_STATIC_ASSERT(ARRAY_SIZE(error_names) == ERROR_CTOR_MAX,
                 error_name_count_mismatch);

V7_PRIVATE void init_error(struct v7 *v7) {
  val_t error;
  size_t i;

  error = v7_create_cfunction_ctor(v7, v7->error_prototype, Error_ctor, 1);
  v7_set_property(v7, v7->global_object, "Error", 5, V7_PROPERTY_DONT_ENUM,
                  error);

  for (i = 0; i < ARRAY_SIZE(error_names); i++) {
    error = v7_create_cfunction_ctor(v7, create_object(v7, v7->error_prototype),
                                     Error_ctor, 1);
    v7_set_property(v7, v7->global_object, error_names[i],
                    strlen(error_names[i]), V7_PROPERTY_DONT_ENUM, error);
    v7->error_objects[i] = error;
  }
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


static val_t Number_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_length(v7, args) <= 0 ? v7_create_number(0.0)
                                              : v7_array_get(v7, args, 0);
  val_t res = v7_is_double(arg0) ? arg0 : v7_create_number(i_as_num(v7, arg0));

  if (v7_is_object(this_obj) && this_obj != v7->global_object) {
    v7_to_object(this_obj)->prototype = v7_to_object(v7->number_prototype);
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, res);
    res = this_obj;
  }

  return res;
}

V7_PRIVATE
val_t n_to_str(struct v7 *v7, val_t t, val_t args, const char *format) {
  val_t arg0 = v7_array_get(v7, args, 0);
  double d = i_as_num(v7, arg0);
  int len, digits = d > 0 ? (int) d : 0;
  char fmt[10], buf[100];

  snprintf(fmt, sizeof(fmt), format, digits);
  len = snprintf(buf, sizeof(buf), fmt, v7_to_double(i_value_of(v7, t)));

  return v7_create_string(v7, buf, len, 1);
}

static val_t Number_toFixed(struct v7 *v7, val_t this_obj, val_t args) {
  return n_to_str(v7, this_obj, args, "%%.%dlf");
}

static val_t Number_toExp(struct v7 *v7, val_t this_obj, val_t args) {
  return n_to_str(v7, this_obj, args, "%%.%de");
}

static val_t Number_toPrecision(struct v7 *v7, val_t this_obj, val_t args) {
  return Number_toExp(v7, this_obj, args);
}

static val_t Number_valueOf(struct v7 *v7, val_t this_obj, val_t args) {
  if (!v7_is_double(this_obj) &&
      (v7_is_object(this_obj) &&
       v7_object_to_value(v7_to_object(this_obj)->prototype) !=
           v7->number_prototype)) {
    throw_exception(v7, TYPE_ERROR,
                    "Number.valueOf called on non-number object");
  }
  return Obj_valueOf(v7, this_obj, args);
}

static val_t Number_toString(struct v7 *v7, val_t this_obj, val_t args) {
  char buf[512];
  (void) args;

  if (this_obj == v7->number_prototype) {
    return v7_create_string(v7, "0", 1, 1);
  }

  if (!v7_is_double(this_obj) &&
      !(v7_is_object(this_obj) &&
        is_prototype_of(v7, this_obj, v7->number_prototype))) {
    throw_exception(v7, TYPE_ERROR,
                    "Number.toString called on non-number object");
  }

  /* TODO(mkm) handle radix first arg */
  v7_stringify_value(v7, i_value_of(v7, this_obj), buf, sizeof(buf));
  return v7_create_string(v7, buf, strlen(buf), 1);
}

static val_t n_isNaN(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  (void) this_obj;
  return v7_create_boolean(!v7_is_double(arg0) || arg0 == V7_TAG_NAN);
}

V7_PRIVATE void init_number(struct v7 *v7) {
  unsigned int attrs =
      V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_ENUM | V7_PROPERTY_DONT_DELETE;
  val_t num =
      v7_create_cfunction_ctor(v7, v7->number_prototype, Number_ctor, 1);
  v7_set_property(v7, v7->global_object, "Number", 6, V7_PROPERTY_DONT_ENUM,
                  num);

  set_cfunc_prop(v7, v7->number_prototype, "toFixed", Number_toFixed);
  set_cfunc_prop(v7, v7->number_prototype, "toPrecision", Number_toPrecision);
  set_cfunc_prop(v7, v7->number_prototype, "toExponential", Number_toExp);
  set_cfunc_prop(v7, v7->number_prototype, "valueOf", Number_valueOf);
  set_cfunc_prop(v7, v7->number_prototype, "toString", Number_toString);

  v7_set_property(v7, num, "MAX_VALUE", 9, attrs,
                  v7_create_number(1.7976931348623157e+308));
  v7_set_property(v7, num, "MIN_VALUE", 9, attrs, v7_create_number(5e-324));
  v7_set_property(v7, num, "NEGATIVE_INFINITY", 17, attrs,
                  v7_create_number(-INFINITY));
  v7_set_property(v7, num, "POSITIVE_INFINITY", 17, attrs,
                  v7_create_number(INFINITY));
  v7_set_property(v7, num, "NaN", 3, attrs, V7_TAG_NAN);

  v7_set_property(v7, v7->global_object, "NaN", 3, attrs, V7_TAG_NAN);
  v7_set_property(v7, v7->global_object, "isNaN", 5, V7_PROPERTY_DONT_ENUM,
                  v7_create_cfunction(n_isNaN));
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


static val_t Json_stringify(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  char buf[100], *p = v7_to_json(v7, arg0, buf, sizeof(buf));
  val_t res = v7_create_string(v7, p, strlen(p), 1);
  (void) this_obj;
  if (p != buf) free(p);
  return res;
}

V7_PRIVATE void init_json(struct v7 *v7) {
  val_t o = v7_create_object(v7);
  set_cfunc_obj_prop(v7, o, "stringify", Json_stringify, 1);
  set_cfunc_obj_prop(v7, o, "parse", Std_eval, 1);
  v7_set_property(v7, v7->global_object, "JSON", 4, V7_PROPERTY_DONT_ENUM, o);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


#ifdef V7_EXE

static void show_usage(char *argv[]) {
  fprintf(stderr, "V7 version %s (c) Cesanta Software, built on %s\n",
          V7_VERSION, __DATE__);
  fprintf(stderr, "Usage: %s [OPTIONS] js_file ...\n", argv[0]);
  fprintf(stderr, "%s\n", "OPTIONS:");
  fprintf(stderr, "%s\n", "  -e <expr>  execute expression");
  fprintf(stderr, "%s\n", "  -t         dump generated text AST");
  fprintf(stderr, "%s\n", "  -b         dump generated binary AST");
  exit(EXIT_FAILURE);
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

static void dump_ast(struct v7 *v7, const char *code, int binary) {
  struct ast ast;

  ast_init(&ast, 0);
  if (parse(v7, &ast, code, 1) != V7_OK) {
    fprintf(stderr, "%s\n", "parse error");
  } else if (binary) {
    fwrite(ast.mbuf.buf, ast.mbuf.len, 1, stdout);
  } else {
    ast_dump(stdout, &ast, 0);
  }
  ast_free(&ast);
}

static void print_error(struct v7 *v7, const char *f, val_t e) {
  char buf[512];
  char *s = v7_to_json(v7, e, buf, sizeof(buf));
  fprintf(stderr, "Exec error [%s]: %s\n", f, s);
  if (s != buf) {
    free(s);
  }
}

int main(int argc, char *argv[]) {
  struct v7 *v7 = v7_create();
  int i, show_ast = 0, binary_ast = 0;
  val_t res = v7_create_undefined();

  /* Execute inline code */
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
      if (show_ast) {
        dump_ast(v7, argv[i + 1], binary_ast);
      } else if (v7_exec(v7, &res, argv[i + 1]) != V7_OK) {
        print_error(v7, argv[i + 1], res);
        res = v7_create_undefined();
      }
      i++;
    } else if (strcmp(argv[i], "-t") == 0) {
      show_ast = 1;
    } else if (strcmp(argv[i], "-b") == 0) {
      show_ast = 1;
      binary_ast = 1;
    } else if (strcmp(argv[i], "-h") == 0) {
      show_usage(argv);
    }
  }

  if (argc == 1) {
    show_usage(argv);
  }

  /* Execute files */
  for (; i < argc; i++) {
    if (show_ast) {
      size_t size;
      char *source_code;
      if ((source_code = read_file(argv[i], &size)) == NULL) {
        fprintf(stderr, "Cannot read [%s]\n", argv[i]);
      } else {
        dump_ast(v7, source_code, binary_ast);
        free(source_code);
      }
    } else if (v7_exec_file(v7, &res, argv[i]) != V7_OK) {
      print_error(v7, argv[i], res);
      res = v7_create_undefined();
    }
  }

  if (!show_ast) {
    char buf[2000];
    v7_to_json(v7, res, buf, sizeof(buf));
    printf("%s\n", buf);
  }

  v7_destroy(v7);
  return EXIT_SUCCESS;
}
#endif
/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */


#ifdef __APPLE__
int64_t strtoll(const char *, char **, int);
#elif !defined(_WIN32)
extern long timezone;
#endif

typedef double etime_t; /* double is suitable type for ECMA time */
/* inernally we have to use 64-bit integer for some operations */
typedef int64_t etimeint_t;
#define INVALID_TIME NAN

/*++++++  ECMA date & time helpers ++++++*/

#define msPerDay 86400000
#define HoursPerDay 24
#define MinutesPerHour 60
#define SecondsPerMinute 60
#define SecondsPerHour 3600
#define msPerSecond 1000
#define msPerMinute 60000
#define msPerHour 3600000
#define MonthsInYear 12

static etimeint_t g_gmtoffms; /* timezone offset, ms, no DST */
static const char *g_tzname;  /* current timezone name */

static etimeint_t ecma_Day(etime_t t) {
  return floor(t / msPerDay);
}

/* Leap year formula copied from ECMA 5.1 standart as is */
static int ecma_DaysInYear(int y) {
  if (y % 4 != 0) {
    return 365;
  } else if (y % 4 == 0 && y % 100 != 0) {
    return 366;
  } else if (y % 100 == 0 && y % 400 != 0) {
    return 365;
  } else if (y % 400 == 0) {
    return 366;
  } else {
    return 365;
  }
}

static etimeint_t ecma_DayFromYear(etimeint_t y) {
  return 365 * (y - 1970) + floor((y - 1969) / 4) - floor((y - 1901) / 100) +
         floor((y - 1601) / 400);
}

static etimeint_t ecma_TimeFromYear(etimeint_t y) {
  return msPerDay * ecma_DayFromYear(y);
}

static int ecma_YearFromTime_s(etime_t t) {
  int first = floor((t / msPerDay) / 366) + 1970,
      last = floor((t / msPerDay) / 365) + 1970, middle = 0;

  if (last < first) {
    int temp = first;
    first = last;
    last = temp;
  }

  while (last > first) {
    middle = (last + first) / 2;
    if (ecma_TimeFromYear(middle) > t) {
      last = middle - 1;
    } else {
      if (ecma_TimeFromYear(middle) <= t) {
        if (ecma_TimeFromYear(middle + 1) > t) {
          first = middle;
          break;
        }
        first = middle + 1;
      }
    }
  }

  return first;
}

static int ecma_IsLeapYear(int year) {
  return ecma_DaysInYear(year) == 366;
}

static int ecma_DayWithinYear(etime_t t, int year) {
  return (int) (ecma_Day(t) - ecma_DayFromYear(year));
}

static int *ecma_getfirstdays(int isleap) {
  static int sdays[2][MonthsInYear + 1] = {
      {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
      {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}};

  return sdays[isleap];
}

static int ecma_MonthFromTime(etime_t t, int year) {
  int *days, i;
  etimeint_t dwy = ecma_DayWithinYear(t, year);

  days = ecma_getfirstdays(ecma_IsLeapYear(year));

  for (i = 0; i < MonthsInYear; i++) {
    if (dwy >= days[i] && dwy < days[i + 1]) {
      return i;
    }
  }

  return -1;
}

static int ecma_DateFromTime(etime_t t, int year) {
  int *days, mft = ecma_MonthFromTime(t, year),
             dwy = ecma_DayWithinYear(t, year);

  if (mft > 11) {
    return -1;
  }

  days = ecma_getfirstdays(ecma_IsLeapYear(year));

  return dwy - days[mft] + 1;
}

static int ecma_WeekDay(etime_t t) {
  int ret = (ecma_Day(t) + 4) % 7;
  if (ret < 0) {
    ret += 7;
  }

  return ret;
}

static int ecma_DaylightSavingTA(etime_t t) {
  time_t time = t / 1000;
  /*
   * Win32 doesn't have locatime_r
   * nixes don't have localtime_s
   * as result using localtime
   */
  struct tm tm = *localtime(&time);
  if (tm.tm_isdst > 0) {
    return msPerHour;
  } else {
    return 0;
  }
}

static int ecma_LocalTZA() {
  return (int) -g_gmtoffms;
}

static etimeint_t ecma_LocalTime(etime_t t) {
  return t + ecma_LocalTZA() + ecma_DaylightSavingTA(t);
}

static etimeint_t ecma_UTC(etime_t t) {
  return t - ecma_LocalTZA() - ecma_DaylightSavingTA(t - ecma_LocalTZA());
}

#define DEF_EXCTRACT_TIMEPART(funcname, c1, c2) \
  static int ecma_##funcname(etime_t t) {       \
    int ret = (etimeint_t) floor(t / c1) % c2;  \
    if (ret < 0) {                              \
      ret += c2;                                \
    }                                           \
    return ret;                                 \
  }

DEF_EXCTRACT_TIMEPART(HourFromTime, msPerHour, HoursPerDay)
DEF_EXCTRACT_TIMEPART(MinFromTime, msPerMinute, MinutesPerHour)
DEF_EXCTRACT_TIMEPART(SecFromTime, msPerSecond, SecondsPerMinute)
DEF_EXCTRACT_TIMEPART(msFromTime, 1, msPerSecond)

static etimeint_t ecma_MakeTime(etimeint_t hour, etimeint_t min, etimeint_t sec,
                                etimeint_t ms) {
  return ((hour * MinutesPerHour + min) * SecondsPerMinute + sec) *
             msPerSecond +
         ms;
}

static etimeint_t ecma_MakeDay(int year, int month, int date) {
  int *days;
  etimeint_t yday, mday;

  year += floor(month / 12);
  month = month % 12;
  yday = floor(ecma_TimeFromYear(year) / msPerDay);
  days = ecma_getfirstdays(ecma_IsLeapYear(year));
  mday = days[month];

  return yday + mday + date - 1;
}

static etimeint_t ecma_MakeDate(etimeint_t day, etimeint_t time) {
  return (day * msPerDay + time);
}

/* ECMA alternative to struct tm */
struct timeparts {
  int year;  /* can be negative, up to +-282000 */
  int month; /* 0-11 */
  int day;   /* 1-31 */
  int hour;  /* 0-23 */
  int min;   /* 0-59 */
  int sec;   /* 0-59 */
  int msec;
  int dayofweek; /* 0-6 */
};

/*+++ this functions is used to get current date/time & timezone */

static void d_gettime(etime_t *t) {
  *t = time(NULL);
}

static const char *d_gettzname() {
  return g_tzname;
}

static etime_t d_mktime_impl(const struct timeparts *tp) {
  return ecma_MakeDate(ecma_MakeDay(tp->year, tp->month, tp->day),
                       ecma_MakeTime(tp->hour, tp->min, tp->sec, tp->msec));
}

/*++++ libc mktime alternatives ++++*/

static etime_t d_lmktime(const struct timeparts *tp) {
  return ecma_UTC(d_mktime_impl(tp));
}

static etime_t d_gmktime(const struct timeparts *tp) {
  return d_mktime_impl(tp);
}

typedef etime_t (*fmaketime_t)(const struct timeparts *);

/*++++ libc gmtime & localtime alternatives ++++*/
static void d_gmtime(const etime_t *t, struct timeparts *tp) {
  tp->year = ecma_YearFromTime_s(*t);
  tp->month = ecma_MonthFromTime(*t, tp->year);
  tp->day = ecma_DateFromTime(*t, tp->year);
  tp->hour = ecma_HourFromTime(*t);
  tp->min = ecma_MinFromTime(*t);
  tp->sec = ecma_SecFromTime(*t);
  tp->msec = ecma_msFromTime(*t);
  tp->dayofweek = ecma_WeekDay(*t);
}

static void d_localtime(const etime_t *time, struct timeparts *tp) {
  etime_t local_time = ecma_LocalTime(*time);
  d_gmtime(&local_time, tp);
}

typedef void (*fbreaktime_t)(const etime_t *, struct timeparts *);

/*++++ some isXXX helpers ++++*/
static val_t d_trytogetobjforstring(struct v7 *v7, val_t obj) {
  val_t ret = i_value_of(v7, obj);
  if (ret == V7_TAG_NAN) {
    throw_exception(v7, TYPE_ERROR, "Date is invalid (for string)");
  }
  return ret;
}

static int d_iscalledasfunction(struct v7 *v7, val_t this_obj) {
  /* TODO(alashkin): verify this statement */
  return is_prototype_of(v7, this_obj, v7->date_prototype);
}

/*++++ from/to string helpers ++++*/

static int d_timetoISOstr(const etime_t *time, char *buf, size_t buf_size) {
  /* ISO format: "+XXYYYY-MM-DDTHH:mm:ss.sssZ"; */
  struct timeparts tp;
  char use_ext = 0;
  const char *ey_frm = "%06d-%02d-%02dT%02d:%02d:%02d.%03dZ";
  const char *simpl_frm = "%d-%02d-%02dT%02d:%02d:%02d.%03dZ";

  d_gmtime(time, &tp);

  if (abs(tp.year) > 9999 || tp.year < 0) {
    *buf = (tp.year > 0) ? '+' : '-';
    use_ext = 1;
  }

  return snprintf(buf + use_ext, buf_size - use_ext,
                  use_ext ? ey_frm : simpl_frm, abs(tp.year), tp.month + 1,
                  tp.day, tp.hour, tp.min, tp.sec, tp.msec) +
         use_ext;
}

/* non-locale function should always return in english and 24h-format */
static const char *wday_name[] = {"Sun", "Mon", "Tue", "Wed",
                                  "Thu", "Fri", "Sat"};

static const char *mon_name[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int d_getnumbyname(const char **arr, int arr_size, const char *str) {
  int i;
  for (i = 0; i < arr_size; i++) {
    if (strncmp(arr[i], str, 3) == 0) {
      return i + 1;
    }
  }

  return -1;
}

int date_parse(const char *str, int *a1, int *a2, int *a3, char sep,
               char *rest) {
  char frmDate[] = " %d/%d/%d%[^\0]";
  frmDate[3] = frmDate[6] = sep;
  return sscanf(str, frmDate, a1, a2, a3, rest);
}

#define NO_TZ 0x7FFFFFFF

/*
 * not very smart but simple, and working according
 * to ECMA5.1 StringToDate function
 */
static int d_parsedatestr(const char *jstr, size_t len, struct timeparts *tp,
                          int *tz) {
  char gmt[4];
  char buf1[100] = {0}, buf2[100] = {0};
  int res = 0;
  char str[101];
  memcpy(str, jstr, len);
  str[len] = '\0';
  memset(tp, 0, sizeof(*tp));
  *tz = NO_TZ;

  /* trying toISOSrting() format */
  {
    const char *frmISOString = " %d-%02d-%02dT%02d:%02d:%02d.%03dZ";
    res = sscanf(str, frmISOString, &tp->year, &tp->month, &tp->day, &tp->hour,
                 &tp->min, &tp->sec, &tp->msec);
    if (res == 7) {
      *tz = 0;
      return 1;
    }
  }

  /* trying toString()/toUTCString()/toDateFormat() formats */
  {
    char month[4];
    int dowlen;
    const char *frmString = " %*s%n %03s %02d %d %02d:%02d:%02d %03s%d";
    res = sscanf(str, frmString, &dowlen, month, &tp->day, &tp->year, &tp->hour,
                 &tp->min, &tp->sec, gmt, tz);
    if ((res == 3 || (res >= 6 && res <= 8)) && dowlen == 3) {
      if ((tp->month = d_getnumbyname(mon_name, ARRAY_SIZE(mon_name), month)) !=
          -1) {
        if (res == 7 && strncmp(gmt, "GMT", 3) == 0) {
          *tz = 0;
        }
        return 1;
      }
    }
  }

  /* trying the rest */

  /* trying date */

  if (!(date_parse(str, &tp->month, &tp->day, &tp->year, '/', buf1) >= 3 ||
        date_parse(str, &tp->day, &tp->month, &tp->year, '.', buf1) >= 3 ||
        date_parse(str, &tp->year, &tp->month, &tp->day, '-', buf1) >= 3)) {
    return 0;
  }

  /*  there is date, trying time; from here return 0 only on errors */

  /* trying HH:mm */
  {
    const char *frmMMhh = " %d:%d%[^\0]";
    res = sscanf(buf1, frmMMhh, &tp->hour, &tp->min, buf2);
    /* can't get time, but have some symbols, assuming error */
    if (res < 2) {
      return (strlen(buf2) == 0);
    }
  }

  /* trying seconds */
  {
    const char *frmss = ":%d%[^\0]";
    memset(buf1, 0, sizeof(buf1));
    res = sscanf(buf2, frmss, &tp->sec, buf1);
  }

  /* even if we don't get seconds we gonna try to get tz */
  {
    char *rest = res ? buf1 : buf2;
    char *buf = res ? buf2 : buf1;
    const char *frmtz = " %03s%d%[^\0]";

    res = sscanf(rest, frmtz, gmt, tz, buf);
    if (res == 1 && strncmp(gmt, "GMT", 3) == 0) {
      *tz = 0;
    }
  }

  /* return OK if we are at the end of string */
  return res <= 2;
}

static int d_timeFromString(etime_t *time, const char *str, size_t str_len) {
  struct timeparts tp;
  int tz;

  *time = INVALID_TIME;

  if (str_len > 100) {
    /* too long for valid date string */
    return 0;
  }

  if (d_parsedatestr(str, str_len, &tp, &tz)) {
    /* check results */
    int valid = 0;

    tp.month--;
    valid = tp.day >= 1 && tp.day <= 31;
    valid &= tp.month >= 0 && tp.month <= 11;
    valid &= tp.hour >= 0 && tp.hour <= 23;
    valid &= tp.min >= 0 && tp.min <= 59;
    valid &= tp.sec >= 0 && tp.sec <= 59;

    if (tz != NO_TZ && tz > 12) {
      tz /= 100;
    }

    valid &= (abs(tz) <= 12 || tz == NO_TZ);

    if (valid) {
      *time = d_gmktime(&tp);

      if (tz != NO_TZ) {
        /* timezone specified, use it */
        *time -= (tz * msPerHour);
      } else if (tz != 0) {
        /* assuming local timezone and moving back to UTC */
        *time = ecma_UTC(*time);
      }
    }
  }

  return !isnan(*time);
}

typedef int (*ftostring_t)(const struct timeparts *, char *, int);

static val_t d_tostring(struct v7 *v7, val_t obj, fbreaktime_t breaktimefunc,
                        ftostring_t tostringfunc, int addtz) {
  struct timeparts tp;
  int len;
  char buf[100];
  etime_t time;

  time = v7_to_double(d_trytogetobjforstring(v7, obj));

  breaktimefunc(&time, &tp);
  len = tostringfunc(&tp, buf, addtz);

  return v7_create_string(v7, buf, len, 1);
}

struct d_locale {
  char locale[50];
};

static void d_getcurrentlocale(struct d_locale *loc) {
  strcpy(setlocale(LC_TIME, 0), loc->locale);
}

static void d_setlocale(const struct d_locale *loc) {
  setlocale(LC_TIME, loc ? loc->locale : "");
}

/* TODO(alashkin): check portability */
static val_t d_tolocalestr(struct v7 *v7, val_t obj, const char *frm) {
  char buf[250];
  size_t len;
  struct tm t;
  etime_t time;
  struct d_locale prev_locale;
  struct timeparts tp;

  time = v7_to_double(d_trytogetobjforstring(v7, obj));

  d_getcurrentlocale(&prev_locale);
  d_setlocale(0);
  d_localtime(&time, &tp);

  memset(&t, 0, sizeof(t));
  t.tm_year = tp.year - 1900;
  t.tm_mon = tp.month;
  t.tm_mday = tp.day;
  t.tm_hour = tp.hour;
  t.tm_min = tp.min;
  t.tm_sec = tp.sec;
  t.tm_wday = tp.dayofweek;

  len = strftime(buf, sizeof(buf), frm, &t);

  d_setlocale(&prev_locale);

  return v7_create_string(v7, buf, len, 1);
}

static int d_tptodatestr(const struct timeparts *tp, char *buf, int addtz) {
  (void) addtz;

  return sprintf(buf, "%s %s %02d %d", wday_name[tp->dayofweek],
                 mon_name[tp->month], tp->day, tp->year);
}

static int d_tptotimestr(const struct timeparts *tp, char *buf, int addtz) {
  int len;

  len = sprintf(buf, "%02d:%02d:%02d GMT", tp->hour, tp->min, tp->sec);

  if (addtz && g_gmtoffms != 0) {
    len = sprintf(buf + len, "%c%02d00 (%s)", g_gmtoffms > 0 ? '-' : '+',
                  abs((int) g_gmtoffms / msPerHour), d_gettzname());
  }

  return (int) strlen(buf);
}

static int d_tptostr(const struct timeparts *tp, char *buf, int addtz) {
  int len = d_tptodatestr(tp, buf, addtz);
  *(buf + len) = ' ';
  return d_tptotimestr(tp, buf + len + 1, addtz) + len + 1;
}

/*+++ setXXXX & getXXXX helpers ++++*/
static struct timeparts *d_getTP(val_t val, struct timeparts *tp,
                                 fbreaktime_t breaktimefunc) {
  etime_t time;
  time = v7_to_double(val);
  breaktimefunc(&time, tp);
  return tp;
}

/* notice: holding month in calendar format (1-12, not 0-11) */
struct dtimepartsarr {
  etime_t args[7];
};

enum detimepartsarr {
  tpyear = 0,
  tpmonth,
  tpdate,
  tphours,
  tpminutes,
  tpseconds,
  tpmsec,
  tpmax
};

static etime_t d_changepartoftime(const etime_t *current,
                                  struct dtimepartsarr *a,
                                  fbreaktime_t breaktimefunc,
                                  fmaketime_t maketimefunc) {
  /*
   * 0 = year, 1 = month , 2 = date , 3 = hours,
   * 4 = minutes, 5 = seconds, 6 = ms
   */
  struct timeparts tp;
  unsigned long i;
  int *tp_arr[7];
  /*
   * C89 doesn't allow initialization
   * like x = {&tp.year, &tp.month, .... }
   */
  tp_arr[0] = &tp.year;
  tp_arr[1] = &tp.month;
  tp_arr[2] = &tp.day;
  tp_arr[3] = &tp.hour;
  tp_arr[4] = &tp.min;
  tp_arr[5] = &tp.sec;
  tp_arr[6] = &tp.msec;

  memset(&tp, 0, sizeof(tp));

  if (breaktimefunc != NULL) {
    breaktimefunc(current, &tp);
  }

  for (i = 0; i < ARRAY_SIZE(tp_arr); i++) {
    if (!isnan(a->args[i]) && !isinf(a->args[i])) {
      *tp_arr[i] = (int) a->args[i];
    }
  }

  return maketimefunc(&tp);
}

static etime_t d_time_number_from_arr(struct v7 *v7, val_t this_obj, val_t args,
                                      int start_pos, fbreaktime_t breaktimefunc,
                                      fmaketime_t makefilefunc) {
  etime_t ret_time = INVALID_TIME;
  long cargs;

  val_t objtime = i_value_of(v7, this_obj);

  if ((cargs = v7_array_length(v7, args)) >= 1 && objtime != V7_TAG_NAN) {
    int i;
    struct dtimepartsarr a = {{INVALID_TIME, INVALID_TIME, INVALID_TIME,
                               INVALID_TIME, INVALID_TIME, INVALID_TIME,
                               INVALID_TIME}};
    etime_t new_part = INVALID_TIME;

    for (i = 0; i < cargs && (i + start_pos < tpmax); i++) {
      new_part = i_as_num(v7, v7_array_get(v7, args, i));
      if (isnan(new_part)) {
        break;
      }

      a.args[i + start_pos] = new_part;
    }

    if (!isnan(new_part)) {
      etime_t current_time = v7_to_double(objtime);
      ret_time =
          d_changepartoftime(&current_time, &a, breaktimefunc, makefilefunc);
    }
  }

  return ret_time;
}

static val_t d_setTimePart(struct v7 *v7, val_t this_obj, val_t args,
                           int start_pos, fbreaktime_t breaktimefunc,
                           fmaketime_t makefilefunc) {
  val_t n;
  etime_t ret_time = d_time_number_from_arr(v7, this_obj, args, start_pos,
                                            breaktimefunc, makefilefunc);

  n = v7_create_number(ret_time);
  v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, n);

  return n;
}

/*++++++++++++++ API +++++++++++++++*/

/* constructor */
static val_t Date_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  etime_t ret_time = INVALID_TIME;
  if (v7_is_object(this_obj) && this_obj != v7->global_object) {
    long cargs = v7_array_length(v7, args);
    if (cargs <= 0) {
      /* no parameters - return current date & time */
      d_gettime(&ret_time);
    } else if (cargs == 1) {
      /* one parameter */
      val_t arg = v7_array_get(v7, args, 0);
      if (v7_is_string(arg)) { /* it could be string */
        size_t str_size;
        const char *str = v7_to_string(v7, &arg, &str_size);
        d_timeFromString(&ret_time, str, str_size);
      }
      if (isnan(ret_time)) {
        /*
         * if cannot be parsed or
         * not string at all - trying to convert to number
         */
        ret_time = i_as_num(v7, arg);
      }
    } else {
      /* 2+ paramaters - should be parts of a date */
      struct dtimepartsarr a;
      int i;

      memset(&a, 0, sizeof(a));

      for (i = 0; i < cargs; i++) {
        a.args[i] = i_as_num(v7, v7_array_get(v7, args, i));
        if (isnan(a.args[i])) {
          break;
        }
      }

      if (i >= cargs) {
        /*
         * If date is supplied then let
         * dt be ToNumber(date); else let dt be 1.
         */
        if (a.args[tpdate] == 0) {
          a.args[tpdate] = 1;
        }

        if (a.args[tpyear] >= 0 && a.args[tpyear] <= 99) {
          /*
           * If y is not NaN and 0 <= ToInteger(y) <= 99,
           * then let yr be 1900+ToInteger(y); otherwise, let yr be y.
           */
          a.args[tpyear] += 1900;
        }

        ret_time = ecma_UTC(d_changepartoftime(0, &a, 0, d_gmktime));
      }
    }

    v7_to_object(this_obj)->prototype = v7_to_object(v7->date_prototype);
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN,
                    v7_create_number(ret_time));
    return this_obj;
  } else {
    /*
     * according to 15.9.2.1 we should ignore all
     * parameters in case of function-call
     */
    struct timeparts tp;
    char buf[50];
    int len;

    d_gettime(&ret_time);
    d_localtime(&ret_time, &tp);
    len = d_tptostr(&tp, buf, 1);

    return v7_create_string(v7, buf, len, 1);
  }
}

/*++++ toXXXString functions ++++*/
static val_t Date_toISOString(struct v7 *v7, val_t this_obj, val_t args) {
  char buf[30];
  etime_t time;
  int len;
  (void) args;

  time = v7_to_double(d_trytogetobjforstring(v7, this_obj));
  len = d_timetoISOstr(&time, buf, sizeof(buf));

  return v7_create_string(v7, buf, len, 1);
}

/* using macros to avoid copy-paste technic */
#define DEF_TOSTR(funcname, breaktimefunc, tostrfunc, addtz)                  \
  static val_t Date_to##funcname(struct v7 *v7, val_t this_obj, val_t args) { \
    (void) args;                                                              \
    return d_tostring(v7, this_obj, breaktimefunc, tostrfunc, addtz);         \
  }

DEF_TOSTR(UTCString, d_gmtime, d_tptostr, 0)
DEF_TOSTR(String, d_localtime, d_tptostr, 1)
DEF_TOSTR(DateString, d_localtime, d_tptodatestr, 1)
DEF_TOSTR(TimeString, d_localtime, d_tptotimestr, 1)

#define DEF_TOLOCALESTR(funcname, frm)                                        \
  static val_t Date_to##funcname(struct v7 *v7, val_t this_obj, val_t args) { \
    (void) args;                                                              \
    return d_tolocalestr(v7, this_obj, frm);                                  \
  }

DEF_TOLOCALESTR(LocaleString, "%c")
DEF_TOLOCALESTR(LocaleDateString, "%x")
DEF_TOLOCALESTR(LocaleTimeString, "%X")

/*++++ get functions ++++*/

#define DEF_GET_TP_FUNC(funcName, tpmember, breaktimefunc)                     \
  static val_t Date_get##funcName(struct v7 *v7, val_t this_obj, val_t args) { \
    struct timeparts tp;                                                       \
    val_t v = i_value_of(v7, this_obj);                                        \
    (void) args;                                                               \
    return v7_create_number(                                                   \
        v == V7_TAG_NAN ? NAN : d_getTP(v, &tp, breaktimefunc)->tpmember);     \
  }

#define DEF_GET_TP(funcName, tpmember)               \
  DEF_GET_TP_FUNC(UTC##funcName, tpmember, d_gmtime) \
  DEF_GET_TP_FUNC(funcName, tpmember, d_localtime)

DEF_GET_TP(Date, day)
DEF_GET_TP(FullYear, year)
DEF_GET_TP(Month, month)
DEF_GET_TP(Hours, hour)
DEF_GET_TP(Minutes, min)
DEF_GET_TP(Seconds, sec)
DEF_GET_TP(Milliseconds, msec)
DEF_GET_TP(Day, dayofweek)

/*++++ set functions ++++*/

static val_t Date_setTime(struct v7 *v7, val_t this_obj, val_t args) {
  etime_t ret_time = INVALID_TIME;
  val_t n;
  if (v7_array_length(v7, args) >= 1) {
    ret_time = i_as_num(v7, v7_array_get(v7, args, 0));
  }

  n = v7_create_number(ret_time);
  v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, n);
  return n;
}

#define DEF_SET_TP(name, start_pos)                                           \
  static val_t Date_setUTC##name(struct v7 *v7, val_t this_obj, val_t args) { \
    return d_setTimePart(v7, this_obj, args, start_pos, d_gmtime, d_gmktime); \
  }                                                                           \
  static val_t Date_set##name(struct v7 *v7, val_t this_obj, val_t args) {    \
    return d_setTimePart(v7, this_obj, args, start_pos, d_localtime,          \
                         d_lmktime);                                          \
  }

DEF_SET_TP(Milliseconds, tpmsec)
DEF_SET_TP(Seconds, tpseconds)
DEF_SET_TP(Minutes, tpminutes)
DEF_SET_TP(Hours, tphours)
DEF_SET_TP(Date, tpdate)
DEF_SET_TP(Month, tpmonth)
DEF_SET_TP(FullYear, tpyear)

/*++++ other API ++++*/

static val_t Date_toJSON(struct v7 *v7, val_t this_obj, val_t args) {
  return Date_toISOString(v7, this_obj, args);
}

static val_t Date_valueOf(struct v7 *v7, val_t this_obj, val_t args) {
  (void) args;
  if (!v7_is_object(this_obj) ||
      (v7_is_object(this_obj) &&
       v7_to_object(this_obj)->prototype != v7_to_object(v7->date_prototype))) {
    throw_exception(v7, TYPE_ERROR, "Date.valueOf called on non-Date object");
  }

  return Obj_valueOf(v7, this_obj, args);
}

static val_t Date_getTime(struct v7 *v7, val_t this_obj, val_t args) {
  return Date_valueOf(v7, this_obj, args);
}

static val_t Date_getTimezoneOffset(struct v7 *v7, val_t this_obj, val_t args) {
  (void) args;
  (void) v7;
  (void) this_obj;
  return v7_create_number(g_gmtoffms / msPerMinute);
}

static val_t Date_now(struct v7 *v7, val_t this_obj, val_t args) {
  etime_t ret_time;
  (void) args;
  (void) v7;
  (void) this_obj;

  d_gettime(&ret_time);

  return v7_create_number(ret_time);
}

static val_t Date_parse(struct v7 *v7, val_t this_obj, val_t args) {
  etime_t ret_time = INVALID_TIME;
  (void) args;

  if (!d_iscalledasfunction(v7, this_obj)) {
    throw_exception(v7, TYPE_ERROR, "Date.parse() called on object");
  }

  if (v7_array_length(v7, args) >= 1) {
    val_t arg0 = v7_array_get(v7, args, 0);
    if (v7_is_string(arg0)) {
      size_t size;
      const char *time_str = v7_to_string(v7, &arg0, &size);

      d_timeFromString(&ret_time, time_str, size);
    }
  }

  return v7_create_number(ret_time);
}

static val_t Date_UTC(struct v7 *v7, val_t this_obj, val_t args) {
  etime_t ret_time;
  (void) args;

  if (!d_iscalledasfunction(v7, this_obj)) {
    throw_exception(v7, TYPE_ERROR, "Date.now() called on object");
  }

  ret_time = d_time_number_from_arr(v7, this_obj, args, tpyear, 0, d_gmktime);
  return v7_create_number(ret_time);
}

/****** Initialization *******/

/*
 * We should set V7_PROPERTY_DONT_ENUM for all Date props
 * TODO(mkm): check other objects
*/
static int d_set_cfunc_prop(struct v7 *v7, val_t o, const char *name,
                            v7_cfunction_t f) {
  return v7_set_property(v7, o, name, strlen(name), V7_PROPERTY_DONT_ENUM,
                         v7_create_cfunction(f));
}

#define DECLARE_GET(func)                                                      \
  d_set_cfunc_prop(v7, v7->date_prototype, "getUTC" #func, Date_getUTC##func); \
  d_set_cfunc_prop(v7, v7->date_prototype, "get" #func, Date_get##func);

#define DECLARE_GET_AND_SET(func)                                              \
  DECLARE_GET(func)                                                            \
  d_set_cfunc_prop(v7, v7->date_prototype, "setUTC" #func, Date_setUTC##func); \
  d_set_cfunc_prop(v7, v7->date_prototype, "set" #func, Date_set##func);

V7_PRIVATE void init_date(struct v7 *v7) {
  val_t date = v7_create_cfunction_ctor(v7, v7->date_prototype, Date_ctor, 7);
  v7_set_property(v7, v7->global_object, "Date", 4, V7_PROPERTY_DONT_ENUM,
                  date);

  DECLARE_GET_AND_SET(Date);
  DECLARE_GET_AND_SET(FullYear);
  DECLARE_GET_AND_SET(Month);
  DECLARE_GET_AND_SET(Hours);
  DECLARE_GET_AND_SET(Minutes);
  DECLARE_GET_AND_SET(Seconds);
  DECLARE_GET_AND_SET(Milliseconds);
  DECLARE_GET(Day);

  d_set_cfunc_prop(v7, date, "now", Date_now);
  d_set_cfunc_prop(v7, date, "parse", Date_parse);
  d_set_cfunc_prop(v7, date, "UTC", Date_UTC);

  d_set_cfunc_prop(v7, v7->date_prototype, "getTimezoneOffset",
                   Date_getTimezoneOffset);

  d_set_cfunc_prop(v7, v7->date_prototype, "getTime", Date_getTime);
  d_set_cfunc_prop(v7, v7->date_prototype, "toISOString", Date_toISOString);
  d_set_cfunc_prop(v7, v7->date_prototype, "valueOf", Date_valueOf);

  d_set_cfunc_prop(v7, v7->date_prototype, "setTime", Date_setTime);
  d_set_cfunc_prop(v7, v7->date_prototype, "toString", Date_toString);
  d_set_cfunc_prop(v7, v7->date_prototype, "toDateString", Date_toDateString);
  d_set_cfunc_prop(v7, v7->date_prototype, "toTimeString", Date_toTimeString);
  d_set_cfunc_prop(v7, v7->date_prototype, "toUTCString", Date_toUTCString);
  d_set_cfunc_prop(v7, v7->date_prototype, "toLocaleString",
                   Date_toLocaleString);
  d_set_cfunc_prop(v7, v7->date_prototype, "toLocaleDateString",
                   Date_toLocaleDateString);
  d_set_cfunc_prop(v7, v7->date_prototype, "toLocaleTimeString",
                   Date_toLocaleTimeString);
  d_set_cfunc_prop(v7, v7->date_prototype, "toJSON", Date_toJSON);

  /*
   * GTM offset without DST
   * TODO(alashkin): check this
   * Could be changed to tm::tm_gmtoff,
   * but tm_gmtoff includes DST, so
   * side effects are possible
   */
  tzset();
  g_gmtoffms = timezone * msPerSecond;
  /*
   * tzname could be changed by localtime_r call,
   * so we have to store pointer
   * TODO(alashkin): need restart on tz change???
   */
  g_tzname = tzname[0];
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


static val_t Function_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  long i, n = 0, num_args = v7_array_length(v7, args);
  size_t size;
  char buf[200];
  const char *s;
  val_t param, body, res = v7_create_undefined();

  (void) this_obj;

  if (num_args <= 0) return res;

  /* TODO(lsm): Constructing function source code here. Optimize this. */
  n += snprintf(buf + n, sizeof(buf) - n, "%s", "(function(");

  for (i = 0; i < num_args - 1; i++) {
    param = i_value_of(v7, v7_array_get(v7, args, i));
    if (v7_is_string(param)) {
      s = v7_to_string(v7, &param, &size);
      if (i > 0) {
        n += snprintf(buf + n, sizeof(buf) - n, "%s", ",");
      }
      n += snprintf(buf + n, sizeof(buf) - n, "%.*s", (int) size, s);
    }
  }
  n += snprintf(buf + n, sizeof(buf) - n, "%s", "){");
  body = i_value_of(v7, v7_array_get(v7, args, num_args - 1));
  if (v7_is_string(body)) {
    s = v7_to_string(v7, &body, &size);
    n += snprintf(buf + n, sizeof(buf) - n, "%.*s", (int) size, s);
  }
  n += snprintf(buf + n, sizeof(buf) - n, "%s", "})");

  if (v7_exec_with(v7, &res, buf, V7_UNDEFINED) != V7_OK) {
    throw_exception(v7, SYNTAX_ERROR, "Invalid function body");
  }

  return res;
}

static val_t Function_length(struct v7 *v7, val_t this_obj, val_t args) {
  struct v7_function *func = v7_to_function(this_obj);
  ast_off_t body, pos = func->ast_off;
  struct ast *a = func->ast;
  int argn = 0;

  (void) args;

  V7_CHECK(v7, ast_fetch_tag(a, &pos) == AST_FUNC);
  body = ast_get_skip(a, pos, AST_FUNC_BODY_SKIP);

  ast_move_to_children(a, &pos);
  if (ast_fetch_tag(a, &pos) == AST_IDENT) {
    ast_move_to_children(a, &pos);
  }
  while (pos < body) {
    V7_CHECK(v7, ast_fetch_tag(a, &pos) == AST_IDENT);
    ast_move_to_children(a, &pos);
    argn++;
  }

  return v7_create_number(argn);
}

static val_t Function_apply(struct v7 *v7, val_t this_obj, val_t args) {
  val_t f = i_value_of(v7, this_obj);
  val_t this_arg = v7_array_get(v7, args, 0);
  val_t func_args = v7_array_get(v7, args, 1);
  return v7_apply(v7, f, this_arg, func_args);
}

V7_PRIVATE void init_function(struct v7 *v7) {
  val_t ctor = v7_create_cfunction_object(v7, Function_ctor, 1);
  v7_set_property(v7, ctor, "prototype", 9, 0, v7->function_prototype);
  v7_set_property(v7, v7->global_object, "Function", 8, 0, ctor);
  set_cfunc_obj_prop(v7, v7->function_prototype, "apply", Function_apply, 1);
  v7_set_property(v7, v7->function_prototype, "length", 6, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Function_length));
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE v7_val_t Std_print(struct v7 *v7, val_t this_obj, val_t args) {
  char *p, buf[1024];
  int i, num_args = v7_array_length(v7, args);

  (void) this_obj;
  for (i = 0; i < num_args; i++) {
    p = v7_to_json(v7, v7_array_get(v7, args, i), buf, sizeof(buf));
    printf("%s", p);
    if (p != buf) {
      free(p);
    }
  }
  putchar('\n');

  return v7_create_null();
}

V7_PRIVATE val_t Std_eval(struct v7 *v7, val_t t, val_t args) {
  val_t res = v7_create_undefined(), arg = v7_array_get(v7, args, 0);
  (void) t;
  if (arg != V7_UNDEFINED) {
    char buf[100], *p;
    p = v7_to_json(v7, arg, buf, sizeof(buf));
    if (p[0] == '"') {
      p[0] = p[strlen(p) - 1] = ' ';
    }
    if (v7_exec(v7, &res, p) != V7_OK) {
      throw_value(v7, res);
    }
    if (p != buf) {
      free(p);
    }
  }
  return res;
}

static val_t Std_exit(struct v7 *v7, val_t t, val_t args) {
  int exit_code = arg_long(v7, args, 0, 0);
  (void) t;
  exit(exit_code);
  return v7_create_undefined();
}

static void base64_encode(const unsigned char *src, int src_len, char *dst) {
  V7_PRIVATE const char *b64 =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int i, j, a, b, c;

  for (i = j = 0; i < src_len; i += 3) {
    a = src[i];
    b = i + 1 >= src_len ? 0 : src[i + 1];
    c = i + 2 >= src_len ? 0 : src[i + 2];

    dst[j++] = b64[a >> 2];
    dst[j++] = b64[((a & 3) << 4) | (b >> 4)];
    if (i + 1 < src_len) {
      dst[j++] = b64[(b & 15) << 2 | (c >> 6)];
    }
    if (i + 2 < src_len) {
      dst[j++] = b64[c & 63];
    }
  }
  while (j % 4 != 0) {
    dst[j++] = '=';
  }
  dst[j++] = '\0';
}

/* Convert one byte of encoded base64 input stream to 6-bit chunk */
static unsigned char from_b64(unsigned char ch) {
  /* Inverse lookup map */
  V7_PRIVATE const unsigned char tab[128] = {
      255, 255, 255, 255, 255, 255, 255, 255, /* 0 */
      255, 255, 255, 255, 255, 255, 255, 255, /* 8 */
      255, 255, 255, 255, 255, 255, 255, 255, /* 16 */
      255, 255, 255, 255, 255, 255, 255, 255, /* 24 */
      255, 255, 255, 255, 255, 255, 255, 255, /* 32 */
      255, 255, 255, 62,  255, 255, 255, 63,  /* 40 */
      52,  53,  54,  55,  56,  57,  58,  59,  /* 48 */
      60,  61,  255, 255, 255, 200, 255, 255, /* 56 '=' is 200, on index 61 */
      255, 0,   1,   2,   3,   4,   5,   6,   /* 64 */
      7,   8,   9,   10,  11,  12,  13,  14,  /* 72 */
      15,  16,  17,  18,  19,  20,  21,  22,  /* 80 */
      23,  24,  25,  255, 255, 255, 255, 255, /* 88 */
      255, 26,  27,  28,  29,  30,  31,  32,  /* 96 */
      33,  34,  35,  36,  37,  38,  39,  40,  /* 104 */
      41,  42,  43,  44,  45,  46,  47,  48,  /* 112 */
      49,  50,  51,  255, 255, 255, 255, 255, /* 120 */
  };
  return tab[ch & 127];
}

static void base64_decode(const unsigned char *s, int len, char *dst) {
  unsigned char a, b, c, d;
  while (len >= 4 && (a = from_b64(s[0])) != 255 &&
         (b = from_b64(s[1])) != 255 && (c = from_b64(s[2])) != 255 &&
         (d = from_b64(s[3])) != 255) {
    if (a == 200 || b == 200) break; /* '=' can't be there */
    *dst++ = a << 2 | b >> 4;
    if (c == 200) break;
    *dst++ = b << 4 | c >> 2;
    if (d == 200) break;
    *dst++ = c << 6 | d;
    s += 4;
    len -= 4;
  }
  *dst = 0;
}

static val_t b64_transform(struct v7 *v7, val_t this_obj, val_t args,
                           void(func)(const unsigned char *, int, char *),
                           double mult) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t res = v7_create_undefined();

  (void) this_obj;
  if (v7_is_string(arg0)) {
    size_t n;
    const char *s = v7_to_string(v7, &arg0, &n);
    char *buf = (char *) malloc(n * mult + 2);
    if (buf != NULL) {
      func((const unsigned char *) s, (int) n, buf);
      res = v7_create_string(v7, buf, strlen(buf), 1);
      free(buf);
    }
  }

  return res;
}

static val_t Std_base64_decode(struct v7 *v7, val_t this_obj, val_t args) {
  return b64_transform(v7, this_obj, args, base64_decode, 0.75);
}

static val_t Std_base64_encode(struct v7 *v7, val_t this_obj, val_t args) {
  return b64_transform(v7, this_obj, args, base64_encode, 1.5);
}

#ifndef V7_NO_FS
static val_t Std_load(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t res = v7_create_undefined();

  (void) this_obj;
  if (v7_is_string(arg0)) {
    size_t n;
    const char *s = v7_to_string(v7, &arg0, &n);
    v7_exec_file(v7, &res, s);
  }

  return res;
}

/*
 * File interface: a wrapper around open(), close(), read(), write().
 * File.open(path, flags) -> fd.
 * File.close(fd) -> undefined
 * File.read(fd) -> string (empty string on EOF)
 * File.write(fd, str) -> num_bytes_written
 */
static val_t Std_read(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  char buf[2048];
  size_t n;

  (void) this_obj;
  if (v7_is_double(arg0)) {
    int fd = v7_to_double(arg0);
    n = read(fd, buf, sizeof(buf));
    if (n > 0) {
      return v7_create_string(v7, buf, n, 1);
    }
  }

  return v7_create_string(v7, "", 0, 1);
}

static val_t Std_write(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t arg1 = v7_array_get(v7, args, 1);
  size_t n = 0, n2;

  (void) this_obj;
  if (v7_is_double(arg0) && v7_is_string(arg1)) {
    const char *s = v7_to_string(v7, &arg1, &n2);
    int fd = v7_to_double(arg0);
    n = write(fd, s, n2);
  }

  return v7_create_number(n);
}

static val_t Std_close(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  (void) this_obj;
  if (v7_is_double(arg0)) {
    close((int) v7_to_double(arg0));
  }
  return v7_create_undefined();
}

static val_t Std_open(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t arg1 = v7_array_get(v7, args, 1);
  val_t res = v7_create_undefined();

  (void) this_obj;
  if (v7_is_string(arg0)) {
    size_t n1;
    const char *s = v7_to_string(v7, &arg0, &n1);
    int flags = v7_is_double(arg1) ? (int) v7_to_double(arg1) : 0;
    int fd = open(s, flags);
    res = v7_create_number(fd);
  }

  return res;
}
#endif

V7_PRIVATE void init_stdlib(struct v7 *v7) {
  /*
   * Ensure the first call to v7_create_value will use a null proto:
   * {}.__proto__.__proto__ == null
   */
  v7->object_prototype = create_object(v7, V7_NULL);
  v7->array_prototype = v7_create_object(v7);
  v7->boolean_prototype = v7_create_object(v7);
  v7->string_prototype = v7_create_object(v7);
  v7->regexp_prototype = v7_create_object(v7);
  v7->number_prototype = v7_create_object(v7);
  v7->error_prototype = v7_create_object(v7);
  v7->global_object = v7_create_object(v7);
  v7->this_object = v7->global_object;
  v7->date_prototype = v7_create_object(v7);
  v7->function_prototype = v7_create_object(v7);
#ifndef V7_DISABLE_SOCKETS
  v7->socket_prototype = v7_create_object(v7);
#endif

  set_cfunc_prop(v7, v7->global_object, "print", Std_print);
  set_cfunc_prop(v7, v7->global_object, "eval", Std_eval);
  set_cfunc_prop(v7, v7->global_object, "exit", Std_exit);
  set_cfunc_prop(v7, v7->global_object, "base64_encode", Std_base64_encode);
  set_cfunc_prop(v7, v7->global_object, "base64_decode", Std_base64_decode);

#ifndef V7_NO_FS
  /* TODO(lsm): move to a File object */
  set_cfunc_prop(v7, v7->global_object, "load", Std_load);
  {
    val_t file_obj = v7_create_object(v7);
    v7_set_property(v7, v7->global_object, "File", 4, 0, file_obj);
    set_cfunc_obj_prop(v7, file_obj, "open", Std_open, 2);
    set_cfunc_obj_prop(v7, file_obj, "close", Std_close, 1);
    set_cfunc_obj_prop(v7, file_obj, "read", Std_read, 0);
    set_cfunc_obj_prop(v7, file_obj, "write", Std_write, 1);
  }
#endif

  v7_set_property(v7, v7->global_object, "Infinity", 8, 0,
                  v7_create_number(INFINITY));
  v7_set_property(v7, v7->global_object, "global", 6, 0, v7->global_object);

  init_object(v7);
  init_array(v7);
  init_error(v7);
  init_boolean(v7);
  init_math(v7);
  init_string(v7);
  init_regex(v7);
  init_number(v7);
  init_json(v7);
  init_date(v7);
#ifndef V7_DISABLE_SOCKETS
  init_socket(v7);
#endif
  init_function(v7);
  init_js_stdlib(v7);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


#define STRINGIFY(x) #x

V7_PRIVATE void init_js_stdlib(struct v7 *v7) {
  val_t res;

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.indexOf = function(a, x) {
      var i; var r = -1; var b = +x;
      if (!b || b < 0) b = 0;
      for (i in this) if (i >= b && (r < 0 || i < r) && this[i] === a) r = +i;
      return r;
    };));

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.lastIndexOf = function(a, x) {
      var i; var r = -1; var b = +x;
      if (isNaN(b) || b < 0 || b >= this.length) b = this.length - 1;
      for (i in this) if (i <= b && (r < 0 || i > r) && this[i] === a) r = +i;
      return r;
    };));

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.reduce = function(a, b) {
      var f = 0;
      if (typeof(a) != 'function') {
        throw new TypeError(a + ' is not a function');
      }
      for (var k in this) {
        if (f == 0 && b === undefined) {
          b = this[k];
          f = 1;
        } else {
          b = a(b, this[k], k, this);
        }
      }
      return b;
    };));

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.pop = function() {
      var i = this.length - 1;
      return this.splice(i, 1)[0];
    };));

  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.shift = function() {
      return this.splice(0, 1)[0];
    };));

  v7_exec(v7, &res, STRINGIFY(
    Function.prototype.call = function() {
      var t = arguments.splice(0, 1)[0];
      return this.apply(t, arguments);
    };));

  /* TODO(lsm): re-enable in a separate PR */
#if 0
  v7_exec(v7, &res, STRINGIFY(
    Array.prototype.unshift = function() {
      var a = new Array(0, 0);
      Array.prototype.push.apply(a, arguments);
      Array.prototype.splice.apply(this, a);
      return this.length;
    };));
#endif
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE val_t to_string(struct v7 *, val_t);

V7_PRIVATE val_t Regex_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  long argnum = v7_array_length(v7, args);
  if (argnum > 0) {
    val_t ro = to_string(v7, v7_array_get(v7, args, 0));
    size_t re_len, flags_len = 0;
    const char *re = v7_to_string(v7, &ro, &re_len), *flags = NULL;
    struct slre_prog *p = NULL;
    struct v7_regexp *rp;

    (void) this_obj;
    if (argnum > 1) {
      val_t fl = to_string(v7, v7_array_get(v7, args, 1));
      flags = v7_to_string(v7, &fl, &flags_len);
    }
    if (slre_compile(re, re_len, flags, flags_len, &p, 1) != SLRE_OK ||
        p == NULL) {
      throw_exception(v7, TYPE_ERROR, "Invalid regex");
      return v7_create_undefined();
    } else {
      rp = (struct v7_regexp *) malloc(sizeof(*rp));
      rp->regexp_string = v7_create_string(v7, re, re_len, 1);
      rp->compiled_regexp = p;
      rp->lastIndex = 0;

      return v7_pointer_to_value(rp) | V7_TAG_REGEXP;
    }
  }
  return v7_create_regexp(v7, "(?:)", 4, NULL, 0);
}

static val_t Regex_global(struct v7 *v7, val_t this_obj, val_t args) {
  int flags = 0;
  val_t r = i_value_of(v7, this_obj);

  (void) args;
  if (v7_is_regexp(r)) flags = slre_get_flags(v7_to_regexp(r)->compiled_regexp);

  return v7_create_boolean(flags & SLRE_FLAG_G);
}

static val_t Regex_ignoreCase(struct v7 *v7, val_t this_obj, val_t args) {
  int flags = 0;
  val_t r = i_value_of(v7, this_obj);

  (void) args;
  if (v7_is_regexp(r)) flags = slre_get_flags(v7_to_regexp(r)->compiled_regexp);

  return v7_create_boolean(flags & SLRE_FLAG_I);
}

static val_t Regex_multiline(struct v7 *v7, val_t this_obj, val_t args) {
  int flags = 0;
  val_t r = i_value_of(v7, this_obj);

  (void) args;
  if (v7_is_regexp(r)) flags = slre_get_flags(v7_to_regexp(r)->compiled_regexp);

  return v7_create_boolean(flags & SLRE_FLAG_M);
}

static val_t Regex_source(struct v7 *v7, val_t this_obj, val_t args) {
  val_t r = i_value_of(v7, this_obj);
  const char *buf = 0;
  size_t len = 0;

  (void) args;
  if (v7_is_regexp(r))
    buf = v7_to_string(v7, &v7_to_regexp(r)->regexp_string, &len);

  return v7_create_string(v7, buf, len, 1);
}

static val_t Regex_get_lastIndex(struct v7 *v7, val_t this_obj, val_t args) {
  long lastIndex = 0;

  (void) v7;
  (void) args;
  if (v7_is_regexp(this_obj)) lastIndex = v7_to_regexp(this_obj)->lastIndex;

  return v7_create_number(lastIndex);
}

static val_t Regex_set_lastIndex(struct v7 *v7, val_t this_obj, val_t args) {
  long lastIndex = 0;

  if (v7_is_regexp(this_obj))
    v7_to_regexp(this_obj)->lastIndex = lastIndex = arg_long(v7, args, 0, 0);

  return v7_create_number(lastIndex);
}

V7_PRIVATE val_t rx_exec(struct v7 *v7, val_t rx, val_t str, int lind) {
  if (v7_is_regexp(rx)) {
    val_t s = to_string(v7, str);
    size_t len;
    struct slre_loot sub;
    struct slre_cap *ptok = sub.caps;
    char *const str = (char *) v7_to_string(v7, &s, &len);
    const char *const end = str + len;
    const char *begin = str;
    struct v7_regexp *rp = v7_to_regexp(rx);
    int flag_g = slre_get_flags(rp->compiled_regexp) & SLRE_FLAG_G;
    if (rp->lastIndex < 0) rp->lastIndex = 0;
    if (flag_g || lind) begin = utfnshift(str, rp->lastIndex);

    if (!slre_exec(rp->compiled_regexp, 0, begin, end, &sub)) {
      int i;
      val_t arr = v7_create_array(v7);

      for (i = 0; i < sub.num_captures; i++, ptok++)
        v7_array_push(v7, arr, v7_create_string(v7, ptok->start,
                                                ptok->end - ptok->start, 1));
      if (flag_g) rp->lastIndex = utfnlen(str, sub.caps->end - str);
      v7_set_property(v7, arr, "index", 5, V7_PROPERTY_READ_ONLY,
                      v7_create_number(utfnlen(str, sub.caps->start - str)));
      return arr;
    } else
      rp->lastIndex = 0;
  }
  return v7_create_null();
}

static val_t Regex_exec(struct v7 *v7, val_t this_obj, val_t args) {
  if (v7_array_length(v7, args) > 0) {
    return rx_exec(v7, this_obj, v7_array_get(v7, args, 0), 0);
  }
  return v7_create_null();
}

static val_t Regex_test(struct v7 *v7, val_t this_obj, val_t args) {
  return v7_create_boolean(!v7_is_null(Regex_exec(v7, this_obj, args)));
}

V7_PRIVATE void init_regex(struct v7 *v7) {
  val_t ctor =
      v7_create_cfunction_ctor(v7, v7->regexp_prototype, Regex_ctor, 1);
  val_t lastIndex = v7_create_dense_array(v7);

  v7_set_property(v7, v7->global_object, "RegExp", 6, V7_PROPERTY_DONT_ENUM,
                  ctor);

  set_cfunc_prop(v7, v7->regexp_prototype, "exec", Regex_exec);
  set_cfunc_prop(v7, v7->regexp_prototype, "test", Regex_test);

  v7_set_property(v7, v7->regexp_prototype, "global", 6, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Regex_global));
  v7_set_property(v7, v7->regexp_prototype, "ignoreCase", 10,
                  V7_PROPERTY_GETTER, v7_create_cfunction(Regex_ignoreCase));
  v7_set_property(v7, v7->regexp_prototype, "multiline", 9, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Regex_multiline));
  v7_set_property(v7, v7->regexp_prototype, "source", 6, V7_PROPERTY_GETTER,
                  v7_create_cfunction(Regex_source));

  v7_array_set(v7, lastIndex, 0, v7_create_cfunction(Regex_get_lastIndex));
  v7_array_set(v7, lastIndex, 1, v7_create_cfunction(Regex_set_lastIndex));
  v7_set_property(v7, v7->regexp_prototype, "lastIndex", 9,
                  V7_PROPERTY_GETTER | V7_PROPERTY_SETTER, lastIndex);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_DISABLE_SOCKETS


#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#define close(x) closesocket(x)
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#endif

#define RECVTYPE_STRING 1
#define RECVTYPE_RAW 2

/*
 * Notes to review: structure below
 * is subject to change, don't pay attention on it
 * I'll remove this comment when it'll be ready
 */
struct socket_internal {
  int socket;
  int type;
  int recvtype;
  int local_port;
  int family;
};

static int get_sockerror() {
#ifdef _WIN32
  return WSAGetLastError();
#else
  return errno;
#endif
}

/*
 * Socket(family, type, recvtype)
 * Defaults: family = AF_INET, type = SOCK_STREAM,
 * recvtype = STRING
 */
static v7_val_t Socket_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  long arg_count;
  struct socket_internal si;

  if (!v7_is_object(this_obj) || this_obj == v7->global_object) {
    throw_exception(v7, TYPE_ERROR, "Socket ctor called as function");
  }

  memset(&si, 0, sizeof(si));

  arg_count = v7_array_length(v7, args);

  si.family = AF_INET;
  si.type = SOCK_STREAM;
  si.recvtype = RECVTYPE_STRING;

  switch (arg_count) {
    case 3:
      si.recvtype = i_as_num(v7, v7_array_get(v7, args, 2));
      if (si.recvtype != RECVTYPE_STRING && si.recvtype != RECVTYPE_RAW) {
        throw_exception(v7, TYPE_ERROR, "Invalid RecvType paramater");
      }
    case 2:
      si.type = i_as_num(v7, v7_array_get(v7, args, 1));
      if (si.type != SOCK_STREAM && si.type != SOCK_DGRAM) {
        throw_exception(v7, TYPE_ERROR, "Invalid Type parameter");
      }
    case 1:
      si.family = i_as_num(v7, v7_array_get(v7, args, 0));
      if (si.family != AF_INET && si.family != AF_INET6) {
        throw_exception(v7, TYPE_ERROR, "Invalid Family parameter");
      }
  }

  si.socket = socket(si.family, si.type, 0);

  if (si.socket < 0) {
    throw_exception(v7, TYPE_ERROR, "Cannot create socket (%d)",
                    get_sockerror());
  }

  {
    val_t si_val;
    struct socket_internal *psi =
        (struct socket_internal *) malloc(sizeof(*psi));
    memcpy(psi, &si, sizeof(*psi));

    si_val = v7_create_foreign(psi);
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, si_val);
  }

  return this_obj;
}

static struct socket_internal *Socket_check_and_get_si(struct v7 *v7,
                                                       val_t this_obj) {
  struct socket_internal *si = NULL;
  struct v7_property *si_prop =
      v7_get_own_property2(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN);

  si = (struct socket_internal *) v7_to_foreign(
      v7_property_value(v7, this_obj, si_prop));

  if (si == NULL) {
    throw_exception(v7, TYPE_ERROR, "Socket is closed");
  }

  return si;
}

static void Socket_getlocal_sockaddr(struct v7 *v7, struct socket_internal *si,
                                     struct sockaddr *sa) {
  memset(sa, 0, sizeof(*sa));

  switch (si->family) {
    case AF_INET: {
      struct sockaddr_in *sa4 = (struct sockaddr_in *) sa;
      sa4->sin_family = si->family;
      sa4->sin_port = htons(si->local_port);
      sa4->sin_addr.s_addr = INADDR_ANY;
      break;
    }
#ifdef V7_ENABLE_IPV6
    case AF_INET6: {
      struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *) sa;
      sa6->sin6_family = si->family;
      sa6->sin6_port = htons(si->local_port);
      sa6->sin6_addr = in6addr_any;
    }
#endif
    default:
      throw_exception(v7, TYPE_ERROR, "Unsupported address family");
  }
}

#ifdef V7_ENABLE_GETADDRINFO
static void Socket_getremote_sockaddr(struct v7 *v7, char *addr, uint16_t port,
                                      struct sockaddr *sa) {
  struct addrinfo *ai;

  if (getaddrinfo(addr, 0, 0, &ai) != 0) {
    throw_exception(v7, TYPE_ERROR, "Invalid host name");
  }

  switch (ai->ai_family) {
    case AF_INET: {
      struct sockaddr_in *psa = (struct sockaddr_in *) ai[0].ai_addr;
      psa->sin_port = htons(port);
      memcpy(sa, psa, sizeof(*psa));
      break;
    };
#ifdef V7_ENABLE_IPV6
    case AF_INET6: {
      /* TODO(alashkin): verify IPv6 [my provider doesn't support it] */
      struct sockaddr_in6 *psa = (struct sockaddr_in6 *) ai[0].ai_addr;
      psa->sin6_port = htons(port);
      memcpy(sa, psa, sizeof(*psa));
      break;
    }
#endif
    default:
      freeaddrinfo(ai);
      throw_exception(v7, TYPE_ERROR, "Unsupported address family");
  }

  freeaddrinfo(ai);
}
#else
static void Socket_getremote_sockaddr(struct v7 *v7, char *addr, uint16_t port,
                                      struct sockaddr *sa) {
  struct hostent *host = gethostbyname(addr);
  memset(sa, 0, sizeof(*sa));

  if (host == NULL) {
    throw_exception(v7, TYPE_ERROR, "Invalid host name");
  }

  switch (host->h_addrtype) {
    case AF_INET: {
      struct sockaddr_in *psa = (struct sockaddr_in *) sa;
      psa->sin_port = htons(port);
      psa->sin_family = AF_INET;
      memcpy(&psa->sin_addr.s_addr, host->h_addr_list[0],
             sizeof(psa->sin_addr.s_addr));
      break;
    };
#ifdef V7_ENABLE_IPV6
    case AF_INET6: {
      /* TODO(alashkin): verify IPv6 [my provider doesn't support it] */
      struct sockaddr_in6 *psa = (struct sockaddr_in6 *) sa;
      psa->sin6_port = htons(port);
      psa->sin_family = AF_INET6;
      memcpy(&psa->sin6_addr, host->h_addr_list[0], sizeof(psa->sin6_addr));

      break;
    }
#endif
    default:
      throw_exception(v7, TYPE_ERROR, "Unsupported address family");
  }
}
#endif

uint16_t Socket_check_and_get_port(struct v7 *v7, val_t port_val) {
  double port_number = i_as_num(v7, port_val);

  if (isnan(port_number) || port_number < 0 || port_number > 0xFFFF) {
    throw_exception(v7, TYPE_ERROR, "Invalid port number");
  }
  return (uint16_t) port_number;
}

/*
 * Associates a local address with a socket.
 * JS: var s = new Socket(); s.bind(80)
 * TODO(alashkin): add address as second parameter
 */
static v7_val_t Socket_bind(struct v7 *v7, val_t this_obj, val_t args) {
  struct sockaddr sa;
  long arg_count;

  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);

  arg_count = v7_array_length(v7, args);

  if (arg_count < 1 && si->local_port == 0) {
    throw_exception(v7, TYPE_ERROR,
                    "Cannot bind socket: no local port specified");
  }

  si->local_port = Socket_check_and_get_port(v7, v7_array_get(v7, args, 0));

  Socket_getlocal_sockaddr(v7, si, &sa);
  if (bind(si->socket, &sa, sizeof(sa)) != 0) {
    throw_exception(v7, TYPE_ERROR, "Cannot bind socket (%d)", get_sockerror());
  }

  return this_obj;
}

/*
 * Places a socket in a state in which it is listening
 * for an incoming connection.
 * JS: var x = new Socket().... x.listen()
 */
static v7_val_t Socket_listen(struct v7 *v7, val_t this_obj, val_t args) {
  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);
  (void) args;

  if (listen(si->socket, SOMAXCONN) != 0) {
    throw_exception(v7, TYPE_ERROR, "Cannot start listening (%d)",
                    get_sockerror());
  }

  return this_obj;
}

static uint8_t *Sockey_JSarray_to_Carray(struct v7 *v7, val_t arr,
                                         size_t *buf_size) {
  uint8_t *retval, *ptr;
  unsigned long i, elem_count = v7_array_length(v7, arr);
  /* Support byte array only */
  *buf_size = elem_count * sizeof(uint8_t);
  retval = ptr = (uint8_t *) malloc(*buf_size);

  for (i = 0; i < elem_count; i++) {
    double elem = i_as_num(v7, v7_array_get(v7, arr, i));
    if (isnan(elem) || elem < 0 || elem > 0xFF) {
      break;
    }
    *ptr = (uint8_t) elem;
    ptr++;
  }

  if (i != elem_count) {
    free(retval);
    throw_exception(v7, TYPE_ERROR, "Parameter should be a byte array");
  }

  return retval;
}

static uint8_t *Socket_get_send_buf(struct v7 *v7, val_t buf_val,
                                    size_t *buf_size, int *free_buf) {
  uint8_t *retval = NULL;

  if (v7_is_string(buf_val)) {
    retval = (uint8_t *) v7_to_string(v7, &buf_val, buf_size);
    *free_buf = 0;
  } else if (is_prototype_of(v7, buf_val, v7->array_prototype)) {
    retval = Sockey_JSarray_to_Carray(v7, buf_val, buf_size);
    *free_buf = 1;
  }

  return retval;
}

/*
 * Sends data on a connected socket.
 * JS: Socket.send(buf)
 * Ex: var x = new Socket().... x.send("Hello, world!")
 */
static v7_val_t Socket_send(struct v7 *v7, val_t this_obj, val_t args) {
  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);
  uint8_t *buf = NULL;
  size_t buf_size = 0;
  long bytes_sent;
  int free_buf = 0;

  if (v7_array_length(v7, args) != 0) {
    buf = Socket_get_send_buf(v7, v7_array_get(v7, args, 0), &buf_size,
                              &free_buf);
  }

  if (buf == NULL || buf_size == 0) {
    throw_exception(v7, TYPE_ERROR, "Invalid data to send");
  }

  bytes_sent = send(si->socket, buf, buf_size, 0);

  if (free_buf) {
    free(buf);
  }

  if (bytes_sent < 0) {
    throw_exception(v7, TYPE_ERROR, "Connot send data (%d)", get_sockerror());
  }

  return v7_create_number(bytes_sent);
}

/*
 * Establishes a connection.
 * JS: Socket.connect(addr, port)
 * Ex: var x = new Socket(); x.connect("www.hello.com",80);
 */
static v7_val_t Socket_connect(struct v7 *v7, val_t this_obj, val_t args) {
  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);
  char addr[100] = {0};
  struct sockaddr sa;
  uint16_t port;

  if (v7_array_length(v7, args) != 2) {
    throw_exception(v7, TYPE_ERROR, "Invalid arguments count");
  }

  {
    val_t addr_val;
    size_t addr_size = 0;
    const char *addr_pointer = 0;
    addr_val = v7_array_get(v7, args, 0);
    if (v7_is_string(addr_val)) {
      addr_pointer = v7_to_string(v7, &addr_val, &addr_size);
    }
    if (addr_pointer == NULL || addr_size > sizeof(addr)) {
      throw_exception(v7, TYPE_ERROR, "Invalid address");
    }
    strncpy(addr, addr_pointer, addr_size);
  }

  port = Socket_check_and_get_port(v7, v7_array_get(v7, args, 1));
  Socket_getremote_sockaddr(v7, addr, port, &sa);

  if (connect(si->socket, (struct sockaddr *) &sa, sizeof(sa)) != 0) {
    throw_exception(v7, TYPE_ERROR, "Cannot connect (%d)", get_sockerror());
  }

  return this_obj;
}

/*
 * Closes a socket.
 * JS: Socket.close();
 * Ex: var x = new Socket(); .... x.close()
 */
static v7_val_t Socket_close(struct v7 *v7, val_t this_obj, val_t args) {
  struct socket_internal *si = Socket_check_and_get_si(v7, this_obj);
  (void) args;

  close(si->socket);
  free(si);

  v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN,
                  v7_create_undefined());

  return this_obj;
}

V7_PRIVATE void init_socket(struct v7 *v7) {
  val_t socket =
      v7_create_cfunction_ctor(v7, v7->socket_prototype, Socket_ctor, 3);
  v7_set_property(v7, v7->global_object, "Socket", 6, V7_PROPERTY_DONT_ENUM,
                  socket);

  set_cfunc_prop(v7, v7->socket_prototype, "close", Socket_close);
  set_cfunc_prop(v7, v7->socket_prototype, "bind", Socket_bind);
  set_cfunc_prop(v7, v7->socket_prototype, "listen", Socket_listen);
  set_cfunc_prop(v7, v7->socket_prototype, "send", Socket_send);
  set_cfunc_prop(v7, v7->socket_prototype, "connect", Socket_connect);

  {
    val_t family = v7_create_object(v7);
    v7_set_property(v7, socket, "Family", 6, 0, family);
    v7_set_property(v7, family, "AF_INET", 7, 0, v7_create_number(AF_INET));
#ifdef V7_ENABLE_IPV6
    v7_set_property(v7, family, "AF_INET6", 8, 0, v7_create_number(AF_INET6));
#endif
  }

  {
    val_t type = v7_create_object(v7);
    v7_set_property(v7, socket, "Type", 4, 0, type);
    v7_set_property(v7, type, "SOCK_STREAM", 11, 0,
                    v7_create_number(SOCK_STREAM));
    v7_set_property(v7, type, "SOCK_DGRAM", 10, 0,
                    v7_create_number(SOCK_DGRAM));
  }

  {
    val_t recvtype = v7_create_object(v7);
    v7_set_property(v7, socket, "RecvType", 8, 0, recvtype);
    v7_set_property(v7, recvtype, "STRING", 6, 0,
                    v7_create_number(RECVTYPE_STRING));
    v7_set_property(v7, recvtype, "RAW", 3, 0, v7_create_number(RECVTYPE_RAW));
  }
#ifdef _WIN32
  {
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
    /* TODO(alashkin): add WSACleanup call */
  }
#else
  signal(SIGPIPE, SIG_IGN);
#endif
}

#endif
