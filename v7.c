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

#ifndef V7_HEAD_H_INCLUDED
#define V7_HEAD_H_INCLUDED

#endif  /* V7_HEAD_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef _UTF_H_
#define _UTF_H_ 1

#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

typedef unsigned char uchar;

typedef unsigned short Rune;  /* 16 bits */

#define nelem(a) (sizeof(a)/sizeof(a)[0])

enum {
  UTFmax    = 3,     /* maximum bytes per rune */
  Runesync  = 0x80,  /* cannot represent part of a UTF sequence (<) */
  Runeself  = 0x80,  /* rune and UTF sequences are the same (<) */
  Runeerror = 0xFFFD /* decoding error in UTF */
  /* Runemax    = 0xFFFC */ /* maximum rune value */
};

/* Edit .+1,/^$/ | cfn $PLAN9/src/lib9/utf/?*.c | grep -v static |grep -v __ */
int   chartorune(Rune *rune, const char *str);
int   fullrune(char *str, int n);
int   isdigitrune(Rune c);
int   isnewline(Rune c);
int   iswordchar(Rune c);
int   isalpharune(Rune c);
int   islowerrune(Rune c);
int   isspacerune(Rune c);
int   istitlerune(Rune c);
int   isupperrune(Rune c);
int   runelen(Rune c);
int   runenlen(Rune *r, int nrune);
Rune* runestrcat(Rune *s1, Rune *s2);
Rune* runestrchr(Rune *s, Rune c);
int   runestrcmp(Rune *s1, Rune *s2);
Rune* runestrcpy(Rune *s1, Rune *s2);
Rune* runestrdup(Rune *s);
Rune* runestrecpy(Rune *s1, Rune *es1, Rune *s2);
long  runestrlen(Rune *s);
Rune* runestrncat(Rune *s1, Rune *s2, long n);
int   runestrncmp(Rune *s1, Rune *s2, long n);
Rune* runestrncpy(Rune *s1, Rune *s2, long n);
Rune* runestrrchr(Rune *s, Rune c);
Rune* runestrstr(Rune *s1, Rune *s2);
int   runetochar(char *str, Rune *rune);
Rune  tolowerrune(Rune c);
Rune  totitlerune(Rune c);
Rune  toupperrune(Rune c);
char* utfecpy(char *to, char *e, char *from);
int   utflen(char *s);
int   utfnlen(char *s, long m);
char* utfnshift(char *s, long m);
char* utfrrune(char *s, long c);
char* utfrune(char *s, long c);
char* utfutf(char *s1, char *s2);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */
#endif  /* _UTF_H_ */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

enum v7_tok {
  TOK_END_OF_INPUT,
  TOK_NUMBER,
  TOK_STRING_LITERAL,
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

V7_PRIVATE int skip_to_next_tok(const char **ptr);
V7_PRIVATE enum v7_tok get_tok(const char **s, double *n);
V7_PRIVATE const char *tok_name(enum v7_tok);
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_INTERNAL_H_INCLUDED
#define V7_INTERNAL_H_INCLUDED

#include "v7.h"

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
#else
#include <stdint.h>
#endif

/*
 * If V7_CACHE_OBJS is defined, then v7_freeval() will not actually free
 * the structure, but append it to the list of free structures.
 * Subsequent allocations try to grab a structure from the free list,
 * which speeds up allocation.
 * #define V7_CACHE_OBJS
 */

/* Maximum length of the string literal */
#define MAX_STRING_LITERAL_LENGTH 2000

#define RE_MAX_SUB 32
#define RE_MAX_RANGES 32
#define RE_MAX_SETS 16
#define RE_MAX_REP 0xFFFF
#define RE_MAX_THREADS 1000
#define V7_RE_MAX_REPL_SUB 255

typedef unsigned short uint16_t;
typedef signed char sint8_t;

#define reg_malloc malloc
#define reg_free free

#ifndef V7_EX_TRY_CATCH
#define V7_EX_TRY_CATCH(catch_point) setjmp(catch_point)
#define V7_EX_THROW(c, m, message) \
  do {                             \
    m = message;                   \
    longjmp(c, 1);                 \
  } while (0)
#endif

/* MSVC6 doesn't have standard C math constants defined */
#ifndef M_PI
#define M_E 2.71828182845904523536028747135266250
#define M_LOG2E 1.44269504088896340735992468100189214
#define M_LOG10E 0.434294481903251827651128918916605082
#define M_LN2 0.693147180559945309417232121458176568
#define M_LN10 2.30258509299404568401799145468436421
#define M_PI 3.14159265358979323846264338327950288
#define M_SQRT2 1.41421356237309504880168872420969808
#define M_SQRT1_2 0.707106781186547524400844362104849039
#ifndef NAN
#define NAN atof("NAN")
#endif
#ifndef INFINITY
#define INFINITY atof("INFINITY") /* TODO: fix this */
#endif
#endif

/* Different classes of V7_TYPE_OBJ type */
enum v7_class {
  V7_CLASS_NONE,
  V7_CLASS_ARRAY,
  V7_CLASS_BOOLEAN,
  V7_CLASS_DATE,
  V7_CLASS_ERROR,
  V7_CLASS_FUNCTION,
  V7_CLASS_NUMBER,
  V7_CLASS_OBJECT,
  V7_CLASS_REGEXP,
  V7_CLASS_STRING,
  V7_NUM_CLASSES
};

/* Sub expression matches */
struct Resub {
  int subexpr_num;
  struct re_tok {
    const char *start; /* points to the beginning of the token */
    const char *end;   /* points to the end of the token */
  } sub[RE_MAX_SUB];
};

struct Rerange {
  Rune s;
  Rune e;
};
/* character class, each pair of rune's defines a range */
struct Reclass {
  struct Rerange *end;
  struct Rerange spans[RE_MAX_RANGES];
};

/* Parser Information */
struct Renode {
  uint8_t type;
  union {
    Rune c;             /* character */
    struct Reclass *cp; /* class pointer */
    struct {
      struct Renode *x;
      union {
        struct Renode *y;
        uint8_t n;
        struct {
          uint8_t ng; /* not greedy flag */
          uint16_t min;
          uint16_t max;
        } rp;
      } y;
    } xy;
  } par;
};

/* Machine instructions */
struct Reinst {
  uint8_t opcode;
  union {
    uint8_t n;
    Rune c;             /* character */
    struct Reclass *cp; /* class pointer */
    struct {
      struct Reinst *x;
      union {
        struct {
          uint16_t min;
          uint16_t max;
        } rp;
        struct Reinst *y;
      } y;
    } xy;
  } par;
};

/* struct Reprogram definition */
struct Reprog {
  struct Reinst *start, *end;
  unsigned int subexpr_num;
  struct Reclass charset[RE_MAX_SETS];
};

/* struct Rethread definition */
struct Rethread {
  struct Reinst *pc;
  const char *start;
  struct Resub sub;
};

typedef void (*v7_prop_func_t)(struct v7_val *this_obj, struct v7_val *arg,
                               struct v7_val *result);

struct v7_prop {
  struct v7_prop *next;
  struct v7_val *key;
  struct v7_val *val;
  unsigned short flags;
#define V7_PROP_NOT_WRITABLE 1   /* property is not changeable */
#define V7_PROP_NOT_ENUMERABLE 2 /* not enumerable in for..in loop */
#define V7_PROP_NOT_DELETABLE 4  /* delete-ing this property must fail */
#define V7_PROP_ALLOCATED 8      /* v7_prop must be free()-ed */
};

/* Vector, describes some memory location pointed by 'p' with length 'len' */
struct v7_vec {
  const char *p;
  int len;
};
#define V7_VEC(str) \
  { (str), sizeof(str) - 1 }

struct v7_string {
  char *buf;           /* Pointer to buffer with string/regexp data */
  unsigned long len;   /* String/regexp length */
  char loc[16];        /* Small strings/regexp are stored here */
  struct Reprog *prog; /* Pointer to compiled regexp */
  long lastIndex;
};

struct v7_func {
  char *source_code;      /* \0-terminated function source code */
  int line_no;            /* Line number where function begins */
  struct v7_val *var_obj; /* Function var object: var decls and func defs */
};

union v7_scalar {
  double num;            /* Holds "Number" or "Boolean" value */
  struct v7_string str;  /* Holds "String" value */
  struct v7_func func;   /* \0-terminated function code */
  struct v7_prop *array; /* List of array elements */
  v7_func_t c_func;      /* Pointer to the C function */
  struct {
    v7_prop_func_t f; /* Object's property function, e.g. String.length */
    struct v7_val *o; /* Current "this" object for property function */
  } prop_func;
};

struct v7_val {
  struct v7_val *next;
  struct v7_val *proto;  /* Prototype */
  struct v7_val *ctor;   /* Constructor object */
  struct v7_prop *props; /* Object's key/value list */
  union v7_scalar v;     /* The value itself */
  enum v7_type type;     /* Value type */
  enum v7_class cls;     /* Object's internal [[Class]] property */
  short ref_count;       /* Reference counter */

  union {
    uint16_t flags; /* Flags - defined below */
    struct v7_val_flags {
      /* TODO(??) avoid using bitfields which are a GCC extension */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
      uint16_t val_alloc : 1; /* Whole "struct v7_val" must be free()-ed */
      uint16_t str_alloc : 1; /* v.str.buf must be free()-ed */
      uint16_t js_func : 1;   /* Function object is a JavsScript code */
      uint16_t prop_func
          : 1; /* Function object is a native property function */
#define V7_PROP_FUNC 8
      uint16_t val_dealloc : 1; /* Value has been deallocated */

      uint16_t re_g : 1; /* execution RegExp flag g */
      uint16_t re_i : 1; /* compiler & execution RegExp flag i */
      uint16_t re_m : 1; /* execution RegExp flag m */
      uint16_t re : 1;   /* parser RegExp flag re */
#pragma GCC diagnostic pop
    } fl;
  } fl;
};

#define V7_MKVAL(_p, _t, _c, _v)                \
  {                                             \
    0, (_p), 0, 0, {(_v)}, (_t), (_c), 0, { 0 } \
  }

struct v7_pstate {
  const char *file_name;
  const char *source_code;
  const char *pc; /* Current parsing position */
  int line_no;    /* Line number */
  int prev_line_no; /* Line number of previous token */
};

struct v7 {
  struct v7_val root_scope; /* "global" object (root-level execution context) */
  struct v7_val *stack[200]; /* TODO: make it non-fixed, auto-grow */
  int sp;                    /* Stack pointer */
  int flags;
#define V7_SCANNING 1 /* Pre-scan to initialize lexical scopes, no exec */
#define V7_NO_EXEC 2  /* Non-executing code block: if (false) { block } */

  struct v7_pstate pstate; /* Parsing state */
  enum v7_tok cur_tok;     /* Current token */
  const char *tok;         /* Parsed terminal token (ident, number, string) */
  unsigned long tok_len;   /* Length of the parsed terminal token */
  int after_newline;       /* True if the cur_tok starts a new line */
  double cur_tok_dbl;

  const char *key;       /* Key for the assignment operation */
  unsigned long key_len; /* Key length for the assignment operation */

  char error_message[100]; /* Placeholder for the error message */

  struct v7_val *cur_obj;   /* Current namespace object ('x=1; x.y=1;', etc) */
  struct v7_val *this_obj;  /* Current "this" object */
  struct v7_val *ctx;       /* Current execution context */
  struct v7_val *cf;        /* Currently executing function */
  struct v7_val *functions; /* List of declared function */
  struct v7_val *free_values; /* List of free (deallocated) values */
  struct v7_prop *free_props; /* List of free (deallocated) props */
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

#define V7_STATIC_ASSERT(COND, MSG) \
      typedef char static_assertion_##MSG[2*(!!(COND)) - 1]

#define THROW(err_code)                                                        \
  do {                                                                         \
    snprintf(v7->error_message, sizeof(v7->error_message), "%s line %d: %s",   \
             v7->pstate.file_name, v7->pstate.line_no, v7_strerror(err_code)); \
    return (err_code);                                                         \
  } while (0)

#define CHECK(cond, code)     \
  do {                        \
    if (!(cond)) THROW(code); \
  } while (0)

#ifdef _WIN32
#define TRACE_CALL(fmt, ...) /* printf(fmt, ...) */
#else
#define TRACE_CALL(fmt, ...)
#endif

extern int __lev;
/* #define TRY(call) do { \
  enum v7_err _e; \
  TRACE_CALL("> %s %d\n", #call, __LINE__);  \
  _e = call;      \
  CHECK(_e == V7_OK, _e);     \
  TRACE_CALL("< %s %d\n", #call, __LINE__); \
} while (0) */
#define TRY(call)           \
  do {                      \
    enum v7_err _e = call;  \
    CHECK(_e == V7_OK, _e); \
  } while (0)

/* Print current function name and stringified object */
#define TRACE_OBJ(O)                                         \
  do {                                                       \
    char x[4000];                                            \
    printf("==> %s [%s]\n", __func__,                        \
           O == NULL ? "@" : v7_stringify(O, x, sizeof(x))); \
  } while (0)

/* Initializer for "struct v7_val", object type */
#define MKOBJ(_proto) V7_MKVAL(_proto, V7_TYPE_OBJ, V7_CLASS_OBJECT, 0)

/* True if current code is executing. TODO(lsm): use bit fields, per vrz@ */
#define EXECUTING(_fl) (!((_fl) & (V7_NO_EXEC | V7_SCANNING)))

#ifndef V7_PRIVATE
#define V7_PRIVATE static
#else
extern struct v7_val s_constructors[];
extern struct v7_val s_prototypes[];
extern struct v7_val s_global;
extern struct v7_val s_math;
extern struct v7_val s_json;
extern struct v7_val s_file;
#endif

/* Adds a read-only attribute "val" by key "name" to the object "obj" */
#define SET_RO_PROP_V(obj, name, val)                                 \
  do {                                                                \
    static struct v7_val key = MKOBJ(&s_prototypes[V7_CLASS_STRING]); \
    static struct v7_prop prop = {NULL, &key, &val, 0};               \
    v7_init_str(&key, (char *)(name), strlen(name), 0);               \
    prop.next = obj.props;                                            \
    obj.props = &prop;                                                \
  } while (0)

/* Adds read-only attribute with given initializers to the object "_o" */
#define SET_RO_PROP2(_o, _name, _t, _proto, _attr, _initializer, _fl) \
  do {                                                                \
    static struct v7_val _val = MKOBJ(_proto);                        \
    _val.v._attr = (_initializer);                                    \
    _val.type = (_t);                                                 \
    _val.fl.flags = (_fl);                                            \
    SET_RO_PROP_V(_o, _name, _val);                                   \
  } while (0)

#define SET_RO_PROP(obj, name, _t, attr, _v) \
  SET_RO_PROP2(obj, name, _t, &s_prototypes[V7_CLASS_OBJECT], attr, _v, 0)

/* Adds property function "_func" with key "_name" to the object "_obj" */
#define SET_PROP_FUNC(_obj, _name, _func) \
  SET_RO_PROP2(_obj, _name, V7_TYPE_NULL, 0, prop_func.f, _func, V7_PROP_FUNC)

/* Adds method "_func" with key "_name" to the object "_obj" */
#define SET_METHOD(_obj, _name, _func)                                 \
  do {                                                                 \
    static struct v7_val _val = MKOBJ(&s_prototypes[V7_CLASS_STRING]); \
    v7_set_class(&_val, V7_CLASS_FUNCTION);                            \
    _val.v.c_func = (_func);                                           \
    SET_RO_PROP_V(_obj, _name, _val);                                  \
  } while (0)

#define OBJ_SANITY_CHECK(obj)          \
  do {                                 \
    assert((obj) != NULL);             \
    assert((obj)->ref_count >= 0);     \
    assert(!(obj)->fl.fl.val_dealloc); \
  } while (0)

#define INC_REF_COUNT(v) \
  do {                   \
    OBJ_SANITY_CHECK(v); \
    (v)->ref_count++;    \
  } while (0)

#define DEC_REF_COUNT(v) \
  do {                   \
    OBJ_SANITY_CHECK(v); \
    (v)->ref_count--;    \
  } while (0)

/* Forward declarations */

V7_PRIVATE sint8_t nextesc(Rune *r, const char **src);
V7_PRIVATE struct Reprog *re_compiler(const char *pattern,
                                      struct v7_val_flags flags,
                                      const char **errorp);
V7_PRIVATE uint8_t re_exec(struct Reprog *prog, struct v7_val_flags flags,
                           const char *string, struct Resub *loot);
V7_PRIVATE void re_free(struct Reprog *prog);
V7_PRIVATE int re_rplc(struct Resub *loot, const char *src, const char *rstr,
                       struct Resub *dstsub);

V7_PRIVATE enum v7_err regex_xctor(struct v7 *v7, struct v7_val *obj,
                                   const char *re, size_t re_len,
                                   const char *fl, size_t fl_len);
V7_PRIVATE enum v7_err regex_check_prog(struct v7_val *re_obj);

/* Parser */

/* TODO(mkm): move to .c file one we get rid of the old parser */
#define EXPECT(t)                                     \
  do {                                                \
    if ((v7)->cur_tok != (t)) return V7_SYNTAX_ERROR; \
    next_tok(v7);                                     \
  } while (0)

V7_PRIVATE enum v7_tok next_tok(struct v7 *v7);
V7_PRIVATE enum v7_tok lookahead(const struct v7 *v7);

V7_PRIVATE int instanceof(const struct v7_val *obj, const struct v7_val *ctor);
V7_PRIVATE enum v7_err parse_expression(struct v7 *);
V7_PRIVATE enum v7_err parse_statement(struct v7 *, int *is_return);
V7_PRIVATE int cmp(const struct v7_val *a, const struct v7_val *b);
V7_PRIVATE enum v7_err do_exec(struct v7 *v7, const char *, const char *, int);
V7_PRIVATE void init_stdlib(void);
V7_PRIVATE void skip_whitespaces_and_comments(struct v7 *v7);
V7_PRIVATE int is_num(const struct v7_val *v);
V7_PRIVATE int is_bool(const struct v7_val *v);
V7_PRIVATE int is_string(const struct v7_val *v);
V7_PRIVATE enum v7_err toString(struct v7 *v7, struct v7_val *obj);
V7_PRIVATE enum v7_err check_str_re_conv(struct v7 *v7, struct v7_val **arg,
                                         int re_fl);

V7_PRIVATE double _conv_to_num(struct v7 *v7, struct v7_val *arg);
V7_PRIVATE long _conv_to_int(struct v7 *v7, struct v7_val *arg);

V7_PRIVATE void init_standard_constructor(enum v7_class cls, v7_func_t ctor);
V7_PRIVATE enum v7_err inc_stack(struct v7 *v7, int incr);
V7_PRIVATE enum v7_err _prop_func_2_value(struct v7 *v7, struct v7_val **f);
V7_PRIVATE struct v7_val *make_value(struct v7 *v7, enum v7_type type);
V7_PRIVATE enum v7_err v7_set2(struct v7 *v7, struct v7_val *obj,
                               struct v7_val *k, struct v7_val *v);
V7_PRIVATE char *v7_strdup(const char *ptr, unsigned long len);
V7_PRIVATE struct v7_prop *mkprop(struct v7 *v7);
V7_PRIVATE void free_prop(struct v7 *v7, struct v7_prop *p);
V7_PRIVATE struct v7_val str_to_val(const char *buf, size_t len);
V7_PRIVATE struct v7_val *find(struct v7 *v7, const struct v7_val *key);
V7_PRIVATE struct v7_val *get2(struct v7_val *obj, const struct v7_val *key);
V7_PRIVATE enum v7_err v7_call2(struct v7 *v7, struct v7_val *, int, int);

V7_PRIVATE enum v7_err v7_make_and_push(struct v7 *v7, enum v7_type type);
V7_PRIVATE enum v7_err v7_append(struct v7 *, struct v7_val *, struct v7_val *);
V7_PRIVATE struct v7_val *v7_mkv(struct v7 *v7, enum v7_type t, ...);
V7_PRIVATE void v7_freeval(struct v7 *v7, struct v7_val *v);
V7_PRIVATE int v7_sp(struct v7 *v7);
V7_PRIVATE struct v7_val **v7_top(struct v7 *);
V7_PRIVATE struct v7_val *v7_top_val(struct v7 *);
V7_PRIVATE const char *v7_strerror(enum v7_err);
V7_PRIVATE int v7_is_class(const struct v7_val *obj, enum v7_class cls);
V7_PRIVATE void v7_set_class(struct v7_val *obj, enum v7_class cls);
V7_PRIVATE void v7_init_func(struct v7_val *v, v7_func_t func);
V7_PRIVATE void v7_init_str(struct v7_val *, const char *, unsigned long, int);
V7_PRIVATE void v7_init_num(struct v7_val *, double);
V7_PRIVATE void v7_init_bool(struct v7_val *, int);
V7_PRIVATE enum v7_err v7_push(struct v7 *, struct v7_val *);
V7_PRIVATE enum v7_err push_bool(struct v7 *, int is_true);
V7_PRIVATE enum v7_err push_string(struct v7 *, const char *, unsigned long,
                                   int);
V7_PRIVATE enum v7_err push_func(struct v7 *v7, v7_func_t func);
V7_PRIVATE enum v7_err push_new_object(struct v7 *v7);
V7_PRIVATE enum v7_err push_number(struct v7 *v7, double num);
V7_PRIVATE void free_props(struct v7 *v7);
V7_PRIVATE void free_values(struct v7 *v7);
V7_PRIVATE struct v7_val v7_str_to_val(const char *buf);
V7_PRIVATE enum v7_err v7_del2(struct v7 *v7, struct v7_val *, const char *,
                               unsigned long);

/* Generic function to set an attribute in an object. */
V7_PRIVATE enum v7_err v7_setv(struct v7 *v7, struct v7_val *obj,
                               enum v7_type key_type, enum v7_type val_type,
                               ...);

V7_PRIVATE void init_array(void);
V7_PRIVATE void init_boolean(void);
V7_PRIVATE void init_crypto(void);
V7_PRIVATE void init_date(void);
V7_PRIVATE void init_error(void);
V7_PRIVATE void init_function(void);
V7_PRIVATE void init_json(void);
V7_PRIVATE void init_math(void);
V7_PRIVATE void init_number(void);
V7_PRIVATE void init_object(void);
V7_PRIVATE void init_string(void);
V7_PRIVATE void init_regex(void);


#endif /* V7_INTERNAL_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED


#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

/* TODO(mkm): reorder */
enum ast_tag {
  AST_NOP,
  AST_SCRIPT,
  AST_VAR,
  AST_VAR_ITEM,
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

  AST_IDENT,
  AST_NUM,
  AST_STRING,
  AST_SEQ,
  AST_WHILE,
  AST_DOWHILE,
  AST_FOR,
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

