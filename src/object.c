/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE val_t Obj_getPrototypeOf(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg = v7_array_at(v7, args, 0);
  (void) this_obj;
  if (!v7_is_object(arg)) {
    throw_exception(v7, "TypeError",
                    "Object.getPrototypeOf called on non-object");
  }
  return v7_object_to_value(v7_to_object(arg)->prototype);
}

V7_PRIVATE val_t Obj_create(struct v7 *v7, val_t this_obj, val_t args) {
  val_t proto = v7_array_at(v7, args, 0);
  (void) this_obj;
  if (!v7_is_null(proto) && !v7_is_object(proto)) {
    throw_exception(v7, "TypeError",
                    "Object prototype may only be an Object or null");
  }
  return create_object(v7, proto);
}

V7_PRIVATE val_t Obj_isPrototypeOf(struct v7 *v7, val_t this_obj, val_t args) {
  val_t obj = v7_array_at(v7, args, 0);
  val_t proto = v7_array_at(v7, args, 1);
  (void) this_obj;
  return v7_create_boolean(is_prototype_of(obj, proto));
}

static val_t _Obj_ownKeys(struct v7 *v7, val_t args, unsigned int ignore_flags) {
  struct v7_property *p;
  char buf[20];
  int i = 0;
  val_t obj = v7_array_at(v7, args, 0);
  val_t res = v7_create_array(v7);
  if (!v7_is_object(obj)) {
    throw_exception(v7, "TypeError",
                    "Object.keys called on non-object");
  }
  for (p = v7_to_object(obj)->properties; p; p = p->next, i++) {
    if (p->attributes & ignore_flags) {
      continue;
    }
    snprintf(buf, sizeof(buf), "%d", i);
    v7_set_property(v7, res, buf, -1, 0,
                    v7_string_to_value(v7, p->name, strlen(p->name), 1));
  }
  return res;
}

static struct v7_property *_Obj_getOwnProperty(struct v7 *v7, val_t obj,
                                               val_t name) {
  char name_buf[512];
  int name_len;
  name_len = v7_stringify_value(v7, name, name_buf, sizeof(name_buf));
  return v7_get_own_property(obj, name_buf, name_len);
}

V7_PRIVATE val_t Obj_keys(struct v7 *v7, val_t this_obj, val_t args) {
  (void) this_obj;
  return _Obj_ownKeys(v7, args, V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM);
}

V7_PRIVATE val_t Obj_getOwnPropertyNames(struct v7 *v7, val_t this_obj, val_t args) {
  (void) this_obj;
  return _Obj_ownKeys(v7, args, V7_PROPERTY_HIDDEN);
}

V7_PRIVATE val_t Obj_getOwnPropertyDescriptor(struct v7 *v7, val_t this_obj, val_t args) {
  struct v7_property *prop;
  val_t obj = v7_array_at(v7, args, 0);
  val_t name = v7_array_at(v7, args, 1);
  val_t desc;
  (void) this_obj;
  if ((prop = _Obj_getOwnProperty(v7, obj, name)) == NULL) {
    return V7_UNDEFINED;
  }
  desc = v7_create_object(v7);
  v7_set_property(v7, desc, "value", 5, 0, prop->value);
  v7_set_property(v7, desc, "writable", 8, 0, v7_create_boolean(
      !(prop->attributes & V7_PROPERTY_READ_ONLY)));
  v7_set_property(v7, desc, "enumerable", 10, 0, v7_create_boolean(
      !(prop->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM))));
  v7_set_property(v7, desc, "configurable", 12, 0, v7_create_boolean(
      !(prop->attributes & V7_PROPERTY_DONT_DELETE)));

  return desc;
}

