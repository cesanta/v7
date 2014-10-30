#include "internal.h"

V7_PRIVATE enum v7_err Number_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj =
      cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  struct v7_val *arg = cfa->args[0];

  v7_init_num(obj, cfa->num_args > 0 ? arg->v.num : 0.0);

  if (cfa->num_args > 0 && !is_num(arg) && !is_bool(arg)) {
    if (is_string(arg)) {
      v7_init_num(obj, strtod(arg->v.str.buf, NULL));
    } else {
      v7_init_num(obj, NAN);
    }
  }

  if (cfa->called_as_constructor) {
    obj->type = V7_TYPE_OBJ;
    obj->proto = &s_prototypes[V7_CLASS_NUMBER];
    obj->ctor = &s_constructors[V7_CLASS_NUMBER];
  }

  return V7_OK;
}

V7_PRIVATE enum v7_err Num_toFixed(struct v7_c_func_arg *cfa) {
  int len, digits = cfa->num_args > 0 ? (int)cfa->args[0]->v.num : 0;
  char fmt[10], buf[100];

  snprintf(fmt, sizeof(fmt), "%%.%dlf", digits);
  len = snprintf(buf, sizeof(buf), fmt, cfa->this_obj->v.num);
  v7_push_string(cfa->v7, buf, len, 1);
  return V7_OK;
}

#define NUMCTOR s_constructors[V7_CLASS_NUMBER]
V7_PRIVATE void init_number(void) {
  init_standard_constructor(V7_CLASS_NUMBER, Number_ctor);
  SET_RO_PROP(NUMCTOR, "MAX_VALUE", V7_TYPE_NUM, num, LONG_MAX);
  SET_RO_PROP(NUMCTOR, "MIN_VALUE", V7_TYPE_NUM, num, LONG_MIN);
  SET_RO_PROP(NUMCTOR, "NaN", V7_TYPE_NUM, num, NAN);
  SET_METHOD(s_prototypes[V7_CLASS_NUMBER], "toFixed", Num_toFixed);
  SET_RO_PROP_V(s_global, "Number", s_constructors[V7_CLASS_NUMBER]);
}