  AST_MAX_TAG
};

#define AST_SIZE_MULTIPLIER (1.5)

struct ast {
  char *buf;
  size_t len;
  size_t size;
};

typedef unsigned long ast_off_t;

V7_PRIVATE void ast_init(struct ast *, size_t);
V7_PRIVATE void ast_free(struct ast *);
V7_PRIVATE void ast_resize(struct ast *, size_t);
V7_PRIVATE void ast_trim(struct ast *);
V7_PRIVATE size_t ast_insert(struct ast *, size_t, const char *, size_t);
V7_PRIVATE size_t ast_append(struct ast *, const char *, size_t);

enum ast_which_skip {
  AST_END_SKIP = 0,
  AST_FOR_BODY_SKIP = 1,
  AST_DO_WHILE_COND_SKIP = 1,
  AST_END_IF_TRUE_SKIP = 1,
  AST_TRY_CATCH_SKIP = 1,
  AST_TRY_FINALLY_SKIP = 2,
  AST_FUNC_BODY_SKIP = 1,
  AST_SWITCH_DEFAULT_SKIP = 1
};

V7_PRIVATE size_t ast_add_node(struct ast *, enum ast_tag);
V7_PRIVATE size_t ast_insert_node(struct ast *, size_t, enum ast_tag);
V7_PRIVATE size_t ast_set_skip(struct ast *, ast_off_t, enum ast_which_skip);
V7_PRIVATE size_t ast_get_skip(struct ast *, ast_off_t, enum ast_which_skip);
V7_PRIVATE enum ast_tag ast_fetch_tag(struct ast *, ast_off_t *);
V7_PRIVATE void ast_move_to_children(struct ast *, ast_off_t *);

V7_PRIVATE void ast_add_num(struct ast *, double);
V7_PRIVATE void ast_add_ident(struct ast *, const char *, size_t);
V7_PRIVATE void ast_add_string(struct ast *, const char *, size_t);

V7_PRIVATE void ast_dump(FILE *, struct ast *, ast_off_t);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif  /* AST_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef V7_APARSER_H_INCLUDED
#define V7_APARSER_H_INCLUDED


#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

V7_PRIVATE enum v7_err aparse(struct ast *, const char*, int);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif  /* V7_APARSER_H_INCLUDED */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE struct v7_val s_constructors[V7_NUM_CLASSES];
V7_PRIVATE struct v7_val s_prototypes[V7_NUM_CLASSES];

V7_PRIVATE struct v7_val s_global = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
V7_PRIVATE struct v7_val s_math = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
V7_PRIVATE struct v7_val s_json = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
V7_PRIVATE struct v7_val s_file = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE int instanceof(const struct v7_val *obj, const struct v7_val *ctor) {
  OBJ_SANITY_CHECK(obj);
  if (obj->type == V7_TYPE_OBJ && ctor != NULL) {
    while (obj != NULL) {
      if (obj->ctor == ctor) return 1;
      if (obj->proto == obj) break; /* Break on circular reference */
      obj = obj->proto;
    }
  }
  return 0;
}

V7_PRIVATE int v7_is_class(const struct v7_val *obj, enum v7_class cls) {
  return instanceof(obj, &s_constructors[cls]);
}

V7_PRIVATE int is_string(const struct v7_val *v) {
  OBJ_SANITY_CHECK(v);
  return v->type == V7_TYPE_STR || v7_is_class(v, V7_CLASS_STRING);
}

V7_PRIVATE int is_num(const struct v7_val *v) {
  OBJ_SANITY_CHECK(v);
  return v->type == V7_TYPE_NUM || v7_is_class(v, V7_CLASS_NUMBER);
}

V7_PRIVATE int is_bool(const struct v7_val *v) {
  OBJ_SANITY_CHECK(v);
  return v->type == V7_TYPE_BOOL || v7_is_class(v, V7_CLASS_BOOLEAN);
}

V7_PRIVATE enum v7_err _prop_func_2_value(struct v7 *v7, struct v7_val **f) {
  if ((*f)->fl.fl.prop_func) {
    struct v7_val *v, *o;
    v = make_value(v7, V7_TYPE_UNDEF);
    CHECK(v != NULL, V7_OUT_OF_MEMORY);
    o = (*f)->v.prop_func.o;
    (*f)->v.prop_func.f(o, NULL, v);
    *f = v;
  }
  return V7_OK;
}

V7_PRIVATE char *v7_strdup(const char *ptr, unsigned long len) {
  char *p = (char *) malloc(len + 1);
  if (p == NULL) return NULL;
  if (ptr)
    memcpy(p, ptr, len);
  else
    memset(p, 0, len);
  p[len] = '\0';
  return p;
}

V7_PRIVATE void v7_init_str(struct v7_val *v, const char *p, unsigned long len,
                            int own) {
  v->type = V7_TYPE_STR;
  v->proto = &s_prototypes[V7_CLASS_STRING];
  v->v.str.buf = (char *) p;
  v->v.str.len = len;
  v->fl.fl.str_alloc = 0;
  if (own) {
    if (len < sizeof(v->v.str.loc) - 1) {
      v->v.str.buf = v->v.str.loc;
      if (p)
        memcpy(v->v.str.loc, p, len);
      else
        memset(v->v.str.loc, 0, len);
      v->v.str.loc[len] = '\0';
    } else {
      v->v.str.buf = v7_strdup(p, len);
      v->fl.fl.str_alloc = 1;
    }
  }
}

V7_PRIVATE void v7_init_num(struct v7_val *v, double num) {
  v->type = V7_TYPE_NUM;
  v->proto = &s_prototypes[V7_CLASS_NUMBER];
  v->v.num = num;
}

V7_PRIVATE void v7_init_bool(struct v7_val *v, int is_true) {
  v->type = V7_TYPE_BOOL;
  v->proto = &s_prototypes[V7_CLASS_BOOLEAN];
  v->v.num = is_true ? 1.0 : 0.0;
}

V7_PRIVATE void v7_init_func(struct v7_val *v, v7_func_t func) {
  v7_set_class(v, V7_CLASS_FUNCTION);
  v->v.c_func = func;
}

V7_PRIVATE void v7_set_class(struct v7_val *v, enum v7_class cls) {
  v->type = V7_TYPE_OBJ;
  v->cls = cls;
  v->proto = &s_prototypes[cls];
  v->ctor = &s_constructors[cls];
}

V7_PRIVATE void free_prop(struct v7 *v7, struct v7_prop *p) {
  if (p->key != NULL) v7_freeval(v7, p->key);
  v7_freeval(v7, p->val);
  p->val = p->key = NULL;
  if (p->flags & V7_PROP_ALLOCATED) {
#ifdef V7_CACHE_OBJS
    p->next = v7->free_props;
    v7->free_props = p;
#else
    free(p);
#endif
  }
  p->flags = 0;
}

V7_PRIVATE void init_standard_constructor(enum v7_class cls, v7_func_t ctor) {
  s_prototypes[cls].type = s_constructors[cls].type = V7_TYPE_OBJ;
  s_prototypes[cls].ref_count = s_constructors[cls].ref_count = 1;
  s_prototypes[cls].proto = &s_prototypes[V7_CLASS_OBJECT];
  s_prototypes[cls].ctor = &s_constructors[cls];
  s_constructors[cls].proto = &s_prototypes[V7_CLASS_OBJECT];
  s_constructors[cls].ctor = &s_constructors[V7_CLASS_FUNCTION];
  s_constructors[cls].v.c_func = ctor;
}

V7_PRIVATE void v7_freeval(struct v7 *v7, struct v7_val *v) {
  assert(v->ref_count > 0);
  if (--v->ref_count > 0) return;

  if (v->type == V7_TYPE_OBJ) {
    struct v7_prop *p, *tmp;
    for (p = v->props; p != NULL; p = tmp) {
      tmp = p->next;
      free_prop(v7, p);
    }
    v->props = NULL;
  }

  if (v7_is_class(v, V7_CLASS_ARRAY)) {
    struct v7_prop *p, *tmp;
    for (p = v->v.array; p != NULL; p = tmp) {
      tmp = p->next;
      free_prop(v7, p);
    }
    v->v.array = NULL;
  } else if (v->type == V7_TYPE_STR || v7_is_class(v, V7_CLASS_STRING) ||
             v7_is_class(v, V7_CLASS_REGEXP)) {
    if (v->v.str.prog) {
      if (v->v.str.prog->start) reg_free(v->v.str.prog->start);
      reg_free(v->v.str.prog);
    }
    if (v->v.str.buf && v->fl.fl.str_alloc) free(v->v.str.buf);
  } else if (v7_is_class(v, V7_CLASS_FUNCTION)) {
    if (v->fl.fl.str_alloc && v->fl.fl.js_func) {
      free(v->v.func.source_code);
      v7_freeval(v7, v->v.func.var_obj);
    }
  } else if (v7_is_class(v, V7_CLASS_OBJECT)) {
    if (v->fl.fl.prop_func && v->v.prop_func.o)
      v7_freeval(v7, v->v.prop_func.o);
  }

  if (v->fl.fl.val_alloc) {
    v->fl.fl.val_alloc = 0;
    v->fl.fl.val_dealloc = 1;
    memset(v, 0, sizeof(*v));
#ifdef V7_CACHE_OBJS
    v->next = v7->free_values;
    v7->free_values = v;
#else
    free(v);
#endif
  }
}

V7_PRIVATE enum v7_err inc_stack(struct v7 *v7, int incr) {
  int i;

  CHECK(v7->sp + incr < (int) ARRAY_SIZE(v7->stack), V7_STACK_OVERFLOW);
  CHECK(v7->sp + incr >= 0, V7_STACK_UNDERFLOW);

  /* Free values pushed on stack (like string literals and functions) */
  for (i = 0; incr < 0 && i < -incr && i < v7->sp; i++) {
    v7_freeval(v7, v7->stack[v7->sp - (i + 1)]);
    v7->stack[v7->sp - (i + 1)] = NULL;
  }

  v7->sp += incr;
  return V7_OK;
}

V7_PRIVATE void free_values(struct v7 *v7) {
  struct v7_val *v;
  while (v7->free_values != NULL) {
    v = v7->free_values->next;
    free(v7->free_values);
    v7->free_values = v;
  }
}

V7_PRIVATE void free_props(struct v7 *v7) {
  struct v7_prop *p;
  while (v7->free_props != NULL) {
    p = v7->free_props->next;
    free(v7->free_props);
    v7->free_props = p;
  }
}

V7_PRIVATE struct v7_val *make_value(struct v7 *v7, enum v7_type type) {
  struct v7_val *v = NULL;

  if ((v = v7->free_values) != NULL) {
    v7->free_values = v->next;
  } else {
    v = (struct v7_val *) calloc(1, sizeof(*v));
  }

  if (v != NULL) {
    assert(v->ref_count == 0);
    v->fl.flags = 0;
    v->fl.fl.val_alloc = 1; /* V7_VAL_ALLOCATED */
    v->type = type;
    switch (type) {
      case V7_TYPE_NUM:
        v->proto = &s_prototypes[V7_CLASS_NUMBER];
        break;
      case V7_TYPE_STR:
        v->proto = &s_prototypes[V7_CLASS_STRING];
        break;
      case V7_TYPE_BOOL:
        v->proto = &s_prototypes[V7_CLASS_BOOLEAN];
        break;
      default:
        break;
    }
  }
  return v;
}

V7_PRIVATE struct v7_prop *mkprop(struct v7 *v7) {
  struct v7_prop *m;
  if ((m = v7->free_props) != NULL) {
    v7->free_props = m->next;
  } else {
    m = (struct v7_prop *) calloc(1, sizeof(*m));
  }
  if (m != NULL) m->flags = V7_PROP_ALLOCATED;
  return m;
}

V7_PRIVATE struct v7_val str_to_val(const char *buf, size_t len) {
  struct v7_val v;
  memset(&v, 0, sizeof(v));
  v.type = V7_TYPE_STR;
  v.v.str.buf = (char *) buf;
  v.v.str.len = len;
  return v;
}

V7_PRIVATE struct v7_val v7_str_to_val(const char *buf) {
  return str_to_val((char *) buf, strlen(buf));
}

V7_PRIVATE int cmp(const struct v7_val *a, const struct v7_val *b) {
  int res;
  double an, bn;
  const struct v7_string *as, *bs;
  struct v7_val ta = MKOBJ(0), tb = MKOBJ(0);

  if (a == NULL || b == NULL) return 1;
  if ((a->type == V7_TYPE_UNDEF || a->type == V7_TYPE_NULL) &&
      (b->type == V7_TYPE_UNDEF || b->type == V7_TYPE_NULL))
    return 0;

  if (is_num(a) && is_num(b)) {
    v7_init_num(&ta, a->v.num);
    v7_init_num(&tb, b->v.num);
    a = &ta;
    b = &tb;
  }

  if (is_string(a) && is_string(b)) {
    v7_init_str(&ta, a->v.str.buf, a->v.str.len, 0);
    v7_init_str(&tb, b->v.str.buf, b->v.str.len, 0);
    a = &ta;
    b = &tb;
  }

  if (a->type != b->type) return 1;

  an = a->v.num, bn = b->v.num;
  as = &a->v.str, bs = &b->v.str;

  switch (a->type) {
    case V7_TYPE_NUM:
      return (isinf(an) && isinf(bn)) || (isnan(an) && isnan(bn)) ? 0 : an - bn;
    case V7_TYPE_BOOL:
      return an != bn;
    case V7_TYPE_STR:
      res = memcmp(as->buf, bs->buf, as->len < bs->len ? as->len : bs->len);
      return res != 0 ? res : (int) as->len - (int) bs->len;
      return as->len != bs->len || memcmp(as->buf, bs->buf, as->len) != 0;
    default:
      return (int) (a - b);
  }
}

V7_PRIVATE struct v7_prop *v7_get2(struct v7_val *obj, const struct v7_val *key,
                                   int own_prop) {
  struct v7_prop *m;
  struct v7_val *o = obj;
  int proto = 0;
  for (; obj != NULL; obj = obj->proto, proto = 1) {
    if (v7_is_class(obj, V7_CLASS_ARRAY) && key->type == V7_TYPE_NUM) {
      int i = (int)key->v.num;
      for (m = obj->v.array; m != NULL; m = m->next) {
        if (i-- == 0) return m;
      }
    } else if (obj->type == V7_TYPE_OBJ) {
      for (m = obj->props; m != NULL; m = m->next) {
        if (cmp(m->key, key) == 0) {
          if (m->val->fl.fl.prop_func) {
            INC_REF_COUNT(o);
            m->val->v.prop_func.o = o;
            return m;
          }
          if (!own_prop || !proto) return m;
        }
      }
    }
    if (obj->proto == obj) break;
  }
  return NULL;
}

V7_PRIVATE struct v7_val *get2(struct v7_val *obj, const struct v7_val *key) {
  struct v7_prop *m = v7_get2(obj, key, 0);
  return (m == NULL) ? NULL : m->val;
}

V7_PRIVATE enum v7_err vinsert(struct v7 *v7, struct v7_prop **h,
                               struct v7_val *key, struct v7_val *val) {
  struct v7_prop *m = mkprop(v7);
  CHECK(m != NULL, V7_OUT_OF_MEMORY);

  INC_REF_COUNT(key);
  INC_REF_COUNT(val);
  m->key = key;
  m->val = val;
  m->next = *h;
  *h = m;

  return V7_OK;
}

V7_PRIVATE struct v7_val *find(struct v7 *v7, const struct v7_val *key) {
  struct v7_val *v, *f;

  if (!EXECUTING(v7->flags)) return NULL;

  for (f = v7->ctx; f != NULL; f = f->next) {
    if ((v = get2(f, key)) != NULL) return v;
  }

  if (v7->cf && (v = get2(v7->cf->v.func.var_obj, key)) != NULL) return v;

  return NULL;
}

V7_PRIVATE enum v7_err v7_set2(struct v7 *v7, struct v7_val *obj,
                               struct v7_val *k, struct v7_val *v) {
  struct v7_prop *m = NULL;

  CHECK(obj != NULL && k != NULL && v != NULL, V7_INTERNAL_ERROR);
  CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_ERROR);

  /* Find attribute inside object */
  if ((m = v7_get2(obj, k, 1)) != NULL) {
    INC_REF_COUNT(v);
    if (m->val->fl.fl.prop_func) {
      m->val->v.prop_func.f(m->val->v.prop_func.o, v, NULL);
      v7_freeval(v7, m->val->v.prop_func.o);
    } else {
      v7_freeval(v7, m->val);
      m->val = v;
    }
  } else {
    TRY(vinsert(v7, &obj->props, k, v));
  }

  return V7_OK;
}

V7_PRIVATE struct v7_val *v7_mkvv(struct v7 *v7, enum v7_type t, va_list *ap) {
  struct v7_val *v = make_value(v7, t);

  /* TODO: check for make_value() failure */
  switch (t) {
    /* case V7_C_FUNC: v->v.c_func = va_arg(*ap, v7_func_t); break; */
    case V7_TYPE_NUM:
      v->v.num = va_arg(*ap, double);
      break;
    case V7_TYPE_STR: {
      char *buf = va_arg(*ap, char *);
      unsigned long len = va_arg(*ap, unsigned long);
      int own = va_arg(*ap, int);
      v7_init_str(v, buf, len, own);
    } break;
    default:
      break;
  }

  return v;
}

V7_PRIVATE struct v7_val *v7_mkv(struct v7 *v7, enum v7_type t, ...) {
  struct v7_val *v = NULL;
  va_list ap;

  va_start(ap, t);
  v = v7_mkvv(v7, t, &ap);
  va_end(ap);

  return v;
}

V7_PRIVATE enum v7_err v7_setv(struct v7 *v7, struct v7_val *obj,
                               enum v7_type key_type, enum v7_type val_type,
                               ...) {
  struct v7_val *k = NULL, *v = NULL;
  va_list ap;

  va_start(ap, val_type);
  k = key_type == V7_TYPE_OBJ ? va_arg(ap, struct v7_val *) : v7_mkvv(v7,
                                                                      key_type,
                                                                      &ap);
  v = val_type == V7_TYPE_OBJ ? va_arg(ap, struct v7_val *) : v7_mkvv(v7,
                                                                      val_type,
                                                                      &ap);
  va_end(ap);

  /* TODO: do not leak here */
  CHECK(k != NULL && v != NULL, V7_OUT_OF_MEMORY);

  INC_REF_COUNT(k);
  TRY(v7_set2(v7, obj, k, v));
  v7_freeval(v7, k);

  return V7_OK;
}

V7_PRIVATE enum v7_err push_number(struct v7 *v7, double num) {
  TRY(v7_make_and_push(v7, V7_TYPE_NUM));
  v7_init_num(v7_top_val(v7), num);
  return V7_OK;
}

V7_PRIVATE enum v7_err push_bool(struct v7 *v7, int is_true) {
  TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
  v7_init_bool(v7_top_val(v7), is_true);
  return V7_OK;
}

V7_PRIVATE enum v7_err push_string(struct v7 *v7, const char *str,
                                   unsigned long n, int own) {
  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v7_init_str(v7_top_val(v7), str, n, own);
  return V7_OK;
}

V7_PRIVATE enum v7_err push_func(struct v7 *v7, v7_func_t func) {
  TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
  v7_init_func(v7_top_val(v7), func);
  return V7_OK;
}

V7_PRIVATE enum v7_err push_new_object(struct v7 *v7) {
  TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
  v7_set_class(v7_top_val(v7), V7_CLASS_OBJECT);
  return V7_OK;
}

V7_PRIVATE const char *v7_strerror(enum v7_err e) {
  V7_PRIVATE const char *strings[] = {
      "no error",            "error",           "eval error",
      "range error",         "reference error", "syntax error",
      "type error",          "URI error",       "out of memory",
      "internal error",      "stack overflow",  "stack underflow",
      "called non-function", "not implemented", "string literal too long",
      "RegExp error"};
  assert(ARRAY_SIZE(strings) == V7_NUM_ERRORS);
  return e >= (int)ARRAY_SIZE(strings) ? "?" : strings[e];
}

V7_PRIVATE struct v7_val **v7_top(struct v7 *v7) { return &v7->stack[v7->sp]; }

V7_PRIVATE int v7_sp(struct v7 *v7) { return (int) (v7_top(v7) - v7->stack); }

V7_PRIVATE struct v7_val *v7_top_val(struct v7 *v7) {
  return v7->sp > 0 ? v7->stack[v7->sp - 1] : NULL;
}

V7_PRIVATE enum v7_err v7_push(struct v7 *v7, struct v7_val *v) {
  INC_REF_COUNT(v);
  TRY(inc_stack(v7, 1));
  v7->stack[v7->sp - 1] = v;
  return V7_OK;
}

V7_PRIVATE enum v7_err v7_make_and_push(struct v7 *v7, enum v7_type type) {
  struct v7_val *v = make_value(v7, type);
  CHECK(v != NULL, V7_OUT_OF_MEMORY);
  return v7_push(v7, v);
}

V7_PRIVATE enum v7_err v7_del2(struct v7 *v7, struct v7_val *obj,
                               const char *key, unsigned long n) {
  struct v7_val k = str_to_val(key, n);
  struct v7_prop **p;
  CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_ERROR);
  for (p = &obj->props; *p != NULL; p = &p[0]->next) {
    if (cmp(&k, p[0]->key) == 0) {
      struct v7_prop *next = p[0]->next;
      free_prop(v7, p[0]);
      p[0] = next;
      break;
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err do_exec(struct v7 *v7, const char *file_name,
                               const char *source_code, int sp) {
  int has_ret = 0;
  struct v7_pstate old_pstate = v7->pstate;
  enum v7_err err = V7_OK;

  v7->pstate.source_code = v7->pstate.pc = source_code;
  v7->pstate.file_name = file_name;
  v7->pstate.line_no = 1;

  /* Prior calls to v7_exec() may have left current_scope modified, reset now */
  /* TODO(lsm): free scope chain */
  v7->this_obj = &v7->root_scope;

  next_tok(v7);
  while ((err == V7_OK) && (v7->cur_tok != TOK_END_OF_INPUT)) {
    /* Reset stack on each statement */
    if ((err = inc_stack(v7, sp - v7->sp)) == V7_OK) {
      err = parse_statement(v7, &has_ret);
    }
  }

  assert(v7->root_scope.proto == &s_global);
  v7->pstate = old_pstate;

  return err;
}

/* Convert object to string, push string on stack */
V7_PRIVATE enum v7_err toString(struct v7 *v7, struct v7_val *obj) {
  struct v7_val *f = NULL;

  if ((f = v7_get(obj, "toString")) == NULL) {
    f = v7_get(&s_prototypes[V7_CLASS_OBJECT], "toString");
  }
  CHECK(f != NULL, V7_INTERNAL_ERROR);
  TRY(v7_push(v7, f));
  TRY(v7_call2(v7, obj, 0, 0));

  return V7_OK;
}

V7_PRIVATE enum v7_err check_str_re_conv(struct v7 *v7, struct v7_val **arg,
                                         int re_fl) {
  /* If argument is not (RegExp + re_fl) or string, do type conversion */
  if (!is_string(*arg) &&
      !(re_fl && instanceof(*arg, &s_constructors[V7_CLASS_REGEXP]))) {
    TRY(toString(v7, *arg));
    *arg = v7_top_val(v7);
    INC_REF_COUNT(*arg);
    TRY(inc_stack(v7, -2));
    TRY(v7_push(v7, *arg));
  }
  return V7_OK;
}

V7_PRIVATE double _conv_to_num(struct v7 *v7, struct v7_val *arg) {
  _prop_func_2_value(v7, &arg);
  if (is_num(arg) || is_bool(arg)) return arg->v.num;
  if (is_string(arg)) return strtod(arg->v.str.buf, NULL);
  return NAN;
}

V7_PRIVATE long _conv_to_int(struct v7 *v7, struct v7_val *arg) {
  double tmp = _conv_to_num(v7, arg);
  if (isnan(tmp) || isinf(tmp)) return 0;
  return tmp;
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


#ifndef V7_DISABLE_CRYPTO

/*************************** START OF MD5 THIRD PARTY CODE */
/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

#define	MD5_BLOCK_LENGTH		64
#define	MD5_DIGEST_LENGTH		16

typedef struct MD5Context {
  uint32_t state[4];			/* state */
  uint64_t count;			/* number of bits, mod 2^64 */
  uint8_t buffer[MD5_BLOCK_LENGTH];	/* input buffer */
} MD5_CTX;

#define PUT_64BIT_LE(cp, value) do {					\
(cp)[7] = (value) >> 56;					\
(cp)[6] = (value) >> 48;					\
(cp)[5] = (value) >> 40;					\
(cp)[4] = (value) >> 32;					\
(cp)[3] = (value) >> 24;					\
(cp)[2] = (value) >> 16;					\
(cp)[1] = (value) >> 8;						\
(cp)[0] = (value); } while (0)

#define PUT_32BIT_LE(cp, value) do {					\
(cp)[3] = (value) >> 24;					\
(cp)[2] = (value) >> 16;					\
(cp)[1] = (value) >> 8;						\
(cp)[0] = (value); } while (0)

static uint8_t PADDING[MD5_BLOCK_LENGTH] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void MD5Init(MD5_CTX *ctx) {
  ctx->count = 0;
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xefcdab89;
  ctx->state[2] = 0x98badcfe;
  ctx->state[3] = 0x10325476;
}

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

static void MD5Transform(uint32_t state[4],
                         const uint8_t block[MD5_BLOCK_LENGTH]) {
  uint32_t a, b, c, d, in[MD5_BLOCK_LENGTH / 4];

#if BYTE_ORDER == LITTLE_ENDIAN
  memcpy(in, block, sizeof(in));
#else
  for (a = 0; a < MD5_BLOCK_LENGTH / 4; a++) {
    in[a] = (u_int32_t)(
                        (u_int32_t)(block[a * 4 + 0]) |
                        (u_int32_t)(block[a * 4 + 1]) <<  8 |
                        (u_int32_t)(block[a * 4 + 2]) << 16 |
                        (u_int32_t)(block[a * 4 + 3]) << 24);
  }
#endif

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];

  MD5STEP(F1, a, b, c, d, in[ 0] + 0xd76aa478,  7);
  MD5STEP(F1, d, a, b, c, in[ 1] + 0xe8c7b756, 12);
  MD5STEP(F1, c, d, a, b, in[ 2] + 0x242070db, 17);
  MD5STEP(F1, b, c, d, a, in[ 3] + 0xc1bdceee, 22);
  MD5STEP(F1, a, b, c, d, in[ 4] + 0xf57c0faf,  7);
  MD5STEP(F1, d, a, b, c, in[ 5] + 0x4787c62a, 12);
  MD5STEP(F1, c, d, a, b, in[ 6] + 0xa8304613, 17);
  MD5STEP(F1, b, c, d, a, in[ 7] + 0xfd469501, 22);
  MD5STEP(F1, a, b, c, d, in[ 8] + 0x698098d8,  7);
  MD5STEP(F1, d, a, b, c, in[ 9] + 0x8b44f7af, 12);
  MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
  MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
  MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122,  7);
  MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
  MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
  MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

  MD5STEP(F2, a, b, c, d, in[ 1] + 0xf61e2562,  5);
  MD5STEP(F2, d, a, b, c, in[ 6] + 0xc040b340,  9);
  MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
  MD5STEP(F2, b, c, d, a, in[ 0] + 0xe9b6c7aa, 20);
  MD5STEP(F2, a, b, c, d, in[ 5] + 0xd62f105d,  5);
  MD5STEP(F2, d, a, b, c, in[10] + 0x02441453,  9);
  MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
  MD5STEP(F2, b, c, d, a, in[ 4] + 0xe7d3fbc8, 20);
  MD5STEP(F2, a, b, c, d, in[ 9] + 0x21e1cde6,  5);
  MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6,  9);
  MD5STEP(F2, c, d, a, b, in[ 3] + 0xf4d50d87, 14);
  MD5STEP(F2, b, c, d, a, in[ 8] + 0x455a14ed, 20);
  MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905,  5);
  MD5STEP(F2, d, a, b, c, in[ 2] + 0xfcefa3f8,  9);
  MD5STEP(F2, c, d, a, b, in[ 7] + 0x676f02d9, 14);
  MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

  MD5STEP(F3, a, b, c, d, in[ 5] + 0xfffa3942,  4);
  MD5STEP(F3, d, a, b, c, in[ 8] + 0x8771f681, 11);
  MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
  MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
  MD5STEP(F3, a, b, c, d, in[ 1] + 0xa4beea44,  4);
  MD5STEP(F3, d, a, b, c, in[ 4] + 0x4bdecfa9, 11);
  MD5STEP(F3, c, d, a, b, in[ 7] + 0xf6bb4b60, 16);
  MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
  MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6,  4);
  MD5STEP(F3, d, a, b, c, in[ 0] + 0xeaa127fa, 11);
  MD5STEP(F3, c, d, a, b, in[ 3] + 0xd4ef3085, 16);
  MD5STEP(F3, b, c, d, a, in[ 6] + 0x04881d05, 23);
  MD5STEP(F3, a, b, c, d, in[ 9] + 0xd9d4d039,  4);
  MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
  MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
  MD5STEP(F3, b, c, d, a, in[2 ] + 0xc4ac5665, 23);

  MD5STEP(F4, a, b, c, d, in[ 0] + 0xf4292244,  6);
  MD5STEP(F4, d, a, b, c, in[7 ] + 0x432aff97, 10);
  MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
  MD5STEP(F4, b, c, d, a, in[5 ] + 0xfc93a039, 21);
  MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3,  6);
  MD5STEP(F4, d, a, b, c, in[3 ] + 0x8f0ccc92, 10);
  MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
  MD5STEP(F4, b, c, d, a, in[1 ] + 0x85845dd1, 21);
  MD5STEP(F4, a, b, c, d, in[8 ] + 0x6fa87e4f,  6);
  MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
  MD5STEP(F4, c, d, a, b, in[6 ] + 0xa3014314, 15);
  MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
  MD5STEP(F4, a, b, c, d, in[4 ] + 0xf7537e82,  6);
  MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
  MD5STEP(F4, c, d, a, b, in[2 ] + 0x2ad7d2bb, 15);
  MD5STEP(F4, b, c, d, a, in[9 ] + 0xeb86d391, 21);

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
}

static void MD5Update(MD5_CTX *ctx, const unsigned char *input, size_t len) {
  size_t have, need;

  /* Check how many bytes we already have and how many more we need. */
  have = (size_t)((ctx->count >> 3) & (MD5_BLOCK_LENGTH - 1));
  need = MD5_BLOCK_LENGTH - have;

  /* Update bitcount */
  ctx->count += (uint64_t)len << 3;

  if (len >= need) {
    if (have != 0) {
      memcpy(ctx->buffer + have, input, need);
      MD5Transform(ctx->state, ctx->buffer);
      input += need;
      len -= need;
      have = 0;
    }

    /* Process data in MD5_BLOCK_LENGTH-byte chunks. */
    while (len >= MD5_BLOCK_LENGTH) {
      MD5Transform(ctx->state, input);
      input += MD5_BLOCK_LENGTH;
      len -= MD5_BLOCK_LENGTH;
    }
  }

  /* Handle any remaining bytes of data. */
  if (len != 0)
    memcpy(ctx->buffer + have, input, len);
}

static void MD5Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5_CTX *ctx) {
  uint8_t count[8];
  size_t padlen;
  int i;

  /* Convert count to 8 bytes in little endian order. */
  PUT_64BIT_LE(count, ctx->count);

  /* Pad out to 56 mod 64. */
  padlen = MD5_BLOCK_LENGTH -
  ((ctx->count >> 3) & (MD5_BLOCK_LENGTH - 1));
  if (padlen < 1 + 8)
    padlen += MD5_BLOCK_LENGTH;
  MD5Update(ctx, PADDING, padlen - 8);		/* padlen - 8 <= 64 */
  MD5Update(ctx, count, 8);

  if (digest != NULL) {
    for (i = 0; i < 4; i++)
      PUT_32BIT_LE(digest + i * 4, ctx->state[i]);
  }
  memset(ctx, 0, sizeof(*ctx));	/* in case it's sensitive */
}
/********************************** END OF MD5 THIRD PARTY CODE */

/********************************** START OF SHA-1 THIRD PARTY CODE */
/*
 * SHA-1 in C
 * By Steve Reid <sreid@sea-to-sky.net>
 * 100% Public Domain
 */

#define SHA1HANDSOFF
#if defined(__sun)
#include "solarisfixes.h"
#endif

union char64long16 {
  unsigned char c[64];
  uint32_t l[16];
};

static int is_big_endian(void) {
  static const int n = 1;
  return ((char *) &n)[0] == 0;
}

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

