/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t Obj_getPrototypeOf(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg = v7_array_get(v7, args, 0);
  (void) this_obj;
  if (!v7_is_object(arg)) {
    throw_exception(v7, TYPE_ERROR,
                    "Object.getPrototypeOf called on non-object");
  }
  return v_get_prototype(v7, arg);
}

static val_t Obj_isPrototypeOf(struct v7 *v7, val_t this_obj, val_t args) {
  val_t obj = v7_array_get(v7, args, 0);
  val_t proto = v7_array_get(v7, args, 1);
  (void) this_obj;
  return v7_create_boolean(is_prototype_of(v7, obj, proto));
}

/* Hack to ensure that the iteration order of the keys array is consistent
 * with the iteration order if properties in `for in`
 * This will be obsoleted when arrays will have a special object type. */
static void _Obj_append_reverse(struct v7 *v7, struct v7_property *p, val_t res,
                                int i, unsigned int ignore_flags) {
  char buf[20];
  while (p && p->attributes & ignore_flags) p = p->next;
  if (p == NULL) return;
  if (p->next) _Obj_append_reverse(v7, p->next, res, i + 1, ignore_flags);

  snprintf(buf, sizeof(buf), "%d", i);
  v7_set_property(v7, res, buf, strlen(buf), 0, p->name);
}

static val_t _Obj_ownKeys(struct v7 *v7, val_t args,
                          unsigned int ignore_flags) {
  val_t obj = v7_array_get(v7, args, 0);
  val_t res = v7_create_array(v7);
  if (!v7_is_object(obj)) {
    throw_exception(v7, TYPE_ERROR, "Object.keys called on non-object");
  }

  _Obj_append_reverse(v7, v7_to_object(obj)->properties, res, 0, ignore_flags);
  return res;
}

static struct v7_property *_Obj_getOwnProperty(struct v7 *v7, val_t obj,
                                               val_t name) {
  char name_buf[512];
  int name_len;
  name_len = v7_stringify_value(v7, name, name_buf, sizeof(name_buf));
  return v7_get_own_property(v7, obj, name_buf, name_len);
}

static val_t Obj_keys(struct v7 *v7, val_t this_obj, val_t args) {
  (void) this_obj;
  return _Obj_ownKeys(v7, args, V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM);
}

static val_t Obj_getOwnPropertyNames(struct v7 *v7, val_t this_obj,
                                     val_t args) {
  (void) this_obj;
  return _Obj_ownKeys(v7, args, V7_PROPERTY_HIDDEN);
}

static val_t Obj_getOwnPropertyDescriptor(struct v7 *v7, val_t this_obj,
                                          val_t args) {
  struct v7_property *prop;
  val_t obj = v7_array_get(v7, args, 0);
  val_t name = v7_array_get(v7, args, 1);
  val_t desc;
  (void) this_obj;
  if ((prop = _Obj_getOwnProperty(v7, obj, name)) == NULL) {
    return V7_UNDEFINED;
  }
  desc = v7_create_object(v7);
  v7_set_property(v7, desc, "value", 5, 0, prop->value);
  v7_set_property(
      v7, desc, "writable", 8, 0,
      v7_create_boolean(!(prop->attributes & V7_PROPERTY_READ_ONLY)));
  v7_set_property(
      v7, desc, "enumerable", 10, 0,
      v7_create_boolean(
          !(prop->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM))));
  v7_set_property(
      v7, desc, "configurable", 12, 0,
      v7_create_boolean(!(prop->attributes & V7_PROPERTY_DONT_DELETE)));

  return desc;
}

static void o_set_attr(struct v7 *v7, val_t desc, const char *name, size_t n,
                       struct v7_property *prop, unsigned int attr) {
  val_t v = v7_get(v7, desc, name, n);
  if (v7_is_true(v7, v)) {
    prop->attributes &= ~attr;
  } else {
    prop->attributes |= attr;
  }
}

