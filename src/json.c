/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t Json_stringify(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  char buf[100], *p = v7_to_json(v7, arg0, buf, sizeof(buf));
  val_t res = v7_create_string(v7, p, strlen(p), 1);
  (void) this_obj;
  if (p != buf) free(p);
  return res;
}

V7_PRIVATE void init_json(struct v7 *v7) {
  val_t o = v7_create_object(v7);
  set_cfunc_obj_prop(v7, o, "stringify", Json_stringify, 1);
  set_cfunc_obj_prop(v7, o, "parse", Std_eval, 1);
  v7_set_property(v7, v7->global_object, "JSON", 4, V7_PROPERTY_DONT_ENUM, o);
}