static uint32_t blk0(union char64long16 *block, int i) {
  /* Forrest: SHA expect BIG_ENDIAN, swap if LITTLE_ENDIAN */
  if (!is_big_endian()) {
    block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) |
    (rol(block->l[i], 8) & 0x00FF00FF);
  }
  return block->l[i];
}

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
^block->l[(i+2)&15]^block->l[i&15],1))
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(block, i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

typedef struct {
  uint32_t state[5];
  uint32_t count[2];
  unsigned char buffer[64];
} SHA1_CTX;

static void SHA1Transform(uint32_t state[5], const unsigned char buffer[64]) {
  uint32_t a, b, c, d, e;
  union char64long16 block[1];

  memcpy(block, buffer, 64);
  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];
  R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
  R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
  R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
  R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
  R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
  R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
  R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
  R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
  R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
  R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
  R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
  R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
  R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
  R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
  R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
  R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
  R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
  R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
  R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
  R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  /*
   * Erase working structures. The order of operations is important,
   * used to ensure that compiler doesn't optimize those out.
   */
  memset(block, 0, sizeof(block));
  a = b = c = d = e = 0;
  (void) a; (void) b; (void) c; (void) d; (void) e;
}

static void SHA1Init(SHA1_CTX *context) {
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0xC3D2E1F0;
  context->count[0] = context->count[1] = 0;
}

static void SHA1Update(SHA1_CTX *context, const unsigned char *data,
                       uint32_t len) {
  uint32_t i, j;

  j = context->count[0];
  if ((context->count[0] += len << 3) < j)
    context->count[1]++;
  context->count[1] += (len>>29);
  j = (j >> 3) & 63;
  if ((j + len) > 63) {
    memcpy(&context->buffer[j], data, (i = 64-j));
    SHA1Transform(context->state, context->buffer);
    for ( ; i + 63 < len; i += 64) {
      SHA1Transform(context->state, &data[i]);
    }
    j = 0;
  }
  else i = 0;
  memcpy(&context->buffer[j], &data[i], len - i);
}

static void SHA1Final(unsigned char digest[20], SHA1_CTX *context) {
  unsigned i;
  unsigned char finalcount[8], c;

  for (i = 0; i < 8; i++) {
    finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
                                     >> ((3-(i & 3)) * 8) ) & 255);
  }
  c = 0200;
  SHA1Update(context, &c, 1);
  while ((context->count[0] & 504) != 448) {
    c = 0000;
    SHA1Update(context, &c, 1);
  }
  SHA1Update(context, finalcount, 8);
  for (i = 0; i < 20; i++) {
    digest[i] = (unsigned char)
    ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
  }
  memset(context, '\0', sizeof(*context));
  memset(&finalcount, '\0', sizeof(finalcount));
}
/********************************** START OF SHA-1 THIRD PARTY CODE */


static struct v7_val s_crypto = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);

static void v7_md5(const struct v7_val *v, char *buf) {
  MD5_CTX ctx;
  MD5Init(&ctx);
  MD5Update(&ctx, (const unsigned char *) v->v.str.buf, v->v.str.len);
  MD5Final((unsigned char *) buf, &ctx);
}

static void v7_sha1(const struct v7_val *v, char *buf) {
  SHA1_CTX ctx;
  SHA1Init(&ctx);
  SHA1Update(&ctx, (const unsigned char *) v->v.str.buf,(uint32_t)v->v.str.len);
  SHA1Final((unsigned char *) buf, &ctx);
}

static void bin2str(char *to, const unsigned char *p, size_t len) {
  static const char *hex = "0123456789abcdef";

  for (; len--; p++) {
    *to++ = hex[p[0] >> 4];
    *to++ = hex[p[0] & 0x0f];
  }
  *to = '\0';
}

V7_PRIVATE enum v7_err Crypto_md5(struct v7_c_func_arg *cfa) {
  if (cfa->num_args == 1 && cfa->args[0]->type == V7_TYPE_STR) {
    char buf[16];
    v7_md5(cfa->args[0], buf);
    v7_push_string(cfa->v7, buf, sizeof(buf), 1);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Crypto_md5_hex(struct v7_c_func_arg *cfa) {
  if (cfa->num_args == 1 && cfa->args[0]->type == V7_TYPE_STR) {
    char hash[16], buf[sizeof(hash) * 2];
    v7_md5(cfa->args[0], hash);
    bin2str(buf, (unsigned char *) hash, sizeof(hash));
    v7_push_string(cfa->v7, buf, sizeof(buf), 1);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Crypto_sha1(struct v7_c_func_arg *cfa) {
  if (cfa->num_args == 1 && cfa->args[0]->type == V7_TYPE_STR) {
    char buf[20];
    v7_sha1(cfa->args[0], buf);
    v7_push_string(cfa->v7, buf, sizeof(buf), 1);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Crypto_sha1_hex(struct v7_c_func_arg *cfa) {
  if (cfa->num_args == 1 && cfa->args[0]->type == V7_TYPE_STR) {
    char hash[20], buf[sizeof(hash) * 2];
    v7_sha1(cfa->args[0], hash);
    bin2str(buf, (unsigned char *) hash, sizeof(hash));
    v7_push_string(cfa->v7, buf, sizeof(buf), 1);
  }
  return V7_OK;
}

V7_PRIVATE void init_crypto(void) {
  SET_METHOD(s_crypto, "md5", Crypto_md5);
  SET_METHOD(s_crypto, "md5_hex", Crypto_md5_hex);
  SET_METHOD(s_crypto, "sha1", Crypto_sha1);
  SET_METHOD(s_crypto, "sha1_hex", Crypto_sha1_hex);

  v7_set_class(&s_crypto, V7_CLASS_OBJECT);
  INC_REF_COUNT(&s_crypto);

  SET_RO_PROP_V(s_global, "Crypto", s_crypto);
}
#endif  /* V7_DISABLE_CRYPTO */
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_err Array_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj =
      cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_ARRAY);
  return V7_OK;
}
V7_PRIVATE void Arr_length(struct v7_val *this_obj, struct v7_val *arg,
                           struct v7_val *result) {
  struct v7_prop *p;
  if (NULL == result || arg) return;
  v7_init_num(result, 0.0);
  for (p = this_obj->v.array; p != NULL; p = p->next) {
    result->v.num += 1.0;
  }
}

V7_PRIVATE enum v7_err Arr_push(struct v7_c_func_arg *cfa) {
  int i;
  for (i = 0; i < cfa->num_args; i++) {
    v7_append(cfa->v7, cfa->this_obj, cfa->args[i]);
  }
  return V7_OK;
}

V7_PRIVATE int cmp_prop(const void *pa, const void *pb) {
  const struct v7_prop *p1 = *(struct v7_prop **) pa;
  const struct v7_prop *p2 = *(struct v7_prop **) pb;
  return cmp(p2->val, p1->val);
}

V7_PRIVATE enum v7_err Arr_sort(struct v7_c_func_arg *cfa) {
  int i = 0, length = 0;
  struct v7_val *v = cfa->this_obj;
  struct v7_prop *p, **arr;

  /* TODO(lsm): do proper error checking */
  for (p = v->v.array; p != NULL; p = p->next) {
    length++;
  }
  arr = (struct v7_prop **) malloc(length * sizeof(p));
  for (i = 0, p = v->v.array; p != NULL; p = p->next) {
    arr[i++] = p;
  }
  qsort(arr, length, sizeof(p), cmp_prop);
  v->v.array = NULL;
  for (i = 0; i < length; i++) {
    arr[i]->next = v->v.array;
    v->v.array = arr[i];
  }
  free(arr);
  return V7_OK;
}

V7_PRIVATE void init_array(void) {
  init_standard_constructor(V7_CLASS_ARRAY, Array_ctor);

  SET_PROP_FUNC(s_prototypes[V7_CLASS_ARRAY], "length", Arr_length);
  SET_METHOD(s_prototypes[V7_CLASS_ARRAY], "push", Arr_push);
  SET_METHOD(s_prototypes[V7_CLASS_ARRAY], "sort", Arr_sort);

  SET_RO_PROP_V(s_global, "Array", s_constructors[V7_CLASS_ARRAY]);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_err Boolean_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj =
      cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_BOOLEAN);
  return V7_OK;
}

V7_PRIVATE void init_boolean(void) {
  init_standard_constructor(V7_CLASS_BOOLEAN, Boolean_ctor);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_err Date_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj :
                       v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_DATE);
  return V7_OK;
}

V7_PRIVATE void init_date(void) {
  init_standard_constructor(V7_CLASS_DATE, Date_ctor);
  SET_RO_PROP_V(s_global, "Date", s_constructors[V7_CLASS_DATE]);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_err Error_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj =
      cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_ERROR);
  return V7_OK;
}

V7_PRIVATE void init_error(void) {
  init_standard_constructor(V7_CLASS_ERROR, Error_ctor);
  SET_RO_PROP_V(s_global, "Error", s_constructors[V7_CLASS_ERROR]);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_err Function_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj =
      cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_FUNCTION);
  return V7_OK;
}

V7_PRIVATE void init_function(void) {
  init_standard_constructor(V7_CLASS_FUNCTION, Function_ctor);
  SET_RO_PROP_V(s_global, "Function", s_constructors[V7_CLASS_FUNCTION]);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_err Math_random(struct v7_c_func_arg *cfa) {
  static int srand_called = 0;

  if (!srand_called) {
    srand((unsigned)(unsigned long) cfa);
    srand_called++;
  }

  v7_push_number(cfa->v7, (double)rand() / RAND_MAX);

  return V7_OK;
}

V7_PRIVATE enum v7_err Math_sin(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 1 ? sin(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_sqrt(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 1 ? sqrt(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_tan(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 1 ? tan(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_pow(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 2
                              ? pow(cfa->args[0]->v.num, cfa->args[1]->v.num)
                              : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_floor(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7,
                 cfa->num_args == 1 ? floor(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_ceil(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 1 ? ceil(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE void init_math(void) {
  SET_METHOD(s_math, "random", Math_random);
  SET_METHOD(s_math, "pow", Math_pow);
  SET_METHOD(s_math, "sin", Math_sin);
  SET_METHOD(s_math, "tan", Math_tan);
  SET_METHOD(s_math, "sqrt", Math_sqrt);
  SET_METHOD(s_math, "floor", Math_floor);
  SET_METHOD(s_math, "ceil", Math_ceil);

  SET_RO_PROP(s_math, "E", V7_TYPE_NUM, num, M_E);
  SET_RO_PROP(s_math, "PI", V7_TYPE_NUM, num, M_PI);
  SET_RO_PROP(s_math, "LN2", V7_TYPE_NUM, num, M_LN2);
  SET_RO_PROP(s_math, "LN10", V7_TYPE_NUM, num, M_LN10);
  SET_RO_PROP(s_math, "LOG2E", V7_TYPE_NUM, num, M_LOG2E);
  SET_RO_PROP(s_math, "LOG10E", V7_TYPE_NUM, num, M_LOG10E);
  SET_RO_PROP(s_math, "SQRT1_2", V7_TYPE_NUM, num, M_SQRT1_2);
  SET_RO_PROP(s_math, "SQRT2", V7_TYPE_NUM, num, M_SQRT2);

  v7_set_class(&s_math, V7_CLASS_OBJECT);
  s_math.ref_count = 1;

  SET_RO_PROP_V(s_global, "Math", s_math);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_err Number_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj =
      cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  struct v7_val *arg = cfa->args[0];

  v7_init_num(obj, cfa->num_args > 0 ? arg->v.num : 0.0);

  if (cfa->num_args > 0 && !is_num(arg) && !is_bool(arg)) {
    if (is_string(arg)) {
      v7_init_num(obj, strtod(arg->v.str.buf, NULL));
    } else {
      v7_init_num(obj, NAN);
    }
  }

  if (cfa->called_as_constructor) {
    obj->type = V7_TYPE_OBJ;
    obj->proto = &s_prototypes[V7_CLASS_NUMBER];
    obj->ctor = &s_constructors[V7_CLASS_NUMBER];
  }

  return V7_OK;
}

V7_PRIVATE enum v7_err Num_toFixed(struct v7_c_func_arg *cfa) {
  int len, digits = cfa->num_args > 0 ? (int)cfa->args[0]->v.num : 0;
  char fmt[10], buf[100];

  snprintf(fmt, sizeof(fmt), "%%.%dlf", digits);
  len = snprintf(buf, sizeof(buf), fmt, cfa->this_obj->v.num);
  v7_push_string(cfa->v7, buf, len, 1);
  return V7_OK;
}

#define NUMCTOR s_constructors[V7_CLASS_NUMBER]
V7_PRIVATE void init_number(void) {
  init_standard_constructor(V7_CLASS_NUMBER, Number_ctor);
  SET_RO_PROP(NUMCTOR, "MAX_VALUE", V7_TYPE_NUM, num, LONG_MAX);
  SET_RO_PROP(NUMCTOR, "MIN_VALUE", V7_TYPE_NUM, num, LONG_MIN);
  SET_RO_PROP(NUMCTOR, "NaN", V7_TYPE_NUM, num, NAN);
  SET_METHOD(s_prototypes[V7_CLASS_NUMBER], "toFixed", Num_toFixed);
  SET_RO_PROP_V(s_global, "Number", s_constructors[V7_CLASS_NUMBER]);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_err Object_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj =
      cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_OBJECT);
  return V7_OK;
}

V7_PRIVATE enum v7_err Obj_toString(struct v7_c_func_arg *cfa) {
  char *p, buf[500];
  p = v7_stringify(cfa->this_obj, buf, sizeof(buf));
  v7_push_string(cfa->v7, p, strlen(p), 1);
  if (p != buf) free(p);
  return V7_OK;
}

V7_PRIVATE enum v7_err Obj_keys(struct v7_c_func_arg *cfa) {
  struct v7_prop *p;
  struct v7_val *result = v7_push_new_object(cfa->v7);
  v7_set_class(result, V7_CLASS_ARRAY);
  for (p = cfa->this_obj->props; p != NULL; p = p->next) {
    v7_append(cfa->v7, result, p->key);
  }
  return V7_OK;
}

V7_PRIVATE void init_object(void) {
  init_standard_constructor(V7_CLASS_OBJECT, Object_ctor);
  SET_METHOD(s_prototypes[V7_CLASS_OBJECT], "toString", Obj_toString);
  SET_METHOD(s_prototypes[V7_CLASS_OBJECT], "keys", Obj_keys);
  SET_RO_PROP_V(s_global, "Object", s_constructors[V7_CLASS_OBJECT]);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


struct re_env {
  struct v7_val_flags flags;
  const char *src;
  Rune curr_rune;

  struct Reprog *prog;
  struct Renode *pstart, *pend;

  struct Renode *sub[RE_MAX_SUB];
  unsigned int subexpr_num;
  unsigned int sets_num;

  int lookahead;
  struct Reclass *curr_set;
  int min_rep, max_rep;

  jmp_buf catch_point;
  const char *err_msg;
};

enum RE_CODE {
  I_END = 10,  /* Terminate: match found */
  I_ANY,
  P_ANY = I_ANY,  /* Any character except newline, . */
  I_ANYNL,        /* Any character including newline, . */
  I_BOL,
  P_BOL = I_BOL,  /* Beginning of line, ^ */
  I_CH,
  P_CH = I_CH,
  I_EOL,
  P_EOL = I_EOL,  /* End of line, $ */
  I_EOS,
  P_EOS = I_EOS,  /* End of string, \0 */
  I_JUMP,
  I_LA,
  P_LA = I_LA,
  I_LA_N,
  P_LA_N = I_LA_N,
  I_LBRA,
  P_BRA = I_LBRA,  /* Left bracket, ( */
  I_REF,
  P_REF = I_REF,
  I_REP,
  P_REP = I_REP,
  I_REP_INI,
  I_RBRA,  /* Right bracket, ) */
  I_SET,
  P_SET = I_SET,  /* Character set, [] */
  I_SET_N,
  P_SET_N = I_SET_N,  /* Negated character set, [] */
  I_SPLIT,
  I_WORD,
  P_WORD = I_WORD,
  I_WORD_N,
  P_WORD_N = I_WORD_N,
  P_ALT,  /* Alternation, | */
  P_CAT,  /* Concatentation, implicit operator */
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

enum RE_MESSAGE {
  INVALID_DEC_DIGIT = -1,
  INVALID_HEX_DIGIT = -2,
  INVALID_ESC_CHAR = -3,
  UNTERM_ESC_SEQ = -4,
  SYNTAX_ERROR = -5,
  UNMATCH_LBR = -6,
  UNMATCH_RBR = -7,
  NUM_OVERFLOW = -8,
  INF_LOOP_M_EMP_STR = -9,
  TOO_MANY_CH_SETS = -10,
  INV_CH_SET_RANGE = -11,
  CH_SET_TOO_LARGE = -12,
  MALFORMED_CH_SET = -13,
  INVALID_BACK_REF = -14,
  TOO_MANY_CAPTURES = -15,
  INVALID_QUANTIFIER = -16,

  BAD_CHAR_AFTER_USD = -64
};

static const char *re_err_msg(enum RE_MESSAGE err) {
  switch (err) {
    case INVALID_DEC_DIGIT:
      return "invalid DEC digit";
    case INVALID_HEX_DIGIT:
      return "invalid HEX digit";
    case INVALID_ESC_CHAR:
      return "invalid escape character";
    case UNTERM_ESC_SEQ:
      return "unterminated escape sequence";
    case SYNTAX_ERROR:
      return "syntax error";
    case UNMATCH_LBR:
      return "'(' unmatched";
    case UNMATCH_RBR:
      return "')' unmatched";
    case NUM_OVERFLOW:
      return "numeric overflow";
    case INF_LOOP_M_EMP_STR:
      return "infinite loop matching the empty string";
    case TOO_MANY_CH_SETS:
      return "too many character sets";
    case INV_CH_SET_RANGE:
      return "invalid character set range";
    case CH_SET_TOO_LARGE:
      return "char set too large; increase struct Reclass.spans size";
    case MALFORMED_CH_SET:
      return "malformed '[]'";
    case INVALID_BACK_REF:
      return "invalid back-reference";
    case TOO_MANY_CAPTURES:
      return "too many captures";
    case INVALID_QUANTIFIER:
      return "invalid quantifier";

    case BAD_CHAR_AFTER_USD:
      return "bad character after '$' in replace pattern";
  }
  return "";
}

static sint8_t dec(int c) {
  if (isdigitrune(c)) return c - '0';
  return INVALID_DEC_DIGIT;
}

static uint8_t re_dec_digit(struct re_env *e, int c) {
  sint8_t ret = dec(c);
  if (ret < 0)
    V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(INVALID_DEC_DIGIT));
  return ret;
}

static sint8_t hex(int c) {
  if (isdigitrune(c)) return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return INVALID_HEX_DIGIT;
}

V7_PRIVATE sint8_t nextesc(Rune *r, const char **src) {
  sint8_t hd;
  *src += chartorune(r, *src);
  switch (*r) {
    case 0:
      return UNTERM_ESC_SEQ;
    case 'c':
      *r = **src & 31;
      ++*src;
      return 0;
    case 'f':
      *r = '\f';
      return 0;
    case 'n':
      *r = '\n';
      return 0;
    case 'r':
      *r = '\r';
      return 0;
    case 't':
      *r = '\t';
      return 0;
    case 'u':
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r = hd << 12;
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r += hd << 8;
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r += hd << 4;
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r += hd;
      if (!*r) {
        *r = '0';
        return 1;
      }
      return 0;
    case 'v':
      *r = '\v';
      return 0;
    case 'x':
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r = hd << 4;
      hd = hex(**src);
      ++*src;
      if (hd < 0) return INVALID_HEX_DIGIT;
      *r += hd;
      if (!*r) {
        *r = '0';
        return 1;
      }
      return 0;
  }
  return 2;
}

static sint8_t re_nextc(Rune *r, const char **src, uint8_t re_flag) {
  *src += chartorune(r, *src);
  if (re_flag && *r == '\\') {
    /* sint8_t ret = */ nextesc(r, src);
    /* if(2 != ret) return ret;
    if (!strchr("$()*+-./0123456789?BDSW[\\]^bdsw{|}", *r))
      return INVALID_ESC_CHAR; */
    return 1;
  }
  return 0;
}

static uint8_t re_nextc_env(struct re_env *e) {
  sint8_t ret = re_nextc(&e->curr_rune, &e->src, e->flags.re);
  if (ret < 0) V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(ret));
  return ret;
}

static void re_nchset(struct re_env *e) {
  if (e->sets_num >= nelem(e->prog->charset))
    V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(TOO_MANY_CH_SETS));
  e->curr_set = e->prog->charset + e->sets_num++;
  e->curr_set->end = e->curr_set->spans;
}

static void re_rng2set(struct re_env *e, Rune start, Rune end) {
  if (start > end)
    V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(INV_CH_SET_RANGE));
  if (e->curr_set->end + 2 == e->curr_set->spans + nelem(e->curr_set->spans))
    V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(CH_SET_TOO_LARGE));
  e->curr_set->end->s = start;
  e->curr_set->end->e = end;
  e->curr_set->end++;
}

#define re_char2set(e, c) re_rng2set(e, c, c)

#define re_d_2set(e) re_rng2set(e, '0', '9')

static void re_D_2set(struct re_env *e) {
  re_rng2set(e, 0, '0' - 1);
  re_rng2set(e, '9' + 1, 0xFFFF);
}

static void re_s_2set(struct re_env *e) {
  re_char2set(e, 0x9);
  re_rng2set(e, 0xA, 0xD);
  re_char2set(e, 0x20);
  re_char2set(e, 0xA0);
  re_rng2set(e, 0x2028, 0x2029);
  re_char2set(e, 0xFEFF);
}

static void re_S_2set(struct re_env *e) {
  re_rng2set(e, 0, 0x9 - 1);
  re_rng2set(e, 0xD + 1, 0x20 - 1);
  re_rng2set(e, 0x20 + 1, 0xA0 - 1);
  re_rng2set(e, 0xA0 + 1, 0x2028 - 1);
  re_rng2set(e, 0x2029 + 1, 0xFEFF - 1);
  re_rng2set(e, 0xFEFF + 1, 0xFFFF);
}

static void re_w_2set(struct re_env *e) {
  re_d_2set(e);
  re_rng2set(e, 'A', 'Z');
  re_char2set(e, '_');
  re_rng2set(e, 'a', 'z');
}

static void re_W_2set(struct re_env *e) {
  re_rng2set(e, 0, '0' - 1);
  re_rng2set(e, '9' + 1, 'A' - 1);
  re_rng2set(e, 'Z' + 1, '_' - 1);
  re_rng2set(e, '_' + 1, 'a' - 1);
  re_rng2set(e, 'z' + 1, 0xFFFF);
}

static uint8_t re_endofcount(Rune c) {
  switch (c) {
    case ',':
    case '}':
      return 1;
  }
  return 0;
}

static void re_ex_num_overfl(struct re_env *e) {
  V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(NUM_OVERFLOW));
}

static enum RE_CODE re_countrep(struct re_env *e) {
  e->min_rep = 0;
  while (!re_endofcount(e->curr_rune = *e->src++))
    e->min_rep = e->min_rep * 10 + re_dec_digit(e, e->curr_rune);
  if (e->min_rep >= RE_MAX_REP) re_ex_num_overfl(e);

  if (e->curr_rune != ',') {
    e->max_rep = e->min_rep;
    return L_COUNT;
  }
  e->max_rep = 0;
  while ((e->curr_rune = *e->src++) != '}')
    e->max_rep = e->max_rep * 10 + re_dec_digit(e, e->curr_rune);
  if (!e->max_rep) {
    e->max_rep = RE_MAX_REP;
    return L_COUNT;
  }
  if (e->max_rep >= RE_MAX_REP) re_ex_num_overfl(e);

  return L_COUNT;
}

static enum RE_CODE re_lexset(struct re_env *e) {
  Rune ch;
  uint8_t esc, ch_fl = 0, dash_fl = 0;
  enum RE_CODE type = L_SET;

  re_nchset(e);

  esc = re_nextc_env(e);
  if (!esc && e->curr_rune == '^') {
    type = L_SET_N;
    esc = re_nextc_env(e);
  }

  for (; esc || e->curr_rune != ']'; esc = re_nextc_env(e)) {
    if (!e->curr_rune)
      V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(MALFORMED_CH_SET));
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
            V7_EX_THROW(e->catch_point, e->err_msg,
            re_err_msg(INVALID_ESC_CHAR)); */
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

static int re_lexer(struct re_env *e) {
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

  if (e->flags.re) {
    switch (e->curr_rune) {
      case 0:
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

static struct Renode *re_nnode(struct re_env *e, int type) {
  memset(e->pend, 0, sizeof(struct Renode));
  e->pend->type = type;
  return e->pend++;
}

static uint8_t re_isemptynd(struct Renode *nd) {
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

static struct Renode *re_nrep(struct re_env *e, struct Renode *nd, int ng,
                              uint16_t min, uint16_t max) {
  struct Renode *rep = re_nnode(e, P_REP);
  if (max == RE_MAX_REP && re_isemptynd(nd))
    V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(INF_LOOP_M_EMP_STR));
  rep->par.xy.y.rp.ng = ng;
  rep->par.xy.y.rp.min = min;
  rep->par.xy.y.rp.max = max;
  rep->par.xy.x = nd;
  return rep;
}

static struct Renode *re_parser(struct re_env *e);

static struct Renode *re_parse_la(struct re_env *e) {
  struct Renode *nd;
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
      if (!e->curr_rune || e->curr_rune > e->subexpr_num ||
          !e->sub[e->curr_rune])
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(INVALID_BACK_REF));
      nd->par.xy.y.n = e->curr_rune;
      nd->par.xy.x = e->sub[e->curr_rune];
      RE_NEXT(e);
      break;
    case '.':
      RE_NEXT(e);
      nd = re_nnode(e, P_ANY);
      break;
    case '(':
      RE_NEXT(e);
      nd = re_nnode(e, P_BRA);
      if (e->subexpr_num == RE_MAX_SUB)
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(TOO_MANY_CAPTURES));
      nd->par.xy.y.n = e->subexpr_num++;
      nd->par.xy.x = re_parser(e);
      e->sub[nd->par.xy.y.n] = nd;
      if (!RE_ACCEPT(e, ')'))
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(UNMATCH_LBR));
      break;
    case L_LA:
      RE_NEXT(e);
      nd = re_nnode(e, P_LA);
      nd->par.xy.x = re_parser(e);
      if (!RE_ACCEPT(e, ')'))
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(UNMATCH_LBR));
      break;
    case L_LA_CAP:
      RE_NEXT(e);
      nd = re_parser(e);
      if (!RE_ACCEPT(e, ')'))
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(UNMATCH_LBR));
      break;
    case L_LA_N:
      RE_NEXT(e);
      nd = re_nnode(e, P_LA_N);
      nd->par.xy.x = re_parser(e);
      if (!RE_ACCEPT(e, ')'))
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(UNMATCH_LBR));
      break;
    default:
      V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(SYNTAX_ERROR));
  }

  switch (e->lookahead) {
    case '*':
      RE_NEXT(e);
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), 0, RE_MAX_REP);
    case '+':
      RE_NEXT(e);
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), 1, RE_MAX_REP);
    case '?':
      RE_NEXT(e);
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), 0, 1);
    case L_COUNT:
      min = e->min_rep, max = e->max_rep;
      RE_NEXT(e);
      if (max < min)
        V7_EX_THROW(e->catch_point, e->err_msg, re_err_msg(INVALID_QUANTIFIER));
      return re_nrep(e, nd, RE_ACCEPT(e, '?'), min, max);
  }
  return nd;
}

static uint8_t re_endofcat(Rune c, uint8_t re_flag) {
  switch (c) {
    case 0:
      return 1;
    case '|':
    case ')':
      if (re_flag) return 1;
  }
  return 0;
}

