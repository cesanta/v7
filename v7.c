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

/* MSVC6 doesn't have standard C math constants defined */
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
#define INFINITY    atof("INFINITY")  /* TODO: fix this */
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

/* Different classes of V7_TYPE_OBJ type */
enum v7_class {
  V7_CLASS_NONE, V7_CLASS_ARRAY, V7_CLASS_BOOLEAN, V7_CLASS_DATE,
  V7_CLASS_ERROR, V7_CLASS_FUNCTION, V7_CLASS_NUMBER, V7_CLASS_OBJECT,
  V7_CLASS_REGEXP, V7_CLASS_STRING, V7_NUM_CLASSES
};

enum v7_tok {
  TOK_END_OF_INPUT, TOK_NUMBER, TOK_STRING_LITERAL, TOK_IDENTIFIER,

  /* Punctuators */
  TOK_OPEN_CURLY, TOK_CLOSE_CURLY, TOK_OPEN_PAREN, TOK_CLOSE_PAREN, TOK_COMMA,
  TOK_OPEN_BRACKET, TOK_CLOSE_BRACKET, TOK_DOT, TOK_COLON, TOK_SEMICOLON,
  TOK_EQ, TOK_EQ_EQ, TOK_NE, TOK_NE_NE,  /* Equality ops, in this order */
  TOK_ASSIGN, TOK_REM_ASSIGN, TOK_MUL_ASSIGN, TOK_DIV_ASSIGN, TOK_XOR_ASSIGN,
  TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN, TOK_LOGICAL_OR_ASSING,
  TOK_LOGICAL_AND_ASSING, TOK_LSHIFT_ASSIGN, TOK_RSHIFT_ASSIGN, /* Assigns */
  TOK_AND, TOK_LOGICAL_OR,
  TOK_PLUS_PLUS, TOK_PLUS, TOK_MINUS_MINUS, TOK_MINUS, TOK_LOGICAL_AND,
  TOK_OR, TOK_QUESTION, TOK_TILDA, TOK_REM, TOK_MUL, TOK_DIV, TOK_XOR,
  TOK_LE, TOK_LT, TOK_GE, TOK_GT,  /* Relational ops, must go in this order */
  TOK_LSHIFT, TOK_RSHIFT, TOK_NOT,

  /* Keywords. must be in the same order as tokenizer.c::s_keywords array */
  TOK_BREAK, TOK_CASE, TOK_CATCH, TOK_CONTINUE, TOK_DEBUGGER, TOK_DEFAULT,
  TOK_DELETE, TOK_DO, TOK_ELSE, TOK_FALSE, TOK_FINALLY, TOK_FOR, TOK_FUNCTION,
  TOK_IF, TOK_IN, TOK_INSTANCEOF, TOK_NEW, TOK_NULL,
  TOK_RETURN, TOK_SWITCH, TOK_THIS, TOK_THROW, TOK_TRUE, TOK_TRY, TOK_TYPEOF,
  TOK_UNDEFINED, TOK_VAR, TOK_VOID, TOK_WHILE, TOK_WITH,

  /* TODO(lsm): process these reserved words too */
  TOK_CLASS, TOK_ENUM, TOK_EXTENDS, TOK_SUPER, TOK_CONST, TOK_EXPORT,
  TOK_IMPORT, TOK_IMPLEMENTS, TOK_LET, TOK_PRIVATE, TOK_PUBLIC, TOK_INTERFACE,
  TOK_PACKAGE, TOK_PROTECTED, TOK_STATIC, TOK_YIELD,

  NUM_TOKENS
};

typedef void (*v7_prop_func_t)(struct v7_val *this_obj, struct v7_val *result);

struct v7_prop {
  struct v7_prop *next;
  struct v7_val *key;
  struct v7_val *val;
  unsigned short flags;
#define V7_PROP_NOT_WRITABLE   1  /* property is not changeable */
#define V7_PROP_NOT_ENUMERABLE 2  /* not enumerable in for..in loop */
#define V7_PROP_NOT_DELETABLE  4  /* delete-ing this property must fail */
#define V7_PROP_ALLOCATED      8  /* v7_prop must be free()-ed */
};

/* Vector, describes some memory location pointed by 'p' with length 'len' */
struct v7_vec {
  const char *p;
  int len;
};

struct v7_string {
  char *buf;                /* Pointer to buffer with string data */
  unsigned long len;        /* String length */
  char loc[16];             /* Small strings are stored here */
};

struct v7_func {
  char *source_code;        /* \0-terminated function source code */
  int line_no;              /* Line number where function begins */
  struct v7_val *var_obj;   /* Function var object: var decls and func defs */
};

union v7_scalar {
  char *regex;              /* \0-terminated regex */
  double num;               /* Holds "Number" or "Boolean" value */
  struct v7_string str;     /* Holds "String" value */
  struct v7_func func;      /* \0-terminated function code */
  struct v7_prop *array;    /* List of array elements */
  v7_func_t c_func;         /* Pointer to the C function */
  v7_prop_func_t prop_func; /* Object's property function, e.g. String.length */
};

struct v7_val {
  struct v7_val *next;
  struct v7_val *proto;       /* Prototype */
  struct v7_val *ctor;        /* Constructor object */
  struct v7_prop *props;      /* Object's key/value list */
  union v7_scalar v;          /* The value itself */
  enum v7_type type;          /* Value type */
  enum v7_class cls;          /* Object's internal [[Class]] property */
  short ref_count;            /* Reference counter */

  unsigned short flags;       /* Flags - defined below */
#define V7_VAL_ALLOCATED   1  /* Whole "struct v7_val" must be free()-ed */
#define V7_STR_ALLOCATED   2  /* v.str.buf must be free()-ed */
#define V7_JS_FUNC         4  /* Function object is a JavsScript code */
#define V7_PROP_FUNC       8  /* Function object is a native property function */
#define V7_VAL_DEALLOCATED 16 /* Value has been deallocated */
};

#define V7_MKVAL(_p,_t,_c,_v) {0,(_p),0,0,{(_v)},(_t),(_c),0,0}

struct v7_pstate {
  const char *file_name;
  const char *source_code;
  const char *pc;              /* Current parsing position */
  int line_no;                 /* Line number */
};

struct v7 {
  struct v7_val root_scope;   /* "global" object (root-level execution context) */
  struct v7_val *stack[200];  /* TODO: make it non-fixed, auto-grow */
  int sp;                     /* Stack pointer */
  int flags;
#define V7_SCANNING  1        /* Pre-scan to initialize lexical scopes, no exec */
#define V7_NO_EXEC   2        /* Non-executing code block: if (false) { block } */

  struct v7_pstate pstate;    /* Parsing state */
  enum v7_tok cur_tok;        /* Current token */
  const char *tok;            /* Parsed terminal token (ident, number, string) */
  unsigned long tok_len;      /* Length of the parsed terminal token */
  double cur_tok_dbl;

  const char *key;            /* Key for the assignment operation */
  unsigned long key_len;      /* Key length for the assignment operation */

  char error_message[100];    /* Placeholder for the error message */

  struct v7_val *cur_obj;     /* Current namespace object ('x=1; x.y=1;', etc) */
  struct v7_val *this_obj;    /* Current "this" object */
  struct v7_val *ctx;         /* Current execution context */
  struct v7_val *cf;          /* Currently executing function */
  struct v7_val *functions;   /* List of declared function */
  struct v7_val *free_values; /* List of free (deallocated) values */
  struct v7_prop *free_props; /* List of free (deallocated) props */
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

#define CHECK(cond, code) do { \
  if (!(cond)) { \
    snprintf(v7->error_message, sizeof(v7->error_message), \
      "%s line %d: %s", v7->pstate.file_name, v7->pstate.line_no, \
      v7_strerror(code)); \
    return (code); \
  } } while (0)

#ifdef _WIN32
#define TRACE_CALL  /* printf */
#else
#define TRACE_CALL(fmt, ...)
#endif

extern int __lev;
#define TRY(call) do { \
  enum v7_err _e; \
  TRACE_CALL("> %s %d\n", #call, __LINE__);  \
  _e = call;      \
  CHECK(_e == V7_OK, _e);     \
  TRACE_CALL("< %s %d\n", #call, __LINE__); \
} while (0)

/* Print current function name and stringified object */
#define TRACE_OBJ(O) do { char x[4000]; printf("==> %s [%s]\n", __func__, \
  O == NULL ? "@" : v7_stringify(O, x, sizeof(x))); } while (0)

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
#define SET_RO_PROP_V(obj, name, val) \
  do { \
    static struct v7_val key = MKOBJ(&s_prototypes[V7_CLASS_STRING]); \
    static struct v7_prop prop = { NULL, &key, &val, 0 }; \
    v7_init_str(&key, (char *) (name), strlen(name), 0); \
    prop.next = obj.props; \
    obj.props = &prop; \
  } while (0)

/* Adds read-only attribute with given initializers to the object "_o" */
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

/* Adds property function "_func" with key "_name" to the object "_obj" */
#define SET_PROP_FUNC(_obj, _name, _func) \
    SET_RO_PROP2(_obj, _name, V7_TYPE_NULL, 0, prop_func, _func, V7_PROP_FUNC)

/* Adds method "_func" with key "_name" to the object "_obj" */
#define SET_METHOD(_obj, _name, _func) \
  do {  \
    static struct v7_val _val = MKOBJ(&s_prototypes[V7_CLASS_STRING]); \
    v7_set_class(&_val, V7_CLASS_FUNCTION); \
    _val.v.c_func = (_func); \
    SET_RO_PROP_V(_obj, _name, _val); \
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
V7_PRIVATE void init_standard_constructor(enum v7_class cls, v7_func_t ctor);
V7_PRIVATE enum v7_err inc_stack(struct v7 *v7, int incr);
V7_PRIVATE void inc_ref_count(struct v7_val *);
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
V7_PRIVATE enum v7_err push_string(struct v7 *, const char *, unsigned long, int);
V7_PRIVATE enum v7_err push_func(struct v7 *v7, v7_func_t func);
V7_PRIVATE enum v7_err push_new_object(struct v7 *v7);
V7_PRIVATE enum v7_err push_number(struct v7 *v7, double num);
V7_PRIVATE void free_props(struct v7 *v7);
V7_PRIVATE void free_values(struct v7 *v7);
V7_PRIVATE struct v7_val v7_str_to_val(const char *buf);
V7_PRIVATE enum v7_err v7_del2(struct v7 *v7, struct v7_val *,
  const char *, unsigned long);

/* Generic function to set an attribute in an object. */
V7_PRIVATE enum v7_err v7_setv(struct v7 *v7, struct v7_val *obj,
                    enum v7_type key_type, enum v7_type val_type, ...);

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
/*
 * Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
 * Copyright (c) 2013 Cesanta Software Limited
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

/*
 * This is a regular expression library that implements a subset of Perl RE.
 * Please refer to README.md for a detailed reference.
 */

#ifndef SLRE_HEADER_DEFINED
#define SLRE_HEADER_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

struct slre_cap {
  const char *ptr;
  int len;
};


int slre_match(const char *regexp, const char *buf, int buf_len,
               struct slre_cap *caps, int num_caps, int flags);

/* Possible flags for slre_match() */
enum { SLRE_IGNORE_CASE = 1 };


/* slre_match() failure codes */
#define SLRE_NO_MATCH               -1
#define SLRE_UNEXPECTED_QUANTIFIER  -2
#define SLRE_UNBALANCED_BRACKETS    -3
#define SLRE_INTERNAL_ERROR         -4
#define SLRE_INVALID_CHARACTER_SET  -5
#define SLRE_INVALID_METACHARACTER  -6
#define SLRE_CAPS_ARRAY_TOO_SMALL   -7
#define SLRE_TOO_MANY_BRANCHES      -8
#define SLRE_TOO_MANY_BRACKETS      -9

#ifdef __cplusplus
}
#endif

