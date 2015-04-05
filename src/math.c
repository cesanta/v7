/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"

#if V7_ENABLE__Math

#ifdef __WATCOM__
int matherr(struct _exception *exc) {
  if (exc->type == DOMAIN) {
    exc->retval = NAN;
    return 0;
  }
}
#endif

#if V7_ENABLE__Math__abs || V7_ENABLE__Math__acos || V7_ENABLE__Math__asin ||  \
    V7_ENABLE__Math__atan || V7_ENABLE__Math__ceil || V7_ENABLE__Math__cos ||  \
    V7_ENABLE__Math__exp || V7_ENABLE__Math__floor || V7_ENABLE__Math__log ||  \
    V7_ENABLE__Math__round || V7_ENABLE__Math__sin || V7_ENABLE__Math__sqrt || \
    V7_ENABLE__Math__tan
static val_t m_one_arg(struct v7 *v7, val_t args, double (*f)(double)) {
  val_t arg0 = v7_array_get(v7, args, 0);
  double d0 = v7_to_double(arg0);
#ifdef V7_BROKEN_NAN
  if (isnan(d0)) return V7_TAG_NAN;
#endif
  return v7_create_number(f(d0));
}
#endif /* V7_ENABLE__Math__* */

#if V7_ENABLE__Math__pow || V7_ENABLE__Math__atan2
static val_t m_two_arg(struct v7 *v7, val_t args, double (*f)(double, double)) {
  val_t arg0 = v7_array_get(v7, args, 0);
  val_t arg1 = v7_array_get(v7, args, 1);
  double d0 = v7_to_double(arg0);
  double d1 = v7_to_double(arg1);
#ifdef V7_BROKEN_NAN
  /* pow(NaN,0) == 1, doesn't fix atan2, but who cares */
  if (isnan(d1)) return V7_TAG_NAN;
#endif
  return v7_create_number(f(d0, d1));
}
#endif /* V7_ENABLE__Math__pow || V7_ENABLE__Math__atan2 */

#define DEFINE_WRAPPER(name, func)                                          \
  V7_PRIVATE val_t Math_##name(struct v7 *v7, val_t this_obj, val_t args) { \
    (void) this_obj;                                                        \
    return func(v7, args, name);                                            \
  }

/* Visual studio 2012+ has round() */
#if V7_ENABLE__Math__round && \
    ((defined(V7_WINDOWS) && _MSC_VER < 1700) || defined(__WATCOM__))
static double round(double n) {
  return n;
}
#endif

#if V7_ENABLE__Math__abs
DEFINE_WRAPPER(fabs, m_one_arg)
#endif
#if V7_ENABLE__Math__acos
DEFINE_WRAPPER(acos, m_one_arg)
#endif
#if V7_ENABLE__Math__asin
DEFINE_WRAPPER(asin, m_one_arg)
#endif
#if V7_ENABLE__Math__atan
DEFINE_WRAPPER(atan, m_one_arg)
#endif
#if V7_ENABLE__Math__atan2
DEFINE_WRAPPER(atan2, m_two_arg)
#endif
#if V7_ENABLE__Math__ceil
DEFINE_WRAPPER(ceil, m_one_arg)
#endif
#if V7_ENABLE__Math__cos
DEFINE_WRAPPER(cos, m_one_arg)
#endif
#if V7_ENABLE__Math__exp
DEFINE_WRAPPER(exp, m_one_arg)
#endif
#if V7_ENABLE__Math__floor
DEFINE_WRAPPER(floor, m_one_arg)
#endif
#if V7_ENABLE__Math__log
DEFINE_WRAPPER(log, m_one_arg)
#endif
#if V7_ENABLE__Math__pow
DEFINE_WRAPPER(pow, m_two_arg)
#endif
#if V7_ENABLE__Math__round
DEFINE_WRAPPER(round, m_one_arg)
#endif
#if V7_ENABLE__Math__sin
DEFINE_WRAPPER(sin, m_one_arg)
#endif
#if V7_ENABLE__Math__sqrt
DEFINE_WRAPPER(sqrt, m_one_arg)
#endif
#if V7_ENABLE__Math__tan
DEFINE_WRAPPER(tan, m_one_arg)
#endif