static struct Renode *re_parser(struct re_env *e) {
  struct Renode *alt = NULL, *cat, *nd;
  if (!re_endofcat(e->lookahead, e->flags.re)) {
    cat = re_parse_la(e);
    while (!re_endofcat(e->lookahead, e->flags.re)) {
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

static unsigned int re_nodelen(struct Renode *nd) {
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
          if (nd->par.xy.y.rp.max >= RE_MAX_REP)
            return re_nodelen(nd->par.xy.x) + 2;
        case 1:
          if (!n) return re_nodelen(nd->par.xy.x);
          if (nd->par.xy.y.rp.max >= RE_MAX_REP)
            return re_nodelen(nd->par.xy.x) + 1;
        default:
          n = 4;
          if (nd->par.xy.y.rp.max >= RE_MAX_REP) n++;
          return re_nodelen(nd->par.xy.x) + n;
      }
    default:
      return 1;
  }
}

static struct Reinst *re_newinst(struct Reprog *prog, int opcode) {
  memset(prog->end, 0, sizeof(struct Reinst));
  prog->end->opcode = opcode;
  return prog->end++;
}

static void re_compile(struct re_env *e, struct Renode *nd) {
  struct Reinst *inst, *split, *jump, *rep;
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
      if (e->flags.re_i) inst->par.c = tolowerrune(nd->par.c);
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
          if (nd->par.xy.y.rp.max >= RE_MAX_REP) {
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
          if (nd->par.xy.y.rp.max >= RE_MAX_REP) {
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
          if (nd->par.xy.y.rp.max >= RE_MAX_REP) {
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
static void print_set(struct Reclass *cp) {
  struct Rerange *p;
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

static void node_print(struct Renode *nd) {
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
      printf("(%d,", nd->par.xy.y.n);
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
      printf(nd->par.c >= 32 && nd->par.c < 127 ? "'%c'" :
             "'\\u%04X'", nd->par.c);
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

static void program_print(struct Reprog *prog) {
  struct Reinst *inst;
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
        printf(inst->par.c >= 32 && inst->par.c < 127 ? "'%c'\n" :
               "'\\u%04X'\n", inst->par.c);
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

struct Reprog *re_compiler(const char *pattern, struct v7_val_flags flags,
                           const char **p_err_msg) {
  struct re_env e;
  struct Renode *nd;
  struct Reinst *split, *jump;

  e.prog = reg_malloc(sizeof(struct Reprog));
  e.pstart = e.pend = reg_malloc(sizeof(struct Renode) * strlen(pattern) * 2);

  if (V7_EX_TRY_CATCH(e.catch_point)) {
    if (p_err_msg) *p_err_msg = e.err_msg;
    reg_free(e.pstart);
    reg_free(e.prog);
    return (struct Reprog *) -1;
  }

  e.src = pattern;
  e.sets_num = 0;
  e.subexpr_num = 1;
  e.flags = flags;
  memset(e.sub, 0, sizeof(e.sub));

  RE_NEXT(&e);
  nd = re_parser(&e);
  if (e.lookahead == ')')
    V7_EX_THROW(e.catch_point, e.err_msg, re_err_msg(UNMATCH_RBR));
  if (e.lookahead != 0)
    V7_EX_THROW(e.catch_point, e.err_msg, re_err_msg(SYNTAX_ERROR));

  e.prog->subexpr_num = e.subexpr_num;
  e.prog->start = e.prog->end =
      reg_malloc((re_nodelen(nd) + 6) * sizeof(struct Reinst));

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

  reg_free(e.pstart);

  if (p_err_msg) *p_err_msg = NULL;
  return e.prog;
}

void re_free(struct Reprog *prog) {
  if (prog) {
    reg_free(prog->start);
    reg_free(prog);
  }
}

static void re_newthread(struct Rethread *t, struct Reinst *pc,
                         const char *start, struct Resub *sub) {
  t->pc = pc;
  t->start = start;
  t->sub = *sub;
}

#define RE_NO_MATCH() \
  if (!(thr = 0)) continue

static uint8_t re_match(struct Reinst *pc, const char *start, const char *bol,
                        struct v7_val_flags flags, struct Resub *loot) {
  struct Rethread threads[RE_MAX_THREADS];
  struct Resub sub, tmpsub;
  Rune c, r;
  struct Rerange *p;
  uint16_t thr_num = 1;
  uint8_t thr;
  int i;

  /* queue initial thread */
  re_newthread(threads, pc, start, loot);

  /* run threads in stack order */
  do {
    pc = threads[--thr_num].pc;
    start = threads[thr_num].start;
    sub = threads[thr_num].sub;
    for (thr = 1; thr;) {
      switch (pc->opcode) {
        case I_END:
          memcpy(loot->sub, sub.sub, sizeof loot->sub);
          return 1;
        case I_ANY:
        case I_ANYNL:
          start += chartorune(&c, start);
          if (!c || (pc->opcode == I_ANY && isnewline(c))) RE_NO_MATCH();
          break;

        case I_BOL:
          if (start == bol) break;
          if (flags.re_m && isnewline(start[-1])) break;
          RE_NO_MATCH();
        case I_CH:
          start += chartorune(&c, start);
          if (c && (flags.re_i ? tolowerrune(c) : c) == pc->par.c) break;
          RE_NO_MATCH();
        case I_EOL:
          if (!*start) break;
          if (flags.re_m && isnewline(*start)) break;
          RE_NO_MATCH();
        case I_EOS:
          if (!*start) break;
          RE_NO_MATCH();

        case I_JUMP:
          pc = pc->par.xy.x;
          continue;

        case I_LA:
          if (re_match(pc->par.xy.x, start, bol, flags, &sub)) {
            pc = pc->par.xy.y.y;
            continue;
          }
          RE_NO_MATCH();
        case I_LA_N:
          tmpsub = sub;
          if (!re_match(pc->par.xy.x, start, bol, flags, &tmpsub)) {
            pc = pc->par.xy.y.y;
            continue;
          }
          RE_NO_MATCH();

        case I_LBRA:
          sub.sub[pc->par.n].start = start;
          break;

        case I_REF:
          i = sub.sub[pc->par.n].end - sub.sub[pc->par.n].start;
          if (flags.re_i) {
            int num = i;
            const char *s = start, *p = sub.sub[pc->par.n].start;
            Rune rr;
            for (; num && *s && *p; num--) {
              s += chartorune(&r, s);
              p += chartorune(&rr, p);
              if (tolowerrune(r) != tolowerrune(rr)) break;
            }
            if (num) RE_NO_MATCH();
          } else if (strncmp(start, sub.sub[pc->par.n].start, i))
            RE_NO_MATCH();
          if (i > 0) start += i;
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
          sub.sub[pc->par.n].end = start;
          break;

        case I_SET:
        case I_SET_N:
          start += chartorune(&c, start);
          if (!c) RE_NO_MATCH();

          i = 1;
          for (p = pc->par.cp->spans; i && p < pc->par.cp->end; p++)
            if (flags.re_i) {
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
          if (thr_num >= RE_MAX_THREADS) {
            fprintf(stderr, "re_match: backtrack overflow!\n");
            return 0;
          }
          re_newthread(&threads[thr_num++], pc->par.xy.y.y, start, &sub);
          pc = pc->par.xy.x;
          continue;

        case I_WORD:
        case I_WORD_N:
          i = (start > bol && iswordchar(start[-1]));
          if (iswordchar(start[0])) i = !i;
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

uint8_t re_exec(struct Reprog *prog, struct v7_val_flags flags,
                const char *start, struct Resub *loot) {
  struct Resub tmpsub;
  const char *st = start;

  if (loot) memset(loot, 0, sizeof(*loot));
  if (!flags.re_g || !loot) {
    if (!loot) loot = &tmpsub;
    loot->subexpr_num = prog->subexpr_num;
    return !re_match(prog->start, start, start, flags, loot);
  }
  while (re_match(prog->start, st, start, flags, &tmpsub)) {
    unsigned int i;
    st = tmpsub.sub[0].end;
    for (i = 0; i < prog->subexpr_num; i++) {
      struct re_tok *l = &loot->sub[loot->subexpr_num + i], *s = &tmpsub.sub[i];
      l->start = s->start;
      l->end = s->end;
    }
    loot->subexpr_num += prog->subexpr_num;
  }
  return !loot->subexpr_num;
}

V7_PRIVATE int re_rplc(struct Resub *loot, const char *src, const char *rstr,
                       struct Resub *dstsub) {
  int size = 0, n;
  Rune curr_rune;

  memset(dstsub, 0, sizeof(*dstsub));
  while (!(n = re_nextc(&curr_rune, &rstr, 1)) && curr_rune) {
    int sz;
    if (n < 0) return n;
    if (curr_rune == '$') {
      n = re_nextc(&curr_rune, &rstr, 1);
      if (n < 0) return n;
      switch (curr_rune) {
        case '&':
          sz = loot->sub[0].end - loot->sub[0].start;
          size += sz;
          dstsub->sub[dstsub->subexpr_num++] = loot->sub[0];
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
            n = re_nextc(&curr_rune, &rstr, 1);
            if (n < 0) return n;
            sz = dec(curr_rune);
            sbn = sbn * 10 + sz;
          }
          if (sbn >= loot->subexpr_num) break;
          sz = loot->sub[sbn].end - loot->sub[sbn].start;
          size += sz;
          dstsub->sub[dstsub->subexpr_num++] = loot->sub[sbn];
          break;
        }
        case '`':
          sz = loot->sub[0].start - src;
          size += sz;
          dstsub->sub[dstsub->subexpr_num].start = src;
          dstsub->sub[dstsub->subexpr_num++].end = loot->sub[0].start;
          break;
        case '\'':
          sz = strlen(loot->sub[0].end);
          size += sz;
          dstsub->sub[dstsub->subexpr_num].start = loot->sub[0].end;
          dstsub->sub[dstsub->subexpr_num++].end = loot->sub[0].end + sz;
          break;
        case '$':
          size++;
          dstsub->sub[dstsub->subexpr_num].start = rstr - 1;
          dstsub->sub[dstsub->subexpr_num++].end = rstr;
          break;
        default:
          return BAD_CHAR_AFTER_USD;
      }
    } else {
      char tmps[300], *d = tmps;
      size += (sz = runetochar(d, &curr_rune));
      if (!dstsub->subexpr_num ||
          dstsub->sub[dstsub->subexpr_num - 1].end != rstr - sz) {
        dstsub->sub[dstsub->subexpr_num].start = rstr - sz;
        dstsub->sub[dstsub->subexpr_num++].end = rstr;
      } else
        dstsub->sub[dstsub->subexpr_num - 1].end = rstr;
    }
  }
  return size;
}

#ifdef RE_TEST

int re_replace(struct Resub *loot, const char *src, const char *rstr,
               char **dst) {
  struct Resub newsub;
  struct re_tok *t = newsub.sub;
  char *d;
  int osz = re_rplc(loot, src, rstr, &newsub);
  int i = newsub.subexpr_num;
  if (osz < 0) {
    printf("re_rplc return: '%s'\n", re_err_msg(osz));
    return 0;
  }
  *dst = NULL;
  if (osz) *dst = reg_malloc(osz + 1);
  if (!*dst) return 0;
  d = *dst;
  do {
    size_t len = t->end - t->start;
    memcpy(d, t->start, len);
    d += len;
    t++;
  } while (--i);
  *d = '\0';
  return osz;
}

#define RE_TEST_STR_SIZE 2000

static struct v7_val_flags get_flags(const char *ch) {
  struct v7_val_flags flags = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t rep = 1;
  for (; rep && *ch; ch++) {
    switch (*ch) {
      case 'g':
        flags.re_g = 1;
        rep = 1;
        break;
      case 'i':
        flags.re_i = 1;
        rep = 1;
        break;
      case 'm':
        flags.re_m = 1;
        rep = 1;
        break;
      case 'r':
        flags.re = 1;
        rep = 1;
        break;
      default:
        rep = 0;
    }
  }
  return flags;
}

static void usage(void) {
  printf(
      "usage: regex_test.exe \"pattern\" [\"flags: gimr\"[ \"source\"[ "
      "\"replaceStr\"]]]\n   or: regex_test.exe -f file_path [>out.txt]\n");
}

int main(int argc, char **argv) {
  const char *src;
  char *dst;
  const char *rstr;
  const char *error;
  struct Reprog *pr;
  struct Resub sub;
  struct v7_val_flags flags = {0, 0, 0, 0, 0, 0, 0, 0, 1};
  unsigned int i, k = 0;

  if (argc > 1) {
    if (strcmp(argv[1], "-f") == 0) {
      FILE *fp;
      char str[RE_TEST_STR_SIZE];
      long file_size;
      if (argc < 3) {
        usage();
        return 0;
      }
      if ((fp = fopen(argv[2], "r")) == NULL) {
        printf("file: \"%s\" not found", argv[2]);
      } else if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0) {
        fclose(fp);
      } else {
        rewind(fp);
        while (fgets(str, RE_TEST_STR_SIZE, fp)) {
          char *patt = NULL, *fl_str = NULL, *curr = str, *beg = NULL;
          src = rstr = NULL;
          k++;
          if ((curr = strchr(curr, '"')) == NULL) continue;
          beg = ++curr;
          while (*curr) {
            if ((curr = strchr(curr, '"')) == NULL) break;
            if (*(curr + 1) == ' ' || *(curr + 1) == '\r' ||
                *(curr + 1) == '\n' || *(curr + 1) == '\0')
              break;
            curr++;
          }
          if (curr == NULL || *curr == '\0') return 1;
          *curr = '\0';
          patt = beg;

          if ((curr = strchr(++curr, '"'))) {
            beg = ++curr;
            while (*curr) {
              if ((curr = strchr(curr, '"')) == NULL) break;
              if (*(curr + 1) == ' ' || *(curr + 1) == '\r' ||
                  *(curr + 1) == '\n' || *(curr + 1) == '\0')
                break;
              curr++;
            }
            if (curr != NULL && *curr != '\0') {
              *curr = '\0';
              fl_str = beg;
              if ((curr = strchr(++curr, '"'))) {
                beg = ++curr;
                while (*curr) {
                  if ((curr = strchr(curr, '"')) == NULL) break;
                  if (*(curr + 1) == ' ' || *(curr + 1) == '\r' ||
                      *(curr + 1) == '\n' || *(curr + 1) == '\0')
                    break;
                  curr++;
                }
                if (curr != NULL && *curr != '\0') {
                  *curr = '\0';
                  src = beg;
                  if ((curr = strchr(++curr, '"'))) {
                    beg = ++curr;
                    while (*curr) {
                      if ((curr = strchr(curr, '"')) == NULL) break;
                      if (*(curr + 1) == ' ' || *(curr + 1) == '\r' ||
                          *(curr + 1) == '\n' || *(curr + 1) == '\0')
                        break;
                      curr++;
                    }
                    if (curr != NULL && *curr != '\0') {
                      *curr = '\0';
                      rstr = beg;
                    }
                  }
                }
              }
            }
          }
          if (patt) {
            if (k > 1) puts("");
            printf("%03d: \"%s\"", k, patt);
            if (fl_str) {
              printf(" \"%s\"", fl_str);
              flags = get_flags(fl_str);
            }
            if (src) printf(" \"%s\"", src);
            if (rstr) printf(" \"%s\"", rstr);
            printf("\n");
            pr = re_compiler(patt, flags, &error);
            if (!pr) {
              printf("re_compiler: %s\n", error);
              return 1;
            }
            printf("number of subexpressions = %d\n", pr->subexpr_num);
            if (src) {
              if (!re_exec(pr, flags, src, &sub)) {
                for (i = 0; i < sub.subexpr_num; ++i) {
                  int n = sub.sub[i].end - sub.sub[i].start;
                  if (n > 0)
                    printf("match: %-3d start:%-3d end:%-3d size:%-3d '%.*s'\n",
                           i, (int)(sub.sub[i].start - src),
                           (int)(sub.sub[i].end - src), n, n, sub.sub[i].start);
                  else
                    printf("match: %-3d ''\n", i);
                }

                if (rstr) {
                  if (re_replace(&sub, src, rstr, &dst)) {
                    printf("output: \"%s\"\n", dst);
                  }
                }
              } else
                printf("no match\n");
            }
            re_free(pr);
          }
        }
        fclose(fp);
      }
      return 0;
    }

    if (argc > 2) flags = get_flags(argv[2]);
    pr = re_compiler(argv[1], flags, &error);
    if (!pr) {
      fprintf(stderr, "re_compiler: %s\n", error);
      return 1;
    }
    printf("number of subexpressions = %d\n", pr->subexpr_num);
    if (argc > 3) {
      src = argv[3];
      if (!re_exec(pr, flags, src, &sub)) {
        for (i = 0; i < sub.subexpr_num; ++i) {
          int n = sub.sub[i].end - sub.sub[i].start;
          if (n > 0)
            printf("match: %-3d start:%-3d end:%-3d size:%-3d '%.*s'\n", i,
                   (int)(sub.sub[i].start - src), (int)(sub.sub[i].end - src),
                   n, n, sub.sub[i].start);
          else
            printf("match: %-3d ''\n", i);
        }

        if (argc > 4) {
          rstr = argv[4];
          if (re_replace(&sub, src, rstr, &dst)) {
            printf("output: \"%s\"\n\n", dst);
          }
        }
      } else
        printf("no match\n");
      re_free(pr);
    }
  } else
    usage();

  return 0;
}
#else

V7_PRIVATE enum v7_err regex_xctor(struct v7 *v7, struct v7_val *obj,
                                   const char *re, size_t re_len,
                                   const char *fl, size_t fl_len) {
  if (NULL == obj) obj = v7_push_new_object(v7);
  v7_init_str(obj, re, re_len, 1);
  v7_set_class(obj, V7_CLASS_REGEXP);
  obj->v.str.prog = NULL;
  obj->fl.fl.re = 1;
  while (fl_len) {
    switch (fl[--fl_len]) {
      case 'g':
        obj->fl.fl.re_g = 1;
        break;
      case 'i':
        obj->fl.fl.re_i = 1;
        break;
      case 'm':
        obj->fl.fl.re_m = 1;
        break;
    }
  }
  obj->v.str.lastIndex = 0;
  return V7_OK;
}

V7_PRIVATE enum v7_err Regex_ctor(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  size_t fl_len = 0;
  const char *fl_start = NULL;
  struct v7_val *re = cfa->args[0], *fl = NULL, *obj = NULL;
  if (cfa->called_as_constructor) obj = cfa->this_obj;

  if (cfa->num_args > 0) {
    TRY(check_str_re_conv(v7, &re, 0));
    if (cfa->num_args > 1) {
      fl = cfa->args[1];
      TRY(check_str_re_conv(v7, &fl, 0));
      fl_len = fl->v.str.len;
      fl_start = fl->v.str.buf;
    }
    regex_xctor(v7, obj, re->v.str.buf, re->v.str.len, fl_start, fl_len);
  }
  return V7_OK;
#undef v7
}

V7_PRIVATE void Regex_global(struct v7_val *this_obj, struct v7_val *arg,
                             struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_bool(result, this_obj->fl.fl.re_g);
}

V7_PRIVATE void Regex_ignoreCase(struct v7_val *this_obj, struct v7_val *arg,
                                 struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_bool(result, this_obj->fl.fl.re_i);
}

V7_PRIVATE void Regex_multiline(struct v7_val *this_obj, struct v7_val *arg,
                                struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_bool(result, this_obj->fl.fl.re_m);
}

V7_PRIVATE void Regex_source(struct v7_val *this_obj, struct v7_val *arg,
                             struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_str(result, this_obj->v.str.buf, this_obj->v.str.len, 1);
}

V7_PRIVATE void Regex_lastIndex(struct v7_val *this_obj, struct v7_val *arg,
                                struct v7_val *result) {
  if (arg)
    this_obj->v.str.lastIndex = arg->v.num;
  else
    v7_init_num(result, this_obj->v.str.lastIndex);
}

V7_PRIVATE enum v7_err regex_check_prog(struct v7_val *re_obj) {
  if (NULL == re_obj->v.str.prog) {
    re_obj->v.str.prog = re_compiler(re_obj->v.str.buf, re_obj->fl.fl, NULL);
    if (-1 == (int)re_obj->v.str.prog) return V7_REGEXP_ERROR;
    if (NULL == re_obj->v.str.prog) return V7_OUT_OF_MEMORY;
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Regex_exec(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0], *arr = NULL;
  struct Resub sub;
  struct re_tok *ptok = sub.sub;

  if (cfa->num_args > 0) {
    char *begin;
    TRY(check_str_re_conv(v7, &arg, 0));
    begin = arg->v.str.buf;
    if (cfa->this_obj->v.str.lastIndex < 0) cfa->this_obj->v.str.lastIndex = 0;
    if (cfa->this_obj->fl.fl.re_g)
      begin = utfnshift(begin, cfa->this_obj->v.str.lastIndex);
    TRY(regex_check_prog(cfa->this_obj));
    if (!re_exec(cfa->this_obj->v.str.prog, cfa->this_obj->fl.fl, begin,
                 &sub)) {
      int i;
      arr = v7_push_new_object(v7);
      v7_set_class(arr, V7_CLASS_ARRAY);
      for (i = 0; i < sub.subexpr_num; i++, ptok++)
        v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, ptok->start,
                                  ptok->end - ptok->start, 1));
      if (cfa->this_obj->fl.fl.re_g)
        cfa->this_obj->v.str.lastIndex = utfnlen(begin, sub.sub->end - begin);
      return V7_OK;
    } else {
      cfa->this_obj->v.str.lastIndex = 0;
    }
  }
  TRY(v7_make_and_push(v7, V7_TYPE_NULL));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Regex_test(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  TRY(Regex_exec(cfa));

  v7_push_bool(v7, v7_top_val(v7)->type != V7_TYPE_NULL);
  return V7_OK;
#undef v7
}

V7_PRIVATE void init_regex(void) {
  init_standard_constructor(V7_CLASS_REGEXP, Regex_ctor);

  SET_METHOD(s_prototypes[V7_CLASS_REGEXP], "exec", Regex_exec);
  SET_METHOD(s_prototypes[V7_CLASS_REGEXP], "test", Regex_test);

  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "global", Regex_global);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "ignoreCase", Regex_ignoreCase);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "multiline", Regex_multiline);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "source", Regex_source);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_REGEXP], "lastIndex", Regex_lastIndex);

  SET_RO_PROP_V(s_global, "RegExp", s_constructors[V7_CLASS_REGEXP]);
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


V7_PRIVATE enum v7_err String_ctor(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  const char *str = NULL;
  size_t len = 0;
  int own = 0;

  if (cfa->num_args > 0) {
    struct v7_val *arg = cfa->args[0];
    TRY(check_str_re_conv(v7, &arg, 0));
    str = arg->v.str.buf;
    len = arg->v.str.len;
    own = 1;
  }
  if (cfa->called_as_constructor) {
    struct v7_val *obj = v7_push_new_object(v7);
    v7_init_str(obj, str, len, own);
    v7_set_class(obj, V7_CLASS_STRING);
  } else
    v7_push_string(v7, str, len, 1);
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_fromCharCode(struct v7_c_func_arg *cfa) {
  long n, blen = 0;
  struct v7_val *str;
  char *p;
  Rune runes[500];
  for (n = 0; n < cfa->num_args; n++) {
    runes[n] = _conv_to_int(cfa->v7, cfa->args[n]);
    blen += runelen(runes[n]);
  }
  str = v7_push_string(cfa->v7, NULL, blen, 1);
  p = str->v.str.buf;
  for (n = 0; n < cfa->num_args; n++) p += runetochar(p, &runes[n]);
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_valueOf(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  if (!is_string(cfa->this_obj)) THROW(V7_TYPE_ERROR);
  TRY(push_string(v7, cfa->this_obj->v.str.buf, cfa->this_obj->v.str.len, 1));
  return V7_OK;
#undef v7
}

static enum v7_err _charAt(struct v7_c_func_arg *cfa, const char **p) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  if (cfa->num_args > 0) {
    long len = utfnlen(cfa->this_obj->v.str.buf, cfa->this_obj->v.str.len),
         idx = _conv_to_int(v7, cfa->args[0]);
    if (idx < 0) idx = len - idx;
    if (idx >= 0 && idx < len)
      return *p = utfnshift(cfa->this_obj->v.str.buf, idx), V7_OK;
  } else
    return *p = cfa->this_obj->v.str.buf, V7_OK;
  return *p = NULL, V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_charAt(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  const char *p;
  TRY(_charAt(cfa, &p));
  TRY(push_string(v7, p, p == NULL ? 0 : 1, 1));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_charCodeAt(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  const char *p;
  Rune rune;
  TRY(_charAt(cfa, &p));
  TRY(push_number(v7, p == NULL ? NAN : (chartorune(&rune, p), rune)));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_concat(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long n, blen;
  struct v7_val *str;
  char *p;
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  blen = cfa->this_obj->v.str.len;
  for (n = 0; n < cfa->num_args; n++) {
    TRY(check_str_re_conv(v7, &cfa->args[n], 0));
    blen += cfa->args[n]->v.str.len;
  }
  str = v7_push_string(v7, cfa->this_obj->v.str.buf, blen, 1);
  p = str->v.str.buf + cfa->this_obj->v.str.len;
  for (n = 0; n < cfa->num_args; n++) {
    memcpy(p, cfa->args[n]->v.str.buf, cfa->args[n]->v.str.len);
    p += cfa->args[n]->v.str.len;
  }
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
#undef v7
}

static long _indexOf(char *pp, char *const end, char *p, long blen,
                     uint8_t last) {
  long i, idx = -1;
  if (0 == blen || end - pp == 0) return 0;
  for (i = 0; pp <= (end - blen); i++, pp = utfnshift(pp, 1))
    if (0 == memcmp(pp, p, blen)) {
      idx = i;
      if (!last) break;
    }
  return idx;
}

V7_PRIVATE enum v7_err Str_indexOf(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long idx = -1, pos = 0;
  char *p, *end;
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  if (cfa->num_args > 0) {
    if (V7_TYPE_UNDEF != cfa->args[0]->type &&
        V7_TYPE_NULL != cfa->args[0]->type) {
      TRY(check_str_re_conv(v7, &cfa->args[0], 0));
      if (cfa->num_args > 1) {
        p = utfnshift(p, pos = _conv_to_int(v7, cfa->args[1]));
      }
      if (p < end)
        idx = _indexOf(p, end, cfa->args[0]->v.str.buf, cfa->args[0]->v.str.len,
                       0);
    } else
      idx = 0;
  }
  if (idx >= 0) idx += pos;
  TRY(push_number(v7, idx));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_lastIndexOf(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long idx = -1;
  char *p, *end;
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  if (cfa->num_args > 0) {
    TRY(check_str_re_conv(v7, &cfa->args[0], 0));
    if (cfa->num_args > 1) {
      end = utfnshift(p, _conv_to_int(v7, cfa->args[1]) + 1);
    }
    idx = _indexOf(p, end, cfa->args[0]->v.str.buf, cfa->args[0]->v.str.len, 1);
  }
  TRY(push_number(v7, idx));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_localeCompare(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0];
  long i, ln = 0, ret = 0;
  Rune s, t;
  char *ps, *pt, *end;
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  TRY(check_str_re_conv(v7, &arg, 0));
  ps = cfa->this_obj->v.str.buf;
  pt = arg->v.str.buf;
  end = ps + cfa->this_obj->v.str.len;
  if (arg->v.str.len < cfa->this_obj->v.str.len) {
    end = ps + arg->v.str.len;
    ln = 1;
  } else if (arg->v.str.len > cfa->this_obj->v.str.len) {
    ln = -1;
  }
  for (i = 0; ps < end; i++) {
    ps += chartorune(&s, ps);
    pt += chartorune(&t, pt);
    if (s < t) {
      ret = -1;
      break;
    }
    if (s > t) {
      ret = 1;
      break;
    }
  }
  if (0 == ret) ret = ln;
  TRY(push_number(v7, ret));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_match(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0];
  struct Resub sub;
  struct v7_val *arr = NULL;
  unsigned long shift = 0;

  if (cfa->num_args > 0) {
    TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
    TRY(check_str_re_conv(v7, &arg, 1));
    TRY(regex_check_prog(arg));
    do {
      if (!re_exec(arg->v.str.prog, arg->fl.fl,
                   cfa->this_obj->v.str.buf + shift, &sub)) {
        struct re_tok *ptok = sub.sub;
        int i;
        if (NULL == arr) {
          arr = v7_push_new_object(v7);
          v7_set_class(arr, V7_CLASS_ARRAY);
        }
        shift = ptok->end - cfa->this_obj->v.str.buf;
        for (i = 0; i < sub.subexpr_num; i++, ptok++)
          v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, ptok->start,
                                  ptok->end - ptok->start, 1));
      }
    } while (arg->fl.fl.re_g && shift < cfa->this_obj->v.str.len);
  }
  if (arr == NULL) TRY(v7_make_and_push(v7, V7_TYPE_NULL));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_replace(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *result = v7_push_new_object(v7);
  const char *out_str;
  uint8_t own = 1;
  size_t out_len;
  int old_sp = v7->sp;

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  out_str = cfa->this_obj->v.str.buf;
  out_len = cfa->this_obj->v.str.len;
  if (cfa->num_args > 1) {
    const char *const str_end =
        cfa->this_obj->v.str.buf + cfa->this_obj->v.str.len;
    char *p = cfa->this_obj->v.str.buf;
    uint32_t out_sub_num = 0;
    struct v7_val *re = cfa->args[0], *str_func = cfa->args[1], *arr = NULL;
    struct re_tok out_sub[V7_RE_MAX_REPL_SUB], *ptok = out_sub;
    struct Resub loot;
    TRY(check_str_re_conv(v7, &re, 1));
    TRY(regex_check_prog(re));
    if (v7_is_class(str_func, V7_CLASS_FUNCTION)) {
      arr = v7_push_new_object(v7);
      v7_set_class(arr, V7_CLASS_ARRAY);
      TRY(v7_push(v7, str_func));
    } else
      TRY(check_str_re_conv(v7, &str_func, 0));

    out_len = 0;
    do {
      int i;
      if (re_exec(re->v.str.prog, re->fl.fl, p, &loot)) break;
      if (p != loot.sub->start) {
        ptok->start = p;
        ptok->end = loot.sub->start;
        ptok++;
        out_len += loot.sub->start - p;
        out_sub_num++;
      }

      if (NULL != arr) { /* replace function */
        int old_sp = v7->sp;
        struct v7_val *rez_str;
        for (i = 0; i < loot.subexpr_num; i++)
          TRY(push_string(v7, loot.sub[i].start,
                          loot.sub[i].end - loot.sub[i].start, 1));
        TRY(push_number(v7, utfnlen(p, loot.sub[0].start - p)));
        TRY(v7_push(v7, cfa->this_obj));
        rez_str = v7_call(v7, cfa->this_obj, loot.subexpr_num + 2);
        TRY(check_str_re_conv(v7, &rez_str, 0));
        if (rez_str->v.str.len) {
          ptok->start = rez_str->v.str.buf;
          ptok->end = rez_str->v.str.buf + rez_str->v.str.len;
          ptok++;
          out_len += rez_str->v.str.len;
          out_sub_num++;
          v7_append(v7, arr, rez_str);
        }
        TRY(inc_stack(v7, old_sp - v7->sp));
      } else { /* replace string */
        struct Resub newsub;
        re_rplc(&loot, cfa->this_obj->v.str.buf, str_func->v.str.buf, &newsub);
        for (i = 0; i < newsub.subexpr_num; i++) {
          ptok->start = newsub.sub[i].start;
          ptok->end = newsub.sub[i].end;
          ptok++;
          out_len += newsub.sub[i].end - newsub.sub[i].start;
          out_sub_num++;
        }
      }
      p = (char *) loot.sub->end;
    } while (re->fl.fl.re_g && p < str_end);
    if (p < str_end) {
      ptok->start = p;
      ptok->end = str_end;
      ptok++;
      out_len += str_end - p;
      out_sub_num++;
    }
    out_str = malloc(out_len + 1);
    CHECK(out_str, V7_OUT_OF_MEMORY);
    ptok = out_sub;
    p = (char *) out_str;
    do {
      size_t ln = ptok->end - ptok->start;
      memcpy(p, ptok->start, ln);
      p += ln;
      ptok++;
    } while (--out_sub_num);
    *p = '\0';
    own = 0;
  }
  TRY(inc_stack(v7, old_sp - v7->sp));
  v7_init_str(result, out_str, out_len, own);
  result->fl.fl.str_alloc = 1;
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_search(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0];
  struct Resub sub;
  int shift = -1, utf_shift = -1;

  if (cfa->num_args > 0) {
    TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
    TRY(check_str_re_conv(v7, &arg, 1));
    TRY(regex_check_prog(arg));
    if (!re_exec(arg->v.str.prog, arg->fl.fl, cfa->this_obj->v.str.buf, &sub))
      shift = sub.sub[0].start - cfa->this_obj->v.str.buf;
  } else
    utf_shift = 0;
  if (shift >= 0) /* calc shift for UTF-8 */
    utf_shift = utfnlen(cfa->this_obj->v.str.buf, shift);
  TRY(push_number(v7, utf_shift));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_slice(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  char *begin, *end;
  long from = 0, to = 0, len;

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  to = len = utfnlen(cfa->this_obj->v.str.buf, cfa->this_obj->v.str.len);
  begin = cfa->this_obj->v.str.buf;
  end = begin + cfa->this_obj->v.str.len;
  if (cfa->num_args > 0) {
    from = _conv_to_int(v7, cfa->args[0]);
    if (from < 0) {
      from += len;
      if (from < 0) from = 0;
    } else if (from > len)
      from = len;
    if (cfa->num_args > 1) {
      to = _conv_to_int(v7, cfa->args[1]);
      if (to < 0) {
        to += len;
        if (to < 0) to = 0;
      } else if (to > len)
        to = len;
    }
  }
  if (from > to) to = from;
  end = utfnshift(begin, to);
  begin = utfnshift(begin, from);
  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v7_init_str(v7_top_val(v7), begin, end - begin, 1);
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_split(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  struct v7_val *arg = cfa->args[0], *arr = v7_push_new_object(v7), *v;
  struct Resub sub;
  int limit = 1000000, elem = 0, i, len;
  unsigned long shift = 0;

  v7_set_class(arr, V7_CLASS_ARRAY);
  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  if (cfa->num_args > 0) {
    if (cfa->num_args > 1 && cfa->args[1]->type == V7_TYPE_NUM)
      limit = cfa->args[1]->v.num;
    if (V7_TYPE_UNDEF != arg->type && V7_TYPE_NULL != arg->type) {
      TRY(check_str_re_conv(v7, &arg, 1));
      TRY(regex_check_prog(arg));
      for (; elem < limit && shift < cfa->this_obj->v.str.len; elem++) {
        if (re_exec(arg->v.str.prog, arg->fl.fl,
                    cfa->this_obj->v.str.buf + shift, &sub))
          break;

        if (sub.sub[0].end - sub.sub[0].start == 0) {
          v7_append(
              v7, arr,
              v7_mkv(v7, V7_TYPE_STR, cfa->this_obj->v.str.buf + shift,
                     1, 1));
          shift++;
          } else {
          v7_append(
              v7, arr,
              v7_mkv(v7, V7_TYPE_STR, cfa->this_obj->v.str.buf + shift,
                     sub.sub[0].start - cfa->this_obj->v.str.buf - shift, 1));
          shift = sub.sub[0].end - cfa->this_obj->v.str.buf;
        }

        for (i = 1; i < sub.subexpr_num; i++) {
          if (sub.sub[i].start != NULL) {
            v = v7_mkv(v7, V7_TYPE_STR, sub.sub[i].start,
                       sub.sub[i].end - sub.sub[i].start, 1);
          } else {
            v = make_value(v7, V7_TYPE_UNDEF);
          }
          v7_append(v7, arr, v);
        }
      }
    }
  }
  len = cfa->this_obj->v.str.len - shift;
  if (elem < limit && len > 0)
    v7_append(v7, arr, v7_mkv(v7, V7_TYPE_STR, cfa->this_obj->v.str.buf + shift,
                              len, 1));
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err _Str_strslice(struct v7_c_func_arg *cfa, int islen) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  char *begin, *end;
  long from = 0, to = 0, len;

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  to = len = utfnlen(cfa->this_obj->v.str.buf, cfa->this_obj->v.str.len);
  begin = cfa->this_obj->v.str.buf;
  end = begin + cfa->this_obj->v.str.len;
  if (cfa->num_args > 0) {
    from = _conv_to_int(v7, cfa->args[0]);
    if (from < 0) from = 0;
    if (from > len) from = len;

    if (cfa->num_args > 1) {
      to = _conv_to_int(v7, cfa->args[1]);
      if (islen) {
        to += from;
      }
      if (to < 0) to = 0;
      if (to > len) to = len;
    }
  }
  if (from > to) {
    long tmp = to;
    to = from;
    from = tmp;
  }
  end = utfnshift(begin, to);
  begin = utfnshift(begin, from);
  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v7_init_str(v7_top_val(v7), begin, end - begin, 1);
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_substr(struct v7_c_func_arg *cfa) {
  return _Str_strslice(cfa, 1);
}

V7_PRIVATE enum v7_err Str_substring(struct v7_c_func_arg *cfa) {
  return _Str_strslice(cfa, 0);
}


V7_PRIVATE enum v7_err Str_toLowerCase(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long n, blen = 0;
  struct v7_val *str;
  char *p, *end;
  Rune runes[500];

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  for (n = 0; p < end; n++) {
    p += chartorune(&runes[n], p);
    runes[n] = tolowerrune(runes[n]);
    blen += runelen(runes[n]);
  }
  str = v7_push_string(v7, NULL, blen, 1);
  p = str->v.str.buf;
  end = p + blen;
  for (n = 0; p < end; n++) p += runetochar(p, &runes[n]);
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_toLocaleLowerCase(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long n, blen = 0;
  struct v7_val *str;
  char *p, *end;
  Rune runes[500];

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  for (n = 0; p < end; n++) {
    p += chartorune(&runes[n], p);
    runes[n] = tolowerrune(runes[n]);
    blen += runelen(runes[n]);
  }
  str = v7_push_string(v7, NULL, blen, 1);
  p = str->v.str.buf;
  end = p + blen;
  for (n = 0; p < end; n++) p += runetochar(p, &runes[n]);
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_toUpperCase(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long n, blen = 0;
  struct v7_val *str;
  char *p, *end;
  Rune runes[500];

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  for (n = 0; p < end; n++) {
    p += chartorune(&runes[n], p);
    runes[n] = toupperrune(runes[n]);
    blen += runelen(runes[n]);
  }
  str = v7_push_string(v7, NULL, blen, 1);
  p = str->v.str.buf;
  end = p + blen;
  for (n = 0; p < end; n++) p += runetochar(p, &runes[n]);
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
#undef v7
}

V7_PRIVATE enum v7_err Str_toLocaleUpperCase(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  long n, blen = 0;
  struct v7_val *str;
  char *p, *end;
  Rune runes[500];

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  end = p + cfa->this_obj->v.str.len;
  for (n = 0; p < end; n++) {
    p += chartorune(&runes[n], p);
    runes[n] = toupperrune(runes[n]);
    blen += runelen(runes[n]);
  }
  str = v7_push_string(v7, NULL, blen, 1);
  p = str->v.str.buf;
  end = p + blen;
  for (n = 0; p < end; n++) p += runetochar(p, &runes[n]);
  *p = '\0';
  str->v.str.len = blen;
  return V7_OK;
#undef v7
}

static int _isspase(Rune c) { return isspacerune(c) || isnewline(c); }

V7_PRIVATE enum v7_err Str_trim(struct v7_c_func_arg *cfa) {
#define v7 (cfa->v7) /* Needed for TRY() macro below */
  char *p, *begin = NULL, *end = NULL, *pend;
  Rune rune = ' ';

  TRY(check_str_re_conv(v7, &cfa->this_obj, 0));
  p = cfa->this_obj->v.str.buf;
  pend = p + cfa->this_obj->v.str.len;
  while (p < pend) {
    char *prevp = p;
    Rune prevrune = rune;
    p += chartorune(&rune, p);
    if (!_isspase(rune)) {
      end = NULL;
      if (_isspase(prevrune))
        if (NULL == begin) begin = prevp;
    } else if (!_isspase(prevrune))
      end = prevp;
  }
  if (NULL == end) end = cfa->this_obj->v.str.buf + cfa->this_obj->v.str.len;
  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v7_init_str(v7_top_val(v7), begin, end - begin, 1);
  return V7_OK;
#undef v7
}

V7_PRIVATE void Str_length(struct v7_val *this_obj, struct v7_val *arg,
                           struct v7_val *result) {
  if (NULL == result || arg) return;
  v7_init_num(result, utfnlen(this_obj->v.str.buf, this_obj->v.str.len));
}

V7_PRIVATE void init_string(void) {
  init_standard_constructor(V7_CLASS_STRING, String_ctor);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "fromCharCode", Str_fromCharCode);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "valueOf", Str_valueOf);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charAt", Str_charAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charCodeAt", Str_charCodeAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "concat", Str_concat);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "indexOf", Str_indexOf);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "lastIndexOf", Str_lastIndexOf);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "localeCompare", Str_localeCompare);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "match", Str_match);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "replace", Str_replace);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "search", Str_search);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "slice", Str_slice);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "split", Str_split);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "substring", Str_substring);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "substr", Str_substr);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "toLowerCase", Str_toLowerCase);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "toLocaleLowerCase",
             Str_toLocaleLowerCase);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "toUpperCase", Str_toUpperCase);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "toLocaleUpperCase",
             Str_toLocaleUpperCase);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "trim", Str_trim);

  SET_PROP_FUNC(s_prototypes[V7_CLASS_STRING], "length", Str_length);

  SET_RO_PROP_V(s_global, "String", s_constructors[V7_CLASS_STRING]);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_err Json_stringify(struct v7_c_func_arg *cfa) {
  v7_push_string(cfa->v7, "implement me", 12, 0);
  /* TODO(lsm): implement JSON.stringify */
  return V7_OK;
}