#endif  /* SLRE_HEADER_DEFINED */
/*
 * Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
 * Copyright (c) 2013 Cesanta Software Limited
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

#include <stdio.h>
#include <ctype.h>
#include <string.h>


#define MAX_BRANCHES 100
#define MAX_BRACKETS 100
#define FAIL_IF(condition, error_code) if (condition) return (error_code)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(ar) (sizeof(ar) / sizeof((ar)[0]))
#endif

#ifdef SLRE_DEBUG
#define DBG(x) printf x
#else
#define DBG(x)
#endif

struct bracket_pair {
  const char *ptr;  /* Points to the first char after '(' in regex  */
  int len;          /* Length of the text between '(' and ')'       */
  int branches;     /* Index in the branches array for this pair    */
  int num_branches; /* Number of '|' in this bracket pair           */
};

struct branch {
  int bracket_index;    /* index for 'struct bracket_pair brackets' */
                        /* array defined below                      */
  const char *schlong;  /* points to the '|' character in the regex */
};

struct regex_info {
  /*
   * Describes all bracket pairs in the regular expression.
   * First entry is always present, and grabs the whole regex.
   */
  struct bracket_pair brackets[MAX_BRACKETS];
  int num_brackets;

  /*
   * Describes alternations ('|' operators) in the regular expression.
   * Each branch falls into a specific branch pair.
   */
  struct branch branches[MAX_BRANCHES];
  int num_branches;

  /* Array of captures provided by the user */
  struct slre_cap *caps;
  int num_caps;

  /* E.g. SLRE_IGNORE_CASE. See enum below */
  int flags;
};

static int is_metacharacter(const unsigned char *s) {
  static const char *metacharacters = "^$().[]*+?|\\Ssdbfnrtv";
  return strchr(metacharacters, *s) != NULL;
}

static int op_len(const char *re) {
  return re[0] == '\\' && re[1] == 'x' ? 4 : re[0] == '\\' ? 2 : 1;
}

static int set_len(const char *re, int re_len) {
  int len = 0;

  while (len < re_len && re[len] != ']') {
    len += op_len(re + len);
  }

  return len <= re_len ? len + 1 : -1;
}

static int get_op_len(const char *re, int re_len) {
  return re[0] == '[' ? set_len(re + 1, re_len - 1) + 1 : op_len(re);
}

static int is_quantifier(const char *re) {
  return re[0] == '*' || re[0] == '+' || re[0] == '?';
}

static int toi(int x) {
  return isdigit(x) ? x - '0' : x - 'W';
}

static int hextoi(const unsigned char *s) {
  return (toi(tolower(s[0])) << 4) | toi(tolower(s[1]));
}

static int match_op(const unsigned char *re, const unsigned char *s,
                    struct regex_info *info) {
  int result = 0;
  switch (*re) {
    case '\\':
      /* Metacharacters */
      switch (re[1]) {
        case 'S': FAIL_IF(isspace(*s), SLRE_NO_MATCH); result++; break;
        case 's': FAIL_IF(!isspace(*s), SLRE_NO_MATCH); result++; break;
        case 'd': FAIL_IF(!isdigit(*s), SLRE_NO_MATCH); result++; break;
        case 'b': FAIL_IF(*s != '\b', SLRE_NO_MATCH); result++; break;
        case 'f': FAIL_IF(*s != '\f', SLRE_NO_MATCH); result++; break;
        case 'n': FAIL_IF(*s != '\n', SLRE_NO_MATCH); result++; break;
        case 'r': FAIL_IF(*s != '\r', SLRE_NO_MATCH); result++; break;
        case 't': FAIL_IF(*s != '\t', SLRE_NO_MATCH); result++; break;
        case 'v': FAIL_IF(*s != '\v', SLRE_NO_MATCH); result++; break;

        case 'x':
          /* Match byte, \xHH where HH is hexadecimal byte representaion */
          FAIL_IF(hextoi(re + 2) != *s, SLRE_NO_MATCH);
          result++;
          break;

        default:
          /* Valid metacharacter check is done in bar() */
          FAIL_IF(re[1] != s[0], SLRE_NO_MATCH);
          result++;
          break;
      }
      break;

    case '|': FAIL_IF(1, SLRE_INTERNAL_ERROR); break;
    case '$': FAIL_IF(1, SLRE_NO_MATCH); break;
    case '.': result++; break;

    default:
      if (info->flags & SLRE_IGNORE_CASE) {
        FAIL_IF(tolower(*re) != tolower(*s), SLRE_NO_MATCH);
      } else {
        FAIL_IF(*re != *s, SLRE_NO_MATCH);
      }
      result++;
      break;
  }

  return result;
}

static int match_set(const char *re, int re_len, const char *s,
                     struct regex_info *info) {
  int len = 0, result = -1, invert = re[0] == '^';

  if (invert) re++, re_len--;

  while (len <= re_len && re[len] != ']' && result <= 0) {
    /* Support character range */
    if (re[len] != '-' && re[len + 1] == '-' && re[len + 2] != ']' &&
        re[len + 2] != '\0') {
      result = info->flags &&  SLRE_IGNORE_CASE ?
        *s >= re[len] && *s <= re[len + 2] :
        tolower(*s) >= tolower(re[len]) && tolower(*s) <= tolower(re[len + 2]);
      len += 3;
    } else {
      result = match_op((unsigned char *) re + len, (unsigned char *) s, info);
      len += op_len(re + len);
    }
  }
  return (!invert && result > 0) || (invert && result <= 0) ? 1 : -1;
}

static int doh(const char *s, int s_len, struct regex_info *info, int bi);

static int bar(const char *re, int re_len, const char *s, int s_len,
               struct regex_info *info, int bi) {
  /* i is offset in re, j is offset in s, bi is brackets index */
  int i, j, n, step;

  for (i = j = 0; i < re_len && j <= s_len; i += step) {

    /* Handle quantifiers. Get the length of the chunk. */
    step = re[i] == '(' ? info->brackets[bi + 1].len + 2 :
      get_op_len(re + i, re_len - i);

    DBG(("%s [%.*s] [%.*s] re_len=%d step=%d i=%d j=%d\n", __func__,
         re_len - i, re + i, s_len - j, s + j, re_len, step, i, j));

    FAIL_IF(is_quantifier(&re[i]), SLRE_UNEXPECTED_QUANTIFIER);
    FAIL_IF(step <= 0, SLRE_INVALID_CHARACTER_SET);

    if (i + step < re_len && is_quantifier(re + i + step)) {
      DBG(("QUANTIFIER: [%.*s]%c [%.*s]\n", step, re + i,
           re[i + step], s_len - j, s + j));
      if (re[i + step] == '?') {
        int result = bar(re + i, step, s + j, s_len - j, info, bi);
        j += result > 0 ? result : 0;
        i++;
      } else if (re[i + step] == '+' || re[i + step] == '*') {
        int j2 = j, nj = j, n1, n2 = -1, ni, non_greedy = 0;

        /* Points to the regexp code after the quantifier */
        ni = i + step + 1;
        if (ni < re_len && re[ni] == '?') {
          non_greedy = 1;
          ni++;
        }

        do {
          if ((n1 = bar(re + i, step, s + j2, s_len - j2, info, bi)) > 0) {
            j2 += n1;
          }
          if (re[i + step] == '+' && n1 < 0) break;

          if (ni >= re_len) {
            /* After quantifier, there is nothing */
            nj = j2;
          } else if ((n2 = bar(re + ni, re_len - ni, s + j2,
                               s_len - j2, info, bi)) >= 0) {
            /* Regex after quantifier matched */
            nj = j2 + n2;
          }
          if (nj > j && non_greedy) break;
        } while (n1 > 0);

        if (n1 < 0 && re[i + step] == '*' &&
            (n2 = bar(re + ni, re_len - ni, s + j, s_len - j, info, bi)) > 0) {
          nj = j + n2;
        }

        DBG(("STAR/PLUS END: %d %d %d %d %d\n", j, nj, re_len - ni, n1, n2));
        FAIL_IF(re[i + step] == '+' && nj == j, SLRE_NO_MATCH);

        /* If while loop body above was not executed for the * quantifier,  */
        /* make sure the rest of the regex matches                          */
        FAIL_IF(nj == j && ni < re_len && n2 < 0, SLRE_NO_MATCH);

        /* Returning here cause we've matched the rest of RE already */
        return nj;
      }
      continue;
    }

    if (re[i] == '[') {
      n = match_set(re + i + 1, re_len - (i + 2), s + j, info);
      DBG(("SET %.*s [%.*s] -> %d\n", step, re + i, s_len - j, s + j, n));
      FAIL_IF(n <= 0, SLRE_NO_MATCH);
      j += n;
    } else if (re[i] == '(') {
      n = SLRE_NO_MATCH;
      bi++;
      FAIL_IF(bi >= info->num_brackets, SLRE_INTERNAL_ERROR);
      DBG(("CAPTURING [%.*s] [%.*s] [%s]\n",
           step, re + i, s_len - j, s + j, re + i + step));

      if (re_len - (i + step) <= 0) {
        /* Nothing follows brackets */
        n = doh(s + j, s_len - j, info, bi);
      } else {
        int j2;
        for (j2 = 0; j2 <= s_len - j; j2++) {
          if ((n = doh(s + j, s_len - (j + j2), info, bi)) >= 0 &&
              bar(re + i + step, re_len - (i + step),
                  s + j + n, s_len - (j + n), info, bi) >= 0) break;
        }
      }

      DBG(("CAPTURED [%.*s] [%.*s]:%d\n", step, re + i, s_len - j, s + j, n));
      FAIL_IF(n < 0, n);
      if (info->caps != NULL) {
        info->caps[bi - 1].ptr = s + j;
        info->caps[bi - 1].len = n;
      }
      j += n;
    } else if (re[i] == '^') {
      FAIL_IF(j != 0, SLRE_NO_MATCH);
    } else if (re[i] == '$') {
      FAIL_IF(j != s_len, SLRE_NO_MATCH);
    } else {
      FAIL_IF(j >= s_len, SLRE_NO_MATCH);
      n = match_op((unsigned char *) (re + i), (unsigned char *) (s + j), info);
      FAIL_IF(n <= 0, n);
      j += n;
    }
  }

  return j;
}

/* Process branch points */
static int doh(const char *s, int s_len, struct regex_info *info, int bi) {
  const struct bracket_pair *b = &info->brackets[bi];
  int i = 0, len, result;
  const char *p;

  do {
    p = i == 0 ? b->ptr : info->branches[b->branches + i - 1].schlong + 1;
    len = b->num_branches == 0 ? b->len :
      i == b->num_branches ? (int) (b->ptr + b->len - p) :
      (int) (info->branches[b->branches + i].schlong - p);
    DBG(("%s %d %d [%.*s] [%.*s]\n", __func__, bi, i, len, p, s_len, s));
    result = bar(p, len, s, s_len, info, bi);
    DBG(("%s <- %d\n", __func__, result));
  } while (result <= 0 && i++ < b->num_branches);  /* At least 1 iteration */

  return result;
}

static int baz(const char *s, int s_len, struct regex_info *info) {
  int i, result = -1, is_anchored = info->brackets[0].ptr[0] == '^';

  for (i = 0; i <= s_len; i++) {
    result = doh(s + i, s_len - i, info, 0);
    if (result >= 0) {
      result += i;
      break;
    }
    if (is_anchored) break;
  }

  return result;
}

static void setup_branch_points(struct regex_info *info) {
  int i, j;
  struct branch tmp;

  /* First, sort branches. Must be stable, no qsort. Use bubble algo. */
  for (i = 0; i < info->num_branches; i++) {
    for (j = i + 1; j < info->num_branches; j++) {
      if (info->branches[i].bracket_index > info->branches[j].bracket_index) {
        tmp = info->branches[i];
        info->branches[i] = info->branches[j];
        info->branches[j] = tmp;
      }
    }
  }

  /*
   * For each bracket, set their branch points. This way, for every bracket
   * (i.e. every chunk of regex) we know all branch points before matching.
   */
  for (i = j = 0; i < info->num_brackets; i++) {
    info->brackets[i].num_branches = 0;
    info->brackets[i].branches = j;
    while (j < info->num_branches && info->branches[j].bracket_index == i) {
      info->brackets[i].num_branches++;
      j++;
    }
  }
}

