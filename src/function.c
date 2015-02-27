/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t Function_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  long i, n = 0, num_args = v7_array_length(v7, args);
  size_t size;
  char buf[200];
  const char *s;
  val_t param, body, res = v7_create_undefined();

  (void) this_obj;

  if (num_args <= 0) return res;

  /* TODO(lsm): Constructing function source code here. Optimize this. */
  n += snprintf(buf + n, sizeof(buf) - n, "%s", "(function(");

  for (i = 0; i < num_args - 1; i++) {
    param = i_value_of(v7, v7_array_at(v7, args, i));
    if (v7_is_string(param)) {
      s = v7_to_string(v7, &param, &size);
      if (i > 0) {
        n += snprintf(buf + n, sizeof(buf) - n, "%s", ",");
      }
      n += snprintf(buf + n, sizeof(buf) - n, "%.*s", (int) size, s);
    }
  }
  n += snprintf(buf + n, sizeof(buf) - n, "%s", "){");
  body = i_value_of(v7, v7_array_at(v7, args, num_args - 1));
  if (v7_is_string(body)) {
    s = v7_to_string(v7, &body, &size);
    n += snprintf(buf + n, sizeof(buf) - n, "%.*s", (int) size, s);
  }
  n += snprintf(buf + n, sizeof(buf) - n, "%s", "})");

  if (v7_exec_with(v7, &res, buf, V7_UNDEFINED) != V7_OK) {
    throw_exception(v7, "SyntaxError", "Invalid function body");
  }

  return res;
}

V7_PRIVATE void init_function(struct v7 *v7) {
  val_t ctor = v7_create_cfunction_object(v7, Function_ctor, 1);
  v7_set_property(v7, ctor, "prototype", 9, 0, v7->function_prototype);
  v7_set_property(v7, v7->global_object, "Function", 8, 0, ctor);
}