V7_PRIVATE void init_json(void) {
  SET_METHOD(s_json, "stringify", Json_stringify);

  v7_set_class(&s_json, V7_CLASS_OBJECT);
  s_json.ref_count = 1;

  SET_RO_PROP_V(s_global, "JSON", s_json);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_err Std_print(struct v7_c_func_arg *cfa) {
  char *p, buf[500];
  int i;
  for (i = 0; i < cfa->num_args; i++) {
    p = v7_stringify(cfa->args[i], buf, sizeof(buf));
    printf("%s", p);
    if (p != buf) free(p);
  }
  putchar('\n');

  return V7_OK;
}

V7_PRIVATE enum v7_err Std_load(struct v7_c_func_arg *cfa) {
  int i;
  struct v7_val *obj = v7_push_new_object(cfa->v7);

  /* Push new object as a context for the loading new module */
  obj->next = cfa->v7->ctx;
  cfa->v7->ctx = obj;

  for (i = 0; i < cfa->num_args; i++) {
    if (v7_type(cfa->args[i]) != V7_TYPE_STR) return V7_TYPE_ERROR;
    if (!v7_exec_file(cfa->v7, cfa->args[i]->v.str.buf)) return V7_ERROR;
  }

  /* Pop context, and return it */
  cfa->v7->ctx = obj->next;
  v7_push_val(cfa->v7, obj);

  return V7_OK;
}

V7_PRIVATE enum v7_err Std_exit(struct v7_c_func_arg *cfa) {
  int exit_code = cfa->num_args > 0 ? (int)cfa->args[0]->v.num : EXIT_SUCCESS;
  exit(exit_code);
  return V7_OK;
}

V7_PRIVATE void base64_encode(const unsigned char *src, int src_len,
                              char *dst) {
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
V7_PRIVATE unsigned char from_b64(unsigned char ch) {
  /* Inverse lookup map */
  V7_PRIVATE const unsigned char tab[128] = {
    255, 255, 255, 255, 255, 255, 255, 255,  /* 0 */
    255, 255, 255, 255, 255, 255, 255, 255,  /* 8 */
    255, 255, 255, 255, 255, 255, 255, 255,  /* 16 */
    255, 255, 255, 255, 255, 255, 255, 255,  /* 24 */
    255, 255, 255, 255, 255, 255, 255, 255,  /* 32 */
    255, 255, 255, 62,  255, 255, 255, 63,   /* 40 */
    52,  53,  54,  55,  56,  57,  58,  59,   /* 48 */
    60,  61,  255, 255, 255, 200, 255, 255,  /* 56 '=' is 200, on index 61 */
    255, 0,   1,   2,   3,   4,   5,   6,    /* 64 */
    7,   8,   9,   10,  11,  12,  13,  14,   /* 72 */
    15,  16,  17,  18,  19,  20,  21,  22,   /* 80 */
    23,  24,  25,  255, 255, 255, 255, 255,  /* 88 */
    255, 26,  27,  28,  29,  30,  31,  32,   /* 96 */
    33,  34,  35,  36,  37,  38,  39,  40,   /* 104 */
    41,  42,  43,  44,  45,  46,  47,  48,   /* 112 */
    49,  50,  51,  255, 255, 255, 255, 255,  /* 120 */
  };
  return tab[ch & 127];
}

V7_PRIVATE void base64_decode(const unsigned char *s, int len, char *dst) {
  unsigned char a, b, c, d;
  while (len >= 4 && (a = from_b64(s[0])) != 255 &&
         (b = from_b64(s[1])) != 255 && (c = from_b64(s[2])) != 255 &&
         (d = from_b64(s[3])) != 255) {
    if (a == 200 || b == 200) break;  /* '=' can't be there */
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

V7_PRIVATE enum v7_err Std_base64_decode(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0], *result;

  result = v7_push_string(cfa->v7, NULL, 0, 0);
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    result->v.str.len = v->v.str.len * 3 / 4 + 1;
    result->v.str.buf = (char *) malloc(result->v.str.len + 1);
    base64_decode((const unsigned char *) v->v.str.buf, (int) v->v.str.len,
                  result->v.str.buf);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_base64_encode(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0], *result;

  result = v7_push_string(cfa->v7, NULL, 0, 0);
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    result->v.str.len = v->v.str.len * 3 / 2 + 1;
    result->v.str.buf = (char *) malloc(result->v.str.len + 1);
    base64_encode((const unsigned char *) v->v.str.buf, (int) v->v.str.len,
                  result->v.str.buf);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_eval(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0];
  if (cfa->num_args == 1 && v->type == V7_TYPE_STR && v->v.str.len > 0) {
    return do_exec(cfa->v7, "<eval>", v->v.str.buf, cfa->v7->sp);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_read(struct v7_c_func_arg *cfa) {
  struct v7_val *v;
  char buf[2048];
  size_t n;

  if ((v = v7_get(cfa->this_obj, "fp")) != NULL &&
      (n = fread(buf, 1, sizeof(buf), (FILE *)(unsigned long) v->v.num)) > 0) {
    v7_push_string(cfa->v7, buf, n, 1);
  } else {
    v7_make_and_push(cfa->v7, V7_TYPE_NULL);
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_write(struct v7_c_func_arg *cfa) {
  struct v7_val *v = cfa->args[0], *result;
  size_t n, i;

  result = v7_push_number(cfa->v7, 0);
  if ((v = v7_get(cfa->this_obj, "fp")) != NULL) {
    for (i = 0; (int)i < cfa->num_args; i++) {
      if (is_string(cfa->args[i]) &&
          (n = fwrite(cfa->args[i]->v.str.buf, 1, cfa->args[i]->v.str.len,
                      (FILE *)(unsigned long) v->v.num)) > 0) {
        result->v.num += n;
      }
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_close(struct v7_c_func_arg *cfa) {
  struct v7_val *v;
  int ok = 0;
  if ((v = v7_get(cfa->this_obj, "fp")) != NULL &&
      fclose((FILE *)(unsigned long) v->v.num) == 0) {
    ok = 1;
  }
  v7_push_bool(cfa->v7, ok);
  return V7_OK;
}

V7_PRIVATE enum v7_err Std_open(struct v7_c_func_arg *cfa) {
  struct v7_val *v1 = cfa->args[0], *v2 = cfa->args[1], *result = NULL;
  FILE *fp;

  if (cfa->num_args == 2 && is_string(v1) && is_string(v2) &&
      (fp = fopen(v1->v.str.buf, v2->v.str.buf)) != NULL) {
    result = v7_push_new_object(cfa->v7);
    result->proto = &s_file;
    v7_setv(cfa->v7, result, V7_TYPE_STR, V7_TYPE_NUM, "fp", 2, 0,
            (double)(unsigned long) fp);  /* after v7_set_class ! */
  } else {
    v7_make_and_push(cfa->v7, V7_TYPE_NULL);
  }
  return V7_OK;
}

V7_PRIVATE void init_stdlib(void) {
  init_object();
  init_number();
  init_array();
  init_string();
  init_regex();
  init_function();
  init_date();
  init_error();
  init_boolean();
  init_math();
  init_json();
#ifndef V7_DISABLE_CRYPTO
  init_crypto();
#endif

  SET_METHOD(s_global, "print", Std_print);
  SET_METHOD(s_global, "exit", Std_exit);
  SET_METHOD(s_global, "load", Std_load);
  SET_METHOD(s_global, "base64_encode", Std_base64_encode);
  SET_METHOD(s_global, "base64_decode", Std_base64_decode);
  SET_METHOD(s_global, "eval", Std_eval);
  SET_METHOD(s_global, "open", Std_open);

  SET_RO_PROP(s_global, "Infinity", V7_TYPE_NUM, num, INFINITY);
  SET_RO_PROP(s_global, "NaN", V7_TYPE_NUM, num, NAN);

  SET_METHOD(s_file, "read", Std_read);
  SET_METHOD(s_file, "write", Std_write);
  SET_METHOD(s_file, "close", Std_close);

  v7_set_class(&s_file, V7_CLASS_OBJECT);
  s_file.ref_count = 1;

  v7_set_class(&s_global, V7_CLASS_OBJECT);
  s_global.ref_count = 1;
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


V7_PRIVATE enum v7_tok lookahead(const struct v7 *v7) {
  const char *s = v7->pstate.pc;
  double d;
  return get_tok(&s, &d);
}

V7_PRIVATE enum v7_tok next_tok(struct v7 *v7) {
  int prev_line_no = v7->pstate.prev_line_no;
  v7->pstate.prev_line_no = v7->pstate.line_no;
  v7->pstate.line_no += skip_to_next_tok(&v7->pstate.pc);
  v7->after_newline = prev_line_no != v7->pstate.line_no;
  v7->tok = v7->pstate.pc;
  v7->cur_tok = get_tok(&v7->pstate.pc, &v7->cur_tok_dbl);
  v7->tok_len = v7->pstate.pc - v7->tok;
  v7->pstate.line_no += skip_to_next_tok(&v7->pstate.pc);
  TRACE_CALL("==tok=> %d [%.*s] %d\n", v7->cur_tok, (int)v7->tok_len, v7->tok,
             v7->pstate.line_no);
  return v7->cur_tok;
}

static void get_v7_state(struct v7 *v7, struct v7_pstate *s) {
  *s = v7->pstate;
  s->pc = v7->tok;
}

static void set_v7_state(struct v7 *v7, struct v7_pstate *s) {
  v7->pstate = *s;
  next_tok(v7);
}

static enum v7_err arith(struct v7 *v7, struct v7_val *a, struct v7_val *b,
                         struct v7_val *res, enum v7_tok op) {
  char *str;

  _prop_func_2_value(v7, &a);
  _prop_func_2_value(v7, &b);
  if (op == TOK_PLUS && (is_string(a) || is_string(b))) {
    /* Do type conversion, result pushed on stack */
    TRY(check_str_re_conv(v7, &a, 0));
    TRY(check_str_re_conv(v7, &b, 0));

    str = (char *) malloc(a->v.str.len + b->v.str.len + 1);
    CHECK(str != NULL, V7_OUT_OF_MEMORY);
    v7_init_str(res, str, a->v.str.len + b->v.str.len, 0);
    memcpy(str, a->v.str.buf, a->v.str.len);
    memcpy(str + a->v.str.len, b->v.str.buf, b->v.str.len);
    str[res->v.str.len] = '\0';
    return V7_OK;
  } else {
    struct v7_val *v = res;
    double an = _conv_to_num(v7, a), bn = _conv_to_num(v7, b);
    if (res->fl.fl.prop_func) v = v7_push_new_object(v7);
    v7_init_num(v, res->v.num);
    switch (op) {
      case TOK_PLUS:
        v->v.num = an + bn;
        break;
      case TOK_MINUS:
        v->v.num = an - bn;
        break;
      case TOK_MUL:
        v->v.num = an * bn;
        break;
      case TOK_DIV:
        v->v.num = an / bn;
        break;
      case TOK_REM:
        v->v.num = (unsigned long)an % (unsigned long)bn;
        break;
      case TOK_XOR:
        v->v.num = (unsigned long)an ^ (unsigned long)bn;
        break;
      default:
        return V7_INTERNAL_ERROR;
    }
    if (res->fl.fl.prop_func) {
      res->v.prop_func.f(res->v.prop_func.o, v, NULL);
      INC_REF_COUNT(v);
      TRY(inc_stack(v7, -2));
      v7_push(v7, v);
      DEC_REF_COUNT(v);
    }
    return V7_OK;
  }
}

static enum v7_err arith_op(struct v7 *v7, enum v7_tok tok, int sp1, int sp2) {
  struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7->stack[sp2 - 1];
  int sp;

  assert(EXECUTING(v7->flags));
  CHECK(v7->sp >= 2, V7_STACK_UNDERFLOW);
  TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
  sp = v7->sp;
  TRY(arith(v7, v1, v2, v7_top_val(v7), tok));

  /*
   * arith() might push another value on stack if type conversion was made.
   * If that happens, re-push the result again
   */
  if (v7->sp > sp) {
    TRY(v7_push(v7, v7->stack[sp - 1]));
  }

  return V7_OK;
}

static enum v7_err parse_compound_statement(struct v7 *v7, int *has_return) {
  if (v7->cur_tok == TOK_OPEN_CURLY) {
    int old_sp = v7->sp;
    next_tok(v7);
    while (v7->cur_tok != TOK_CLOSE_CURLY) {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(parse_statement(v7, has_return));
      if (*has_return && EXECUTING(v7->flags)) return V7_OK;
    }
    EXPECT(TOK_CLOSE_CURLY);
  } else {
    TRY(parse_statement(v7, has_return));
  }
  return V7_OK;
}

static enum v7_err parse_function_definition(struct v7 *v7, struct v7_val **v,
                                             int num_params) { /* <#fdef#> */
  int i = 0, old_flags = v7->flags, old_sp = v7->sp, has_ret = 0,
      line_no = v7->pstate.line_no;
  unsigned long func_name_len = 0;
  const char *src = v7->pstate.pc, *func_name = NULL;
  struct v7_val *ctx = NULL, *f = NULL;

  EXPECT(TOK_FUNCTION);
  if (v7->cur_tok == TOK_IDENTIFIER) {
    /* function name is given, e.g. function foo() {} */
    CHECK(v == NULL, V7_SYNTAX_ERROR);
    func_name = v7->tok;
    func_name_len = v7->tok_len;
    src = v7->pstate.pc;
    line_no = v7->pstate.line_no;
    next_tok(v7);
  }

  /*
   * 1. SCANNING: do nothing, just pass through the function code
   * 2. EXECUTING && v == 0: don't execute but create a closure
   * 3. EXECUTING && v != 0: execute the closure
   */

  if (EXECUTING(v7->flags) && v == NULL) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    f = v7_top_val(v7);
    v7_set_class(f, V7_CLASS_FUNCTION);
    f->fl.fl.js_func = 1;

    f->v.func.source_code = (char *) src;
    f->v.func.line_no = line_no;
    /* printf("PFD [%.*s]\n", 45, f->v.func.source_code); */

    f->v.func.var_obj = v7->ctx;
    INC_REF_COUNT(v7->ctx);

    v7->flags |= V7_NO_EXEC;
  } else if (EXECUTING(v7->flags) && v != NULL) {
    f = v[0];
    assert(v7_is_class(f, V7_CLASS_FUNCTION));

    f->next = v7->cf;
    v7->cf = f;

    ctx = make_value(v7, V7_TYPE_OBJ);
    v7_set_class(ctx, V7_CLASS_OBJECT);
    INC_REF_COUNT(ctx);

    ctx->next = v7->ctx;
    v7->ctx = ctx;
  }

  /* Add function arguments to the variable object */
  EXPECT(TOK_OPEN_PAREN);
  while (v7->cur_tok != TOK_CLOSE_PAREN) {
    const char *key = v7->tok;
    unsigned long key_len = v7->tok_len;
    EXPECT(TOK_IDENTIFIER);
    if (EXECUTING(v7->flags)) {
      struct v7_val *val =
          i < num_params ? v[i + 1] : make_value(v7, V7_TYPE_UNDEF);
      TRY(v7_setv(v7, ctx, V7_TYPE_STR, V7_TYPE_OBJ, key, key_len, 1, val));
    }
    i++;
    if (v7->cur_tok == TOK_COMMA) {
      next_tok(v7);
    }
  }
  EXPECT(TOK_CLOSE_PAREN);

  /* Execute (or pass) function body */
  TRY(parse_compound_statement(v7, &has_ret));

  /* Add function to the namespace for notation "function x(y,z) { ... } " */
  if (EXECUTING(old_flags) && v == NULL && func_name != NULL) {
    TRY(v7_setv(v7, v7->ctx, V7_TYPE_STR, V7_TYPE_OBJ, func_name, func_name_len,
                1, f));
  }

  if (EXECUTING(v7->flags)) {
    /* Cleanup execution context */
    v7->ctx = ctx->next;
    ctx->next = NULL;
#if 0
    assert(f->v.func.var_obj == NULL);
    f->v.func.var_obj = ctx;
#endif
    v7_freeval(v7, ctx);

    v7->cf = f->next;
    f->next = NULL;

    /* If function didn't have return statement, return UNDEF */
    if (!has_ret) {
      TRY(inc_stack(v7, old_sp - v7->sp));
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    }
  }

  v7->flags = old_flags;

  return V7_OK;
}

V7_PRIVATE enum v7_err v7_call2(struct v7 *v7, struct v7_val *this_obj,
                                int num_args, int called_as_ctor) {
  struct v7_val **top = v7_top(v7), **v = top - (num_args + 1), *f;

  if (!EXECUTING(v7->flags)) return V7_OK;
  f = v[0];
  CHECK(v7->sp > num_args, V7_INTERNAL_ERROR);
  CHECK(f != NULL, V7_TYPE_ERROR);
  CHECK(v7_is_class(f, V7_CLASS_FUNCTION), V7_CALLED_NON_FUNCTION);

  /*
   * Stack looks as follows:
   *  v   --->  <called_function>     v[0]
   *            <argument_0>        ---+
   *            <argument_1>           |
   *            <argument_2>           |  <-- num_args
   *            ...                    |
   *            <argument_N>        ---+
   * top  --->  <return_value>
   */
  if (f->fl.fl.js_func) {
    struct v7_pstate old_pstate = v7->pstate;
    enum v7_tok tok = v7->cur_tok;

    /* Move control flow to the function body */
    v7->pstate.pc = f->v.func.source_code;
    v7->pstate.line_no = f->v.func.line_no;
    v7->cur_tok = TOK_FUNCTION;
    TRY(parse_function_definition(v7, v, num_args));

    /* Return control flow back */
    v7->pstate = old_pstate;
    v7->cur_tok = tok;
    CHECK(v7_top(v7) >= top, V7_INTERNAL_ERROR);
  } else {
    int old_sp = v7->sp;
    struct v7_c_func_arg arg;
    arg.v7 = v7;
    arg.this_obj = this_obj;
    arg.args = v + 1;
    arg.num_args = num_args;
    arg.called_as_constructor = called_as_ctor;

    TRY(f->v.c_func(&arg));
    if (old_sp == v7->sp) {
      v7_make_and_push(v7, V7_TYPE_UNDEF);
    }
  }
  return V7_OK;
}

static enum v7_err parse_function_call(struct v7 *v7, struct v7_val *this_obj,
                                       int called_as_ctor) {
  struct v7_val **v = v7_top(v7) - 1;
  int num_args = 0;

  /* TRACE_OBJ(v[0]); */
  CHECK(!EXECUTING(v7->flags) || v7_is_class(v[0], V7_CLASS_FUNCTION),
        V7_CALLED_NON_FUNCTION);

  /* Push arguments on stack */
  EXPECT(TOK_OPEN_PAREN);
  while (v7->cur_tok != TOK_CLOSE_PAREN) {
    TRY(parse_expression(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *v = v7_top_val(v7);
      INC_REF_COUNT(v);
      TRY(inc_stack(v7, -1));
      DEC_REF_COUNT(v);
      _prop_func_2_value(v7, &v);
      v7_push(v7, v);
    }
    if (v7->cur_tok == TOK_COMMA) {
      next_tok(v7);
    }
    num_args++;
  }
  EXPECT(TOK_CLOSE_PAREN);

  TRY(v7_call2(v7, this_obj, num_args, called_as_ctor));

  return V7_OK;
}

static enum v7_err push_string_literal(struct v7 *v7) {
  struct v7_val *v;
  char *p;
  const char *ps = &v7->tok[1], *end = ps + v7->tok_len - 2;
  Rune rune;

  if (!EXECUTING(v7->flags)) return V7_OK;
  TRY(push_string(v7, &v7->tok[1], v7->tok_len - 2, 1));
  v = v7_top_val(v7);
  CHECK(v->v.str.buf != NULL, V7_OUT_OF_MEMORY);
  p = v->v.str.buf;

  /* Scan string literal into the buffer, handle escape sequences */
  while (ps < end) {
    ps += chartorune(&rune, ps);
    if (rune == '\\') switch (nextesc(&rune, &ps)) {
        case 0:
        case 1:
          break;
        case 2:
          switch (rune) {
            case '\\':
            case '\'':
            case '"':
              break;
            default:
              *p++ = '\\';
          }
          break;
        default:
          return V7_SYNTAX_ERROR;
      }
    p += runetochar(p, &rune);
  }
  v->v.str.len = p - v->v.str.buf;
  assert(v->v.str.len < v7->tok_len - 1);
  *p = '\0';

  return V7_OK;
}

static enum v7_err parse_array_literal(struct v7 *v7) {
  /* Push empty array on stack */
  if (EXECUTING(v7->flags)) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    v7_set_class(v7_top(v7)[-1], V7_CLASS_ARRAY);
  }
  CHECK(v7->cur_tok == TOK_OPEN_BRACKET, V7_SYNTAX_ERROR);
  next_tok(v7);

  /* Scan array literal, append elements one by one */
  while (v7->cur_tok != TOK_CLOSE_BRACKET) {
    /* Push new element on stack */
    TRY(parse_expression(v7));
    if (EXECUTING(v7->flags)) {
      TRY(v7_append(v7, v7_top(v7)[-2], v7_top(v7)[-1]));
      TRY(inc_stack(v7, -1));
    }
    if (v7->cur_tok == TOK_COMMA) {
      next_tok(v7);
    }
  }
  CHECK(v7->cur_tok == TOK_CLOSE_BRACKET, V7_SYNTAX_ERROR);
  return V7_OK;
}

static enum v7_err parse_object_literal(struct v7 *v7) {
  /* Push empty object on stack */
  TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
  EXPECT(TOK_OPEN_CURLY);

  /* Assign key/values to the object, until closing "}" is found */
  while (v7->cur_tok != TOK_CLOSE_CURLY) {
    /* Push key on stack */
    if (v7->cur_tok == TOK_STRING_LITERAL) {
      TRY(push_string_literal(v7));
    } else {
      struct v7_val *v;
      CHECK(v7->cur_tok == TOK_IDENTIFIER, V7_SYNTAX_ERROR);
      v = v7_mkv(v7, V7_TYPE_STR, v7->tok, v7->tok_len, 1);
      CHECK(v != NULL, V7_OUT_OF_MEMORY);
      TRY(v7_push(v7, v));
    }

    /* Push value on stack */
    next_tok(v7);
    EXPECT(TOK_COLON);
    TRY(parse_expression(v7));

    /* Stack should now have object, key, value. Assign, and remove key/value */
    if (EXECUTING(v7->flags)) {
      struct v7_val **v = v7_top(v7) - 3;
      CHECK(v[0]->type == V7_TYPE_OBJ, V7_INTERNAL_ERROR);
      TRY(v7_set2(v7, v[0], v[1], v[2]));
      TRY(inc_stack(v7, -2));
    }
    if (v7->cur_tok == TOK_COMMA) {
      next_tok(v7);
    }
  }
  CHECK(v7->cur_tok == TOK_CLOSE_CURLY, V7_SYNTAX_ERROR);
  return V7_OK;
}

static enum v7_err parse_delete_statement(struct v7 *v7) {
  EXPECT(TOK_DELETE);
  TRY(parse_expression(v7));
  TRY(v7_del2(v7, v7->cur_obj, v7->key, v7->key_len));
  return V7_OK;
}

static enum v7_err parse_regex(struct v7 *v7) {
  size_t i, j;
  uint8_t done = 0;

  if (!EXECUTING(v7->flags)) return V7_OK;
  /* CHECK(*v7->tok == '/', V7_SYNTAX_ERROR); */

  for (i = 1; !done; i++) {
    switch (v7->tok[i]) {
      case '\0':
      case '\r':
      case '\n':
        THROW(V7_SYNTAX_ERROR);
      case '/':
        if ('\\' == v7->tok[i - 1]) continue;
        done = 1;
        break;
    }
  }
  done = 0;
  for (j = i; !done; j++) {
    switch (v7->tok[j]) {
      default:
        done = 1;
        j--;
      case 'g':
      case 'i':
      case 'm':
        break;
    }
  }

  TRY(regex_xctor(v7, NULL, &v7->tok[1], i - 2, &v7->tok[i], j - i));
  v7->pstate.pc = &v7->tok[j];
  return V7_OK;
}

static enum v7_err parse_variable(struct v7 *v7) {
  struct v7_val key = str_to_val(v7->tok, v7->tok_len), *v = NULL;
  v7->key = v7->tok;
  v7->key_len = v7->tok_len;
  v = find(v7, &key);
  if (v == NULL) {
    TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
  } else {
    TRY(v7_push(v7, v));
  }
  return V7_OK;
}

static enum v7_err parse_precedence_0(struct v7 *v7) {
  enum v7_tok tok = v7->cur_tok;
  int ex = EXECUTING(v7->flags);

  switch (tok) {
    case TOK_OPEN_PAREN:
      next_tok(v7);
      TRY(parse_expression(v7));
      CHECK(v7->cur_tok == TOK_CLOSE_PAREN, V7_SYNTAX_ERROR);
      break;
    case TOK_OPEN_BRACKET:
      TRY(parse_array_literal(v7));
      break;
    case TOK_OPEN_CURLY:
      TRY(parse_object_literal(v7));
      break;
    case TOK_DIV:
      TRY(parse_regex(v7));
      break;
    case TOK_STRING_LITERAL:
      TRY(push_string_literal(v7));
      break;
    case TOK_THIS:
      if (ex) TRY(v7_push(v7, v7->this_obj));
      break;
    case TOK_NULL:
      if (ex) TRY(v7_make_and_push(v7, V7_TYPE_NULL));
      break;
    case TOK_TRUE:
      if (ex) TRY(push_bool(v7, 1));
      break;
    case TOK_FALSE:
      if (ex) TRY(push_bool(v7, 0));
      break;
#if 0
    case TOK_NAN: if (ex) TRY(push_number(v7, NAN)); break;
    case TOK_INFINITY: if (ex) TRY(push_number(v7, INFINITY)); break;
#endif
    case TOK_NUMBER:
      if (ex) TRY(push_number(v7, v7->cur_tok_dbl));
      break;
    case TOK_IDENTIFIER:
      if (ex) TRY(parse_variable(v7));
      break;
    case TOK_FUNCTION:
      TRY(parse_function_definition(v7, NULL, 0));
      break;
    default:
      return V7_SYNTAX_ERROR;
  }

  if (tok != TOK_FUNCTION) {
    next_tok(v7);
  }

  return V7_OK;
}

static enum v7_err parse_prop_accessor(struct v7 *v7, enum v7_tok op) {
  struct v7_val *ns = NULL, *cur_obj = NULL;

  if (EXECUTING(v7->flags)) {
    ns = v7_top(v7)[-1];
    v7->cur_obj = v7->this_obj = cur_obj = ns;
  }
  CHECK(!EXECUTING(v7->flags) || ns != NULL, V7_SYNTAX_ERROR);

  if (op == TOK_DOT) {
    CHECK(v7->cur_tok == TOK_IDENTIFIER, V7_SYNTAX_ERROR);
    v7->key = v7->tok;
    v7->key_len = v7->tok_len;
    if (EXECUTING(v7->flags)) {
      struct v7_val key = str_to_val(v7->tok, v7->tok_len);
      ns = get2(ns, &key);
    }
    next_tok(v7);
  } else {
    TRY(parse_expression(v7));
    EXPECT(TOK_CLOSE_BRACKET);
    if (EXECUTING(v7->flags)) {
      struct v7_val *expr_val = v7_top_val(v7);

      ns = get2(ns, expr_val);

      /*
       * If we're doing an assignment,
       * then parse_assign() looks at v7->key, v7->key_len for the key.
       * Initialize key properly for cases like "a.b['c'] = d;"
       */
      TRY(check_str_re_conv(v7, &expr_val, 0));
      v7->key = expr_val->v.str.buf;
      v7->key_len = expr_val->v.str.len;
    }
  }

  /* Set those again cause parse_expression() above could have changed it */
  v7->cur_obj = v7->this_obj = cur_obj;

  if (EXECUTING(v7->flags)) {
    if (NULL == ns) {
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
    } else {
      TRY(v7_push(v7, ns));
    }
  }

  return V7_OK;
}

/*
 * Member Access            left-to-right    x . x
 * Computed Member Access   left-to-right    x [ x ]
 * new (with argument list) n/a              new x ( x )
 */
static enum v7_err parse_precedence_1(struct v7 *v7, int has_new) {
  struct v7_val *old_this = v7->this_obj;

  TRY(parse_precedence_0(v7));

  while (v7->cur_tok == TOK_DOT || v7->cur_tok == TOK_OPEN_BRACKET ||
         v7->cur_tok == TOK_OPEN_PAREN) {
    enum v7_tok tok = v7->cur_tok;
    if (tok == TOK_OPEN_BRACKET || tok == TOK_DOT) {
      next_tok(v7);
      TRY(parse_prop_accessor(v7, tok));
    } else {
      TRY(parse_function_call(v7, v7->cur_obj, has_new));
    }
  }
  v7->this_obj = old_this;

  return V7_OK;
}

/*
 * x . y () . z () ()
 *
 * Function Call                 left-to-right     x ( x )
 * new (without argument list)   right-to-left     new x
 */
static enum v7_err parse_precedence_2(struct v7 *v7) {
  int has_new = 0;
  struct v7_val *old_this_obj = v7->this_obj, *cur_this = v7->this_obj;

  if (v7->cur_tok == TOK_NEW) {
    has_new++;
    next_tok(v7);
    if (EXECUTING(v7->flags)) {
      v7_make_and_push(v7, V7_TYPE_OBJ);
      cur_this = v7->this_obj = v7_top(v7)[-1];
      v7_set_class(cur_this, V7_CLASS_OBJECT);
    }
  }
  TRY(parse_precedence_1(v7, has_new));
#if 0
  while (*v7->pstate.pc == '(') {
    /* Use cur_this, not v7->this_obj: v7->this_obj could have been changed */
    TRY(parse_function_call(v7, cur_this, has_new));
  }
#endif
  if (has_new && EXECUTING(v7->flags)) {
    TRY(v7_push(v7, cur_this));
  }

  v7->this_obj = old_this_obj;

  return V7_OK;
}

/*
 * Postfix Increment    n/a      x ++
 * Postfix Decrement    n/a      x --
 */
static enum v7_err parse_postfix_inc_dec(struct v7 *v7) {
  TRY(parse_precedence_2(v7));
  if (v7->cur_tok == TOK_PLUS_PLUS || v7->cur_tok == TOK_MINUS_MINUS) {
    int increment = (v7->cur_tok == TOK_PLUS_PLUS) ? 1 : -1;
    next_tok(v7);
    if (EXECUTING(v7->flags)) {
      struct v7_val *v, *v1;
      v = v7_top(v7)[-1];
      TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
      v1 = v7_top(v7)[-1];
      if (v->fl.fl.prop_func) {
        v->v.prop_func.f(v->v.prop_func.o, NULL, v1);
        CHECK(v1->type == V7_TYPE_NUM, V7_TYPE_ERROR);
        v1->v.num += increment;
        v->v.prop_func.f(v->v.prop_func.o, v1, NULL);
        v1->v.num -= increment;
      } else {
        CHECK(v->type == V7_TYPE_NUM, V7_TYPE_ERROR);
        v7_init_num(v1, v->v.num);
        v->v.num += increment;
      }
      INC_REF_COUNT(v1);
      TRY(inc_stack(v7, -2));
      v7_push(v7, v1);
      DEC_REF_COUNT(v1);
    }
  }
  return V7_OK;
}

/*
 * Logical NOT        right-to-left    ! x
 * Bitwise NOT        right-to-left    ~ x
 * Unary Plus         right-to-left    + x
 * Unary Negation     right-to-left    - x
 * Prefix Increment   right-to-left    ++ x
 * Prefix Decrement   right-to-left    -- x
 * typeof             right-to-left    typeof x
 * void               right-to-left    void x
 * delete             right-to-left    delete x
 */
static enum v7_err parse_unary(struct v7 *v7) {
  static const char *type_names[] = {"undefined", "object", "boolean",
                                     "string",    "number", "object"};
  const char *str;
  int unary = TOK_END_OF_INPUT;

  switch (v7->cur_tok) {
    case TOK_NOT:
    case TOK_TILDA:
    case TOK_PLUS:
    case TOK_MINUS:
    case TOK_PLUS_PLUS:
    case TOK_MINUS_MINUS:
    case TOK_TYPEOF:
    case TOK_VOID:
    case TOK_DELETE:
      unary = v7->cur_tok;
      next_tok(v7);
      break;
    default:
      unary = TOK_END_OF_INPUT;
      break;
  }

  TRY(parse_postfix_inc_dec(v7));

  if (EXECUTING(v7->flags) && unary != TOK_END_OF_INPUT) {
    struct v7_val *result = v7_top_val(v7);
    if (result->fl.fl.prop_func) {
      switch (unary) {
        case TOK_TILDA:
        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_NOT:
        case TOK_TYPEOF:
          _prop_func_2_value(v7, &result);
          v7_push(v7, result);
      }
    }
    switch (unary) {
      case TOK_TILDA:
        TRY(push_number(v7, ~(long)_conv_to_num(v7, result)));
        break;
      case TOK_PLUS:
        TRY(push_number(v7, _conv_to_num(v7, result)));
        break;
      case TOK_MINUS:
        TRY(push_number(v7, -_conv_to_num(v7, result)));
        break;
      case TOK_NOT:
        TRY(push_bool(v7, !v7_is_true(result)));
        break;
      case TOK_TYPEOF:
        str = type_names[result->type];
        if (v7_is_class(result, V7_CLASS_FUNCTION)) str = "function";
        TRY(push_string(v7, str, strlen(str), 0));
        break;
      case TOK_VOID:
        TRY(inc_stack(v7, -1));
        TRY(v7_make_and_push(v7, V7_TYPE_UNDEF));
        break;
      default:
        break;
    }
  }

  return V7_OK;
}

static enum v7_err parse_mul_div_rem(struct v7 *v7) {
  TRY(parse_unary(v7));
  while (v7->cur_tok == TOK_MUL || v7->cur_tok == TOK_DIV ||
         v7->cur_tok == TOK_REM) {
    int sp1 = v7->sp;
    enum v7_tok tok = v7->cur_tok;
    next_tok(v7);
    TRY(parse_unary(v7));
    if (EXECUTING(v7->flags)) {
      TRY(arith_op(v7, tok, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err logical_op(struct v7 *v7, enum v7_tok op, int sp1, int sp2) {
  struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7->stack[sp2 - 1];
  int res = 0;
  double n1 = _conv_to_num(v7, v1), n2 = _conv_to_num(v7, v2);

  if (v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM) {
    switch (op) {
      case TOK_GT:
        res = n1 > n2;
        break;
      case TOK_GE:
        res = n1 >= n2;
        break;
      case TOK_LT:
        res = n1 < n2;
        break;
      case TOK_LE:
        res = n1 <= n2;
        break;
      case TOK_EQ:  /* FALLTHROUGH */
      case TOK_EQ_EQ:
        res = cmp(v1, v2) == 0;
        break;
      case TOK_NE:  /* FALLTHROUGH */
      case TOK_NE_NE:
        res = cmp(v1, v2) != 0;
        break;
      default:
        return V7_INTERNAL_ERROR;
    }
  } else if (op == TOK_EQ || op == TOK_EQ_EQ) {
    res = cmp(v1, v2) == 0;
  } else if (op == TOK_NE || op == TOK_NE_NE) {
    res = cmp(v1, v2) != 0;
  }
  TRY(push_bool(v7, res));
  return V7_OK;
}

static enum v7_err parse_add_sub(struct v7 *v7) {
  TRY(parse_mul_div_rem(v7));
  while (v7->cur_tok == TOK_PLUS || v7->cur_tok == TOK_MINUS) {
    int sp1 = v7->sp;
    enum v7_tok op = v7->cur_tok;
    next_tok(v7);
    TRY(parse_mul_div_rem(v7));
    if (EXECUTING(v7->flags)) {
      TRY(arith_op(v7, op, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err parse_relational(struct v7 *v7) {
  TRY(parse_add_sub(v7));
  while (v7->cur_tok >= TOK_LE && v7->cur_tok <= TOK_GT) {
    int sp1 = v7->sp;
    enum v7_tok op = v7->cur_tok;
    next_tok(v7);
    TRY(parse_add_sub(v7));
    if (EXECUTING(v7->flags)) {
      TRY(logical_op(v7, op, sp1, v7->sp));
    }
  }
  if (v7->cur_tok == TOK_INSTANCEOF) {
    CHECK(next_tok(v7) == TOK_IDENTIFIER, V7_SYNTAX_ERROR);
    if (EXECUTING(v7->flags)) {
      struct v7_val key = str_to_val(v7->tok, v7->tok_len);
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = instanceof(v7_top(v7)[-2], find(v7, &key));
    }
  }
  return V7_OK;
}

static enum v7_err parse_equality(struct v7 *v7) {
  TRY(parse_relational(v7));
  if (v7->cur_tok >= TOK_EQ && v7->cur_tok <= TOK_NE_NE) {
    int sp1 = v7->sp;
    enum v7_tok op = v7->cur_tok;
    next_tok(v7);
    TRY(parse_relational(v7));
    if (EXECUTING(v7->flags)) {
      TRY(logical_op(v7, op, sp1, v7->sp));
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_and(struct v7 *v7) {
  TRY(parse_equality(v7));
  if (v7->cur_tok == TOK_AND) {
    int sp1 = v7->sp;
    next_tok(v7);
    TRY(parse_equality(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = _conv_to_num(v7, v1), b = _conv_to_num(v7, v2);
      CHECK(v1->type == V7_TYPE_NUM && v1->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a & b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_xor(struct v7 *v7) {
  TRY(parse_bitwise_and(v7));
  if (v7->cur_tok == TOK_XOR) {
    int sp1 = v7->sp;
    next_tok(v7);
    TRY(parse_bitwise_and(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = _conv_to_num(v7, v1), b = _conv_to_num(v7, v2);
      CHECK(v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a ^ b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_bitwise_or(struct v7 *v7) {
  TRY(parse_bitwise_xor(v7));
  if (v7->cur_tok == TOK_OR) {
    int sp1 = v7->sp;
    next_tok(v7);
    TRY(parse_bitwise_xor(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      unsigned long a = _conv_to_num(v7, v1), b = _conv_to_num(v7, v2);
      CHECK(v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      TRY(v7_make_and_push(v7, V7_TYPE_NUM));
      v7_top(v7)[-1]->v.num = a | b;
    }
  }
  return V7_OK;
}

static enum v7_err parse_logical_and(struct v7 *v7) {
  TRY(parse_bitwise_or(v7));
  while (v7->cur_tok == TOK_LOGICAL_AND) {
    int sp1 = v7->sp;
    next_tok(v7);
    TRY(parse_bitwise_or(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      int is_true = v7_is_true(v1) && v7_is_true(v2);
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = is_true ? 1.0 : 0.0;
    }
  }
  return V7_OK;
}

static enum v7_err parse_logical_or(struct v7 *v7) {
  TRY(parse_logical_and(v7));
  if (v7->cur_tok == TOK_LOGICAL_OR) {
    int sp1 = v7->sp;
    next_tok(v7);
    TRY(parse_logical_and(v7));
    if (EXECUTING(v7->flags)) {
      struct v7_val *v1 = v7->stack[sp1 - 1], *v2 = v7_top(v7)[-1];
      int is_true = v7_is_true(v1) || v7_is_true(v2);
      TRY(v7_make_and_push(v7, V7_TYPE_BOOL));
      v7_top(v7)[-1]->v.num = is_true ? 1.0 : 0.0;
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err parse_ternary(struct v7 *v7) {
  TRY(parse_logical_or(v7));

  if (v7->cur_tok == TOK_QUESTION) {
    int old_flags = v7->flags;
    int condition_true = 1;

    if (EXECUTING(v7->flags)) {
      CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
      condition_true = v7_is_true(v7_top(v7)[-1]);
      TRY(inc_stack(v7, -1));  /* Remove condition result */
    }

    EXPECT(TOK_QUESTION);
    if (!condition_true || !EXECUTING(old_flags)) v7->flags |= V7_NO_EXEC;
    TRY(parse_expression(v7));
    EXPECT(TOK_COLON);
    v7->flags = old_flags;
    if (condition_true || !EXECUTING(old_flags)) v7->flags |= V7_NO_EXEC;
    TRY(parse_expression(v7));
    v7->flags = old_flags;
  }

  return V7_OK;
}

static enum v7_err do_assign(struct v7 *v7, struct v7_val *obj, const char *key,
                             unsigned long key_len, enum v7_tok tok) {
  if (EXECUTING(v7->flags)) {
    struct v7_val **top = v7_top(v7), *a = top[-2], *b = top[-1];

    /*
     * Stack layout at this point (assuming stack grows down):
     *
     *          | object's value (rvalue)    |    top[-2]
     *          +----------------------------+
     *          | expression value (lvalue)  |    top[-1]
     *          +----------------------------+
     * top -->  |       nothing yet          |
     */
    switch (tok) {
      case TOK_ASSIGN:
        CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
        TRY(v7_setv(v7, obj, V7_TYPE_STR, V7_TYPE_OBJ, key, key_len, 1,
                    b));  /* TODO(vrz) ERROR */
        return V7_OK;
      case TOK_PLUS_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_PLUS));
        break;
      case TOK_MINUS_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_MINUS));
        break;
      case TOK_MUL_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_MUL));
        break;
      case TOK_DIV_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_DIV));
        break;
      case TOK_REM_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_REM));
        break;
      case TOK_XOR_ASSIGN:
        TRY(arith(v7, a, b, a, TOK_XOR));
        break;
      default:
        return V7_NOT_IMPLEMENTED;
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err parse_expression(struct v7 *v7) {
  int old_sp = v7->sp;

  /* Set up the reference to be the current execution context */
  v7->cur_obj = v7->ctx;
  v7->key = NULL;

  /* TODO(lsm): parse_yield() should be here, do we want to implement it? */
  TRY(parse_ternary(v7));

  /* Parse assignment */
  if (v7->cur_tok >= TOK_ASSIGN && v7->cur_tok <= TOK_OR_ASSIGN) {
    /* Remember current reference */
    const char *key = v7->key;
    unsigned long key_len = v7->key_len;
    struct v7_val *cur_obj = v7->cur_obj;
    enum v7_tok op = v7->cur_tok;

    next_tok(v7);

    /* This recursion implements right-to-left association for assignment */
    TRY(parse_expression(v7));
    if (EXECUTING(v7->flags)) {
      TRY(do_assign(v7, cur_obj, key, key_len, op));
    }
  }

  /* Collapse stack, leave only one value on top */
  if (EXECUTING(v7->flags)) {
    struct v7_val *result = v7_top(v7)[-1];
    INC_REF_COUNT(result);
    TRY(inc_stack(v7, old_sp - v7->sp));
    TRY(v7_push(v7, result));
    assert(result->ref_count > 1);
    v7_freeval(v7, result);
  } else {
    TRY(inc_stack(v7, old_sp - v7->sp));
  }

  return V7_OK;
}

static enum v7_err parse_declaration(struct v7 *v7) { /* <#parse_decl#> */
  int old_sp = v7_sp(v7);

  EXPECT(TOK_VAR);
  do {
    const char *key = v7->tok;
    unsigned long key_len = v7->tok_len;

    inc_stack(v7, old_sp - v7_sp(v7));
    EXPECT(TOK_IDENTIFIER);
    if (v7->cur_tok == TOK_ASSIGN) {
      next_tok(v7);
      TRY(parse_expression(v7));
      if (EXECUTING(v7->flags)) {
        TRY(do_assign(v7, v7->ctx, key, key_len, TOK_ASSIGN));
      }
    }
  } while (v7->cur_tok == TOK_IDENTIFIER);

  return V7_OK;
}

static enum v7_err parse_if_statement(struct v7 *v7, int *has_return) {
  int old_flags = v7->flags, condition_true;

  EXPECT(TOK_IF);
  EXPECT(TOK_OPEN_PAREN);
  TRY(parse_expression(v7)); /* Evaluate condition, pushed on stack */
  EXPECT(TOK_CLOSE_PAREN);

  if (EXECUTING(old_flags)) {
    /* If condition is false, do not execute "if" body */
    CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
    condition_true = v7_is_true(v7_top_val(v7));
    if (!condition_true) v7->flags |= V7_NO_EXEC;
    TRY(inc_stack(v7, -1)); /* Cleanup condition result from the stack */
  }
  TRY(parse_compound_statement(v7, has_return));

  if (v7->cur_tok == TOK_ELSE) {
    next_tok(v7);
    v7->flags = old_flags;
    if (!EXECUTING(old_flags) || condition_true) v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
  }

  v7->flags = old_flags;  /* Restore old execution flags */
  return V7_OK;
}

static enum v7_err parse_for_in_statement(struct v7 *v7, int has_var,
                                          int *has_return) {
  const char *tok = v7->tok;
  unsigned long tok_len = v7->tok_len;
  struct v7_pstate s_block;

  EXPECT(TOK_IDENTIFIER);
  EXPECT(TOK_IN);
  TRY(parse_expression(v7));
  EXPECT(TOK_CLOSE_PAREN);
  s_block = v7->pstate;

  /* Execute loop body */
  if (!EXECUTING(v7->flags)) {
    TRY(parse_compound_statement(v7, has_return));
  } else {
    int old_sp = v7->sp;
    struct v7_val *obj = v7_top(v7)[-1];
    struct v7_val *scope = has_var ? v7->ctx : &v7->root_scope;
    struct v7_prop *prop;

    CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_ERROR);
    for (prop = obj->props; prop != NULL; prop = prop->next) {
      TRY(v7_setv(v7, scope, V7_TYPE_STR, V7_TYPE_OBJ, tok, tok_len, 1,
                  prop->key));
      v7->pstate = s_block;
      TRY(parse_compound_statement(v7, has_return)); /* Loop body */
      TRY(inc_stack(v7, old_sp - v7->sp));           /* Clean up stack */
    }
  }

  return V7_OK;
}

static enum v7_err parse_for_statement(struct v7 *v7, int *has_return) {
  int is_true, old_flags = v7->flags, has_var = 0;
  struct v7_pstate s2, s3, s_block, s_end;

  EXPECT(TOK_FOR);
  EXPECT(TOK_OPEN_PAREN);

  if (v7->cur_tok == TOK_VAR) {
    has_var++;
    next_tok(v7);
  }

  if (v7->cur_tok == TOK_IDENTIFIER && lookahead(v7) == TOK_IN) {
    return parse_for_in_statement(v7, has_var, has_return);
#if 0
  } else if (v7->cur_tok == TOK_IDENTIFIER && has_var) {
    printf("(%s)\n", "rr");
    TRY(parse_declaration(v7));
#endif
  } else {
    TRY(parse_expression(v7));
  }

  EXPECT(TOK_SEMICOLON);

  /* Pass through the loop, don't execute it, just remember locations */
  v7->flags |= V7_NO_EXEC;
  get_v7_state(v7, &s2);
  TRY(parse_expression(v7));  /* expr2 (condition) */
  EXPECT(TOK_SEMICOLON);

  get_v7_state(v7, &s3);
  TRY(parse_expression(v7));  /* expr3  (post-iteration) */
  EXPECT(TOK_CLOSE_PAREN);

  get_v7_state(v7, &s_block);
  TRY(parse_compound_statement(v7, has_return));
  get_v7_state(v7, &s_end);

  v7->flags = old_flags;

  /* Execute loop */
  if (EXECUTING(v7->flags)) {
    int old_sp = v7->sp;
    for (;;) {
      set_v7_state(v7, &s2);
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_expression(v7));  /* Evaluate condition */
      assert(v7->sp > old_sp);
      is_true = !v7_is_true(v7_top(v7)[-1]);
      if (is_true) break;

      set_v7_state(v7, &s_block);
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_compound_statement(v7, has_return));  /* Loop body */
      assert(!EXECUTING(v7->flags) == 0);

      set_v7_state(v7, &s3);
      TRY(parse_expression(v7)); /* expr3  (post-iteration) */

      TRY(inc_stack(v7, old_sp - v7->sp)); /* Clean up stack */
    }
  }

  /* Jump to the code after the loop */
  set_v7_state(v7, &s_end);

  return V7_OK;
}

static enum v7_err parse_while_statement(struct v7 *v7, int *has_return) {
  int is_true, old_flags = v7->flags;
  struct v7_pstate s_cond, s_block, s_end;

  EXPECT(TOK_WHILE);
  EXPECT(TOK_OPEN_PAREN);
  get_v7_state(v7, &s_cond);
  v7->flags |= V7_NO_EXEC;
  TRY(parse_expression(v7));
  EXPECT(TOK_CLOSE_PAREN);

  get_v7_state(v7, &s_block);
  TRY(parse_compound_statement(v7, has_return));
  get_v7_state(v7, &s_end);

  v7->flags = old_flags;

  /* Execute loop */
  if (EXECUTING(v7->flags)) {
    int old_sp = v7->sp;
    for (;;) {
      set_v7_state(v7, &s_cond);
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_expression(v7));  /* Evaluate condition */
      assert(v7->sp > old_sp);
      is_true = !v7_is_true(v7_top_val(v7));
      if (is_true) break;

      set_v7_state(v7, &s_block);
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_compound_statement(v7, has_return));  /* Loop body */
      assert(!EXECUTING(v7->flags) == 0);

      TRY(inc_stack(v7, old_sp - v7->sp));  /* Clean up stack */
    }
  }

  /* Jump to the code after the loop */
  set_v7_state(v7, &s_end);

  return V7_OK;
}

static enum v7_err parse_return_statement(struct v7 *v7, int *has_return) {
  if (EXECUTING(v7->flags)) {
    *has_return = 1;
  }
  EXPECT(TOK_RETURN);
  if (v7->cur_tok != TOK_SEMICOLON && v7->cur_tok != TOK_CLOSE_CURLY) {
    TRY(parse_expression(v7));
  }
  return V7_OK;
}

static enum v7_err parse_try_statement(struct v7 *v7, int *has_return) {
  enum v7_err err_code;
  const char *old_pc = v7->pstate.pc;
  int old_flags = v7->flags, old_line_no = v7->pstate.line_no;

  EXPECT(TOK_TRY);
  CHECK(v7->cur_tok == TOK_OPEN_CURLY, V7_SYNTAX_ERROR);
  err_code = parse_compound_statement(v7, has_return);

  if (!EXECUTING(old_flags) && err_code != V7_OK) {
    return err_code;
  }

  /* If exception has happened, skip the block */
  if (err_code != V7_OK) {
    v7->pstate.pc = old_pc;
    v7->pstate.line_no = old_line_no;
    v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
  }

  /* Process catch/finally blocks */
  CHECK(next_tok(v7) == TOK_IDENTIFIER, V7_SYNTAX_ERROR);

  /* if (test_token(v7, "catch", 5)) { */
  if (v7->cur_tok == TOK_CATCH) {
    const char *key;
    unsigned long key_len;

    EXPECT(TOK_CATCH);
    EXPECT(TOK_OPEN_PAREN);
    key = v7->tok;
    key_len = v7->tok_len;
    EXPECT(TOK_IDENTIFIER);
    EXPECT(TOK_CLOSE_PAREN);

    /* Insert error variable into the namespace */
    if (err_code != V7_OK) {
      TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
      v7_set_class(v7_top_val(v7), V7_CLASS_ERROR);
      v7_setv(v7, v7->ctx, V7_TYPE_STR, V7_TYPE_OBJ, key, key_len, 1,
              v7_top_val(v7));
    }

    /* If there was no exception, do not execute catch block */
    if (!EXECUTING(old_flags) || err_code == V7_OK) v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
    v7->flags = old_flags;

    if (v7->cur_tok == TOK_FINALLY) {
      TRY(parse_compound_statement(v7, has_return));
    }
  } else if (v7->cur_tok == TOK_FINALLY) {
    v7->flags = old_flags;
    TRY(parse_compound_statement(v7, has_return));
  } else {
    v7->flags = old_flags;
    return V7_SYNTAX_ERROR;
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err parse_statement(struct v7 *v7, int *has_return) {
  switch (v7->cur_tok) {
    case TOK_VAR:
      TRY(parse_declaration(v7));
      break;
    case TOK_RETURN:
      TRY(parse_return_statement(v7, has_return));
      break;
    case TOK_IF:
      TRY(parse_if_statement(v7, has_return));
      break;
    case TOK_FOR:
      TRY(parse_for_statement(v7, has_return));
      break;
    case TOK_WHILE:
      TRY(parse_while_statement(v7, has_return));
      break;
    case TOK_TRY:
      TRY(parse_try_statement(v7, has_return));
      break;
    case TOK_DELETE:
      TRY(parse_delete_statement(v7));
      break;
    default:
      TRY(parse_expression(v7));
      break;
  }

  /*
   * Skip optional colons and semicolons.
   * TODO(lsm): follow automatic semicolon insertion rules
   */
  while (v7->cur_tok == TOK_COMMA || v7->cur_tok == TOK_SEMICOLON) {
    next_tok(v7);
  }

  return V7_OK;
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


/*
 * NOTE(lsm): Must be in the same order as enum for keywords. See comment
 * for function get_tok() for rationale for that.
 */
static struct v7_vec s_keywords[] = {
    V7_VEC("break"),      V7_VEC("case"),      V7_VEC("catch"),
    V7_VEC("continue"),   V7_VEC("debugger"),  V7_VEC("default"),
    V7_VEC("delete"),     V7_VEC("do"),        V7_VEC("else"),
    V7_VEC("false"),      V7_VEC("finally"),   V7_VEC("for"),
    V7_VEC("function"),   V7_VEC("if"),        V7_VEC("in"),
    V7_VEC("instanceof"), V7_VEC("new"),       V7_VEC("null"),
    V7_VEC("return"),     V7_VEC("switch"),    V7_VEC("this"),
    V7_VEC("throw"),      V7_VEC("true"),      V7_VEC("try"),
    V7_VEC("typeof"),     V7_VEC("var"),       V7_VEC("void"),
    V7_VEC("while"),      V7_VEC("with")};

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

/* TODO(lsm): use lookup table to speed it up */
static int is_ident_char(int ch) {
  return ch == '$' || ch == '_' || isalnum(ch);
}

static void ident(const char **s) {
  while (is_ident_char((unsigned char) s[0][0])) (*s)++;
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

  /* Scan string literal into the buffer, handle escape sequences */
  while (*s != quote && *s != '\0') {
    switch (*s) {
      case '\\':
        s++;
        switch (*s) {
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
            if (*s == quote) s++;
            break;
        }
        break;
      default:
        break;
    }
    s++;
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
 * Switch statement is by the first character of the input stream. If first
 * character begins with a letter, it could be either keyword or identifier.
 * get_tok() calls ident() which shifts `s` pointer to the end of the word.
 * Now, tokenizer knows that the word begins at `p` and ends at `s`.
 * It calls function kw() to scan over the keywords that start with `p[0]`
 * letter. Therefore, keyword tokens and keyword strings must be in the
 * same order, to let kw() function work properly.
 * If kw() finds a keyword match, it returns keyword token.
 * Otherwise, it returns TOK_IDENTIFIER.
 */
V7_PRIVATE enum v7_tok get_tok(const char **s, double *n) {
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

    default:
      return TOK_END_OF_INPUT;
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
    printf("%d [%.*s]\n", tok, (int)(src - p), p);
    skip_to_next_tok(&src);
    p = src;
  }
  printf("%d [%.*s]\n", tok, (int)(src - p), p);

  return 0;
}
#endif
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


struct v7 *v7_create(void) {
  static int prototypes_initialized = 0;
  struct v7 *v7 = NULL;

  if (prototypes_initialized == 0) {
    prototypes_initialized++;
    init_stdlib();  /* One-time initialization */
  }

  if ((v7 = (struct v7 *) calloc(1, sizeof(*v7))) != NULL) {
    v7_set_class(&v7->root_scope, V7_CLASS_OBJECT);
    v7->root_scope.proto = &s_global;
    v7->root_scope.ref_count = 1;
    v7->ctx = &v7->root_scope;
  }

  return v7;
}

struct v7_val *v7_global(struct v7 *v7) {
  return &v7->root_scope;
}

void v7_destroy(struct v7 **v7) {
  if (v7 == NULL || v7[0] == NULL) return;
  assert(v7[0]->sp >= 0);
  inc_stack(v7[0], -v7[0]->sp);
  v7[0]->root_scope.ref_count = 1;
  v7_freeval(v7[0], &v7[0]->root_scope);
  free_values(v7[0]);
  free_props(v7[0]);
  free(v7[0]);
  v7[0] = NULL;
}

struct v7_val *v7_push_number(struct v7 *v7, double num) {
  return push_number(v7, num) == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_push_bool(struct v7 *v7, int is_true) {
  return push_bool(v7, is_true) == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_push_string(struct v7 *v7, const char *str, unsigned long n,
                              int own) {
  return push_string(v7, str, n, own) == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_push_func(struct v7 *v7, v7_func_t func) {
  return push_func(v7, func) == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_push_new_object(struct v7 *v7) {
  return push_new_object(v7) == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_push_val(struct v7 *v7, struct v7_val *v) {
  return v7_push(v7, v) == V7_OK ? v : NULL;
}

enum v7_type v7_type(const struct v7_val *v) { return v->type; }

double v7_number(const struct v7_val *v) { return v->v.num; }

const char *v7_string(const struct v7_val *v, unsigned long *plen) {
  if (plen != NULL) *plen = v->v.str.len;
  return v->v.str.buf;
}

struct v7_val *v7_get(struct v7_val *obj, const char *key) {
  struct v7_val k = v7_str_to_val(key);
  return get2(obj, &k);
}

int v7_is_true(const struct v7_val *v) {
  return (v->type == V7_TYPE_BOOL && v->v.num != 0.0) ||
         (v->type == V7_TYPE_NUM && v->v.num != 0.0 && !isnan(v->v.num)) ||
         (v->type == V7_TYPE_STR && v->v.str.len > 0) ||
         (v->type == V7_TYPE_OBJ);
}

enum v7_err v7_append(struct v7 *v7, struct v7_val *arr, struct v7_val *val) {
  struct v7_prop **head, *prop;
  CHECK(v7_is_class(arr, V7_CLASS_ARRAY), V7_INTERNAL_ERROR);
  /* Append to the end of the list, to make indexing work */
  for (head = &arr->v.array; *head != NULL; head = &head[0]->next) {
  }
  prop = mkprop(v7);
  CHECK(prop != NULL, V7_OUT_OF_MEMORY);
  prop->next = *head;
  *head = prop;
  prop->key = NULL;
  prop->val = val;
  INC_REF_COUNT(val);
  return V7_OK;
}

void v7_copy(struct v7 *v7, struct v7_val *orig, struct v7_val *v) {
  struct v7_prop *p;

  switch (v->type) {
    case V7_TYPE_OBJ:
      for (p = orig->props; p != NULL; p = p->next) {
        v7_set2(v7, v, p->key, p->val);
      }
      break;
      /* TODO(lsm): add the rest of types */
    default:
      abort();
      break;
  }
}

const char *v7_get_error_string(const struct v7 *v7) {
  return v7->error_message;
}

struct v7_val *v7_call(struct v7 *v7, struct v7_val *this_obj, int num_args) {
  v7_call2(v7, this_obj, num_args, 0);
  return v7_top_val(v7);
}

enum v7_err v7_set(struct v7 *v7, struct v7_val *obj, const char *key,
                   struct v7_val *val) {
  return v7_setv(v7, obj, V7_TYPE_STR, V7_TYPE_OBJ, key, strlen(key), 1, val);
}

enum v7_err v7_del(struct v7 *v7, struct v7_val *obj, const char *key) {
  return v7_del2(v7, obj, key, strlen(key));
}

static void arr_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m, *head = v->v.array;
  int n = snprintf(buf, bsiz, "%s", "[");

  for (m = head; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != head) n += snprintf(buf + n, bsiz - n, "%s", ", ");
    v7_stringify(m->val, buf + n, bsiz - n);
    n = (int)strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "]");
}

static void obj_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m, *head = v->props;
  int n = snprintf(buf, bsiz, "%s", "{");

  for (m = head; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != head) n += snprintf(buf + n, bsiz - n, "%s", ", ");
    v7_stringify(m->key, buf + n, bsiz - n);
    n = (int)strlen(buf);
    n += snprintf(buf + n, bsiz - n, "%s", ": ");
    v7_stringify(m->val, buf + n, bsiz - n);
    n = (int)strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "}");
}

char *v7_stringify(const struct v7_val *v, char *buf, int bsiz) {
  if (v->type == V7_TYPE_UNDEF) {
    snprintf(buf, bsiz, "%s", "undefined");
  } else if (v->type == V7_TYPE_NULL) {
    snprintf(buf, bsiz, "%s", "null");
  } else if (is_bool(v)) {
    snprintf(buf, bsiz, "%s", v->v.num ? "true" : "false");
  } else if (is_num(v)) {
    /* TODO: check this on 32-bit arch */
    if (INFINITY == v->v.num)
      snprintf(buf, bsiz, "Infinity");
    else if (-INFINITY == v->v.num)
      snprintf(buf, bsiz, "-Infinity");
    else if (isnan(v->v.num))
      snprintf(buf, bsiz, "NaN");
    else if (v->v.num > ((uint64_t)1 << 52) || ceil(v->v.num) != v->v.num)
      snprintf(buf, bsiz, "%lg", v->v.num);
    else
      snprintf(buf, bsiz, "%ld", (unsigned long)v->v.num);
  } else if (is_string(v)) {
    snprintf(buf, bsiz, "%.*s", (int)v->v.str.len, v->v.str.buf);
  } else if (v7_is_class(v, V7_CLASS_ARRAY)) {
    arr_to_string(v, buf, bsiz);
  } else if (v7_is_class(v, V7_CLASS_FUNCTION)) {
    if (v->fl.fl.js_func) {
      snprintf(buf, bsiz, "'function%s'", v->v.func.source_code);
    } else {
      snprintf(buf, bsiz, "'c_func_%p'", v->v.c_func);
    }
  } else if (v7_is_class(v, V7_CLASS_REGEXP)) {
    int sz = snprintf(buf, bsiz, "/%s/", v->v.str.buf);
    if (v->fl.fl.re_g) sz += snprintf(buf + sz, bsiz, "g");
    if (v->fl.fl.re_i) sz += snprintf(buf + sz, bsiz, "i");
    if (v->fl.fl.re_m) snprintf(buf + sz, bsiz, "m");
  } else if (v->type == V7_TYPE_OBJ) {
    obj_to_string(v, buf, bsiz);
  } else {
    snprintf(buf, bsiz, "??");
  }

  buf[bsiz - 1] = '\0';
  return buf;
}

struct v7_val *v7_exec(struct v7 *v7, const char *source_code) {
  enum v7_err er = do_exec(v7, "<exec>", source_code, 0);
  return v7->sp > 0 && er == V7_OK ? v7_top_val(v7) : NULL;
}

struct v7_val *v7_exec_file(struct v7 *v7, const char *path) {
  FILE *fp;
  char *p;
  long file_size;
  int old_sp = v7->sp;
  enum v7_err status = V7_INTERNAL_ERROR;

  if ((fp = fopen(path, "r")) == NULL) {
  } else if (fseek(fp, 0, SEEK_END) != 0 || (file_size = ftell(fp)) <= 0) {
    fclose(fp);
  } else if ((p = (char *) calloc(1, (size_t) file_size + 1)) == NULL) {
    fclose(fp);
  } else {
    rewind(fp);
    fread(p, 1, (size_t) file_size, fp);
    fclose(fp);
    status = do_exec(v7, path, p, v7->sp);
    free(p);
  }

  return v7->sp > old_sp && status == V7_OK ? v7_top_val(v7) : NULL;
  /* return status; */
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


typedef unsigned short ast_skip_t;

struct ast_node_def {
  const char *name;  /* tag name, for debugging and serialization */
  size_t fixed_len;  /* bytes */
  int num_skips;     /* number of skips */
  int num_subtrees;  /* number of fixed subtrees */
};

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
 * they don't have a termination tag; all nodes whose position is before the skip
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
V7_PRIVATE struct ast_node_def ast_node_defs[] = {
  {"NOP", 0, 0, 0}, /* struct {} */
  /*
   * struct {
   *   ast_skip_t end;
   *   child body[];
   * end:
   * }
   */
  {"SCRIPT", 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child decls[];
   * end:
   * }
   */
  {"VAR", 0, 1, 0},
  /*
   * struct {
   *   child name; // TODO(mkm): inline
   *   child expr;
   * }
   */
  {"VAR_DECL", 0, 0, 2},
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
  {"IF", 0, 2, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   ast_skip_t body;
   *   child name;
   *   child params[];
   * body:
   *   child body[];
   * end:
   * }
   */
  {"FUNC", 0, 2, 1},
  {"ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"REM_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"MUL_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"DIV_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"XOR_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"PLUS_ASSIGN", 0, 0, 2}, /* struct { child left, right; } */
  {"MINUS_ASSIGN", 0, 0, 2},   /* struct { child left, right; } */
  {"OR_ASSIGN", 0, 0, 2},      /* struct { child left, right; } */
  {"AND_ASSIGN", 0, 0, 2},     /* struct { child left, right; } */
  {"LSHIFT_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"RSHIFT_ASSIGN", 0, 0, 2},  /* struct { child left, right; } */
  {"URSHIFT_ASSIGN", 0, 0, 2}, /* struct { child left, right; } */
  {"IDENT", 4 + sizeof(char *), 0, 0},  /* struct { char var; } */
  {"NUM", 8, 0, 0},                     /* struct { double n; } */
  {"STRING", 4 + sizeof(char *), 0, 0}, /* struct { uint32_t len, char *s; } */
  /*
   * struct {
   *   ast_skip_t end;
   *   child body[];
   * end:
   * }
   */
  {"SEQ", 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child cond;
   *   child body[];
   * end:
   * }
   */
  {"WHILE", 0, 1, 1},
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
  {"DOWHILE", 0, 2, 0},
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
  {"FOR", 0, 2, 3},
  {"COND", 0, 0, 3},     /* struct { child cond, iftrue, iffalse; } */
  {"DEBUGGER", 0, 0, 0}, /* struct {} */
  {"BREAK", 0, 0, 0},    /* struct {} */
  /*
   * struct {
   *   child label; // TODO(mkm): inline
   * }
   */
  {"LAB_BREAK", 0, 0, 1},
  {"CONTINUE", 0, 0, 0},  /* struct {} */
  /*
   * struct {
   *   child label; // TODO(mkm): inline
   * }
   */
  {"LAB_CONTINUE", 0, 0, 1},
  {"RETURN", 0, 0, 0},     /* struct {} */
  {"VAL_RETURN", 0, 0, 1}, /* struct { child expr; } */
  {"THROW", 0, 0, 1},      /* struct { child expr; } */
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
  {"TRY", 0, 3, 1},
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
  {"SWITCH", 0, 2, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child val;
   *   child stmts[];
   * end:
   * }
   */
  {"CASE", 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child stmts[];
   * end:
   * }
   */
  {"DEFAULT", 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child expr;
   *   child body[];
   * end:
   * }
   */
  {"WITH", 0, 1, 1},
  {"LOG_OR", 0, 0, 2},  /* struct { child left, right; } */
  {"LOG_AND", 0, 0, 2}, /* struct { child left, right; } */
  {"OR", 0, 0, 2},      /* struct { child left, right; } */
  {"XOR", 0, 0, 2},     /* struct { child left, right; } */
  {"AND", 0, 0, 2},     /* struct { child left, right; } */
  {"EQ", 0, 0, 2},      /* struct { child left, right; } */
  {"EQ_EQ", 0, 0, 2},   /* struct { child left, right; } */
  {"NE", 0, 0, 2},      /* struct { child left, right; } */
  {"NE_NE", 0, 0, 2},   /* struct { child left, right; } */
  {"LE", 0, 0, 2},      /* struct { child left, right; } */
  {"LT", 0, 0, 2},      /* struct { child left, right; } */
  {"GE", 0, 0, 2},      /* struct { child left, right; } */
  {"GT", 0, 0, 2},      /* struct { child left, right; } */
  {"IN", 0, 0, 2},      /* struct { child left, right; } */
  {"INSTANCEOF", 0, 0, 2},  /* struct { child left, right; } */
  {"LSHIFT", 0, 0, 2},      /* struct { child left, right; } */
  {"RSHIFT", 0, 0, 2},      /* struct { child left, right; } */
  {"URSHIFT", 0, 0, 2},     /* struct { child left, right; } */
  {"ADD", 0, 0, 2},         /* struct { child left, right; } */
  {"SUB", 0, 0, 2},         /* struct { child left, right; } */
  {"REM", 0, 0, 2},         /* struct { child left, right; } */
  {"MUL", 0, 0, 2},         /* struct { child left, right; } */
  {"DIV", 0, 0, 2},         /* struct { child left, right; } */
  {"POS", 0, 0, 1},         /* struct { child expr; } */
  {"NEG", 0, 0, 1},         /* struct { child expr; } */
  {"NOT", 0, 0, 1},         /* struct { child expr; } */
  {"LOGICAL_NOT", 0, 0, 1}, /* struct { child expr; } */
  {"VOID", 0, 0, 1},        /* struct { child expr; } */
  {"DELETE", 0, 0, 1},      /* struct { child expr; } */
  {"TYPEOF", 0, 0, 1},      /* struct { child expr; } */
  {"PREINC", 0, 0, 1},      /* struct { child expr; } */
  {"PREDEC", 0, 0, 1},      /* struct { child expr; } */
  {"POSTINC", 0, 0, 1},     /* struct { child expr; } */
  {"POSTDEC", 0, 0, 1},     /* struct { child expr; } */
  /*
   * struct {
   *   child expr;
   *   child ident; // TODO(mkm): inline
   * }
   */
  {"MEMBER", 0, 0, 2},
  /*
   * struct {
   *   child expr;
   *   child index;
   * }
   */
  {"INDEX", 0, 0, 2},
  /*
   * struct {
   *   ast_skip_t end;
   *   child expr;
   *   child args[];
   * end:
   * }
   */
  {"CALL", 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child expr;
   *   child args[];
   * end:
   * }
   */
  {"NEW", 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child elements[];
   * end:
   * }
   */
  {"ARRAY", 0, 1, 0},
  /*
   * struct {
   *   ast_skip_t end;
   *   child props[];
   * end:
   * }
   */
  {"OBJECT", 0, 1, 0},
  /*
   * struct {
   *   child name; // TODO(mkm): inline
   *   child expr;
   * }
   */
  {"PROP", 0, 0, 2},
  /*
   * struct {
   *   ast_skip_t end;
   *   child name; // TODO(mkm): inline
   *   child body[];
   * end:
   * }
   */
  {"GETTER", 0, 1, 1},
  /*
   * struct {
   *   ast_skip_t end;
   *   child name; // TODO(mkm): inline
   *   child param; // TODO(mkm): reuse func decl?
   *   child body[];
   * end:
   * }
   */
  {"SETTER", 0, 1, 2},
  {"THIS", 0, 0, 0},  /* struct {} */
  {"TRUE", 0, 0, 0},  /* struct {} */
  {"FALSE", 0, 0, 0}, /* struct {} */
  {"NULL", 0, 0, 0},  /* struct {} */
  {"UNDEF", 0, 0, 0}, /* struct {} */
};

V7_STATIC_ASSERT(AST_MAX_TAG == ARRAY_SIZE(ast_node_defs), bad_node_defs);

/*
 * Code and API based on Fossa IO buffers.
 * TODO(mkm): optimize to our specific use case or fully reuse fossa.
 */

/* Initializes an AST buffer. */
V7_PRIVATE void ast_init(struct ast *ast, size_t initial_size) {
  ast->len = ast->size = 0;
  ast->buf = NULL;
  ast_resize(ast, initial_size);
}

/* Frees the space allocated for the iobuffer and resets the iobuf structure. */
V7_PRIVATE void ast_free(struct ast *ast) {
  if (ast->buf != NULL) {
    free(ast->buf);
    ast_init(ast, 0);
  }
}

/*
 * Resize an AST buffer.
 *
 * If `new_size` is smaller than buffer's `len`, the
 * resize is not performed.
 */
V7_PRIVATE void ast_resize(struct ast *a, size_t new_size) {
  char *p;
  if ((new_size > a->size || (new_size < a->size && new_size >= a->len)) &&
      (p = (char *) realloc(a->buf, new_size)) != NULL) {
    a->size = new_size;
    a->buf = p;
  }
}

/* Shrinks the ast size to just fit it's length. */
V7_PRIVATE void ast_trim(struct ast *ast) {
  ast_resize(ast, ast->len);
}

/*
 * Appends data to the AST.
 *
 * It returns the amount of bytes appended.
 */
V7_PRIVATE size_t ast_append(struct ast *a, const char *buf, size_t len) {
  return ast_insert(a, a->len, buf, len);
}

/*
 * Inserts data at a specified offset in the AST.
 *
 * Existing data will be shifted forwards and the buffer will
 * be grown if necessary.
 * It returns the amount of bytes inserted.
 */
V7_PRIVATE size_t ast_insert(struct ast *a, size_t off, const char *buf,
                             size_t len) {
  char *p = NULL;

  assert(a != NULL);
  assert(a->len <= a->size);
  assert(off <= a->len);

  /* check overflow */
  if (~(size_t)0 - (size_t)a->buf < len)
    return 0;

  if (a->len + len <= a->size) {
    memmove(a->buf + off + len, a->buf + off, a->len - off);
    if (buf != NULL) {
      memcpy(a->buf + off, buf, len);
    }
    a->len += len;
  } else if ((p = (char *)
              realloc(a->buf,
                      (a->len + len) * AST_SIZE_MULTIPLIER)) != NULL) {
    a->buf = p;
    memmove(a->buf + off + len, a->buf + off, a->len - off);
    if (buf != NULL) {
      memcpy(a->buf + off, buf, len);
    }
    a->len += len;
    a->size = a->len * AST_SIZE_MULTIPLIER;
  } else {
    len = 0;
  }

  return len;
}

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
V7_PRIVATE size_t ast_add_node(struct ast *a, enum ast_tag tag) {
  size_t start = a->len;
  uint8_t t = (uint8_t) tag;
  struct ast_node_def *d = &ast_node_defs[tag];

  assert(tag < AST_MAX_TAG);

  ast_append(a, (char *)&t, sizeof(t));
  ast_append(a, NULL, d->fixed_len + sizeof(ast_skip_t) * d->num_skips);
  return start + 1;
}

V7_PRIVATE size_t ast_insert_node(struct ast *a, size_t start,
                                  enum ast_tag tag) {
  uint8_t t = (uint8_t) tag;
  struct ast_node_def *d = &ast_node_defs[tag];

  assert(tag < AST_MAX_TAG);

  ast_insert(a, start, NULL, d->fixed_len + sizeof(ast_skip_t) * d->num_skips);
  ast_insert(a, start, (char *)&t, sizeof(t));

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
V7_PRIVATE size_t ast_set_skip(struct ast *a, size_t start,
                               enum ast_which_skip skip) {
  uint8_t *p = (uint8_t *) a->buf + start + skip * sizeof(ast_skip_t);
  uint16_t delta = a->len - start;
  enum ast_tag tag;

  /* assertion, to be optimizable out */
  tag = (enum ast_tag) (uint8_t) * (a->buf + start - 1);
  struct ast_node_def *def = &ast_node_defs[tag];
  assert((int) skip < def->num_skips);

  p[0] = delta >> 8;
  p[1] = delta & 0xff;
  return a->len;
}

V7_PRIVATE size_t ast_get_skip(struct ast *a, ast_off_t pos,
                               enum ast_which_skip skip) {
  uint8_t * p = (uint8_t *) a->buf + pos + skip * sizeof(ast_skip_t);
  return pos + (p[1] | p[0] << 8);
}

V7_PRIVATE enum ast_tag ast_fetch_tag(struct ast *a, ast_off_t *pos) {
  return (enum ast_tag) (uint8_t) * (a->buf + (*pos)++);
}

/*
 * Assumes a cursor positioned right after a tag.
 *
 * TODO(mkm): add doc, find better name.
 */
V7_PRIVATE void ast_move_to_children(struct ast *a, ast_off_t *pos) {
  enum ast_tag tag;
  tag = (enum ast_tag) (uint8_t) * (a->buf + *pos - 1);
  struct ast_node_def *def = &ast_node_defs[tag];
  *pos += def->fixed_len + def->num_skips * sizeof(ast_skip_t);
}

/* Helper to add a NUM node. */
V7_PRIVATE void ast_add_num(struct ast *a, double num) {
  size_t start = ast_add_node(a, AST_NUM);
  memcpy(a->buf + start, &num, sizeof(num));
}

static void ast_set_string(char *buf, const char *name, size_t len) {
  uint32_t slen = (uint32_t) len;
  /* 4GB ought to be enough for anybody */
  if (sizeof(size_t) == 8) {
    assert((len & 0xFFFFFFFF00000000) == 0);
  }
  memcpy(buf, &slen, sizeof(slen));
  memcpy(buf + sizeof(slen), &name, sizeof(char *));
}

/* Helper to add an IDENT node. */
V7_PRIVATE void ast_add_ident(struct ast *a, const char *name, size_t len) {
  size_t start = ast_add_node(a, AST_IDENT);
  ast_set_string(a->buf + start, name, len);
}

/* Helper to add a STRING node. */
V7_PRIVATE void ast_add_string(struct ast *a, const char *name, size_t len) {
  size_t start = ast_add_node(a, AST_STRING);
  ast_set_string(a->buf + start, name, len);
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

static void ast_dump_tree(FILE *fp, struct ast *a, ast_off_t *pos, int depth) {
  enum ast_tag tag = ast_fetch_tag(a, pos);
  struct ast_node_def *def = &ast_node_defs[tag];
  ast_off_t skips = *pos;
  int i;
  double dv;

  for (i = 0; i < depth; i++) {
    fprintf(fp, "  ");
  }

  fprintf(fp, "%s", def->name);

  switch (tag) {
    case AST_NUM:
      memcpy(&dv, a->buf + *pos, sizeof(dv));
      fprintf(fp, " %lf\n", dv);
      break;
    case AST_IDENT:
      fprintf(fp, " %.*s\n", * (int *) (a->buf + *pos), * (char **) (a->buf + *pos + sizeof(uint32_t)));
      break;
    case AST_STRING:
      fprintf(fp, " \"%.*s\"\n", * (int *) (a->buf + *pos), * (char **) (a->buf + *pos + sizeof(uint32_t)));
      break;
    default:
      fprintf(fp, "\n");
  }
  *pos += def->fixed_len;
  *pos += sizeof(ast_skip_t) * def->num_skips;

  for (i = 0; i < def->num_subtrees; i++) {
    ast_dump_tree(fp, a, pos, depth + 1);
  }

  if (ast_node_defs[tag].num_skips) {
    /*
     * first skip always encodes end of the last children sequence.
     * so unless we care how the subtree sequences are grouped together
     * (and we currently don't) we can just read until the end of that skip.
     */
    size_t end = ast_get_skip(a, skips, AST_END_SKIP);

    comment_at_depth(fp, "...", depth + 1);
    while (*pos < end) {
      int s;
      for (s = ast_node_defs[tag].num_skips - 1; s > 0; s--) {
        if (*pos == ast_get_skip(a, skips, s)) {
          comment_at_depth(fp, "%d ->", depth + 1, s);
          break;
        }
      }
      ast_dump_tree(fp, a, pos, depth + 1);
    }
  }
}

/* Dumps an AST to stdout. */
V7_PRIVATE void ast_dump(FILE *fp, struct ast *a, ast_off_t pos) {
  ast_dump_tree(fp, a, &pos, 0);
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


#define ACCEPT(t)                                   \
  (((v7)->cur_tok == (t)) ? next_tok((v7)), 1 : 0)

#define PARSE(p) TRY(aparse_ ## p(v7, a))
#define PARSE_ARG(p, arg) TRY(aparse_ ## p(v7, a, arg))

static enum v7_err aparse_expression(struct v7 *, struct ast *);
static enum v7_err aparse_statement(struct v7 *, struct ast *);
static enum v7_err aparse_statements(struct v7 *, struct ast *, int);
static enum v7_err aparse_terminal(struct v7 *, struct ast *);
static enum v7_err aparse_assign(struct v7 *, struct ast *);
static enum v7_err aparse_memberexpr(struct v7 *, struct ast *);
static enum v7_err aparse_funcdecl(struct v7 *, struct ast *, int);

static enum v7_err aparse_ident(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok == TOK_IDENTIFIER) {
    ast_add_ident(a, v7->tok, v7->tok_len);  /* TODO(mkm): symbol table */
    next_tok(v7);
    return V7_OK;
  }
  return V7_ERROR;
}

static enum v7_err aparse_prop(struct v7 *v7, struct ast *a) {
  size_t start;
  if (v7->cur_tok == TOK_IDENTIFIER &&
      strncmp(v7->tok, "get", v7->tok_len) == 0 &&
      lookahead(v7) != TOK_COLON) {
    start = ast_add_node(a, AST_GETTER);
    next_tok(v7);
    PARSE(ident);
    EXPECT(TOK_OPEN_PAREN);
    EXPECT(TOK_CLOSE_PAREN);
    aparse_statements(v7, a, 1);
    ast_set_skip(a, start, AST_END_SKIP);
  } else if (v7->cur_tok == TOK_IDENTIFIER &&
             strncmp(v7->tok, "set", v7->tok_len) == 0 &&
             lookahead(v7) != TOK_COLON) {
    start = ast_add_node(a, AST_SETTER);
    next_tok(v7);
    PARSE(ident);
    EXPECT(TOK_OPEN_PAREN);
    PARSE(ident);
    EXPECT(TOK_CLOSE_PAREN);
    aparse_statements(v7, a, 1);
    ast_set_skip(a, start, AST_END_SKIP);
  } else {
    ast_add_node(a, AST_PROP);
    PARSE(terminal);
    EXPECT(TOK_COLON);
    PARSE(assign);
  }
  return V7_OK;
}

static enum v7_err aparse_terminal(struct v7 *v7, struct ast *a) {
  size_t start;
  switch (v7->cur_tok) {
    case TOK_OPEN_PAREN:
      next_tok(v7);
      PARSE(expression);
      EXPECT(TOK_CLOSE_PAREN);
      break;
    case TOK_OPEN_BRACKET:
      next_tok(v7);
      start = ast_add_node(a, AST_ARRAY);
      if (v7->cur_tok != TOK_CLOSE_BRACKET) {
        /* TODO(mkm): simplify please */
        do {
          if (v7->cur_tok == TOK_COMMA) {
            ast_add_node(a, AST_NOP);
            if (lookahead(v7) == TOK_CLOSE_BRACKET) {
              next_tok(v7);
              break;
            }
          } else {
            PARSE(assign);
          }
        } while(ACCEPT(TOK_COMMA));
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
        } while(ACCEPT(TOK_COMMA));
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
    case TOK_NUMBER:
      ast_add_num(a, v7->cur_tok_dbl);
      next_tok(v7);
      break;
    case TOK_STRING_LITERAL:
      ast_add_string(a, v7->tok + 1, v7->tok_len - 2);
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

static enum v7_err aparse_arglist(struct v7 *v7, struct ast *a) {
  if (v7->cur_tok != TOK_CLOSE_PAREN) {
    do {
      PARSE(assign);
    } while (ACCEPT(TOK_COMMA));
  }
  return V7_OK;
}

static enum v7_err aparse_newexpr(struct v7 *v7, struct ast *a) {
  size_t start;
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
      PARSE_ARG(funcdecl, 0);
      break;
    default:
      PARSE(terminal);
      break;
  }
  return V7_OK;
}

static enum v7_err aparse_memberexpr(struct v7 *v7, struct ast *a) {
  size_t pos = a->len;
  PARSE(newexpr);

  for (;;) {
    switch (v7->cur_tok) {
      case TOK_DOT:
        next_tok(v7);
        PARSE(ident);
        ast_insert_node(a, pos, AST_MEMBER);
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
  }
}

static enum v7_err aparse_callexpr(struct v7 *v7, struct ast *a) {
  size_t pos = a->len;
  PARSE(newexpr);

  for (;;) {
    switch (v7->cur_tok) {
      case TOK_DOT:
        next_tok(v7);
        PARSE(ident);
        ast_insert_node(a, pos, AST_MEMBER);
        break;
      case TOK_OPEN_BRACKET:
        next_tok(v7);
        PARSE(expression);
        EXPECT(TOK_CLOSE_BRACKET);
        ast_insert_node(a, pos, AST_INDEX);
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

static enum v7_err aparse_postfix(struct v7 *v7, struct ast *a) {
  size_t pos = a->len;
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
      break;  /* nothing */
  }
  return V7_OK;
}

enum v7_err aparse_prefix(struct v7 *v7, struct ast *a) {
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
        return aparse_postfix(v7, a);
    }
  }
}

static enum v7_err aparse_binary(struct v7 *v7, struct ast *a,
                                 int level) {
  struct {
    int len;
    struct {
      enum v7_tok start_tok;
      enum v7_tok end_tok;
      enum ast_tag start_ast;
    } parts[2];
  } levels[] = {
    {1, {{TOK_ASSIGN, TOK_URSHIFT_ASSIGN, AST_ASSIGN}, {0, 0, 0}}},
    {1, {{TOK_QUESTION, TOK_QUESTION, AST_COND}, {0, 0, 0}}},
    {1, {{TOK_LOGICAL_OR, TOK_LOGICAL_OR, AST_LOGICAL_OR}, {0, 0, 0}}},
    {1, {{TOK_LOGICAL_AND, TOK_LOGICAL_AND, AST_LOGICAL_AND}, {0, 0, 0}}},
    {1, {{TOK_OR, TOK_OR, AST_OR}, {0, 0, 0}}},
    {1, {{TOK_XOR, TOK_XOR, AST_XOR}, {0, 0, 0}}},
    {1, {{TOK_AND, TOK_AND, AST_AND}, {0, 0, 0}}},
    {1, {{TOK_EQ, TOK_NE_NE, AST_EQ}, {0, 0, 0}}},
    {2, {{TOK_LE, TOK_GT, AST_LE}, {TOK_IN, TOK_INSTANCEOF, AST_IN}}},
    {1, {{TOK_LSHIFT, TOK_URSHIFT, AST_LSHIFT}, {0, 0, 0}}},
    {1, {{TOK_PLUS, TOK_MINUS, AST_ADD}, {0, 0, 0}}},
    {1, {{TOK_REM, TOK_DIV, AST_REM}, {0, 0, 0}}}
  };

  int i;
  enum v7_tok tok;
  enum ast_tag ast;
  size_t pos = a->len;

  if (level == (int) ARRAY_SIZE(levels) - 1) {
    PARSE(prefix);
  } else {
    PARSE_ARG(binary, level + 1);
  }

  for (i = 0; i < levels[level].len; i++) {
    tok = levels[level].parts[i].start_tok;
    ast = levels[level].parts[i].start_ast;
    do {
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
        PARSE_ARG(binary, level);
        ast_insert_node(a, pos, ast);
      }
    } while(ast++, tok++ < levels[level].parts[i].end_tok);
  }

  return V7_OK;
}

static enum v7_err aparse_assign(struct v7 *v7, struct ast *a) {
  return aparse_binary(v7, a, 0);
}

static enum v7_err aparse_expression(struct v7 *v7, struct ast *a) {
  size_t pos = a->len;
  int group = 0;
  do {
    PARSE(assign);
  } while(ACCEPT(TOK_COMMA) && (group = 1));
  if (group) {
    ast_insert_node(a, pos, AST_SEQ);
  }
  return V7_OK;
}

static enum v7_err end_of_statement(struct v7 *v7) {
  if (v7->cur_tok == TOK_SEMICOLON ||
      v7->cur_tok == TOK_END_OF_INPUT ||
      v7->cur_tok == TOK_CLOSE_CURLY ||
      v7->after_newline) {
    return V7_OK;
  }
  return V7_ERROR;
}

static enum v7_err aparse_statements(struct v7 *v7, struct ast *a,
                                     int require_block) {
  if (ACCEPT(TOK_OPEN_CURLY)) {
    while (!ACCEPT(TOK_CLOSE_CURLY)) {
      PARSE(statement);
    }
    return V7_OK;
  } else if (!require_block) {
    PARSE(statement);
    return V7_OK;
  }
  return V7_ERROR;
}

static enum v7_err aparse_var(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_VAR);
  do {
    ast_add_node(a, AST_VAR_ITEM);
    PARSE(ident);
    if (ACCEPT(TOK_ASSIGN)) {
      PARSE(assign);
    } else {
      ast_add_node(a, AST_NOP);
    }
  } while (ACCEPT(TOK_COMMA));
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static int aparse_optional(struct v7 *v7, struct ast *a,
                    enum v7_tok terminator) {
  if (v7->cur_tok != terminator) {
    return 1;
  }
  ast_add_node(a, AST_NOP);
  return 0;
}

static enum v7_err aparse_if(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_IF);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  PARSE_ARG(statements, 0);
  ast_set_skip(a, start, AST_END_IF_TRUE_SKIP);
  if (ACCEPT(TOK_ELSE)) {
    PARSE_ARG(statements, 0);
  }
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_while(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_WHILE);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  PARSE_ARG(statements, 0);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_dowhile(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_DOWHILE);
  PARSE_ARG(statements, 0);
  ast_set_skip(a, start, AST_DO_WHILE_COND_SKIP);
  EXPECT(TOK_WHILE);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_for(struct v7 *v7, struct ast *a) {
  /* TODO(mkm): for in, for of, for each in */
  size_t start = ast_add_node(a, AST_FOR);
  EXPECT(TOK_OPEN_PAREN);

  if(aparse_optional(v7, a, TOK_SEMICOLON)) {
    if (ACCEPT(TOK_VAR)) {
      aparse_var(v7, a);
    } else {
      PARSE(expression);
    }
  }
  EXPECT(TOK_SEMICOLON);
  if (aparse_optional(v7, a, TOK_SEMICOLON)) {
    PARSE(expression);
  }
  EXPECT(TOK_SEMICOLON);
  if (aparse_optional(v7, a, TOK_CLOSE_PAREN)) {
    PARSE(expression);
  }
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_FOR_BODY_SKIP);
  PARSE_ARG(statements, 0);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_switch(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_SWITCH);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  EXPECT(TOK_OPEN_CURLY);
  while (v7->cur_tok != TOK_CLOSE_CURLY) {
    size_t case_start;
    switch (v7->cur_tok) {
      case TOK_CASE:
        next_tok(v7);
        case_start = ast_add_node(a, AST_CASE);
        PARSE(expression);
        EXPECT(TOK_COLON);
        while (v7->cur_tok != TOK_CASE &&
               v7->cur_tok != TOK_DEFAULT &&
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
        while (v7->cur_tok != TOK_CASE &&
               v7->cur_tok != TOK_DEFAULT &&
               v7->cur_tok != TOK_CLOSE_CURLY) {
          PARSE(statement);
        }
        ast_set_skip(a, case_start, AST_END_SKIP);
        break;
      default:
        return V7_ERROR;
    }
  }
  EXPECT(TOK_CLOSE_CURLY);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_try(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_TRY);
  PARSE_ARG(statements, 1);
  ast_set_skip(a, start, AST_TRY_CATCH_SKIP);
  if (ACCEPT(TOK_CATCH)) {
    EXPECT(TOK_OPEN_PAREN);
    PARSE(ident);
    EXPECT(TOK_CLOSE_PAREN);
    PARSE_ARG(statements, 1);
  }
  ast_set_skip(a, start, AST_TRY_FINALLY_SKIP);
  if (ACCEPT(TOK_FINALLY)) {
    PARSE_ARG(statements, 1);
  }
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_with(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_WITH);
  EXPECT(TOK_OPEN_PAREN);
  PARSE(expression);
  EXPECT(TOK_CLOSE_PAREN);
  PARSE_ARG(statements, 0);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

#define PARSE_WITH_OPT_ARG(tag, arg_tag, arg_parser)  \
  do {                                                \
    if (end_of_statement(v7) == V7_OK) {              \
      ast_add_node(a, tag);                           \
    } else {                                          \
      ast_add_node(a, arg_tag);                       \
      PARSE(arg_parser);                              \
    }                                                 \
  } while(0)                                          \

static enum v7_err aparse_statement(struct v7 *v7, struct ast *a) {
  switch (v7->cur_tok) {
    case TOK_SEMICOLON:
      next_tok(v7);
      return V7_OK;  /* empty statement */
    case TOK_OPEN_CURLY:  /* block */
      aparse_statements(v7, a, 1);
      break;
    case TOK_IF:
      next_tok(v7);
      return aparse_if(v7, a); /* returning because no semicolon required */
    case TOK_WHILE:
      next_tok(v7);
      return aparse_while(v7, a);
    case TOK_DO:
      next_tok(v7);
      return aparse_dowhile(v7, a);
    case TOK_FOR:
      next_tok(v7);
      return aparse_for(v7, a);
    case TOK_TRY:
      next_tok(v7);
      return aparse_try(v7, a);
    case TOK_SWITCH:
      next_tok(v7);
      return aparse_switch(v7, a);
    case TOK_WITH:
      next_tok(v7);
      return aparse_with(v7, a);
    case TOK_BREAK:
      next_tok(v7);
      PARSE_WITH_OPT_ARG(AST_BREAK, AST_LABELED_BREAK, ident);
      break;
    case TOK_CONTINUE:
      next_tok(v7);
      PARSE_WITH_OPT_ARG(AST_CONTINUE, AST_LABELED_CONTINUE, ident);
      break;
    case TOK_RETURN:
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
      aparse_var(v7, a);
      break;
    default:
      PARSE(expression);
      break;
  }

  /* TODO(mkm): labels, function statements */

  TRY(end_of_statement(v7));
  ACCEPT(TOK_SEMICOLON);  /* swallow optional semicolon */
  return V7_OK;
}

static enum v7_err aparse_funcdecl(struct v7 *v7, struct ast *a,
                                   int require_named) {
  size_t start = ast_add_node(a, AST_FUNC);
  if (aparse_ident(v7, a) == V7_ERROR) {
    if (require_named) {
      return V7_ERROR;
    }
    ast_add_ident(a, "?", 1);  /* TODO(mkm): symboltable API */
  }
  EXPECT(TOK_OPEN_PAREN);
  PARSE(arglist);
  EXPECT(TOK_CLOSE_PAREN);
  ast_set_skip(a, start, AST_FUNC_BODY_SKIP);
  PARSE_ARG(statements, 1);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

static enum v7_err aparse_body(struct v7 *v7, struct ast *a,
                               enum v7_tok end) {
  while (v7->cur_tok != end) {
    if (ACCEPT(TOK_FUNCTION)) {
      PARSE_ARG(funcdecl, 1);
    } else {
      PARSE(statement);
    }
  }
  return V7_OK;
}

static enum v7_err aparse_script(struct v7 *v7, struct ast *a) {
  size_t start = ast_add_node(a, AST_SCRIPT);
  PARSE_ARG(body, TOK_END_OF_INPUT);
  ast_set_skip(a, start, AST_END_SKIP);
  return V7_OK;
}

V7_PRIVATE enum v7_err aparse(struct ast *a, const char *src, int verbose) {
  enum v7_err err;
  struct v7 *v7 = v7_create();
  v7->pstate.source_code = v7->pstate.pc = src;
  v7->pstate.file_name = "<stdin>";
  v7->pstate.line_no = 1;

  next_tok(v7);
  err = aparse_script(v7, a);
  if (err == V7_OK && v7->cur_tok != TOK_END_OF_INPUT) {
    printf("WARNING parse input not consumed\n");
  }
  if (verbose && err != V7_OK) {
      printf("Parse error at at line %d\n", v7->pstate.line_no);
  }
  return err;
}
/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */


#ifdef V7_EXE
int main(int argc, char *argv[]) {
  struct v7 *v7 = v7_create();
  int i; /*, error_code; */

  /* Execute inline code */
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
      if (!v7_exec(v7, argv[i + 1])) {
        fprintf(stderr, "Error executing [%s]: %s\n", argv[i + 1],
                v7_get_error_string(v7));
      }
      i++;
    }
  }

  /* Execute files */
  for (; i < argc; i++) {
    if (!v7_exec_file(v7, argv[i])) {
      fprintf(stderr, "%s\n", v7_get_error_string(v7));
    }
  }

  v7_destroy(&v7);
  return EXIT_SUCCESS;
}
#endif