static int foo(const char *re, int re_len, const char *s, int s_len,
               struct regex_info *info) {
  int i, step, depth = 0;

  /* First bracket captures everything */
  info->brackets[0].ptr = re;
  info->brackets[0].len = re_len;
  info->num_brackets = 1;

  /* Make a single pass over regex string, memorize brackets and branches */
  for (i = 0; i < re_len; i += step) {
    step = get_op_len(re + i, re_len - i);

    if (re[i] == '|') {
      FAIL_IF(info->num_branches >= (int) ARRAY_SIZE(info->branches),
              SLRE_TOO_MANY_BRANCHES);
      info->branches[info->num_branches].bracket_index =
        info->brackets[info->num_brackets - 1].len == -1 ?
        info->num_brackets - 1 : depth;
      info->branches[info->num_branches].schlong = &re[i];
      info->num_branches++;
    } else if (re[i] == '\\') {
      FAIL_IF(i >= re_len - 1, SLRE_INVALID_METACHARACTER);
      if (re[i + 1] == 'x') {
        /* Hex digit specification must follow */
        FAIL_IF(re[i + 1] == 'x' && i >= re_len - 3,
                SLRE_INVALID_METACHARACTER);
        FAIL_IF(re[i + 1] ==  'x' && !(isxdigit(re[i + 2]) &&
                isxdigit(re[i + 3])), SLRE_INVALID_METACHARACTER);
      } else {
        FAIL_IF(!is_metacharacter((unsigned char *) re + i + 1),
                SLRE_INVALID_METACHARACTER);
      }
    } else if (re[i] == '(') {
      FAIL_IF(info->num_brackets >= (int) ARRAY_SIZE(info->brackets),
              SLRE_TOO_MANY_BRACKETS);
      depth++;  /* Order is important here. Depth increments first. */
      info->brackets[info->num_brackets].ptr = re + i + 1;
      info->brackets[info->num_brackets].len = -1;
      info->num_brackets++;
      FAIL_IF(info->num_caps > 0 && info->num_brackets - 1 > info->num_caps,
              SLRE_CAPS_ARRAY_TOO_SMALL);
    } else if (re[i] == ')') {
      int ind = info->brackets[info->num_brackets - 1].len == -1 ?
        info->num_brackets - 1 : depth;
      info->brackets[ind].len = (int) (&re[i] - info->brackets[ind].ptr);
      DBG(("SETTING BRACKET %d [%.*s]\n",
           ind, info->brackets[ind].len, info->brackets[ind].ptr));
      depth--;
      FAIL_IF(depth < 0, SLRE_UNBALANCED_BRACKETS);
      FAIL_IF(i > 0 && re[i - 1] == '(', SLRE_NO_MATCH);
    }
  }

  FAIL_IF(depth != 0, SLRE_UNBALANCED_BRACKETS);
  setup_branch_points(info);

  return baz(s, s_len, info);
}

int slre_match(const char *regexp, const char *s, int s_len,
               struct slre_cap *caps, int num_caps, int flags) {
  struct regex_info info;

  /* Initialize info structure */
  info.flags = flags;
  info.num_brackets = info.num_branches = 0;
  info.num_caps = num_caps;
  info.caps = caps;

  DBG(("========================> [%s] [%.*s]\n", regexp, s_len, s));
  return foo(regexp, (int) strlen(regexp), s, s_len, &info);
}

V7_PRIVATE struct v7_val s_constructors[V7_NUM_CLASSES];
V7_PRIVATE struct v7_val s_prototypes[V7_NUM_CLASSES];

V7_PRIVATE struct v7_val s_global = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
V7_PRIVATE struct v7_val s_math = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
V7_PRIVATE struct v7_val s_json = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);
V7_PRIVATE struct v7_val s_file = MKOBJ(&s_prototypes[V7_CLASS_OBJECT]);

V7_PRIVATE void obj_sanity_check(const struct v7_val *obj) {
  assert(obj != NULL);
  assert(obj->ref_count >= 0);
  assert(!(obj->flags & V7_VAL_DEALLOCATED));
}

V7_PRIVATE int instanceof(const struct v7_val *obj, const struct v7_val *ctor) {
  obj_sanity_check(obj);
  if (obj->type == V7_TYPE_OBJ && ctor != NULL) {
    while (obj != NULL) {
      if (obj->ctor == ctor) return 1;
      if (obj->proto == obj) break;  // Break on circular reference
      obj = obj->proto;
    }
  }
  return 0;
}

V7_PRIVATE int v7_is_class(const struct v7_val *obj, enum v7_class cls) {
  return instanceof(obj, &s_constructors[cls]);
}

V7_PRIVATE int is_string(const struct v7_val *v) {
  obj_sanity_check(v);
  return v->type == V7_TYPE_STR || v7_is_class(v, V7_CLASS_STRING);
}

V7_PRIVATE int is_num(const struct v7_val *v) {
  obj_sanity_check(v);
  return v->type == V7_TYPE_NUM || v7_is_class(v, V7_CLASS_NUMBER);
}

V7_PRIVATE int is_bool(const struct v7_val *v) {
  obj_sanity_check(v);
  return v->type == V7_TYPE_BOOL || v7_is_class(v, V7_CLASS_BOOLEAN);
}

V7_PRIVATE void inc_ref_count(struct v7_val *v) {
  obj_sanity_check(v);
  v->ref_count++;
}

V7_PRIVATE char *v7_strdup(const char *ptr, unsigned long len) {
  char *p = (char *) malloc(len + 1);
  if (p == NULL) return NULL;
  memcpy(p, ptr, len);
  p[len] = '\0';
  return p;
}

V7_PRIVATE void v7_init_str(struct v7_val *v, const char *p,
  unsigned long len, int own) {
  v->type = V7_TYPE_STR;
  v->proto = &s_prototypes[V7_CLASS_STRING];
  v->v.str.buf = (char *) p;
  v->v.str.len = len;
  v->flags &= ~V7_STR_ALLOCATED;
  if (own) {
    if (len < sizeof(v->v.str.loc) - 1) {
      v->v.str.buf = v->v.str.loc;
      memcpy(v->v.str.loc, p, len);
      v->v.str.loc[len] = '\0';
    } else {
      v->v.str.buf = v7_strdup(p, len);
      v->flags |= V7_STR_ALLOCATED;
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
  if (p->flags & V7_VAL_ALLOCATED) {
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
  } else if (v->type == V7_TYPE_STR && (v->flags & V7_STR_ALLOCATED)) {
    free(v->v.str.buf);
  } else if (v7_is_class(v, V7_CLASS_REGEXP) && (v->flags & V7_STR_ALLOCATED)) {
    free(v->v.regex);
  } else if (v7_is_class(v, V7_CLASS_FUNCTION)) {
    if ((v->flags & V7_STR_ALLOCATED) && (v->flags & V7_JS_FUNC)) {
      free(v->v.func.source_code);
      v7_freeval(v7, v->v.func.var_obj);
    }
  }

  if (v->flags & V7_VAL_ALLOCATED) {
    v->flags &= ~V7_VAL_ALLOCATED;
    v->flags |= ~V7_VAL_DEALLOCATED;
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

  // Free values pushed on stack (like string literals and functions)
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
    v->flags = V7_VAL_ALLOCATED;
    v->type = type;
    switch (type) {
      case V7_TYPE_NUM: v->proto = &s_prototypes[V7_CLASS_NUMBER]; break;
      case V7_TYPE_STR: v->proto = &s_prototypes[V7_CLASS_STRING]; break;
      case V7_TYPE_BOOL: v->proto = &s_prototypes[V7_CLASS_BOOLEAN]; break;
      default: break;
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
      (b->type == V7_TYPE_UNDEF || b->type == V7_TYPE_NULL)) return 0;

  if (is_num(a) && is_num(b)) {
    v7_init_num(&ta, a->v.num);
    v7_init_num(&tb, b->v.num);
    a = &ta; b = &tb;
  }

  if (is_string(a) && is_string(b)) {
    v7_init_str(&ta, a->v.str.buf, a->v.str.len, 0);
    v7_init_str(&tb, b->v.str.buf, b->v.str.len, 0);
    a = &ta; b = &tb;
  }

  if (a->type != b->type) return 1;

  an = a->v.num, bn = b->v.num;
  as = &a->v.str, bs = &b->v.str;

  switch (a->type) {
    case V7_TYPE_NUM:
      return (isinf(an) && isinf(bn)) ||
      (isnan(an) && isnan(bn)) ? 0 : an - bn;
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
  for (; obj != NULL; obj = obj->proto) {
    if (v7_is_class(obj, V7_CLASS_ARRAY) && key->type == V7_TYPE_NUM) {
      int i = (int) key->v.num;
      for (m = obj->v.array; m != NULL; m = m->next) {
        if (i-- == 0) return m;
      }
    } else if (obj->type == V7_TYPE_OBJ) {
      for (m = obj->props; m != NULL; m = m->next) {
        if (cmp(m->key, key) == 0) return m;
      }
    }
    if (own_prop) break;
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

  inc_ref_count(key);
  inc_ref_count(val);
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

  // Find attribute inside object
  if ((m = v7_get2(obj, k, 1)) != NULL) {
    v7_freeval(v7, m->val);
    inc_ref_count(v);
    m->val = v;
  } else {
    TRY(vinsert(v7, &obj->props, k, v));
  }

  return V7_OK;
}

V7_PRIVATE struct v7_val *v7_mkvv(struct v7 *v7, enum v7_type t, va_list *ap) {
  struct v7_val *v = make_value(v7, t);

  // TODO: check for make_value() failure
  switch (t) {
      //case V7_C_FUNC: v->v.c_func = va_arg(*ap, v7_func_t); break;
    case V7_TYPE_NUM:
      v->v.num = va_arg(*ap, double);
      break;
    case V7_TYPE_STR: {
      char *buf = va_arg(*ap, char *);
      unsigned long len = va_arg(*ap, unsigned long);
      int own = va_arg(*ap, int);
      v7_init_str(v, buf, len, own);
    }
      break;
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
                          enum v7_type key_type, enum v7_type val_type, ...) {
  struct v7_val *k = NULL, *v = NULL;
  va_list ap;

  va_start(ap, val_type);
  k = key_type == V7_TYPE_OBJ ?
  va_arg(ap, struct v7_val *) : v7_mkvv(v7, key_type, &ap);
  v = val_type == V7_TYPE_OBJ ?
  va_arg(ap, struct v7_val *) : v7_mkvv(v7, val_type, &ap);
  va_end(ap);

  // TODO: do not leak here
  CHECK(k != NULL && v != NULL, V7_OUT_OF_MEMORY);

  inc_ref_count(k);
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
    "no error", "error", "eval error", "range error", "reference error",
    "syntax error", "type error", "URI error",
    "out of memory", "internal error", "stack overflow", "stack underflow",
    "called non-function", "not implemented", "string literal too long"
  };
  assert(ARRAY_SIZE(strings) == V7_NUM_ERRORS);
  return e >= (int) ARRAY_SIZE(strings) ? "?" : strings[e];
}

V7_PRIVATE struct v7_val **v7_top(struct v7 *v7) {
  return &v7->stack[v7->sp];
}

V7_PRIVATE int v7_sp(struct v7 *v7) {
  return (int) (v7_top(v7) - v7->stack);
}

V7_PRIVATE struct v7_val *v7_top_val(struct v7 *v7) {
  return v7->sp > 0 ? v7->stack[v7->sp - 1] : NULL;
}

V7_PRIVATE enum v7_err v7_push(struct v7 *v7, struct v7_val *v) {
  inc_ref_count(v);
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

  // Prior calls to v7_exec() may have left current_scope modified, reset now
  // TODO(lsm): free scope chain
  v7->this_obj = &v7->root_scope;

  next_tok(v7);
  while ((err == V7_OK) && (v7->cur_tok != TOK_END_OF_INPUT)) {
    // Reset stack on each statement
    if ((err = inc_stack(v7, sp - v7->sp)) == V7_OK) {
      err = parse_statement(v7, &has_ret);
    }
  }

  //printf("%s: [%s] %d %d\n", __func__, file_name, v7->pstate.line_no, err);
  assert(v7->root_scope.proto == &s_global);
  v7->pstate = old_pstate;

  return err;
}

// Convert object to string, push string on stack
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

#ifndef V7_DISABLE_CRYPTO

//////////////////////////////// START OF MD5 THIRD PARTY CODE
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
/////////////////////////////////// END OF MD5 THIRD PARTY CODE

/////////////////////////////////// START OF SHA-1 THIRD PARTY CODE
// SHA-1 in C
// By Steve Reid <sreid@sea-to-sky.net>
// 100% Public Domain

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
  // Forrest: SHA expect BIG_ENDIAN, swap if LITTLE_ENDIAN
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
  // Erase working structures. The order of operations is important,
  // used to ensure that compiler doesn't optimize those out.
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
/////////////////////////////////// START OF SHA-1 THIRD PARTY CODE


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
  inc_ref_count(&s_crypto);

  SET_RO_PROP_V(s_global, "Crypto", s_crypto);
}
#endif  // V7_DISABLE_CRYPTO

V7_PRIVATE enum v7_err Array_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_ARRAY);
  return V7_OK;
}
V7_PRIVATE void Arr_length(struct v7_val *this_obj, struct v7_val *result) {
  struct v7_prop *p;
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
  const struct v7_prop *p1 = * (struct v7_prop **) pa;
  const struct v7_prop *p2 = * (struct v7_prop **) pb;
  return cmp(p2->val, p1->val);
}

V7_PRIVATE enum v7_err Arr_sort(struct v7_c_func_arg *cfa) {
  int i = 0, length = 0;
  struct v7_val *v = cfa->this_obj;
  struct v7_prop *p, **arr;

  // TODO(lsm): do proper error checking
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

V7_PRIVATE enum v7_err Boolean_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_BOOLEAN);
  return V7_OK;
  return V7_OK;
}

V7_PRIVATE void init_boolean(void) {
  init_standard_constructor(V7_CLASS_BOOLEAN, Boolean_ctor);
}

V7_PRIVATE enum v7_err Date_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_DATE);
  return V7_OK;
}

