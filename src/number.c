/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t Number_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  /* TODO(lsm): if arg0 is not a number, do type conversion */
  val_t res = v7_is_double(arg0) ? arg0 : v7_create_number(NAN);

  if (v7_is_object(this_obj) && this_obj != v7->global_object) {
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, res);
    return this_obj;
  }

  return res;
}

static val_t n_to_str(struct v7 *v7, val_t t, val_t args, const char *format) {
  val_t arg0 = v7_array_at(v7, args, 0);
  double d = v7_to_double(arg0);
  int len, digits = v7_is_double(arg0) && d > 0 ? (int) d : 0;
  char fmt[10], buf[100];

  snprintf(fmt, sizeof(fmt), format, digits);
  len = snprintf(buf, sizeof(buf), fmt, v7_to_double(t));

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

V7_PRIVATE void init_number(struct v7 *v7) {
  val_t num = v7_create_cfunction(Number_ctor);
  v7_set_property(v7, v7->global_object, "Number", 6, 0, num);

  set_cfunc_prop(v7, v7->number_prototype, "toFixed", Number_toFixed);
  set_cfunc_prop(v7, v7->number_prototype, "toPrecision", Number_toPrecision);
  set_cfunc_prop(v7, v7->number_prototype, "toExponentioal", Number_toExp);

  v7_set_property(v7, v7->number_prototype, "MAX_VALUE", 9, 0,
                  v7_create_number(LONG_MAX));
  v7_set_property(v7, v7->number_prototype, "MIN_VALUE", 9, 0,
                  v7_create_number(LONG_MIN));
  v7_set_property(v7, v7->number_prototype, "NEGATIVE_INFINITY", 17, 0,
                  v7_create_number(-INFINITY));
  v7_set_property(v7, v7->number_prototype, "POSITIVE_INFINITY", 17, 0,
                  v7_create_number(INFINITY));
  v7_set_property(v7, v7->number_prototype, "NaN", 3, 0, V7_TAG_NAN);
}
