/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE val_t Obj_getPrototypeOf(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg = v7_array_at(v7, args, 0);
  (void) this_obj;
  if (!v7_is_object(arg)) {
    throw_exception(v7, "Object.getPrototypeOf called on non-object");
  }
  return v7_object_to_value(val_to_object(arg)->prototype);
}

V7_PRIVATE val_t Obj_create(struct v7 *v7, val_t this_obj, val_t args) {
  val_t proto = v7_array_at(v7, args, 0);
  (void) this_obj;
  if (!v7_is_null(proto) && !v7_is_object(proto)) {
    throw_exception(v7, "Object prototype may only be an Object or null");
  }
  return create_object(v7, proto);
}

V7_PRIVATE val_t Obj_isPrototypeOf(struct v7 *v7, val_t this_obj, val_t args) {
  val_t obj = v7_array_at(v7, args, 0);
  val_t proto = v7_array_at(v7, args, 1);
  (void) this_obj;
  return v7_create_boolean(is_prototype_of(obj, proto));
}

#if 0
V7_PRIVATE enum v7_err Obj_toString(struct v7_c_func_arg *cfa) {
  char *p, buf[500];
  p = v7_stringify(cfa->this_obj, buf, sizeof(buf));
  v7_push_string(cfa->v7, p, strlen(p), 1);
  if (p != buf) free(p);
  return V7_OK;
}

V7_PRIVATE enum v7_err Obj_keys(struct v7_c_func_arg *cfa) {
  struct v7_prop *p;
  struct v7_val *result = v7_push_new_object(cfa->v7);
  v7_set_class(result, V7_CLASS_ARRAY);
  for (p = cfa->this_obj->props; p != NULL; p = p->next) {
    v7_append(cfa->v7, result, p->key);
  }
  return V7_OK;
}

#endif
V7_PRIVATE void init_object(struct v7 *v7) {
  val_t object;
  /* TODO(mkm): initialize global object without requiring a parser */
  v7_exec(v7, "function Object() {}");

  object = v7_property_value(v7_get_property(v7->global_object, "Object", 6));
  v7_set_property(v7, object, "prototype", 9, 0, v7->object_prototype);
  v7_set_property(v7, object, "getPrototypeOf", 14, 0,
                  v7_create_cfunction(Obj_getPrototypeOf));
  v7_set_property(v7, object, "isPrototypeOf", 14, 0,
                  v7_create_cfunction(Obj_isPrototypeOf));
  v7_set_property(v7, object, "create", 6, 0,
                  v7_create_cfunction(Obj_create));
}
