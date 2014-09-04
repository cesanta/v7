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
#include "slre.h"

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

// If V7_CACHE_OBJS is defined, then v7_freeval() will not actually free
// the structure, but append it to the list of free structures.
// Subsequent allocations try to grab a structure from the free list,
// which speeds up allocation.
//#define V7_CACHE_OBJS

// Maximum length of the string literal
#define MAX_STRING_LITERAL_LENGTH 2000

// Different classes of V7_TYPE_OBJ type
enum v7_class {
  V7_CLASS_NONE, V7_CLASS_ARRAY, V7_CLASS_BOOLEAN, V7_CLASS_DATE,
  V7_CLASS_ERROR, V7_CLASS_FUNCTION, V7_CLASS_NUMBER, V7_CLASS_OBJECT,
  V7_CLASS_REGEXP, V7_CLASS_STRING, V7_NUM_CLASSES
};

enum v7_tok {
  TOK_END_OF_INPUT, TOK_NUMBER, TOK_STRING_LITERAL, TOK_IDENTIFIER,

  // Punctuators
  TOK_OPEN_CURLY, TOK_CLOSE_CURLY, TOK_OPEN_PAREN, TOK_CLOSE_PAREN,
  TOK_OPEN_BRACKET, TOK_CLOSE_BRACKET, TOK_DOT, TOK_COLON, TOK_SEMICOLON,
  TOK_COMMA, TOK_EQ_EQ, TOK_EQ, TOK_ASSIGN, TOK_NE, TOK_NE_NE, TOK_NOT,
  TOK_REM_ASSIGN, TOK_REM, TOK_MUL_ASSIGN, TOK_MUL, TOK_DIV_ASSIGN, TOK_DIV,
  TOK_XOR_ASSIGN, TOK_XOR, TOK_PLUS_PLUS, TOK_PLUS_ASSING, TOK_PLUS,
  TOK_MINUS_MINUS, TOK_MINUS_ASSING, TOK_MINUS, TOK_LOGICAL_AND,
  TOK_LOGICAL_AND_ASSING, TOK_AND, TOK_LOGICAL_OR, TOK_LOGICAL_OR_ASSING,
  TOK_OR, TOK_QUESTION, TOK_TILDA, TOK_LE, TOK_LT, TOK_GE, TOK_GT,
  TOK_LSHIFT_ASSIGN, TOK_LSHIFT, TOK_RSHIFT_ASSIGN, TOK_RSHIFT,

  // Keywords. must be in the same order as tokenizer.c::s_keywords array
  TOK_BREAK, TOK_CASE, TOK_CATCH, TOK_CONTINUE, TOK_DEBUGGER, TOK_DEFAULT,
  TOK_DELETE, TOK_DO, TOK_ELSE, TOK_FALSE, TOK_FINALLY, TOK_FOR, TOK_FUNCTION,
  TOK_IF, TOK_IN, TOK_INSTANCEOF, TOK_NEW, TOK_NULL, TOK_RETURN, TOK_SWITCH,
  TOK_THIS, TOK_THROW, TOK_TRUE, TOK_TRY, TOK_TYPEOF, TOK_UNDEFINED, TOK_VAR,
  TOK_VOID, TOK_WHILE, TOK_WITH,

  // TODO(lsm): process these reserved words too
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
#define V7_PROP_NOT_WRITABLE   1  // property is not changeable
#define V7_PROP_NOT_ENUMERABLE 2  // not enumerable in for..in loop
#define V7_PROP_NOT_DELETABLE  4  // delete-ing this property must fail
#define V7_PROP_ALLOCATED      8  // v7_prop must be free()-ed
};

// Vector, describes some memory location pointed by 'p' with length 'len'
struct v7_vec {
  const char *p;
  int len;
};

struct v7_string {
  char *buf;                // Pointer to buffer with string data
  unsigned long len;        // String length
  char loc[16];             // Small strings are stored here
};

struct v7_func {
  char *source_code;        // \0-terminated function source code
  int line_no;              // Line number where function begins
  struct v7_val *var_obj;   // Function var object: var decls and func defs
};

union v7_scalar {
  char *regex;              // \0-terminated regex
  double num;               // Holds "Number" or "Boolean" value
  struct v7_string str;     // Holds "String" value
  struct v7_func func;      // \0-terminated function code
  struct v7_prop *array;    // List of array elements
  v7_func_t c_func;       // Pointer to the C function
  v7_prop_func_t prop_func; // Object's property function, e.g. String.length
};

