// Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
// Copyright (c) 2013-2014 Cesanta Software Limited
// All rights reserved
//
// This library is dual-licensed: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation. For the terms of this
// license, see <http://www.gnu.org/licenses/>.
//
// You are free to use this library under the terms of the GNU General
// Public License, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// Alternatively, you can license this library under a commercial
// license, as set out in <http://cesanta.com/products.html>.

#ifndef V7_HEADER_INCLUDED
#define  V7_HEADER_INCLUDED

#define V7_VERSION "1.0"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum v7_type {
  V7_UNDEF, V7_NULL, V7_OBJ, V7_NUM, V7_STR, V7_BOOL, V7_FUNC,
  V7_C_FUNC, V7_REF
};

enum v7_err {
  V7_OK, V7_SYNTAX_ERROR, V7_OUT_OF_MEMORY, V7_INTERNAL_ERROR,
  V7_STACK_OVERFLOW, V7_STACK_UNDERFLOW, V7_UNDEFINED_VARIABLE,
  V7_TYPE_MISMATCH, V7_RECURSION_TOO_DEEP
};

struct v7;
struct v7_val;
struct v7_map;
typedef void (*v7_func_t)(struct v7 *, struct v7_val *result,
                          struct v7_val *params, int num_params);

// A string.
struct v7_str {
  char *buf;      // Pointer to buffer with string data
  int len;        // String length
  int buf_size;   // Buffer size. Should be greater or equal to string length
};

union v7_v {
  struct v7_str str;
  double num;
  v7_func_t c_func;
  char *func;
  struct v7_map *map;
};

struct v7_val {
  enum v7_type type;
  union v7_v v;
};

// Key/value pair. "struct v7_map *" is a key/val list head, represents object
struct v7_map {
  struct v7_map *next;
  struct v7_val key;
  struct v7_val val;
};

struct v7 *v7_create(void);
void v7_destroy(struct v7 **);

enum v7_err v7_exec(struct v7 *, const char *source_code);
enum v7_err v7_exec_file(struct v7 *, const char *path);
enum v7_err v7_define_func(struct v7 *, const char *name, v7_func_t c_func);
enum v7_err v7_assign(struct v7_val *obj, struct v7_val *k, struct v7_val *v);
void v7_call(struct v7 *v7, struct v7_val *function, int num_params);
int v7_sp(struct v7 *v7);
struct v7_val *v7_stk(struct v7 *);    // Get bottom of the stack
struct v7_val *v7_top(struct v7 *);    // Get top of the stack
struct v7_val *v7_push(struct v7 *v7, enum v7_type type);

void v7_init_stdlib(struct v7 *);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // V7_HEADER_INCLUDED