static val_t _Obj_defineProperty(struct v7 *v7, val_t obj, const char *name,
                                 int name_len, val_t desc) {
  val_t val = v7_get(v7, desc, "value", 5);
  struct v7_property *prop = v7_get_own_property(v7, obj, name, name_len);

  if (prop == NULL) {
    val_t key = v7_create_string(v7, name, name_len, 1);
    prop = v7_set_prop(v7, obj, key, 0, val);
  }

  if (prop == NULL) {
    throw_exception(v7, INTERNAL_ERROR, "OOM");
  } else {
    o_set_attr(v7, desc, "enumerable", 10, prop, V7_PROPERTY_DONT_ENUM);
    o_set_attr(v7, desc, "writable", 8, prop, V7_PROPERTY_READ_ONLY);
    o_set_attr(v7, desc, "configurable", 12, prop, V7_PROPERTY_DONT_DELETE);
    if (!v7_is_undefined(val)) {
      prop->value = val;
    }
  }

  return obj;
}

static val_t Obj_defineProperty(struct v7 *v7, val_t this_obj, val_t args) {
  val_t obj = v7_array_get(v7, args, 0);
  val_t name = v7_array_get(v7, args, 1);
  val_t desc = v7_array_get(v7, args, 2);
  char name_buf[512];
  int name_len;
  (void) this_obj;
  if (!v7_is_object(obj)) {
    throw_exception(v7, TYPE_ERROR, "object expected");
  }
  name_len = v7_stringify_value(v7, name, name_buf, sizeof(name_buf));
  return _Obj_defineProperty(v7, obj, name_buf, name_len, desc);
}

static void o_define_props(struct v7 *v7, val_t obj, val_t descs) {
  struct v7_property *p;
  if (!v7_is_object(descs)) {
    throw_exception(v7, TYPE_ERROR, "object expected");
  }
  for (p = v7_to_object(descs)->properties; p; p = p->next) {
    size_t n;
    const char *s = v7_to_string(v7, &p->name, &n);
    if (p->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)) {
      continue;
    }
    _Obj_defineProperty(v7, obj, s, n, p->value);
  }
}

static val_t Obj_defineProperties(struct v7 *v7, val_t this_obj, val_t args) {
  val_t obj = v7_array_get(v7, args, 0);
  val_t descs = v7_array_get(v7, args, 1);
  (void) this_obj;
  o_define_props(v7, obj, descs);
  return obj;
}

static val_t Obj_create(struct v7 *v7, val_t this_obj, val_t args) {
  val_t res, proto = v7_array_get(v7, args, 0);
  val_t descs = v7_array_get(v7, args, 1);
  (void) this_obj;
  if (!v7_is_null(proto) && !v7_is_object(proto)) {
    throw_exception(v7, TYPE_ERROR,
                    "Object prototype may only be an Object or null");
  }
  res = create_object(v7, proto);
  if (v7_is_object(descs)) {
    o_define_props(v7, res, descs);
  }
  return res;
}

static val_t Obj_propertyIsEnumerable(struct v7 *v7, val_t this_obj,
                                      val_t args) {
  struct v7_property *prop;
  val_t name = v7_array_get(v7, args, 0);
  if ((prop = _Obj_getOwnProperty(v7, this_obj, name)) == NULL) {
    return v7_create_boolean(0);
  }
  return v7_create_boolean(
      !(prop->attributes & (V7_PROPERTY_HIDDEN | V7_PROPERTY_DONT_ENUM)));
}

static val_t Obj_hasOwnProperty(struct v7 *v7, val_t this_obj, val_t args) {
  val_t name = v7_array_get(v7, args, 0);
  return v7_create_boolean(_Obj_getOwnProperty(v7, this_obj, name) != NULL);
}

#if 0
static enum v7_err Obj_toString(struct v7_c_func_arg *cfa) {
  char *p, buf[500];
  p = v7_stringify(cfa->this_obj, buf, sizeof(buf));
  v7_push_string(cfa->v7, p, strlen(p), 1);
  if (p != buf) free(p);
  return V7_OK;
}

