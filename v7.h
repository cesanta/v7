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
  V7_UNDEF, V7_NULL, V7_OBJ, V7_NUM, V7_STR, V7_BOOL, V7_FUNC, V7_C_FUNC,
  V7_RO_PROP, V7_ARRAY, V7_REGEX
};

enum v7_err {
  V7_OK, V7_SYNTAX_ERROR, V7_OUT_OF_MEMORY, V7_INTERNAL_ERROR,
  V7_STACK_OVERFLOW, V7_STACK_UNDERFLOW, V7_UNDEFINED_VARIABLE,
  V7_TYPE_MISMATCH, V7_CALLED_NON_FUNCTION, V7_NOT_IMPLEMENTED,
  V7_NUM_ERRORS
};

struct v7;
struct v7_val;
typedef void (*v7_c_func_t)(struct v7 *, struct v7_val *this_obj,
                            struct v7_val *result,
                            struct v7_val **params, int num_params);
typedef void (*v7_prop_func_t)(struct v7_val *this_obj, struct v7_val *result);

struct v7_prop {
  struct v7_prop *next;
  struct v7_val *key;
  struct v7_val *val;
};

struct v7_string {
  char *buf;                // Pointer to buffer with string data
  unsigned long len;        // String length
};

struct v7_func {
  char *source_code;        // \0-terminated function source code
  int line_no;              // Line number where function begins
  struct v7_val *scope;     // Function's scope
  struct v7_val *upper;     // Upper-level function
  struct v7_val *args;      // Function arguments
};

union v7_scalar {
  char *regex;              // \0-terminated regex
  double num;
  struct v7_string str;
  struct v7_func func;      // \0-terminated function code
  v7_c_func_t c_func;       // C function
  v7_prop_func_t prop_func; // Object's property function, e.g. String.length
  struct v7_prop *props;    // Object's key/value list
};

struct v7_val {
  struct v7_val *next;
  struct v7_val *proto;       // Prototype
  enum v7_type type;          // Value type
  short ref_count;            // Reference counter
  unsigned short flags;
  union v7_scalar v;          // The value itself
};

struct v7 {
  struct v7_val *stack[200];  // TODO: make it non-fixed, auto-grow
  struct v7_val root_scope;
  struct v7_val *curr_func;   // Currently executing function
  int sp;                     // Stack pointer

  const char *source_code;    // Pointer to the source codeing
  const char *pc;             // Current parsing position
  const char *tok;            // Parsed terminal token (ident, number, string)
  unsigned long tok_len;      // Length of the parsed terminal token
  int line_no;                // Line number
  int no_exec;                // No-execute flag. For parsing function defs
  struct v7_val *cur_obj;     // Current namespace object ('x=1; x.y=1;', etc)
  struct v7_val *this_obj;
  struct v7_val *free_values;
  struct v7_prop *free_props;
};

struct v7 *v7_create(void);
void v7_destroy(struct v7 **);

enum v7_err v7_exec(struct v7 *, const char *source_code);
enum v7_err v7_exec_file(struct v7 *, const char *path);
enum v7_err v7_push(struct v7 *v7, struct v7_val *v);
enum v7_err v7_make_and_push(struct v7 *v7, enum v7_type type);
enum v7_err v7_call(struct v7 *v7, struct v7_val *this_obj, int num_args);
enum v7_err v7_setv(struct v7 *v7, struct v7_val *obj,
                    enum v7_type key_type, enum v7_type val_type, ...);
enum v7_err v7_append(struct v7 *, struct v7_val *array, struct v7_val *val);
enum v7_err v7_del(struct v7 *v7, struct v7_val *obj, struct v7_val *key);
enum v7_err v7_pop(struct v7 *, int num);
struct v7_val *v7_mkv(struct v7 *v7, enum v7_type t, ...);
void v7_freeval(struct v7 *v7, struct v7_val *v);
void v7_set_value_type(struct v7_val *v, enum v7_type type);
struct v7_val *v7_lookup(struct v7_val *obj, const char *key);
struct v7_val *v7_rootns(struct v7 *);
void v7_copy(struct v7 *v7, struct v7_val *from, struct v7_val *to);
int v7_is_true(const struct v7_val *v);
int v7_sp(struct v7 *v7);
struct v7_val **v7_top(struct v7 *);
const char *v7_to_string(const struct v7_val *v, char *buf, int bsiz);
const char *v7_strerror(enum v7_err);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // V7_HEADER_INCLUDED
