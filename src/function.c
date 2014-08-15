static enum v7_err Function_ctor(struct v7_c_func_arg *cfa) {
  struct v7_val *obj = cfa->called_as_constructor ? cfa->this_obj : cfa->result;
  v7_set_class(obj, V7_CLASS_FUNCTION);
  return V7_OK;
}

static void init_function(void) {
  SET_RO_PROP_V(s_global, "Function", s_constructors[V7_CLASS_FUNCTION]);
}