V7_PRIVATE void init_date(void) {
  init_standard_constructor(V7_CLASS_DATE, Date_ctor);
  SET_RO_PROP_V(s_global, "Date", s_constructors[V7_CLASS_DATE]);
}

V7_PRIVATE enum v7_err Error_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_ERROR);
  return V7_OK;
}

V7_PRIVATE void init_error(void) {
  init_standard_constructor(V7_CLASS_ERROR, Error_ctor);
  SET_RO_PROP_V(s_global, "Error", s_constructors[V7_CLASS_ERROR]);
}

V7_PRIVATE enum v7_err Function_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_FUNCTION);
  return V7_OK;
}

V7_PRIVATE void init_function(void) {
  init_standard_constructor(V7_CLASS_FUNCTION, Function_ctor);
  SET_RO_PROP_V(s_global, "Function", s_constructors[V7_CLASS_FUNCTION]);
}

V7_PRIVATE enum v7_err Math_random(struct v7_c_func_arg *cfa) {
  static int srand_called = 0;

  if (!srand_called) {
    srand((unsigned) (unsigned long) cfa);
    srand_called++;
  }

  v7_push_number(cfa->v7, (double) rand() / RAND_MAX);

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
  v7_push_number(cfa->v7, cfa->num_args == 2 ?
              pow(cfa->args[0]->v.num, cfa->args[1]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_floor(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 1 ?
              floor(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_ceil(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 1 ?
              ceil(cfa->args[0]->v.num) : 0.0);
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

V7_PRIVATE enum v7_err Number_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
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
  int len, digits = cfa->num_args > 0 ? (int) cfa->args[0]->v.num : 0;
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

V7_PRIVATE enum v7_err Object_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
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

V7_PRIVATE enum v7_err Regex_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_OBJECT);
  return V7_OK;
}

V7_PRIVATE void init_regex(void) {
  init_standard_constructor(V7_CLASS_REGEXP, Regex_ctor);
  SET_RO_PROP_V(s_global, "RegExp", s_constructors[V7_CLASS_REGEXP]);
}

V7_PRIVATE enum v7_err String_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  struct v7_val *arg = cfa->args[0];
  struct v7 *v7 = cfa->v7;  // Needed for TRY() macro below

  // If argument is not a string, do type conversion
  if (cfa->num_args == 1 && !is_string(arg)) {
    TRY(toString(cfa->v7, arg));
    arg = v7_top_val(cfa->v7);
  }

  if (cfa->num_args == 1 && arg->type == V7_TYPE_STR) {
    v7_init_str(obj, arg->v.str.buf, arg->v.str.len, 1);
  } else {
    v7_init_str(obj, NULL, 0, 0);
  }

  if (cfa->called_as_constructor) {
    cfa->this_obj->type = V7_TYPE_OBJ;
    cfa->this_obj->proto = &s_prototypes[V7_CLASS_STRING];
    cfa->this_obj->ctor = &s_constructors[V7_CLASS_STRING];
  }
  return V7_OK;
}

V7_PRIVATE void Str_length(struct v7_val *this_obj, struct v7_val *result) {
  v7_init_num(result, this_obj->v.str.len);
}

static const char *StrAt(struct v7_c_func_arg *cfa) {
  if (cfa->num_args > 0 && cfa->args[0]->type == V7_TYPE_NUM &&
    cfa->this_obj->type == V7_TYPE_STR &&
    fabs(cfa->args[0]->v.num) < cfa->this_obj->v.str.len) {
    int idx = (int) cfa->args[0]->v.num;
    const char *p = cfa->this_obj->v.str.buf;
    return idx > 0 ? p + idx : p + cfa->this_obj->v.str.len - idx;
  }
  return NULL;
}

V7_PRIVATE enum v7_err Str_charCodeAt(struct v7_c_func_arg *cfa) {
  const char *p = StrAt(cfa);
  v7_push_number(cfa->v7, p == NULL ? NAN : p[0]);
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_charAt(struct v7_c_func_arg *cfa) {
  const char *p = StrAt(cfa);
  v7_push_string(cfa->v7, p, p == NULL ? 0 : 1, 1);
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_match(struct v7_c_func_arg *cfa) {
  struct slre_cap caps[100];
  const struct v7_string *s = &cfa->this_obj->v.str;
  struct v7_val *result;
  int i, n;

  //cfa->result->type = V7_TYPE_NULL;
  memset(caps, 0, sizeof(caps));

  if (cfa->num_args == 1 &&
      v7_is_class(cfa->args[0], V7_CLASS_REGEXP) &&
      (n = slre_match(cfa->args[0]->v.regex, s->buf, (int) s->len,
                      caps, ARRAY_SIZE(caps) - 1, 0)) > 0) {
    result = v7_push_new_object(cfa->v7);
    v7_set_class(result, V7_CLASS_ARRAY);
    v7_append(cfa->v7, result,
              v7_mkv(cfa->v7, V7_TYPE_STR, s->buf, (long) n, 1));
    for (i = 0; i < (int) ARRAY_SIZE(caps); i++) {
      if (caps[i].len == 0) break;
      v7_append(cfa->v7, result,
                v7_mkv(cfa->v7, V7_TYPE_STR, caps[i].ptr, (long) caps[i].len, 1));
    }
  }
  return V7_OK;
}

// Implementation of memmem()
V7_PRIVATE const char *memstr(const char *a, size_t al, const char *b, size_t bl) {
  const char *end;
  if (al == 0 || bl == 0 || al < bl) return NULL;
  for (end = a + (al - bl); a < end; a++) if (!memcmp(a, b, bl)) return a;
  return NULL;
}

V7_PRIVATE enum v7_err Str_split(struct v7_c_func_arg *cfa) {
  const struct v7_string *s = &cfa->this_obj->v.str;
  const char *p1, *p2, *e = s->buf + s->len;
  int limit = cfa->num_args == 2 && cfa->args[1]->type == V7_TYPE_NUM ?
  cfa->args[1]->v.num : -1;
  int num_elems = 0;
  struct v7_val *result = v7_push_new_object(cfa->v7);

  v7_set_class(result, V7_CLASS_ARRAY);
  if (cfa->num_args == 0) {
    v7_append(cfa->v7, result,
              v7_mkv(cfa->v7, V7_TYPE_STR, s->buf, s->len, 1));
  } else if (cfa->args[0]->type == V7_TYPE_STR) {
    const struct v7_string *sep = &cfa->args[0]->v.str;
    if (sep->len == 0) {
      // Separator is empty. Split string by characters.
      for (p1 = s->buf; p1 < e; p1++) {
        if (limit >= 0 && limit <= num_elems) break;
        v7_append(cfa->v7, result, v7_mkv(cfa->v7, V7_TYPE_STR, p1, 1, 1));
        num_elems++;
      }
    } else {
      p1 = s->buf;
      while ((p2 = memstr(p1, e - p1, sep->buf, sep->len)) != NULL) {
        if (limit >= 0 && limit <= num_elems) break;
        v7_append(cfa->v7, result,
                  v7_mkv(cfa->v7, V7_TYPE_STR, p1, p2 - p1, 1));
        p1 = p2 + sep->len;
        num_elems++;
      }
      if (limit < 0 || limit > num_elems) {
        v7_append(cfa->v7, result,
                  v7_mkv(cfa->v7, V7_TYPE_STR, p1, e - p1, 1));
      }
    }
  } else if (instanceof(cfa->args[0], &s_constructors[V7_CLASS_REGEXP])) {
    char regex[MAX_STRING_LITERAL_LENGTH];
    struct slre_cap caps[40];
    int n = 0;

    snprintf(regex, sizeof(regex), "(%s)", cfa->args[0]->v.regex);
    p1 = s->buf;
    while ((n = slre_match(regex, p1, (int) (e - p1),
                           caps, ARRAY_SIZE(caps), 0)) > 0) {
      if (limit >= 0 && limit <= num_elems) break;
      v7_append(cfa->v7, result,
                v7_mkv(cfa->v7, V7_TYPE_STR, p1, caps[0].ptr - p1, 1));
      p1 += n;
      num_elems++;
    }
    if (limit < 0 || limit > num_elems) {
      v7_append(cfa->v7, result,
                v7_mkv(cfa->v7, V7_TYPE_STR, p1, e - p1, 1));
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_indexOf(struct v7_c_func_arg *cfa) {
  double index = -1.0;
  if (cfa->this_obj->type == V7_TYPE_STR &&
      cfa->num_args > 0 &&
      cfa->args[0]->type == V7_TYPE_STR) {
    int i = cfa->num_args > 1 && cfa->args[1]->type == V7_TYPE_NUM ?
    (int) cfa->args[1]->v.num : 0;
    const struct v7_string *a = &cfa->this_obj->v.str,
    *b = &cfa->args[0]->v.str;

    // Scan the string, advancing one byte at a time
    for (; i >= 0 && a->len >= b->len && i <= (int) (a->len - b->len); i++) {
      if (memcmp(a->buf + i, b->buf, b->len) == 0) {
        index = i;
        break;
      }
    }
  }
  v7_push_number(cfa->v7, index);
  return V7_OK;
}

V7_PRIVATE enum v7_err Str_substr(struct v7_c_func_arg *cfa) {
  struct v7_val *result = v7_push_string(cfa->v7, NULL, 0, 0);
  long start = 0;

  if (cfa->num_args > 0 && cfa->args[0]->type == V7_TYPE_NUM) {
    start = (long) cfa->args[0]->v.num;
  }
  if (start < 0) {
    start += (long) cfa->this_obj->v.str.len;
  }
  if (start >= 0 && start < (long) cfa->this_obj->v.str.len) {
    long n = cfa->this_obj->v.str.len - start;
    if (cfa->num_args > 1 && cfa->args[1]->type == V7_TYPE_NUM) {
      n = cfa->args[1]->v.num;
    }
    if (n > 0 && n <= ((long) cfa->this_obj->v.str.len - start)) {
      v7_init_str(result, cfa->this_obj->v.str.buf + start, n, 1);
    }
  }
  return V7_OK;
}

V7_PRIVATE void init_string(void) {
  init_standard_constructor(V7_CLASS_STRING, String_ctor);
  SET_PROP_FUNC(s_prototypes[V7_CLASS_STRING], "length", Str_length);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charCodeAt", Str_charCodeAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "charAt", Str_charAt);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "indexOf", Str_indexOf);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "substr", Str_substr);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "match", Str_match);
  SET_METHOD(s_prototypes[V7_CLASS_STRING], "split", Str_split);

  SET_RO_PROP_V(s_global, "String", s_constructors[V7_CLASS_STRING]);
}

V7_PRIVATE enum v7_err Json_stringify(struct v7_c_func_arg *cfa) {
  v7_push_string(cfa->v7, "implement me", 12, 0);
  // TODO(lsm): implement JSON.stringify
  return V7_OK;
}

V7_PRIVATE void init_json(void) {
  SET_METHOD(s_json, "stringify", Json_stringify);

  v7_set_class(&s_json, V7_CLASS_OBJECT);
  s_json.ref_count = 1;

  SET_RO_PROP_V(s_global, "JSON", s_json);
}

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
  int i, failed = 0;
  struct v7_val *obj = v7_push_new_object(cfa->v7);

  // Push new object as a context for the loading new module
  obj->next = cfa->v7->ctx;
  cfa->v7->ctx = obj;

  for (i = 0; i < cfa->num_args; i++) {
    if (v7_type(cfa->args[i]) != V7_TYPE_STR) return V7_TYPE_ERROR;
    if (!v7_exec_file(cfa->v7, cfa->args[i]->v.str.buf)) {
      failed++;
      break;
    };
  }

  // Pop context, and return it
  cfa->v7->ctx = obj->next;
  if (failed) {
    v7_make_and_push(cfa->v7, V7_TYPE_NULL);
  } else {
    v7_push_val(cfa->v7, obj);
  }

  return V7_OK;
}

V7_PRIVATE enum v7_err Std_exit(struct v7_c_func_arg *cfa) {
  int exit_code = cfa->num_args > 0 ? (int) cfa->args[0]->v.num : EXIT_SUCCESS;
  exit(exit_code);
  return V7_OK;
}

V7_PRIVATE void base64_encode(const unsigned char *src, int src_len, char *dst) {
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

// Convert one byte of encoded base64 input stream to 6-bit chunk
V7_PRIVATE unsigned char from_b64(unsigned char ch) {
  // Inverse lookup map
  V7_PRIVATE const unsigned char tab[128] = {
    255, 255, 255, 255, 255, 255, 255, 255, //  0
    255, 255, 255, 255, 255, 255, 255, 255, //  8
    255, 255, 255, 255, 255, 255, 255, 255, //  16
    255, 255, 255, 255, 255, 255, 255, 255, //  24
    255, 255, 255, 255, 255, 255, 255, 255, //  32
    255, 255, 255,  62, 255, 255, 255,  63, //  40
     52,  53,  54,  55,  56,  57,  58,  59, //  48
     60,  61, 255, 255, 255, 200, 255, 255, //  56   '=' is 200, on index 61
    255,   0,   1,   2,   3,   4,   5,   6, //  64
      7,   8,   9,  10,  11,  12,  13,  14, //  72
     15,  16,  17,  18,  19,  20,  21,  22, //  80
     23,  24,  25, 255, 255, 255, 255, 255, //  88
    255,  26,  27,  28,  29,  30,  31,  32, //  96
     33,  34,  35,  36,  37,  38,  39,  40, //  104
     41,  42,  43,  44,  45,  46,  47,  48, //  112
     49,  50,  51, 255, 255, 255, 255, 255, //  120
  };
  return tab[ch & 127];
}

V7_PRIVATE void base64_decode(const unsigned char *s, int len, char *dst) {
  unsigned char a, b, c, d;
  while (len >= 4 &&
         (a = from_b64(s[0])) != 255 &&
         (b = from_b64(s[1])) != 255 &&
         (c = from_b64(s[2])) != 255 &&
         (d = from_b64(s[3])) != 255) {
    if (a == 200 || b == 200) break;  // '=' can't be there
    *dst++ = a << 2 | b >> 4;
    if (c == 200) break;
    *dst++ = b << 4 | c >> 2;
    if (d == 200) break;
    *dst++ = c << 6 | d;
    s += 4;
    len -=4;
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
      (n = fread(buf, 1, sizeof(buf), (FILE *) (unsigned long) v->v.num)) > 0) {
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
    for (i = 0; (int) i < cfa->num_args; i++) {
      if (is_string(cfa->args[i]) &&
          (n = fwrite(cfa->args[i]->v.str.buf, 1, cfa->args[i]->v.str.len,
                      (FILE *) (unsigned long) v->v.num)) > 0) {
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
      fclose((FILE *) (unsigned long) v->v.num) == 0) {
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
    v7_setv(cfa->v7, result, V7_TYPE_STR, V7_TYPE_NUM,
            "fp", 2, 0, (double) (unsigned long) fp);  // after v7_set_class !
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

#define EXPECT(v7, t) \
  do {if ((v7)->cur_tok != (t)) return V7_SYNTAX_ERROR; next_tok(v7);} while (0)

static enum v7_err arith(struct v7 *v7, struct v7_val *a, struct v7_val *b,
                         struct v7_val *res, enum v7_tok op) {
  char *str;

  if (a->type == V7_TYPE_STR && op == TOK_PLUS) {
    if (b->type != V7_TYPE_STR) {
      // Do type conversion, result pushed on stack
      TRY(toString(v7, b));
      b = v7_top_val(v7);
    }
    str = (char *) malloc(a->v.str.len + b->v.str.len + 1);
    CHECK(str != NULL, V7_OUT_OF_MEMORY);
    v7_init_str(res, str, a->v.str.len + b->v.str.len, 0);
    memcpy(str, a->v.str.buf, a->v.str.len);
    memcpy(str + a->v.str.len, b->v.str.buf, b->v.str.len);
    str[res->v.str.len] = '\0';
    return V7_OK;
  } else if (a->type == V7_TYPE_NUM && b->type == V7_TYPE_NUM) {
    v7_init_num(res, res->v.num);
    switch (op) {
      case TOK_PLUS: res->v.num = a->v.num + b->v.num; break;
      case TOK_MINUS: res->v.num = a->v.num - b->v.num; break;
      case TOK_MUL: res->v.num = a->v.num * b->v.num; break;
      case TOK_DIV: res->v.num = a->v.num / b->v.num; break;
      case TOK_REM: res->v.num = (unsigned long) a->v.num %
        (unsigned long) b->v.num; break;
      case TOK_XOR: res->v.num = (unsigned long) a->v.num ^
        (unsigned long) b->v.num; break;
      default: return V7_INTERNAL_ERROR;
    }
    return V7_OK;
  } else {
    return V7_TYPE_ERROR;
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

  // arith() might push another value on stack if type conversion was made.
  // if that happens, re-push the result again
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
    EXPECT(v7, TOK_CLOSE_CURLY);
  } else {
    TRY(parse_statement(v7, has_return));
  }
  return V7_OK;
}

static enum v7_err parse_function_definition(struct v7 *v7, struct v7_val **v,
                                             int num_params) { // <#fdef#>
  int i = 0, old_flags = v7->flags, old_sp = v7->sp, has_ret = 0,
    line_no = v7->pstate.line_no;
  unsigned long func_name_len = 0;
  const char *src = v7->pstate.pc, *func_name = NULL;
  struct v7_val *ctx = NULL, *f = NULL;

  EXPECT(v7, TOK_FUNCTION);
  if (v7->cur_tok == TOK_IDENTIFIER) {
    // function name is given, e.g. function foo() {}
    CHECK(v == NULL, V7_SYNTAX_ERROR);
    func_name = v7->tok;
    func_name_len = v7->tok_len;
    src = v7->pstate.pc;
    line_no = v7->pstate.line_no;
    next_tok(v7);
  }

  // 1. SCANNING: do nothing, just pass through the function code
  // 2. EXECUTING && v == 0: don't execute but create a closure
  // 3. EXECUTING && v != 0: execute the closure

  if (EXECUTING(v7->flags) && v == NULL) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    f = v7_top_val(v7);
    v7_set_class(f, V7_CLASS_FUNCTION);
    f->flags |= V7_JS_FUNC;

    f->v.func.source_code = (char *) src;
    f->v.func.line_no = line_no;
    //printf("PFD [%.*s]\n", 45, f->v.func.source_code);

    f->v.func.var_obj = v7->ctx;
    inc_ref_count(v7->ctx);

    v7->flags |= V7_NO_EXEC;
  } else if (EXECUTING(v7->flags) && v != NULL) {
    f = v[0];
    assert(v7_is_class(f, V7_CLASS_FUNCTION));

    f->next = v7->cf;
    v7->cf = f;

    ctx = make_value(v7, V7_TYPE_OBJ);
    v7_set_class(ctx, V7_CLASS_OBJECT);
    inc_ref_count(ctx);

    ctx->next = v7->ctx;
    v7->ctx = ctx;
  }

  // Add function arguments to the variable object
  EXPECT(v7, TOK_OPEN_PAREN);
  while (v7->cur_tok != TOK_CLOSE_PAREN) {
    const char *key = v7->tok;
    unsigned long key_len = v7->tok_len;
    EXPECT(v7, TOK_IDENTIFIER);
    if (EXECUTING(v7->flags)) {
      struct v7_val *val = i < num_params ? v[i + 1] : make_value(v7, V7_TYPE_UNDEF);
      TRY(v7_setv(v7, ctx, V7_TYPE_STR, V7_TYPE_OBJ, key, key_len, 1, val));
    }
    i++;
    if (v7->cur_tok == TOK_COMMA) {
      next_tok(v7);
    }
  }
  EXPECT(v7, TOK_CLOSE_PAREN);

  // Execute (or pass) function body
  TRY(parse_compound_statement(v7, &has_ret));

  // Add function to the namespace for notation "function x(y,z) { ... } "
  if (EXECUTING(old_flags) && v == NULL && func_name != NULL) {
    TRY(v7_setv(v7, v7->ctx, V7_TYPE_STR, V7_TYPE_OBJ,
                func_name, func_name_len, 1, f));
  }

  if (EXECUTING(v7->flags)) {
    // Cleanup execution context
    v7->ctx = ctx->next;
    ctx->next = NULL;
    //assert(f->v.func.var_obj == NULL);
    //f->v.func.var_obj = ctx;
    v7_freeval(v7, ctx);

    v7->cf = f->next;
    f->next = NULL;

    // If function didn't have return statement, return UNDEF
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


  // Stack looks as follows:
  //  v   --->  <called_function>     v[0]
  //            <argument_0>        ---+
  //            <argument_1>           |
  //            <argument_2>           |  <-- num_args
  //            ...                    |
  //            <argument_N>        ---+
  // top  --->  <return_value>
  if (f->flags & V7_JS_FUNC) {
    struct v7_pstate old_pstate = v7->pstate;
    enum v7_tok tok = v7->cur_tok;

    // Move control flow to the function body
    v7->pstate.pc = f->v.func.source_code;
    v7->pstate.line_no = f->v.func.line_no;
    v7->cur_tok = TOK_FUNCTION;
    TRY(parse_function_definition(v7, v, num_args));

    // Return control flow back
    v7->pstate = old_pstate;
    v7->cur_tok = tok;
    CHECK(v7_top(v7) >= top, V7_INTERNAL_ERROR);
  } else {
    int old_sp = v7->sp;
    struct v7_c_func_arg arg = {v7, this_obj, v + 1, num_args, called_as_ctor};
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

  //TRACE_OBJ(v[0]);
  CHECK(!EXECUTING(v7->flags) || v7_is_class(v[0], V7_CLASS_FUNCTION),
        V7_CALLED_NON_FUNCTION);

  // Push arguments on stack
  EXPECT(v7, TOK_OPEN_PAREN);
  while (v7->cur_tok != TOK_CLOSE_PAREN) {
    TRY(parse_expression(v7));
    if (v7->cur_tok == TOK_COMMA) {
      next_tok(v7);
    }
    num_args++;
  }
  EXPECT(v7, TOK_CLOSE_PAREN);

  TRY(v7_call2(v7, this_obj, num_args, called_as_ctor));

  return V7_OK;
}

static enum v7_err push_string_literal(struct v7 *v7) {
  // TODO(lsm): do not use stack buffer here, only dynamic alloc
  //char buf[MAX_STRING_LITERAL_LENGTH];
  struct v7_val *v;
  char *p;
  size_t i;

  if (!EXECUTING(v7->flags)) return V7_OK;
  TRY(v7_make_and_push(v7, V7_TYPE_STR));
  v = v7_top_val(v7);
  v7_init_str(v, (char *) malloc(v7->tok_len - 1), 0, 1);
  CHECK(v->v.str.buf != NULL, V7_OUT_OF_MEMORY);
  p = v->v.str.buf;

  // Scan string literal into the buffer, handle escape sequences
  for (i = 1; i < v7->tok_len - 1; i++) {
    switch (v7->tok[i]) {
      case '\\':
        i++;
        switch (v7->tok[i]) {
          // TODO: add escapes for quotes, \XXX, \xXX, \uXXXX
          case 'b': *p++ = '\b'; break;
          case 'f': *p++ = '\f'; break;
          case 'n': *p++ = '\n'; break;
          case 'r': *p++ = '\r'; break;
          case 't': *p++ = '\t'; break;
          case 'v': *p++ = '\v'; break;
          case '\\': *p++ = '\\'; break;
          default: if (v7->tok[i] == v7->tok[0]) *p++ = v7->tok[i]; break;
        }
        break;
      default:
        *p++ = v7->tok[i];
        break;
    }
  }
  v->v.str.len = p - v->v.str.buf;
  assert(v->v.str.len < v7->tok_len - 1);
  *p = '\0';

  return V7_OK;
}

static enum v7_err parse_array_literal(struct v7 *v7) {
  // Push empty array on stack
  if (EXECUTING(v7->flags)) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    v7_set_class(v7_top(v7)[-1], V7_CLASS_ARRAY);
  }
  CHECK(v7->cur_tok == TOK_OPEN_BRACKET, V7_SYNTAX_ERROR);
  next_tok(v7);

  // Scan array literal, append elements one by one
  while (v7->cur_tok != TOK_CLOSE_BRACKET) {
    // Push new element on stack
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
  // Push empty object on stack
  TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
  EXPECT(v7, TOK_OPEN_CURLY);

  // Assign key/values to the object, until closing "}" is found
  while (v7->cur_tok != TOK_CLOSE_CURLY) {
    // Push key on stack
    if (v7->cur_tok == TOK_STRING_LITERAL) {
      TRY(push_string_literal(v7));
    } else {
      struct v7_val *v;
      CHECK(v7->cur_tok == TOK_IDENTIFIER, V7_SYNTAX_ERROR);
      v = v7_mkv(v7, V7_TYPE_STR, v7->tok, v7->tok_len, 1);
      CHECK(v != NULL, V7_OUT_OF_MEMORY);
      TRY(v7_push(v7, v));
    }

    // Push value on stack
    next_tok(v7);
    EXPECT(v7, TOK_COLON);
    TRY(parse_expression(v7));

    // Stack should now have object, key, value. Assign, and remove key/value
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
  EXPECT(v7, TOK_DELETE);
  TRY(parse_expression(v7));
  TRY(v7_del2(v7, v7->cur_obj, v7->key, v7->key_len));
  return V7_OK;
}

static enum v7_err parse_regex(struct v7 *v7) {
  char regex[MAX_STRING_LITERAL_LENGTH];
  size_t i;

  CHECK(*v7->pstate.pc == '/', V7_SYNTAX_ERROR);
  for (i = 0, v7->pstate.pc++; i < sizeof(regex) - 1 && *v7->pstate.pc != '/' &&
    *v7->pstate.pc != '\0'; i++, v7->pstate.pc++) {
    if (*v7->pstate.pc == '\\' && v7->pstate.pc[1] == '/') v7->pstate.pc++;
    regex[i] = *v7->pstate.pc;
  }
  regex[i] = '\0';
  EXPECT(v7, TOK_DIV);
  if (EXECUTING(v7->flags)) {
    TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
    v7_set_class(v7_top(v7)[-1], V7_CLASS_REGEXP);
    v7_top(v7)[-1]->v.regex = v7_strdup(regex, strlen(regex));
  }

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
    case TOK_OPEN_BRACKET: TRY(parse_array_literal(v7)); break;
    case TOK_OPEN_CURLY: TRY(parse_object_literal(v7)); break;
    case TOK_DIV: TRY(parse_regex(v7)); break;
    case TOK_STRING_LITERAL: TRY(push_string_literal(v7)); break;
    case TOK_THIS: if (ex) TRY(v7_push(v7, v7->this_obj)); break;
    case TOK_NULL: if (ex) TRY(v7_make_and_push(v7, V7_TYPE_NULL)); break;
    case TOK_UNDEFINED: if (ex) TRY(v7_make_and_push(v7, V7_TYPE_UNDEF)); break;
    case TOK_TRUE: if (ex) TRY(push_bool(v7, 1)); break;
    case TOK_FALSE: if (ex) TRY(push_bool(v7, 0)); break;
#if 0
    case TOK_NAN: if (ex) TRY(push_number(v7, NAN)); break;
    case TOK_INFINITY: if (ex) TRY(push_number(v7, INFINITY)); break;
#endif
    case TOK_NUMBER: if (ex) TRY(push_number(v7, v7->cur_tok_dbl)); break;
    case TOK_IDENTIFIER: if (ex) TRY(parse_variable(v7)); break;
    case TOK_FUNCTION: TRY(parse_function_definition(v7, NULL, 0)); break;
    default: return V7_SYNTAX_ERROR;
  }

  if (tok != TOK_FUNCTION) {
    next_tok(v7);
  }

  return V7_OK;
}

static enum v7_err parse_prop_accessor(struct v7 *v7, enum v7_tok op) {
  struct v7_val *v = NULL, *ns = NULL, *cur_obj = NULL;

  if (EXECUTING(v7->flags)) {
    ns = v7_top(v7)[-1];
    v7_make_and_push(v7, V7_TYPE_UNDEF);
    v = v7_top(v7)[-1];
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
      if (ns != NULL && (ns->flags & V7_PROP_FUNC)) {
        ns->v.prop_func(v7->cur_obj, v);
        ns = v;
      }
    }
    next_tok(v7);
  } else {
    TRY(parse_expression(v7));
    EXPECT(v7, TOK_CLOSE_BRACKET);
    if (EXECUTING(v7->flags)) {
      struct v7_val *expr_val = v7_top_val(v7);

      ns = get2(ns, expr_val);
      if (ns != NULL && (ns->flags & V7_PROP_FUNC)) {
        ns->v.prop_func(v7->cur_obj, v);
        ns = v;
      }

      // If we're doing an assignment,
      // then parse_assign() looks at v7->key, v7->key_len for the key.
      // Initialize key properly for cases like "a.b['c'] = d;"
      if (expr_val->type != V7_TYPE_STR) {
        TRY(toString(v7, expr_val));
        expr_val = v7_top_val(v7);
      }
      v7->key = expr_val->v.str.buf;
      v7->key_len = expr_val->v.str.len;
    }
  }

  // Set those again cause parse_expression() above could have changed it
  v7->cur_obj = v7->this_obj = cur_obj;

  if (EXECUTING(v7->flags)) {
    TRY(v7_push(v7, ns == NULL ? v : ns));
  }

  return V7_OK;
}

// Member Access            left-to-right    x . x
// Computed Member Access   left-to-right    x [ x ]
// new (with argument list) n/a              new x ( x )
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

// x . y () . z () ()

// Function Call                 left-to-right     x ( x )
// new (without argument list)   right-to-left     new x
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
    // Use cur_this, not v7->this_obj: v7->this_obj could have been changed
    TRY(parse_function_call(v7, cur_this, has_new));
  }
#endif
  if (has_new && EXECUTING(v7->flags)) {
    TRY(v7_push(v7, cur_this));
  }

  v7->this_obj = old_this_obj;

  return V7_OK;
}

// Postfix Increment    n/a      x ++
// Postfix Decrement    n/a      x --
static enum v7_err parse_postfix_inc_dec(struct v7 *v7) {
  TRY(parse_precedence_2(v7));
  if (v7->cur_tok == TOK_PLUS_PLUS || v7->cur_tok == TOK_MINUS_MINUS) {
    int increment = (v7->cur_tok == TOK_PLUS_PLUS) ? 1 : -1;
    next_tok(v7);
    if (EXECUTING(v7->flags)) {
      struct v7_val *v = v7_top(v7)[-1];
      CHECK(v->type == V7_TYPE_NUM, V7_TYPE_ERROR);
      v->v.num += increment;
    }
  }
  return V7_OK;
}

// Logical NOT        right-to-left    ! x
// Bitwise NOT        right-to-left    ~ x
// Unary Plus         right-to-left    + x
// Unary Negation     right-to-left    - x
// Prefix Increment   right-to-left    ++ x
// Prefix Decrement   right-to-left    -- x
// typeof             right-to-left    typeof x
// void               right-to-left    void x
// delete             right-to-left    delete x
static enum v7_err parse_unary(struct v7 *v7) {
  static const char *type_names[] = {
    "undefined", "object", "boolean", "string", "number", "object"
  };
  const char *str;
  enum v7_tok unary = TOK_END_OF_INPUT;

  switch (v7->cur_tok) {
    case TOK_NOT: case TOK_TILDA: case TOK_PLUS: case TOK_MINUS:
    case TOK_PLUS_PLUS: case TOK_MINUS_MINUS: case TOK_TYPEOF:
    case TOK_VOID: case TOK_DELETE:
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
    switch (unary) {
      case TOK_PLUS:
        CHECK(is_num(result), V7_TYPE_ERROR);
        break;
      case TOK_MINUS:
        CHECK(is_num(result), V7_TYPE_ERROR);
        TRY(push_number(v7, - result->v.num));
        break;
      case TOK_NOT:
        TRY(push_bool(v7, !v7_is_true(result)));
        break;
      case TOK_TYPEOF:
        str = type_names[result->type];
        if (v7_is_class(result, V7_CLASS_FUNCTION)) str = "function";
        TRY(push_string(v7, str, strlen(str), 0));
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

  if (v1->type == V7_TYPE_NUM && v2->type == V7_TYPE_NUM) {
    switch (op) {
      case TOK_GT: res = v1->v.num >  v2->v.num; break;
      case TOK_GE: res = v1->v.num >= v2->v.num; break;
      case TOK_LT: res = v1->v.num <  v2->v.num; break;
      case TOK_LE: res = v1->v.num <= v2->v.num; break;
      case TOK_EQ: // FALLTHROUGH
      case TOK_EQ_EQ:    res = cmp(v1, v2) == 0; break;
      case TOK_NE: // FALLTHROUGH
      case TOK_NE_NE:  res = cmp(v1, v2) != 0; break;
      default: return V7_INTERNAL_ERROR;
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
      unsigned long a = v1->v.num, b = v2->v.num;
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
      unsigned long a = v1->v.num, b = v2->v.num;
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
      unsigned long a = v1->v.num, b = v2->v.num;
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
      TRY(inc_stack(v7, -1));   // Remove condition result
    }

    EXPECT(v7, TOK_QUESTION);
    if (!condition_true || !EXECUTING(old_flags)) v7->flags |= V7_NO_EXEC;
    TRY(parse_expression(v7));
    EXPECT(v7, TOK_COLON);
    v7->flags = old_flags;
    if (condition_true || !EXECUTING(old_flags)) v7->flags |= V7_NO_EXEC;
    TRY(parse_expression(v7));
    v7->flags = old_flags;
  }

  return V7_OK;
}

static enum v7_err do_assign(struct v7 *v7, struct v7_val *obj,
                             const char *key, unsigned long key_len,
                             enum v7_tok tok) {
  if (EXECUTING(v7->flags)) {
    struct v7_val **top = v7_top(v7), *a = top[-2], *b = top[-1];

    // Stack layout at this point (assuming stack grows down):
    //
    //          | object's value (rvalue)    |    top[-2]
    //          +----------------------------+
    //          | expression value (lvalue)  |    top[-1]
    //          +----------------------------+
    // top -->  |       nothing yet          |

    switch (tok) {
      case TOK_ASSIGN:
        CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
        TRY(v7_setv(v7, obj, V7_TYPE_STR, V7_TYPE_OBJ, key, key_len, 1, b));
        return V7_OK;
      case TOK_PLUS_ASSIGN: TRY(arith(v7, a, b, a, TOK_PLUS)); break;
      case TOK_MINUS_ASSIGN: TRY(arith(v7, a, b, a, TOK_MINUS)); break;
      case TOK_MUL_ASSIGN: TRY(arith(v7, a, b, a, TOK_MUL)); break;
      case TOK_DIV_ASSIGN: TRY(arith(v7, a, b, a, TOK_DIV)); break;
      case TOK_REM_ASSIGN: TRY(arith(v7, a, b, a, TOK_REM)); break;
      case TOK_XOR_ASSIGN: TRY(arith(v7, a, b, a, TOK_XOR)); break;
      default: return V7_NOT_IMPLEMENTED;
    }
  }
  return V7_OK;
}

V7_PRIVATE enum v7_err parse_expression(struct v7 *v7) {
  int old_sp = v7->sp;

  // Set up the reference to be the current execution context
  v7->cur_obj = v7->ctx;
  v7->key = NULL;

  // TODO(lsm): parse_yield() should be here, do we want to implement it?
  TRY(parse_ternary(v7));

  // Parse assignment.
  if (v7->cur_tok >= TOK_ASSIGN && v7->cur_tok <= TOK_LOGICAL_OR_ASSING) {
    // Remember current reference
    const char *key = v7->key;
    unsigned long key_len = v7->key_len;
    struct v7_val *cur_obj = v7->cur_obj;
    enum v7_tok op = v7->cur_tok;

    next_tok(v7);

    // This recursion implements right-to-left association for assignment
    TRY(parse_expression(v7));
    if (EXECUTING(v7->flags)) {
      TRY(do_assign(v7, cur_obj, key, key_len, op));
    }
  }

  // Collapse stack, leave only one value on top
  if (EXECUTING(v7->flags)) {
    struct v7_val *result = v7_top(v7)[-1];
    inc_ref_count(result);
    TRY(inc_stack(v7, old_sp - v7->sp));
    TRY(v7_push(v7, result));
    assert(result->ref_count > 1);
    v7_freeval(v7, result);
  } else {
    TRY(inc_stack(v7, old_sp - v7->sp));
  }

  return V7_OK;
}

static enum v7_err parse_declaration(struct v7 *v7) { // <#parse_decl#>
  int old_sp = v7_sp(v7);

  EXPECT(v7, TOK_VAR);
  do {
    const char *key = v7->tok;
    unsigned long key_len = v7->tok_len;

    inc_stack(v7, old_sp - v7_sp(v7));
    EXPECT(v7, TOK_IDENTIFIER);
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

  EXPECT(v7, TOK_IF);
  EXPECT(v7, TOK_OPEN_PAREN);
  TRY(parse_expression(v7));      // Evaluate condition, pushed on stack
  EXPECT(v7, TOK_CLOSE_PAREN);

  if (EXECUTING(old_flags)) {
    // If condition is false, do not execute "if" body
    CHECK(v7->sp > 0, V7_INTERNAL_ERROR);
    condition_true = v7_is_true(v7_top_val(v7));
    if (!condition_true) v7->flags |= V7_NO_EXEC;
    TRY(inc_stack(v7, -1));   // Cleanup condition result from the stack
  }
  TRY(parse_compound_statement(v7, has_return));

  if (v7->cur_tok == TOK_ELSE) {
    next_tok(v7);
    v7->flags = old_flags;
    if (!EXECUTING(old_flags) || condition_true) v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
  }

  v7->flags = old_flags;  // Restore old execution flags
  return V7_OK;
}

static enum v7_err parse_for_in_statement(struct v7 *v7, int has_var,
                                          int *has_return) {
  const char *tok = v7->tok;
  unsigned long tok_len = v7->tok_len;
  struct v7_pstate s_block;

  EXPECT(v7, TOK_IDENTIFIER);
  EXPECT(v7, TOK_IN);
  TRY(parse_expression(v7));
  EXPECT(v7, TOK_CLOSE_PAREN);
  s_block = v7->pstate;

  // Execute loop body
  if (!EXECUTING(v7->flags)) {
    TRY(parse_compound_statement(v7, has_return));
  } else {
    int old_sp = v7->sp;
    struct v7_val *obj = v7_top(v7)[-1];
    struct v7_val *scope = has_var ? v7->ctx : &v7->root_scope;
    struct v7_prop *prop;

    CHECK(obj->type == V7_TYPE_OBJ, V7_TYPE_ERROR);
    for (prop = obj->props; prop != NULL; prop = prop->next) {
      TRY(v7_setv(v7, scope, V7_TYPE_STR, V7_TYPE_OBJ,
                  tok, tok_len, 1, prop->key));
      v7->pstate = s_block;
      TRY(parse_compound_statement(v7, has_return));  // Loop body
      TRY(inc_stack(v7, old_sp - v7->sp));  // Clean up stack
    }
  }

  return V7_OK;
}

static enum v7_err parse_for_statement(struct v7 *v7, int *has_return) {
  int is_true, old_flags = v7->flags, has_var = 0;
  struct v7_pstate s2, s3, s_block, s_end;

  EXPECT(v7, TOK_FOR);
  EXPECT(v7, TOK_OPEN_PAREN);

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

  EXPECT(v7, TOK_SEMICOLON);

  // Pass through the loop, don't execute it, just remember locations
  v7->flags |= V7_NO_EXEC;
  s2 = v7->pstate;
  TRY(parse_expression(v7));    // expr2 (condition)
  EXPECT(v7, TOK_SEMICOLON);

  s3 = v7->pstate;
  TRY(parse_expression(v7));    // expr3  (post-iteration)
  EXPECT(v7, TOK_CLOSE_PAREN);

  s_block = v7->pstate;
  TRY(parse_compound_statement(v7, has_return));
  s_end = v7->pstate;

  v7->flags = old_flags;

  // Execute loop
  if (EXECUTING(v7->flags)) {
    int old_sp = v7->sp;
    for (;;) {
      v7->pstate = s2;
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_expression(v7));    // Evaluate condition
      assert(v7->sp > old_sp);
      is_true = !v7_is_true(v7_top(v7)[-1]);
      if (is_true) break;

      v7->pstate = s_block;
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_compound_statement(v7, has_return));  // Loop body
      assert(!EXECUTING(v7->flags) == 0);

      v7->pstate = s3;
      TRY(parse_expression(v7));    // expr3  (post-iteration)

      TRY(inc_stack(v7, old_sp - v7->sp));  // Clean up stack
    }
  }

  // Jump to the code after the loop
  v7->pstate = s_end;

  return V7_OK;
}

static enum v7_err parse_while_statement(struct v7 *v7, int *has_return) {
  int is_true, old_flags = v7->flags;
  struct v7_pstate s_cond, s_block, s_end;

  EXPECT(v7, TOK_OPEN_PAREN);
  s_cond = v7->pstate;
  v7->flags |= V7_NO_EXEC;
  TRY(parse_expression(v7));
  EXPECT(v7, TOK_CLOSE_PAREN);

  s_block = v7->pstate;
  TRY(parse_compound_statement(v7, has_return));
  s_end = v7->pstate;

  v7->flags = old_flags;

  // Execute loop
  if (EXECUTING(v7->flags)) {
    int old_sp = v7->sp;
    for (;;) {
      v7->pstate = s_cond;
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_expression(v7));    // Evaluate condition
      assert(v7->sp > old_sp);
      is_true = !v7_is_true(v7_top_val(v7));
      if (is_true) break;

      v7->pstate = s_block;
      assert(!EXECUTING(v7->flags) == 0);
      TRY(parse_compound_statement(v7, has_return));  // Loop body
      assert(!EXECUTING(v7->flags) == 0);

      TRY(inc_stack(v7, old_sp - v7->sp));  // Clean up stack
    }
  }

  // Jump to the code after the loop
  v7->pstate = s_end;

  return V7_OK;
}

static enum v7_err parse_return_statement(struct v7 *v7, int *has_return) {
  if (EXECUTING(v7->flags)) {
    *has_return = 1;
  }
  EXPECT(v7, TOK_RETURN);
  if (v7->cur_tok != TOK_SEMICOLON && v7->cur_tok != TOK_CLOSE_CURLY) {
    TRY(parse_expression(v7));
  }
  return V7_OK;
}

static enum v7_err parse_try_statement(struct v7 *v7, int *has_return) {
  enum v7_err err_code;
  const char *old_pc = v7->pstate.pc;
  int old_flags = v7->flags, old_line_no = v7->pstate.line_no;

  EXPECT(v7, TOK_TRY);
  CHECK(v7->cur_tok == TOK_OPEN_CURLY, V7_SYNTAX_ERROR);
  err_code = parse_compound_statement(v7, has_return);

  if (!EXECUTING(old_flags) && err_code != V7_OK) {
    return err_code;
  }

  // If exception has happened, skip the block
  if (err_code != V7_OK) {
    v7->pstate.pc = old_pc;
    v7->pstate.line_no = old_line_no;
    v7->flags |= V7_NO_EXEC;
    TRY(parse_compound_statement(v7, has_return));
  }

  // Process catch/finally blocks
  CHECK(next_tok(v7) == TOK_IDENTIFIER, V7_SYNTAX_ERROR);

  //if (test_token(v7, "catch", 5)) {
  if (v7->cur_tok == TOK_CATCH) {
    const char *key;
    unsigned long key_len;

    EXPECT(v7, TOK_CATCH);
    EXPECT(v7, TOK_OPEN_PAREN);
    key = v7->tok;
    key_len = v7->tok_len;
    EXPECT(v7, TOK_IDENTIFIER);
    EXPECT(v7, TOK_CLOSE_PAREN);

    // Insert error variable into the namespace
    if (err_code != V7_OK) {
      TRY(v7_make_and_push(v7, V7_TYPE_OBJ));
      v7_set_class(v7_top_val(v7), V7_CLASS_ERROR);
      v7_setv(v7, v7->ctx, V7_TYPE_STR, V7_TYPE_OBJ,
              key, key_len, 1, v7_top_val(v7));
    }

    // If there was no exception, do not execute catch block
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
    case TOK_VAR:     TRY(parse_declaration(v7));                   break;
    case TOK_RETURN:  TRY(parse_return_statement(v7, has_return));  break;
    case TOK_IF:      TRY(parse_if_statement(v7, has_return));      break;
    case TOK_FOR:     TRY(parse_for_statement(v7, has_return));     break;
    case TOK_WHILE:   TRY(parse_while_statement(v7, has_return));   break;
    case TOK_TRY:     TRY(parse_try_statement(v7, has_return));     break;
    case TOK_DELETE:  TRY(parse_delete_statement(v7));              break;
    default:          TRY(parse_expression(v7));                    break;
  }

  // Skip optional colons and semicolons.
  // TODO(lsm): follow automatic semicolon insertion rules
  while (v7->cur_tok == TOK_COMMA || v7->cur_tok == TOK_SEMICOLON) {
    next_tok(v7);
  }

  return V7_OK;
}

// NOTE(lsm): Must be in the same order as enum for keywords
struct { const char *p; int len; } s_keywords[] = {
  {"break", 5}, {"case", 4}, {"catch", 5}, {"continue", 8}, {"debugger", 8},
  {"default", 7}, {"delete", 6}, {"do", 2}, {"else", 4}, {"false", 5},
  {"finally", 7}, {"for", 3}, {"function", 8}, {"if", 2}, {"in", 2},
  {"instanceof", 10}, {"new", 3}, {"null", 4}, {"return", 6}, {"switch", 6},
  {"this", 4}, {"throw", 5}, {"true", 4}, {"try", 3}, {"typeof", 6},
  {"undefined", 9}, {"var", 3}, {"void", 4}, {"while", 5}, {"with", 4}
};

// Move ptr to the next token, skipping comments and whitespaces.
// Return number of new line characters detected.
static int skip_to_next_tok(const char **ptr) {
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

// TODO(lsm): use lookup table to speed it up
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
        memcmp(s_keywords[(tok - TOK_BREAK) + i].p + 1, s + 1,
               len - 1) == 0) break;
  }

  return i == ntoks ? TOK_IDENTIFIER :(enum v7_tok) (tok + i);
}

static enum v7_tok punct1(const char **s, int ch1,
                          enum v7_tok tok1, enum v7_tok tok2) {

  (*s)++;
  if (s[0][0] == ch1) {
    (*s)++; return tok1;
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

  // Scan string literal into the buffer, handle escape sequences
  while (*s != quote && *s != '\0') {
    switch (*s) {
      case '\\':
        s++;
        switch (*s) {
          case 'b': case 'f': case 'n': case 'r': case 't':
          case 'v': case '\\': s++; break;
          default: if (*s == quote) s++; break;
        }
        break;
      default: break;
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

static enum v7_tok get_tok(const char **s, double *n) {
  const char *p = *s;

  switch (*p) {
    // Letters
    case 'a': ident(s); return TOK_IDENTIFIER;
    case 'b': ident(s); return kw(p, *s - p, 1, TOK_BREAK);
    case 'c': ident(s); return kw(p, *s - p, 2, TOK_CASE);
    case 'd': ident(s); return kw(p, *s - p, 4, TOK_DEBUGGER);
    case 'e': ident(s); return kw(p, *s - p, 1, TOK_ELSE);
    case 'f': ident(s); return kw(p, *s - p, 4, TOK_FALSE);
    case 'g':
    case 'h': ident(s); return TOK_IDENTIFIER;
    case 'i': ident(s); return kw(p, *s - p, 3, TOK_IF);
    case 'j':
    case 'k':
    case 'l':
    case 'm': ident(s); return TOK_IDENTIFIER;
    case 'n': ident(s); return kw(p, *s - p, 2, TOK_NEW);
    case 'o':
    case 'p':
    case 'q': ident(s); return TOK_IDENTIFIER;
    case 'r': ident(s); return kw(p, *s - p, 1, TOK_RETURN);
    case 's': ident(s); return kw(p, *s - p, 1, TOK_SWITCH);
    case 't': ident(s); return kw(p, *s - p, 5, TOK_THIS);
    case 'u': ident(s); return kw(p, *s - p, 1, TOK_UNDEFINED);
    case 'v': ident(s); return kw(p, *s - p, 2, TOK_VAR);
    case 'w': ident(s); return kw(p, *s - p, 2, TOK_WHILE);
    case 'x':
    case 'y':
    case 'z': ident(s); return TOK_IDENTIFIER;

    case '_': case '$':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y':
    case 'Z': ident(s); return TOK_IDENTIFIER;

    // Numbers
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8':
    case '9': parse_number(p, s, n); return TOK_NUMBER;

    // String literals
    case '\'':
    case '"': return parse_str_literal(s);

    // Punctuators
    case '=': return punct2(s, '=', TOK_EQ, '=', TOK_EQ_EQ, TOK_ASSIGN);
    case '!': return punct2(s, '=', TOK_NE, '=', TOK_NE_NE, TOK_NOT);

    case '%': return punct1(s, '=', TOK_REM_ASSIGN, TOK_REM);
    case '*': return punct1(s, '=', TOK_MUL_ASSIGN, TOK_MUL);
    case '/': return punct1(s, '=', TOK_DIV_ASSIGN, TOK_DIV);
    case '^': return punct1(s, '=', TOK_XOR_ASSIGN, TOK_XOR);

    case '+': return punct3(s, '+', TOK_PLUS_PLUS, '=',
                            TOK_PLUS_ASSIGN, TOK_PLUS);
    case '-': return punct3(s, '-', TOK_MINUS_MINUS, '=',
                            TOK_MINUS_ASSIGN, TOK_MINUS);
    case '&': return punct3(s, '&', TOK_LOGICAL_AND, '=',
                            TOK_LOGICAL_AND_ASSING, TOK_AND);
    case '|': return punct3(s, '|', TOK_LOGICAL_OR, '=',
                            TOK_LOGICAL_OR_ASSING, TOK_OR);

    case '<':
      if (s[0][1] == '=') { (*s) += 2; return TOK_LE; }
      return punct2(s, '<', TOK_LSHIFT, '=', TOK_LSHIFT_ASSIGN, TOK_LT);
    case '>':
      if (s[0][1] == '=') { (*s) += 2; return TOK_GE; }
      return punct2(s, '<', TOK_RSHIFT, '=', TOK_RSHIFT_ASSIGN, TOK_GT);

    case '{': (*s)++; return TOK_OPEN_CURLY;
    case '}': (*s)++; return TOK_CLOSE_CURLY;
    case '(': (*s)++; return TOK_OPEN_PAREN;
    case ')': (*s)++; return TOK_CLOSE_PAREN;
    case '[': (*s)++; return TOK_OPEN_BRACKET;
    case ']': (*s)++; return TOK_CLOSE_BRACKET;
    case '.': (*s)++; return TOK_DOT;
    case ';': (*s)++; return TOK_SEMICOLON;
    case ':': (*s)++; return TOK_COLON;
    case '?': (*s)++; return TOK_QUESTION;
    case '~': (*s)++; return TOK_TILDA;
    case ',': (*s)++; return TOK_COMMA;

    default: return TOK_END_OF_INPUT;
  }
}

V7_PRIVATE enum v7_tok lookahead(const struct v7 *v7) {
  const char *s = v7->pstate.pc;
  double d;
  return get_tok(&s, &d);
}

V7_PRIVATE enum v7_tok next_tok(struct v7 *v7) {
  v7->pstate.line_no += skip_to_next_tok(&v7->pstate.pc);
  v7->tok = v7->pstate.pc;
  v7->cur_tok = get_tok(&v7->pstate.pc, &v7->cur_tok_dbl);
  v7->tok_len = v7->pstate.pc - v7->tok;
  v7->pstate.line_no += skip_to_next_tok(&v7->pstate.pc);
  TRACE_CALL("==tok=> %d [%.*s] %d\n", v7->cur_tok, (int) v7->tok_len, v7->tok,
             v7->pstate.line_no);
  return v7->cur_tok;
}

#ifdef TEST_RUN
int main(void) {
  const char *src = "for (var fo++ = -1; /= <= 1.17; x<<) { == <<=, 'x')} "
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

struct v7 *v7_create(void) {
  static int prototypes_initialized = 0;
  struct v7 *v7 = NULL;

  if (prototypes_initialized == 0) {
    prototypes_initialized++;
    init_stdlib();  // One-time initialization
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

struct v7_val *v7_push_string(struct v7 *v7, const char *str,
                              unsigned long n, int own) {
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

enum v7_type v7_type(const struct v7_val *v) {
  return v->type;
}

double v7_number(const struct v7_val *v) {
  return v->v.num;
}

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
  // Append to the end of the list, to make indexing work
  for (head = &arr->v.array; *head != NULL; head = &head[0]->next);
  prop = mkprop(v7);
  CHECK(prop != NULL, V7_OUT_OF_MEMORY);
  prop->next = *head;
  *head = prop;
  prop->key = NULL;
  prop->val = val;
  inc_ref_count(val);
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
      // TODO(lsm): add the rest of types
    default: abort(); break;
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
    if (m != head) n += snprintf(buf + n , bsiz - n, "%s", ", ");
    v7_stringify(m->val, buf + n, bsiz - n);
    n = (int) strlen(buf);
  }
  n += snprintf(buf + n, bsiz - n, "%s", "]");
}

static void obj_to_string(const struct v7_val *v, char *buf, int bsiz) {
  const struct v7_prop *m, *head = v->props;
  int n = snprintf(buf, bsiz, "%s", "{");

  for (m = head; m != NULL && n < bsiz - 1; m = m->next) {
    if (m != head) n += snprintf(buf + n , bsiz - n, "%s", ", ");
    v7_stringify(m->key, buf + n, bsiz - n);
    n = (int) strlen(buf);
    n += snprintf(buf + n , bsiz - n, "%s", ": ");
    v7_stringify(m->val, buf + n, bsiz - n);
    n = (int) strlen(buf);
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
    // TODO: check this on 32-bit arch
    if (v->v.num > ((uint64_t) 1 << 52) || ceil(v->v.num) != v->v.num) {
      snprintf(buf, bsiz, "%lg", v->v.num);
    } else {
      snprintf(buf, bsiz, "%lu", (unsigned long) v->v.num);
    }
  } else if (is_string(v)) {
    snprintf(buf, bsiz, "%.*s", (int) v->v.str.len, v->v.str.buf);
  } else if (v7_is_class(v, V7_CLASS_ARRAY)) {
    arr_to_string(v, buf, bsiz);
  } else if (v7_is_class(v, V7_CLASS_FUNCTION)) {
    if (v->flags & V7_JS_FUNC) {
      snprintf(buf, bsiz, "'function%s'", v->v.func.source_code);
    } else {
      snprintf(buf, bsiz, "'c_func_%p'", v->v.c_func);
    }
  } else if (v7_is_class(v, V7_CLASS_REGEXP)) {
    snprintf(buf, bsiz, "/%s/", v->v.regex);
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
  //return status;
}

#ifdef V7_EXE
int main(int argc, char *argv[]) {
  struct v7 *v7 = v7_create();
  int i;//, error_code;

  // Execute inline code
  for (i = 1; i < argc && argv[i][0] == '-'; i++) {
    if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
      if (!v7_exec(v7, argv[i + 1])) {
        fprintf(stderr, "Error executing [%s]: %s\n", argv[i + 1],
                v7_get_error_string(v7));
      }
      i++;
    }
  }

  // Execute files
  for (; i < argc; i++) {
    if (!v7_exec_file(v7, argv[i])) {
      fprintf(stderr, "%s\n", v7_get_error_string(v7));
    }
  }

  v7_destroy(&v7);
  return EXIT_SUCCESS;
}
#endif
