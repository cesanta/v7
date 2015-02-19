/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

static val_t m_one_arg(struct v7 *v7, val_t args, double (*f)(double)) {
  val_t arg0 = v7_array_at(v7, args, 0);
  return v7_create_number(f(v7_to_double(arg0)));
}

static val_t m_two_arg(struct v7 *v7, val_t args, double (*f)(double, double)) {
  val_t arg0 = v7_array_at(v7, args, 0);
  val_t arg1 = v7_array_at(v7, args, 1);
  return v7_create_number(f(v7_to_double(arg0), v7_to_double(arg1)));
}

#define DEFINE_WRAPPER(name, func)                                          \
V7_PRIVATE val_t Math_##name(struct v7 *v7, val_t this_obj, val_t args) {   \
  (void) this_obj;                                                      \
  return func(v7, args, name);                                          \
}

DEFINE_WRAPPER(fabs, m_one_arg)
DEFINE_WRAPPER(acos, m_one_arg)
DEFINE_WRAPPER(asin, m_one_arg)
DEFINE_WRAPPER(atan, m_one_arg)
DEFINE_WRAPPER(atan2, m_two_arg)
DEFINE_WRAPPER(ceil, m_one_arg)
DEFINE_WRAPPER(cos, m_one_arg)
DEFINE_WRAPPER(exp, m_one_arg)
DEFINE_WRAPPER(floor, m_one_arg)
DEFINE_WRAPPER(log, m_one_arg)
DEFINE_WRAPPER(pow, m_two_arg)
DEFINE_WRAPPER(round, m_one_arg)
DEFINE_WRAPPER(sin, m_one_arg)
DEFINE_WRAPPER(sqrt, m_one_arg)
DEFINE_WRAPPER(tan, m_one_arg)

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

static val_t min_max(struct v7 *v7, val_t args, int is_min) {
  double res = NAN;
  int i, len = v7_array_length(v7, args);

  for (i = 0; i < len; i++) {
    double v = v7_to_double(v7_array_at(v7, args, i));
    if (isnan(res) || (is_min && v < res) || (!is_min && v > res)) {
      res = v;
    }
  }

  return v7_create_number(res);
}

V7_PRIVATE val_t Math_min(struct v7 *v7, val_t this_obj, val_t args) {
  (void) this_obj;
  return min_max(v7, args, 1);
}

V7_PRIVATE val_t Math_max(struct v7 *v7, val_t this_obj, val_t args) {
  (void) this_obj;
  return min_max(v7, args, 0);
}

V7_PRIVATE void init_math(struct v7 *v7) {
  val_t math = v7_create_object(v7);

  set_cfunc_prop(v7, math, "abs", Math_fabs);
  set_cfunc_prop(v7, math, "acos", Math_acos);
  set_cfunc_prop(v7, math, "asin", Math_asin);
  set_cfunc_prop(v7, math, "atan", Math_atan);
  set_cfunc_prop(v7, math, "atan2", Math_atan2);
  set_cfunc_prop(v7, math, "ceil", Math_ceil);
  set_cfunc_prop(v7, math, "cos", Math_cos);
  set_cfunc_prop(v7, math, "exp", Math_exp);
  set_cfunc_prop(v7, math, "floor", Math_floor);
  set_cfunc_prop(v7, math, "log", Math_log);
  set_cfunc_prop(v7, math, "max", Math_max);
  set_cfunc_prop(v7, math, "min", Math_min);
  set_cfunc_prop(v7, math, "pow", Math_pow);
  set_cfunc_prop(v7, math, "random", Math_random);
  set_cfunc_prop(v7, math, "round", Math_round);
  set_cfunc_prop(v7, math, "sin", Math_sin);
  set_cfunc_prop(v7, math, "sqrt", Math_sqrt);
  set_cfunc_prop(v7, math, "tan", Math_tan);

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
