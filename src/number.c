/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t Number_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_length(v7, args) <= 0 ? v7_create_number(0.0)
                                              : v7_array_get(v7, args, 0);
  val_t res = v7_is_double(arg0) ? arg0 : v7_create_number(i_as_num(v7, arg0));

  if (v7_is_object(this_obj) && this_obj != v7->global_object) {
    v7_to_object(this_obj)->prototype = v7_to_object(v7->number_prototype);
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, res);
    res = this_obj;
  }

  return res;
}

static val_t n_to_str(struct v7 *v7, val_t t, val_t args, const char *format) {
  val_t arg0 = v7_array_get(v7, args, 0);
  double d = i_as_num(v7, arg0);
  int len, digits = d > 0 ? (int) d : 0;
  char fmt[10], buf[100];

  snprintf(fmt, sizeof(fmt), format, digits);
  len = snprintf(buf, sizeof(buf), fmt, v7_to_double(i_value_of(v7, t)));

  return v7_create_string(v7, buf, len, 1);
}

static val_t Number_toFixed(struct v7 *v7, val_t this_obj, val_t args) {
  return n_to_str(v7, this_obj, args, "%%.%dlf");
}

static val_t Number_toExp(struct v7 *v7, val_t this_obj, val_t args) {
  return n_to_str(v7, this_obj, args, "%%.%de");
}

static val_t Number_toPrecision(struct v7 *v7, val_t this_obj, val_t args) {
  return Number_toExp(v7, this_obj, args);
}

static val_t Number_valueOf(struct v7 *v7, val_t this_obj, val_t args) {
  if (!v7_is_double(this_obj) &&
      (v7_is_object(this_obj) &&
       v7_object_to_value(v7_to_object(this_obj)->prototype) !=
           v7->number_prototype)) {
    throw_exception(v7, TYPE_ERROR,
                    "Number.valueOf called on non-number object");
  }
  return Obj_valueOf(v7, this_obj, args);
}

static val_t Number_toString(struct v7 *v7, val_t this_obj, val_t args) {
  char buf[512];
  (void) args;

  if (this_obj == v7->number_prototype) {
    return v7_create_string(v7, "0", 1, 1);
  }

  if (!v7_is_double(this_obj) &&
      !(v7_is_object(this_obj) &&
        is_prototype_of(v7, this_obj, v7->number_prototype))) {
    throw_exception(v7, TYPE_ERROR,
                    "Number.toString called on non-number object");
  }

  /* TODO(mkm) handle radix first arg */
  v7_stringify_value(v7, i_value_of(v7, this_obj), buf, sizeof(buf));
  return v7_create_string(v7, buf, strlen(buf), 1);
}

static val_t n_isNaN(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_get(v7, args, 0);
  (void) this_obj;
  return v7_create_boolean(!v7_is_double(arg0) || arg0 == V7_TAG_NAN);
}

V7_PRIVATE void init_number(struct v7 *v7) {
  unsigned int attrs =
      V7_PROPERTY_READ_ONLY | V7_PROPERTY_DONT_ENUM | V7_PROPERTY_DONT_DELETE;
  val_t num =
      v7_create_cfunction_ctor(v7, v7->number_prototype, Number_ctor, 1);
  v7_set_property(v7, v7->global_object, "Number", 6, V7_PROPERTY_DONT_ENUM,
                  num);

  set_cfunc_prop(v7, v7->number_prototype, "toFixed", Number_toFixed);
  set_cfunc_prop(v7, v7->number_prototype, "toPrecision", Number_toPrecision);
  set_cfunc_prop(v7, v7->number_prototype, "toExponential", Number_toExp);
  set_cfunc_prop(v7, v7->number_prototype, "valueOf", Number_valueOf);
  set_cfunc_prop(v7, v7->number_prototype, "toString", Number_toString);

  v7_set_property(v7, num, "MAX_VALUE", 9, attrs,
                  v7_create_number(1.7976931348623157e+308));
  v7_set_property(v7, num, "MIN_VALUE", 9, attrs, v7_create_number(5e-324));
  v7_set_property(v7, num, "NEGATIVE_INFINITY", 17, attrs,
                  v7_create_number(-INFINITY));
  v7_set_property(v7, num, "POSITIVE_INFINITY", 17, attrs,
                  v7_create_number(INFINITY));
  v7_set_property(v7, num, "NaN", 3, attrs, V7_TAG_NAN);

  v7_set_property(v7, v7->global_object, "NaN", 3, attrs, V7_TAG_NAN);
  v7_set_property(v7, v7->global_object, "isNaN", 5, V7_PROPERTY_DONT_ENUM,
                  v7_create_cfunction(n_isNaN));
}
