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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>   /* For size_t */

#define V7_VERSION "1.0"

struct v7;     /* Opaque structure. V7 engine handler. */
struct v7_val; /* Opaque structure. Holds V7 value, which has v7_type type. */


/* TODO(lsm): fix this. */
#include <inttypes.h>
typedef uint64_t v7_val_t;

typedef v7_val_t (*v7_cfunction_t)(struct v7 *, v7_val_t args);

struct v7 *v7_create(void);     /* Creates and initializes V7 engine */
void v7_destroy(struct v7 *);   /* Cleanes up and deallocates V7 engine */
v7_val_t v7_exec(struct v7 *, const char *str);       /* Execute string */
v7_val_t v7_exec_file(struct v7 *, const char *path); /* Execute file */

v7_val_t v7_create_object(struct v7 *v7);
v7_val_t v7_create_array(struct v7 *v7);
v7_val_t v7_create_cfunction(v7_cfunction_t func);
v7_val_t v7_create_number(double num);
v7_val_t v7_create_boolean(int is_true);
v7_val_t v7_create_null(void);
v7_val_t v7_create_undefined(void);
v7_val_t v7_create_string(struct v7 *v7, const char *, size_t, int);

v7_val_t v7_get_global_object(struct v7 *);
int v7_set_property(struct v7 *, v7_val_t obj, const char *name, size_t len,
                    unsigned int attributes, v7_val_t val);
char *v7_to_json(struct v7 *, v7_val_t, char *, size_t);
int v7_is_true(struct v7 *v7, v7_val_t v);
void v7_array_append(struct v7 *, v7_val_t arr, v7_val_t v);
v7_val_t v7_array_at(struct v7 *, v7_val_t arr, long index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* V7_HEADER_INCLUDED */
