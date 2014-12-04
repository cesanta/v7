/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

V7_PRIVATE enum v7_err Math_random(struct v7_c_func_arg *cfa) {
  static int srand_called = 0;

  if (!srand_called) {
    srand((unsigned)(unsigned long) cfa);
    srand_called++;
  }

  v7_push_number(cfa->v7, (double)rand() / RAND_MAX);

  return V7_OK;
}

V7_PRIVATE enum v7_err Math_sin(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 1 ? sin(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_sqrt(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 1 ? sqrt(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_tan(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 1 ? tan(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_pow(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 2
                              ? pow(cfa->args[0]->v.num, cfa->args[1]->v.num)
                              : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_floor(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7,
                 cfa->num_args == 1 ? floor(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE enum v7_err Math_ceil(struct v7_c_func_arg *cfa) {
  v7_push_number(cfa->v7, cfa->num_args == 1 ? ceil(cfa->args[0]->v.num) : 0.0);
  return V7_OK;
}

V7_PRIVATE void init_math(void) {
  SET_METHOD(s_math, "random", Math_random);
  SET_METHOD(s_math, "pow", Math_pow);
  SET_METHOD(s_math, "sin", Math_sin);
  SET_METHOD(s_math, "tan", Math_tan);
  SET_METHOD(s_math, "sqrt", Math_sqrt);
  SET_METHOD(s_math, "floor", Math_floor);
  SET_METHOD(s_math, "ceil", Math_ceil);

  SET_RO_PROP(s_math, "E", V7_TYPE_NUM, num, M_E);
  SET_RO_PROP(s_math, "PI", V7_TYPE_NUM, num, M_PI);
  SET_RO_PROP(s_math, "LN2", V7_TYPE_NUM, num, M_LN2);
  SET_RO_PROP(s_math, "LN10", V7_TYPE_NUM, num, M_LN10);
  SET_RO_PROP(s_math, "LOG2E", V7_TYPE_NUM, num, M_LOG2E);
  SET_RO_PROP(s_math, "LOG10E", V7_TYPE_NUM, num, M_LOG10E);
  SET_RO_PROP(s_math, "SQRT1_2", V7_TYPE_NUM, num, M_SQRT1_2);
  SET_RO_PROP(s_math, "SQRT2", V7_TYPE_NUM, num, M_SQRT2);

  v7_set_class(&s_math, V7_CLASS_OBJECT);
  s_math.ref_count = 1;

  SET_RO_PROP_V(s_global, "Math", s_math);
}