static enum v7_err Obj_keys(struct v7_c_func_arg *cfa) {
  struct v7_prop *p;
  struct v7_val *result = v7_push_new_object(cfa->v7);
  v7_set_class(result, V7_CLASS_ARRAY);
  for (p = cfa->this_obj->props; p != NULL; p = p->next) {
    v7_append(cfa->v7, result, p->key);
  }
  return V7_OK;
}
#endif

V7_PRIVATE val_t Obj_valueOf(struct v7 *v7, val_t this_obj, val_t args) {
  val_t res = this_obj;
  struct v7_property *p;

  (void) args;
  p = v7_get_own_property2(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN);
  if (p != NULL) {
    res = p->value;
  }

  return res;
}

static val_t Obj_toString(struct v7 *v7, val_t this_obj, val_t args) {
  char buf[20];
  const char *type = "Object";
  (void) args;
  if (is_prototype_of(v7, this_obj, v7->array_prototype)) {
    type = "Array";
  }
  snprintf(buf, sizeof(buf), "[object %s]", type);
  return v7_create_string(v7, buf, strlen(buf), 1);
}

static val_t Obj_preventExtensions(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg = v7_array_get(v7, args, 0);
  (void) this_obj;
  if (!v7_is_object(arg)) {
    throw_exception(v7, TYPE_ERROR, "Object expected");
  }
  v7_to_object(arg)->attributes |= V7_OBJ_NOT_EXTENSIBLE;
  return arg;
}

static val_t Obj_isExtensible(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg = v7_array_get(v7, args, 0);
  (void) this_obj;
  if (!v7_is_object(arg)) {
    throw_exception(v7, TYPE_ERROR, "Object expected");
  }
  return v7_create_boolean(
      !(v7_to_object(arg)->attributes & V7_OBJ_NOT_EXTENSIBLE));
}

V7_PRIVATE void init_object(struct v7 *v7) {
  val_t object, v;
  /* TODO(mkm): initialize global object without requiring a parser */
  v7_exec(v7, &v,
          "function Object(v) {"
          "if (typeof v === 'boolean') return new Boolean(v);"
          "if (typeof v === 'number') return new Number(v);"
          "if (typeof v === 'string') return new String(v);"
          "if (typeof v === 'date') return new Date(v);"
          "}");

  object = v7_get(v7, v7->global_object, "Object", 6);
  v7_set(v7, object, "prototype", 9, v7->object_prototype);
  v7_set(v7, v7->object_prototype, "constructor", 11, object);

  set_cfunc_obj_prop(v7, v7->object_prototype, "toString", Obj_toString, 0);
  set_cfunc_prop(v7, object, "getPrototypeOf", Obj_getPrototypeOf);
  set_cfunc_prop(v7, object, "getOwnPropertyDescriptor",
                 Obj_getOwnPropertyDescriptor);
  set_cfunc_obj_prop(v7, object, "defineProperty", Obj_defineProperty, 3);
  set_cfunc_prop(v7, object, "defineProperties", Obj_defineProperties);
  set_cfunc_prop(v7, object, "create", Obj_create);
  set_cfunc_prop(v7, object, "keys", Obj_keys);
  set_cfunc_prop(v7, object, "getOwnPropertyNames", Obj_getOwnPropertyNames);
  set_cfunc_obj_prop(v7, object, "preventExtensions", Obj_preventExtensions, 1);
  set_cfunc_obj_prop(v7, object, "isExtensible", Obj_isExtensible, 1);

  set_cfunc_prop(v7, v7->object_prototype, "propertyIsEnumerable",
                 Obj_propertyIsEnumerable);
  set_cfunc_prop(v7, v7->object_prototype, "hasOwnProperty",
                 Obj_hasOwnProperty);
  set_cfunc_prop(v7, v7->object_prototype, "isPrototypeOf", Obj_isPrototypeOf);
  set_cfunc_prop(v7, v7->object_prototype, "valueOf", Obj_valueOf);
}
