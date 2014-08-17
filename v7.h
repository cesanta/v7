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

#ifndef V7_HEADER_INCLUDED
#define  V7_HEADER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define V7_VERSION "1.0"

enum v7_type {
  V7_TYPE_UNDEF, V7_TYPE_NULL, V7_TYPE_BOOL, V7_TYPE_STR, V7_TYPE_NUM,
  V7_TYPE_OBJ, V7_NUM_TYPES
};

// Different classes of V7_TYPE_OBJ type
enum v7_class {
  V7_CLASS_ARRAY, V7_CLASS_BOOLEAN, V7_CLASS_DATE, V7_CLASS_ERROR,
  V7_CLASS_FUNCTION, V7_CLASS_NUMBER, V7_CLASS_OBJECT, V7_CLASS_REGEXP,
  V7_CLASS_STRING, V7_NUM_CLASSES
};

enum v7_err {
  V7_OK, V7_ERROR, V7_EVAL_ERROR, V7_RANGE_ERROR, V7_REFERENCE_ERROR,
  V7_SYNTAX_ERROR, V7_TYPE_ERROR, V7_URI_ERROR,
  V7_OUT_OF_MEMORY, V7_INTERNAL_ERROR, V7_STACK_OVERFLOW, V7_STACK_UNDERFLOW,
  V7_CALLED_NON_FUNCTION, V7_NOT_IMPLEMENTED,
  V7_NUM_ERRORS
};

struct v7;
struct v7_val;

struct v7_c_func_arg {
  struct v7 *v7;
  struct v7_val *this_obj;
  struct v7_val *result;
  struct v7_val **args;
  int num_args;
  int called_as_constructor;
};

typedef enum v7_err (*v7_c_func_t)(struct v7_c_func_arg *arg);
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

struct v7_string {
  char *buf;                // Pointer to buffer with string data
  unsigned long len;        // String length
  char loc[16];             // Small strings are stored here
};

struct v7_func {
  char *source_code;        // \0-terminated function source code
  int line_no;              // Line number where function begins
  struct v7_val *scope;     // Function's scope
  struct v7_val *upper;     // Upper-level function
  struct v7_val *args;      // Function arguments
  struct v7_val *var_obj;   // Function var object: var decls and func defs
};

union v7_scalar {
  char *regex;              // \0-terminated regex
  double num;               // Holds "Number" or "Boolean" value
  struct v7_string str;     // Holds "String" value
  struct v7_func func;      // \0-terminated function code
  struct v7_prop *array;    // List of array elements
  v7_c_func_t c_func;       // Pointer to the C function
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

struct v7_pstate {
  const char *source_code;    // Pointer to the source code
  const char *pc;             // Current parsing position
  int line_no;                // Line number
};

struct v7 {
  struct v7_val root_scope;   // "global" object (root-level execution context)
  struct v7_val *curr_func;   // Currently executing function
  struct v7_val *stack[200];  // TODO: make it non-fixed, auto-grow
  int sp;                     // Stack pointer

  struct v7_pstate pstate;    // Parsing state
  const char *tok;            // Parsed terminal token (ident, number, string)
  unsigned long tok_len;      // Length of the parsed terminal token
  int no_exec;                // No-execute flag. For parsing function defs
  struct v7_val *cur_obj;     // Current namespace object ('x=1; x.y=1;', etc)
  struct v7_val *this_obj;    // Current "this" object

  struct v7_val global_obj;   // Global object
  struct v7_val global_ctx;   // Global execution context
  struct v7_val *ctx;         // Current execution context
  struct v7_val *cur_var_obj; // Current var_obj

  struct v7_val *free_values; // List of free (deallocated) values
  struct v7_prop *free_props; // List of free (deallocated) props
};

struct v7 *v7_create(void);
void v7_destroy(struct v7 **);

enum v7_err v7_exec(struct v7 *, const char *source_code);
enum v7_err v7_exec_file(struct v7 *, const char *path);
enum v7_err v7_push(struct v7 *v7, struct v7_val *v);
enum v7_err v7_make_and_push(struct v7 *v7, enum v7_type type);
enum v7_err v7_call(struct v7 *v7, struct v7_val *this_obj, int num_args, int);
enum v7_err v7_setv(struct v7 *v7, struct v7_val *obj,
                    enum v7_type key_type, enum v7_type val_type, ...);
enum v7_err v7_append(struct v7 *, struct v7_val *array, struct v7_val *val);
enum v7_err v7_del(struct v7 *v7, struct v7_val *obj, struct v7_val *key);
enum v7_err v7_pop(struct v7 *, int num);
struct v7_val *v7_mkv(struct v7 *v7, enum v7_type t, ...);
void v7_freeval(struct v7 *v7, struct v7_val *v);
struct v7_val *v7_lookup(struct v7_val *obj, const char *key);
struct v7_val *v7_rootns(struct v7 *);
void v7_copy(struct v7 *v7, struct v7_val *from, struct v7_val *to);
int v7_is_true(const struct v7_val *v);
int v7_sp(struct v7 *v7);
struct v7_val **v7_top(struct v7 *);
struct v7_val *v7_top_val(struct v7 *);
const char *v7_to_string(const struct v7_val *v, char *buf, int bsiz);
const char *v7_strerror(enum v7_err);
int v7_is_class(const struct v7_val *obj, enum v7_class cls);
void v7_set_class(struct v7_val *obj, enum v7_class cls);
void v7_init_func(struct v7_val *v, v7_c_func_t func);
void v7_init_str(struct v7_val *v, char *p, unsigned long len, int own);
void v7_init_num(struct v7_val *v, double num);
void v7_init_bool(struct v7_val *v, int);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // V7_HEADER_INCLUDED