struct v7_val {
  struct v7_val *next;
  struct v7_val *proto;       // Prototype
  struct v7_val *ctor;        // Constructor object
  struct v7_prop *props;      // Object's key/value list
  union v7_scalar v;          // The value itself
  enum v7_type type;          // Value type
  enum v7_class cls;          // Object's internal [[Class]] property
  short ref_count;            // Reference counter

  unsigned short flags;       // Flags - defined below
#define V7_VAL_ALLOCATED   1  // Whole "struct v7_val" must be free()-ed
#define V7_STR_ALLOCATED   2  // v.str.buf must be free()-ed
#define V7_JS_FUNC         4  // Function object is a JavsScript code
#define V7_PROP_FUNC       8  // Function object is a native property function
#define V7_VAL_DEALLOCATED 16 // Value has been deallocated
};

#define V7_MKVAL(_p,_t,_c,_v) {0,(_p),0,0,{(_v)},(_t),(_c),0,0}
#define V7_MKNUM(_v) V7_MKVAL(0,V7_TYPE_NUM,V7_CLASS_NONE,\
  (char *) (unsigned long) (_v))

struct v7_pstate {
  const char *file_name;
  const char *source_code;
  const char *pc;             // Current parsing position
  int line_no;                // Line number
};

struct v7 {
  struct v7_val root_scope;   // "global" object (root-level execution context)
  struct v7_val *stack[200];  // TODO: make it non-fixed, auto-grow
  int sp;                     // Stack pointer
  int flags;
#define V7_SCANNING  1        // Pre-scan to initialize lexical scopes, no exec
#define V7_NO_EXEC   2        // Non-executing code block: if (false) { block }

  struct v7_pstate pstate;    // Parsing state
  enum v7_tok cur_tok;        // Current token
  const char *tok;            // Parsed terminal token (ident, number, string)
  unsigned long tok_len;      // Length of the parsed terminal token

  char error_message[100];    // Placeholder for the error message

  struct v7_val *cur_obj;     // Current namespace object ('x=1; x.y=1;', etc)
  struct v7_val *this_obj;    // Current "this" object
  struct v7_val *ctx;         // Current execution context
  struct v7_val *cf;          // Currently executing function
  struct v7_val *functions;   // List of declared function
  struct v7_val *free_values; // List of free (deallocated) values
  struct v7_prop *free_props; // List of free (deallocated) props
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
#define TRY(call) do {enum v7_err _e = call; CHECK(_e == V7_OK, _e); } while (0)

// Print current function name and stringified object
#define TRACE_OBJ(O) do { char x[4000]; printf("==> %s [%s]\n", __func__, \
  O == NULL ? "@" : v7_stringify(O, x, sizeof(x))); } while (0)

// Initializer for "struct v7_val", object type
#define MKOBJ(_proto) V7_MKVAL(_proto, V7_TYPE_OBJ, V7_CLASS_OBJECT, 0)

// True if current code is executing. TODO(lsm): use bit fields, per vrz@
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

// Adds a read-only attribute "val" by key "name" to the object "obj"
#define SET_RO_PROP_V(obj, name, val) \
  do { \
    static struct v7_val key = MKOBJ(&s_prototypes[V7_CLASS_STRING]); \
    static struct v7_prop prop = { NULL, &key, &val, 0 }; \
    v7_init_str(&key, (char *) (name), strlen(name), 0); \
    prop.next = obj.props; \
    obj.props = &prop; \
  } while (0)

// Adds read-only attribute with given initializers to the object "_o"
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

// Adds property function "_func" with key "_name" to the object "_obj"
#define SET_PROP_FUNC(_obj, _name, _func) \
    SET_RO_PROP2(_obj, _name, V7_TYPE_NULL, 0, prop_func, _func, V7_PROP_FUNC)

// Adds method "_func" with key "_name" to the object "_obj"
#define SET_METHOD(_obj, _name, _func) \
  do {  \
    static struct v7_val _val = MKOBJ(&s_prototypes[V7_CLASS_STRING]); \
    v7_set_class(&_val, V7_CLASS_FUNCTION); \
    _val.v.c_func = (_func); \
    SET_RO_PROP_V(_obj, _name, _val); \
  } while (0)


// Forward declarations
V7_PRIVATE int skip_to_next_tok(const char **ptr);
V7_PRIVATE enum v7_tok get_tok(const char **s, double *n);
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
V7_PRIVATE void free_props(struct v7 *v7);
V7_PRIVATE void free_values(struct v7 *v7);
V7_PRIVATE struct v7_val v7_str_to_val(const char *buf);
V7_PRIVATE enum v7_err v7_del2(struct v7 *v7, struct v7_val *,
  const char *, unsigned long);

// Generic function to set an attribute in an object.
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
