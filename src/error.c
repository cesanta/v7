/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t Error_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t res;

  if (v7_is_object(this_obj) && this_obj != v7->global_object) {
    res = this_obj;
  } else {
    res = create_object(v7, v7->error_prototype);
  }
  /* TODO(mkm): set non enumerable but provide toString method */
  v7_set_property(v7, res, "message", 7, 0, arg0);

  return res;
}

static const char *error_names[] = {"TypeError", "SyntaxError",
                                    "ReferenceError", "InternalError",
                                    "RangeError"};
V7_STATIC_ASSERT(ARRAY_SIZE(error_names) == ERROR_CTOR_MAX,
                 error_name_count_mismatch);

V7_PRIVATE void init_error(struct v7 *v7) {
  val_t error;
  size_t i;

  error = v7_create_cfunction_ctor(v7, v7->error_prototype, Error_ctor, 1);
  v7_set_property(v7, v7->global_object, "Error", 5, V7_PROPERTY_DONT_ENUM,
                  error);

  for (i = 0; i < ARRAY_SIZE(error_names); i++) {
    error = v7_create_cfunction_ctor(v7, create_object(v7, v7->error_prototype),
                                     Error_ctor, 1);
    v7_set_property(v7, v7->global_object, error_names[i],
                    strlen(error_names[i]), V7_PROPERTY_DONT_ENUM, error);
    v7->error_objects[i] = error;
  }
}