V7_PRIVATE val_t _Obj_defineProperty(struct v7 *v7, val_t obj, const char *name,
                                     int name_len, val_t desc) {
  unsigned int flags = 0;
  val_t val = v7_property_value(v7_get_property(desc, "value", 5));
  if (!v7_is_true(v7, v7_property_value(v7_get_property(desc, "enumerable",
                                                       10)))) {
    flags |= V7_PROPERTY_DONT_ENUM;
  }
  if (!v7_is_true(v7, v7_property_value(v7_get_property(desc, "writable",
                                                       8)))) {
    flags |= V7_PROPERTY_READ_ONLY;
  }
  if (!v7_is_true(v7, v7_property_value(v7_get_property(desc, "configurable",
                                                       12)))) {
    flags |= V7_PROPERTY_DONT_DELETE;
  }

  v7_set_property(v7, obj, name, name_len, flags, val);
  return obj;
}

V7_PRIVATE val_t Obj_defineProperty(struct v7 *v7, val_t this_obj, val_t args) {
  val_t obj = v7_array_at(v7, args, 0);
  val_t name = v7_array_at(v7, args, 1);
  val_t desc = v7_array_at(v7, args, 2);
  char name_buf[512];
  int name_len;
  (void) this_obj;
  name_len = v7_stringify_value(v7, name, name_buf, sizeof(name_buf));
  return _Obj_defineProperty(v7, obj, name_buf, name_len, desc);
}

V7_PRIVATE val_t Obj_defineProperties(struct v7 *v7, val_t this_obj, val_t args) {
  struct v7_property *p;
  val_t obj = v7_array_at(v7, args, 0);
  val_t descs = v7_array_at(v7, args, 1);
  (void) this_obj;

  if (!v7_is_object(descs)) {
    throw_exception(v7, "TypeError", "object expected");
  }
  for (p = v7_to_object(descs)->properties; p; p = p->next) {
    if (p->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)) {
      continue;
    }
    _Obj_defineProperty(v7, obj, p->name, strlen(p->name), p->value);
  }
  return obj;
}

V7_PRIVATE val_t Obj_propertyIsEnumerable(struct v7 *v7, val_t this_obj,
                                          val_t args) {
  struct v7_property *prop;
  val_t name = v7_array_at(v7, args, 0);
  if ((prop = _Obj_getOwnProperty(v7, this_obj, name)) == NULL) {
    return v7_create_boolean(0);
  }
  return v7_create_boolean(!(prop->attributes &
                             (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)));
}

V7_PRIVATE val_t Obj_hasOwnProperty(struct v7 *v7, val_t this_obj, val_t args) {
  val_t name = v7_array_at(v7, args, 0);
  return v7_create_boolean(_Obj_getOwnProperty(v7, this_obj, name) != NULL);
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
  val_t object, v;
  /* TODO(mkm): initialize global object without requiring a parser */
  v7_exec(v7, &v, "function Object() {}");

  object = v7_property_value(v7_get_property(v7->global_object, "Object", 6));
  v7_set_property(v7, object, "prototype", 9, 0, v7->object_prototype);

  set_cfunc_prop(v7, object, "getPrototypeOf", Obj_getPrototypeOf);
  set_cfunc_prop(v7, object, "isPrototypeOf", Obj_isPrototypeOf);
  set_cfunc_prop(v7, object, "getOwnPropertyDescriptor",
                 Obj_getOwnPropertyDescriptor);
  set_cfunc_prop(v7, object, "defineProperty", Obj_defineProperty);
  set_cfunc_prop(v7, object, "defineProperties", Obj_defineProperties);
  set_cfunc_prop(v7, object, "create", Obj_create);
  set_cfunc_prop(v7, object, "keys", Obj_keys);
  set_cfunc_prop(v7, object, "getOwnPropertyNames", Obj_getOwnPropertyNames);
  set_cfunc_prop(v7, v7->object_prototype, "propertyIsEnumerable",
                 Obj_propertyIsEnumerable);
  set_cfunc_prop(v7, v7->object_prototype, "hasOwnProperty",
                 Obj_hasOwnProperty);
}
