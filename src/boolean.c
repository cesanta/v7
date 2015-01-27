/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE val_t Boolean_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  val_t v = v7_create_boolean(0);   /* false by default */

  if (v7_is_true(v7, v7_array_at(v7, args, 0))) {
    v = v7_create_boolean(1);
  }

  if (v7_is_object(this_obj) && this_obj != v7->global_object) {
    /* called as "new Boolean(...)" */
    v7_to_object(this_obj)->prototype = v7_to_object(v7->boolean_prototype);
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, v);
    v = this_obj;
  }

  return v;
}

V7_PRIVATE void init_boolean(struct v7 *v7) {
  val_t boolean = v7_create_cfunction(Boolean_ctor);
  v7_set_property(v7, v7->global_object, "Boolean", 7, 0, boolean);
  v7_set(v7, v7->boolean_prototype, "constructor", 11, boolean);
}