#if V7_ENABLE__Math__random
V7_PRIVATE val_t Math_random(struct v7 *v7, val_t this_obj, val_t args) {
  static int srand_called = 0;

  if (!srand_called) {
    srand((unsigned) (unsigned long) v7);
    srand_called++;
  }

  (void) this_obj;
  (void) args;
  return v7_create_number((double) rand() / RAND_MAX);
}
#endif /* V7_ENABLE__Math__random */

#if V7_ENABLE__Math__min || V7_ENABLE__Math__max
static val_t min_max(struct v7 *v7, val_t args, int is_min) {
  double res = NAN;
  int i, len = v7_array_length(v7, args);

  for (i = 0; i < len; i++) {
    double v = v7_to_double(v7_array_get(v7, args, i));
    if (isnan(res) || (is_min && v < res) || (!is_min && v > res)) {
      res = v;
    }
  }

  return v7_create_number(res);
}
#endif /* V7_ENABLE__Math__min || V7_ENABLE__Math__max */

#if V7_ENABLE__Math__min
V7_PRIVATE val_t Math_min(struct v7 *v7, val_t this_obj, val_t args) {
  (void) this_obj;
  return min_max(v7, args, 1);
}
#endif

#if V7_ENABLE__Math__max
V7_PRIVATE val_t Math_max(struct v7 *v7, val_t this_obj, val_t args) {
  (void) this_obj;
  return min_max(v7, args, 0);
}
#endif

V7_PRIVATE void init_math(struct v7 *v7) {
  val_t math = v7_create_object(v7);

#if V7_ENABLE__Math__abs
  set_cfunc_prop(v7, math, "abs", Math_fabs);
#endif
#if V7_ENABLE__Math__acos
  set_cfunc_prop(v7, math, "acos", Math_acos);
#endif
#if V7_ENABLE__Math__asin
  set_cfunc_prop(v7, math, "asin", Math_asin);
#endif
#if V7_ENABLE__Math__atan
  set_cfunc_prop(v7, math, "atan", Math_atan);
#endif
#if V7_ENABLE__Math__atan2
  set_cfunc_prop(v7, math, "atan2", Math_atan2);
#endif
#if V7_ENABLE__Math__ceil
  set_cfunc_prop(v7, math, "ceil", Math_ceil);
#endif
#if V7_ENABLE__Math__cos
  set_cfunc_prop(v7, math, "cos", Math_cos);
#endif
#if V7_ENABLE__Math__exp
  set_cfunc_prop(v7, math, "exp", Math_exp);
#endif
#if V7_ENABLE__Math__floor
  set_cfunc_prop(v7, math, "floor", Math_floor);
#endif
#if V7_ENABLE__Math__log
  set_cfunc_prop(v7, math, "log", Math_log);
#endif
#if V7_ENABLE__Math__max
  set_cfunc_prop(v7, math, "max", Math_max);
#endif
#if V7_ENABLE__Math__min
  set_cfunc_prop(v7, math, "min", Math_min);
#endif
#if V7_ENABLE__Math__pow
  set_cfunc_prop(v7, math, "pow", Math_pow);
#endif
#if V7_ENABLE__Math__random
  set_cfunc_prop(v7, math, "random", Math_random);
#endif
#if V7_ENABLE__Math__round
  set_cfunc_prop(v7, math, "round", Math_round);
#endif
#if V7_ENABLE__Math__sin
  set_cfunc_prop(v7, math, "sin", Math_sin);
#endif
#if V7_ENABLE__Math__sqrt
  set_cfunc_prop(v7, math, "sqrt", Math_sqrt);
#endif
#if V7_ENABLE__Math__tan
  set_cfunc_prop(v7, math, "tan", Math_tan);
#endif

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

#endif /* V7_ENABLE__Math */
