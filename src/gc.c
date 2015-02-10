/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"
#include "gc.h"

V7_PRIVATE struct v7_object *new_object(struct v7 *v7) {
  (void) v7;
  return (struct v7_object *) malloc(sizeof(struct v7_object));
}

V7_PRIVATE struct v7_property *new_property(struct v7 *v7) {
  (void) v7;
  return (struct v7_property *) malloc(sizeof(struct v7_property));
}

V7_PRIVATE struct v7_function *new_function(struct v7 *v7) {
  (void) v7;
  return (struct v7_function *) malloc(sizeof(struct v7_function));
}
