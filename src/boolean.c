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
  if (this_obj != v7->global_object) {
    /* called as "new Boolean(...)" */
    val_t obj = create_object(v7, v7->boolean_prototype);
    v7_set_property(v7, obj, "", 0, V7_PROPERTY_HIDDEN, v);
    return obj;
  } else {
    return v;
  }
}

V7_PRIVATE void init_boolean(struct v7 *v7) {
  v7_set_property(v7, v7->global_object, "Boolean", 7, 0,
                  v7_create_cfunction(Boolean_ctor));
}
