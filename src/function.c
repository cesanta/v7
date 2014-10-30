#include "internal.h"

V7_PRIVATE enum v7_err Function_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj =
      cfa->called_as_constructor ? cfa->this_obj : v7_push_new_object(cfa->v7);
  v7_set_class(obj, V7_CLASS_FUNCTION);
  return V7_OK;
}

V7_PRIVATE void init_function(void) {
  init_standard_constructor(V7_CLASS_FUNCTION, Function_ctor);
  SET_RO_PROP_V(s_global, "Function", s_constructors[V7_CLASS_FUNCTION]);
}
