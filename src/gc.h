/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef GC_H_INCLUDED
#define GC_H_INCLUDED

#include "internal.h"
#include "vm.h"

#if defined(__cplusplus)
extern "C" {
#endif  /* __cplusplus */

V7_PRIVATE struct v7_object *new_object(struct v7 *);
V7_PRIVATE struct v7_property *new_property(struct v7 *);
V7_PRIVATE struct v7_function *new_function(struct v7 *);

V7_PRIVATE void gc_mark(val_t);

#if defined(__cplusplus)
}
#endif  /* __cplusplus */

#endif  /* GC_H_INCLUDED */
