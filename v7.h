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

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif

#ifdef V7_DEBUG
#define DBG(x) do { printf("%-20s ", __func__); printf x; putchar('\n'); \
  fflush(stdout); } while(0)
#else
#define DBG(x)
#endif

#define V7_VERSION "1.0"
#define MAX_STRING_LITERAL_LENGTH 500
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

// Linked list interface
struct llhead { struct llhead *prev, *next; };

#define LL_INIT(N)              ((N)->next = (N)->prev = (N))
#define LL_DECLARE_AND_INIT(H)  struct llhead H = { &H, &H }
#define LL_ENTRY(P,T,N)         ((T *)((char *)(P) - offsetof(T, N)))
#define LL_IS_EMPTY(N)          ((N)->next == (N))
#define LL_FOREACH(H,N,T)       for (N = (H)->next, T = (N)->next; N != (H); \
                                     N = (T), T = (N)->next)
#define LL_ADD(H, N)  do { ((H)->next)->prev = (N); (N)->next = ((H)->next); \
                            (N)->prev = (H); (H)->next = (N); } while (0)
#define LL_TAIL(H, N) do { ((H)->prev)->next = (N); (N)->prev = ((H)->prev); \
                            (N)->next = (H); (H)->prev = (N); } while (0)
#define LL_DEL(N)     do { ((N)->next)->prev = ((N)->prev); \
                            ((N)->prev)->next = ((N)->next); \
                            LL_INIT(N); } while (0)

typedef unsigned int v7_string_size_t;

enum v7_type {
  V7_UNDEF, V7_NULL, V7_OBJ, V7_NUM, V7_STR, V7_BOOL,
  V7_FUNC, V7_C_FUNC, V7_REF
};

enum v7_err {
  V7_OK, V7_SYNTAX_ERROR, V7_OUT_OF_MEMORY, V7_INTERNAL_ERROR,
  V7_STACK_OVERFLOW, V7_STACK_UNDERFLOW, V7_UNDEFINED_VARIABLE,
  V7_TYPE_MISMATCH, V7_RECURSION_TOO_DEEP
};

struct v7;
struct v7_val;
struct v7_map;
typedef void (*v7_func_t)(struct v7 *, struct v7_val *obj,
                          struct v7_val *result,
                          struct v7_val **params, int num_params);

// A string.
struct v7_str {
  char *buf;      // Pointer to buffer with string data
  int len;        // String length
};

union v7_v {
  struct v7_str str;
  double num;
  v7_func_t c_func;
  char *func;
  struct v7_map *map;
  struct v7_val *ref;
};

struct v7_val {
  struct llhead link;           // Linkage in struct v7::values
  enum v7_type type;            // Value type
  unsigned char ref_count;      // Reference counter
  unsigned char str_unowned:1;  // Object's string must not fe free-ed
  unsigned char val_unowned:1;  // Object must not be free-d
  union v7_v v;                 // The value itself
};

// Key/value pair. "struct v7_map *" is a key/val list head, represents object
struct v7_map {
  struct v7_map *next;
  struct v7_val *key;
  struct v7_val *val;
};

struct v7 {
  struct v7_val *stack[200];
  int sp;                     // Stack pointer
  struct v7_val scopes[20];   // Namespace objects (scopes)
  int current_scope;          // Pointer to the current scope

  const char *source_code;    // Pointer to the source codeing
  const char *cursor;         // Current parsing position
  int line_no;                // Line number
  int no_exec;                // No-execute flag. For parsing function defs
  const char *tok;            // Parsed terminal token (ident, number, string)
  int tok_len;                // Length of the parsed terminal token
  struct v7_val *cur_obj;     // Current namespace object ('x=1; x.y=1;', etc)
  struct llhead values;       // List of allocated values
};

struct v7 *v7_create(void);
void v7_destroy(struct v7 **);

enum v7_err v7_exec(struct v7 *, const char *source_code);
enum v7_err v7_exec_file(struct v7 *, const char *path);

enum v7_err v7_push(struct v7 *v7, enum v7_type type);
enum v7_err v7_call(struct v7 *v7, struct v7_val *func);

#if 0
struct v7_val *v7_set(struct v7_val *obj, struct v7_val *k, struct v7_val *v);
struct v7_val *v7_set_num(struct v7_val *, const char *key, double num);
struct v7_val *v7_set_str(struct v7_val *, const char *key, const char *, int);
struct v7_val *v7_set_obj(struct v7_val *, const char *key);
struct v7_val *v7_set_func(struct v7_val *, const char *key, v7_func_t);
#endif
void v7_reg_func(struct v7 *v7, const char *key, v7_func_t c_func);

struct v7_map *v7_get(struct v7_val *obj, const struct v7_val *key);
struct v7_val *v7_get_root_namespace(struct v7 *);

int v7_sp(struct v7 *v7);             // Get number of values in the stack
struct v7_val **v7_top(struct v7 *);  // Get top of the stack

const char *v7_to_string(const struct v7_val *v, char *buf, int bsiz);
struct v7_val v7_str_to_val(const char *buf);

void v7_init_stdlib(struct v7 *);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // V7_HEADER_INCLUDED
