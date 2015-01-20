/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE val_t Math_random(struct v7 *v7, val_t this_obj, val_t args) {
  static int srand_called = 0;

  if (!srand_called) {
    srand((unsigned)(unsigned long) v7);
    srand_called++;
  }

  (void) this_obj;
  (void) args;
  return v7_create_number((double) rand() / RAND_MAX);
}

V7_PRIVATE val_t Math_sin(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  (void) this_obj;
  return v7_create_number(sin(v7_to_double(arg0)));
}

V7_PRIVATE val_t Math_sqrt(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  (void) this_obj;
  return v7_create_number(sqrt(v7_to_double(arg0)));
}

V7_PRIVATE val_t Math_tan(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  (void) this_obj;
  return v7_create_number(tan(v7_to_double(arg0)));
}

V7_PRIVATE val_t Math_pow(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  val_t arg1 = v7_array_at(v7, args, 0);
  (void) this_obj;
  return v7_create_number(pow(v7_to_double(arg0), v7_to_double(arg1)));
}

V7_PRIVATE val_t Math_floor(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  (void) this_obj;
  return v7_create_number(floor(v7_to_double(arg0)));
}

V7_PRIVATE val_t Math_ceil(struct v7 *v7, val_t this_obj, val_t args) {
  val_t arg0 = v7_array_at(v7, args, 0);
  (void) this_obj;
  return v7_create_number(ceil(v7_to_double(arg0)));
}

V7_PRIVATE void init_math(struct v7 *v7) {
  val_t math = v7_create_object(v7);

  set_cfunc_prop(v7, math, "random", Math_random);
  set_cfunc_prop(v7, math, "pow", Math_pow);
  set_cfunc_prop(v7, math, "sin", Math_sin);
  set_cfunc_prop(v7, math, "tan", Math_tan);
  set_cfunc_prop(v7, math, "sqrt", Math_sqrt);
  set_cfunc_prop(v7, math, "floor", Math_floor);
  set_cfunc_prop(v7, math, "ceil", Math_ceil);

  v7_set_property(v7, math, "E", 1, 0, v7_create_number(M_E));
  v7_set_property(v7, math, "PI", 2, 0, v7_create_number(M_PI));
  v7_set_property(v7, math, "LN2", 3, 0, v7_create_number(M_LN2));
  v7_set_property(v7, math, "LN10", 4, 0, v7_create_number(M_LN10));
  v7_set_property(v7, math, "LOG2E", 5, 0, v7_create_number(M_LOG2E));
  v7_set_property(v7, math, "LOG10E", 6, 0, v7_create_number(M_LOG10E));
  v7_set_property(v7, math, "SQRT1_2", 7, 0, v7_create_number(M_SQRT1_2));
  v7_set_property(v7, math, "SQRT2", 5, 0, v7_create_number(M_SQRT2));

  v7_set_property(v7, v7->global_object, "Math", 4, 0, math);
  v7_set_property(v7, v7->global_object, "NaN", 3, 0, V7_TAG_NAN);
}
