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

typedef void (*v7_func_t)(struct v7 *, int num_params);

struct v7 *v7_create(void);
void v7_destroy(struct v7 **);

// All functions declared below return these error codes:
enum v7_error {
  V7_OK, V7_SYNTAX_ERROR, V7_OUT_OF_MEMORY, V7_INTERNAL_ERROR,
  V7_STACK_OVERFLOW, V7_STACK_UNDERFLOW, V7_UNDEFINED_VARIABLE,
  V7_TYPE_MISMATCH
};

// A string.
struct v7_str {
  char *buf;      // Pointer to buffer with string data
  int len;        // String length
  int buf_size;   // Buffer size. Should be greater or equal to string length
};

enum v7_type {
  V7_UNDEF, V7_NULL, V7_OBJ, V7_NUM, V7_STR, V7_BOOL, V7_FUNC, V7_C_FUNC
};
struct v7_value {
  enum v7_type type;
  union {
    struct v7_str str;
    double num;
    v7_func_t c_func;
    char *func;
    void *obj;
  } v;
};

enum v7_error v7_exec(struct v7 *, const char *source_code);
enum v7_error v7_exec_file(struct v7 *, const char *path);
enum v7_error v7_define_func(struct v7 *, const char *name, v7_func_t c_func);

struct v7_value *v7_bottom(struct v7 *);      // Return bottom of the stack
struct v7_value *v7_top(struct v7 *);         // Return top of the stack

struct v7_value *v7_push_null(struct v7 *);
struct v7_value *v7_push_undefined(struct v7 *);
struct v7_value *v7_push_double(struct v7 *, double val);
struct v7_value *v7_push_boolean(struct v7 *, int val);
struct v7_value *v7_push_string(struct v7 *, const char *str, int len);
struct v7_value *v7_push_object(struct v7 *);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // V7_HEADER_INCLUDED
